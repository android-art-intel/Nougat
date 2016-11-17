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

#include "ext_utility.h"
#include "loop_iterators.h"
#include "pure_invokes_analysis.h"

namespace art {
const SideEffects HPureInvokesAnalysis::se_none_ = SideEffects::None();
const SideEffects HPureInvokesAnalysis::se_memory_read_ = SideEffects::AllReads();
const SideEffects HPureInvokesAnalysis::se_memory_read_and_alloc_ =
    SideEffects::AllReads().Union(SideEffects::CanTriggerGC());

HPureInvokesAnalysis::PureMethodSignature HPureInvokesAnalysis::whitelist_[] = {
  { "bool java.lang.String.isEmpty()", HPureInvokesAnalysis::se_none_ },
  { "int java.lang.String.indexOf(int)", HPureInvokesAnalysis::se_none_ },
  { "int java.lang.String.indexOf(int, int)", HPureInvokesAnalysis::se_none_ },
  { "int java.lang.String.indexOf(java.lang.String)", HPureInvokesAnalysis::se_none_ },
  { "int java.lang.String.indexOf(java.lang.String, int)", HPureInvokesAnalysis::se_none_ },
  { "int java.lang.String.lastIndexOf(int)", HPureInvokesAnalysis::se_none_ },
  { "int java.lang.String.lastIndexOf(int, int)", HPureInvokesAnalysis::se_none_ },
  { "int java.lang.String.lastIndexOf(java.lang.String)", HPureInvokesAnalysis::se_none_ },
  { "int java.lang.String.lastIndexOf(java.lang.String, int)", HPureInvokesAnalysis::se_none_ },
  { "int java.lang.String.length()", HPureInvokesAnalysis::se_none_ },
  { "java.lang.String java.lang.String.toString()", HPureInvokesAnalysis::se_memory_read_and_alloc_ },
  // StringBuffer methods.
  { "bool java.lang.StringBuffer.isEmpty()", HPureInvokesAnalysis::se_memory_read_ },
  { "int java.lang.StringBuffer.indexOf(int)", HPureInvokesAnalysis::se_memory_read_ },
  { "int java.lang.StringBuffer.indexOf(int, int)", HPureInvokesAnalysis::se_memory_read_ },
  { "int java.lang.StringBuffer.indexOf(java.lang.String)", HPureInvokesAnalysis::se_memory_read_ },
  { "int java.lang.StringBuffer.indexOf(java.lang.String, int)", HPureInvokesAnalysis::se_memory_read_ },
  { "int java.lang.StringBuffer.lastIndexOf(int)", HPureInvokesAnalysis::se_memory_read_ },
  { "int java.lang.StringBuffer.lastIndexOf(int, int)", HPureInvokesAnalysis::se_memory_read_ },
  { "int java.lang.StringBuffer.lastIndexOf(java.lang.String)", HPureInvokesAnalysis::se_memory_read_ },
  { "int java.lang.StringBuffer.lastIndexOf(java.lang.String, int)", HPureInvokesAnalysis::se_memory_read_ },
  { "int java.lang.StringBuffer.length()", HPureInvokesAnalysis::se_memory_read_ },
  { "java.lang.String java.lang.StringBuffer.toString()", HPureInvokesAnalysis::se_memory_read_and_alloc_ },
  // StringBuilder methods.
  { "bool java.lang.StringBuilder.isEmpty()", HPureInvokesAnalysis::se_memory_read_ },
  { "int java.lang.StringBuilder.indexOf(int)", HPureInvokesAnalysis::se_memory_read_ },
  { "int java.lang.StringBuilder.indexOf(int, int)", HPureInvokesAnalysis::se_memory_read_ },
  { "int java.lang.StringBuilder.indexOf(java.lang.String)", HPureInvokesAnalysis::se_memory_read_ },
  { "int java.lang.StringBuilder.indexOf(java.lang.String, int)", HPureInvokesAnalysis::se_memory_read_ },
  { "int java.lang.StringBuilder.lastIndexOf(int)", HPureInvokesAnalysis::se_memory_read_ },
  { "int java.lang.StringBuilder.lastIndexOf(int, int)", HPureInvokesAnalysis::se_memory_read_ },
  { "int java.lang.StringBuilder.lastIndexOf(java.lang.String)", HPureInvokesAnalysis::se_memory_read_ },
  { "int java.lang.StringBuilder.lastIndexOf(java.lang.String, int)", HPureInvokesAnalysis::se_memory_read_ },
  { "int java.lang.StringBuilder.length()", HPureInvokesAnalysis::se_memory_read_ },
  { "java.lang.String java.lang.StringBuilder.toString()", HPureInvokesAnalysis::se_memory_read_and_alloc_ },
  // Math methods.
  { "int java.lang.Math.abs(int)", HPureInvokesAnalysis::se_none_ },
  { "long java.lang.Math.abs(long)", HPureInvokesAnalysis::se_none_ },
  { "float java.lang.Math.abs(float)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.abs(double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.asin(double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.acos(double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.atan(double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.atan2(double, double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.cbrt(double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.ceil(double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.copySign(double, double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.cos(double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.cosh(double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.exp(double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.expm1(double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.floor(double)", HPureInvokesAnalysis::se_none_ },
  { "int java.lang.Math.getExponent(double)", HPureInvokesAnalysis::se_none_ },
  { "int java.lang.Math.getExponent(float)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.hypot(double, double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.log(double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.log10(double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.max(double, double)", HPureInvokesAnalysis::se_none_ },
  { "float java.lang.Math.max(float, float)", HPureInvokesAnalysis::se_none_ },
  { "int java.lang.Math.max(int, int)", HPureInvokesAnalysis::se_none_ },
  { "long java.lang.Math.max(long, long)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.min(double, double)", HPureInvokesAnalysis::se_none_ },
  { "float java.lang.Math.min(float, float)", HPureInvokesAnalysis::se_none_ },
  { "int java.lang.Math.min(int, int)", HPureInvokesAnalysis::se_none_ },
  { "long java.lang.Math.min(long, long)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.nextAfter(double, double)", HPureInvokesAnalysis::se_none_ },
  { "float java.lang.Math.nextAfter(float, double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.nextUp(double)", HPureInvokesAnalysis::se_none_ },
  { "float java.lang.Math.nextUp(float)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.pow(double, double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.rint(double)", HPureInvokesAnalysis::se_none_ },
  { "long java.lang.Math.round(double)", HPureInvokesAnalysis::se_none_ },
  { "int java.lang.Math.round(float)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.scalb(double, int)", HPureInvokesAnalysis::se_none_ },
  { "float java.lang.Math.scalb(float, int)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.signum(double)", HPureInvokesAnalysis::se_none_ },
  { "float java.lang.Math.signum(float)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.sin(double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.sinh(double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.sqrt(double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.tan(double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.tanh(double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.toDigrees(double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.toRadians(double)", HPureInvokesAnalysis::se_none_ },
  { "double java.lang.Math.ulp(double)", HPureInvokesAnalysis::se_none_ },
  { "float java.lang.Math.ulp(float)", HPureInvokesAnalysis::se_none_ },
};

const char* HPureInvokesAnalysis::never_returns_null_[] = {
  // String methods.
  "java.lang.String java.lang.String.concat(java.lang.String)",
  "java.lang.String java.lang.String.intern()",
  "java.lang.String java.lang.String.replace(char, char)",
  "java.lang.String java.lang.String.replace(java.lang.CharSequence, java.lang.CharSequence)",
  "java.lang.String java.lang.String.replaceAll(java.lang.String, java.lang.String)",
  "java.lang.String java.lang.String.replaceFirst(java.lang.String, java.lang.String)",
  "java.lang.String[] java.lang.String.split(java.lang.String)",
  "java.lang.String[] java.lang.String.split(java.lang.String, int)",
  "java.lang.CharSequence java.lang.String.subSequence(int, int)",
  "java.lang.String java.lang.String.substring(int)",
  "java.lang.String java.lang.String.substring(int, int)",
  "char[] java.lang.String.toCharArray()",
  "java.lang.String java.lang.String.toLowerCase()",
  "java.lang.String java.lang.String.toLowerCase(java.util.Locale)",
  "java.lang.String java.lang.String.toString()",
  "java.lang.String java.lang.String.toUpperCase()",
  "java.lang.String java.lang.String.toUpperCase(java.util.Locale)",
  "java.lang.String java.lang.String.trim()",
  // StringBuffer methods.
  "java.lang.StringBuffer java.lang.StringBuffer.append(boolean)",
  "java.lang.StringBuffer java.lang.StringBuffer.append(char)",
  "java.lang.StringBuffer java.lang.StringBuffer.append(char[])",
  "java.lang.StringBuffer java.lang.StringBuffer.append(char[], int, int)",
  "java.lang.StringBuffer java.lang.StringBuffer.append(java.lang.CharSequence)",
  "java.lang.StringBuffer java.lang.StringBuffer.append(java.lang.CharSequence, int, int)",
  "java.lang.StringBuffer java.lang.StringBuffer.append(double)",
  "java.lang.StringBuffer java.lang.StringBuffer.append(float)",
  "java.lang.StringBuffer java.lang.StringBuffer.append(int)",
  "java.lang.StringBuffer java.lang.StringBuffer.append(long)",
  "java.lang.StringBuffer java.lang.StringBuffer.append(java.lang.Object)",
  "java.lang.StringBuffer java.lang.StringBuffer.append(java.lang.String)",
  "java.lang.StringBuffer java.lang.StringBuffer.append(java.lang.StringBuffer)",
  "java.lang.StringBuffer java.lang.StringBuffer.appendCodePoint(int)",
  "java.lang.StringBuffer java.lang.StringBuffer.delete(int, int)",
  "java.lang.StringBuffer java.lang.StringBuffer.deleteCharAt(int)",
  "java.lang.StringBuffer java.lang.StringBuffer.deleteCharAt(int)",
  "java.lang.StringBuffer java.lang.StringBuffer.insert(int, boolean)",
  "java.lang.StringBuffer java.lang.StringBuffer.insert(int, char)",
  "java.lang.StringBuffer java.lang.StringBuffer.insert(int, char[])",
  "java.lang.StringBuffer java.lang.StringBuffer.insert(int, char[], int, int)",
  "java.lang.StringBuffer java.lang.StringBuffer.insert(int, java.lang.CharSequence)",
  "java.lang.StringBuffer java.lang.StringBuffer.insert(int, java.lang.CharSequence, int, int)",
  "java.lang.StringBuffer java.lang.StringBuffer.insert(int, double)",
  "java.lang.StringBuffer java.lang.StringBuffer.insert(int, float)",
  "java.lang.StringBuffer java.lang.StringBuffer.insert(int, int)",
  "java.lang.StringBuffer java.lang.StringBuffer.insert(int, long)",
  "java.lang.StringBuffer java.lang.StringBuffer.insert(int, java.lang.Object)",
  "java.lang.StringBuffer java.lang.StringBuffer.insert(int, java.lang.String)",
  "java.lang.StringBuffer java.lang.StringBuffer.replace(int, int, java.lang.String)",
  "java.lang.StringBuffer java.lang.StringBuffer.reverse()",
  "java.lang.CharSequence java.lang.StringBuffer.subSequence(int, int)",
  "java.lang.String java.lang.StringBuffer.substring(int)",
  "java.lang.String java.lang.StringBuffer.substring(int, int)",
  "java.lang.String java.lang.StringBuffer.toString()",
  // StringBuilder methods.
  "java.lang.StringBuilder java.lang.StringBuilder.append(boolean)",
  "java.lang.StringBuilder java.lang.StringBuilder.append(char)",
  "java.lang.StringBuilder java.lang.StringBuilder.append(char[])",
  "java.lang.StringBuilder java.lang.StringBuilder.append(char[], int, int)",
  "java.lang.StringBuilder java.lang.StringBuilder.append(java.lang.CharSequence)",
  "java.lang.StringBuilder java.lang.StringBuilder.append(java.lang.CharSequence, int, int)",
  "java.lang.StringBuilder java.lang.StringBuilder.append(double)",
  "java.lang.StringBuilder java.lang.StringBuilder.append(float)",
  "java.lang.StringBuilder java.lang.StringBuilder.append(int)",
  "java.lang.StringBuilder java.lang.StringBuilder.append(long)",
  "java.lang.StringBuilder java.lang.StringBuilder.append(java.lang.Object)",
  "java.lang.StringBuilder java.lang.StringBuilder.append(java.lang.String)",
  "java.lang.StringBuilder java.lang.StringBuilder.append(java.lang.StringBuffer)",
  "java.lang.StringBuilder java.lang.StringBuilder.appendCodePoint(int)",
  "java.lang.StringBuilder java.lang.StringBuilder.delete(int, int)",
  "java.lang.StringBuilder java.lang.StringBuilder.deleteCharAt(int)",
  "java.lang.StringBuilder java.lang.StringBuilder.deleteCharAt(int)",
  "java.lang.StringBuilder java.lang.StringBuilder.insert(int, boolean)",
  "java.lang.StringBuilder java.lang.StringBuilder.insert(int, char)",
  "java.lang.StringBuilder java.lang.StringBuilder.insert(int, char[])",
  "java.lang.StringBuilder java.lang.StringBuilder.insert(int, char[], int, int)",
  "java.lang.StringBuilder java.lang.StringBuilder.insert(int, java.lang.CharSequence)",
  "java.lang.StringBuilder java.lang.StringBuilder.insert(int, java.lang.CharSequence, int, int)",
  "java.lang.StringBuilder java.lang.StringBuilder.insert(int, double)",
  "java.lang.StringBuilder java.lang.StringBuilder.insert(int, float)",
  "java.lang.StringBuilder java.lang.StringBuilder.insert(int, int)",
  "java.lang.StringBuilder java.lang.StringBuilder.insert(int, long)",
  "java.lang.StringBuilder java.lang.StringBuilder.insert(int, java.lang.Object)",
  "java.lang.StringBuilder java.lang.StringBuilder.insert(int, java.lang.String)",
  "java.lang.StringBuilder java.lang.StringBuilder.replace(int, int, java.lang.String)",
  "java.lang.StringBuilder java.lang.StringBuilder.reverse()",
  "java.lang.CharSequence java.lang.StringBuilder.subSequence(int, int)",
  "java.lang.String java.lang.StringBuilder.substring(int)",
  "java.lang.String java.lang.StringBuilder.substring(int, int)",
  "java.lang.String java.lang.StringBuilder.toString()",
};

void HPureInvokesAnalysis::Run() {
  // Motion and removal of invokes may break the debugability.
  if (graph_->IsDebuggable()) {
    return;
  }
  // Reduce the scope to method with loops only to not hurt compile time.
  HOnlyInnerLoopIterator loop_iter(GetGraphX86()->GetLoopInformation());
  if (loop_iter.Done()) {
    // TODO: This limitation can be removed when the purity checks gets cheaper.
    PRINT_PASS_OSTREAM_MESSAGE(this, "Skip the method " << GetMethodName(graph_)
                                     << " because it has no loops");
    return;
  }
  PRINT_PASS_OSTREAM_MESSAGE(this, "Start " << GetMethodName(graph_));

  ProcessPureInvokes();

  PRINT_PASS_OSTREAM_MESSAGE(this, "End " << GetMethodName(graph_));
}

bool HPureInvokesAnalysis::CanReturnNull(HInstruction* insn) {
  if (insn->IsNewInstance() || insn->IsNewArray()) {
    // Constructors never return null.
    return false;
  }

  if (insn->IsNullCheck()) {
    // Only if we proved that the arg of the NullCheck is not null.
    return CanReturnNull(insn->InputAt(0));
  }

  if (insn->IsInvokeStaticOrDirect()) {
    // Check the non-null method whitelist for this.
    return IsInvokeThatCanReturnNull(insn->AsInvokeStaticOrDirect());
  }

  // We can guarantee nothing for other instructions.
  return true;
}

bool HPureInvokesAnalysis::IsPureMethodInvoke(HInvokeStaticOrDirect* call) {
  DCHECK(call != nullptr);
  const MethodReference target_method = call->GetTargetMethod();
  // Do we already know the answer for this method?
  auto it_pure = pure_invokes_.find(target_method);
  if (it_pure != pure_invokes_.end()) {
    if (it_pure->second.is_pure) {
      // Mark call as pure.
      call->SetPure();
      call->SetSideEffects(it_pure->second.se);
    }
    return it_pure->second.is_pure;
  }

  const size_t len = arraysize(whitelist_);
  const std::string method_name = PrettyMethod(target_method.dex_method_index,
                                               *target_method.dex_file);
  const char* char_name = method_name.c_str();
  for (size_t i = 0; i < len; i++) {
    if (strcmp(char_name, whitelist_[i].method_name) == 0) {
        // Remember that this one is pure.
        MethodPurityInfo pure = { true, whitelist_[i].se };
        pure_invokes_.Put(target_method, pure);
        // Mark call as pure.
        call->SetPure();
        call->SetSideEffects(whitelist_[i].se);
        return true;
    }
  }

  MethodPurityInfo not_pure = { false, call->GetSideEffects() };
  // Remember that this one is not pure.
  pure_invokes_.Put(target_method, not_pure);
  return false;
}

bool HPureInvokesAnalysis::IsInvokeThatCanReturnNull(HInvokeStaticOrDirect* call) {
  DCHECK(call != nullptr);
  const MethodReference target_method = call->GetTargetMethod();
  // Do we already know the answer for this method?
  auto it_null = null_invokes_.find(target_method);
  if (it_null != null_invokes_.end()) {
    return it_null->second;
  }

  // We assume that all invokes except the never_returns_null_ can return null.
  const size_t len = arraysize(never_returns_null_);
  const std::string method_name = PrettyMethod(target_method.dex_method_index,
                                               *target_method.dex_file);
  const char* char_name = method_name.c_str();
  for (size_t i = 0; i < len; i++) {
    if (strcmp(char_name, never_returns_null_[i]) == 0) {
        null_invokes_.Put(target_method, false);
        return false;
    }
  }
  null_invokes_.Put(target_method, true);
  return true;
}

void HPureInvokesAnalysis::ProcessPureInvokes() {
  for (HPostOrderIterator block_iter(*graph_); !block_iter.Done(); block_iter.Advance()) {
    HBasicBlock* block = block_iter.Current();

    for (HBackwardInstructionIterator insn_iter(block->GetInstructions());
         !insn_iter.Done(); insn_iter.Advance()) {
      HInstruction* insn = insn_iter.Current();
      // Paranoid.
      DCHECK(insn != nullptr);

      if (insn->IsNullCheck()) {
        if (!CanReturnNull(insn->InputAt(0))) {
          insn->ReplaceWith(insn->InputAt(0));
          PRINT_PASS_OSTREAM_MESSAGE(this, "Eliminated nullcheck " << insn
                                           << " because its argument is guaranteed to be not null");
          MaybeRecordStat(MethodCompilationStat::kIntelUselessNullCheckDeleted);
          block->RemoveInstruction(insn);
        }
        continue;
      }

      auto call = insn->AsInvokeStaticOrDirect();
      if (call == nullptr) {
        // We are interested in static/direct invokes only.
        continue;
      }

      if (!IsPureMethodInvoke(call)) {
        // We can optimize away only invokes with no side effects.
        continue;
      }

      if (call->IsStatic()) {
        // For static pure invoke, it is enough to check that the result is unused.
        if (!call->HasUses()) {
          PRINT_PASS_OSTREAM_MESSAGE(this, "Eliminated static invoke " << call
                                           << " of pure method "
                                           << CalledMethodName(call));
          MaybeRecordStat(MethodCompilationStat::kIntelPureStaticCallDeleted);
          TryKillUseTree(this, call);
          DCHECK(call->GetBlock() == nullptr) << call << " was not removed as expected!";
          continue;
        }
      } else {
        // The argument should not be null to prove that this invoke cannot throw
        // NullPointerException.
        HInstruction* callee_object = call->InputAt(0);

        if (!CanReturnNull(callee_object) && !call->HasUses()) {
          PRINT_PASS_OSTREAM_MESSAGE(this, "Eliminated direct invoke " << call
                                           << " of pure method "
                                           << CalledMethodName(call)
                                           << " because it is called for not-null object "
                                           << callee_object);
          MaybeRecordStat(MethodCompilationStat::kIntelPureDirectCallDeleted);
          TryKillUseTree(this, call);
          DCHECK(call->GetBlock() == nullptr) << call << " was not removed as expected!";
          continue;
        }
      }

      // We have marked it as pure during the check. Since it was not
      // removed, report here that it was marked.
      PRINT_PASS_OSTREAM_MESSAGE(this, "Marked as pure: " << call);
      MaybeRecordStat(MethodCompilationStat::kIntelInvokeMarkedAsPure);
    }
  }
}

}  // namespace art
