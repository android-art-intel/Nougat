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

#ifndef ART_COMPILER_OPTIMIZING_EXTENSIONS_PASSES_AUR_H_
#define ART_COMPILER_OPTIMIZING_EXTENSIONS_PASSES_AUR_H_

#include "driver/compiler_driver-inl.h"
#include "driver/compiler_options.h"
#include "optimization_x86.h"

namespace art {

/**
 * @brief This is an optimization pass that cleans up environments in non-debuggable mode.
 * @details Environments currently keep a list of live dex virtual registers which are
 * potentially live into runtime. This makes it possible to transition to interpreter
 * at any point there is interaction with runtime. However, for non-debuggable applications,
 * that is not needed unless it is an explicit deoptimize.
 */
class HAggressiveUseRemoverPass : public HOptimization_X86 {
 public:
  HAggressiveUseRemoverPass(HGraph* graph,
                            CompilerDriver* compiler_driver,
                            OptimizingCompilerStats* stats = nullptr)
    : HOptimization_X86(graph, "aur", stats),
      is_boot_image_(compiler_driver->GetCompilerOptions().IsBootImage()) {}

  void Run() OVERRIDE;

 private:
  const bool is_boot_image_;
};

}  // namespace art

#endif  // ART_COMPILER_OPTIMIZING_EXTENSIONS_PASSES_AUR_H_
