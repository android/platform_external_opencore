LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pv_2way_audio_input_mio_node_factory.cpp \
 	src/pv_2way_video_input_mio_node_factory.cpp \
 	src/pv_2way_audio_output_mio_node_factory.cpp \
 	src/pv_2way_dummy_output_mio_node_factory.cpp \
 	src/pv_2way_dummy_input_mio_node_factory.cpp \
 	src/pv_2way_media_input_mio_node_factory.cpp \
 	src/pv_2way_media_output_mio_node_factory.cpp \
 	src/pv_2way_source_and_sinks_base.cpp \
 	src/pv_2way_audio_sink.cpp \
 	src/pv_2way_audio_source.cpp \
 	src/pv_2way_video_sink.cpp \
 	src/pv_2way_video_source.cpp \
 	src/pv_2way_media.cpp \
 	src/pv_2way_mio.cpp \
 	src/pv_2way_sink.cpp \
 	src/pv_2way_source.cpp \
 	src/twowaysocket.cpp


LOCAL_MODULE := libpv2waysample

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/engines/2way/sample_app/pv2waysample/src \
 	$(PV_TOP)/engines/2way/sample_app/pv2waysample/include \
 	$(PV_TOP)/engines/2way/sample_app/pv2waysample/src \
 	$(PV_TOP)/engines/2way/sample_app/pv2waysample/include \
 	$(PV_TOP)/nodes/pvdummyinputnode/include \
 	$(PV_TOP)/nodes/pvdummyoutputnode/include \
 	$(PV_TOP)/protocols/systems/common/include \
 	$(PV_TOP)/nodes/pvclientserversocketnode/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pv_2way_codecspecifier.h \
 	include/pv_2way_source_and_sinks_base.h \
 	include/pv_2way_mio.h \
 	include/pv_2way_mio_node_factory.h \
 	include/twowaysocket.h

include $(BUILD_STATIC_LIBRARY)
