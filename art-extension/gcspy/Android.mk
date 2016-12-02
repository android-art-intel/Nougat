/**
 ** See the file "license.terms" for information on usage and redistribution
 ** of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 **/


LOCAL_PATH:= $(call my-dir)

include $(VENDOR_ART_PATH)/build/Android.common_build.mk

gcspy_src_files := \
	src/gcspy_array_input.c \
	src/gcspy_array_output.c \
	src/gcspy_buffered_input.c \
	src/gcspy_buffered_output.c \
	src/gcspy_color_db.c \
	src/gcspy_command_stream.c \
	src/gcspy_comm.c \
	src/gcspy_d_utils.c \
	src/gcspy_gc_driver.c \
	src/gcspy_gc_stream.c \
	src/gcspy_interpreter.c \
	src/gcspy_main_server.c \
	src/gcspy_shape_stream.c \
	src/gcspy_timer.c \
	src/gcspy_utils.c

GCSPY_CFLAGS := -D_LINUX_ -std=gnu11 -ggdb3


include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_CFLAGS += $(ART_TARGET_CFLAGS)
LOCAL_CFLAGS += $(GCSPY_CFLAGS)
LOCAL_ASFLAGS += $(ART_TARGET_ASFLAGS)
LOCAL_SRC_FILES := $(gcspy_src_files)
LOCAL_CLANG = $(ART_TARGET_CLANG)
LOCAL_MODULE:= libgcspy
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk
LOCAL_ADDITIONAL_DEPENDENCIES += art/build/Android.common_build.mk
LOCAL_NATIVE_COVERAGE := $(ART_COVERAGE)
$(eval $(call set-target-local-clang-vars))
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_CFLAGS += $(ART_TARGET_CFLAGS)
LOCAL_CFLAGS += $(GCSPY_CFLAGS)
LOCAL_ASFLAGS += $(ART_TARGET_ASFLAGS)
LOCAL_SRC_FILES := $(gcspy_src_files)
LOCAL_CLANG = $(ART_TARGET_CLANG)
LOCAL_MODULE:= libgcspy
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk
LOCAL_ADDITIONAL_DEPENDENCIES += art/build/Android.common_build.mk
$(eval $(call set-target-local-clang-vars))
include $(BUILD_STATIC_LIBRARY)

# Build host library.
include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_IS_HOST_MODULE := true
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
#LOCAL_CFLAGS += $(ART_HOST_CFLAGS)
LOCAL_CFLAGS := $(GCSPY_CFLAGS)
#LOCAL_ASFLAGS += $(ART_HOST_ASFLAGS)
LOCAL_CLANG = false #$(ART_HOST_CLANG)
LOCAL_SRC_FILES := $(gcspy_src_files)
LOCAL_MODULE:= libgcspy
#LOCAL_ADDITIONAL_DEPENDENCIES += $(LOCAL_PATH)/Android.mk
LOCAL_LDLIBS = -ldl
LOCAL_MULTILIB := both
LOCAL_NATIVE_COVERAGE := $(ART_COVERAGE)
include $(BUILD_HOST_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_IS_HOST_MODULE := true
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
#LOCAL_CFLAGS += $(ART_HOST_CFLAGS)
LOCAL_CFLAGS := $(GCSPY_CFLAGS)
#LOCAL_ASFLAGS += $(ART_HOST_ASFLAGS)
LOCAL_CLANG = false #$(ART_HOST_CLANG)
LOCAL_SRC_FILES := $(gcspy_src_files)
LOCAL_MODULE:= libgcspy
#LOCAL_ADDITIONAL_DEPENDENCIES += $(LOCAL_PATH)/Android.mk
LOCAL_LDLIBS = -ldl
LOCAL_MULTILIB := both
LOCAL_NATIVE_COVERAGE := $(ART_COVERAGE)
include $(BUILD_HOST_STATIC_LIBRARY)
