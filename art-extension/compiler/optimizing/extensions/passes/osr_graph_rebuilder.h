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
 *
 */

#ifndef ART_OPT_PASSES_OSR_GRAPH_REBUILDER_H_
#define ART_OPT_PASSES_OSR_GRAPH_REBUILDER_H_

#include "optimization_x86.h"

namespace art {

/**
 * @brief Transforms loops in graph to allow more optimizations in OSR mode.
 */
class HOsrGraphRebuilder : public HOptimization_X86 {
 public:
  explicit HOsrGraphRebuilder(HGraph* graph, OptimizingCompilerStats* stats = nullptr)
    : HOptimization_X86(graph, "osr_graph_rebuilder", stats),
      fixable_loops_(graph->GetArena()->Adapter()) {}

  void Run() OVERRIDE;

 private:
  /**
   * @brief Collects the loops that can be prepared for OSR.
   * @return True, if at least one valid loop is found.
   */
  bool Gate();

  /**
   * @brief Prepares graph to allow more optimizations.
   */
  void DoOsrPreparation();

  /**
   * @brief Transforms CFG, creating blocks for OSR and normal path.
   *        Returns newly-created blocks in params.
   * @param osr_fork contains one fictive branching between OSR and normal paths.
   * @param normal_path is taken in normal execution, contains parameters.
   * @param osr_path is fictively taken in case of OSR.
   */
  void TransformCfg(HBasicBlock*& osr_fork,
                    HBasicBlock*& normal_path,
                    HBasicBlock*& osr_path);

  /**
    * @brief Moves params and method entry suspend check from entry block
    *        to normal path block.
    * @param entry_block The graph entry block.
    * @param normal_path The newly-created normal path block.
    */
  void MoveParams(HBasicBlock* entry_block,
                  HBasicBlock* normal_path);

  /**
    * @brief Injects edges from osr_path to all found fixable loops.
    * @param osr_path The OSR path block.
    */
  void InjectOsrIntoLoops(HBasicBlock* osr_path);

  /**
    * @brief Performs Phi node fixup after OSR path injection.
    * @param loop The loop to be fixed.
    * @param osr_path The OSR path block.
    * @param new_pre_header The loop pre-header.
    */
  void InjectionPhiFixup(HLoopInformation_X86* loop,
                         HBasicBlock* osr_path,
                         HBasicBlock* new_pre_header);

  // Used to collect the loops that can be prepared for OSR.
  ArenaVector<HLoopInformation_X86*> fixable_loops_;
};

}  // namespace art

#endif  // ART_OPT_PASSES_OSR_GRAPH_REBUILDER_H_
