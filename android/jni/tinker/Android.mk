LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := tinker

SDL_PATH := ../SDL

TINKER_PATH := ../../../

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include $(LOCAL_PATH)/$(TINKER_PATH)/ $(LOCAL_PATH)/$(TINKER_PATH)/common $(LOCAL_PATH)/$(TINKER_PATH)/common/math $(LOCAL_PATH)/$(TINKER_PATH)/tinker
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
LOCAL_SRC_FILES := $(TINKER_PATH)/common/mempool.cpp \
	$(TINKER_PATH)/common/mtrand.cpp \
	$(TINKER_PATH)/common/stb_image.cpp \
	$(TINKER_PATH)/common/stb_image_write.cpp \
	$(TINKER_PATH)/common/parallelize.cpp \
	$(TINKER_PATH)/common/platform_android.cpp \
	$(TINKER_PATH)/common/sockets/sockets.cpp \
	$(TINKER_PATH)/common/math/color.cpp \
	$(TINKER_PATH)/common/math/matrix.cpp \
	$(TINKER_PATH)/common/math/quaternion.cpp \
	$(TINKER_PATH)/common/math/frustum.cpp \
	$(TINKER_PATH)/common/math/vector.cpp \
	$(TINKER_PATH)/common/math/mesh.cpp \
	$(TINKER_PATH)/datamanager/data.cpp \
	$(TINKER_PATH)/datamanager/dataserializer.cpp \
	$(TINKER_PATH)/common/protobuf/math.pb.cc \
	$(TINKER_PATH)/tinker/glgui/glgui.cpp \
	$(TINKER_PATH)/tinker/glgui/basecontrol.cpp \
	$(TINKER_PATH)/tinker/glgui/button.cpp \
	$(TINKER_PATH)/tinker/glgui/checkbox.cpp \
	$(TINKER_PATH)/tinker/glgui/colorpicker.cpp \
	$(TINKER_PATH)/tinker/glgui/droppablepanel.cpp \
	$(TINKER_PATH)/tinker/glgui/filedialog.cpp \
	$(TINKER_PATH)/tinker/glgui/label.cpp \
	$(TINKER_PATH)/tinker/glgui/menu.cpp \
	$(TINKER_PATH)/tinker/glgui/movablepanel.cpp \
	$(TINKER_PATH)/tinker/glgui/panel.cpp \
	$(TINKER_PATH)/tinker/glgui/picturebutton.cpp \
	$(TINKER_PATH)/tinker/glgui/rootpanel.cpp \
	$(TINKER_PATH)/tinker/glgui/scrollbar.cpp \
	$(TINKER_PATH)/tinker/glgui/slidingpanel.cpp \
	$(TINKER_PATH)/tinker/glgui/tree.cpp \
	$(TINKER_PATH)/tinker/glgui/textfield.cpp \
	$(TINKER_PATH)/tinker/renderer/renderer.cpp \
	$(TINKER_PATH)/tinker/renderer/renderingcontext.cpp \
	$(TINKER_PATH)/tinker/renderer/shaders.cpp \
	$(TINKER_PATH)/tinker/textures/materiallibrary.cpp \
	$(TINKER_PATH)/tinker/textures/texturelibrary.cpp \
	$(TINKER_PATH)/tinker/textures/texturesheet.cpp \
	$(TINKER_PATH)/tinker/shell.cpp \
	$(TINKER_PATH)/tinker/application.cpp \
	$(TINKER_PATH)/tinker/console.cpp \
	$(TINKER_PATH)/tinker/cvar.cpp \
	$(TINKER_PATH)/tinker/profiler.cpp \


LOCAL_SHARED_LIBRARIES := SDL2 SDL2_image protobuf250 ftgl3

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -lGLESv3 -llog -landroid

include $(BUILD_SHARED_LIBRARY)
