#
# Copyright (C) 2015 Intel Corporation
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

LOCAL_PATH := $(call my-dir)

include $(VENDOR_ART_PATH)/build/Android.executable.mk

PROFDUMP_SRC_FILES := \
	profdump.cc

# TODO: Remove this when the framework (installd) supports pushing the
# right instruction-set parameter for the primary architecture.
ifneq ($(filter ro.zygote=zygote64,$(PRODUCT_DEFAULT_PROPERTY_OVERRIDES)),)
  profdump_arch := 64
else
  profdump_arch := 32
endif

ifeq ($(ART_BUILD_TARGET_NDEBUG),true)
  $(eval $(call build-art-executable,profdump,$(PROFDUMP_SRC_FILES),libcutils,$(VENDOR_ART_PATH)/compiler,target,ndebug,$(profdump_arch),))
endif
ifeq ($(ART_BUILD_TARGET_DEBUG),true)
  $(eval $(call build-art-executable,profdump,$(PROFDUMP_SRC_FILES),libcutils,$(VENDOR_ART_PATH)/compiler,target,debug,$(profdump_arch),))
endif

# We always build profdump and dependencies, even if the host build is otherwise disabled, since they are used to cross compile for the target.
ifeq ($(ART_BUILD_HOST_NDEBUG),true)
  $(eval $(call build-art-executable,profdump,$(PROFDUMP_SRC_FILES),libcutils,$(VENDOR_ART_PATH)/compiler,host,ndebug,,))
endif
ifeq ($(ART_BUILD_HOST_DEBUG),true)
  $(eval $(call build-art-executable,profdump,$(PROFDUMP_SRC_FILES),libcutils,$(VENDOR_ART_PATH)/compiler,host,debug,,))
endif
