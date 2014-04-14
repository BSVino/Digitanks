LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := tengine

TINKER_PATH := ../../../
TENGINE_PATH := ../../../tengine/
TOYS_PATH := ../../../toys/

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include $(LOCAL_PATH)/../SDL2_mixer $(LOCAL_PATH)/$(TINKER_PATH)/../ext-deps/protobuf-2.5.0/src
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(TINKER_PATH)/ $(LOCAL_PATH)/$(TINKER_PATH)/common $(LOCAL_PATH)/$(TINKER_PATH)/common/math $(LOCAL_PATH)/$(TINKER_PATH)/tinker $(LOCAL_PATH)/$(TINKER_PATH)/tengine

LOCAL_CFLAGS += -DTINKER_OPENGLES_3 -DTINKER_NO_TOOLS

ifeq ($(NDK_DEBUG),1)
    $(warning Building DEBUG MODE)
    LOCAL_CFLAGS += -g -D_DEBUG
else
    $(warning Building RELEASE MODE)
endif

# Add your application source files here...
LOCAL_SRC_FILES := $(TENGINE_PATH)/game/cameramanager.cpp \
	$(TENGINE_PATH)/game/gameserver.cpp \
	$(TENGINE_PATH)/game/level.cpp \
	$(TENGINE_PATH)/game/networkedeffect.cpp \
	$(TENGINE_PATH)/game/entities/baseentity.cpp \
	$(TENGINE_PATH)/game/entities/baseentitydata.cpp \
	$(TENGINE_PATH)/game/entities/beam.cpp \
	$(TENGINE_PATH)/game/entities/camera.cpp \
	$(TENGINE_PATH)/game/entities/counter.cpp \
	$(TENGINE_PATH)/game/entities/character.cpp \
	$(TENGINE_PATH)/game/entities/charactercamera.cpp \
	$(TENGINE_PATH)/game/entities/game.cpp \
	$(TENGINE_PATH)/game/entities/kinematic.cpp \
	$(TENGINE_PATH)/game/entities/logicgate.cpp \
	$(TENGINE_PATH)/game/entities/mathgate.cpp \
	$(TENGINE_PATH)/game/entities/player.cpp \
	$(TENGINE_PATH)/game/entities/playerstart.cpp \
	$(TENGINE_PATH)/game/entities/prop.cpp \
	$(TENGINE_PATH)/game/entities/static.cpp \
	$(TENGINE_PATH)/game/entities/target.cpp \
	$(TENGINE_PATH)/game/entities/team.cpp \
	$(TENGINE_PATH)/game/entities/trigger.cpp \
	$(TENGINE_PATH)/game/entities/weapon.cpp \
	$(TENGINE_PATH)/models/models.cpp \
	$(TENGINE_PATH)/models/loadsource.cpp \
	$(TENGINE_PATH)/network/commands.cpp \
	$(TENGINE_PATH)/network/replication.cpp \
	$(TENGINE_PATH)/network/network_loopback.cpp \
	$(TENGINE_PATH)/physics/tphysics/tphysics.cpp \
	$(TENGINE_PATH)/physics/tphysics/heightmap.cpp \
	$(TENGINE_PATH)/physics/physicsmanager.cpp \
	$(TENGINE_PATH)/renderer/particles.cpp \
	$(TENGINE_PATH)/renderer/game_renderer.cpp \
	$(TENGINE_PATH)/renderer/game_renderingcontext.cpp \
	$(TENGINE_PATH)/renderer/roperenderer.cpp \
	$(TENGINE_PATH)/sound/sound.cpp \
	$(TENGINE_PATH)/ui/hudviewport.cpp \
	$(TENGINE_PATH)/ui/gamewindow.cpp \
	$(TENGINE_PATH)/ui/chatbox.cpp \
	$(TENGINE_PATH)/ui/instructor.cpp \
	$(TENGINE_PATH)/lobby/lobby_client.cpp \
	$(TENGINE_PATH)/lobby/lobby_server.cpp \
	$(TENGINE_PATH)/portals/portal_stubs.cpp \
	$(TOYS_PATH)/toy.cpp \
	$(TOYS_PATH)/toy_util.cpp \
	$(TOYS_PATH)/toy.pb.cc \

LOCAL_SHARED_LIBRARIES := tinker SDL2 SDL2_mixer protobuf250

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -lGLESv3 -llog -landroid

include $(BUILD_STATIC_LIBRARY)
