LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL

TINKER_PATH := ../../../

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include $(LOCAL_PATH)/$(TINKER_PATH)/ $(LOCAL_PATH)/$(TINKER_PATH)/common $(LOCAL_PATH)/$(TINKER_PATH)/common/math $(LOCAL_PATH)/$(TINKER_PATH)/tinker
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(TINKER_PATH)/../viewback
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(TINKER_PATH)/../ext-deps/protobuf-2.5.0/src $(LOCAL_PATH)/$(TINKER_PATH)/../ext-deps/ftgl-gl3/src $(LOCAL_PATH)/$(TINKER_PATH)/../ext-deps/freetype-2.3.11/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../SDL2_image

LOCAL_CFLAGS += -DTINKER_OPENGLES_3 -DTINKER_NO_TOOLS

ifeq ($(NDK_DEBUG),1)
    $(warning Building DEBUG MODE)
    LOCAL_CFLAGS += -g -D_DEBUG
else
    $(warning Building RELEASE MODE)
endif

# Add your application source files here...
LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.c \
	$(TINKER_PATH)/monitor/main.cpp \
	$(TINKER_PATH)/monitor/monitor_window.cpp \
	$(TINKER_PATH)/monitor/monitor_menu.cpp \
	$(TINKER_PATH)/monitor/panel_container.cpp \
	$(TINKER_PATH)/monitor/panel_base.cpp \
	$(TINKER_PATH)/monitor/panel_2d.cpp \
	$(TINKER_PATH)/monitor/panel_console.cpp \
	$(TINKER_PATH)/monitor/panel_time.cpp \
	$(TINKER_PATH)/../viewback/client/viewback_client.cpp \
	$(TINKER_PATH)/../viewback/client/viewback_data.cpp \
	$(TINKER_PATH)/../viewback/client/viewback_servers.cpp \
	$(TINKER_PATH)/../viewback/protobuf/data.pb.cc \

LOCAL_SHARED_LIBRARIES := tinker SDL2 SDL2_image protobuf250 ftgl3

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -lGLESv3 -llog -landroid

include $(BUILD_SHARED_LIBRARY)
