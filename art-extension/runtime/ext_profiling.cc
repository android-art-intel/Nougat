/*
 * Copyright (C) 2015 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ext_profiling.h"

#include <algorithm>
#include "base/mutex.h"
#include "base/scoped_flock.h"
#include "base/unix_file/fd_file.h"
#include "dex_file.h"
#include "driver/compiler_driver.h"
#include "graph_x86.h"
#include "mirror/object-inl.h"
#include "thread.h"
#include "nodes.h"
#include "oat_file.h"
#include "oat_file_manager.h"
#include "os.h"
#include "runtime.h"
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __ANDROID__
#include "cutils/properties.h"
#endif

#define MORE_DEBUG 0

namespace art {

  struct foobar {
    foobar() {
#ifdef __ANDROID__
      const char* propertyName = "dalvik.vm.log_exact";
      char bar[PROPERTY_VALUE_MAX];
      if (property_get(propertyName, bar, "") > 0 && strcmp(bar, "true") == 0) {
        gLogVerbosity.exact_profiler = true;
      }
#else
      const char* p = getenv("LOG_EXACT");
      if (p != nullptr) {
        gLogVerbosity.exact_profiler = true;
      }
#endif
    }
  } foobar_init;

std::set<ExactProfiler*> ExactProfiler::all_exact_profiles_;
ExactProfiler::MethodProfileMap ExactProfiler::profile_infos_;

ExactProfiler::ExactProfiler(const std::string& oat_file_name,
                             const std::string& profile_dir)
    : method_lock_("method info lock"),
      oat_file_name_(oat_file_name) {
  VLOG(exact_profiler) << "ExactProfiler ctor: " << oat_file_name
                       << ", " << profile_dir;

  // Compute the prof filename from the name of the OAT file.
  size_t last_dot = oat_file_name.rfind('.');
  size_t last_slash = oat_file_name.rfind('/');

  // Ensure that we only care about the last component of the name.
  if (last_slash != std::string::npos && last_dot < last_slash) {
    last_dot = std::string::npos;
  }

  // Use the profile_dir if we have one.
  if (!profile_dir.empty()) {
      prof_file_name_ = profile_dir;
  } else {
    // Figure out the directory name.
    if (last_slash == std::string::npos) {
      // The file is in the current directory.
      prof_file_name_ = "profile/";
    } else {
      prof_file_name_ = oat_file_name.substr(0, last_slash+1) + "profile";
    }
  }

  // Save the path up until the last component for later use.
  prof_dir_name_ = prof_file_name_;

  // Now append the filename.prof.
  if (last_slash == std::string::npos) {
    // Use the whole pathname.
    last_slash = 0;
  }

  if (last_dot == std::string::npos) {
    // Just use the whole thing.
    prof_file_name_ += oat_file_name.substr(last_slash);
  } else {
    // Replace the last component with .prof.
    prof_file_name_ += oat_file_name.substr(last_slash, last_dot - last_slash);
  }
  prof_file_name_ += ".prof";

  // Register this profile for GC.
  MutexLock mu(nullptr, method_lock_);
  all_exact_profiles_.insert(this);
}

ExactProfiler::~ExactProfiler() {
  for (auto it : existing_profiles_) {
    delete it;
  }
}

void ExactProfiler::RegisterMethod(HGraph_X86& graph) {

  if (graph.GetNumProfiledBlocks() == 0) {
    return;
  }

  const DexFile& dex_file = graph.GetDexFile();
  int method_idx = graph.GetMethodIdx();

  // Find the Dex file in info_.
  MutexLock mu(nullptr, method_lock_);
  auto dex_it = info_.find(&dex_file);
  if (dex_it == info_.end()) {
    // First time we have seen this dex file.
    dex_it = info_.Put(&dex_file, PerMethodInformation());
  }

  // Now find the method in the dex file.
  PerMethodInformation& method_info = dex_it->second;

  // Set the block count and invoke dex_pcs for the method.
  ProfileInformation prof_info(graph.GetNumProfiledBlocks(), std::vector<uint16_t>());

  if (graph.HasProfiledInvokesDexPcs()) {
    ArenaVector<uint16_t>& profiled_invokes = graph.GetProfiledInvokesDexPcs();
    std::vector<uint16_t>& dex_pcs = prof_info.second;
    dex_pcs.reserve(profiled_invokes.size());
    for (auto dex_pc : profiled_invokes) {
      dex_pcs.push_back(dex_pc);
    }
  }
  method_info.Put(method_idx, prof_info);
}

// Make one directory.
static int do_mkdir(const char *path, mode_t mode) {
  struct stat st;
  int status = 0;

  if (stat(path, &st) != 0) {
    /* Directory does not exist. EEXIST for race condition */
    if (mkdir(path, mode) != 0 && errno != EEXIST) {
      LOG(INFO) << "mkdir " << path << " failed: " << strerror(errno);
      status = -1;
    }
  } else if (!S_ISDIR(st.st_mode)) {
    errno = ENOTDIR;
    status = -1;
  }

  return status;
}

// mkpath - ensure all directories in path exist.
int mkpath(const char *path, mode_t mode) {
  char* copypath = new char[strlen(path)+1];
  strcpy(copypath, path);

  int status = 0;
  char* pp = copypath;
  char* sp;
  while (status == 0 && (sp = strchr(pp, '/')) != 0) {
    if (sp != pp) {
      // Neither root nor double slash in path.
      *sp = '\0';
      status = do_mkdir(copypath, mode);
      *sp = '/';
    }
    pp = sp + 1;
  }
  if (status == 0)
    status = do_mkdir(path, mode);
  delete [] copypath;
  return status;
}

class OpenLocked {
 public:
  explicit OpenLocked(const char *file_name) {
    fd_ = ::open(file_name, O_RDWR|O_CREAT, 0666);
    if (fd_ < 0) {
      LOG(WARNING) << "Unable to open profile file '" << file_name
                   << "' for update: " << strerror(errno);
    } else if (flock(fd_, LOCK_EX) < 0) {
      LOG(WARNING) << "Unable to flock profile file '" << file_name
                   << "' for update: " << strerror(errno);
      close(fd_);
      fd_ = -1;
    }
  }

  ~OpenLocked() {
    if (fd_ >= 0) {
      close(fd_);
    }
  }

  bool IsOpen() const { return fd_ >= 0; }

  int Get() const { DCHECK_GE(fd_, 0); return fd_; }

 private:
  int fd_;
};

static bool checked_write(int fd, const char *buffer, ssize_t size, const std::string& fname) {
  if (write(fd, buffer, size) != size) {
    LOG(ERROR) << "Error writing data to profile file: " << fname << ": " << strerror(errno);
    close(fd);
    return false;
  }
  return true;
}

bool ExactProfiler::WriteProfileFile(bool update_checksum_if_needed) {
  VLOG(exact_profiler) << "WriteProfileFile";
  ExactProfileFile prof_file;
  MutexLock mu(nullptr, method_lock_);

  std::unique_ptr<File> oat_file(OS::OpenFileForReading(oat_file_name_.c_str()));
  if (!oat_file) {
    LOG(ERROR) << "Unable to open OAT file: " << oat_file_name_;
    return false;
  }

  std::string error_msg;
  std::unique_ptr<OatFile> oat(
      OatFile::OpenReadable(oat_file.get(), oat_file_name_, nullptr, &error_msg));
  if (!oat) {
    LOG(ERROR) << "Failed opening OAT file: " << error_msg;
    return false;
  }

  uint32_t oat_check_sum = oat->GetOatHeader().GetChecksum();

  if (update_checksum_if_needed) {
    // If we already have a profile, but we have recompiled, we need to update
    // the checksum in the file, as the OAT file has probably changed.
    OpenLocked prof_fd(prof_file_name_.c_str());
    if (prof_fd.IsOpen()) {
      // The profile exists.  Do we have to update it?
      int fd = prof_fd.Get();
      ExactProfileFile old_header;
      size_t num_bytes = ::read(fd, &old_header, sizeof(old_header));
      if (num_bytes == sizeof(old_header)) {
        if (oat_check_sum != old_header.oat_checksum) {
          // Update it in the file.
          old_header.oat_checksum = oat_check_sum;
          ::lseek(fd, 0, SEEK_SET);
          num_bytes = ::write(fd, &old_header, sizeof(old_header));
          DCHECK_EQ(num_bytes, sizeof(old_header));
          VLOG(exact_profiler) << "Updated checksum in " << prof_file_name_;
        } else {
          VLOG(exact_profiler) << "Same checksum in " << prof_file_name_;
        }
        // The checksum is now correct.
        return true;
      }
    }
    // We failed for some reason.  Write a new profile file.
    VLOG(exact_profiler) << "Checksum update failed";
  }

  prof_file.magic1 = Magic1_;
  prof_file.version = CurrentVersion_;
  prof_file.runtime_temp = 0;
  prof_file.generating_profile = 0;

  // Fields to be accumulated.
  prof_file.total_num_counters = 0;
  prof_file.total_num_methods = 0;

  // Support for OAT class information for invokes.
  prof_file.offset_to_oat_table = 0;
  prof_file.offset_to_dex_table = 0;
  prof_file.offset_to_string_table = 0;

  const std::vector<const OatDexFile*>& oat_dex_files = oat->GetOatDexFiles();
  prof_file.num_dex_files = oat_dex_files.size();
  prof_file.oat_checksum = oat_check_sum;

  // Estimate size for one_method_data.
  uint32_t total_num_methods = 0;
  for (auto& it : info_) {
    total_num_methods += it.second.size();
  }

  // Remember where we are for offset to first method in dex file.
  uint32_t current_offset =
      sizeof(ExactProfileFile) +                     // Header.
      sizeof(uint32_t) * prof_file.num_dex_files;    // Offsets to dex files.

  std::vector<OneDexFile> dex_file_data;
  dex_file_data.reserve(prof_file.num_dex_files);
  std::vector<uint32_t> one_method_data;
  one_method_data.reserve(total_num_methods * 10);
  std::vector<char> name_buffer;
  PerMethodInformation empty_method_info;

  // Calculate the offsets to the dex files.
  std::vector<uint32_t> offset_to_dex_files;
  offset_to_dex_files.reserve(prof_file.num_dex_files);
  for (uint32_t i = 0; i < prof_file.num_dex_files; i++) {
    offset_to_dex_files.push_back(current_offset);
    current_offset += sizeof(OneDexFile);
  }

  // Process each Dex file in the OAT.
  for (auto it : oat_dex_files) {
    // Find the matching DexFile.
    const std::string& dex_file_name = it->GetDexFileLocation();
    std::unique_ptr<const DexFile> dex_file_from_oat;
    const DexFile* df = nullptr;
    PerMethodInformation* method_info = nullptr;
    for (auto& it2 : info_) {
      if (it2.first->GetLocation() == dex_file_name) {
        df = it2.first;
        method_info = &it2.second;
        break;
      }
    }
    if (df == nullptr) {
      // Must be JNI/Quick/interpreted Dex file.
      VLOG(exact_profiler) << "Unable to match dex file: " << dex_file_name;
      std::string error_message;
      dex_file_from_oat = it->OpenDexFile(&error_message);
      df = dex_file_from_oat.get();
      if (df == nullptr) {
        LOG(ERROR) << "Unable to open dex file '" << dex_file_name << "': " << error_message;
        continue;
      }
      method_info = &empty_method_info;
    }

    OneDexFile df_info;
    df_info.offset_to_dex_file_name = name_buffer.size();
    const std::string& dex_name = df->GetLocation();
    VLOG(exact_profiler) << "Processing dex file: " << dex_file_name
                         << ", num methods = " << df->NumMethodIds()
                         << ", current_offset = 0x" << std::hex << current_offset;
    // Insert dex file name, NUL terminated.
    name_buffer.insert(name_buffer.end(), dex_name.begin(), dex_name.end());
    name_buffer.push_back('\0');
    df_info.dex_checksum = df->GetHeader().checksum_;
    df_info.num_methods = df->NumMethodIds();
    std::vector<uint32_t> method_offsets;
    method_offsets.reserve(df_info.num_methods);

    for (uint32_t i = 0; i < df_info.num_methods; i++) {
      auto method_it = method_info->find(i);
      // If we haven't seen the method, the count must be 1.
      uint32_t bb_count = 1;
      uint32_t invoke_count = 0;
      ProfileInformation* info = nullptr;
      if (method_it != method_info->end()) {
        // Found the information for this method.
        info = &method_it->second;
        bb_count = info->first;
        invoke_count = info->second.size();
      }
      prof_file.total_num_counters += bb_count;

      // Generate a 'OneMethod' for this method.
      one_method_data.push_back(bb_count);
      one_method_data.push_back(invoke_count);
      one_method_data.insert(one_method_data.end(),
                             bb_count * (sizeof(OneMethod::CountType) / sizeof(uint32_t)),
                             0);
      if (invoke_count > 0) {
        // Enter the invoke information for the method.
        DCHECK(info != nullptr);
        for (auto dex_pc : info->second) {
          one_method_data.push_back(dex_pc);
          constexpr uint32_t padding_count =
              OneCallSite::kNumInvokeTargets * (sizeof(OneInvoke) / sizeof(uint32_t));
          one_method_data.insert(one_method_data.end(), padding_count, 0);
        }
      }

      method_offsets.push_back(current_offset);
      current_offset += OneMethod::AllocationSize(bb_count, invoke_count);
    }
    // Finish off the method index table, which lies at the end of the methods.
    one_method_data.insert(one_method_data.end(), method_offsets.begin(), method_offsets.end());
    df_info.method_index_offsets = current_offset;
    current_offset += method_offsets.size() * sizeof(uint32_t);
    dex_file_data.push_back(df_info);

    prof_file.total_num_methods += df_info.num_methods;
  }

  VLOG(exact_profiler) << "Total number of counters = " << prof_file.total_num_counters;
  VLOG(exact_profiler) << "Total number of methods = " << prof_file.total_num_methods;

  // Update the offsets to the name strings by the intervening space.
  current_offset += sizeof(Magic2_);
  for (auto& it : dex_file_data) {
    it.offset_to_dex_file_name += current_offset;
  }

  // Pad the string table to a multiple of uint32_t.
  while ((name_buffer.size() % sizeof(uint32_t)) != 0) {
    name_buffer.push_back('\0');
  }

  // Compute the start of the variable portion of the file.
  uint32_t page_size = getpagesize();
  uint32_t after_magic3 = current_offset + name_buffer.size() + sizeof(Magic3_);
  if (after_magic3 % page_size != 0) {
    after_magic3 += page_size - (after_magic3 % page_size);
  }
  DCHECK_EQ(after_magic3 % page_size, 0U);
  prof_file.variable_start_offset = after_magic3;

  // Ensure that the directory is available.
  mkpath(prof_dir_name_.c_str(), 0777);
  // Ensure it is readable by everyone, even if umask forced above 0775 to change.
  if (chmod(prof_dir_name_.c_str(), 0777) < 0) {
    VLOG(exact_profiler) << "chmod " << prof_dir_name_ << " failed: " << strerror(errno);
  }

  // Write out the data.  The file needs to be read/writable by everyone on target.
  OpenLocked prof_fd(prof_file_name_.c_str());

  if (!prof_fd.IsOpen()) {
    LOG(ERROR) << "Unable to open profile file: " << prof_file_name_
               << ", error: " << strerror(errno);
    return false;
  }
  int fd = prof_fd.Get();
  VLOG(exact_profiler) << "file: " << prof_file_name_;
  // Needs to be read/writable by everyone on target.
  fchmod(fd, 0666);

  // Fixed header.
  if (!checked_write(fd, reinterpret_cast<const char *>(&prof_file),
                     sizeof(prof_file), prof_file_name_)) {
    return false;
  }

  // Offsets to dex files.
  if (!checked_write(fd, reinterpret_cast<const char *>(&offset_to_dex_files[0]),
                     offset_to_dex_files.size() * sizeof(uint32_t), prof_file_name_)) {
    return false;
  }

  // Dex file information.
  if (!checked_write(fd, reinterpret_cast<const char *>(&dex_file_data[0]),
                     dex_file_data.size() * sizeof(OneDexFile), prof_file_name_)) {
    return false;
  }

  // Per method information.
  if (!checked_write(fd, reinterpret_cast<const char *>(&one_method_data[0]),
                     one_method_data.size() * sizeof(uint32_t), prof_file_name_)) {
    return false;
  }


  // Magic2.
  uint32_t magic = Magic2_;
  if (!checked_write(fd, reinterpret_cast<const char *>(&magic),
                     sizeof(magic), prof_file_name_)) {
    return false;
  }

  // String table.
  if (!checked_write(fd, &name_buffer[0], name_buffer.size(), prof_file_name_)) {
    return false;
  }


  // Magic3.
  magic = Magic3_;
  if (!checked_write(fd, reinterpret_cast<const char *>(&magic),
                     sizeof(magic), prof_file_name_)) {
    return false;
  }

  // No more to write.
  close(fd);

  // All okay.
  return true;
}

std::string ExactProfiler::GetRelativeProfileFileName() const {
  // Find the '/' before the last component.
  size_t last_slash = prof_file_name_.rfind('/');
  DCHECK_NE(last_slash, std::string::npos);

  // Now see if there is a previous slash.
  size_t next_last_slash = prof_file_name_.rfind('/', last_slash-1);
  if (next_last_slash == std::string::npos) {
    // This is relative to the current directory.
    return prof_file_name_;
  }

  // Otherwise, we want the portion after next_last_slash.
  return prof_file_name_.substr(next_last_slash + 1);
}

static void ZeroCounts(ExactProfileFile* hdr) {
  char *const base = reinterpret_cast<char *>(hdr);
  uint32_t* dex_file_table = hdr->offset_to_dex_infos;
  for (uint32_t i = 0; i < hdr->num_dex_files; i++) {
    if (dex_file_table[i] == 0) {
      // Unallocated dex file.
      continue;
    }
    OneDexFile& df = *reinterpret_cast<OneDexFile*>(base + dex_file_table[i]);
    uint32_t* method_table = reinterpret_cast<uint32_t*>(base + df.method_index_offsets);
    for (uint32_t j = 0; j < df.num_methods; j++) {
      if (method_table[j] == 0U) {
        // Unallocated method.
        continue;
      }
      OneMethod* method_info = reinterpret_cast<OneMethod*>(base + method_table[j]);
      for (uint32_t k = 0; k < method_info->num_blocks; k++) {
        method_info->counts[k] = 0;
      }

      for (uint32_t k = 0; k < method_info->num_method_invokes; k++) {
        OneCallSite* call_site = method_info->CallSiteAt(k);
        memset(call_site->targets, '\0', sizeof(call_site->targets));
      }
    }
  }
}

int ExactProfiler::OpenProfileFile(const OatFile& oat_file,
                                   bool read_write,
                                   bool is_image,
                                   std::string& prof_name) {
  // Figure out the real path to the profiling file.
  std::string oat_file_name = oat_file.GetLocation();
  VLOG(exact_profiler) << "OpenProfileFile: OAT file name: " << oat_file_name;

  const char* exact_profiling = oat_file.GetOatHeader().GetStoreValueByKey("profile");
  if (!exact_profiling) {
    VLOG(exact_profiler) << "OpenProfileFile: " << oat_file.GetLocation() << ": no profile key";
    return -1;
  }

  // Figure out the real path to the profiling file.
  struct stat file_info;
  int ret;
  std::string orig_prof_name;
  if (exact_profiling[0] == '/') {
    // We have an absolute pathname to the profile file.
    prof_name = exact_profiling;
    VLOG(exact_profiler) << "OpenProfileFile: profile name(0): " << prof_name;
    // Get the file information.
    ret = stat(prof_name.c_str(), &file_info);
    orig_prof_name = prof_name;
  } else {
    // The profile file is relative to the OAT file.
    VLOG(exact_profiler) << "OpenProfileFile: OAT file name: " << oat_file_name;

    // We are in the dalvik cache.  Change the cache filename to get the original
    // file location.
    size_t last_slash = oat_file_name.rfind('/');
    if (last_slash == std::string::npos) {
      // No clue.
      VLOG(exact_profiler) << "OpenProfileFile: Unable to locate last slash";
      return -1;
    }

    // First, just try the name replacement from the exact_profiling string.
    // Use Oat filename (minus last component) + path to profiling.
    prof_name = oat_file_name.substr(0, last_slash + 1) + exact_profiling;
    orig_prof_name = prof_name;

    // Is it there and valid?
    ret = stat(prof_name.c_str(), &file_info);
    VLOG(exact_profiler) << "OpenProfileFile: profile name(1): " << prof_name;
    VLOG(exact_profiler) << "OpenProfileFile: size: " << file_info.st_size;
    if (ret < 0 || static_cast<size_t>(file_info.st_size) < sizeof(ExactProfileFile)) {
      // No, it wasn't there or was too small. See if it is an image file.
      if (ret < 0) {
        VLOG(exact_profiler) << "ret = " << ret << ", errno = " << errno
            << '(' << strerror(errno) << ')';
      } else {
        VLOG(exact_profiler) << "file size = " << file_info.st_size
            << ", header size = " << sizeof(ExactProfileFile);
      }

      prof_name = oat_file_name.substr(last_slash);

      // Replace the '@' with '/'.
      std::replace(prof_name.begin(), prof_name.end(), '@', '/');

      // We now have a new pathname.  We need to remove the last component, add
      // the instruction set, and the profile file name.
      last_slash = prof_name.rfind('/');
      if (last_slash == std::string::npos) {
        VLOG(exact_profiler) << "OpenProfileFile: Unable to locate last slash (2)";
        return -1;
      }
      prof_name = prof_name.substr(0, last_slash) + '/' +
          GetInstructionSetString(oat_file.GetOatHeader().GetInstructionSet()) + '/' +
          exact_profiling;
      VLOG(exact_profiler) << "OpenProfileFile: profile name(2): " << prof_name;

      // Retry looking for the file.
      ret = stat(prof_name.c_str(), &file_info);
    }
  }

  // Okay, we have a file name.  Now let us (re-)confirm that we have a valid file.
  if (ret < 0 || static_cast<size_t>(file_info.st_size) < sizeof(ExactProfileFile)) {
    if (GeneratingProfile(oat_file)) {
      // Only log the problem if we need the file to be present.
      LOG(INFO) << "Oat filename: " << oat_file_name << ", is_image " << is_image;
      if (ret < 0) {
        LOG(INFO) << "Unable to open file: " << strerror(errno);
      } else {
        LOG(INFO) << "File too small: " << static_cast<size_t>(file_info.st_size);
      }
      LOG(ERROR) << "Non-existent or too small profile file: " << orig_prof_name;
    }
    return -1;
  }

  int fd = ::open(prof_name.c_str(), read_write ? O_RDWR : O_RDONLY);

  // Lock the file appropriately.  The lock will be released on close.
  if (flock(fd, read_write ? LOCK_EX : LOCK_SH) < 0) {
    close(fd);
    LOG(FATAL) << "Unable to flock profile file for "
               << (read_write ? "updating: " : "reading: ")
               << prof_name << ": " << strerror(errno);
    UNREACHABLE();
  }

  return fd;
}

ExactProfileFile* ExactProfiler::GetProfileFile(const OatFile& oat_file,
                                                bool is_image,
                                                bool zero_counts) {
  std::string prof_name;
  int fd = OpenProfileFile(oat_file, false, is_image, prof_name);

  if (fd < 0) {
    // We failed to open the profile file.  Was this a serious failure?
    if (prof_name.empty()) {
      // No profile file exists.
      return nullptr;
    }
    if (!GeneratingProfile(oat_file)) {
      // We are not generating profiling information, so we don't need it.
      return nullptr;
    }

    LOG(FATAL) << "Unable to open profile file for reading: " << prof_name;
    UNREACHABLE();
  }

  // Read in the file, and confirm that we have a valid profile.
  struct stat file_info;
  if (flock(fd, LOCK_SH) < 0) {
    close(fd);
    LOG(FATAL) << "Unable to flock profile file for reading: " << prof_name
               << ": " << strerror(errno);
    UNREACHABLE();
  }
  if (::fstat(fd, &file_info) != 0) {
    close(fd);
    LOG(FATAL) << "Unable to get fstat: " << prof_name
               << ": " << strerror(errno);
    UNREACHABLE();
  }
  char* buffer = new char[file_info.st_size];
  if (buffer == nullptr) {
    LOG(FATAL) << "Unable to allocate memory for profile file: " << prof_name;
    UNREACHABLE();
  }

  // Remember that we have the buffer.
  std::unique_ptr<ExactProfileFile> prof;
  prof.reset(reinterpret_cast<ExactProfileFile*>(buffer));

  size_t bytes_read = read(fd, buffer, file_info.st_size);
  close(fd);
  if (bytes_read != static_cast<size_t>(file_info.st_size)) {
    LOG(FATAL) << "Problem reading profile file: " << prof_name;
    UNREACHABLE();
  }

  // Is the profile file valid?
  if (prof->magic1 != Magic1_) {
    LOG(FATAL) << "Invalid magic number for " << prof_name
        << ", found 0x" << std::hex << prof->magic1 << ", expected 0x" << Magic1_;
    UNREACHABLE();
  }

  if (prof->version != CurrentVersion_) {
    LOG(FATAL) << "Invalid version number for " << prof_name
        << ", found 0x" << std::hex << prof->version << ", expected 0x" << CurrentVersion_;
    UNREACHABLE();
  }

  // Checksum may differ if we have both x86 and x86_64 versions of the files compiled.
  if (prof->oat_checksum != oat_file.GetOatHeader().GetChecksum()) {
    VLOG(exact_profiler) << "Checksum mismatch for " << prof_name << " and "
                         << oat_file.GetLocation() << ", found 0x" << std::hex
                         << prof->oat_checksum << ", expected 0x"
                         << oat_file.GetOatHeader().GetChecksum();
  }

  // Ensure that all the counters are zeroed, or we may incorrectly double existing values.
  if (zero_counts) {
    ZeroCounts(prof.get());
  }

  VLOG(exact_profiler) << "Total number of counters = " << prof->total_num_counters;
  VLOG(exact_profiler) << "Total number of methods = " << prof->total_num_methods;
  // Save the profile filename for updating later.
  char *name = new char[prof_name.size()+1];
  strcpy(name, prof_name.c_str());
  // Nasty trick, to store a (possible 64 bit) pointer in runtime_temp without
  // declaring them as void *, in order to keep all fields aligned properly.
  char **runtime_temp_as_ptr = reinterpret_cast<char **>(&prof->runtime_temp);
  *runtime_temp_as_ptr = name;

  // Are we generating profile information for this OAT file?
  prof->generating_profile = GeneratingProfile(oat_file);

  // Good enough for now.
  return prof.release();
}

std::ostream& operator<<(std::ostream& os, const PersistentClassIndex& index) {
  os << '<' << static_cast<int>(index.OatIndex()) << ':'
     << static_cast<int>(index.DexIndex()) << ':'
     << index.ClassDefIndex() << '>';
  return os;
}

class OatDexTableManager {
  public:
    /*
     * @brief Load OAT and Dex tables from a file image.
     * @param hdr ExactProfile image start.
     */
    OatDexTableManager(ExactProfileFile* hdr);

    /*
     * @brief Add a reference to a class to the OAT/Dex table.
     * @param klass Class type to add.
     * @returns an index into the oat/dex tables representing the type.
     */
    PersistentClassIndex AddClassType(mirror::Class* klass)
        SHARED_REQUIRES(Locks::mutator_lock_);

    /*
     * @brief Add a string to the OAT/Dex string table.
     * @param string String to be added.
     * @returns an offset into the string table.
     */
    uint32_t AddStringLocation(const std::string& string);

    /*
     * @brief Return a string in the string table
     * @param offset Offset into string table.
     * @returns the string at that offset.
     */
    std::string GetStringLocation(uint32_t offset) const {
      return &string_table_[offset];
    }

    /*
     * Is the OatDexTable empty?
     * @returns 'true' if there is nothing in the OAT/Dex table.
     */
    bool IsEmpty() const {
      return oat_map_.empty();
    }

    /*
     * Has the OatDexTable been updated since initialization?
     * @returns 'true' if something has been added to the OatDexTable.
     */
    bool Updated() const {
      return updated_;
    }

    /*
     * Return the vector of OneOatIndex.
     * @returns vector of OneOatIndex.
     */
    const std::vector<OneOatIndex>& GetOatIndicies() const {
      return oats_;
    }

    /*
     * Return the vector of OneDexIndex.
     * @returns vector of OneDexIndex.
     */
    const std::vector<OneDexIndex>& GetDexIndicies() const {
      return dexs_;
    }

    /*
     * Return the string table.
     * @returns the string table.
     */
    const std::vector<char>& GetStringTable() const {
      return string_table_;
    }

    /*
     * Return the OAT file name for a given index.
     * @param index OAT file index.
     * @returns the matching OAT name or an empty string.
     */
    const std::string GetOatFileName(uint32_t index) const {
      if (index < oats_.size()) {
        return GetStringLocation(oats_[index].offset_of_oat_location_string);
      }
      return "";
    }

    /*
     * Return the OAT checksum for a given index.
     * @param index OAT file index.
     * @returns the matching OAT checksum or -1.
     */
    uint32_t GetOatChecksum(uint32_t index) const {
      if (index < oats_.size()) {
        return oats_[index].oat_checksum;
      }
      return -1;
    }

    /*
     * Return the Dex file name for a given index.
     * @param index Dex file index.
     * @returns the matching Dex name or an empty string.
     */
    const std::string GetDexFileName(uint32_t index) const {
      if (index < dexs_.size()) {
        return GetStringLocation(dexs_[index].offset_of_dex_location_string);
      }
      return "";
    }

    /*
     * Return the Dex checksum for a given index.
     * @param index Dex file index.
     * @returns the matching Dex checksum or -1 for an invalid index.
     */
    uint32_t GetDexChecksum(uint32_t index) const {
      if (index < dexs_.size()) {
        return dexs_[index].dex_checksum;
      }
      return 0xFFFFFFFFU;
    }

    /*
     * Return the DexFile location checksum for a given index.
     * @param index Dex file index.
     * @returns the matching DexFile location checksum or -1 for an invalid index.
     */
    uint32_t GetDexLocChecksum(uint32_t index) const {
      if (index < dexs_.size()) {
        return dexs_[index].dex_loc_checksum;
      }
      return 0xFFFFFFFFU;
    }

  private:
    std::map<mirror::Class*, PersistentClassIndex> class_map_;
    std::map<const OatFile*, uint32_t> oat_map_;
    std::map<std::pair<const DexFile*, uint32_t>, uint32_t> dex_map_;
    std::vector<OneOatIndex> oats_;
    std::vector<OneDexIndex> dexs_;
    std::vector<char> string_table_;
    std::map<std::string, uint32_t> string_offset_map_;
    bool updated_;
    uint32_t FindOatFile(const OatFile* oat_file)
        SHARED_REQUIRES(Locks::mutator_lock_);
    uint32_t FindDexFile(const DexFile& dex_file, uint32_t index_in_oat)
        SHARED_REQUIRES(Locks::mutator_lock_);
};

uint32_t OatDexTableManager::AddStringLocation(const std::string& string) {
  // Already present?
  auto it = string_offset_map_.find(string);
  if (it != string_offset_map_.end()) {
    return it->second;
  }

  // Nope.  Need to add it in.
  uint32_t index = string_table_.size();
  const char* c_str = string.c_str();
  // Ensure that the trailing NUL is included.
  string_table_.insert(string_table_.end(), c_str, c_str + string.length() + 1);
  string_offset_map_.insert(std::pair<std::string, uint32_t>(string, index));
  return index;
}

OatDexTableManager::OatDexTableManager(ExactProfileFile* hdr)
    : updated_(false) {
  // Is there anything to read?
  if (hdr->offset_to_oat_table == 0 ||
      hdr->offset_to_dex_table == 0 ||
      hdr->offset_to_string_table == 0) {
    return;
  }

  // We have some data already in the Profile file.  Read it in and populate the
  // tables.
  char* base = reinterpret_cast<char*>(hdr);

  // Copy the OneOatIndex table.
  OatTable* oat_table = reinterpret_cast<OatTable*>(base + hdr->offset_to_oat_table);
  oats_.reserve(oat_table->num_oat_locations * 2);
  oats_.insert(oats_.end(),
               &oat_table->oat_locations[0],
               &oat_table->oat_locations[oat_table->num_oat_locations]);

  // Copy the OneDexIndex table.
  DexTable* dex_table = reinterpret_cast<DexTable*>(base + hdr->offset_to_dex_table);
  dexs_.reserve(dex_table->num_dex_locations * 2);
  dexs_.insert(dexs_.end(),
               &dex_table->dex_locations[0],
               &dex_table->dex_locations[dex_table->num_dex_locations]);

  // Copy the string table.
  LocationStringTable* location_table =
      reinterpret_cast<LocationStringTable*>(base + hdr->offset_to_string_table);
  string_table_.reserve(location_table->num_string_chars * 2);
  string_table_.insert(string_table_.end(),
                       &location_table->strings[0],
                       &location_table->strings[location_table->num_string_chars]);
  // Recreate the string map.
  char *string_start = &location_table->strings[0];
  char *p = string_start;
  while (static_cast<uint32_t>((p - string_start)) < location_table->num_string_chars) {
    uint32_t len = strlen(p);
    string_offset_map_.insert(std::pair<std::string, uint32_t>(p, p-string_start));
    p += len + 1;
  }
}

uint32_t OatDexTableManager::FindOatFile(const OatFile* oat_file) {
  auto it2 = oat_map_.find(oat_file);
  if (it2 != oat_map_.end()) {
    return it2->second;
  }

  // Need to add it to the list.  Is it pre-loaded?
  uint32_t oat_index = 0;
  const std::string& oat_location = oat_file->GetLocation();
  uint32_t oat_check_sum = oat_file->GetOatHeader().GetChecksum();
  for (uint32_t e = oats_.size(); oat_index < e; oat_index++) {
    const OneOatIndex& ooi = oats_[oat_index];
    if (ooi.oat_checksum == oat_check_sum) {
      // Ensure we find it quickly next time.
      oat_map_.insert(std::pair<const OatFile*, uint32_t>(oat_file, oat_index));
      return oat_index;
    }
  }

  // Need to add a new OAT file to the OAT table.
  OneOatIndex new_oat_index;
  new_oat_index.oat_checksum = oat_check_sum;
  new_oat_index.offset_of_oat_location_string = AddStringLocation(oat_location);
  oat_index = oats_.size();
  oats_.push_back(new_oat_index);
  oat_map_.insert(std::pair<const OatFile*, uint32_t>(oat_file, oat_index));
  updated_ = true;
  return oat_index;
}

uint32_t OatDexTableManager::FindDexFile(const DexFile& dex_file,
                                         uint32_t index_in_oat) {
  std::pair<const DexFile*, uint32_t> dex_index_pair(&dex_file, index_in_oat);
  auto it2 = dex_map_.find(dex_index_pair);
  if (it2 != dex_map_.end()) {
    return it2->second;
  }

  // Need to add it to the list.  Is it pre-loaded?
  uint32_t dex_index = 0;
  const std::string& dex_location = dex_file.GetLocation();
  uint32_t dex_check_sum = dex_file.GetHeader().checksum_;
  uint32_t dex_loc_check_sum = dex_file.GetLocationChecksum();
  for (uint32_t e = dexs_.size(); dex_index < e; dex_index++) {
    const OneDexIndex& odi = dexs_[dex_index];
    if (odi.dex_checksum == dex_check_sum && odi.dex_index_in_oat == index_in_oat) {
      DCHECK_EQ(odi.dex_loc_checksum, dex_loc_check_sum);
      // Ensure we find it quickly next time.
      dex_map_.insert(std::make_pair(dex_index_pair, dex_index));
      return dex_index;
    }
  }

  // Need to add a new OAT file to the OAT table.
  OneDexIndex new_dex_index;
  new_dex_index.dex_index_in_oat = index_in_oat;
  new_dex_index.dex_checksum = dex_check_sum;
  new_dex_index.dex_loc_checksum = dex_loc_check_sum;
  new_dex_index.offset_of_dex_location_string = AddStringLocation(dex_location);
  dex_index = dexs_.size();
  dexs_.push_back(new_dex_index);
  dex_map_.insert(std::make_pair(dex_index_pair, dex_index));
  updated_ = true;
  return dex_index;
}

PersistentClassIndex OatDexTableManager::AddClassType(mirror::Class* klass) {
  // Have we seen this class already?
  CHECK(klass != nullptr);
  auto it = class_map_.find(klass);
  if (it != class_map_.end()) {
    return it->second;
  }

  uint16_t class_def_index = klass->GetDexClassDefIndex();
  const DexFile& dex_file = klass->GetDexFile();
  const OatDexFile* oat_dex_file = dex_file.GetOatDexFile();
  CHECK(oat_dex_file != nullptr) << PrettyDescriptor(klass);
  const OatFile* oat_file = oat_dex_file->GetOatFile();
  CHECK(oat_file != nullptr) << PrettyDescriptor(klass);

  // Find a match for the OAT file.
  uint32_t oat_index = FindOatFile(oat_file);
  CHECK_LT(oat_index, oats_.size()) << PrettyDescriptor(klass);

  // Find the dex file index in the OAT file.
  const std::vector<const OatDexFile*>& oat_dex_files =
      oat_dex_file->GetOatFile()->GetOatDexFiles();
  auto index_in_oat = std::find(oat_dex_files.begin(), oat_dex_files.end(), oat_dex_file);
  CHECK(index_in_oat != oat_dex_files.end()) << PrettyDescriptor(klass);
  uint32_t dex_index = FindDexFile(dex_file, index_in_oat - oat_dex_files.begin());

  // Remember this for next time this class is used.
  PersistentClassIndex index(oat_index, dex_index, class_def_index);
  class_map_.insert(std::pair<mirror::Class*, PersistentClassIndex>(klass, index));

  return index;
}

static void MergeInvokes(OatDexTableManager& oat_dex_table,
                         OneMethod* method_info,
                         OneMethod* file_method_info,
                         bool zero_counters)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  for (uint32_t i = 0; i < method_info->num_method_invokes; i++) {
    OneCallSite* call_site = method_info->CallSiteAt(i);
    OneCallSite* file_call_site = file_method_info->CallSiteAt(i);
    for (int j = 0; j < OneCallSite::kNumInvokeTargets; j++) {
      mirror::Class* klass = call_site->targets[j].klass.Read();
      if (klass == nullptr) {
        // There are no more slots to check.
        break;
      }

      // Handle(Ignore) known problems.
      if (klass->GetDexCache() == nullptr) {
        // TODO: Handle this case.
        // This is a builtin class type, such as byte[] or java.lang.Object[],
        // And there is no dex file associated with it.  Ignore for now
        VLOG(exact_profiler) << "Missing dex cache for class: " << PrettyDescriptor(klass);
        continue;
      }
      if (klass->GetDexFile().GetOatDexFile() == nullptr) {
        // TODO: Figure this out!
        // There is no OAT file associated with the Dex file. Ignore for now
        VLOG(exact_profiler) << "Missing OAT dex file for class: " << PrettyDescriptor(klass);
        continue;
      }

      PersistentClassIndex class_index = oat_dex_table.AddClassType(klass);
      // Is this class already known?
      bool seen_class = false;
      int k;
      uint32_t new_count = call_site->targets[j].count;
      int smallest_index = 0;
      uint32_t smallest_count = file_call_site->targets[0].count;
      for (k = 0; k < OneCallSite::kNumInvokeTargets; k++) {
        uint32_t old_count = file_call_site->targets[k].count;
        PersistentClassIndex old_class_index = file_call_site->targets[k].class_index;
        if (old_class_index.IsNull()) {
          // Slot is empty.  No more types can be present.
          break;
        }

        if (class_index == old_class_index) {
          seen_class = true;
          if (std::numeric_limits<OneInvoke::CountType>::max() - old_count > new_count) {
            file_call_site->targets[k].count += new_count;
          } else {
            file_call_site->targets[k].count = std::numeric_limits<OneInvoke::CountType>::max();
          }
          break;
        }

        // Remember the smallest index, in case we need it later.
        if (old_count < smallest_count) {
          smallest_index = k;
          smallest_count = old_count;
        }
      }
      if (!seen_class) {
        // This is a new class type.  Should we add it to the list?
        if (k == OneCallSite::kNumInvokeTargets) {
          DCHECK(!file_call_site->targets[k - 1].class_index.IsNull());
          DCHECK(!file_call_site->targets[smallest_index].class_index.IsNull());
          // There are no more free slots in the invoke type list.  Replace the
          // smallest index if we are larger than it.
          if (new_count > smallest_count) {
            file_call_site->targets[smallest_index].class_index = class_index;
            file_call_site->targets[smallest_index].count = new_count;
          }
        } else {
          // There is still space for a new class type.
          DCHECK(file_call_site->targets[k].class_index.IsNull());
          file_call_site->targets[k].class_index = class_index;
          file_call_site->targets[k].count = new_count;
        }
      }
      if (zero_counters) {
        call_site->targets[j].count = 0;
      }
    }
  }
}

static void UpdateOneProfileFile(ExactProfileFile* hdr,
                                 ExactProfileFile* file_hdr,
                                 OatDexTableManager& oat_table,
                                 char *profile_file_name,
                                 bool zero_counters)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  if (hdr->version != file_hdr->version) {
    LOG(WARNING) << "Version mismatch on profile file '" << profile_file_name << '\'';
    return;
  }
  // Don't check the OAT checksum, as a mismatch is possible for x86/x86_64.
  if (hdr->num_dex_files != file_hdr->num_dex_files) {
    LOG(WARNING) << "Number of dex files mismatch on profile file '" << profile_file_name << '\'';
    return;
  }
  if (hdr->total_num_counters != file_hdr->total_num_counters) {
    LOG(WARNING) << "Number of counters mismatch on profile file '" << profile_file_name << '\'';
    return;
  }
  if (hdr->total_num_methods != file_hdr->total_num_methods) {
    LOG(WARNING) << "Number of methods mismatch on profile file '" << profile_file_name << '\'';
    return;
  }
  if (hdr->num_dex_files != file_hdr->num_dex_files) {
    LOG(WARNING) << "Number of dex files mismatch on profile file '" << profile_file_name << '\'';
    return;
  }

  // Handle each Dex file.
  char *base = reinterpret_cast<char*>(hdr);
  char *file_base = reinterpret_cast<char*>(file_hdr);
  uint32_t* dex_file_table = hdr->offset_to_dex_infos;
  uint32_t* file_dex_file_table = file_hdr->offset_to_dex_infos;

  VLOG(exact_profiler) << "Total number of counters = " << hdr->total_num_counters;
  VLOG(exact_profiler) << "Total number of methods = " << hdr->total_num_methods;

  for (uint32_t i = 0; i < hdr->num_dex_files; i++) {
    if (dex_file_table[i] == 0) {
      // Unallocated dex file.
      continue;
    }
    OneDexFile& df = *reinterpret_cast<OneDexFile*>(base + dex_file_table[i]);
    DCHECK_NE(file_dex_file_table[i], 0U);
    OneDexFile& file_df = *reinterpret_cast<OneDexFile*>(file_base + file_dex_file_table[i]);
    uint32_t* method_table = reinterpret_cast<uint32_t*>(base + df.method_index_offsets);
    uint32_t* file_method_table = reinterpret_cast<uint32_t*>(file_base + df.method_index_offsets);
    if (df.num_methods != file_df.num_methods) {
      LOG(WARNING) << "Number of methods mismatch on profile file '" << profile_file_name << ", dex file index: " << i;
      return;
    }

    for (uint32_t j = 0; j < df.num_methods; j++) {
      if (method_table[j] == 0U) {
        // Unallocated method.
        continue;
      }
      OneMethod* method_info = reinterpret_cast<OneMethod*>(base + method_table[j]);
      // Don't know how to handle this now.
      DCHECK_NE(file_method_table[j], 0U);
      OneMethod* file_method_info =
          reinterpret_cast<OneMethod*>(file_base + file_method_table[j]);
      DCHECK_EQ(method_info->num_blocks, file_method_info->num_blocks);
      for (uint32_t k = 0; k < method_info->num_blocks; k++) {
        OneMethod::CountType count = method_info->counts[k];
        if (count != 0) {
          // We have a non-zero count.  We have to update the file copy.
          // Ensure that we don't overflow.
          if (file_method_info->counts[k] > (ExactProfiler::MaxCount - count)) {
            file_method_info->counts[k] = ExactProfiler::MaxCount;
          } else {
            file_method_info->counts[k] += count;
            DCHECK_NE(file_method_info->counts[k], ExactProfiler::MaxCount);
          }
          if (zero_counters) {
            method_info->counts[k] = 0;
          }
        }
      }

      // Check on the invoke types.
      MergeInvokes(oat_table, method_info, file_method_info, zero_counters);
    }
  }
}

void ExactProfiler::UpdateProfileFiles(OatToExactProfileFileMap& profiles,
                                       bool zero_counters)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  VLOG(exact_profiler) << "ExactProfiler::UpdateProfileFiles()";
  for (auto it : profiles) {
    ExactProfileFile* hdr = it.second;

    // Grab the saved filename from the runtime_temp field.
    char **profile_file_name = reinterpret_cast<char **>(&hdr->runtime_temp);
    VLOG(exact_profiler) << "Updating profile file: " << *profile_file_name;

    OpenLocked prof_fd(*profile_file_name);

    if (!prof_fd.IsOpen()) {
      continue;
    }

    // We have exclusive access to the file.  Memory map it in, so that we can update it.
    int fd = prof_fd.Get();
    struct stat prof_stat;
    if (fstat(fd, &prof_stat) == 0) {
      VLOG(exact_profiler) << "profile file: " << *profile_file_name << " has size "
                           << prof_stat.st_size;
      void *memmap_addr = mmap(nullptr, prof_stat.st_size, PROT_READ|PROT_WRITE,
                               MAP_SHARED, fd, 0);
      if (memmap_addr == reinterpret_cast<void*>(-1)) {
        LOG(WARNING) << "mmap of profile file '" << *profile_file_name << "' failed";
      } else {
        // Update the information in the file.
        ExactProfileFile *file_hdr = reinterpret_cast<ExactProfileFile*>(memmap_addr);
        OatDexTableManager oat_dex_table(file_hdr);
        UpdateOneProfileFile(hdr, file_hdr, oat_dex_table, *profile_file_name, zero_counters);

        // Update the OAT/Dex location information, if needed.
        if (!oat_dex_table.IsEmpty() && oat_dex_table.Updated()) {
          // For now, just re-write the tables each time.
          // Ensure that we don't mess up due to mapped pages, as we may be
          // overwriting part of the file which was mmap'ed.
          DCHECK_NE(file_hdr->variable_start_offset, 0U);
          DCHECK_EQ(file_hdr->variable_start_offset % getpagesize(), 0U);

          // Update the offsets to our areas.
          const std::vector<OneOatIndex>& oats = oat_dex_table.GetOatIndicies();
          size_t oat_tab_size = sizeof(OneOatIndex) * oats.size();
          file_hdr->offset_to_oat_table = file_hdr->variable_start_offset;

          const std::vector<OneDexIndex>& dexs = oat_dex_table.GetDexIndicies();
          size_t dex_tab_size = sizeof(OneDexIndex) * dexs.size();
          file_hdr->offset_to_dex_table =
              file_hdr->offset_to_oat_table + sizeof(OatTable) + oat_tab_size;

          file_hdr->offset_to_string_table =
              file_hdr->offset_to_dex_table + sizeof(DexTable) + dex_tab_size;

          // We no longer need the memory mapped image of the file. This will write
          // any changes to disk.
          ::lseek(fd, file_hdr->variable_start_offset, SEEK_SET);
          munmap(memmap_addr, prof_stat.st_size);

          // First write the OAT table.
          OatTable oat_tab;
          oat_tab.num_oat_locations = oats.size();
          oat_tab.num_allocated_oat_locations = oat_tab.num_oat_locations;
          (void) ::write(fd, &oat_tab, sizeof(oat_tab));
          (void) ::write(fd, &oats[0], oat_tab_size);

          // Then write the Dex table.
          DexTable dex_tab;
          dex_tab.num_dex_locations = dexs.size();
          dex_tab.num_allocated_dex_locations = dex_tab.num_dex_locations;

          (void) ::write(fd, &dex_tab, sizeof(dex_tab));
          (void) ::write(fd, &dexs[0], dex_tab_size);

          // Finally write the oat/dex string table.
          const std::vector<char>& strings = oat_dex_table.GetStringTable();
          LocationStringTable string_tab;
          string_tab.num_string_chars = strings.size();
          string_tab.num_allocated_string_chars = string_tab.num_string_chars;
          (void) ::write(fd, &string_tab, sizeof(string_tab));
          (void) ::write(fd, &strings[0], string_tab.num_string_chars);
        } else {
          // We no longer need the memory mapped image of the file.
          munmap(memmap_addr, prof_stat.st_size);
        }
      }
    } else {
      LOG(WARNING) << "mmap of profile file '" << *profile_file_name << "' failed";
    }
  }
}

void ExactProfiler::AllocateProfileCounters(ArtMethod* method,
                                          ExactProfileFile* hdr,
                                          const DexFile* dex_file,
                                          uint32_t dex_file_index,
                                          uint32_t method_index)
    SHARED_REQUIRES(Locks::mutator_lock_) REQUIRES(Locks::profiler_lock_) {
  // Allocate memory for one method.
  DCHECK_LT(static_cast<uint32_t>(dex_file_index), hdr->num_dex_files);
  uint32_t* dex_file_table = hdr->offset_to_dex_infos;
  CHECK_NE(dex_file_table[dex_file_index], 0U);
  OneDexFile& dex_info =
    *reinterpret_cast<OneDexFile*>(reinterpret_cast<char*>(hdr) + dex_file_table[dex_file_index]);
  DCHECK_LT(method_index, dex_info.num_methods);
  uint32_t method_idx = method->GetDexMethodIndex();
  MethodReference method_ref(dex_file, method_idx);

  char* base = reinterpret_cast<char *>(hdr);

  // Locate the method index table.
  uint32_t* index_table = reinterpret_cast<uint32_t*>(base + dex_info.method_index_offsets);
  OneMethod* method_info =
      reinterpret_cast<OneMethod*>(base + index_table[method_index]);
  Runtime::ProfileBuffersMap& prof_counters = Runtime::Current()->GetProfileBuffers();
  auto it = prof_counters.find(method_ref);
  if (it == prof_counters.end()) {
    prof_counters.Put(method_ref, method_info);
  } else if (it->second->num_blocks < method_info->num_blocks) {
    // We have already seen this method. Switch to using this one, which is bigger.
    prof_counters.Overwrite(method_ref, method_info);
  }
}

void ExactProfiler::AllocateProfileCounters(ExactProfileFile* hdr,
                                          int32_t dex_file_index,
                                          const std::string& dex_file_location,
                                          const DexFile* dex_file,
                                          ArtMethod** art_methods,
                                          size_t num_methods)
    SHARED_REQUIRES(Locks::mutator_lock_) REQUIRES(Locks::profiler_lock_) {
  // Some sanity checks.
  if (hdr->magic1 != Magic1_) {
    LOG(FATAL) << "Invalid magic number: found 0x" << std::hex << hdr->magic1
               << ", expected 0x" << Magic1_ << " while looking for index "
               << dex_file_index << " file: " << dex_file_location;
    UNREACHABLE();
  }

  if (hdr->version != CurrentVersion_) {
    LOG(FATAL) << "Invalid version number: found 0x" << std::hex << hdr->version
               << ", expected 0x" << CurrentVersion_ << " while looking for index "
               << dex_file_index << " file: " << dex_file_location;
    UNREACHABLE();
  }

  CHECK(art_methods != nullptr);
  DCHECK_LT(static_cast<uint32_t>(dex_file_index), hdr->num_dex_files);
  uint32_t* dex_file_table = hdr->offset_to_dex_infos;
  DCHECK_NE(dex_file_table[dex_file_index], 0U);
  char* base = reinterpret_cast<char*>(hdr);
  OneDexFile& dex_info = *reinterpret_cast<OneDexFile*>(base + dex_file_table[dex_file_index]);
  uint32_t* method_table = reinterpret_cast<uint32_t*>(base + dex_info.method_index_offsets);
  char* dex_file_name = base + dex_info.offset_to_dex_file_name;

  // Some more checks for correctness.
  if (dex_file_location != dex_file_name) {
    LOG(FATAL) << "Expected dex file " << dex_file_index << " at location " << dex_file_name
               << ", found " << dex_file_location;
    UNREACHABLE();
  }
  if (dex_info.dex_checksum != dex_file->GetHeader().checksum_) {
    LOG(FATAL) << "Expected dex file " << dex_file_index << " checksum 0x" << std::hex
               << dex_info.dex_checksum << ", found 0x"
               << dex_file->GetHeader().checksum_;
    UNREACHABLE();
  }

  // Populate the profile_counters_ map.
  Runtime::ProfileBuffersMap& prof_counters = Runtime::Current()->GetProfileBuffers();
  DCHECK_EQ(num_methods, static_cast<size_t>(dex_info.num_methods));
  size_t ptr_size = Runtime::Current()->GetClassLinker()->GetImagePointerSize();
  for (size_t j = 0; j < num_methods; ++j) {
    if (method_table[j] == 0U) {
      // Unallocated method.
      continue;
    }
    OneMethod* method_info = reinterpret_cast<OneMethod*>(base + method_table[j]);
    ArtMethod* method = mirror::DexCache::GetElementPtrSize<ArtMethod*>(art_methods, j, ptr_size);
    uint32_t method_idx = method->GetDexMethodIndex();
    if (method_idx >= dex_file->NumMethodIds()) {
      // Invalid method index.
      continue;
    }

    MethodReference method_ref(dex_file, method_idx);

    // Use the counters in the ExactProfile for this Method.
    auto it = prof_counters.find(method_ref);

    if (it == prof_counters.end()) {
      prof_counters.Put(method_ref, method_info);
    } else if (it->second->num_blocks < method_info->num_blocks) {
      // We have already seen this method. Switch to using this one, which is bigger.
      prof_counters.Overwrite(method_ref, method_info);
    }
  }
}

class ExactProfileSaver {
 public:
  /*
   * @brief Start background saver task.
   */
  static void Start();

  /*
   * @brief Stop background saver task.
   */
  static void Stop();

  /*
   * @brief Is the background saver task running?
   */
  bool IsStarted();

 private:
  ExactProfileSaver()
    : shutting_down_(false),
      wait_lock_("Exact profiler Saver lock"),
      period_condition_("Exact profiler Saver period condition", wait_lock_) {}

  void Run();
  bool ShuttingDown(Thread* self);
  static void* RunProfileSaverThread(void* arg);

  static ExactProfileSaver* instance_ GUARDED_BY(Locks::profiler_lock_);
  static pthread_t updater_thread_ GUARDED_BY(Locks::profiler_lock_);
  bool shutting_down_ GUARDED_BY(Locks::profiler_lock_);
  Mutex wait_lock_ DEFAULT_MUTEX_ACQUIRED_AFTER;
  ConditionVariable period_condition_ GUARDED_BY(wait_lock_);

  DISALLOW_COPY_AND_ASSIGN(ExactProfileSaver);
};

ExactProfileSaver* ExactProfileSaver::instance_;
pthread_t ExactProfileSaver::updater_thread_;

void* ExactProfileSaver::RunProfileSaverThread(void* arg) {
  Runtime* runtime = Runtime::Current();
  ExactProfileSaver* saver = reinterpret_cast<ExactProfileSaver*>(arg);

  // We have to be careful about attaching while shutting down.
  {
    MutexLock shutdown_mu(nullptr, *Locks::runtime_shutdown_lock_);
    if (Runtime::Current()->IsShuttingDownLocked()) {
      VLOG(exact_profiler) << "Shutdown coming: Exact Profile Background saver not started";
      return nullptr;
    }
    runtime->StartThreadBirth();
  }

  CHECK(runtime->AttachCurrentThread("Exact Profile Background Saver",
                                     /*as_daemon*/true,
                                     runtime->GetSystemThreadGroup(),
                                     /*create_peer*/true));
  {
    MutexLock shutdown_mu(nullptr, *Locks::runtime_shutdown_lock_);
    runtime->EndThreadBirth();
  }

  VLOG(exact_profiler) << "Exact Profile Background saver start";
  saver->Run();

  runtime->DetachCurrentThread();
  VLOG(exact_profiler) << "Exact Profile Background saver shutdown";
  return nullptr;
}

void ExactProfileSaver::Start() {
  MutexLock mu(Thread::Current(), *Locks::profiler_lock_);

  // Don't start two saver threads.
  if (instance_ != nullptr) {
    DCHECK(false) << "Tried to start two exact profile savers";
    return;
  }

  // Ensure that we aren't starting up as we are shutting down.
  {
    MutexLock shutdown_mu(nullptr, *Locks::runtime_shutdown_lock_);
    VLOG(exact_profiler) << "Starting exact profile background saver";
    instance_ = new ExactProfileSaver();

    if (!Runtime::Current()->IsShuttingDownLocked()) {
      // Create a new thread which does the saving.
      CHECK_PTHREAD_CALL(
          pthread_create,
          (&updater_thread_, nullptr, &RunProfileSaverThread, reinterpret_cast<void*>(instance_)),
          "Exact Profiler Background Saver thread");
    }
  }
}

void ExactProfileSaver::Stop() NO_THREAD_SAFETY_ANALYSIS {
  ExactProfileSaver* saver = nullptr;
  pthread_t updater_thread = 0U;

  {
    MutexLock profiler_mutex(Thread::Current(), *Locks::profiler_lock_);
    VLOG(exact_profiler) << "Stopping exact profile background saver thread";
    saver = instance_;
    updater_thread = updater_thread_;
    if (instance_ == nullptr) {
      return;
    }
    if (instance_->shutting_down_) {
      // Already shutting down.
      DCHECK(false) << "Tried to stop the exact profile saver twice";
      return;
    }
    instance_->shutting_down_ = true;
  }

  {
    // Wake up the saver thread if it is sleeping to allow for a clean exit.
    MutexLock wait_mutex(Thread::Current(), saver->wait_lock_);
    saver->period_condition_.Signal(Thread::Current());
  }

  // Wait for the saver thread to stop.
  CHECK_PTHREAD_CALL(pthread_join, (updater_thread, nullptr), "exact profile saver thread shutdown");

  {
    MutexLock profiler_mutex(Thread::Current(), *Locks::profiler_lock_);
    instance_ = nullptr;
    updater_thread_ = 0U;
  }
  delete saver;
}

bool ExactProfileSaver::ShuttingDown(Thread* self) {
  MutexLock mu(self, *Locks::profiler_lock_);
  return shutting_down_;
}

bool ExactProfileSaver::IsStarted() {
  MutexLock mu(Thread::Current(), *Locks::profiler_lock_);
  return instance_ != nullptr;
}

void ExactProfileSaver::Run() REQUIRES(!wait_lock_) {
  // Add a random delay each time we go to sleep so that we don't hammer the CPU
  // with all profile savers running at the same time.
  const uint64_t kRandomDelayMaxMs = 4 * 1000;  // 4 second;
  const uint64_t kSavePeriodMs = 20 * 1000;  // 20 seconds;

  srand(MicroTime() * getpid());
  Thread* self = Thread::Current();

  uint64_t save_period_ms = kSavePeriodMs;
  VLOG(exact_profiler) << "Save profiling information every " << save_period_ms << " ms";
  while (true) {
    if (ShuttingDown(self)) {
      break;
    }

    uint64_t random_sleep_delay_ms = rand() % kRandomDelayMaxMs;
    uint64_t sleep_time_ms = save_period_ms + random_sleep_delay_ms;
    {
      MutexLock mu(self, wait_lock_);
      period_condition_.TimedWait(self, sleep_time_ms, 0);
    }

    VLOG(exact_profiler) << "BG profile saver awake";
    if (ShuttingDown(self)) {
      break;
    }

    ReaderMutexLock mu(self, *Locks::mutator_lock_);
    ExactProfiler::OatToExactProfileFileMap& profiles = Runtime::Current()->GetProfilers();
    if (!profiles.empty()) {
      ExactProfiler::UpdateProfileFiles(profiles);
    }
  }
}

void ExactProfiler::StartProfileSaver() {
  VLOG(exact_profiler) << "ExactProfileSaver::Start";
  ExactProfileSaver::Start();
}

void ExactProfiler::StopProfileSaver() {
  VLOG(exact_profiler) << "ExactProfileSaver::Stop";
  ExactProfileSaver::Stop();
}

ExactProfileFile* ExactProfiler::AddProfileFile(const std::string& oat_location,
                                                bool is_image,
                                                bool is_boot_image_option,
                                                InstructionSet isa) {
  // Open an existing profile, based on the arguments.
  VLOG(exact_profiler) << "AddProfileFile: " << oat_location
                       << ", prof dir name: " << prof_dir_name_
                       << ", prof file name: " << prof_file_name_;
  std::string oat_file_name = oat_location;
  std::string prof_name;

  // There are 3 cases here:
  if (is_boot_image_option) {
    DCHECK_NE(isa, kNone);
    // 1. oat_location is similar to:
    //    -Ximage:/media/Dev/oam/out/host/linux-x86/framework/core.art
    //    -Ximage:/system/framework/boot.art
    oat_file_name = oat_file_name.substr(1);

    // Remove leading '/' if present.
    if (oat_file_name[0] == '/') {
      oat_file_name = oat_file_name.substr(1);
    }

    if (oat_file_name.substr(oat_file_name.size()-4) != ".art") {
      VLOG(exact_profiler) << "Malformed name: " << oat_file_name;;
      return nullptr;
    }

    // Replace '/' by '@'.
    std::replace(oat_file_name.begin(), oat_file_name.end(), '/', '@');

    // Final profile name = <cache_dir>/profile/<name_with_@s_and_no_dot_oat>.prof.
    std::string cache_dir = GetDalvikCache(GetInstructionSetString(isa), false);

    // Convert from .art to .oat/.prof.
    oat_file_name = oat_file_name.substr(0, oat_file_name.size()-4);
    prof_name = cache_dir + "/profile/" + oat_file_name + ".prof";
    oat_file_name = cache_dir + '/' + oat_file_name + ".oat";

    VLOG(exact_profiler) << "profile name(2): " << prof_name;;
    VLOG(exact_profiler) << "OAT name(2): " << oat_file_name;;
  } else if (!prof_file_name_.empty()) {
    // 2. We have already computed the name.
    prof_name = prof_file_name_;
    VLOG(exact_profiler) << "profile name(3): " << prof_name;
  } else {
    DCHECK(is_image);
    // 3. is_image == true && is_boot_image_option == false:
    //    This is for compiling {boot,core}.oat.
    size_t last_slash = oat_file_name.rfind('/');
    if (last_slash == std::string::npos) {
      // Try in the current directory.
      oat_file_name = "./" + oat_file_name;
      last_slash = 1;
    }

    size_t last_dot = oat_file_name.rfind('.');
    if (last_dot == std::string::npos || last_dot < last_slash) {
      // Malformed name.
      VLOG(exact_profiler) << "Malformed name: " << oat_file_name;;
      return nullptr;
    }

    // Construct the profile name from the oat file name.
    prof_name = oat_file_name.substr(0, last_slash + 1) + "profile/";
    prof_name += oat_file_name.substr(last_slash + 1, last_dot - last_slash) + "prof";
    VLOG(exact_profiler) << "profile name(1): " << prof_name;;
  }

  // Is it there and valid?
  struct stat file_info;
  int ret = stat(prof_name.c_str(), &file_info);
  if (ret < 0 || static_cast<size_t>(file_info.st_size) < sizeof(ExactProfileFile)) {
    // No, it wasn't there or was too small. See if it is an image file.
    if (ret < 0) {
      VLOG(exact_profiler) << "ret = " << ret << ", errno = " << errno
          << " (" << strerror(errno) << ')';
    } else {
      VLOG(exact_profiler) << "file size = " << file_info.st_size
          << ", header size = " << sizeof(ExactProfileFile);
    }
    return nullptr;
  }

  // Read in the file, and confirm that we have a valid profile.
  char* buffer = new char[file_info.st_size];
  if (buffer == nullptr) {
    LOG(WARNING) << "Unable to allocate memory for profile file: " << prof_name;
    return nullptr;
  }

  // Remember that we have the buffer.
  std::unique_ptr<ExactProfileFile> prof;
  prof.reset(reinterpret_cast<ExactProfileFile*>(buffer));

  int fd = ::open(prof_name.c_str(), O_RDONLY);

  if (fd < 0) {
    LOG(WARNING) << "Unable to open profile file for reading: " << prof_name;
    return nullptr;
  }

  // This better not change while we are reading it.
  if (flock(fd, LOCK_SH) < 0) {
    close(fd);
    LOG(WARNING) << "Unable to flock profile file for reading: " << prof_name
                 << ": " << strerror(errno);
    return nullptr;
  }

  size_t bytes_read = read(fd, buffer, file_info.st_size);
  // Lock will be released on close.
  close(fd);
  if (bytes_read != static_cast<size_t>(file_info.st_size)) {
    LOG(WARNING) << "Problem reading profile file: " << prof_name;
    return nullptr;
  }

  // Is the profile file valid?
  if (prof->magic1 != Magic1_) {
    LOG(WARNING) << "Invalid magic number for " << prof_name
                 << ", found 0x" << std::hex << prof->magic1
                 << ", expected 0x" << Magic1_;
    return nullptr;
  }

  if (prof->version != CurrentVersion_) {
    LOG(WARNING) << "Invalid version number for " << prof_name
                 << ", found 0x" << std::hex << prof->version
                 << ", expected 0x" << CurrentVersion_;
    return nullptr;
  }

  // Save the OAT filename for matching in FindMethodCounts.
  std::string *name = new std::string(oat_file_name);
  // Nasty trick, to store a (possible 64 bit) pointer in runtime_temp without
  // declaring them as void *, in order to keep all fields aligned properly.
  std::string **runtime_temp_as_ptr = reinterpret_cast<std::string**>(&prof->runtime_temp);
  *runtime_temp_as_ptr = name;

  // Lets keep it.
  existing_profiles_.push_back(prof.get());

  // Read in the OAT table, if present.
  use_oat_dex_table_.reset(new OatDexTableManager(prof.get()));
  return prof.release();
}

OneMethod* ExactProfiler::FindMethodCounts(HGraph* graph) {
  // Find the matching method counts.
  const DexFile& dex_file = graph->GetDexFile();
  uint32_t method_idx = graph->GetMethodIdx();
  const std::string& location = dex_file.GetLocation();
#if MORE_DEBUG
  VLOG(exact_profiler) << "Dexfile: " << &dex_file;
  VLOG(exact_profiler) << "Dex file location: " << dex_file.GetLocation();
  VLOG(exact_profiler) << "Dex method idx: " << method_idx;
  VLOG(exact_profiler) << "Dex base file location: " << dex_file.GetBaseLocation();
  VLOG(exact_profiler) << "Checksum: 0x" << std::hex << dex_file.GetHeader().checksum_;
#endif
  OneMethod* method_data = nullptr;

  // Have we seen this DexFile before?
  {
    MutexLock mu(nullptr, method_lock_);
    auto it = profile_map_.find(&dex_file);
    if (it != profile_map_.end()) {
      ProfileDexFile& prof_df = it->second;
      ExactProfileFile* prof = prof_df.first;
      char *base = reinterpret_cast<char*>(prof);
      const OneDexFile* df = prof_df.second;
#if MORE_DEBUG
      VLOG(exact_profiler) << "Try to match with: " << base + df->offset_to_dex_file_name;
#endif
      DCHECK_EQ(df->dex_checksum, dex_file.GetHeader().checksum_);
      DCHECK_EQ(location, base + df->offset_to_dex_file_name);
#if MORE_DEBUG
      VLOG(exact_profiler) << "1: Matched profile: Checksum = 0x" << std::hex << df->dex_checksum;
      VLOG(exact_profiler) << "1: Num methods = " << df->num_methods;
#endif
      uint32_t* index_table = reinterpret_cast<uint32_t*>(base + df->method_index_offsets);

      if (method_idx < df->num_methods) {
        method_data = reinterpret_cast<OneMethod*>(base + index_table[method_idx]);
        return method_data;
      }
      return nullptr;
    }
  }

  // Look it up the slow way.
  for (auto prof : existing_profiles_) {
    // Try to match the dex file name.
    char *base = reinterpret_cast<char*>(prof);
    uint32_t* dex_file_table = prof->offset_to_dex_infos;
    for (uint32_t i = 0; i < prof->num_dex_files; i++) {
      if (dex_file_table[i] == 0) {
        // Unallocated dex file.
        continue;
      }
      OneDexFile& df = *reinterpret_cast<OneDexFile*>(base + dex_file_table[i]);
#if MORE_DEBUG
      VLOG(exact_profiler) << "Try to match with: " << base + df.offset_to_dex_file_name;
#endif
      if (df.dex_checksum != dex_file.GetHeader().checksum_) {
        // Can't possibly match.
        continue;
      }
      if (location == base + df.offset_to_dex_file_name) {
#if MORE_DEBUG
        VLOG(exact_profiler) << "Matched profile: Checksum = 0x" << std::hex << df.dex_checksum;
        VLOG(exact_profiler) << "Num methods = " << df.num_methods;
#endif

        // Remember this for later.
        {
          MutexLock mu(nullptr, method_lock_);
          auto it = profile_map_.find(&dex_file);
          if (it == profile_map_.end()) {
            // It wasn't entered while we were looking.
            ProfileDexFile info(prof, &df);
            profile_map_.Put(&dex_file, info);
          }
        }
        // Continue checking...
        uint32_t* index_table = reinterpret_cast<uint32_t*>(base + df.method_index_offsets);

        if (method_idx < df.num_methods) {
          method_data = reinterpret_cast<OneMethod*>(base + index_table[method_idx]);
          return method_data;
        }
      }
    }
  }
  return nullptr;
}

ExactProfiler* ExactProfiler::FindExactProfiler(std::vector<std::unique_ptr<ExactProfiler>>& eps,
                                                HGraph* graph) {
  if (eps.size() == 1) {
    return eps[0].get();
  }

  // Multiple OAT files.  Look for a match.
  // Have we seen this DexFile before?
  const DexFile& dex_file = graph->GetDexFile();
  static Mutex finder_lock("exact profile finder");
  static std::map<const DexFile*, ExactProfiler*> map;
  MutexLock mu(nullptr, finder_lock);
  auto it = map.find(&dex_file);
  if (it != map.end()) {
    return it->second;
  }

  for (uint32_t i = 0, e = eps.size(); i < e; i++) {
    if (eps[i]->ContainsDexFile(dex_file)) {
      ExactProfiler* ep = eps[i].get();
      map.insert(std::pair<const DexFile*, ExactProfiler*>(&dex_file, ep));
      return ep;
    }
  }

  // Remember that we don't know about it for next time.
  map.insert(std::pair<const DexFile*, ExactProfiler*>(&dex_file, nullptr));
  return nullptr;
}

void ExactProfiler::SetContainsDexFile(const DexFile* dex_file)
    REQUIRES(!method_lock_) {
  MutexLock mu(nullptr, method_lock_);
  dex_files_.insert(dex_file);
}

void ExactProfiler::ClearClassMap()
    REQUIRES(!method_lock_) {
  MutexLock mu(nullptr, method_lock_);
  class_map_.clear();
}

void ExactProfiler::ClearProfileInfos()
    REQUIRES(!Locks::profiler_lock_) {
  MutexLock mu(Thread::Current(), *Locks::profiler_lock_);
  profile_infos_.clear();
}

bool ExactProfiler::ContainsDexFile(const DexFile& dex_file)
    NO_THREAD_SAFETY_ANALYSIS {
  MutexLock mu(nullptr, method_lock_);
  return dex_files_.find(&dex_file) != dex_files_.end();
}

void ExactProfiler::VisitRoots(OatToExactProfileFileMap& profiles, RootVisitor* visitor)
    NO_THREAD_SAFETY_ANALYSIS {
  VLOG(exact_profiler) << "ExactProfiler::VisitRoots() Start";
  RootInfo root_info(kRootUnknown);
  for (auto& it : profiles) {
    ExactProfileFile* hdr = it.second;
    char *base = reinterpret_cast<char*>(hdr);
    uint32_t* dex_file_table = hdr->offset_to_dex_infos;

    for (uint32_t i = 0; i < hdr->num_dex_files; i++) {
      if (dex_file_table[i] == 0) {
        // Unallocated dex file.
        continue;
      }
      OneDexFile& df = *reinterpret_cast<OneDexFile*>(base + dex_file_table[i]);
      uint32_t* method_table = reinterpret_cast<uint32_t*>(base + df.method_index_offsets);
      for (uint32_t j = 0; j < df.num_methods; j++) {
        if (method_table[j] == 0U) {
          // Unallocated method.
          continue;
        }
        OneMethod* method_info = reinterpret_cast<OneMethod*>(base + method_table[j]);
        for (uint32_t k = 0; k < method_info->num_method_invokes; k++) {
          OneCallSite* call_site = method_info->CallSiteAt(k);
          for (int l = 0; l < OneCallSite::kNumInvokeTargets; l++) {
            call_site->targets[l].klass.VisitRootIfNonNull(visitor, root_info);
          }
        }
      }
    }
  }
  VLOG(exact_profiler) << "ExactProfiler::VisitRoots() Visit class maps";
  {
    for (auto it : all_exact_profiles_) {
      MutexLock mu(nullptr, it->method_lock_);
      for (auto& it2 : it->class_map_) {
        it2.second.VisitRootIfNonNull(visitor, root_info);
      }
    }
  }
  VLOG(exact_profiler) << "ExactProfiler::VisitRoots() Visit method profiles";
  {
    MutexLock mu(Thread::Current(), *Locks::profiler_lock_);
    BufferedRootVisitor<kDefaultBufferedRootCount> buffered_visitor(visitor, root_info);
    for (auto& it : profile_infos_) {
      it.second->VisitRoots(buffered_visitor);
    }
  }
  VLOG(exact_profiler) << "ExactProfiler::VisitRoots() End";
}

mirror::Class* ExactProfiler::FindClass(const CompilerDriver* driver, PersistentClassIndex index)
    REQUIRES(!method_lock_) SHARED_REQUIRES(Locks::mutator_lock_) {
  if (use_oat_dex_table_ == nullptr) {
    // Nothing that we can do.
    return nullptr;
  }

  // Have we seen this already?
  Thread* thread = Thread::Current();
  {
    MutexLock mu(thread, method_lock_);
    {
      auto it = class_map_.find(index);
      if (it != class_map_.end()) {
        return it->second.Read();
      }
    }
  }

  mirror::Class* result = nullptr;

  uint32_t dex_checksum = use_oat_dex_table_->GetDexChecksum(index.DexIndex());
  const DexFile* found_dex_file = nullptr;
  for (const DexFile* dex_file : driver->GetDexFilesForOatFile()) {
    if (dex_file->GetHeader().checksum_ == dex_checksum) {
      found_dex_file = dex_file;
      break;
    }
  }

  if (found_dex_file == nullptr) {
    // We didn't match in the file we are compiling.  Let's try to boot OAT(s).
// if (index.DexIndex() == 0) asm("int3");
    auto it = saved_boot_dex_files_.find(index.DexIndex());
    if (it != saved_boot_dex_files_.end()) {
      found_dex_file = it->second.get();
    } else {
      uint32_t dex_loc_checksum = use_oat_dex_table_->GetDexLocChecksum(index.DexIndex());
      OatFileManager& oat_mgr = Runtime::Current()->GetOatFileManager();
      for (const OatFile* oat_file : oat_mgr.GetBootOatFiles()) {
        for (const OatDexFile* oat_dex_file : oat_file->GetOatDexFiles()) {

          if (oat_dex_file->GetDexFileLocationChecksum() == dex_loc_checksum) {
            // Open the Dex file and save for later use.
            std::string error_message;
            std::unique_ptr<const DexFile> dex_file(oat_dex_file->OpenDexFile(&error_message));
            if (dex_file != nullptr) {
              found_dex_file = dex_file.get();
              saved_boot_dex_files_.insert(
                  BootDexMap::value_type(index.DexIndex(), std::move(dex_file)));
              break;
            }
          }
        }
        if (found_dex_file != nullptr) {
          break;
        }
      }
    }
  }

  // Did we get anything?
  if (found_dex_file != nullptr && index.ClassDefIndex() < found_dex_file->NumClassDefs()) {
    const DexFile::ClassDef& class_def = found_dex_file->GetClassDef(index.ClassDefIndex());
    const char* class_descriptor = found_dex_file->GetClassDescriptor(class_def);
    std::vector<mirror::Class*> classes;
    Runtime::Current()->GetClassLinker()->LookupClasses(class_descriptor, classes);
    // Find a matching class with the right DexFile checksum.
    for (auto it : classes) {
      const DexFile* found_dex = &it->GetDexFile();
      if (dex_checksum == found_dex->GetHeader().checksum_) {
        // We have a match!
        result = it;
        break;
      }
    }
  }

  // Remember the result for the next query.
  MutexLock mu(thread, method_lock_);
  class_map_.insert(std::pair<PersistentClassIndex, GcRoot<mirror::Class>>(index, result));
  return result;
}

void ExactProfiler::SetProfileForMethod(ArtMethod* method, ProfilingInfo* profile)
    REQUIRES(!Locks::profiler_lock_) {
  MutexLock mu(nullptr, *Locks::profiler_lock_);
  profile_infos_[method] = std::unique_ptr<ProfilingInfo>(profile);
}

ProfilingInfo* ExactProfiler::FindProfileForMethod(ArtMethod* method)
    REQUIRES(!Locks::profiler_lock_) {
  MutexLock mu(nullptr, *Locks::profiler_lock_);
  auto it = profile_infos_.find(method);
  if (it == profile_infos_.end()) {
    return nullptr;
  }
  return it->second.get();
}

bool ExactProfiler::GeneratingProfile(const OatFile& oat_file) {
  const char* profile_generate =
      oat_file.GetOatHeader().GetStoreValueByKey("profile-generate");
  return profile_generate != nullptr && strcmp(profile_generate, "true") == 0;
}


}  // namespace art
