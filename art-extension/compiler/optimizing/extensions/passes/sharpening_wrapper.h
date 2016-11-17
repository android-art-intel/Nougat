/*
 * Copyright (C) 2016 Intel Corporation
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

#ifndef ART_COMPILER_OPTIMIZING_EXTENSIONS_PASSES_SHARPENING_X86_H_
#define ART_COMPILER_OPTIMIZING_EXTENSIONS_PASSES_SHARPENING_X86_H_

#include "optimization_x86.h"
#include "sharpening.h"

namespace art {

class HSharpeningWrapper : public HOptimization_X86 {
 public:
  HSharpeningWrapper(HGraph* graph,
                     CodeGenerator* codegen,
                     const DexCompilationUnit& compilation_unit,
                     CompilerDriver* compiler_driver,
                     OptimizingCompilerStats* stats = nullptr)
    : HOptimization_X86(graph,
                        kSharpeningAfterInliningPassName,
                        stats),
      codegen_(codegen),
      compilation_unit_(compilation_unit),
      compiler_driver_(compiler_driver) { }
  ~HSharpeningWrapper() { }

  void Run() OVERRIDE {
    HSharpening sharpening(graph_, codegen_, compilation_unit_,
                           compiler_driver_, stats_);
    sharpening.Run();
  }

  static constexpr const char* kSharpeningAfterInliningPassName = "sharpening_after_inlining";

 private:
  CodeGenerator* const codegen_;
  const DexCompilationUnit& compilation_unit_;
  CompilerDriver* const compiler_driver_;

  DISALLOW_COPY_AND_ASSIGN(HSharpeningWrapper);
};

}  // namespace art

#endif  // ART_COMPILER_OPTIMIZING_EXTENSIONS_PASSES_SHARPENING_X86_H_
