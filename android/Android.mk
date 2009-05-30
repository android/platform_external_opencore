LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# Use the PV variables
include external/opencore/Config.mk

LOCAL_SRC_FILES := \
	autodetect.cpp \
	metadatadriver.cpp \
	playerdriver.cpp \
	thread_init.cpp \
	mediascanner.cpp \
	android_surface_output.cpp \
	android_audio_output.cpp \
	android_audio_stream.cpp \
	android_audio_mio.cpp \
	android_audio_output_threadsafe_callbacks.cpp

LOCAL_C_INCLUDES := $(PV_INCLUDES) \
	$(PV_TOP)/engines/common/include \
	$(PV_TOP)/fileformats/mp4/parser/include \
	$(PV_TOP)/pvmi/media_io/pvmiofileoutput/include \
	$(PV_TOP)/nodes/pvmediaoutputnode/include \
	$(PV_TOP)/nodes/pvmediainputnode/include \
	$(PV_TOP)/nodes/pvmp4ffcomposernode/include \
	$(PV_TOP)/engines/player/include \
	$(PV_TOP)/nodes/common/include \
	$(PV_TOP)/fileformats/pvx/parser/include \
        $(PV_TOP)/nodes/pvprotocolenginenode/download_protocols/common/src \
	libs/drm/mobile1/include \
    include/graphics \
	external/skia/include/corecg \
	external/tremor/Tremor \
	external/icu4c/common \
	$(call include-path-for, graphics corecg)

LOCAL_MODULE := libandroid_opencore_player

-include $(PV_TOP)/Android_platform_extras.mk
LOCAL_SHARED_LIBRARIES += libopencore_player libopencore_common

-include $(PV_TOP)/Android_system_extras.mk

include $(BUILD_SHARED_LIBRARY)

