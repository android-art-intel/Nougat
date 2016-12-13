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
#include "aur.h"
#include "base/dumpable.h"
#include "base/timing_logger.h"
#include "bb_simplifier.h"
#include "code_generator.h"
#include "commutative_trees_flipper.h"
#include "constant_calculation_sinking.h"
#include "constant_folding_x86.h"
#include "devirtualization.h"
#include "ext_utility.h"
#include "driver/compiler_driver.h"
#include "ext_utility.h"
#include "form_bottom_loops.h"
#include "find_ivs.h"
#include "graph_visualizer.h"
#include "gvn_after_fbl.h"
#include "inliner.h"
#include "loadhoist_storesink.h"
#include "loop_formation.h"
#include "loop_full_unrolling.h"
#ifndef SOFIA
#include "non_temporal_move.h"
#endif
#include "optimization.h"
#include "optimizing_compiler_stats.h"
#include "osr_graph_rebuilder.h"
#include "pass_framework.h"
#include "peeling.h"
#include "phi_cleanup.h"
#include "pure_invokes_analysis.h"
#include "remove_suspend.h"
#include "remove_unused_loops.h"
#include "scoped_thread_state_change.h"
#include "sharpening_wrapper.h"
#include "speculation_pass.h"
#include "thread.h"
#include "trivial_loop_evaluator.h"
#include "value_propagation_through_heap.h"

namespace art {

// Enumeration defining possible commands to be applied to each pass.
enum PassInstrumentation {
  kPassInsertBefore,
  kPassInsertAfter,
  kPassReplace,
  kPassAppend,
  kPassPrepend,
};

/**
 * @brief Used to provide directive for custom pass placement.
 */
struct HCustomPassPlacement {
  const char* pass_to_place;      /**!< The pass that has a custom location. */
  const char* pass_relative_to;   /**!< The pass for which the new pass is relative to. */
  PassInstrumentation directive;  /**!< How to do the pass insert (before, after, etc). */
};

/**
 * @brief Static array holding information about custom placements.
 */
static HCustomPassPlacement kPassCustomPlacement[] = {
  // Devirtualization is always inserted before sharpening. We also insert it twice to increase
  // its effectiveness - before and after inlining.
  { HDevirtualization::kDevirtualizationPassName,
    HSharpening::kSharpeningPassName,
    kPassInsertBefore },
  { HDevirtualization::kDevirtualizationAfterInliningPassName,
    HInliner::kInlinerPassName,
    kPassInsertAfter },
  { HSharpeningWrapper::kSharpeningAfterInliningPassName,
    HDevirtualization::kDevirtualizationAfterInliningPassName,
    kPassInsertAfter },
  { "loop_peeling", "select_generator", kPassInsertBefore },
  // We apply pure invoke analysis to methods with loops only.
  // If this restriction is removed, it may go right after the
  // sharpening after inlining.
  { "pure_invokes_analysis", "loop_peeling", kPassInsertBefore },
  { "value_propagation_through_heap", "pure_invokes_analysis", kPassInsertBefore },
  { "osr_graph_rebuilder", "value_propagation_through_heap", kPassInsertBefore },
  { "loop_formation_before_peeling", "osr_graph_rebuilder", kPassInsertBefore },
  { "form_bottom_loops", "load_store_elimination", kPassInsertAfter },
  { "loop_formation_before_bottom_loops", "form_bottom_loops", kPassInsertBefore },
  { "GVN_after_form_bottom_loops", "form_bottom_loops", kPassInsertAfter },
  { "phi_cleanup", "GVN_after_form_bottom_loops", kPassInsertAfter },
  { "constant_folding_after_phi_cleanup", "phi_cleanup", kPassInsertAfter },
  { "loop_formation", "constant_folding_after_phi_cleanup", kPassInsertAfter },
  { "find_ivs", "loop_formation", kPassInsertAfter },
  { "trivial_loop_evaluator", "find_ivs", kPassInsertAfter },
  { "non_temporal_move", "trivial_loop_evaluator", kPassInsertAfter },
  { "constant_calculation_sinking", "non_temporal_move", kPassInsertAfter },
  { "remove_loop_suspend_checks", "constant_calculation_sinking", kPassInsertAfter },
  { "loadhoist_storesink", "remove_loop_suspend_checks", kPassInsertAfter },
  { "remove_unused_loops", "loadhoist_storesink", kPassInsertAfter },
  { "loop_full_unrolling", "remove_unused_loops", kPassInsertAfter },
  { "load_store_elimination", "value_propagation_through_heap", kPassInsertBefore },
  { "bb_simplifier", "remove_unused_loops", kPassInsertBefore },
  { "aur", "bb_simplifier", kPassInsertBefore },
  { "phi_cleanup_after_aur", "aur", kPassInsertAfter },
  { "commutative_trees_flipper", "dead_code_elimination_final", kPassInsertAfter },
  { "abi_transition_helper", "", kPassAppend },
};

/**
 * @brief Static array holding names of passes that need removed.
 * @details This is done in cases where common code pass ordering and
 * existing passes are not appropriate or compatible with extension.
 */
static const char* kPassRemoval[] = {
  nullptr,
};

static void AddX86Optimization(HOptimization* optimization,
                               ArenaVector<HOptimization*>& list,
                               ArenaSafeMap<const char*, HCustomPassPlacement*> &placements) {
  ArenaSafeMap<const char*, HCustomPassPlacement*>::iterator iter = placements.find(optimization->GetPassName());

  if (iter == placements.end()) {
    return;
  }

  HCustomPassPlacement* placement = iter->second;

  if (placement->directive == kPassAppend) {
    list.push_back(optimization);
    return;
  } else if (placement->directive == kPassPrepend) {
    list.insert(list.begin(), optimization);
    return;
  }

  // Find the right pass to change now.
  size_t len = list.size();
  size_t idx;
  for (idx = 0; idx < len; idx++) {
    if (strcmp(list[idx]->GetPassName(), placement->pass_relative_to) == 0) {
      switch (placement->directive) {
        case kPassReplace:
          list[idx] = optimization;
          break;
        case kPassInsertBefore:
        case kPassInsertAfter: {
          // Add an empty element.
          list.push_back(nullptr);

          // Find the start, is it idx or idx + 1?
          size_t start = idx;

          if (placement->directive == kPassInsertAfter) {
            start++;
          }

          // Push elements backwards.
          DCHECK_NE(len, list.size());
          for (size_t idx2 = len; idx2 >= start; idx2--) {
            list[idx2] = list[idx2 - 1];
          }

          // Place the new element.
          list[start] = optimization;
          break;
        }
        default:
          if (kIsDebugBuild) {
            LOG(FATAL) << "Unexpected placement directive << " << placement->directive;
          }
          break;
      }
      // Done here.
      break;
    }
  }
  // It must be the case that the custom placement was found.
  DCHECK_NE(len, idx) << "couldn't insert " << optimization->GetPassName() << " relative to " << placement->pass_relative_to;
}

static void FillCustomPlacement(ArenaSafeMap<const char*, HCustomPassPlacement*>& placements) {
  size_t len = arraysize(kPassCustomPlacement);

  for (size_t i = 0; i < len; i++) {
    placements.Overwrite(kPassCustomPlacement[i].pass_to_place, kPassCustomPlacement + i);
  }
}

static void FillOptimizationList(HGraph* graph,
                                 ArenaVector<HOptimization*>& list,
                                 HOptimization_X86* optimizations_x86[],
                                 size_t opts_x86_length) {
  // Get the custom placements for our passes.
  ArenaSafeMap<const char*, HCustomPassPlacement*> custom_placement(
      std::less<const char*>(),
      graph->GetArena()->Adapter(kArenaAllocMisc));
  FillCustomPlacement(custom_placement);

  for (size_t i = 0; i < opts_x86_length; i++) {
    HOptimization_X86* opt = optimizations_x86[i];
    if (opt != nullptr) {
      AddX86Optimization(opt, list, custom_placement);
    }
  }
}

/**
 * @brief Remove the passes in the optimization list.
 * @param opts the optimization vector.
 * @param driver the compilation driver.
 */
static void RemoveOptimizations(ArenaVector<HOptimization*>& opts,
                                CompilerDriver* driver) {
  std::unordered_set<std::string> disabled_passes;

  SplitStringIntoSet(driver->GetCompilerOptions().
                       GetPassManagerOptions()->GetDisablePassList(),
                     ',',
                     disabled_passes);

  // Add elements from kPassRemoval.
  for (size_t i = 0, len = arraysize(kPassRemoval); i < len; i++) {
    if (kPassRemoval[i] != nullptr) {
      disabled_passes.insert(std::string(kPassRemoval[i]));
    }
  }

  // If there are no disabled passes, bail.
  if (disabled_passes.empty()) {
    return;
  }

  size_t opts_len = opts.size();

  // We replace the opts with nullptr if we find a match.
  //   This is cheaper than rearranging the vectors.
  for (size_t opts_idx = 0; opts_idx < opts_len; opts_idx++) {
    HOptimization* opt = opts[opts_idx];
    if (disabled_passes.find(opt->GetPassName()) != disabled_passes.end()) {
      opts[opts_idx] = nullptr;
    }
  }
}

void PrintPasses(ArenaVector<HOptimization*>& opts) {
  size_t opts_len = opts.size();

  // We replace the opts with nullptr if we find a match.
  //   This is cheaper than rearranging the vectors.
  LOG(INFO) << "Pass List:";
  if (opts_len == 0) {
    LOG(INFO) << "\t<Empty>";
  }

  for (size_t opts_idx = 0; opts_idx < opts_len; opts_idx++) {
    HOptimization* opt = opts[opts_idx];
    if (opt != nullptr) {
      LOG(INFO) << "\t- " << opt->GetPassName();
    }
  }
}

bool PrintPassesOnlyOnce(ArenaVector<HOptimization*>& opts,
                         CompilerDriver* driver) {
  bool need_print = driver->GetCompilerOptions().
                            GetPassManagerOptions()->GetPrintPassNames();

  if (!need_print) {
    return false;
  }

  // Flags that we have already printed the pass name list.
  static volatile bool pass_names_already_printed_ = false;

  // Have we already printed the names?
  if (!pass_names_already_printed_) {
    // Double-check it under the lock.
    ScopedObjectAccess soa(Thread::Current());
    if (!pass_names_already_printed_) {
      pass_names_already_printed_ = true;
    } else {
      need_print = false;
    }
  } else {
    need_print = false;
  }

  if (!need_print) {
    return false;
  }

  PrintPasses(opts);
  return true;
}

/**
 * @brief Sets verbosity for passes.
 * @param optimizations the optimization array.
 * @param opts_len the length of optimizations array.
 * @param driver the compilation driver.
 */
void FillVerbose(HOptimization_X86* optimizations[],
                 size_t opts_len,
                 CompilerDriver* driver) {
  std::unordered_set<std::string> print_passes;
  const bool print_all_passes = driver->GetCompilerOptions().
                                GetPassManagerOptions()->GetPrintAllPasses();
  if (!print_all_passes) {
    // If we don't print all passes, we need to check the list.
    SplitStringIntoSet(driver->GetCompilerOptions().
                         GetPassManagerOptions()->GetPrintPassList(),
                       ',',
                       print_passes);

    // Are there any passes to print?
    if (print_passes.empty()) {
      return;
    }
  }

  for (size_t opts_idx = 0; opts_idx < opts_len; opts_idx++) {
    HOptimization* opt = optimizations[opts_idx];
    if (opt != nullptr) {
      if (print_all_passes ||
          print_passes.find(opt->GetPassName()) != print_passes.end()) {
        optimizations[opts_idx]->SetVerbose(true);
      }
    }
  }
}

void RunOptimizationsX86(HGraph* graph,
                         CodeGenerator* codegen,
                         CompilerDriver* driver,
                         OptimizingCompilerStats* stats,
                         ArenaVector<HOptimization*>& opt_list,
                         const DexCompilationUnit& dex_compilation_unit,
                         PassObserver* pass_observer,
                         StackHandleScopeCollection* handles) {
  // Create the array for the opts.
  ArenaAllocator* arena = graph->GetArena();
  HLoopFormation* loop_formation = new (arena) HLoopFormation(graph);
  HFindInductionVariables* find_ivs = new (arena) HFindInductionVariables(graph, stats);
  HRemoveLoopSuspendChecks* remove_suspends = new (arena) HRemoveLoopSuspendChecks(graph, driver, stats);
  HRemoveUnusedLoops* remove_unused_loops = new (arena) HRemoveUnusedLoops(graph, stats);
  TrivialLoopEvaluator* tle = new (arena) TrivialLoopEvaluator(graph, driver, stats);
  HConstantCalculationSinking* ccs = new (arena) HConstantCalculationSinking(graph, stats);
#ifndef SOFIA
  HNonTemporalMove* non_temporal_move = new (arena) HNonTemporalMove(graph, driver, stats);
#endif
  LoadHoistStoreSink* lhss = new (arena) LoadHoistStoreSink(graph, stats);
  HValuePropagationThroughHeap* value_propagation_through_heap =
      new (arena) HValuePropagationThroughHeap(graph, driver, stats);
  HLoopFormation* formation_before_peeling =
      new (arena) HLoopFormation(graph, "loop_formation_before_peeling");
  HLoopPeeling* peeling = new (arena) HLoopPeeling(graph, driver, stats);
  HPureInvokesAnalysis* pure_invokes_analysis = new (arena) HPureInvokesAnalysis(graph, stats);
  HLoopFormation* formation_before_bottom_loops =
      new (arena) HLoopFormation(graph, "loop_formation_before_bottom_loops");
  HFormBottomLoops* form_bottom_loops =
      new (arena) HFormBottomLoops(graph, dex_compilation_unit, handles, stats);
  GVNAfterFormBottomLoops* gvn_after_fbl = new (arena) GVNAfterFormBottomLoops(graph);
  HConstantFolding_X86* constant_folding =
      new (arena) HConstantFolding_X86(graph, stats, "constant_folding_after_phi_cleanup");
  HLoopFullUnrolling* loop_full_unrolling = new (arena) HLoopFullUnrolling(graph, driver, stats);
  HAggressiveUseRemoverPass* aur = new (arena) HAggressiveUseRemoverPass(graph, driver, stats);
  HPhiCleanup* phi_cleanup = new (arena) HPhiCleanup(graph, "phi_cleanup", stats);
  HPhiCleanup* phi_cleanup_after_aur = new (arena) HPhiCleanup(graph, "phi_cleanup_after_aur", stats);
  HDevirtualization* devirtualization = new (arena) HDevirtualization(graph,
                                                                      dex_compilation_unit,
                                                                      driver,
                                                                      handles,
                                                                      /* after_inlining */ false,
                                                                      stats);
  HDevirtualization* devirtualization2 = new (arena) HDevirtualization(graph,
                                                                       dex_compilation_unit,
                                                                       driver,
                                                                       handles,
                                                                       /* after_inlining */ true,
                                                                       stats);
  HSharpeningWrapper* sharpening2 = new (arena) HSharpeningWrapper(graph,
                                                                   codegen,
                                                                   dex_compilation_unit,
                                                                   driver,
                                                                   stats);
  HAbiTransitionHelper* abi_helper = new (arena) HAbiTransitionHelper(graph, driver, stats);
  HOsrGraphRebuilder* osr_graph_rebuilder = new (arena) HOsrGraphRebuilder(graph, stats);
  HBBSimplifier* bb_simplifier = new (arena) HBBSimplifier(graph, stats);
  HCommutativeTreesFlipper* commutative_trees_flipper =
      new (arena) HCommutativeTreesFlipper(graph, stats);

  HOptimization_X86* opt_array[] = {
    devirtualization,
    devirtualization2,
    sharpening2,
    peeling,
    pure_invokes_analysis,
    value_propagation_through_heap,
    osr_graph_rebuilder,
    formation_before_peeling,
    form_bottom_loops,
    formation_before_bottom_loops,
    gvn_after_fbl,
    phi_cleanup,
    constant_folding,
    loop_formation,
    find_ivs,
    tle,
#ifndef SOFIA
    non_temporal_move,
#endif
    ccs,
    remove_suspends,
    lhss,
    remove_unused_loops,
    loop_full_unrolling,
    bb_simplifier,
    aur,
    phi_cleanup_after_aur,
    commutative_trees_flipper,
    abi_helper,
  };

  // Fill verbose flags where we need it.
  FillVerbose(opt_array, arraysize(opt_array),
              driver);

  // Create the vector for the optimizations.
  FillOptimizationList(graph, opt_list, opt_array, arraysize(opt_array));

  // Finish by removing the ones we do not want.
  RemoveOptimizations(opt_list, driver);

  // Print the pass list, if needed.
  PrintPassesOnlyOnce(opt_list, driver);

  // Now execute the optimizations.
  size_t phase_id = 0;
  for (auto optimization : opt_list) {
    if (optimization != nullptr) {
      const char* name = optimization->GetPassName();
      // if debug option --stop-optimizing-after is passed
      // then check whether we need to stop optimization.
      if (driver->GetCompilerOptions().IsConditionalCompilation()) {
        if (driver->GetCompilerOptions().GetStopOptimizingAfter() < phase_id ||
            driver->GetCompilerOptions().GetStopOptimizingAfter() ==
            std::numeric_limits<uint32_t>::max()) {
          break;
        }
        VLOG(compiler) << "Applying " << name << ", phase_id = " << phase_id;
      }
      RunOptWithPassScope scope(optimization, pass_observer);
      scope.Run();
      phase_id++;
    }
  }
}
}  // namespace art
