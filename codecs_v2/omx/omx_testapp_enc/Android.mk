LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/omx_threadsafe_callbacks.cpp \
 	src/omxenctest.cpp \
 	src/omxenctestbase.cpp \
 	src/omxtest_buffer_busy_enc.cpp \
 	src/omxtest_eosmissing_enc.cpp \
 	src/omxtest_extra_partialframes_enc.cpp \
 	src/omxtest_get_role_enc.cpp \
 	src/omxtest_param_negotiation_enc.cpp \
 	src/omxtest_partialframe_enc.cpp \
 	src/omxtest_pause_resume_enc.cpp \
 	src/omxtest_usebuffer_enc.cpp \
 	src/omxtest_without_marker_enc.cpp


LOCAL_MODULE := test_omxenc_client

LOCAL_CFLAGS :=  $(PV_CFLAGS)

LOCAL_ARM_MODE := arm

LOCAL_STATIC_LIBRARIES := libunit_test  

LOCAL_SHARED_LIBRARIES :=  libopencore_author libopencore_common

LOCAL_C_INCLUDES := \
	$(PV_TOP)/codecs_v2/omx/omx_testapp_enc/src \
 	$(PV_TOP)/codecs_v2/omx/omx_testapp_enc/include \
 	$(PV_TOP)/codecs_v2/common/include \
 	$(PV_TOP)/codecs_v2/omx/omx_testapp_enc/src/single_core \
 	$(PV_TOP)/codecs_v2/omx/omx_testapp_enc/config/android \
 	$(PV_TOP)/pvmi/pvmf/include \
 	$(PV_TOP)/nodes/common/include \
 	$(PV_TOP)/extern_libs_v2/khronos/openmax/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
 	

-include $(PV_TOP)/Android_system_extras.mk

include $(BUILD_EXECUTABLE)
