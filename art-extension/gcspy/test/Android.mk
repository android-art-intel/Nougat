#TODO Copyrights

LOCAL_PATH:= $(call my-dir)

include art/build/Android.common_build.mk

gcspy_test_files := 

GCSPY_CFLAGS := -D_LINUX_ -std=gnu11 -ggdb3

include $(CLEAR_VARS)

LOCAL_MODULE := gcspy_dummy_server
LOCAL_SRC_FILES := dummy_server.c dummy_driver.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include
LOCAL_CFLAGS += $(GCSPY_CFLAGS)
LOCAL_LDLIBS := -llog -lgcspy
LOCAL_CLANG := false

include $(BUILD_HOST_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE := gcspy_test_server
LOCAL_SRC_FILES := gcspy_test_server.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_CFLAGS += $(GCSPY_CFLAGS)
LOCAL_LDLIBS := -llog -lgcspy
LOCAL_CLANG := false

#include $(BUILD_HOST_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE := gcspy_test_shape_comm
LOCAL_SRC_FILES := gcspy_test_shape_comm.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_CFLAGS += $(GCSPY_CFLAGS)
LOCAL_LDLIBS := -llog -lgcspy
LOCAL_CLANG := false

#include $(BUILD_HOST_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE := gcspy_test_simple_com
LOCAL_SRC_FILES := gcspy_test_simple_comm.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_CFLAGS += $(GCSPY_CFLAGS)
LOCAL_LDLIBS := -llog -lgcspy
LOCAL_CLANG := false

#include $(BUILD_HOST_EXECUTABLE)
