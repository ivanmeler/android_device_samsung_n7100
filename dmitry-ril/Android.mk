LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= dmitry-ril.c
LOCAL_SHARED_LIBRARIES := liblog libbinder
LOCAL_MODULE:= dmitry-ril

include $(BUILD_SHARED_LIBRARY)
