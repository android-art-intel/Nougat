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

#ifndef ART_OPT_INFRASTRUCTURE_GRAPH_X86_H_
#define ART_OPT_INFRASTRUCTURE_GRAPH_X86_H_

#include "nodes.h"
#include "loop_information.h"
#include "optimizing_compiler_stats.h"

namespace art {

#ifndef NDEBUG
#define GRAPH_MAGIC 0xdeadcafe
#define GRAPH_TO_GRAPH_X86(X) HGraph_X86::DownCast(X)
#else
#define GRAPH_TO_GRAPH_X86(X) static_cast<HGraph_X86*>(X)
#endif

class HGraph_X86 : public HGraph {
 public:
  HGraph_X86(ArenaAllocator* arena, const DexFile& dex_file, uint32_t method_idx,
      bool should_generate_constructor_barrier, InstructionSet instruction_set,
      InvokeType invoke_type = kInvalidInvokeType, bool debuggable = false,
      bool osr = false, int start_instruction_id = 0) :
          HGraph(arena, dex_file, method_idx, should_generate_constructor_barrier,
          instruction_set, invoke_type, debuggable, osr, start_instruction_id),
          loop_information_(nullptr),
          profiled_invokes_(arena->Adapter()) {
#ifndef NDEBUG
        down_cast_checker_ = GRAPH_MAGIC;
#endif
  }

  /**
   * @brief Return the graph's loop information.
   * @return the outermost loop in the graph.
   */
  HLoopInformation_X86* GetLoopInformation() const {
    return loop_information_;
  }

  void SetLoopInformation(HLoopInformation_X86* loop) {
    loop_information_ = loop;
  }

  /**
   * @brief Clear the loop information.
   */
  void ClearLoopInformation() {
    loop_information_ = nullptr;
  }

  /**
   * @brief Add a loop to the graph.
   * @param information The loop information to be added.
   */
  void AddLoopInformation(HLoopInformation_X86* information) {
    // Due to the way this method is being called,
    //   the first call is one of the outer loops.

    // Ensure we are only adding a single loop in.
    information->ResetLinks();
    if (loop_information_ == nullptr) {
      loop_information_ = information;
    } else {
      loop_information_->Add(information);
    }
  }

  /**
   * @brief Delete a block, cleaning up all the loose ends such as
   * successors, predecessors, etc.
   * @param block The HBasicBlock to delete from the graph.
   * @param remove_from_loops Whether we should also delete the block
   *                          from its loop infos correctly.
   */
  void DeleteBlock(HBasicBlock* block, bool remove_from_loops = false);

  /**
   * @brief Used to update the links between two blocks when one is added
   * that follows an existing one unconditionally.
   * @details Updates successor/predecessor info, updates domination information,
   * and maintains the ordering information.
   * @param existing_block The block that is already in the graph.
   * @param block_being_added The new block being added to follow existing_block.
   * @param add_as_dominator Whether the block being added is dominated by the exist block.
   * @param add_after Whether the block being added is before or after. When false, it means it
   * is being added as a predecessor.
   */
  void CreateLinkBetweenBlocks(HBasicBlock* existing_block,
                               HBasicBlock* block_being_added,
                               bool add_as_dominator,
                               bool add_after);

  void Dump();

#ifndef NDEBUG
  static HGraph_X86* DownCast(HGraph* graph) {
    HGraph_X86* res = static_cast<HGraph_X86*>(graph);

    if (res != nullptr) {
      DCHECK_EQ(res->down_cast_checker_, GRAPH_MAGIC);
    }
    return res;
  }
#endif

  /**
   * @brief Used to create a new basic that is added to graph.
   * @param dex_pc The dex pc of this block (optional).
   * @return Returns the newly created block.
   */
  HBasicBlock* CreateNewBasicBlock(uint32_t dex_pc = kNoDexPc) {
    HBasicBlock* new_block = new (arena_) HBasicBlock(this, dex_pc);
    AddBlock(new_block);
    return new_block;
  }

  /**
   * @brief Split critial edge and set loop information splitter.
   * @param from input of new critical edge.
   * @param to output of new critical edge.
   */
  void SplitCriticalEdgeAndUpdateLoopInformation(HBasicBlock* from, HBasicBlock* to);

  /**
   * @brief Called after an optimization pass in order to rebuild domination
   * information and ordering.
   * @details This also has effect of cleaning up graph and normalizing loops
   * (depending on what Google overloaded it to do).
   */
  void RebuildDomination();

  /*
   * @brief Move a Phi from one block to another block.
   * @param phi Phi to move.
   * @param to_block Block to which to move the Phi.
   */
  void MovePhi(HPhi* phi, HBasicBlock* to_block);

  /*
   * @brief Move an from one block to another block.
   * @param instr Instruction to move.
   * @param cursor The target instruction.
   * @note The instruction to move must be in a different block from the cursor.
   */
  void MoveInstructionBefore(HInstruction* instr, HInstruction* cursor);

  /**
   * @brief How many blocks have exact profiling block increments?
   * @return Returns The number of blocks with exact profile bumps.
   */
  int GetNumProfiledBlocks() const {
    return num_profiled_blocks_;
  }

  /**
   * @brief Set the number of blocks with profiling increments.
   * @param val Number of blocks.
   */
  void SetNumProfiledBlocks(int val) {
    num_profiled_blocks_ = val;
  }

  /**
   * @brief Are there any saved invoke dex pcs?
   * @ereturns 'true' if there are virtual/interface invokes in the method.
   */
  bool HasProfiledInvokesDexPcs() const {
    return !profiled_invokes_.empty();
  }

  /**
   * @brief Return the saved invoke dex pcs list.
   * @returns the vector of saved virtual/interface invokes.  Will create the
   * list if not already created.
   */
  ArenaVector<uint16_t>& GetProfiledInvokesDexPcs() {
    return profiled_invokes_;
  }

  enum HasExactProfileInformation {
    kNoCounts,          // No information at all.
    kMethodCount,       // Just a count of the method.
    kBasicBlockCounts,  // Counts for all (initial) basic blocks.
  };

  /*
   * @brief What kind of profiling is available for use?
   * @returns the available profiling kind.
   */
  HasExactProfileInformation GetProfileCountKind() const {
    return exact_profile_kind_;
  }

  /*
   * @brief Set the kind of profiling available for use.
   * @param kind the available profiling kind.
   */
  void SetProfileCountKind(HasExactProfileInformation kind) {
    exact_profile_kind_ = kind;
  }

  /*
   * @brief Is this method hot?
   * @returns 'true' if the method was executed frequently.
   */
  bool IsHot() const {
    return hot_;
  }

  /*
   * @brief Set the hotness of this method.
   * @param hot Whether or not the method was frequently executed.
   */
  void SetHot(bool hot) {
    hot_ = hot;
  }

  /*
   * @brief Return the 'cold' blocks in a method.
   * @param blocks Set to the set of all cold blocks in the method.
   * @returns 'true' if any cold blocks are returned.
   */
  bool GetColdBlocks(std::set<HBasicBlock*>& blocks) const;

  /*
   * @brief Update the block order to move cold blocks out of line.
   */
  void UpdateBlockOrder();

 protected:
#ifndef NDEBUG
  uint32_t down_cast_checker_;
#endif

  HLoopInformation_X86* loop_information_;

  /**
   * @brief Used to record the maximum block number that contains exact profiling.
   */
  int num_profiled_blocks_ = 0;

  /**
   * @brief Used to record the dex_pcs for virtual/interface invoke calls.
   */
  ArenaVector<uint16_t> profiled_invokes_;

  HasExactProfileInformation exact_profile_kind_ = kNoCounts;

  bool hot_ = false;
};

/**
 * @brief Helper function to create a control flow graph using HGraph_X86 for
 * use in standalone tests.
 * @param allocator ArenaAllocator to use to allocate the HGraph_X86.
 * @returns the newly created HGraph_X86.
 */
HGraph_X86* CreateX86CFG(ArenaAllocator* allocator,
                         const uint16_t* data = nullptr,
                         Primitive::Type return_type = Primitive::kPrimInt);
}  // namespace art

#endif  // ART_OPT_INFRASTRUCTURE_GRAPH_X86_H_
