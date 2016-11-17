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

#include "base/arena_allocator.h"
#include "builder.h"
#include "dex_file.h"
#include "dex_instruction.h"
#include "graph_x86.h"
#include "nodes.h"
#include "loop_information.h"
#include "loop_formation.h"
#include "optimizing_unit_test.h"
#include "ssa_liveness_analysis.h"
#include "pretty_printer.h"

#include "gtest/gtest.h"

namespace art {

class LoopHierarchyTest : public CommonCompilerTest {};

static HGraph* TestCode(const uint16_t* data, ArenaAllocator* allocator) {
  // Build the graph.
  HGraph_X86* graph = CreateX86CFG(allocator, data, Primitive::kPrimVoid);

  // Run our pass.
  HLoopFormation formation(graph);
  formation.Run();

  return graph;
}

static void TestLoop(HGraph* graph,
                      int block_id,
                      int depth,
                      bool is_inner,
                      bool has_siblings,
                      bool has_parent,
                      uint32_t sibling_prev = 0,
                      uint32_t sibling_next = 0) {
  HBasicBlock* block = graph->GetBlocks()[block_id];
  HLoopInformation_X86* info = LOOPINFO_TO_LOOPINFO_X86(block->GetLoopInformation());
  ASSERT_NE(info, nullptr);
  ASSERT_EQ(info->GetInner() == nullptr, is_inner);
  ASSERT_EQ(info->GetNextSibling() != nullptr || info->GetPrevSibling() != nullptr, has_siblings);

  if (has_siblings) {
    if (sibling_next != std::numeric_limits<uint32_t>::max()) {
      HLoopInformation_X86* sibling = info->GetNextSibling();
      ASSERT_NE(sibling, nullptr);
      HBasicBlock* sibling_header = sibling->GetHeader();
      ASSERT_EQ(sibling_next, sibling_header->GetBlockId());
    }

    if (sibling_prev != std::numeric_limits<uint32_t>::max()) {
      HLoopInformation_X86* sibling = info->GetPrevSibling();
      ASSERT_NE(sibling, nullptr);
      HBasicBlock* sibling_header = sibling->GetHeader();
      ASSERT_EQ(sibling_prev, sibling_header->GetBlockId());
    }
  }
  ASSERT_EQ(info->GetParent() != nullptr, has_parent);
  ASSERT_EQ(info->GetDepth(), depth);
}

TEST_F(LoopHierarchyTest, Loop1) {
  // Simple loop with one preheader and one back edge.
  // var a = 0;
  // while (a == a) {
  // }
  // return;
  // Below should create this.
  const uint16_t data[] = ONE_REGISTER_CODE_ITEM(
    Instruction::CONST_4 | 0 | 0,
    Instruction::IF_EQ, 3,
    Instruction::GOTO | 0xFE00,
    Instruction::RETURN_VOID);

  ArenaPool arena;
  ArenaAllocator allocator(&arena);
  HGraph* graph = TestCode(data, &allocator);

  // Test if the LoopInformation is of depth 1, inner, no siblings, no parent.
  TestLoop(graph, 2, 0, true, false, false);
}

TEST_F(LoopHierarchyTest, InnerLoop) {
  const uint16_t data[] = ONE_REGISTER_CODE_ITEM(
    Instruction::CONST_4 | 0 | 0,
    Instruction::IF_EQ, 6,
    Instruction::IF_EQ, 3,
    Instruction::GOTO | 0xFE00,  // inner loop.
    Instruction::GOTO | 0xFB00,
    Instruction::RETURN | 0 << 8);

  ArenaPool arena;
  ArenaAllocator allocator(&arena);
  HGraph* graph = TestCode(data, &allocator);

  TestLoop(graph, 2, 0, false, false, false);
  TestLoop(graph, 3, 1, true, false, true);
}

TEST_F(LoopHierarchyTest, TwoLoops) {
  const uint16_t data[] = ONE_REGISTER_CODE_ITEM(
    Instruction::CONST_4 | 0 | 0,
    Instruction::IF_EQ, 3,
    Instruction::GOTO | 0xFE00,  // first loop.
    Instruction::IF_EQ, 3,
    Instruction::GOTO | 0xFE00,  // second loop.
    Instruction::RETURN | 0 << 8);

  ArenaPool arena;
  ArenaAllocator allocator(&arena);
  HGraph* graph = TestCode(data, &allocator);

  TestLoop(graph, 2, 0, true, true, false, 4, std::numeric_limits<uint32_t>::max());
  TestLoop(graph, 4, 0, true, true, false, std::numeric_limits<uint32_t>::max(), 2);
}

TEST_F(LoopHierarchyTest, NestedSibling) {
  /**
   * Complicated test with a bit of everything here:
   * for () {
   *  for () {
   *    for () {
   *    }
   *    for () {
   *    }
   *  }
   *  for () {
   *    for () {
   *    }
   *    for () {
   *    }
   *  }
   * }
   * Below should generate this.
   */
  const uint16_t data[] = ONE_REGISTER_CODE_ITEM(
    Instruction::CONST_4 | 0 | 0,
    Instruction::CONST_4 | 0 | 0,
    Instruction::IF_EQ, 3,
    Instruction::GOTO | 0xFE00,  // first inner loop.
    Instruction::IF_EQ, 3,
    Instruction::GOTO | 0xFE00,  // second inner loop.
    Instruction::IF_EQ, 3,
    Instruction::GOTO | 0xF700,  // second level loop.
    Instruction::CONST_4 | 0 | 0,
    Instruction::IF_EQ, 3,
    Instruction::GOTO | 0xFE00,  // third inner loop.
    Instruction::IF_EQ, 3,
    Instruction::GOTO | 0xFE00,  // fourth inner loop.
    Instruction::IF_EQ, 3,
    Instruction::GOTO | 0xF700,  // second level loop.
    Instruction::IF_EQ, 3,
    Instruction::GOTO | 0xE900,  // first level loop.
    Instruction::RETURN | 0 << 8);

  ArenaPool arena;
  ArenaAllocator allocator(&arena);
  HGraph* graph = TestCode(data, &allocator);

  TestLoop(graph, 2, 1, false, true, true, 9, std::numeric_limits<uint32_t>::max());
  TestLoop(graph, 3, 2, true, true, true, 5, std::numeric_limits<uint32_t>::max());
  TestLoop(graph, 5, 2, true, true, true, std::numeric_limits<uint32_t>::max(), 3);

  TestLoop(graph, 9, 1, false, true, true, std::numeric_limits<uint32_t>::max(), 2);
  TestLoop(graph, 10, 2, true, true, true, std::numeric_limits<uint32_t>::max(), 12);
  TestLoop(graph, 12, 2, true, true, true, 10, std::numeric_limits<uint32_t>::max());

  TestLoop(graph, 1, 0, false, false, false);
}

TEST_F(LoopHierarchyTest, DoWhileLoop) {
  const uint16_t data[] = ONE_REGISTER_CODE_ITEM(
    Instruction::CONST_4 | 0 | 0,
    Instruction::GOTO | 0x0100,
    Instruction::IF_EQ, 0xFFFF,
    Instruction::RETURN | 0 << 8);

  ArenaPool arena;
  ArenaAllocator allocator(&arena);
  HGraph* graph = TestCode(data, &allocator);

  TestLoop(graph, 2, 0, true, false, false);
}

}  // namespace art
