LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/omx_threadsafe_callbacks.cpp \
 	src/omxdectest.cpp \
 	src/omxdectestbase.cpp \
 	src/omxtest_buffer_busy.cpp \
 	src/omxtest_corrupt_nal.cpp \
 	src/omxtest_dynamic_reconfig.cpp \
 	src/omxtest_eos_missing.cpp \
 	src/omxtest_extra_partialframes.cpp \
 	src/omxtest_flush_eos.cpp \
 	src/omxtest_flush_port.cpp \
 	src/omxtest_get_role.cpp \
 	src/omxtest_incomplete_nal.cpp \
 	src/omxtest_missing_nal.cpp \
 	src/omxtest_param_negotiation.cpp \
 	src/omxtest_partialframes.cpp \
 	src/omxtest_pause_resume.cpp \
 	src/omxtest_portreconfig_transit_1.cpp \
 	src/omxtest_portreconfig_transit_2.cpp \
 	src/omxtest_portreconfig_transit_3.cpp \
 	src/omxtest_reposition.cpp \
 	src/omxtest_usebuffer.cpp \
 	src/omxtest_without_marker.cpp


LOCAL_MODULE := test_omx_client

LOCAL_CFLAGS :=  $(PV_CFLAGS)

LOCAL_ARM_MODE := arm

LOCAL_STATIC_LIBRARIES := libunit_test  

LOCAL_SHARED_LIBRARIES :=  libopencore_player libopencore_common

LOCAL_C_INCLUDES := \
	$(PV_TOP)/codecs_v2/omx/omx_testapp/src \
 	$(PV_TOP)/codecs_v2/omx/omx_testapp/include \
 	$(PV_TOP)/codecs_v2/common/include \
 	$(PV_TOP)/codecs_v2/omx/omx_testapp/src/single_core \
 	$(PV_TOP)/codecs_v2/omx/omx_testapp/config/android \
 	$(PV_TOP)/pvmi/pvmf/include \
 	$(PV_TOP)/nodes/common/include \
 	$(PV_TOP)/extern_libs_v2/khronos/openmax/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
 	

-include $(PV_TOP)/Android_system_extras.mk

include $(BUILD_EXECUTABLE)
