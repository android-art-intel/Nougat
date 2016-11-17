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

#include <vector>
#include <algorithm>

#include "vtune_support.h"
#include "mapping_table.h"
#include "oat_file-inl.h"
#include "dex_file-inl.h"
#include "dex_instruction.h"
#include "vtune/jitprofiling.h"

namespace art {

typedef std::vector<LineNumberInfo> LineInfoTable;

struct SortLineNumberInfoByOffset {
  bool operator()(LineNumberInfo const& lhs, LineNumberInfo const& rhs) {
    return lhs.Offset < rhs.Offset;
  }

  static void sort(LineInfoTable &table) {
    std::sort(table.begin(), table.end(), SortLineNumberInfoByOffset());
  }
};

struct SortLineNumberInfoByLine {
  static const unsigned NOT_FOUND = 0 - 1;

  bool operator()(LineNumberInfo const& lhs, LineNumberInfo const& rhs) {
    return lhs.LineNumber < rhs.LineNumber;
  }

  static void sort(LineInfoTable &table) {
    std::sort(table.begin(), table.end(), SortLineNumberInfoByLine());
  }

  static int find(LineInfoTable &table, unsigned line, bool strict = false) {
    LineInfoTable::iterator lower =
           std::lower_bound(table.begin(),
                            table.end(),
                            LineNumberInfo({0, line}),
                            SortLineNumberInfoByLine());
    return lower == table.end() || (strict && lower->LineNumber != line)
           ? NOT_FOUND
           : lower->Offset;
  }
};

struct LineInfoReader {
  LineInfoReader(LineInfoTable* pc2dex, LineInfoTable* pc2java, std::string* source_file_name)
    : pc2dex_(pc2dex), pc2java_(pc2java), source_file_name_(source_file_name) {
    DCHECK((pc2dex == nullptr) == (pc2java == nullptr));
    DCHECK((pc2dex != nullptr) || (source_file_name != nullptr));
  }
  LineInfoTable* pc2dex_;
  LineInfoTable* pc2java_;
  std::string* source_file_name_;

  static bool readEntry(void* context, const DexFile::PositionInfo& entry) {
    LineInfoReader* self = static_cast<LineInfoReader*>(context);
    if (self->source_file_name_ != nullptr &&
        entry.source_file_ != nullptr &&
        self->source_file_name_->length() == 0) {
      self->source_file_name_[0] = entry.source_file_;
      if (self->pc2java_ == nullptr) {
        return true;  // We just needed the source file name.
      }
    }

    if (self->pc2java_ != nullptr) {
      unsigned offset = SortLineNumberInfoByLine::find(self->pc2dex_[0], entry.address_);
      if (offset != SortLineNumberInfoByLine::NOT_FOUND) {
        self->pc2java_->push_back({offset, entry.line_});
      }
    }

    return false;
  }
 private:
  DISALLOW_COPY_AND_ASSIGN(LineInfoReader);
};


// convert dex offsets to dex disassembly line numbers in pc2dex
static void getLineInfoForDex(const DexFile::CodeItem* code_item, LineInfoTable& pc2dex) {
  SortLineNumberInfoByLine::sort(pc2dex);

  size_t offset = 0; // in code units
  size_t end = code_item->insns_size_in_code_units_;
  int line_no = 1;
  auto li = pc2dex.begin();
  for ( ; offset < end && li != pc2dex.end(); ++line_no) {
    const Instruction* instr = Instruction::At(&code_item->insns_[offset]);
    for ( ; li != pc2dex.end() && li->LineNumber <= offset; li++) {
      // REVIEW: to be strict we have to remove li if li->lineNumber != offset
      li->LineNumber = line_no;
    }
    offset += instr->SizeInCodeUnits();
  }
}

// TODO: come up with another way to get options:
// 1. filter by app package (process nice name)
// 2. filter by dex/oat/class/method
// 3. output map type: none/dex/java
// 4. vtune jit api library path (set INTEL_JIT_PROFILER32 or
//    INTEL_JIT_PROFILER64 environment variable)
// 5. set any other environment vars for the profiler lib (output path, ...)
static const char * ART_VTUNE_JIT_API_SRC = getenv("ART_VTUNE_JIT_API_SRC");

void SendMethodToVTune(const char* method_name,
                       const void* code,
                       size_t code_size,
                       const char* class_file_name,
                       const char* source_file_name,
                       const void* optimized_code_info_ptr,
                       const DexFile* dex_file,
                       const DexFile::CodeItem* code_item) {
  if (code == nullptr) {
    return;
  }

  // Older versions of VTune treated IDs < 1000 as reserved.
  static unsigned next_method_id = 1000;
  next_method_id++;

  const char *method_name_c = strchr(method_name, ' '); // skip return type

  iJIT_Method_Load jit_method;
  memset(&jit_method, 0, sizeof(jit_method));
  jit_method.class_file_name = const_cast<char*>(class_file_name);
  jit_method.source_file_name = const_cast<char*>(source_file_name);
  jit_method.method_name = const_cast<char*>(method_name_c == nullptr ?
      method_name : (method_name_c + 1));
  jit_method.method_id = next_method_id;
  jit_method.method_load_address = const_cast<void*>(code);
  jit_method.method_size = code_size;

  int is_notified;
  if (optimized_code_info_ptr == nullptr) {
    is_notified = iJIT_NotifyEvent(iJVM_EVENT_TYPE_METHOD_LOAD_FINISHED, (void*)&jit_method);
  } else {
    // Prepare pc2src to point to either pc2java or pc2dex.
    CodeInfo code_info(optimized_code_info_ptr);
    LineInfoTable* pc2src = nullptr;
    LineInfoTable pc2dex;
    LineInfoTable pc2java;

    // Read pc->dex mapping from StackMaps.
    CodeInfoEncoding encoding = code_info.ExtractEncoding();
    uint32_t number_of_stack_maps = code_info.GetNumberOfStackMaps(encoding);
    std::string source_name;

    if (dex_file != nullptr && code_item != nullptr && number_of_stack_maps > 0 &&
        (ART_VTUNE_JIT_API_SRC == nullptr || strcmp(ART_VTUNE_JIT_API_SRC, "none") != 0)) {
      for (uint32_t i = 0; i < number_of_stack_maps; ++i) {
        StackMap stack_map = code_info.GetStackMapAt(i, encoding);
        pc2dex.push_back({stack_map.GetNativePcOffset(encoding.stack_map_encoding),
                          stack_map.GetDexPc(encoding.stack_map_encoding)});
      }

      if (ART_VTUNE_JIT_API_SRC != nullptr && strcmp(ART_VTUNE_JIT_API_SRC, "dex") == 0) {
        pc2src = &pc2dex;
        getLineInfoForDex(code_item, pc2dex);
        // TODO: set dexdump file name for this method
      } else { // default is pc -> java
        pc2src = &pc2java;
        LineInfoReader line_info_reader(&pc2dex, &pc2java, &source_name);
        dex_file->DecodeDebugPositionInfo(code_item,
                                          LineInfoReader::readEntry,
                                          &line_info_reader);
        if (source_name.length() > 0) {
          jit_method.source_file_name = const_cast<char*>(source_name.c_str());
        }
      }
    }

    if (pc2src != nullptr && pc2src->size() > 0) {
      // shift offsets
      SortLineNumberInfoByOffset::sort(*pc2src);
      for (unsigned i = 1; i < pc2src->size(); ++i) {
        (*pc2src)[i - 1].Offset = (*pc2src)[i].Offset;
      }
      (*pc2src)[pc2src->size() - 1].Offset = code_size;

      jit_method.line_number_size = pc2src->size();
      jit_method.line_number_table = pc2src->size() == 0 ? nullptr : &((*pc2src)[0]);
    }
    is_notified = iJIT_NotifyEvent(iJVM_EVENT_TYPE_METHOD_LOAD_FINISHED, (void*)&jit_method);
  }

  if (is_notified) {
    VLOG(jit) << "JIT API: method '" << jit_method.method_name
              << "' is written successfully: id=" << jit_method.method_id
              << ", address=" << jit_method.method_load_address
              << ", size=" << jit_method.method_size;
  } else if (VLOG_IS_ON(jit)) {
    LOG(WARNING) << "JIT API: failed to write method '" << jit_method.method_name
                 << "': id=" << jit_method.method_id
                 << ", address=" << jit_method.method_load_address
                 << ", size=" << jit_method.method_size;
  } else {
    static bool printedOnce = false;
    if (!printedOnce) {
      printedOnce = true;
      LOG(WARNING) << "JIT API: Did not write some methods. "
                   << "Enable verbose:jit mode for more details.";
    }
  }
}

}  // namespace art
