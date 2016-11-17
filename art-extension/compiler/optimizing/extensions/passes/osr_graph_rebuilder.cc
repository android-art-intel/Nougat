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
 *
 */

#include "ext_utility.h"
#include "loop_iterators.h"
#include "osr_graph_rebuilder.h"

namespace art {

void HOsrGraphRebuilder::InjectionPhiFixup(HLoopInformation_X86* loop,
                                           HBasicBlock* osr_path,
                                           HBasicBlock* new_pre_header) {
  DCHECK(loop != nullptr);
  HBasicBlock* header = loop->GetHeader();
  auto arena = graph_->GetArena();
  // Traverse over loop phis and substitute them with
  // new phis in preheader.
  for (HInstructionIterator phi_it(header->GetPhis());
       !phi_it.Done();
       phi_it.Advance()) {
    // Create a new phi merging OSR and non-osr inputs.
    HPhi* phi = phi_it.Current()->AsPhi();

    // Paranoid: ensure that we have 2 inputs.
    DCHECK_EQ(phi->InputCount(), 2u);

    bool degenerate = (phi->InputAt(1) == phi);

    PRINT_PASS_OSTREAM_MESSAGE(this, "Making OSR entry for"
                                      << (degenerate? " degenerate" : "")
                                      << " " << phi);
    HInstruction* input = phi->InputAt(0);

    HPhi* new_phi = new (arena) HPhi(arena,
                                     phi->GetRegNumber(),
                                     2u,
                                     HPhi::ToPhiType(phi->GetType()));

    // Create a fictive input in OSR path and set phi inputs.
    HOsrFictiveValue* osr_value = new (arena) HOsrFictiveValue(input->GetType());
    if (input->GetType() == Primitive::Type::kPrimNot) {
      osr_value->SetReferenceTypeInfo(input->GetReferenceTypeInfo());
      new_phi->SetReferenceTypeInfo(input->GetReferenceTypeInfo());
    }
    osr_path->InsertInstructionBefore(osr_value, osr_path->GetLastInstruction());
    new_phi->SetRawInputAt(0, input);
    new_phi->SetRawInputAt(1, osr_value);
    new_pre_header->AddPhi(new_phi);

    phi->ReplaceInput(new_phi, 0);

    if (degenerate) {
      PRINT_PASS_OSTREAM_MESSAGE(this, "Removing degenerate " << phi);
      phi->ReplaceWith(phi->InputAt(0));
      header->RemovePhi(phi);
    }
  }
}

void HOsrGraphRebuilder::InjectOsrIntoLoops(HBasicBlock* osr_path) {
  DCHECK(osr_path != nullptr);
  DCHECK(osr_path->GetLoopInformation() == nullptr);
  HGraph_X86* graph = GetGraphX86();

  for (auto loop : fixable_loops_) {
    HBasicBlock* header = loop->GetHeader();
    HBasicBlock* old_pre_header = loop->GetPreHeader();
    PRINT_PASS_OSTREAM_MESSAGE(this, "Inserting OSR for loop #"
                               << header->GetBlockId()
                               << " (pre-header #" << old_pre_header->GetBlockId()
                               << ")");

    HBasicBlock* new_pre_header = graph->CreateNewBasicBlock();
    PRINT_PASS_OSTREAM_MESSAGE(this, "Inserting new pre-header #"
                                     << new_pre_header->GetBlockId());
    new_pre_header->InsertBetween(old_pre_header, header);
    // Insert this to avoid appearance of critical edge.
    HBasicBlock* split = graph->CreateNewBasicBlock();
    split->AddInstruction(new (graph->GetArena()) HGoto());
    osr_path->AddSuccessor(split);
    split->AddSuccessor(new_pre_header);

    // Create OSR fixup Phis.
    InjectionPhiFixup(loop, osr_path, new_pre_header);

    // Now replace suspend check entry point with pre-header entry point.
    auto suspend_check = loop->GetSuspendCheck();
    suspend_check->RemoveOsrEntryPoint();

    HOsrEntryPoint* osr_entry = new (graph->GetArena())
                                HOsrEntryPoint(suspend_check->GetDexPc());
    new_pre_header->AddInstruction(osr_entry);
    osr_entry->CopyEnvironmentFromWithLoopPhiAdjustment(
               suspend_check->GetEnvironment(), header);
    new_pre_header->AddInstruction(new (graph->GetArena()) HGoto());
    new_pre_header->SetLoopInformation(old_pre_header->GetLoopInformation());

    // Finally, make the loop reducible.
    loop->MakeReducible();
  }
}

void HOsrGraphRebuilder::TransformCfg(HBasicBlock*& osr_fork,
                                      HBasicBlock*& normal_path,
                                      HBasicBlock*& osr_path) {
  HGraph_X86* graph = GetGraphX86();

  HBasicBlock* method_entry = graph->GetEntryBlock();
  osr_fork = graph->CreateNewBasicBlock();
  normal_path = graph->CreateNewBasicBlock();
  osr_path = graph->CreateNewBasicBlock();

  // Create OSR fork block.
  PRINT_PASS_OSTREAM_MESSAGE(this, "Inserting OSR fork block #" << osr_fork->GetBlockId());
  osr_fork->InsertBetween(method_entry, method_entry->GetSingleSuccessor());
  osr_fork->AddInstruction(new (graph->GetArena()) HOsrFork());

  PRINT_PASS_OSTREAM_MESSAGE(this, "Branching: normal path #" << normal_path->GetBlockId()
                                   << ", osr path #" << osr_path->GetBlockId());
  normal_path->AddInstruction(new (graph->GetArena()) HGoto());
  normal_path->InsertBetween(osr_fork, osr_fork->GetSingleSuccessor());
  osr_fork->AddSuccessor(osr_path);
  osr_path->AddInstruction(new (graph->GetArena()) HOsrJump());
}

void HOsrGraphRebuilder::MoveParams(HBasicBlock* entry_block,
                                    HBasicBlock* normal_path) {
  for (HBackwardInstructionIterator inst_it(entry_block->GetInstructions());
     !inst_it.Done();
     inst_it.Advance()) {
    HInstruction* insn = inst_it.Current();
    if (insn->IsParameterValue() || insn->IsSuspendCheck()) {
      PRINT_PASS_OSTREAM_MESSAGE(this, "Moving " << insn
                                        << " to block #"
                                        << normal_path->GetBlockId());
      insn->MoveBefore(normal_path->GetFirstInstruction());
    }
  }
}

void HOsrGraphRebuilder::DoOsrPreparation() {
  HGraph_X86* graph = GetGraphX86();

  HBasicBlock* osr_fork = nullptr;
  HBasicBlock* normal_path = nullptr;
  HBasicBlock* osr_path = nullptr;

  // First, transform CFG, creating auxiliary blocks.
  TransformCfg(osr_fork, normal_path, osr_path);

  // Total paranoia.
  HBasicBlock* method_entry = graph->GetEntryBlock();
  DCHECK(method_entry != nullptr);
  DCHECK(osr_fork != nullptr);
  DCHECK(normal_path != nullptr);
  DCHECK(osr_path != nullptr);

  // Move params and suspend check to normal path.
  MoveParams(method_entry, normal_path);

  // Inject jumps from OSR path to loops headers.
  InjectOsrIntoLoops(osr_path);

  // Since we changed CFG, we need to rebuild domination.
  graph->RebuildDomination();
}

bool HOsrGraphRebuilder::Gate() {
  HGraph_X86* graph = GetGraphX86();
  HLoopInformation_X86* loop_info = graph->GetLoopInformation();

  // Collect all fixable loops.
  for (HOnlyInnerLoopIterator loop_iter(loop_info);
       !loop_iter.Done(); loop_iter.Advance()) {
    HLoopInformation_X86* loop = loop_iter.Current();
    if (loop->IsIrreducible() &&
        loop->HasSuspendCheck() &&
        loop->NumberOfBackEdges() == 1 &&
        !loop->GetHeader()->IsTryBlock() &&
        !loop->IsIrreducibleSlowCheck()) {
      DCHECK(loop->IsInner());
      fixable_loops_.push_back(loop);
    }
  }

  // Have we found at least one?
  return !fixable_loops_.empty();
}

void HOsrGraphRebuilder::Run() {
  if (!graph_->IsCompilingOsr()) {
    return;
  }

  PRINT_PASS_OSTREAM_MESSAGE(this, "Start " << GetMethodName(graph_));

  if (Gate()) {
    PRINT_PASS_OSTREAM_MESSAGE(this, "Preparing " << fixable_loops_.size() << " loops");
    DoOsrPreparation();
  } else {
    PRINT_PASS_OSTREAM_MESSAGE(this, "Nothing to prepare");
  }

  PRINT_PASS_OSTREAM_MESSAGE(this, "End " << GetMethodName(graph_));
}

}  // namespace art
