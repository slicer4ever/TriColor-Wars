
LOCAL_PATH := $(call my-dir)
CONFIG = Release
ifdef NDK_DEBUG
	CONFIG = Debug
endif

BaseDir := ../../../
LightWaveDir := $(BaseDir)../Lightwave/
Src := $(BaseDir)Source/

include $(CLEAR_VARS)
LOCAL_MODULE := LWCore
LOCAL_SRC_FILES := $(LightWaveDir)Framework/Binarys/$(CONFIG)/$(TARGET_ARCH_ABI)/libLWCore.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := LWPlatform
LOCAL_SRC_FILES := $(LightWaveDir)Framework/Binarys/$(CONFIG)/$(TARGET_ARCH_ABI)/libLWPlatform.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := LWVideo
LOCAL_SRC_FILES := $(LightWaveDir)Framework/Binarys/$(CONFIG)/$(TARGET_ARCH_ABI)/libLWVideo.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := LWAudio
LOCAL_SRC_FILES := $(LightWaveDir)Framework/Binarys/$(CONFIG)/$(TARGET_ARCH_ABI)/libLWAudio.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libpng
LOCAL_SRC_FILES := $(LightWaveDir)Dependency/Binarys/$(CONFIG)/$(TARGET_ARCH_ABI)/libpng.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libfreetype
LOCAL_SRC_FILES := $(LightWaveDir)Dependency/Binarys/$(CONFIG)/$(TARGET_ARCH_ABI)/libfreetype.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := LWEngine
LOCAL_SRC_FILES := $(LightWaveDir)Engine/Binarys/$(CONFIG)/$(TARGET_ARCH_ABI)/libLWEngine.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libogg
LOCAL_SRC_FILES := $(LightWaveDir)Dependency/Binarys/$(CONFIG)/$(TARGET_ARCH_ABI)/libogg.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libvorbis
LOCAL_SRC_FILES := $(LightWaveDir)Dependency/Binarys/$(CONFIG)/$(TARGET_ARCH_ABI)/libvorbis.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
Includes = $(LOCAL_PATH)../$(LightWaveDir)Framework/Includes/C++11/
Includes += $(LOCAL_PATH)../$(LightWaveDir)Engine/Includes/C++11/
Includes += $(LOCAL_PATH)../$(BaseDir)Includes/C++11/
Includes += $(LOCAL_PATH)../$(BaseDir)Includes/Neosnake/

LOCAL_MODULE    := TriColorWars
LOCAL_SRC_FILES += $(Src)C++11/App.cpp
LOCAL_SRC_FILES += $(Src)C++11/main.cpp
LOCAL_SRC_FILES += $(Src)C++11/Renderer.cpp
LOCAL_SRC_FILES += $(Src)C++11/Sprite.cpp
LOCAL_SRC_FILES += $(Src)C++11/State_Game.cpp
LOCAL_SRC_FILES += $(Src)C++11/State_Menu.cpp

LOCAL_C_INCLUDES := $(Includes)
LOCAL_STATIC_LIBRARIES := LWEngine LWAudio LWCore LWVideo libfreetype libpng libvorbis
LOCAL_SHARED_LIBRARIES :=
LOCAL_WHOLE_STATIC_LIBRARIES := LWPlatform libogg
LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv2 -lOpenSLES -lz -latomic
LOCAL_CFLAGS :=
LOCAL_CPPFLAGS := -frtti -std=c++14
LOCAL_LDFLAGS :=
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a armeabi-v7a-hard x86)
LOCAL_ARM_NEON := true
LOCAL_CFLAGS := -DPNG_USE_ARM_NEON
endif
include $(BUILD_SHARED_LIBRARY)
