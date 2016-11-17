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

#include "graph_x86.h"

#include "builder.h"
#include "ext_utility.h"
#include "loop_iterators.h"
#include "pretty_printer.h"
#include "ssa_builder.h"
#include "scoped_thread_state_change.h"

namespace art {

void HGraph_X86::Dump() {
  for (HBasicBlock* block : blocks_) {
    LOG(INFO) << "Block " << block->GetBlockId() << " has LoopInformation " << block->GetLoopInformation();
  }

  StringPrettyPrinter printer(this);
  printer.VisitInsertionOrder();
  const std::string& print_string = printer.str();

  LOG(INFO) << print_string;
}

void HGraph_X86::DeleteBlock(HBasicBlock* block, bool remove_from_loops) {
  // Remove all Phis.
  for (HInstructionIterator it2(block->GetPhis()); !it2.Done(); it2.Advance()) {
    HInstruction* insn = it2.Current();
    RemoveAsUser(insn);
    RemoveFromEnvironmentUsers(insn);
    block->RemovePhi(insn->AsPhi(), false);
  }

  // Remove the rest of the instructions.
  for (HInstructionIterator it2(block->GetInstructions()); !it2.Done(); it2.Advance()) {
    HInstruction* insn = it2.Current();
    RemoveAsUser(insn);
    RemoveFromEnvironmentUsers(insn);
    block->RemoveInstruction(insn, false);
  }

  // Remove all successors from the block.
  const ArenaVector<HBasicBlock*>& successors = block->GetSuccessors();
  for (size_t j = successors.size(); j > 0; j--) {
    HBasicBlock* successor = successors[j - 1];
    if (std::find(successor->GetPredecessors().begin(),
                  successor->GetPredecessors().end(),
                  block) != successor->GetPredecessors().end()) {
      successor->RemovePredecessor(block);
    }
    block->RemoveSuccessor(successor);
  }

  // Remove all predecessors.
  block->ClearAllPredecessors();

  // Remove all data structures pointing to the block.
  blocks_[block->GetBlockId()] = nullptr;
  RemoveElement(reverse_post_order_, block);
  if (linear_order_.size() > 0) {
    RemoveElement(linear_order_, block);
  }

  if (remove_from_loops) {
    for (auto loop = LOOPINFO_TO_LOOPINFO_X86(block->GetLoopInformation());
         loop != nullptr;
         loop = loop->GetParent()) {
      if (loop->Contains(*block)) {
        if (loop->IsBackEdge(*block)) {
          loop->RemoveBackEdge(block);
        }
        loop->Remove(block);
      }
    }
  }
}

void HGraph_X86::CreateLinkBetweenBlocks(HBasicBlock* existing_block,
                                         HBasicBlock* block_being_added,
                                         bool add_as_dominator,
                                         bool add_after) {
  if (add_after) {
    existing_block->AddSuccessor(block_being_added);
  } else {
    block_being_added->AddSuccessor(existing_block);
  }

  if (add_as_dominator) {
    if (add_after) {
      // Fix the domination information.
      block_being_added->SetDominator(existing_block);
      existing_block->AddDominatedBlock(block_being_added);
    } else {
      // Fix the domination information.
      existing_block->SetDominator(block_being_added);
      block_being_added->AddDominatedBlock(existing_block);
    }
  }

  // Fix reverse post ordering.
  size_t index = IndexOfElement(reverse_post_order_, existing_block);
  MakeRoomFor(&reverse_post_order_, 1, index);
  if (add_after) {
    reverse_post_order_[index + 1] = block_being_added;
  } else {
    reverse_post_order_[index] = block_being_added;
    reverse_post_order_[index + 1] = existing_block;
  }
}

void HGraph_X86::SplitCriticalEdgeAndUpdateLoopInformation(HBasicBlock* from, HBasicBlock* to) {
  // Remember index, to find a new added splitter.
  size_t index = to->GetPredecessorIndexOf(from);

  // First split.
  SplitCriticalEdge(from, to);

  // Find splitter.
  HBasicBlock* splitter = to->GetPredecessors()[index];

  // Set loop information for splitter.
  HLoopInformation* loop_information = to->IsLoopHeader() ?
          from->GetLoopInformation() : to->GetLoopInformation();
  if (loop_information != nullptr) {
    LOOPINFO_TO_LOOPINFO_X86(loop_information)->AddToAll(splitter);
  }
}

void HGraph_X86::RebuildDomination() {
  ClearDominanceInformation();
  ComputeDominanceInformation();
  ComputeTryBlockInformation();
}

void HGraph_X86::MovePhi(HPhi* phi, HBasicBlock* to_block) {
  DCHECK(phi != nullptr);
  HBasicBlock* from_block = phi->GetBlock();
  if (from_block != to_block) {
    from_block->phis_.RemoveInstruction(phi);
    to_block->phis_.AddInstruction(phi);
    phi->SetBlock(to_block);
  }
}

void HGraph_X86::MoveInstructionBefore(HInstruction* instr, HInstruction* cursor) {
  DCHECK(instr != nullptr);
  HBasicBlock* from_block = instr->GetBlock();
  DCHECK(cursor != nullptr);
  DCHECK(!cursor->IsPhi());
  HBasicBlock* to_block = cursor->GetBlock();
  DCHECK(from_block != to_block);

  // Disconnect from the old block.
  from_block->RemoveInstruction(instr, false);

  // Connect up to the new block.
  DCHECK_NE(instr->GetId(), -1);
  DCHECK_NE(cursor->GetId(), -1);
  DCHECK(!instr->IsControlFlow());
  instr->SetBlock(to_block);
  to_block->instructions_.InsertInstructionBefore(instr, cursor);
}

HGraph_X86* CreateX86CFG(ArenaAllocator* allocator,
                         const uint16_t* data,
                         Primitive::Type return_type) {
  DexFile* df = reinterpret_cast<DexFile*>(allocator->Alloc(sizeof(DexFile)));
  HGraph_X86* graph = new (allocator) HGraph_X86(allocator, *df, -1, false, kRuntimeISA);
  if (data != nullptr) {
    ScopedObjectAccess soa(Thread::Current());
    StackHandleScopeCollection handles(soa.Self());
    const DexFile::CodeItem* item = reinterpret_cast<const DexFile::CodeItem*>(data);
    HGraphBuilder builder(graph, *item, &handles, return_type);
    bool graph_built = (builder.BuildGraph() == kAnalysisSuccess);
    return graph_built ? graph : nullptr;
  }
  return graph;
}

bool HGraph_X86::GetColdBlocks(std::set<HBasicBlock*>& blocks) const {
  if (GetProfileCountKind() != kBasicBlockCounts &&
      !(Runtime::Current() != nullptr && Runtime::Current()->UseJitCompilation())) {
    // We don't have enough information to tell.
    return false;
  }

  if (!entry_block_->HasBlockCount() || entry_block_->GetBlockCount() == 0) {
    // Still not enough to tell.
    return false;
  }

  // A cold block is executed at least kColdBlockFactor times LESS than the method.
  const uint64_t entry_count = entry_block_->GetBlockCount();
  uint64_t cold_count = entry_count / kColdBlockFactor;

  if (entry_count <= kColdBlockFactor) {
    // Minimal assumption.  Execution counts of 0 are cold blocks.
    cold_count = 1;
  }

  // Add each cold block in the method to the set of cold blocks.
  for (HBasicBlock* block : blocks_) {
    if (block == nullptr) {
      continue;
    }

    // We don't want Goto blocks to be marked as cold, as they aren't generated.
    // IsSingleGoto isn't quite what we need.
    if (block->GetPhis().IsEmpty() &&
        block->GetFirstInstruction() == block->GetLastInstruction() &&
        block->GetLastInstruction()->IsGoto()) {
      continue;
    }

    // Ignore blocks without counts.
    if (!block->HasBlockCount()) {
      continue;
    }

    // If we are less frequent than the 'cold count' then we are a cold block.
    // Ignore blocks that have no count, as they may be false alarms.
    uint64_t block_count = block->GetBlockCount();
    if (block_count < cold_count) {
      blocks.insert(block);
      continue;
    }

    // We are also cold if we are in a loop execute much less frequently than
    // the loop header.
    HLoopInformation* loop_info = block->GetLoopInformation();
    if (loop_info != nullptr) {
      HBasicBlock* header = loop_info->GetHeader();
      if (header->HasBlockCount()) {
        uint64_t header_count = header->GetBlockCount();
        if (block_count < header_count / kColdLoopBlockFactor) {
          blocks.insert(block);
          continue;
        }
      }
    }
  }

  // Did we find any blocks?
  return !blocks.empty();
}

static SetBoolValue should_dump("dex2oat.bb.dump", "BB_DUMP");

static void DumpBlocks(const char* msg,
                       HGraph* graph,
                       ArenaVector<HBasicBlock*>& blocks) {
  std::ostringstream s;
  for (HBasicBlock* bb : blocks) {
    if (bb == nullptr) {
      continue;
    }
    s << "BB" << bb->GetBlockId() << ": ";
    if (bb->HasBlockCount()) {
      s << bb->GetBlockCount() << ' ';
    } else {
      s << "? ";
    }
  }

  LOG(INFO) << msg << " for " << GetMethodName(graph) << ": " << s.str();
}

void HGraph_X86::UpdateBlockOrder() {
  // If we have BB counts, move the cold blocks to the end of the method.
  // For cold loop blocks, move them to after the loop.
  std::set<HBasicBlock*> cold_blocks;
  if (!GetColdBlocks(cold_blocks)) {
    // Nothing to do.
    if (should_dump()) {
      LOG(INFO) << "UpdateBlockOrder: No cold blocks for " << GetMethodName(this);
    }
    return;
  }

  if (should_dump()) {
    LOG(INFO) << "UpdateBlockOrder: Start " << GetMethodName(this) << ", OSR: " << IsCompilingOsr();
    DumpBlocks("UpdateBlockOrder: Initial order", this, linear_order_);
  }

  // Remember the non-cold block for each block.  Use the loop information to
  // figure out which loop (if any) the block belongs to.
  std::map<HLoopInformation*, size_t> last_block;

  // We have a list of cold blocks.  Run through the order twice, and move the cold
  // ones to the back of the new list.  This preserves the relative order.
  size_t num_blocks = linear_order_.size();
  ArenaVector<HBasicBlock*> updated_block_order(GetArena()->Adapter(kArenaAllocBlockList));
  updated_block_order.reserve(num_blocks);

  // First, blocks that are not cold.
  for (size_t i = 0; i < num_blocks; i++) {
    HBasicBlock* block = linear_order_[i];
    if (block != nullptr && cold_blocks.find(block) == cold_blocks.end()) {
      // Remember the index of the last block in each loop for later.
      HLoopInformation* loop_info = block->GetLoopInformation();
      if (loop_info != nullptr) {
        last_block[loop_info] = updated_block_order.size();
      }
      updated_block_order.push_back(block);
    }
  }

  // Then insert the blocks that are cold into the right location.
  for (size_t i = 0; i < num_blocks; i++) {
    HBasicBlock* block = linear_order_[i];
    if (block != nullptr && cold_blocks.find(block) != cold_blocks.end()) {
      HLoopInformation* loop_info = block->GetLoopInformation();
      if (loop_info == nullptr) {
        // Just add to the end of the method.
        if (should_dump()) {
          LOG(INFO) << "UpdateBlockOrder: Move cold block "
                    << block->GetBlockId() << " to end of method";
        }
        updated_block_order.push_back(block);
        continue;
      }

      // Add the block to the correct location, and fix all the indices.
      auto it = last_block.find(loop_info);
      if (it == last_block.end()) {
        // The whole loop must have been cold.
        // Just add to the end of the method.
        if (should_dump()) {
          LOG(INFO) << "UpdateBlockOrder: Move cold loop block "
                    << block->GetBlockId() << " to end of method";
        }
        updated_block_order.push_back(block);
        continue;
      }

      // Add it after the matching loop.
      size_t last_existing_index = it->second;
      DCHECK_LT(last_existing_index, updated_block_order.size());
      if (should_dump()) {
        LOG(INFO) << "UpdateBlockOrder: Move cold loop block "
                  << block->GetBlockId() << " to end of loop with header "
                  << loop_info->GetHeader()->GetBlockId();
      }
      updated_block_order.insert(updated_block_order.begin() + last_existing_index + 1, block);

      // Bump all the indices at or after this index to account for the insertion.
      for (auto& it2 : last_block) {
        if (it2.second >= last_existing_index) {
          it2.second++;
        }
      }
    }
  }

  if (should_dump()) {
    DumpBlocks("UpdateBlockOrder: Final order", this, updated_block_order);
    LOG(INFO) << "UpdateBlockOrder: End " << GetMethodName(this);
  }

  // Now use the new order.
  linear_order_ = updated_block_order;
}

 }  // namespace art
