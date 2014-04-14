LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL

TINKER_PATH := ../../../
DIGITANKS_PATH := ../../../digitanks/src/digitanks

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(TINKER_PATH)/ $(LOCAL_PATH)/$(TINKER_PATH)/common $(LOCAL_PATH)/$(TINKER_PATH)/common/math $(LOCAL_PATH)/$(TINKER_PATH)/tinker $(LOCAL_PATH)/$(TINKER_PATH)/tengine
LOCAL_C_INCLUDES += $(LOCAL_PATH)/$(DIGITANKS_PATH)

LOCAL_CFLAGS += -DTINKER_OPENGLES_3 -DTINKER_NO_TOOLS

ifeq ($(NDK_DEBUG),1)
    $(warning Building DEBUG MODE)
    LOCAL_CFLAGS += -g -D_DEBUG
else
    $(warning Building RELEASE MODE)
endif

# Add your application source files here...
LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.c \
	$(DIGITANKS_PATH)/main.cpp \
	$(DIGITANKS_PATH)/bot.cpp \
	$(DIGITANKS_PATH)/bot_campaign.cpp \
	$(DIGITANKS_PATH)/digitanksentity.cpp \
	$(DIGITANKS_PATH)/digitanksgame.cpp \
	$(DIGITANKS_PATH)/digitankslevel.cpp \
	$(DIGITANKS_PATH)/digitanksplayer.cpp \
	$(DIGITANKS_PATH)/dt_camera.cpp \
	$(DIGITANKS_PATH)/dt_common.cpp \
	$(DIGITANKS_PATH)/dt_lobbylistener.cpp \
	$(DIGITANKS_PATH)/dt_renderer.cpp \
	$(DIGITANKS_PATH)/dt_systems.cpp \
	$(DIGITANKS_PATH)/instructor_entity.cpp \
	$(DIGITANKS_PATH)/menumarcher.cpp \
	$(DIGITANKS_PATH)/powerup.cpp \
	$(DIGITANKS_PATH)/selectable.cpp \
	$(DIGITANKS_PATH)/supplyline.cpp \
	$(DIGITANKS_PATH)/terrain.cpp \
	$(DIGITANKS_PATH)/updates.cpp \
	$(DIGITANKS_PATH)/wreckage.cpp \
	$(DIGITANKS_PATH)/dissolver.cpp \
	$(DIGITANKS_PATH)/campaign/campaigndata.cpp \
	$(DIGITANKS_PATH)/campaign/campaignentity.cpp \
	$(DIGITANKS_PATH)/campaign/campaigninfo.cpp \
	$(DIGITANKS_PATH)/campaign/userfile.cpp \
	$(DIGITANKS_PATH)/structures/autoturret.cpp \
	$(DIGITANKS_PATH)/structures/buffer.cpp \
	$(DIGITANKS_PATH)/structures/collector.cpp \
	$(DIGITANKS_PATH)/structures/cpu.cpp \
	$(DIGITANKS_PATH)/structures/loader.cpp \
	$(DIGITANKS_PATH)/structures/props.cpp \
	$(DIGITANKS_PATH)/structures/resource.cpp \
	$(DIGITANKS_PATH)/structures/structure.cpp \
	$(DIGITANKS_PATH)/units/artillery.cpp \
	$(DIGITANKS_PATH)/units/barbarians.cpp \
	$(DIGITANKS_PATH)/units/digitank.cpp \
	$(DIGITANKS_PATH)/units/maintank.cpp \
	$(DIGITANKS_PATH)/units/mechinf.cpp \
	$(DIGITANKS_PATH)/units/mobilecpu.cpp \
	$(DIGITANKS_PATH)/units/scout.cpp \
	$(DIGITANKS_PATH)/units/standardtank.cpp \
	$(DIGITANKS_PATH)/weapons/baseweapon.cpp \
	$(DIGITANKS_PATH)/weapons/cameraguided.cpp \
	$(DIGITANKS_PATH)/weapons/laser.cpp \
	$(DIGITANKS_PATH)/weapons/missiledefense.cpp \
	$(DIGITANKS_PATH)/weapons/projectile.cpp \
	$(DIGITANKS_PATH)/weapons/specialshells.cpp \
	$(DIGITANKS_PATH)/weapons/turretmissile.cpp \
	$(DIGITANKS_PATH)/ui/digitankswindow.cpp \
	$(DIGITANKS_PATH)/ui/hud.cpp \
	$(DIGITANKS_PATH)/ui/lobbyui.cpp \
	$(DIGITANKS_PATH)/ui/menu.cpp \
	$(DIGITANKS_PATH)/ui/scenetree.cpp \
	$(DIGITANKS_PATH)/ui/ui.cpp \
	$(DIGITANKS_PATH)/ui/updatespanel.cpp \
	$(DIGITANKS_PATH)/ui/weaponpanel.cpp \

LOCAL_SHARED_LIBRARIES := tinker SDL2 SDL2_image SDL2_mixer protobuf250 ftgl3
LOCAL_STATIC_LIBRARIES := tengine

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -lGLESv3 -llog -landroid

include $(BUILD_SHARED_LIBRARY)
