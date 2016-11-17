/*
 * Copyright (C) 2015 Intel Corporation.
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
 */

#include "abi_transition_helper.h"
#include "driver/compiler_driver-inl.h"

namespace art {

void HAbiTransitionHelper::Run() {
  CollectStats();
}

void HAbiTransitionHelper::CollectStats() const {
  if (stats_ == nullptr) {
    // No point in collecting stats if there is no place to aggregate them.
    return;
  }

  const bool cannot_use_dex_cache = CannotUseDexCache();
  for (HReversePostOrderIterator it(*graph_); !it.Done(); it.Advance()) {
    HBasicBlock* block = it.Current();
    for (HInstructionIterator inst_it(block->GetInstructions()); !inst_it.Done();
        inst_it.Advance()) {
      HInstruction* instr = inst_it.Current();

      if (instr->IsCurrentMethod()) {
        DCHECK_EQ(instr->GetEnvUses().SizeSlow(), 0u);
        const HUseList<HInstruction*>& uses = instr->GetUses();
        for (auto it2 = uses.begin(), end = uses.end(); it2 != end; ++it2) {
          HInstruction* user = it2->GetUser();

          switch(user->GetKind()) {
            case HInstruction::kNewInstance:
              MaybeRecordStat(kCurrentMethodUserNewInstance);
              break;
            case HInstruction::kNewArray:
              MaybeRecordStat(kCurrentMethodUserNewArray);
              break;
            case HInstruction::kInvokeStaticOrDirect:
              MaybeRecordStat(kCurrentMethodUserInvoke);
              break;
            case HInstruction::kLoadClass:
              MaybeRecordStat(kCurrentMethodUserLoadClass);
              break;
            case HInstruction::kLoadString:
              MaybeRecordStat(kCurrentMethodUserLoadString);
              break;
            default:
              MaybeRecordStat(kCurrentMethodUserOther);
              break;
          }
        }
      }

      if (instr->IsInvokeStaticOrDirect() && !instr->AsInvoke()->IsIntrinsic()) {
        HInvokeStaticOrDirect::DispatchInfo dispatch_info =
            instr->AsInvokeStaticOrDirect()->GetDispatchInfoUnchecked();
        switch (dispatch_info.method_load_kind) {
          case HInvokeStaticOrDirect::MethodLoadKind::kStringInit:
          case HInvokeStaticOrDirect::MethodLoadKind::kDirectAddress:
          case HInvokeStaticOrDirect::MethodLoadKind::kDirectAddressWithFixup:
            MaybeRecordStat(kMethodLoadViaDirectPtr);
            break;
          case HInvokeStaticOrDirect::MethodLoadKind::kRecursive:
          case HInvokeStaticOrDirect::MethodLoadKind::kDexCacheViaMethod:
            MaybeRecordStat(kMethodLoadViaCurrentMethod);
            break;
          case HInvokeStaticOrDirect::MethodLoadKind::kDexCachePcRelative:
            if (cannot_use_dex_cache) {
              MaybeRecordStat(kMethodLoadViaCurrentMethod);
            } else {
              MaybeRecordStat(kMethodLoadViaDexCache);
            }
            break;
          default:
            // Only fail in debug builds because not being able to report stats is not fatal.
            if (kIsDebugBuild) {
              LOG(FATAL) << "Unhandled method load kind " << dispatch_info.method_load_kind;
            }
            break;
        }
      }
    }
  }
}

bool HAbiTransitionHelper::CannotUseDexCache() const {
  return graph_->HasIrreducibleLoops() && compiler_driver_->GetInstructionSet() == kX86;
}

}  // namespace art
