LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# Use the PV variables
include external/opencore/Config.mk

LOCAL_SRC_FILES := \
	authordriver.cpp \
	PVMediaRecorder.cpp \
	android_camera_input.cpp \
	android_audio_input.cpp \
	android_audio_input_threadsafe_callbacks.cpp \
	../thread_init.cpp \

# board-specific configuration
LOCAL_CFLAGS := $(BOARD_OPENCORE_FLAGS)

LOCAL_C_INCLUDES := $(PV_INCLUDES) \
	$(PV_TOP)/engines/common/include \
	$(PV_TOP)/codecs_v2/omx/omx_common/include \
	$(PV_TOP)/fileformats/mp4/parser/include \
	$(PV_TOP)/pvmi/media_io/pvmiofileoutput/include \
	$(PV_TOP)/nodes/pvmediaoutputnode/include \
	$(PV_TOP)/nodes/pvmediainputnode/include \
	$(PV_TOP)/nodes/pvmp4ffcomposernode/include \
	$(PV_TOP)/engines/player/include \
	$(PV_TOP)/nodes/common/include \
	external/tremor/Tremor \
	libs/drm/mobile1/include \
    $(call include-path-for, graphics corecg)

LOCAL_MODULE := libandroid_opencore_author

-include $(PV_TOP)/Android_platform_extras.mk
LOCAL_SHARED_LIBRARIES += libopencore_author libopencore_common

-include $(PV_TOP)/Android_system_extras.mk

include $(BUILD_SHARED_LIBRARY)

