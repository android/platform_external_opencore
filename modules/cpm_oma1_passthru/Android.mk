LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
 	src/pvmf_oma1_passthru_plugin_module.cpp


LOCAL_MODULE := libpvoma1passthruplugininterface

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/modules/cpm_oma1_passthru/src \
 	$(PV_TOP)/modules/cpm_oma1_passthru/build/make \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
 	

include $(BUILD_STATIC_LIBRARY)
