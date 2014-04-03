LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := ftgl3

FTGL3_PATH := ../../../../ext-deps/ftgl-gl3

LOCAL_C_INCLUDES := . $(LOCAL_PATH)/$(FTGL3_PATH)/src $(LOCAL_PATH)/$(FTGL3_PATH)/../freetype-2.3.11/include

LOCAL_SRC_FILES := \
    $(FTGL3_PATH)/src/FTBuffer.cpp \
    $(FTGL3_PATH)/src/FTCharmap.cpp \
    $(FTGL3_PATH)/src/FTContour.cpp \
    $(FTGL3_PATH)/src/FTFace.cpp \
    $(FTGL3_PATH)/src/FTGlyphContainer.cpp \
    $(FTGL3_PATH)/src/FTLibrary.cpp \
    $(FTGL3_PATH)/src/FTPoint.cpp \
    $(FTGL3_PATH)/src/FTSize.cpp \
    $(FTGL3_PATH)/src/FTVectoriser.cpp \
    $(FTGL3_PATH)/src/FTGL/gl3Glue.cpp \
    $(FTGL3_PATH)/src/FTFont/FTFont.cpp \
    $(FTGL3_PATH)/src/FTFont/FTFontGlue.cpp \
    $(FTGL3_PATH)/src/FTFont/FTTextureFont.cpp \
    $(FTGL3_PATH)/src/FTGlyph/FTGlyph.cpp \
    $(FTGL3_PATH)/src/FTGlyph/FTGlyphGlue.cpp \
    $(FTGL3_PATH)/src/FTGlyph/FTTextureGlyph.cpp \
    $(FTGL3_PATH)/src/FTLayout/FTLayout.cpp \
    $(FTGL3_PATH)/src/FTLayout/FTLayoutGlue.cpp \
    $(FTGL3_PATH)/src/FTLayout/FTSimpleLayout.cpp \

LOCAL_SHARED_LIBRARIES := freetype

LOCAL_LDLIBS := -lGLESv3

include $(BUILD_SHARED_LIBRARY)
