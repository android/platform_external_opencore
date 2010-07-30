LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	autodetect.cpp \
	metadatadriver.cpp \
	playerdriver.cpp \
	thread_init.cpp \
	mediascanner.cpp \
	pvmediascanner.cpp \
	android_surface_output.cpp \
	android_audio_output.cpp \
	android_audio_stream.cpp \
	android_audio_mio.cpp \
	android_audio_output_threadsafe_callbacks.cpp \
	extension_handler_registry.cpp \
	PVPlayerExtHandler.cpp

ifeq ($(PLATFORM_VERSION),1.5)
else ifeq ($(PLATFORM_VERSION),1.6)
else ifeq ($(PLATFORM_VERSION),2.1)
  ifeq ($(PV_WERROR),1)
    LOCAL_CFLAGS := -Wno-psabi
  endif
else
  ifeq ($(PV_WERROR),1)
    LOCAL_CFLAGS := -Wno-psabi
  endif
endif

ifeq ($(PV_WERROR),1)
 LOCAL_CFLAGS += -Werror
endif

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
	external/tremolo/Tremolo \
        external/wavpack/include \
	external/icu4c/common \
	$(call include-path-for, graphics corecg)

LOCAL_MODULE := libandroidpv

LOCAL_LDLIBS += 

include $(BUILD_STATIC_LIBRARY)

