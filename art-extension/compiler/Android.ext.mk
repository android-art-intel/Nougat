#
# Copyright (C) 2014 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Modified by Intel Corporation
#

# Let us be careful and not re-add extension files multiple times.
# To do the check we pick the name of one of the tests below.
ifeq ($(findstring graph_x86,$(LIBART_COMPILER_SRC_FILES)),)

  VENDOR_EXTENSIONS_FOLDER := optimizing/extensions

  COMPILER_EXTENSION_SRC_FILES := \
    $(VENDOR_EXTENSIONS_FOLDER)/infrastructure/bound_information.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/infrastructure/cloning.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/infrastructure/ext_alias.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/infrastructure/ext_utility.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/infrastructure/graph_x86.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/infrastructure/induction_variable.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/infrastructure/loop_information.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/infrastructure/loop_unrolling.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/infrastructure/pass_framework.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/abi_transition_helper.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/aur.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/bb_simplifier.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/commutative_trees_flipper.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/constant_calculation_sinking.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/constant_folding_x86.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/devirtualization.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/devirtualization_helper.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/find_ivs.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/form_bottom_loops.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/gvn_after_fbl.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/insert_profiling.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/loadhoist_storesink.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/loop_formation.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/loop_full_unrolling.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/non_temporal_move.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/osr_graph_rebuilder.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/peeling.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/phi_cleanup.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/pure_invokes_analysis.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/remove_unused_loops.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/remove_suspend.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/speculation_pass.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/value_propagation_through_heap.cc \
    $(VENDOR_EXTENSIONS_FOLDER)/passes/trivial_loop_evaluator.cc

  COMPILER_EXTENSION_ENUM_OPERATOR_OUT_HEADER_FILES = \
    base/stringprintf.h \
    dex/pass_manager.h

  LIBART_COMPILER_SRC_FILES += \
    $(COMPILER_EXTENSION_SRC_FILES) \
    $(COMPILER_EXTENSION_ENUM_OPERATOR_OUT_HEADER_FILES)

endif
