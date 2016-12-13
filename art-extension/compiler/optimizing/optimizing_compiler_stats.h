/*
 * Copyright (C) 2014 The Android Open Source Project
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

#ifndef ART_COMPILER_OPTIMIZING_OPTIMIZING_COMPILER_STATS_H_
#define ART_COMPILER_OPTIMIZING_OPTIMIZING_COMPILER_STATS_H_

#include <iomanip>
#include <string>
#include <type_traits>

#include "atomic.h"

namespace art {

enum MethodCompilationStat {
  kAttemptCompilation = 0,
  kCompiled,
  kInlinedInvoke,
  kReplacedInvokeWithSimplePattern,
  kInstructionSimplifications,
  kInstructionSimplificationsArch,
  kUnresolvedMethod,
  kUnresolvedField,
  kUnresolvedFieldNotAFastAccess,
  kRemovedCheckedCast,
  kRemovedDeadInstruction,
  kRemovedNullCheck,
  kNotCompiledSkipped,
  kNotCompiledInvalidBytecode,
  kNotCompiledThrowCatchLoop,
  kNotCompiledAmbiguousArrayOp,
  kNotCompiledHugeMethod,
  kNotCompiledLargeMethodNoBranches,
  kNotCompiledMalformedOpcode,
  kNotCompiledNoCodegen,
  kNotCompiledPathological,
  kNotCompiledSpaceFilter,
  kNotCompiledUnhandledInstruction,
  kNotCompiledUnsupportedIsa,
  kNotCompiledVerificationError,
  kNotCompiledVerifyAtRuntime,
  kInlinedMonomorphicCall,
  kInlinedPolymorphicCall,
  kMonomorphicCall,
  kPolymorphicCall,
  kMegamorphicCall,
  kBooleanSimplified,
  kIntrinsicRecognized,
  kLoopInvariantMoved,
  kSelectGenerated,
  kRemovedInstanceOf,
  kInlinedInvokeVirtualOrInterface,
  kImplicitNullCheckGenerated,
  kExplicitNullCheckGenerated,
  kDirectCallOptimized,
  kCurrentMethodUserNewInstance,
  kCurrentMethodUserNewArray,
  kCurrentMethodUserInvoke,
  kCurrentMethodUserLoadClass,
  kCurrentMethodUserLoadString,
  kCurrentMethodUserOther,
  kMethodLoadViaCurrentMethod,
  kMethodLoadViaDexCache,
  kMethodLoadViaDirectPtr,
  kRemovedInstructionViaGVN,
  kIntelBIVFound,
  kIntelRemoveUnusedLoops,
  kIntelLoopPeeled,
  kIntelRemoveTrivialLoops,
  kIntelRemoveSuspendCheck,
  kIntelCCS,
  kIntelNonTemporalMove,
  kIntelFormBottomLoop,
  kIntelLHSS,
  kIntelStoreSink,
  kIntelSelect,
  kIntelValuePropagationThroughHeap,
  kIntelPureDirectCallDeleted,
  kIntelPureStaticCallDeleted,
  kIntelUselessNullCheckDeleted,
  kIntelInvokeMarkedAsPure,
  kIntelLoopFullyUnrolled,
  kIntelRemovedDeadInstructionViaAUR,
  kIntelRemovedDeadReferenceViaAUR,
  kIntelPhiNodeEliminated,
  kIntelDevirtualized,
  kIntelDevirtualizationConsideration,
  kIntelSpeculationHoisted,
  kIntelSpeculationEliminated,
  kIntelCliqueInstructionEliminated,
  kIntelFinalFieldRecognized,
  kIntelCHANotAnalyzed,
  kIntelCHAOneTarget,
  kIntelCHATwoOrMoreTargets,
  kIntelBranchSimplified,
  kIntelBranchConditionDeleted,
  kIntelCommutativeOperationFlipped,
  kLastStat
};

class OptimizingCompilerStats {
 public:
  OptimizingCompilerStats() {}

  void RecordStat(MethodCompilationStat stat, size_t count = 1) {
    compile_stats_[stat] += count;
  }

  void Log() const {
    if (!kIsDebugBuild && !VLOG_IS_ON(compiler)) {
      // Log only in debug builds or if the compiler is verbose.
      return;
    }

    if (compile_stats_[kAttemptCompilation] == 0) {
      LOG(INFO) << "Did not compile any method.";
    } else {
      float compiled_percent =
          compile_stats_[kCompiled] * 100.0f / compile_stats_[kAttemptCompilation];
      LOG(INFO) << "Attempted compilation of " << compile_stats_[kAttemptCompilation]
          << " methods: " << std::fixed << std::setprecision(2)
          << compiled_percent << "% (" << compile_stats_[kCompiled] << ") compiled.";

      for (int i = 0; i < kLastStat; i++) {
        if (compile_stats_[i] != 0) {
          LOG(INFO) << PrintMethodCompilationStat(static_cast<MethodCompilationStat>(i)) << ": "
              << compile_stats_[i];
        }
      }
    }
  }

  void MergeStats(OptimizingCompilerStats& other) {
    for (int i = 0; i < kLastStat; i++) {
      compile_stats_[i] += other.compile_stats_[i];
    }
  }

 private:
  std::string PrintMethodCompilationStat(MethodCompilationStat stat) const {
    std::string name;
    switch (stat) {
      case kAttemptCompilation : name = "AttemptCompilation"; break;
      case kCompiled : name = "Compiled"; break;
      case kInlinedInvoke : name = "InlinedInvoke"; break;
      case kReplacedInvokeWithSimplePattern: name = "ReplacedInvokeWithSimplePattern"; break;
      case kInstructionSimplifications: name = "InstructionSimplifications"; break;
      case kInstructionSimplificationsArch: name = "InstructionSimplificationsArch"; break;
      case kUnresolvedMethod : name = "UnresolvedMethod"; break;
      case kUnresolvedField : name = "UnresolvedField"; break;
      case kUnresolvedFieldNotAFastAccess : name = "UnresolvedFieldNotAFastAccess"; break;
      case kRemovedCheckedCast: name = "RemovedCheckedCast"; break;
      case kRemovedDeadInstruction: name = "RemovedDeadInstruction"; break;
      case kRemovedNullCheck: name = "RemovedNullCheck"; break;
      case kNotCompiledSkipped: name = "NotCompiledSkipped"; break;
      case kNotCompiledInvalidBytecode: name = "NotCompiledInvalidBytecode"; break;
      case kNotCompiledThrowCatchLoop : name = "NotCompiledThrowCatchLoop"; break;
      case kNotCompiledAmbiguousArrayOp : name = "NotCompiledAmbiguousArrayOp"; break;
      case kNotCompiledHugeMethod : name = "NotCompiledHugeMethod"; break;
      case kNotCompiledLargeMethodNoBranches : name = "NotCompiledLargeMethodNoBranches"; break;
      case kNotCompiledMalformedOpcode : name = "NotCompiledMalformedOpcode"; break;
      case kNotCompiledNoCodegen : name = "NotCompiledNoCodegen"; break;
      case kNotCompiledPathological : name = "NotCompiledPathological"; break;
      case kNotCompiledSpaceFilter : name = "NotCompiledSpaceFilter"; break;
      case kNotCompiledUnhandledInstruction : name = "NotCompiledUnhandledInstruction"; break;
      case kNotCompiledUnsupportedIsa : name = "NotCompiledUnsupportedIsa"; break;
      case kNotCompiledVerificationError : name = "NotCompiledVerificationError"; break;
      case kNotCompiledVerifyAtRuntime : name = "NotCompiledVerifyAtRuntime"; break;
      case kInlinedMonomorphicCall: name = "InlinedMonomorphicCall"; break;
      case kInlinedPolymorphicCall: name = "InlinedPolymorphicCall"; break;
      case kMonomorphicCall: name = "MonomorphicCall"; break;
      case kPolymorphicCall: name = "PolymorphicCall"; break;
      case kMegamorphicCall: name = "MegamorphicCall"; break;
      case kBooleanSimplified : name = "BooleanSimplified"; break;
      case kIntrinsicRecognized : name = "IntrinsicRecognized"; break;
      case kLoopInvariantMoved : name = "LoopInvariantMoved"; break;
      case kSelectGenerated : name = "SelectGenerated"; break;
      case kRemovedInstanceOf: name = "RemovedInstanceOf"; break;
      case kInlinedInvokeVirtualOrInterface: name = "InlinedInvokeVirtualOrInterface"; break;
      case kImplicitNullCheckGenerated: name = "ImplicitNullCheckGenerated"; break;
      case kExplicitNullCheckGenerated: name = "ExplicitNullCheckGenerated"; break;
      case kDirectCallOptimized : name = "DirectCallOptimized"; break;
      case kCurrentMethodUserNewInstance : name = "CurrentMethodUserNewInstance"; break;
      case kCurrentMethodUserNewArray : name = "CurrentMethodUserNewArray"; break;
      case kCurrentMethodUserInvoke : name = "CurrentMethodUserInvoke"; break;
      case kCurrentMethodUserLoadClass : name = "CurrentMethodUserLoadClass"; break;
      case kCurrentMethodUserLoadString : name = "CurrentMethodUserLoadString"; break;
      case kCurrentMethodUserOther : name = "CurrentMethodUserOther"; break;
      case kMethodLoadViaCurrentMethod : name = "MethodLoadViaCurrentMethod"; break;
      case kMethodLoadViaDexCache : name = "MethodLoadViaDexCache"; break;
      case kMethodLoadViaDirectPtr : name = "MethodLoadViaDirectPtr"; break;
      case kRemovedInstructionViaGVN : name = "RemovedInstructionViaGVN"; break;
      case kIntelBIVFound: return "kIntelBIVFound";
      case kIntelRemoveUnusedLoops: return "kIntelRemoveUnusedLoops";
      case kIntelLoopPeeled: return "kIntelLoopPeeled";
      case kIntelRemoveTrivialLoops: return "kIntelRemoveTrivialLoops";
      case kIntelRemoveSuspendCheck: return "kIntelRemoveSuspendCheck";
      case kIntelCCS: return "kIntelCCS";
      case kIntelNonTemporalMove: return "kIntelNonTemporalMove";
      case kIntelFormBottomLoop: return "kIntelFormBottomLoop";
      case kIntelLHSS: return "kIntelLHSS";
      case kIntelStoreSink: return "kIntelStoreSink";
      case kIntelSelect: return "kIntelSelect";
      case kIntelValuePropagationThroughHeap: return "kIntelValuePropagationThroughHeap";
      case kIntelPureDirectCallDeleted: return "kIntelPureDirectCallDeleted";
      case kIntelPureStaticCallDeleted: return "kIntelPureStaticCallDeleted";
      case kIntelUselessNullCheckDeleted: return "kIntelUselessNullCheckDeleted";
      case kIntelInvokeMarkedAsPure: return "IntelInvokeMarkedAsPure";
      case kIntelLoopFullyUnrolled: return "kIntelLoopFullyUnrolled";
      case kIntelRemovedDeadInstructionViaAUR: return "kIntelRemovedDeadInstructionViaAUR";
      case kIntelRemovedDeadReferenceViaAUR: return "kIntelRemovedDeadReferenceViaAUR";
      case kIntelPhiNodeEliminated: return "kIntelPhiNodeEliminated";
      case kIntelDevirtualized: return "IntelDevirtualized";
      case kIntelDevirtualizationConsideration: return "IntelConsideredForDevirtualization";
      case kIntelSpeculationHoisted: return "IntelSpeculationHoisted";
      case kIntelSpeculationEliminated: return "IntelSpeculationEliminated";
      case kIntelCliqueInstructionEliminated: return "kIntelCliqueInstructionEliminated";
      case kIntelFinalFieldRecognized: return "IntelFinalFieldRecognized";
      case kIntelCHANotAnalyzed: return "kIntelCHANotAnalyzed";
      case kIntelCHAOneTarget: return "kIntelCHAOneTarget";
      case kIntelCHATwoOrMoreTargets: return "kIntelCHATwoOrMoreTargets";
      case kIntelBranchSimplified: return "kIntelBranchSimplified";
      case kIntelBranchConditionDeleted: return "kIntelBranchConditionDeleted";
      case kIntelCommutativeOperationFlipped: return "kIntelCommutativeOperationFlipped";
      default: LOG(FATAL) << "invalid stat";
    }
    return "OptStat#" + name;
  }

  AtomicInteger compile_stats_[kLastStat];

  DISALLOW_COPY_AND_ASSIGN(OptimizingCompilerStats);
};

}  // namespace art

#endif  // ART_COMPILER_OPTIMIZING_OPTIMIZING_COMPILER_STATS_H_
