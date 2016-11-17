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

#ifndef COMPILER_OPTIMIZING_EXTENSIONS_INFRASTRUCTURE_EXT_PROFILING_H
#define COMPILER_OPTIMIZING_EXTENSIONS_INFRASTRUCTURE_EXT_PROFILING_H

#include "base/mutex.h"
#include "oat_file.h"
#include "safe_map.h"
#include <ostream>

namespace art {

namespace mirror {
  class Class;
}

class CompilerDriver;
class DexFile;
class ExactProfileSaver;
class HGraph;
class HGraph_X86;
class HBasicBlock;
class OatDexTableManager;
class ProfilingInfo;

// These are defined outside of ExactProfiler, to be able to be used as a forward declaration.
struct OneDexFile {
  // Should be a multiple of 64 bits to align properly.
  uint32_t offset_to_dex_file_name;    // Offset to UTF-8 filename in 0xB00DDADB section.
  uint32_t dex_checksum;               // Checksum of Dex file.
  uint32_t num_methods;                // Number of methods defined in the Dex file.
  uint32_t method_index_offsets;       // Offset to uint32_t[num_methods] of offsets.
};

// Information about the class of a virtual/interface invoke.
struct PersistentClassIndex {
  struct Info {
    uint8_t oat_index;                   // Index into OatTable.
    uint8_t dex_index;                   // Index of dex file withing OAT file.
    uint16_t class_def_index;            // Class definition within dex file.
  };

  union {
    Info info;
    uint32_t info_as_uint;
  };

  /*
   * @brief Create an empty index.
   */
  PersistentClassIndex() : info_as_uint(0) {}

  /*
   * @brief Create an index corresponding to the oat/dex/class indicies.
   */
  PersistentClassIndex(uint8_t oat_index_, uint8_t dex_index_, uint16_t class_def_index_) {
    DCHECK_NE(oat_index_, std::numeric_limits<uint8_t>::max());
    info.oat_index = oat_index_ + 1;
    info.dex_index = dex_index_;
    info.class_def_index = class_def_index_;
  }

  /*
   * @brief Return the index into the oat table.
   * @returns the index into the oat table.
   */
  uint8_t OatIndex() const {
    return info.oat_index - 1;
  }

  /*
   * @brief Return the index of the dex file in the oat file.
   * @returns Return the index of the dex file in the oat file
   */
  uint8_t DexIndex() const {
    return info.dex_index;
  }

  /*
   * @brief Return the index of the class type in the dex file.
   * @returns Return the index of the class type in the dex file.
   */
  uint16_t ClassDefIndex() const {
    return info.class_def_index;
  }

  /*
   * @brief Is this an empty (null) index?
   * @returns true if the index does not represent a valid class.
   */
  bool IsNull() const {
    return info_as_uint == 0;
  }

  /*
   * @brief Does the rhs represent the same type as me?
   * @returns true if the rhs index is identical.
   */
  bool operator==(const PersistentClassIndex& rhs) const {
    return info_as_uint == rhs.info_as_uint;
  }

  /*
   * @brief Implement '<' for maps.
   * @returns 'true' for an arbitrary ordering.
   */
  bool operator<(const PersistentClassIndex& rhs) const {
    return info_as_uint < rhs.info_as_uint;
  }
};

std::ostream& operator<<(std::ostream& os, const PersistentClassIndex& index);

// Information about one invoke.
struct OneInvoke {
  typedef uint32_t CountType;

  union {
    GcRoot<mirror::Class> klass;       // Pointer to target class.
    PersistentClassIndex class_index;  // Version target class for storing in file.
  };
  CountType count;                     // Number of this target. Saturates at MAX_UINT.
};

// Information for one virtual/invoke call site.
struct OneCallSite {
  // Maximum recorded  invoke targets per call site.
  static int constexpr kNumInvokeTargets = 5;

  uint16_t  dex_pc;                      // Offset of invoke in method

  OneInvoke targets[kNumInvokeTargets];  // The recorded targets.
};

// Profiling information for one Method.
struct OneMethod {
  typedef uint64_t CountType;

  uint32_t num_blocks;                   // number of blocks being counted in the method.
  uint32_t num_method_invokes;           // number of invoke target sites.
  CountType counts[0 /* num_blocks */];  // Execution counts for the method.
                                         // Merging will cap at max(CountType)-1.

  // Only one 0 sized array is legal in C++.
  // OneCallSite invokes[0 /* num_method_invokes */];

  /*
   * Compute the necessary size to hold the profiling information.
   * @param num_blocks Number of Basic blocks in the method.
   * @param num_method_invokes Number of virtual/interface invokes in the method.
   * @returns the number of bytes needed to hold the profiling data for the method.
   */
  static size_t AllocationSize(uint32_t num_blocks, uint32_t num_method_invokes) {
    return sizeof(OneMethod) + num_blocks * sizeof(CountType) +
        num_method_invokes * sizeof(OneCallSite);
  }

  /*
   * Compute the address of the indexed OneCallSite.
   * @param index index number.
   * @returns a pointer to the indexed OneMethod.
   */
  OneCallSite* CallSiteAt(uint32_t index) {
    char* base = reinterpret_cast<char *>(this);
    base += sizeof(OneMethod) + num_blocks * sizeof(CountType);
    OneCallSite* call_sites = reinterpret_cast<OneCallSite*>(base);
    DCHECK_LT(index, num_method_invokes);
    return &call_sites[index];
  }
};

// Information about one OAT file for invoke class targets.
struct OneOatIndex {
  uint32_t oat_checksum;                   // Use instead of string compares?
  uint32_t offset_of_oat_location_string;  // in LocationStringTable.
};

// Table of all OAT file targets.
struct OatTable {
  uint32_t num_oat_locations;              // Number of used OAT locations.
  uint32_t num_allocated_oat_locations;    // Number of allocated OAT locations.
  OneOatIndex oat_locations[0 /* num_allocated_oat_locations */];
 };

// Information about one Dex file for invoke class targets.
struct OneDexIndex {
  uint32_t dex_index_in_oat;               // Index of Dex file in OAT.
  uint32_t dex_checksum;                   // Use instead of string compares.
  uint32_t dex_loc_checksum;               // Use instead of string compares for boot.
  uint32_t offset_of_dex_location_string;  // in LocationStringTable.
};

// Table of all Dex file targets.
struct DexTable {
  uint32_t num_dex_locations;              // Number of used Dex locations.
  uint32_t num_allocated_dex_locations;    // Number of allocated Dex locations.
  OneDexIndex dex_locations[0 /* num_allocated_dex_locations */];
 };

// Strings for OAT/Dex locations.  Will grow dynamically as new OAT/Dex locations added.
// Locations are NUL terminated UTF-8 strings.
struct LocationStringTable {
  uint32_t num_string_chars;            // Number of string characters in use.
  uint32_t num_allocated_string_chars;  // Number of string characters in allocated.
  char strings[0 /* num_allocated_string_chars */];
};

// Format for fixed part of prof file:
struct ExactProfileFile {
  uint32_t    magic1;                    // 0xB00DDADA.
  uint32_t    version;                   // Currently 0x1.
  uint32_t    oat_checksum;              // Checksum of oat file.
  uint32_t    num_dex_files;             // methods compiled by optimizing compiler.
  uint32_t    total_num_counters;        // Total number of counters.
  uint32_t    total_num_methods;         // Total number of methods.
  uint32_t    offset_to_oat_table;       // Offset from base to OAT table.
  uint32_t    offset_to_dex_table;       // Offset from base to Dex table.
  uint32_t    offset_to_string_table;    // Offset from base to OAT/Dex string table.
  uint32_t    variable_start_offset;     // Where the variable part of the file starts.
  uint64_t    runtime_temp;              // Used by runtime to hold filename.
  uint32_t    generating_profile;        // Used by runtime to remember if OAT file
                                         // is generating profiling information.
  uint32_t    dummy_align;               // Ensure non-varying is multiple of 64 bits.
  // offsets to each dex file.  0 for 'not present'.
  uint32_t    offset_to_dex_infos[0 /* num_dex_files */];
};

class ExactProfiler {
 public:
  /*
   * @brief Construct the data structures for profiling counts.
   * @param oat_file_name Name of the OAT file, to be used to generate the
   * @param profile_dir Name of the profile_dir (empty if not boot.oat)
   * '.prof' filename.
   */
  ExactProfiler(const std::string& oat_file_name, const std::string& profile_dir);

  /*
   * Destructor
   */
  ~ExactProfiler();

  /*
   * @brief Map from an OatFile to the matching ExactProfileFile.
   */
  typedef SafeMap<const OatFile*, ExactProfileFile*> OatToExactProfileFileMap;

  /*
   * @brief Register a method with the profiler.
   * @param graph HGraph_X86 for the method.
   */
  void RegisterMethod(HGraph_X86& graph)
      REQUIRES(!method_lock_);

  /*
   * @brief Populate and write the profiling data file at prof_file_name_.
   * @param update_checksum_if_needed 'true' if only the checksum needs to be updated.
   * @returns 'true' if the creation was successful.
   */
  bool WriteProfileFile(bool update_checksum_if_needed)
      REQUIRES(!method_lock_);

  /*
   * @brief Update the profiling data file after an execution.
   * @param profiles Map from OatFile to ExactProfileFile with new counts.
   * @param zero_counters 'true' if the counters need to be zero'ed after writing.
   */
  static void UpdateProfileFiles(OatToExactProfileFileMap& profiles, bool zero_counters = false)
      SHARED_REQUIRES(Locks::mutator_lock_);

  /*
   * @brief Return the file name for the profiling data file.
   * @returns the file name for the profiling data file.
   */
  const std::string& GetProfileFileName() const { return prof_file_name_; }

  /*
   * @brief Return the file name for the profiling data file.
   * @returns pathname of the profile file
   * @note this may be relative to the dirname of the OAT file if not compiling a package.
   */
  std::string GetRelativeProfileFileName() const;

  static constexpr uint32_t Magic1_ = 0xB00DDADA;
  static constexpr uint32_t Magic2_ = 0xB00DDADB;
  static constexpr uint32_t Magic3_ = 0xB00DDADC;
  static constexpr uint32_t CurrentVersion_ = 0x00000001;
  static constexpr uint64_t MaxCount = std::numeric_limits<OneMethod::CountType>::max();

  /*
   * @brief Examine the OatFile and return the address of a matching profile
   * file.
   * @param oat_file The OAT file that may point to a profile file.
   * @param is_image 'true' if this is core.oat or boot.oat image.
   * @param zero_counts 'true' if the returned counts should be zeroed.
   * @returns An allocated buffer containing the profile file or nullptr if not available.
   */
  static ExactProfileFile* GetProfileFile(const OatFile& oat_file,
                                          bool is_image,
                                          bool zero_counts);

  /*
   * @brief Allocate Profiling counters for the given method in loaded OAT file.
   * @param method ArtMethod* pointer for this method.
   * @param hdr Matching Exact Profile file mapped into memory.
   * @param dex_file Internal version of the dex file.
   * @param method_index Index in dex file for this method.
   */
  static void AllocateProfileCounters(ArtMethod* method,
                                      ExactProfileFile* hdr,
                                      const DexFile* dex_file,
                                      uint32_t dex_file_index,
                                      uint32_t method_index)
      SHARED_REQUIRES(Locks::mutator_lock_) REQUIRES(Locks::profiler_lock_);

  /*
   * @brief Allocate Profiling counters for the given methods in boot.oat.
   * @param hdr Matching Exact Profile file mapped into memory.
   * @param dex_file_index The number of the matching dex file in the Profile file.
   * @param dex_file_location The name of the matching dex file.
   * @param dex_file Internal version of the dex file.
   * @param art_methods Array of ArtMethod* for boot.oat.
   * @param num_methods number of ArtMethod* in art_methods.
   */
  static void AllocateProfileCounters(ExactProfileFile* hdr,
                                      int32_t dex_file_index,
                                      const std::string& dex_file_location,
                                      const DexFile* dex_file,
                                      ArtMethod** art_methods,
                                      size_t num_methods)
      SHARED_REQUIRES(Locks::mutator_lock_) REQUIRES(Locks::profiler_lock_);

  /*
   * @brief Start background saver task.
   */
  static void StartProfileSaver();

  /*
   * @brief Start background saver task.
   */
  static void StopProfileSaver();

  /*
   * @brief Are there any existing profiles?
   * @return 'true' if there is at least one profile.
   */
  bool HasExistingProfiles() const { return existing_profiles_.size() != 0; }

  /*
   * @brief Look for an existing profile file and add it to the known profiles if present.
   * @param oat_location The OAT file that being compiled (or boot/core ART file).
   * @param is_image 'true' if this is core.oat or boot.oat image.
   * @param is_boot_image_option 'true' if this is oat_location is
   * really -Ximage:boot_image_art string.
   * @param isa Instruction set being compiled.
   * @returns An allocated buffer containing the profile file or nullptr if not available.
   * @note If the buffer is allocated, it is also added to exiting_profiles_.
   */
  ExactProfileFile* AddProfileFile(const std::string& oat_location,
                                   bool is_image,
                                   bool is_boot_image_option,
                                   InstructionSet isa = kNone);

  /*
   * @brief Find the matching OneMethod for a method being compiled.
   * @param graph HGraph for the method being compiled.
   * @returns the matching OneMethod, or nullptr if not found.
   */
  OneMethod* FindMethodCounts(HGraph* graph)
      REQUIRES(!method_lock_);

  /*
   * @brief Return the filename of the OAT file for this profile file.
   * @returns the OAT filename.
   */
  const std::string& GetOatFileName() const {
    return oat_file_name_;
  }

  /*
   * @brief Map from an index to a matching mirror:Class*.
   * @note This is used for AOT use-profile to populate the JIT invoke map.
   * @param driver The compiler driver for this compilation.
   * @param index The saved class index for this invoke type.
   * @returns The matching Class* if found, or 'nullptr'.
   */
  mirror::Class* FindClass(const CompilerDriver* driver, PersistentClassIndex index)
      REQUIRES(!method_lock_) SHARED_REQUIRES(Locks::mutator_lock_);

  /*
   * @brief Locate the correct ExactProfiler that matches the method being compiled.
   * @param eps vector of possible ExactProfiler(s).
   * @param graph HGraph for the method being compiled.
   * @returns the matching ExactProfiler or nullptr if not found.
   */
  static ExactProfiler* FindExactProfiler(std::vector<std::unique_ptr<ExactProfiler>>& eps,
                                          HGraph* graph);

  /*
   * @brief Note that the ExactProfiler will profile this dex file.
   * @param dex_file Dex_file to be associated with this ExactProfiler.
   */
  void SetContainsDexFile(const DexFile* dex_file)
      REQUIRES(!method_lock_);

  /*
   * @brief Clear the class map used to map from indicies to mirror::Class*.
   */
  void ClearClassMap()
      REQUIRES(!method_lock_);

  /*
   * @brief Visit all roots in the profiling buffers.
   * @param profiles Profiles currently active in the runtime.
   * @param visitor root visitor to be invoked for each root.
   */
  // NO_THREAD_SAFETY_ANALYSIS since we don't know what the callback requires.
  static void VisitRoots(OatToExactProfileFileMap& profiles, RootVisitor* visitor)
      NO_THREAD_SAFETY_ANALYSIS;

  /*
   * @brief Associate a ProfilingInfo with an ArtMethod to hold invoke information.
   * @param method ArtMethod being compiled.
   * @param profile Profiling information for the method.
   */
  static void SetProfileForMethod(ArtMethod* method, ProfilingInfo* profile)
      REQUIRES(!Locks::profiler_lock_);

  /*
   * @brief Return the ProfilingInfo for an ArtMethod to hold invoke information.
   * @param method ArtMethod being compiled.
   * @returns Profiling information for the method, or nullptr if not present.
   */
  static ProfilingInfo* FindProfileForMethod(ArtMethod* method)
      REQUIRES(!Locks::profiler_lock_);

  /*
   * @brief Clear the profile map used to map from ArtMethod* to ProfileInformation.
   */
  static void ClearProfileInfos()
      REQUIRES(!Locks::profiler_lock_);

 private:
  mutable Mutex method_lock_ DEFAULT_MUTEX_ACQUIRED_AFTER;

  // Information needed for profiling a method.
  typedef std::pair<uint32_t, std::vector<uint16_t>> ProfileInformation;

  // Map from method index in dex file to #blocks used.
  typedef SafeMap<uint32_t, ProfileInformation> PerMethodInformation;

  // Each Dex file maintains an independent map of methods in the dex file.
  typedef SafeMap<const DexFile*, PerMethodInformation> Info;

  Info info_ GUARDED_BY(method_lock_);

  std::set<const DexFile*> dex_files_ GUARDED_BY(method_lock_);

  std::string oat_file_name_;
  std::string prof_file_name_;
  std::string prof_dir_name_;

  // Existing profiles to be used when optimizing.
  std::vector<ExactProfileFile*> existing_profiles_;

  // Faster lookup into existing_profiles_;
  typedef std::pair<ExactProfileFile*, const OneDexFile*> ProfileDexFile;
  typedef SafeMap<const DexFile*, ProfileDexFile> DexFileToOneDexFile;
  DexFileToOneDexFile profile_map_;

  static ExactProfileSaver *saver_ GUARDED_BY(Locks::profiler_lock_);

  // Return the file descriptor of the open file, or -1 on failure.
  static int OpenProfileFile(const OatFile& oat_file,
      bool read_write,
      bool is_image,
      std::string& prof_name);

  bool ContainsDexFile(const DexFile& dex_file);

  static bool GeneratingProfile(const OatFile& oat_file);

  std::unique_ptr<OatDexTableManager> use_oat_dex_table_;

  // Remember all ExactProfiler(s) so we can GC the class_map_(s).
  static std::set<ExactProfiler*> all_exact_profiles_ GUARDED_BY(Locks::profiler_lock_);

  // Map for invoke target classes.
  std::map<PersistentClassIndex, GcRoot<mirror::Class>> class_map_ GUARDED_BY(method_lock_);

  // Map for associating ProfilingInfo with an ArtMethod.
  typedef std::map<ArtMethod*, std::unique_ptr<ProfilingInfo>> MethodProfileMap;
  static MethodProfileMap profile_infos_ GUARDED_BY(Locks::profiler_lock_);

  // Map for saving DexFiles that were matched in FindClass from Boot oatfiles.
  typedef std::map<uint32_t, std::unique_ptr<const DexFile>> BootDexMap;
  BootDexMap saved_boot_dex_files_;
};

}  // namespace art

#endif  // COMPILER_OPTIMIZING_EXTENSIONS_INFRASTRUCTURE_EXT_PROFILING_H
