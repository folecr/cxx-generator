LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := cxxgenerator-spidermonkey

LOCAL_MODULE_FILENAME := libcxxgenerator-spidermonkey

LOCAL_SRC_FILES := jsbScriptingCore.cpp \
                   js_manual_conversions.cpp \

LOCAL_C_INCLUDES := $(LOCAL_PATH)

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)

LOCAL_WHOLE_STATIC_LIBRARIES := spidermonkey_static
LOCAL_WHOLE_STATIC_LIBRARIES += cocos2dx_static

LOCAL_LDLIBS := -landroid
LOCAL_LDLIBS += -llog

include $(BUILD_STATIC_LIBRARY)

$(call import-module,scripting/javascript/spidermonkey-android)
$(call import-module,cocos2dx)
