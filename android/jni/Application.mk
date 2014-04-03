
# Uncomment this if you're using STL in your project
# See CPLUSPLUS-SUPPORT.html in the NDK documentation for more information
APP_STL := gnustl_static 
NDK_TOOLCHAIN_VERSION := 4.8
LOCAL_CFLAGS += -D__GXX_EXPERIMENTAL_CXX0X__
APP_CPPFLAGS += -std=c++11 -frtti
#APP_ABI := armeabi armeabi-v7a x86
APP_ABI := armeabi
