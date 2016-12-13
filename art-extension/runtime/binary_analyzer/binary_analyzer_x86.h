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

#ifndef ART_RUNTIME_BINARY_ANALYZER_BINARY_ANALYZER_X86_H_
#define ART_RUNTIME_BINARY_ANALYZER_BINARY_ANALYZER_X86_H_

#include <iostream>
#include <list>
#include <ostream>
#include <set>
#include <string>
#include <sys/uio.h>
#include <tuple>

#include "art_field-inl.h"
#include "art_method-inl.h"
#include "base/logging.h"
#include "disassembler.h"
#include "mirror/class-inl.h"
#include "mirror/class_loader.h"
#include "mirror/object-inl.h"
#include "mirror/object_array-inl.h"
#include "mirror/string-inl.h"
#include "mirror/throwable.h"
#include "utils/assembler.h"

namespace art {
namespace x86 {

/**
 * @brief The instruction class stores information about
 * the decoded/analyzed binary instruction.
 */
class MachineInstruction {
 public:
  MachineInstruction(std::string instruction_str, uint8_t length, const uint8_t* ptr)
      : instr_(instruction_str), instr_ptr_(ptr), length_(length) {
  }

  ~MachineInstruction() {}

  /**
   * @brief prints the decoded x86 instruction in human readable/dot format to logcat.
   * @param output - output stream.
   * @param is_dot - whether the output must be in dot format.
   */
  void Print(std::ostream& output, bool is_dot);

  /**
   * @brief prints the decoded x86 instruction in human readable/dot format.
   * @param is_dot - whether the output must be in dot format.
   * @return the output stream.
   */
  std::ostringstream Print(bool is_dot);

  /**
   * @brief returns the human readable x86 instruction.
   * @return the decoded x86 instruction.
   */
  const std::string& GetInstruction() const {
    return instr_;
  }

  /**
   * @brief Get the number of bytes of assembly instruction.
   * @return the number of bytes the instruction consumes.
   */
  uint8_t GetLength() const {
    return length_;
  }

  /**
   * @brief Get the pointer to the instruction.
   * @return the byte pointer to the x86 assembly code.
   */
  const uint8_t* GetInstructionPtr() const {
    return instr_ptr_;
  }

  /**
   * @brief Get the previous Instruction pointer.
   * @return the previous instruction's pointer.
   */
  const MachineInstruction* GetPrevInstruction() {
    return prev_instr_;
  }

  /**
   * @brief Get the next Instruction pointer.
   * @return the next instruction's pointer.
   */
  MachineInstruction* GetNextInstruction() {
    return next_instr_;
  }

  /**
   * @brief Set the previous instruction pointer for the current Instruction.
   * @param prev the previous instruction pointer.
   */
  void SetPrevInstruction(MachineInstruction* prev) {
    prev_instr_ = prev;
  }

  /**
   * @brief Set the next instruction pointer for the current Instruction.
   * @param next the next instruction pointer.
   */
  void SetNextInstruction(MachineInstruction* next) {
    next_instr_ = next;
  }

 private:
  std::string instr_;
  const uint8_t* instr_ptr_;
  uint8_t length_;
  MachineInstruction* prev_instr_;
  MachineInstruction* next_instr_;
};

/**
 * @brief BBlock class contains information about a Basic Block.
 * Note : A dummy basic block is the one that connects a basic block
 * that ends with a call instruction to the starting of the call site
 * Basic Block. A dummy basic block does not contain any instructions.
 * It merely serves as a link.
 */
class MachineBlock {
 public:
  explicit MachineBlock(MachineBlock* pred_bb, const uint8_t* function_start_addr)
      : num_of_instrs_(0),
        is_dummy_(false) {
    function_start_addr_ = function_start_addr;
    if (pred_bb != nullptr) {
      AddPredBBlock(pred_bb);
      pred_bb->AddSuccBBlock(this);
    }
    id_ = -1;
    start_addr_ = nullptr;
    end_addr_ = nullptr;
  }

  ~MachineBlock();

  /**
   * @brief Get the last Instruction class pointer for the Basic Block.
   * @return the pointer to the last Instruction of the Basic Block.
   */
  MachineInstruction* GetLastInstruction() const {
    if (!instrs_.empty()) {
      return instrs_.back();
    }
    return nullptr;
  }

  /**
   * @brief Set the starting address of the Basic Block.
   * @param start - Starting address of the Basic Block.
   */
  void SetStartAddr(const uint8_t* start) {
    start_addr_ = start;
    if (function_start_addr_ == nullptr) {
      function_start_addr_ = start; // Probably we are the first in CFG.
    }
  }

  /**
   * @brief Get the starting address of the Basic Block.
   * @return Starting address of the Basic Block.
   */
  const uint8_t* GetStartAddr() const {
    return start_addr_;
  }

  /**
   * @brief Set the ending address of the Basic Block.
   * @param end - Ending address of the Basic Block.
   */
  void SetEndAddr(const uint8_t* end) {
    end_addr_ = end;
  }

  /**
   * @brief Get the ending address of the Basic Block.
   * @return Ending address of the Basic Block.
   */
  const uint8_t* GetEndAddr() const {
    return end_addr_;
  }

  /**
  * @param start - address of the first instruction of the function which this Basic Block belongs.
   * @brief Set start of the function which this Basic Block belongs.
   */
  void SetFunctionStartAddr(const uint8_t* start) {
    function_start_addr_ = start;
  }

  /*
   * @brief Get start of the function which this Basic Block belongs.
   * @return Address of the first instruction of this function.
   */
  const uint8_t* GetFunctionStartAddr() const {
    return function_start_addr_;
  }

  /**
   * @brief Set this Basic Block as Dummy.
   */
  void SetDummy() {
    is_dummy_ = true;
  }

  /**
   * @brief Is this Basic Block a dummy.
   * @return whether the basic block is a dummy or not.
   */
  bool IsDummy() const {
    return is_dummy_;
  }

  /**
   * @brief Get the number of Instructions.
   * @return number of Instructions in the Basic Block.
   */
  uint32_t GetInstrCnt() const {
    return num_of_instrs_;
  }

  /**
   * @brief Get The Id of this Basic Block.
   * @return the Basic Block's id.
   */
  uint32_t GetId() const {
    return id_;
  }

  /**
   * @brief Set the Basic Block's Id.
   * @param - the Id for the Basic Block.
   */
  void SetId(uint32_t id) {
    id_ = id;
  }

  /**
   * @brief Add the Instruction to the Basic Block.
   * @param instruction - The instruction to be added to the Basic Block.
   */
  void AddInstruction(MachineInstruction* instruction) {
    instrs_.push_back(instruction);
    ++num_of_instrs_;
  }

  /**
   * @brief Add a Basic Block to list of predecessor Basic Blocks.
   * @param bblock - the predecessor Basic Block.
   */
  void AddPredBBlock(MachineBlock* bblock);

  /**
   * @brief Add a Basic Block to list of successor Basic Blocks.
   * @param bblock - the successor Basic Block.
   */
  void AddSuccBBlock(MachineBlock* bblock);

  /**
   * @brief Delete a certain Basic Block from the list of predecessor Basic Blocks.
   * @param bblock - the Basic Block to be deleted.
   */
  void DeletePredBBlock(MachineBlock* bblock);

  /**
   * Get the list of predecessor Basic Blocks.
   * @return the predecessor Basic Block List.
   */
  const std::vector<MachineBlock*>& GetPredBBlockList() const {
    return pred_bblock_;
  }

  /**
   * Get the list of successor Basic Blocks.
   * @return the successor Basic Block List.
   */
  const std::vector<MachineBlock*>& GetSuccBBlockList() const {
    return succ_bblock_;
  }

  /**
   * @brief Make a copy of the array of predecessor Basic Blocks.
   * @param to_copy - The vector that contain predecessor Basic Blocks to be copied.
   */
  void CopyPredBBlockList(const std::vector<MachineBlock*>& to_copy);

  /**
   * @brief Make a copy of the array of successor Basic Blocks.
   * @param to_copy - The vector that contain successor Basic Blocks to be copied.
   */
  void CopySuccBBlockList(const std::vector<MachineBlock*>& to_copy);

  /**
   * @brief Clear the array of predecessor Basic Blocks.
   */
  void ClearPredBBlockList() {
    pred_bblock_.clear();
  }

  /**
   * @brief Clear the array of successor Basic Blocks.
   */
  void ClearSuccBBlockList() {
    succ_bblock_.clear();
  }

  /**
   * @brief Get the Instruction Class Pointer List for the Basic Block.
   * @return The Instruction class pointer list.
   */
  const std::list<MachineInstruction*>& GetInstructions() const {
    return instrs_;
  }

  /**
   * @brief prints the info on Basic Block in human readable/dot format to logcat.
   * @param output - output stream.
   * @param is_dot - whether the output must be in dot format.
   */
  void Print(std::ostream& output, bool is_dot);

  /**
   * @brief prints the info on Basic Block in human readable/dot format.
   * @param is_dot - whether the output must be in dot format.
   * @return the output stream.
   */
  std::ostringstream Print(bool is_dot);

  /**
   * @brief Copies the Instructions Starting from a certain Address to the Basic Block.
   * @param from_bblock - the Basic Block from which Instructions have to be copied.
   * @param start - The starting pointer to the Instruction Class.
   */
  void CopyInstruction(MachineBlock* from_bblock, const uint8_t* start);

  /**
   * @brief Deletes a certain Instruction from the Basic Block.
   * @param it - iterator for the Instruction List.
   */
  void DeleteInstruction(std::list<MachineInstruction*>::iterator it);

 private:
  uint32_t id_;
  const uint8_t* function_start_addr_;
  const uint8_t* start_addr_;
  const uint8_t* end_addr_;
  uint32_t num_of_instrs_;
  std::vector<MachineBlock*> pred_bblock_;
  std::vector<MachineBlock*> succ_bblock_;
  std::list<MachineInstruction*> instrs_;
  bool is_dummy_;
};

/**
 * The BackLogDs Data Structure stores the code paths that have not yet been
 * analyzed. This could be due to call, jump (conditional/unconditional) etc.
 */
struct BackLogDs {
  MachineBlock* pred_bb;
  const uint8_t* ptr;
  MachineBlock* succ_bb;
  const uint8_t* function_start;
  bool is_function_start;
  uint32_t call_depth;
};

/**
 * Call graph used to detect recursion since we disabled detection of cycles
 * on CFG level (it was replaced with heuristic which cannot detect recursion).
 */
class CallGraph {
  struct CallEntry;
 public:
  CallGraph(CallGraph&& other) = default;
  CallGraph& operator=(CallGraph&& other) = default;

  static std::unique_ptr<CallGraph> CreateNew() {
    return std::unique_ptr<CallGraph>(new CallGraph);
  }

  static bool HasCycles(std::unique_ptr<CallGraph> graph);

  void AddCall(const uint8_t* caller, const uint8_t* callee);

 private:
  enum class NodeState {
    kNotVisited,     // Not visited node.
    kInCurrentPath,  // Already visited during current sub-path.
    kAlreadyChecked, // Node from checked subgraph.
  };

  struct CallEntry {
    CallEntry(const uint8_t* call_entry_address)
        : call_entry_addr(call_entry_address) {}

    void AddCallee(CallEntry* callee) {
      callees.insert(callee);
    }

    const uint8_t* const call_entry_addr;
    std::unordered_set<CallEntry*> callees;
    NodeState state = NodeState::kNotVisited;
  };

  DISALLOW_COPY_AND_ASSIGN(CallGraph);
  CallGraph() = default;

  bool SubgraphCheckCycles(CallEntry* node);
  CallEntry* GetOrAddCallEntry(const uint8_t* entry_start_address);

  CallEntry* root = nullptr;
  std::unordered_map<const uint8_t*, std::unique_ptr<CallEntry>> entries;
};

enum class AnalysisResult {
  kFast,
  kHasLocks,
  kHasCycles,
  kHasInterrupts,
  kHasIndirectCalls,
  kHasIndirectJumps,
  kHasUnknownInstructions,
  kCallDepthLimitExceeded,
  kBasicBlockLimitExceeded,
  kInstructionLimitExceeded,
};

inline const char* AnalysisResultToStr(AnalysisResult res) {
  switch (res) {
    case AnalysisResult::kFast:
      return "fast";
    case AnalysisResult::kHasLocks:
      return "has locks";
    case AnalysisResult::kHasCycles:
      return "has cycles";
    case AnalysisResult::kHasInterrupts:
      return "has interrupts";
    case AnalysisResult::kHasIndirectCalls:
      return "has indirect calls";
    case AnalysisResult::kHasIndirectJumps:
      return "has indirect jumps";
    case AnalysisResult::kHasUnknownInstructions:
      return "has unknown instructions";
    case AnalysisResult::kCallDepthLimitExceeded:
      return "exceeds call depth limit";
    case AnalysisResult::kBasicBlockLimitExceeded:
      return "exceeds basic block limit";
    case AnalysisResult::kInstructionLimitExceeded:
      return "exceeds instruction limit";
    default:
      LOG(ERROR) << "Unknown auto fast JNI analysis result!";
      return "";
  }
}

/**
 * CFGraph Class has information about the Control Flow Graph.
 */
class CFGraph {
 public:
  explicit CFGraph(std::string method_name)
      : method_name_(method_name) {}

  ~CFGraph() {
    for (auto it : cfg_bblock_list_) {
      delete it;
    }
  }

  /**
   * @brief Delete a certain Basic Block & Add a certain Basic Block from & to a
   * list of Basic Blocks respectively.
   * @param bblock_list - the list from which a Basic Block has to deleted &
   * to which a Basic Block has to be added.
   * @param bblock_to_be_deleted - the Basic Block to be deleted.
   * @param bblock_to_be_added - the Basic Block to de added.
   */
  void ChangePredecessors(const std::vector<MachineBlock*> &bblock_list,
                          MachineBlock* bblock_to_be_deleted,
                          MachineBlock* bblock_to_be_added);

  /**
   * In case a Basic Block is a dummy, returns it's predecessor.
   * @param bblock - the Basic Block which is being tested.
   * @return bblock itself if not dummy; else returns it's predecessor.
   */
  MachineBlock* GetCorrectBB(MachineBlock* bblock);

  /**
   * @brief Get the name of the method being analyzed.
   * @return the method's PrettyName.
   */
  const std::string& GetMethodName() const {
    return method_name_;
  }

  void SetHasUnknownInstructions() {
    state_ = AnalysisResult::kHasUnknownInstructions;
  }

  void SetHasCycles() {
    state_ = AnalysisResult::kHasCycles;
  }

  void SetHasLocks() {
    state_ = AnalysisResult::kHasLocks;
  }

  void SetHasIndirectJumps() {
    state_ = AnalysisResult::kHasIndirectJumps;
  }

  void SetHasInterrupts() {
    state_ = AnalysisResult::kHasInterrupts;
  }

  void SetHasIndirectCalls() {
    state_ = AnalysisResult::kHasIndirectCalls;
  }

  /**
   * @brief Get the levels of call nesting.
   * @return the levels of call nesting.
   */
  uint32_t GetCallDepth() const {
    return call_depth_;
  }

  /**
   * @brief Sets the call nesting level (and checks if we are still in budget)
   * @param depth - the level of call nesting.
   */
  void SetCallDepth(uint32_t depth);

  /**
   * @brief Get the Number of Instructions in the CFG.
   * @return the number of Instructions.
   */
  uint32_t GetInstructionCnt() const {
    return num_of_instrs_;
  }

  /**
   * @brief Increase instruction count by the given amount (and check if we are still in budget).
   * @param amount - amount of added instructions.
   */
  void IncreaseInstructionCnt(uint32_t amount);

  /**
   * @brief Get the Number of Basic Blocks in the CFG.
   * @return the number of Basic Blocks.
   */
  uint32_t GetBBlockCnt() const {
    return num_of_bblocks_;
  }

  /**
   * @brief Increment basic block count (and check if we are still in budget)
   */
  void IncBBlockCnt();

  /**
   * @brief Add a Basic Block as the Starting Basic Block for CFG.
   * @param start_bblock - the starting Basic Block.
   */
  void AddStartBBlock(MachineBlock* start_bblock) {
    start_bblock_ = start_bblock;
  }

  /**
   * @brief Creates a Basic Block in the CFG & updates the predecessor.
   * @param predecessor_bblock - Predecessor Basic Block.
   * @return the created Basic Block.
   */
  MachineBlock* CreateBBlock(MachineBlock* predecessor_bblock, const uint8_t* function_start);

  /**
   * @brief Update the Predecessors & successors caused by backlog due
   * to calls/jumps in the CFG.
   * @param old_pred - The old predecessor to be updated.
   * @param new_pred - The new predecessor.
   * @param backlog - List of backlog Data Structure.
   */
  void ChangePredForBacklog(MachineBlock* old_pred,
                            MachineBlock* new_pred,
                            std::vector<BackLogDs*>* backlog);

  /**
   * @brief Get the First Basic Block of the CFG.
   * @return The first Basic Block.
   */
  MachineBlock* GetStartBBlock() {
    return start_bblock_;
  }

  /**
   * @brief Keeps track of visited Basic Blocks.
   * @param bblock - the Basic Block that was visited.
   * @param start - Starting instruction pointer.
   * @param end - ending instruction pointer.
   */
  void AddTuple(MachineBlock* bblock, const uint8_t* start, const uint8_t* end);

  /**
   * @brief Check whether the Basic Block was already visited.
   * @param addr - the pointer to a certain location that has to be tested.
   * @param prev_bblock - Predecessor Basic Block.
   * @return true if visited. False otherwise.
   */
  bool IsVisitedForCall(uint8_t* addr, MachineBlock* prev_bblock) const;

  /**
   * @brief Check if an entry in the backlog was analyzed already.
   * @param entry - the backlog entry.
   * @param backlog - list of backlog entries.
   * @return true of visited; false otherwise.
   */
  bool IsVisitedForBacklog(BackLogDs* entry, std::vector<BackLogDs*>* backlog);

  /**
   * @brief Check if a certain location(instruction) was already analyzed or not.
   * @param addr - The address that is being checked.
   * @param prev_bblock - Predecessor Basic Block.
   * @param succ_bblock - Successor Basic Block.
   * @param backlog - List of backlog entries.
   * @param is_function_start - True if it is the first instruction in function otherwise false.
   * @return true if analyzed already; false otherwise.
   */
  bool IsVisited(const uint8_t* addr,
                 MachineBlock* prev_bblock,
                 MachineBlock* succ_bblock,
                 std::vector<BackLogDs*>* backlog,
                 const bool is_function_start);

  /**
   * @brief prints the CFG in human readable/dot format to logcat.
   * @param output - output stream.
   * @param is_dot - whether the output must be in dot format.
   */
  void Print(std::ostringstream &output, bool is_dot) const;

  /**
   * @brief Determine if the CFG falls within the budget to call it Fast or not.
   * @return true if method's CFG satisfies budget; false otherwise.
   */
  bool IsStillFast() const;

  /**
   * @brief Get the current state of CFG analysis.
   * @return Current state of analysis.
   */
  AnalysisResult GetAnalysisState() const {
    return state_;
  }

 private:
  MachineBlock* start_bblock_;
  uint32_t num_of_bblocks_ = 0u;
  uint32_t num_of_instrs_ = 0u;
  uint32_t call_depth_ = 0u;
  AnalysisResult state_ = AnalysisResult::kFast;
  std::vector<MachineBlock*> cfg_bblock_list_;
  std::vector<MachineBlock*> visited_bblock_list_;
  std::string method_name_;
};

/**
 * @brief Analyze a method and determine whether it can be marked fast or not.
 * @param method_idx - dex method Index.
 * @param dex_file - dex File.
 * @param fn_ptr - Function pointer of method to be analyzed.
 * @return true if fast; false otherwise.
 */
AnalysisResult AnalyzeMethod(uint32_t method_idx, const DexFile& dex_file, const void* fn_ptr);

}  // namespace x86
}  // namespace art

#endif  // ART_RUNTIME_BINARY_ANALYZER_BINARY_ANALYZER_X86_H_

