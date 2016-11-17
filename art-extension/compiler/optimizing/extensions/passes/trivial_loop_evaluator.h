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

#ifndef ART_COMPILER_OPTIMIZING_TRIVIAL_LOOP_EVALUATOR_H_
#define ART_COMPILER_OPTIMIZING_TRIVIAL_LOOP_EVALUATOR_H_

#include "driver/compiler_driver.h"
#include "optimization_x86.h"

namespace art {
// Forward declaration.
class TLEVisitor;

/**
 * @brief Trivial Loop Evaluator is an optimization pass which tries to evaluate the loops of
 * a HGraph when possible. If so, it removes the loop of the graph, writes back the evaluated
 * results and updates their users.
 */
class TrivialLoopEvaluator : public HOptimization_X86 {
  public:
    explicit TrivialLoopEvaluator(HGraph* graph, CompilerDriver* driver,
                                  OptimizingCompilerStats* stats = nullptr)
      : HOptimization_X86(graph, "trivial_loop_evaluator", stats), driver_(driver) {}

    virtual ~TrivialLoopEvaluator() {}

    void Run() OVERRIDE;

  private:
    /**
     * @brief Narrows down the scope of application of TLE to the loops having
     * specific properties.
     * @param loop The HLoopInformation_X86 loop this method will check.
     * @return True if the loop is a valid candidate for TLE, or false otherwise.
     */
    bool LoopGate(HLoopInformation_X86* loop) const;

    /**
     * @brief Tries to statically evaluate the given loop.
     * @param loop The HLoopInformation_X86 loop this method will try to evaluate
     * statically.
     * @param visitor The structure which will hold all the writes resulting of the
     * evaluation of the loop. It should be empty when calling this method.
     * @return True if TLE successfully evaluated the loop, or false otherwise.
     */
    bool EvaluateLoop(HLoopInformation_X86* loop, TLEVisitor& visitor);

    /**
     * @brief This method deletes all the instructions in the loop's only BB,
     * it replaces them by the constant values previously evaluated by TLE, and deletes
     * the backedge of the given loop.
     * @param loop The HLoopInformation_X86 loop previously evaluated successfully by TLE.
     * @param visitor The result structure holding values we need to write back.
     */
    void UpdateRegisters(HLoopInformation_X86* loop, TLEVisitor& visitor);

    /** Maximum number of iterations in an evaluable loop. Beyond this limits, loops are considered
     * too costly to be statically evaluated. */
    static constexpr int64_t kDefaultLoopEvalMaxIter = 1000;

    const CompilerDriver* const driver_;

    /** Copy and assignment are not allowed. */
    DISALLOW_COPY_AND_ASSIGN(TrivialLoopEvaluator);
};

}  // namespace art.

#endif  // ART_COMPILER_OPTIMIZING_TRIVIAL_LOOP_EVALUATOR_H_

