/*
 * Copyright (C) 2016 Intel Corporation.
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

#include "binary_analyzer_x86.h"

#include <inttypes.h>
#include <iostream>
#include <ostream>
#include <sstream>

#include "base/logging.h"
#include "base/stringprintf.h"
#include "thread.h"

namespace art {
namespace x86 {

// Budgetary constraints for classifying a native method as fast.
static constexpr size_t kCallDepthLimit = 3;
static constexpr size_t kBasicBlockLimit = 20;
static constexpr size_t kInstructionLimit = 100;

enum ControlTransferType {
  kNone,                 // Instructions with no control flow transfer.
  kConditionalBranch,    // Conditional branch.
  kUnconditionalBranch,  // Unconditional branch.
  kInterrupt,            // Software Interrupt.
  kCall,                 // Direct call.
  kUnknown,              // Unsupported instruction.
  kIndirectCall,         // Indirect call.
  kIndirectJump,         // Indirect jump.
  kReturn,               // Return instruction.
  kLock,                 // Lock prefix.
  kCycle,                // Cycling prefix/instruction.
};

/**
 * @brief Analyze how instruction affects control flow (see ControlTransferType)
 * @param instr - The instruction pointer.
 * @param curr_bb - Pointer to the Current Basic Block.
 * @param is_bb_end - Does the instruction mark the end of Basic Block.
 * @param target - Adress for direct jump/call.
 * @param disassembler - Disassembler to be used for instruction decoding.
 * @return Size of analyzed instruction in bytes. -1 in case of error.
 */
ptrdiff_t AnalyzeInstruction(const uint8_t* instr,
                             MachineBlock* curr_bb,
                             int32_t* is_bb_end,
                             const uint8_t** target,
                             Disassembler* disassembler) {
  *is_bb_end = kNone;
  if (!disassembler->IsDisassemblerValid()) {
    return -1;
  }
  disassembler->Seek(instr);
  const cs_insn* insn = disassembler->Next();
  if (insn == nullptr) {
    return -1;
  }
  *target = instr + insn->size;
  const cs_x86& insn_x86 = insn->detail->x86;

  switch(insn_x86.prefix[0]) {
  case X86_PREFIX_REP:
  case X86_PREFIX_REPNE:
    *is_bb_end = kCycle;
    break;
  case X86_PREFIX_LOCK:
    *is_bb_end = kLock;
    break;
  }

  switch(insn->id) {
  case X86_INS_LOOP:
  case X86_INS_LOOPE:
  case X86_INS_LOOPNE:
    *is_bb_end = kCycle;
    break;
  case X86_INS_INVALID:
    *is_bb_end = kUnknown;
    break;
  case X86_INS_INT:
  case X86_INS_INT1:
  case X86_INS_INT3:
    *is_bb_end = kInterrupt;
    break;
  case X86_INS_JMP:
    DCHECK_GE(insn_x86.op_count, 1);
    if (insn_x86.operands[0].type != X86_OP_IMM) {
      *is_bb_end = kIndirectJump;
    } else {
      *is_bb_end = kUnconditionalBranch;
      *target = reinterpret_cast<uint8_t*>(insn_x86.operands[0].imm);
    }
    break;
  case X86_INS_JA:
  case X86_INS_JAE:
  case X86_INS_JB:
  case X86_INS_JBE:
  case X86_INS_JCXZ:
  case X86_INS_JE:
  case X86_INS_JECXZ:
  case X86_INS_JG:
  case X86_INS_JGE:
  case X86_INS_JL:
  case X86_INS_JLE:
  case X86_INS_JNE:
  case X86_INS_JNO:
  case X86_INS_JNP:
  case X86_INS_JNS:
  case X86_INS_JO:
  case X86_INS_JP:
  case X86_INS_JRCXZ:
  case X86_INS_JS:
    DCHECK_GE(insn_x86.op_count, 1);
    if (insn_x86.operands[0].type != X86_OP_IMM) {
      *is_bb_end = kIndirectJump;
    } else {
      *is_bb_end = kConditionalBranch;
      *target = reinterpret_cast<uint8_t*>(insn_x86.operands[0].imm);
    }
    break;
  case X86_INS_RET:
    *is_bb_end = kReturn;
    break;
  case X86_INS_CALL:
    DCHECK_GE(insn_x86.op_count, 1);
    if (insn_x86.operands[0].type != X86_OP_IMM) {
      *is_bb_end = kIndirectCall;
    } else {
      *is_bb_end = kCall;
      *target = reinterpret_cast<uint8_t*>(insn_x86.operands[0].imm);
    }
    break;
  }
  MachineInstruction* ir = new MachineInstruction(std::string(insn->mnemonic) + " " + insn->op_str,
                                                  static_cast<uint8_t>(insn->size),
                                                  reinterpret_cast<const uint8_t*>(instr));
  MachineInstruction* prev_ir = curr_bb->GetLastInstruction();
  ir->SetPrevInstruction(prev_ir);
  if (prev_ir != nullptr) {
    prev_ir->SetNextInstruction(ir);
  }
  curr_bb->AddInstruction(ir);
  return insn->size;
}

MachineBlock* CFGraph::GetCorrectBB(MachineBlock* bblock) {
  if (bblock->IsDummy()) {
    if (!bblock->GetPredBBlockList().empty()) {
      return bblock->GetPredBBlockList().front();
    } else {
      return nullptr;
    }
  } else {
    return bblock;
  }
}

bool CFGraph::IsVisited(const uint8_t* addr,
                        MachineBlock* prev_bblock,
                        MachineBlock* succ_bblock,
                        std::vector<BackLogDs*>* backlog,
                        const bool is_function_start) {
  for (const auto current_bb : visited_bblock_list_) {
    if (!current_bb->IsDummy()) {
      const uint8_t* start = current_bb->GetStartAddr();
      const uint8_t* end = current_bb->GetEndAddr();
      // Recognize it as a cycle only if a back-branch found and it was a jmp, not call.
      if (addr <= prev_bblock->GetEndAddr() && !is_function_start) {
        this->SetHasCycles();
      }

      // In case we are branching to the beginning of an existing Basic Block,
      // it is already visited.
      if (addr == start) {
        MachineBlock* bb_existing = current_bb;
        if (prev_bblock->IsDummy()) {
          prev_bblock->AddSuccBBlock(bb_existing);
          bb_existing->AddPredBBlock(prev_bblock);
          succ_bblock->AddPredBBlock(bb_existing);
          bb_existing->AddSuccBBlock(succ_bblock);
        } else {
          prev_bblock->AddSuccBBlock(bb_existing);
          bb_existing->AddPredBBlock(prev_bblock);
        }
        return true;
      }

      // In case we are branching to some address in the middle of an existing Basic Block,
      // then that Basic Block needs to be split.
      if ((addr > start) && (addr < end)) {
        MachineBlock* bb_to_be_split = current_bb;

        const uint8_t* prev_instr = nullptr;
        const uint8_t* curr_instr = start;

        // Scan the bblock that is being split to identify the end instruction.
        const auto& instructions = bb_to_be_split->GetInstructions();

        for (auto it_instr : instructions) {
          prev_instr = curr_instr;
          curr_instr = reinterpret_cast<const uint8_t*>(curr_instr + it_instr->GetLength());

          // The previous instruction is the last instruction of the Basic Block being split.
          if (curr_instr == addr) {
            // Change the last instruction.
            bb_to_be_split->SetEndAddr(prev_instr);
            MachineBlock* new_bb = CreateBBlock(nullptr, nullptr);
            const auto& succ_list = bb_to_be_split->GetSuccBBlockList();
            bb_to_be_split->ClearSuccBBlockList();
            bb_to_be_split->AddSuccBBlock(new_bb);
            new_bb->AddPredBBlock(bb_to_be_split);
            new_bb->CopySuccBBlockList(succ_list);
            if (prev_bblock->IsDummy()) {
              prev_bblock->AddSuccBBlock(new_bb);
              new_bb->AddPredBBlock(prev_bblock);
              succ_bblock->AddPredBBlock(new_bb);
              new_bb->AddSuccBBlock(succ_bblock);
            } else {
              prev_bblock->AddSuccBBlock(new_bb);
              new_bb->AddPredBBlock(prev_bblock);
            }
            AddTuple(new_bb, reinterpret_cast<const uint8_t*>(addr), end);
            new_bb->CopyInstruction(bb_to_be_split, addr);
            ChangePredForBacklog(bb_to_be_split, new_bb, backlog);
            ChangePredecessors(succ_list, bb_to_be_split, new_bb);
            if (prev_bblock == bb_to_be_split) {
              new_bb->AddPredBBlock(new_bb);
              new_bb->AddSuccBBlock(new_bb);
            }
            return true;
          } else if (curr_instr > addr) {
            return false;
          }
        }
      }
    }
  }
  // Else, we have not visited this earlier.
  return false;
}

void CFGraph::ChangePredecessors(const std::vector<MachineBlock*>& bblock_list,
                                 MachineBlock* bblock_to_be_deleted,
                                 MachineBlock* bblock_to_be_added) {
  for (const auto it : bblock_list) {
    it->DeletePredBBlock(bblock_to_be_deleted);
    it->AddPredBBlock(bblock_to_be_added);
  }
}

void CFGraph::ChangePredForBacklog(MachineBlock* old_pred,
                                   MachineBlock* new_pred,
                                   std::vector<BackLogDs*>* backlog) {
  for (const auto bb : *backlog) {
    if (bb->pred_bb->GetId() == old_pred->GetId()) {
      bb->pred_bb = new_pred;
      return;
    }
  }
}

void MachineBlock::CopyInstruction(MachineBlock* from_bblock, const uint8_t* start) {
  const uint8_t* ptr = from_bblock->GetStartAddr();
  std::list<MachineInstruction*>::iterator it_bb = from_bblock->instrs_.begin();
  while (it_bb != from_bblock->instrs_.end()) {
    if (ptr >= start) {
      AddInstruction((*it_bb));
      ptr += (*it_bb)->GetLength();
      from_bblock->DeleteInstruction(it_bb++);
    } else {
      ptr += (*it_bb)->GetLength();
      it_bb++;
    }
  }
}

void MachineBlock::DeleteInstruction(std::list<MachineInstruction*>::iterator it) {
  instrs_.erase(it);
  --num_of_instrs_;
}

bool CFGraph::IsVisitedForBacklog(BackLogDs* entry, std::vector<BackLogDs*>* backlog) {
  if (entry != nullptr) {
    MachineBlock* prev_bb = entry->pred_bb;
    const uint8_t* addr = entry->ptr;
    const bool is_function_start = entry->is_function_start;

    if (prev_bb != nullptr) {
      return IsVisited(addr, prev_bb, entry->succ_bb, backlog, is_function_start);
    }
  }
  return false;
}

MachineBlock* GetTailBB(MachineBlock* bblock) {
  MachineBlock* first_succ_bb = bblock->GetSuccBBlockList().front();
  if (first_succ_bb == nullptr) {
    return bblock;
  } else {
    return first_succ_bb;
  }
}

/**
 * @brief The Helper function to build CFG for the given method.
 * @param cfg - The CFG.
 * @param ptr - Instruction Pointer.
 * @param curr_bb - Current Basic Block.
 * @param backlog - Backlog array.
 * @param depth - Call depth/levels of call nesting.
 * @param dummy_end - Dummy basic block.
 * @param disasm - Disassembler to be used for instructions decoding.
 */
void CFGHelper(CFGraph* cfg,
               const uint8_t* instr_ptr,
               MachineBlock* curr_bblock,
               std::vector<BackLogDs*>* backlog,
               uint32_t depth,
               MachineBlock* dummy_end,
               Disassembler* disasm,
               CallGraph* call_graph) {
  ptrdiff_t len = 0;
  int32_t is_bb_end = kNone;
  const uint8_t* target = nullptr;
  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(instr_ptr);
  const uint8_t* start_ptr = reinterpret_cast<const uint8_t*>(ptr);

  while ((len = AnalyzeInstruction(ptr, curr_bblock, &is_bb_end, &target, disasm)) > 0) {
    switch (is_bb_end) {
    case kUnconditionalBranch: {
      // Push the jmp target to backlog.
      BackLogDs* uncond_jmp = new BackLogDs();
      uncond_jmp->function_start = curr_bblock->GetFunctionStartAddr();
      uncond_jmp->pred_bb = curr_bblock;
      if (depth > 0) {
        uncond_jmp->succ_bb = dummy_end;
      } else {
        uncond_jmp->succ_bb = nullptr;
      }
      uncond_jmp->ptr = target;
      uncond_jmp->is_function_start = false;
      uncond_jmp->call_depth = 0;
      backlog->push_back(uncond_jmp);
      // Add the current BB to CFG.
      cfg->AddTuple(curr_bblock, start_ptr, reinterpret_cast<const uint8_t*>(ptr));
      break;
    }
    case kConditionalBranch: {
      // Push the conditional (if) jmp target to backlog.
      BackLogDs* cond_if_jmp = new BackLogDs();
      cond_if_jmp->pred_bb = curr_bblock;
      cond_if_jmp->function_start = curr_bblock->GetFunctionStartAddr();
      if (depth > 0) {
        cond_if_jmp->succ_bb = dummy_end;
      } else {
        cond_if_jmp->succ_bb = nullptr;
      }
      cond_if_jmp->ptr = target;
      cond_if_jmp->call_depth = 0;
      cond_if_jmp->is_function_start = false;
      backlog->push_back(cond_if_jmp);
      // Push the else (subsequent instruction) to the backlog.
      BackLogDs* cond_else_jmp = new BackLogDs();
      cond_else_jmp->is_function_start = false;
      cond_else_jmp->pred_bb = curr_bblock;
      cond_else_jmp->function_start = curr_bblock->GetFunctionStartAddr();
      if (depth > 0) {
        cond_else_jmp->succ_bb = dummy_end;
      } else {
        cond_else_jmp->succ_bb = nullptr;
      }
      cond_else_jmp->ptr = reinterpret_cast<const uint8_t*>(ptr + len);
      cond_else_jmp->call_depth = 0;
      backlog->push_back(cond_else_jmp);
      // Add the current Basic Block to the CFG.
      cfg->AddTuple(curr_bblock, start_ptr, reinterpret_cast<const uint8_t*>(ptr));
      break;
    }
    case kCall: {
      // Add the current Basic Block to the CFG.
      cfg->AddTuple(curr_bblock, start_ptr, reinterpret_cast<const uint8_t*>(ptr));
      MachineBlock* start_bb = cfg->CreateBBlock(curr_bblock, curr_bblock->GetFunctionStartAddr());
      MachineBlock* end_bb = cfg->CreateBBlock(nullptr, curr_bblock->GetFunctionStartAddr());
      start_bb->SetDummy();
      end_bb->SetDummy();
      BackLogDs* call_entry = new BackLogDs();
      call_entry->pred_bb = start_bb;
      call_entry->ptr = target;
      call_entry->function_start = target;
      call_entry->succ_bb = end_bb;
      call_entry->is_function_start = true;
      call_entry->call_depth = depth + 1;
      if (call_entry->call_depth > cfg->GetCallDepth()) {
        cfg->SetCallDepth(call_entry->call_depth);
      }
      backlog->push_back(call_entry);
      MachineBlock* bb_after_call = cfg->CreateBBlock(end_bb, curr_bblock->GetFunctionStartAddr());
      curr_bblock = bb_after_call;
      is_bb_end = kNone;
      start_ptr = ptr + len;
      cfg->AddTuple(start_bb, nullptr, nullptr);
      cfg->AddTuple(end_bb, nullptr, nullptr);

      call_graph->AddCall(curr_bblock->GetFunctionStartAddr(), target);
      break;
    }
    case kReturn: {
      if (dummy_end != nullptr) {
        dummy_end->AddPredBBlock(curr_bblock);
        curr_bblock->AddSuccBBlock(dummy_end);
      }
      cfg->AddTuple(curr_bblock, start_ptr, reinterpret_cast<const uint8_t*>(ptr));
      break;
    }
    case kInterrupt: {
      cfg->SetHasInterrupts();
      cfg->AddTuple(curr_bblock, start_ptr, reinterpret_cast<const uint8_t*>(ptr));
      break;
    }
    case kIndirectCall: {
      cfg->SetHasIndirectCalls();
      cfg->AddTuple(curr_bblock, start_ptr, reinterpret_cast<const uint8_t*>(ptr));
      break;
    }
    case kIndirectJump: {
      cfg->SetHasIndirectJumps();
      cfg->AddTuple(curr_bblock, start_ptr, reinterpret_cast<const uint8_t*>(ptr));
      break;
    }
    case kUnknown: {
      cfg->SetHasUnknownInstructions();
      cfg->AddTuple(curr_bblock, start_ptr, reinterpret_cast<const uint8_t*>(ptr));
      break;
    }
    case kLock: {
      cfg->SetHasLocks();
      cfg->AddTuple(curr_bblock, start_ptr, reinterpret_cast<const uint8_t*>(ptr));
      break;
    }
    case kCycle: {
      cfg->SetHasCycles();
      cfg->AddTuple(curr_bblock, start_ptr, reinterpret_cast<const uint8_t*>(ptr));
      break;
    }
    }

    ptr += len;
    if (is_bb_end != kNone && is_bb_end != kCall) {
      break;
    }
  }
}

/**
 * @brief Constructs the CFG for the method by binary analysis.
 * @param ptr - the method function pointer.
 * @param method_name - Pretty Name of method.
 * @return the CFG for the analyzed method.
 */
AnalysisResult AnalyzeCFG(const uint8_t* ptr, const std::string& method_name) {
  CFGraph cfg(method_name);
  auto call_graph = CallGraph::CreateNew();
  Disassembler disassembler(Runtime::Current()->GetInstructionSet());
  MachineBlock* predecessor_bb = nullptr;
  MachineBlock* start_bb = cfg.CreateBBlock(predecessor_bb, nullptr);
  MachineBlock* curr_bb = start_bb;
  cfg.AddStartBBlock(start_bb);
  MachineBlock* dummy_end = nullptr;
  uint32_t depth = 0;
  std::vector<BackLogDs*> backlog;
  CFGHelper(&cfg, ptr, curr_bb, &backlog, depth, dummy_end, &disassembler, call_graph.get());
  do {
    BackLogDs* entry = nullptr;
    if (!backlog.empty()) {
      entry = backlog.back();
      backlog.pop_back();
      if (!cfg.IsVisitedForBacklog(entry, &backlog)) {
        ptr = entry->ptr;
        predecessor_bb  = entry->pred_bb;
        curr_bb = cfg.CreateBBlock(predecessor_bb, entry->function_start);
        dummy_end = entry->succ_bb;
        depth = entry->call_depth;
        CFGHelper(&cfg, ptr, curr_bb, &backlog, depth, dummy_end, &disassembler, call_graph.get());
      }
    }
    delete entry;
    if (!cfg.IsStillFast()) {
      for (auto& e : backlog) {
        delete e;
      }
      return cfg.GetAnalysisState();
    }
  } while ((!backlog.empty()));
  if (CallGraph::HasCycles(std::move(call_graph))) {
    return AnalysisResult::kHasCycles;
  }
  return cfg.GetAnalysisState();
}

bool CFGraph::IsStillFast() const {
  return state_ == AnalysisResult::kFast;
}

void MachineInstruction::Print(std::ostream& os, bool is_dot) {
  os << Print(is_dot).str();
}

void ReplaceString(std::string& subject,
                   const std::string& search,
                   const std::string& replace) {
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }
}

std::ostringstream MachineInstruction::Print(bool is_dot) {
  std::ostringstream os;
  if (is_dot) {
    ReplaceString(instr_, "0x", "");
    os << instr_ << "\\l";
  } else {
    os << "\t\t" << instr_ << std::endl;
  }
  return os;
}

void MachineBlock::Print(std::ostream& os, bool is_dot) {
  os << Print(is_dot).str();
}

std::ostringstream MachineBlock::Print(bool is_dot) {
  std::ostringstream os;
  if (is_dot) {
    std::string label = StringPrintf("BB#%d\\n", id_);
    if (!is_dummy_) {
      std::ostringstream o_instr;
      for (auto it : instrs_) {
        it->Print(o_instr, true);
      }
      label = label + o_instr.str();
    } else {
      label = label + " (Dummy)";
    }
    os << StringPrintf("\nB_%d [shape=rectangle, label=\"%s\"];", id_, label.c_str());
    for (MachineBlock* bb : succ_bblock_) {
      os << StringPrintf("\nB_%d -> B_%d;", id_, bb->GetId());
    }
  } else {
    os << "   Basic Block Id : " << id_
       << "\n Call entry: 0x" << std::hex << size_t(function_start_addr_)
       << "\n BB -No. of instructions " << GetInstrCnt()
        << "\n    List of predecessor BBs : ";
    for (auto it : pred_bblock_) {
      os << it->GetId() << "   ";
    }
    os << "\n    List of successor BBs : ";
    for (auto it : succ_bblock_) {
      os << it->GetId() << "   ";
    }
    if (!is_dummy_) {
      os << "\n   Instructions begin at : " << StringPrintf("%p", start_addr_) << "\n";
      for (auto it : instrs_) {
        it->Print(os, false);
      }
      os << "   Instructions end at : " << StringPrintf("%p", end_addr_) << "\n";
    } else {
      os << "\n Dummy BB \n";
    }
  }
  return os;
}

void CFGraph::Print(std::ostringstream& os, bool is_dot) const {
  if (is_dot) {
    std::string method_name = GetMethodName();
    ReplaceString(method_name, ".", "_");
    ReplaceString(method_name, " ", "_");
    ReplaceString(method_name, ",", "_");
    ReplaceString(method_name, ")", "_");
    ReplaceString(method_name, "(", "_");
    os << "\ndigraph G_" << method_name <<" {";
  } else {
    os << "--- CFG begins ---\nCFG -No. of instructions " << GetInstructionCnt();
  }
  for (auto bb : visited_bblock_list_) {
    bb->Print(os, is_dot);
  }
  if (is_dot) {
    os << std::endl << "}";
  } else {
    os << "--- CFG ends ---\n";
  }
}

MachineBlock::~MachineBlock() {
  for (auto it : instrs_) {
    delete it;
  }
}

AnalysisResult AnalyzeMethod(uint32_t method_idx, const DexFile& dex_file, const void* fn_ptr) {
  return AnalyzeCFG((unsigned char*) fn_ptr, PrettyMethod(method_idx, dex_file));
}

void MachineBlock::AddPredBBlock(MachineBlock* bblock) {
  auto it = std::find(pred_bblock_.begin(), pred_bblock_.end(), bblock);
  if (it == pred_bblock_.end()) {
    pred_bblock_.push_back(bblock);
  }
  if (function_start_addr_ == nullptr && bblock != nullptr) {
    function_start_addr_ = bblock->GetFunctionStartAddr();
  }
}

void MachineBlock::AddSuccBBlock(MachineBlock* bblock) {
  auto it = std::find(succ_bblock_.begin(), succ_bblock_.end(), bblock);
  if (it == succ_bblock_.end()) {
    succ_bblock_.push_back(bblock);
  }
}

void MachineBlock::DeletePredBBlock(MachineBlock* bblock) {
  for (auto it = pred_bblock_.begin();
      it != pred_bblock_.end();) {
    if ((*it) == bblock) {
      it = pred_bblock_.erase(it);
    } else {
      it++;
    }
  }
}

void MachineBlock::CopyPredBBlockList(const std::vector<MachineBlock*>& to_copy) {
  for (auto it : to_copy) {
    pred_bblock_.push_back(it);
  }
}

void MachineBlock::CopySuccBBlockList(const std::vector<MachineBlock*>& to_copy) {
  for (auto it : to_copy) {
    succ_bblock_.push_back(it);
  }
}

void CFGraph::SetCallDepth(uint32_t depth) {
  call_depth_ = depth;
  if (call_depth_ >= kCallDepthLimit) {
    state_ = AnalysisResult::kCallDepthLimitExceeded;
  }
}

void CFGraph::IncBBlockCnt() {
  ++num_of_bblocks_;
  if (num_of_bblocks_ > kBasicBlockLimit) {
    state_ = AnalysisResult::kBasicBlockLimitExceeded;
  }
}

void CFGraph::IncreaseInstructionCnt(uint32_t amount) {
  num_of_instrs_ += amount;
  if (num_of_instrs_ > kInstructionLimit) {
    state_ = AnalysisResult::kInstructionLimitExceeded;
  }
}

MachineBlock* CFGraph::CreateBBlock(MachineBlock* predecessor_bb, const uint8_t* function_start) {
  MachineBlock* new_bb = new MachineBlock(predecessor_bb, function_start);
  new_bb->SetId(GetBBlockCnt());
  new_bb->SetStartAddr(nullptr);
  new_bb->SetEndAddr(nullptr);
  IncBBlockCnt();
  cfg_bblock_list_.push_back(new_bb);
  return new_bb;
}

void CFGraph::AddTuple(MachineBlock* bblock, const uint8_t* start, const uint8_t* end) {
  bblock->SetStartAddr(start);
  bblock->SetEndAddr(end);
  visited_bblock_list_.push_back(bblock);
  IncreaseInstructionCnt(bblock->GetInstrCnt());
}

bool CallGraph::HasCycles(std::unique_ptr<CallGraph> graph) {
  if (graph->root == nullptr) {
    return false;
  }
  return graph->SubgraphCheckCycles(graph->root);
}

void CallGraph::AddCall(const uint8_t* caller, const uint8_t* callee) {
  auto caller_entry = GetOrAddCallEntry(caller);
  auto callee_entry = GetOrAddCallEntry(callee);
  if (root == nullptr) {
    root = caller_entry;
  }
  caller_entry->AddCallee(callee_entry);
}

bool CallGraph::SubgraphCheckCycles(CallEntry* node) {
  // The classic algorithm for checking a directed graph for the presence of cycles in it.
  if (node->state == NodeState::kAlreadyChecked) {
    return false;
  }
  if (node->state == NodeState::kInCurrentPath) {
    return true;
  }

  node->state = NodeState::kInCurrentPath;
  for (auto& child : node->callees) {
    if (SubgraphCheckCycles(child)) {
      return true;
    }
  }

  node->state = NodeState::kAlreadyChecked;
  return false;
}

CallGraph::CallEntry* CallGraph::GetOrAddCallEntry(const uint8_t* entry_start_address) {
  auto it = entries.find(entry_start_address);
  if (it != entries.end()) {
    return it->second.get();
  }
  auto new_entry = std::unique_ptr<CallEntry>(new CallEntry(entry_start_address));
  auto new_entry_ptr = new_entry.get();
  entries[entry_start_address] = std::move(new_entry);
  return new_entry_ptr;
}

}  // namespace x86
}  // namespace art
