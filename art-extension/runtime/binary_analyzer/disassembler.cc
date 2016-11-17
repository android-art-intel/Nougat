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

#include "arch/instruction_set.h"
#include "base/logging.h"
#include "disassembler.h"

#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <limits>

namespace art {

Disassembler::Disassembler(InstructionSet insn_set) {
  ptr_ = nullptr;
  address_ = 0;

  // Instantiate a capstone handle/instance.
  switch (insn_set) {
  case kX86:
    err_ = cs_open(CS_ARCH_X86, CS_MODE_32, &handle_);
    break;

  case kX86_64:
    err_ = cs_open(CS_ARCH_X86, CS_MODE_64, &handle_);
    break;

  default:
    err_ = CS_ERR_ARCH;
    return;
  }

  if (err_ != CS_ERR_OK) {
    return;
  }

  err_ = cs_option(handle_, CS_OPT_DETAIL, CS_OPT_ON);

  if (err_ != CS_ERR_OK) {
    return;
  }

  // Allocate memory for the the iterator.
  insn_ = cs_malloc(handle_);

  if (insn_ == nullptr) {
    err_ = CS_ERR_MEM;
  }
}

Disassembler::~Disassembler() {
  if (insn_ != nullptr) {
    cs_free(insn_, 1);
  }

  cs_close(&handle_);
}

}  // namespace art
