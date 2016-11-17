/*
 * Copyright (C) 2015 The Android Open Source Project
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
 *
 * Modified by Intel Corporation
 */

#include "profiling_info.h"

#include "art_method-inl.h"
#include "dex_instruction.h"
#include "dex_instruction-inl.h"
#include "jit/jit.h"
#include "jit/jit_code_cache.h"
#include "scoped_thread_state_change.h"
#include "thread.h"

namespace art {

void ProfilingInfo::ExtractInformation(ArtMethod* method,
                                       std::vector<uint32_t>& entries,
                                       std::vector<uint32_t>& dex_pcs)
    SHARED_REQUIRES(Locks::mutator_lock_) {
  // Walk over the dex instructions of the method and keep track of
  // instructions we are interested in profiling.
  DCHECK(!method->IsNative());
  const DexFile::CodeItem& code_item = *method->GetCodeItem();
  const uint16_t* code_ptr = code_item.insns_;
  const uint16_t* code_end = code_item.insns_ + code_item.insns_size_in_code_units_;

  uint32_t dex_pc = 0;
  std::set<uint32_t> dex_pc_bb_starts;
  // Method entry starts a block.
  dex_pc_bb_starts.insert(0);
  while (code_ptr < code_end) {
    const Instruction& instruction = *Instruction::At(code_ptr);
    switch (instruction.Opcode()) {
      case Instruction::INVOKE_VIRTUAL:
      case Instruction::INVOKE_VIRTUAL_RANGE:
      case Instruction::INVOKE_VIRTUAL_QUICK:
      case Instruction::INVOKE_VIRTUAL_RANGE_QUICK:
      case Instruction::INVOKE_INTERFACE:
      case Instruction::INVOKE_INTERFACE_RANGE:
        entries.push_back(dex_pc);
        break;

      case Instruction::GOTO:
      case Instruction::GOTO_16:
      case Instruction::GOTO_32:
        dex_pc_bb_starts.insert(dex_pc + instruction.GetTargetOffset());
        break;

      case Instruction::IF_EQ:
      case Instruction::IF_NE:
      case Instruction::IF_GT:
      case Instruction::IF_GE:
      case Instruction::IF_LT:
      case Instruction::IF_LE:
      case Instruction::IF_EQZ:
      case Instruction::IF_NEZ:
      case Instruction::IF_GTZ:
      case Instruction::IF_GEZ:
      case Instruction::IF_LTZ:
      case Instruction::IF_LEZ:
        // Need fall-through and target.
        dex_pc_bb_starts.insert(dex_pc + instruction.SizeInCodeUnits());
        dex_pc_bb_starts.insert(dex_pc + instruction.GetTargetOffset());
        break;

      case Instruction::PACKED_SWITCH: {
        const uint16_t* switch_data =
            reinterpret_cast<const uint16_t*>(&instruction) + instruction.VRegB_31t();
        uint16_t size = switch_data[1];
        // After the switch.
        dex_pc_bb_starts.insert(dex_pc + 3);
        const int32_t* targets = reinterpret_cast<const int32_t*>(&switch_data[4]);
        for (uint32_t i = 0; i < size; i++) {
          dex_pc_bb_starts.insert(dex_pc + targets[i]);
        }
        }
        break;

      case Instruction::SPARSE_SWITCH: {
        const uint16_t* switch_data =
            reinterpret_cast<const uint16_t*>(&instruction) + instruction.VRegB_31t();
        uint16_t size = switch_data[1];
        // After the switch.
        dex_pc_bb_starts.insert(dex_pc + 3);
        const int32_t* keys = reinterpret_cast<const int32_t*>(&switch_data[2]);
        const int32_t* switch_entries = keys + size;
        for (uint32_t i = 0; i < size; i++) {
          dex_pc_bb_starts.insert(dex_pc + switch_entries[i]);
        }
        }
        break;

      default:
        break;
    }
    dex_pc += instruction.SizeInCodeUnits();
    code_ptr += instruction.SizeInCodeUnits();
  }

  // Add in catch clauses.
  if (code_item.tries_size_ > 0) {
    const DexFile::TryItem* tries = DexFile::GetTryItems(code_item, 0);
    for (uint32_t i = 0, e = code_item.tries_size_; i < e; i++) {
      const DexFile::TryItem* try_item = &tries[i];
      for (CatchHandlerIterator it(code_item, *try_item); it.HasNext(); it.Next()) {
        dex_pc_bb_starts.insert(it.GetHandlerAddress());
      }
    }
  }

  // Create the list of BB targets from the set.
  dex_pcs.reserve(dex_pc_bb_starts.size());
  for (auto i : dex_pc_bb_starts) {
    dex_pcs.push_back(i);
  }
}

bool ProfilingInfo::Create(Thread* self, ArtMethod* method, bool retry_allocation) {
  std::vector<uint32_t> entries;
  std::vector<uint32_t> dex_pcs;
  ExtractInformation(method, entries, dex_pcs);

  // We always create a `ProfilingInfo` object, even if there is no instruction we are
  // interested in. The JIT code cache internally uses it.

  // Allocate the `ProfilingInfo` object int the JIT's data space.
  jit::JitCodeCache* code_cache = Runtime::Current()->GetJit()->GetCodeCache();
  return code_cache->AddProfilingInfo(self, method, entries, dex_pcs, retry_allocation) != nullptr;
}

ProfilingInfo* ProfilingInfo::Create(ArtMethod* method) {
  std::vector<uint32_t> entries;
  std::vector<uint32_t> dex_pcs;
  ExtractInformation(method, entries, dex_pcs);

  size_t profile_info_size = RoundUp(
      sizeof(ProfilingInfo) + sizeof(InlineCache) * entries.size() +
          sizeof(ProfilingInfo::BBCounts) * dex_pcs.size(),
      sizeof(void*));

  uint8_t* data = new uint8_t[profile_info_size];
  if (data == nullptr) {
    return nullptr;
  }
  memset(data, 0, profile_info_size);
  return new(data) ProfilingInfo(method, entries, dex_pcs);
}

InlineCache* ProfilingInfo::GetInlineCache(uint32_t dex_pc) {
  InlineCache* cache = nullptr;
  // TODO: binary search if array is too long.
  for (size_t i = 0; i < number_of_inline_caches_; ++i) {
    if (cache_[i].dex_pc_ == dex_pc) {
      cache = &cache_[i];
      break;
    }
  }
  return cache;
}

void ProfilingInfo::AddInvokeInfo(uint32_t dex_pc, mirror::Class* cls) {
  InlineCache* cache = GetInlineCache(dex_pc);
  CHECK(cache != nullptr) << PrettyMethod(method_) << "@" << dex_pc;
  for (size_t i = 0; i < InlineCache::kIndividualCacheSize; ++i) {
    mirror::Class* existing = cache->classes_[i].Read();
    if (existing == cls) {
      // Receiver type is already in the cache, increment count.
      uint32_t new_count = cache->class_counts_[i] + 1;

      // Let us not overflow.
      if (new_count != 0) {
        cache->class_counts_[i] = new_count;
      }
      return;
    } else if (existing == nullptr) {
      // Cache entry is empty, try to put `cls` in it.
      GcRoot<mirror::Class> expected_root(nullptr);
      GcRoot<mirror::Class> desired_root(cls);
      if (!reinterpret_cast<Atomic<GcRoot<mirror::Class>>*>(&cache->classes_[i])->
              CompareExchangeStrongSequentiallyConsistent(expected_root, desired_root)) {
        // Some other thread put a class in the cache, continue iteration starting at this
        // entry in case the entry contains `cls`.
        --i;
      } else {
        // We successfully set `cls`, just return.
        cache->class_counts_[i] = 1;
        return;
      }
    }
  }
  // Unsuccessfull - cache is full, making it megamorphic. We do not DCHECK it though,
  // as the garbage collector might clear the entries concurrently.
}

void ProfilingInfo::IncrementBBCount(uint32_t dex_pc) {
  BBCounts* counts = GetBBCounts();

  int lo = 0;
  int hi = number_of_bb_counts_ - 1;
  while (lo <= hi) {
    int mid = (lo + hi) >> 1;

    uint32_t found_dex_pc = counts[mid].dex_pc_;
    if (dex_pc < found_dex_pc) {
      hi = mid - 1;
    } else if (dex_pc > found_dex_pc) {
      lo = mid + 1;
    } else {
      if (counts[mid].count_ != std::numeric_limits<uint32_t>::max()) {
        counts[mid].count_++;
      }
      return;
    }
  }
  DCHECK(false) << "Unable to locate BB Dex PC: 0x" << std::hex << dex_pc;
}

void ProfilingInfo::LogInformation() {
  VLOG(jit) << "\tLogging information for method";
  VLOG(jit) << "\tBB counts:";

  BBCounts* counts = GetBBCounts();
  for (uint32_t i = 0; i < number_of_bb_counts_; i++) {
    VLOG(jit) << "\t\t" << i << ": 0x"
              << std::hex << counts[i].dex_pc_ << std::dec
              << ", " << counts[i].count_;
  }

  if (number_of_inline_caches_ > 0) {
    VLOG(jit) << "\tInline cache information and counts";
    for (uint32_t i = 0; i < number_of_inline_caches_; i++) {
      const InlineCache& class_info = cache_[i];

      VLOG(jit) << "\t\t" << i << " from dex : 0x" << std::hex << class_info.dex_pc_;
      VLOG(jit) << "\t\t\tMonomorphic: actual: " << std::boolalpha
                << class_info.IsMonomorphic() << " ; basic: " << class_info.IsSkewedToMonomorphic();
      VLOG(jit) << "\t\t\tPolymorphic: actual: " << std::boolalpha
                << class_info.IsPolymorphic() << " ; basic: " << (class_info.IsPolymorphic() && !class_info.IsSkewedToMonomorphic());
      VLOG(jit) << "\t\t\tMegamorphic: " << std::boolalpha << class_info.IsMegamorphic();
      VLOG(jit) << "\t\t\tCounts for each class info: ";

      for (int j = 0; j < InlineCache::kIndividualCacheSize; j++) {
        if (class_info.class_counts_[j] != 0 ) {
          VLOG(jit) << "\t\t\t" << j << ": " << class_info.class_counts_[j];
        }
      }
    }
  }
}

}  // namespace art
