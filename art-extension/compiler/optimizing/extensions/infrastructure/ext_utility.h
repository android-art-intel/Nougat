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

#ifndef COMPILER_OPTIMIZING_EXTENSIONS_INFRASTRUCTURE_EXT_UTILITY_H
#define COMPILER_OPTIMIZING_EXTENSIONS_INFRASTRUCTURE_EXT_UTILITY_H

#include "base/stringprintf.h"
#include "driver/dex_compilation_unit.h"
#include "nodes.h"
#include <ostream>
#include <unordered_set>

namespace art {

class HInstruction;

  // This macro can be used to print messages within passes. The possible usage is:
  // PRINT_PASS_MESSAGE(this, "Value of x = %.2f", x);
  #define PRINT_PASS_MESSAGE(pass, format, ...) \
    do { \
      if (pass->IsVerbose()) { \
        LOG(INFO) << pass->GetPassName() << ": " << StringPrintf(format, ##__VA_ARGS__); \
      } \
    } while (false)

  // PRINT_PASS_OSTREAM_MESSAGE(this, x << yy << zz);
  #define PRINT_PASS_OSTREAM_MESSAGE(pass, ...) \
    do { \
      if (pass->IsVerbose()) { \
        LOG(INFO) << pass->GetPassName() << ": " << __VA_ARGS__; \
      } \
    } while (false)

  // PRINT_OSTREAM_MESSAGE(os, prefix, x << yy << zz);
  #define PRINT_OSTREAM_MESSAGE(os, prefix, ...) \
    do { \
      os << prefix << ": " << __VA_ARGS__ << std::endl; \
    } while (false)

class HAllUseIterator {
 public:
  explicit HAllUseIterator(HInstruction* insn) :
      env_use_it_(insn->GetEnvUses().begin()),
      env_use_it_end_(insn->GetEnvUses().end()),
      use_it_(insn->GetUses().begin()),
      use_it_end_(insn->GetUses().end()),
      index_(0),
      env_user_(nullptr),
      user_(nullptr) {
    Advance();
  }

  bool Done() {
    return (use_it_ == use_it_end_) &&
           (env_use_it_ == env_use_it_end_) &&
           env_user_ == nullptr &&
           user_ == nullptr;
  }

  HInstruction* Current() {
    return IsEnv() ? env_user_->GetHolder() : user_;
  }

  bool IsEnv() {
    return env_user_ != nullptr;
  }

  void Advance() {
    if (use_it_ == use_it_end_) {
      if (env_use_it_ == env_use_it_end_) {
        env_user_ = nullptr;
        user_ = nullptr;
      } else {
        index_ = env_use_it_->GetIndex();
        env_user_ = env_use_it_->GetUser();
        ++env_use_it_;
      }
    } else {
      index_ = use_it_->GetIndex();
      user_ = use_it_->GetUser();
      ++use_it_;
    }
  }

  void ReplaceInput(HInstruction* new_input) {
    if (IsEnv()) {
      DCHECK(new_input == nullptr || new_input->GetBlock() != nullptr);
      env_user_->RemoveAsUserOfInput(index_);
      env_user_->SetRawEnvAt(index_, new_input);
      if (new_input != nullptr) {
        new_input->AddEnvUseAt(env_user_, index_);
      }
    } else {
      DCHECK(new_input->GetBlock() != nullptr);
      user_->ReplaceInput(new_input, index_);
    }
  }

 private:
  HUseList<HEnvironment*>::const_iterator env_use_it_;
  HUseList<HEnvironment*>::const_iterator env_use_it_end_;
  HUseList<HInstruction*>::const_iterator use_it_;
  HUseList<HInstruction*>::const_iterator use_it_end_;
  size_t index_;
  HEnvironment* env_user_;
  HInstruction* user_;
};

  /**
   * @brief Facility to get the name of the current method.
   * @param graph The HGgraph corresponding to the method.
   * @return The name of the current method.
   */
  std::string GetMethodName(const HGraph* graph);

  /**
   * @brief Split string by delimeter and put tokens to the set.
   * @param s The string to split.
   * @param delim The delimeter character.
   * @param tokens The output set for tokens.
   */
  void SplitStringIntoSet(const std::string& s, char delim, std::unordered_set<std::string>& tokens);

  /**
   * @brief Get the integer constant value from the HConstant.
   * @param constant the HConstant we care about.
   * @param value the destination where we put the HConstant value.
   * @return if we obtained the constant value.
   */
  bool GetIntConstantValue(HConstant* constant, int64_t& value);

  /**
   * @brief Get the compare instruction from the uses of instruction.
   * @param instruction the base instruction.
   * @return the compare instruction based on the uses of the instruction,
             instruction if neither is one.
   */
  HInstruction* GetCompareInstruction(HInstruction* instruction);

  /**
   * @brief Get the FP Constant value.
   * @param constant the HConstant.
   * @param value the value of the constant filled out.
   * @return whether or not the constant is assessed.
   */
  bool GetFPConstantValue(HConstant* constant, double& value);

  /**
   * @brief Flip the condition if we flip the operands.
   * @details if we have A OP B, this returns OP2 with B OP2 A being equivalent.
   * @param cond the original condition.
   * @return the new flipped condition.
   */
  IfCondition FlipConditionForOperandSwap(IfCondition cond);

  /**
   * @brief Get the negated conditional operation.
   * For example, for if-ge, it returns if-lt.
   * @param cond The conditional if operation to negate.
   * @return Returns the negated conditional if.
   */
  IfCondition NegateCondition(IfCondition cond);

  /**
   * @brief Get sign, power and mantissa of floating point value.
   * @param value The FP value represented as double.
   * @param is_double Whether the value is double-precision FP or not.
   * @param with_implicit_one Whether we should get mantissa with implicit 1 or not.
   * @param sign The output value for sign (0 or 1).
   * @param power The output value for power. Derived from the exponent of FP.
                  For non-zeros, power = exponent - 1111...11; power(1) = 0.
   * @param mantissa The output value for mantissa. If with_implicit_one was true, it will
   *                 also contain the 23-rd (52-nd for double) bit set to 1 for non-zero value.
   */
  void DeconstructFP(double value, bool is_double, bool with_implicit_one, int32_t& sign, int32_t& power, int64_t& mantissa);

  /**
   * @brief Get the number of zeros in the end of mantissa of floating point value.
   * @param mantissa The mantissa.
   * @param is_double Whether the mantissa is of double-precision FP or not.
   * @return The number of zeros in the end of mantissa. The maximum return
   *         value is mantissa length (23 for single FP and 52 for double FP).
   */
  int CountEndZerosInMantissa(int64_t mantissa, bool is_double);

  /**
   * @brief Write an instruction to an output stream.
   * @param os Output stream.
   * @param instruction Instruction to write.
   */
  std::ostream& operator<<(std::ostream& os, HInstruction* instruction);

  /**
   * @brief Remove the given instruction as user of its inputs and its environment inputs.
   * @param instruction The HInstruction to handle.
   * @details RemoveEnvAsUser is called internally to handle environment inputs.
   */
  void RemoveAsUser(HInstruction* instruction);

  /**
   * @brief Remove the given instruction's environment as user of its inputs.
   * @param instruction The HInstruction to handle.
   */
  void RemoveEnvAsUser(HInstruction* instruction);

  /**
   * @brief Used to mark the current instruction as not being used by any environments.
   * @param instruction The HInstruction to handle.
   */
  void RemoveFromEnvironmentUsers(HInstruction* instruction);

  /**
   * @brief Get the character equivalent of the type for print or debug purposes.
   * @param type The type we want to get the equivalent of.
   * @return The resulting character equivalent.
   */
  char GetTypeId(Primitive::Type type);

  /**
   * @brief Returns pretty method name for the invoke.
   * @param call The invoke.
   * @return Called method name as string.
   */
  std::string CalledMethodName(HInvokeStaticOrDirect* call);

  /**
   * @brief Dumps a basic block to logcat. For debugging purposes only!
   * @param name The name of a block being dumped (to distinguish them).
   * @param block The block to dump.
   */
  void DumpBasicBlock(const char* name, HBasicBlock* block);

  /**
   * @brief Dumps a loop to logcat. For debugging purposes only!
   * @param loop The loop to dump.
   */
  void DumpLoop(HLoopInformation_X86* loop);

  /**
   * @brief Whether the expression can be safely removed.
   * @param instruction The instruction to check.
   * @return true, if the instruction can be safely removed from code.
   */
  bool CanExpressionBeRemoved(HInstruction* instruction);

  /**
    * @brief Tries to kill an instruction, its inputs, their inputs etc.
    * @param pass The pass that removes the instruction.
    * @param insn The instruction to remove.
    * @param all_removed The output set that contains all removed instructions.
    *                    May be nullptr, if we don't need to collect those instructions.
    */
  void TryKillUseTree(HOptimization_X86* pass,
                      HInstruction* insn,
                      std::unordered_set<HInstruction*>* all_removed = nullptr);

  enum {
    kHotMethodCount = 50,            // How often must a method be called to be hot?
    kHotMethodBlockCountSum = 2000,  // Sum of all block counts for a method to be hot.
    kColdBlockFactor = 3,            // A cold block is executed kColdBlockFactor times
                                     // LESS than the entry block.
    kColdLoopBlockFactor = 10,       // A cold block is executed kColdLoopBlockFactor times
                                     // LESS than the loop header block.
    kHotBlockFactor = 10,            // A hot block is executed kHotBlockFactor times
                                     // MORE than the entry block.
  };

  enum BlockHotness {
    kUnknown,   // No BB counts are available or the method has not been invoked.
    kCold,      // Block is execute kColdBlockFactor less frequently than the method.
    kWarm,      // Block is executed with the about the same as the method.
    kHot,       // Block is executed kHotBlockFactor more frequently than the method.
  };

  /*
   * @brief Return the 'hotness' of this basic block.
   * @param block Block to check.
   * @returns An indication of how often the block is executed per method invoke.
   */
  BlockHotness GetBlockHotness(HBasicBlock* block);

  /*
   * @brief Simple interface to interact at runtime on host/target.
   */
  class SetBoolValue {
   public:
    /*
     * @brief Set a boolean based on a property or environment variable.
     * @param property Boolean property on target ('true'/anything else)
     * @param env_var Boolean environment variable on host (set or not set).
     */
    SetBoolValue(const char* property, const char* env_var);

    // Return 'true' if the property/environment variable was set.
    bool operator()() const {
      return value_;
    }

   private:
    bool value_;
  };

}  // namespace art
#endif  // COMPILER_OPTIMIZING_EXTENSIONS_INFRASTRUCTURE_EXT_UTILITIES_H
