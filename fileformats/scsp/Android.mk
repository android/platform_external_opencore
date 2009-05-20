LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
 	src/pvmf_shoutcast_stream_parser.cpp


LOCAL_MODULE := libscsp

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/fileformats/scsp/src \
 	$(PV_TOP)/fileformats/scsp/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
 	include/pvmf_shoutcast_stream_parser.h

include $(BUILD_STATIC_LIBRARY)
