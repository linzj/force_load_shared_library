LOCAL_PATH := $(call my-dir)/../

include $(CLEAR_VARS)

LOCAL_MODULE    := plt_caller_test
LOCAL_SRC_FILES := got_finder.cpp log.cpp ptracer.cpp arm/plt_caller_test.cpp arm/plt_caller.cpp
LOCAL_LDFLAGS = -pie
LOCAL_LDLIBS += -llog

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE    := test
LOCAL_SRC_FILES := test.cpp

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    := ldso
LOCAL_SRC_FILES := got_finder.cpp log.cpp ptracer.cpp arm/plt_caller.cpp main.cpp
LOCAL_LDFLAGS = -pie
LOCAL_LDLIBS += -llog

include $(BUILD_EXECUTABLE)
