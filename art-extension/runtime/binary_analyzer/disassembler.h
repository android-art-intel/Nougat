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

#ifndef ART_RUNTIME_BINARY_ANALYZER_DISASSEMBLER_H_
#define ART_RUNTIME_BINARY_ANALYZER_DISASSEMBLER_H_

#include "arch/instruction_set.h"
#include <cstddef>

namespace art {

// Make sure capstone does not pollute the global namespace.
extern "C" {
#include <capstone.h>
}

/**
 * @brief The Disassembler class is a wrapper around the capstone disassembly library.
 * The Disassembler can be viewed as a iterator which walks a binary stream and returns
 * a cs_insn object each time the iterator is invoked. The Disassembler maintains both
 * a user-specified address and a pointer (the address being useful in cases in which
 * the address at which stream might get linked/placed at is different than the current
 * location of the stream on the host).
 */
class Disassembler {
 public:
  explicit Disassembler(InstructionSet insn_set);
  ~Disassembler();

  /**
   * @brief Returns the address of the head of the stream (that is, the address of the
   * stream that the user indicated when the last Seek operation was performed).
   *
   * @return Address of the head of the stream.
   */
  uint64_t GetAddress() const {
    return address_;
  }

  /**
   * @brief Returns a pointer corresponding to the head of the binary stream.
   *
   * @return Pointer to the head of the strema.
   */
  const uint8_t* GetPtr() const {
    return ptr_;
  }

  /**
   * @brief Checks if the Disassembler was constructed successfully. The constructor
   * will fail if the library was not configured for the requested architecture.
   *
   * @return true if the Disassembler is ready for use; false otherwise.
   */
  bool IsDisassemblerValid() const {
    return err_ == CS_ERR_OK;
  }

  /**
   * @brief Diassembles the next instruction in the binary stream. The contents of any
   * prior cs_insn returned by this method will have its contents overwritten.
   *
   * @return A cs_insn object that is populated with details about the next instruction.
   */
  const cs_insn* Next() {
    size_t sz = std::numeric_limits<size_t>::max();
    if (UNLIKELY(!cs_disasm_iter(handle_, &ptr_, &sz, &address_, insn_))) {
      return nullptr;
    }

    return insn_;
  }

  /**
   * @brief Positions the iterator head at ptr and sets the user-specified address to ptr.
   */
  void Seek(const uint8_t* ptr) {
    Seek(ptr, reinterpret_cast<uint64_t>(ptr));
  }

  /**
   * @brief Positions the iterator head at ptr and sets the user-specified address.
   */
  void Seek(const uint8_t* ptr, uint64_t address) {
    ptr_ = ptr;
    address_ = address;
  }

 private:
  uint64_t address_;
  const uint8_t* ptr_;
  cs_insn* insn_;
  csh handle_;
  cs_err err_;

 private:
  DISALLOW_COPY_AND_ASSIGN(Disassembler);
};

}  // namespace art

#endif  // ART_RUNTIME_BINARY_ANALYZER_DISASSEMBLER_H_
