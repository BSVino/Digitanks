LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := freetype

FREETYPE_PATH := ../../../../ext-deps/freetype-2.3.11

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(FREETYPE_PATH)/src $(LOCAL_PATH)/$(FREETYPE_PATH)/include

LOCAL_CFLAGS += -DFT2_BUILD_LIBRARY

LOCAL_SRC_FILES := \
    $(FREETYPE_PATH)/src/winfonts/winfnt.c \
    $(FREETYPE_PATH)/src/autofit/autofit.c \
    $(FREETYPE_PATH)/src/bdf/bdf.c \
    $(FREETYPE_PATH)/src/cff/cff.c \
    $(FREETYPE_PATH)/src/base/ftbase.c \
    $(FREETYPE_PATH)/src/base/ftbitmap.c \
    $(FREETYPE_PATH)/src/cache/ftcache.c \
    $(FREETYPE_PATH)/src/base/ftfstype.c \
    $(FREETYPE_PATH)/src/base/ftgasp.c \
    $(FREETYPE_PATH)/src/base/ftglyph.c \
    $(FREETYPE_PATH)/src/gzip/ftgzip.c \
    $(FREETYPE_PATH)/src/base/ftinit.c \
    $(FREETYPE_PATH)/src/lzw/ftlzw.c \
    $(FREETYPE_PATH)/src/base/ftstroke.c \
    $(FREETYPE_PATH)/src/base/ftsystem.c \
    $(FREETYPE_PATH)/src/smooth/smooth.c \
    $(FREETYPE_PATH)/src/base/ftbbox.c \
    $(FREETYPE_PATH)/src/base/ftmm.c \
    $(FREETYPE_PATH)/src/base/ftpfr.c \
    $(FREETYPE_PATH)/src/base/ftsynth.c \
    $(FREETYPE_PATH)/src/base/fttype1.c \
    $(FREETYPE_PATH)/src/base/ftwinfnt.c \
    $(FREETYPE_PATH)/src/pcf/pcf.c \
    $(FREETYPE_PATH)/src/pfr/pfr.c \
    $(FREETYPE_PATH)/src/psaux/psaux.c \
    $(FREETYPE_PATH)/src/pshinter/pshinter.c \
    $(FREETYPE_PATH)/src/psnames/psmodule.c \
    $(FREETYPE_PATH)/src/raster/raster.c \
    $(FREETYPE_PATH)/src/sfnt/sfnt.c \
    $(FREETYPE_PATH)/src/truetype/truetype.c \
    $(FREETYPE_PATH)/src/type1/type1.c \
    $(FREETYPE_PATH)/src/cid/type1cid.c \
    $(FREETYPE_PATH)/src/type42/type42.c \

include $(BUILD_SHARED_LIBRARY)
