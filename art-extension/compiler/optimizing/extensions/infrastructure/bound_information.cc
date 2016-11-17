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

#include "bound_information.h"
#include "ext_utility.h"
#include "induction_variable.h"

namespace art {

void HLoopBoundInformation::Dump(std::ostream& os) {
  const char* prefix = HLoopInformation_X86::kLoopDumpPrefix;
  if (GetLoopBIV() == nullptr) {
    PRINT_OSTREAM_MESSAGE(os, prefix, "The loop does not have a basic IV");
  } else {
    GetLoopBIV()->Dump(os);
    if (IsSimpleCountUp()) {
      PRINT_OSTREAM_MESSAGE(os, prefix,
        "The loop is a simple count up loop:");
    } else if (is_simple_count_down_) {
      PRINT_OSTREAM_MESSAGE(os, prefix,
        "The loop is a simple count down loop");
    } else {
      PRINT_OSTREAM_MESSAGE(os, prefix,
        "The loop is neither a simple count up nor count down loop");
    }

    std::string cond;
    switch (GetComparisonCondition()) {
      case kCondLT:
        cond = "<";
        break;
      case kCondLE:
        cond = "<=";
        break;
      case kCondGE:
        cond = ">=";
        break;
      case kCondGT:
        cond = ">";
        break;
      case kCondEQ:
        cond = "==";
        break;
      case kCondNE:
        cond = "!=";
        break;
      default:
        DCHECK(false) << "Unexpected condition " << GetComparisonCondition();
        cond = "???";
    }

    if (IsFP()) {
      PRINT_OSTREAM_MESSAGE(os, prefix,
         "\tfor (i = " << GetFPBIVStartValue() << "; i " << cond
         << " " << GetFPBIVEndValue()  << "; i += "
         << GetLoopBIV()->GetFPIncrement() << ")");
    } else {
      PRINT_OSTREAM_MESSAGE(os, prefix,
         "\tfor (i = " << GetIntegralBIVStartValue() << "; i " << cond
         << " " << GetIntegralBIVEndValue()  << "; i += "
         << GetLoopBIV()->GetIncrement() << ")");
    }
  }

  if (num_iterations_ == -1) {
    PRINT_OSTREAM_MESSAGE(os, prefix, "The loop has unknown number of iterations");
  } else {
    DCHECK_GE(num_iterations_, 0);
    PRINT_OSTREAM_MESSAGE(os, prefix, "The loop has " << num_iterations_ << " iterations");
  }
}

bool HLoopBoundInformation::IsFP() const {
  auto biv = GetLoopBIV();
  return biv != nullptr && biv->IsFP();
}

bool HLoopBoundInformation::IsInteger() const {
  auto biv = GetLoopBIV();
  return biv != nullptr && biv->IsInteger();
}

bool HLoopBoundInformation::IsLong() const {
  auto biv = GetLoopBIV();
  return biv != nullptr && biv->IsLong();
}

bool HLoopBoundInformation::IsFloat() const {
  auto biv = GetLoopBIV();
  return biv != nullptr && biv->IsFloat();
}

bool HLoopBoundInformation::IsDouble() const {
  auto biv = GetLoopBIV();
  return biv != nullptr && biv->IsDouble();
}

bool HLoopBoundInformation::IsWide() const {
  auto biv = GetLoopBIV();
  return biv != nullptr && biv->IsWide();
}

int64_t HLoopBoundInformation::GetIntegralBIVStartValue() const {
  DCHECK(GetLoopBIV() != nullptr);
  DCHECK(!IsFP());
  return biv_start_value_.integral;
}

void HLoopBoundInformation::SetIntegralBIVStartValue(int64_t start_value) {
  DCHECK(GetLoopBIV() != nullptr);
  DCHECK(!IsFP());
  biv_start_value_.integral = start_value;
}

int64_t HLoopBoundInformation::GetIntegralBIVEndValue() const {
  DCHECK(GetLoopBIV() != nullptr);
  DCHECK(!IsFP());
  return biv_end_value_.integral;
}

void HLoopBoundInformation::SetIntegralBIVEndValue(int64_t end_value) {
  DCHECK(GetLoopBIV() != nullptr);
  DCHECK(!IsFP());
  biv_end_value_.integral = end_value;
}

double HLoopBoundInformation::GetFPBIVStartValue() const {
  DCHECK(GetLoopBIV() != nullptr);
  DCHECK(IsFP());
  return biv_start_value_.fp;
}

void HLoopBoundInformation::SetFPBIVStartValue(double start_value) {
  DCHECK(GetLoopBIV() != nullptr);
  DCHECK(IsFP());
  biv_start_value_.fp = start_value;
}

double HLoopBoundInformation::GetFPBIVEndValue() const {
  DCHECK(GetLoopBIV() != nullptr);
  DCHECK(IsFP());
  return biv_end_value_.fp;
}

void HLoopBoundInformation::SetFPBIVEndValue(double end_value) {
  DCHECK(GetLoopBIV() != nullptr);
  DCHECK(IsFP());
  biv_end_value_.fp = end_value;
}

}  // namespace art
