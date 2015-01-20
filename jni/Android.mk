LOCAL_PATH := $(call my-dir)/../

include $(CLEAR_VARS)

LOCAL_MODULE    := plt_caller_test
ARM_SRC := arm/plt_caller_test.cpp arm/plt_caller.cpp
I686_SRC := i686/plt_caller_test.cpp i686/plt_caller.cpp
ifeq ($(TARGET_ARCH), arm)
TARGET_SRC := $(ARM_SRC)
else
ifeq ($(TARGET_ARCH), x86)
TARGET_SRC := $(I686_SRC)
endif # x86
endif # arm
LOCAL_SRC_FILES := got_finder.cpp log.cpp ptracer.cpp $(TARGET_SRC)
LOCAL_LDFLAGS = -pie
LOCAL_LDLIBS += -llog

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE    := test
LOCAL_SRC_FILES := test.cpp
LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

ARM_SRC := arm/plt_caller.cpp
I686_SRC := i686/plt_caller.cpp
ifeq ($(TARGET_ARCH), arm)
TARGET_SRC := $(ARM_SRC)
else
ifeq ($(TARGET_ARCH), x86)
TARGET_SRC := $(I686_SRC)
endif # x86
endif # arm

LOCAL_MODULE    := ldso
LOCAL_SRC_FILES := got_finder.cpp log.cpp ptracer.cpp main.cpp $(TARGET_SRC)
LOCAL_LDFLAGS = -pie
LOCAL_LDLIBS += -llog

include $(BUILD_EXECUTABLE)
