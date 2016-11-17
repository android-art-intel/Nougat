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

#ifndef ART_COMPILER_OPTIMIZING_REMOVE_SUSPEND_H_
#define ART_COMPILER_OPTIMIZING_REMOVE_SUSPEND_H_

#include "driver/compiler_driver.h"
#include "optimization_x86.h"

#define MAX_SUSPEND_TIME_CYCLES 1000000  // 1 million, < 1 ms @ 1.3 GHz.

namespace art {

class HRemoveLoopSuspendChecks : public HOptimization_X86 {
 public:
  explicit HRemoveLoopSuspendChecks(HGraph* graph,
                                    CompilerDriver* driver,
                                    OptimizingCompilerStats* stats = nullptr)
      : HOptimization_X86(graph, kRemoveLoopSuspendChecks, stats),
        driver_(driver) {}

  void Run() OVERRIDE;

 private:
  static constexpr const char* kRemoveLoopSuspendChecks = "remove_loop_suspend_checks";

  const CompilerDriver* const driver_;

  DISALLOW_COPY_AND_ASSIGN(HRemoveLoopSuspendChecks);
};

}  // namespace art

#endif  // ART_COMPILER_OPTIMIZING_REMOVE_SUSPEND_H_
