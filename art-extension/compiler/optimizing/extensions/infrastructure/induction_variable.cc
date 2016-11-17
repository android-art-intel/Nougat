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

#include "ext_utility.h"
#include "induction_variable.h"

namespace art {

void HInductionVariable::Dump(std::ostream& os) const {
  const char* prefix = HLoopInformation_X86::kLoopDumpPrefix;
  if (IsFP()) {
    PRINT_OSTREAM_MESSAGE(os, prefix, "IV: " << linear_insn_
      << " wide: " << IsWide() << ", FP: " << IsFP()
      << ", loop increment: " << GetFPIncrement());
  } else {
    PRINT_OSTREAM_MESSAGE(os, prefix, "IV: " << linear_insn_
      << " wide: " << IsWide() << ", FP: " << IsFP()
      << ", loop increment: " << GetIncrement());
  }
}
}  // namespace art
