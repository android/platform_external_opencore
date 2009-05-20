LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmf_dummy_fileoutput_inport.cpp \
 	src/pvmf_dummy_fileoutput_node.cpp \
 	src/pvmf_dummy_fileoutput_factory.cpp \
 	src/pvmf_dummy_fileoutput_node_cap_config.cpp


LOCAL_MODULE := libpvdummyoutputnode

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/pvdummyoutputnode/src \
 	$(PV_TOP)/nodes/pvdummyoutputnode/include \
 	$(PV_TOP)/nodes/pvdummyoutputnode/include \
 	$(PV_TOP)/pvmi/pvmf/include \
 	$(PV_TOP)/nodes/common/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvmf_dummy_fileoutput_config.h \
 	include/pvmf_dummy_fileoutput_factory.h

include $(BUILD_STATIC_LIBRARY)
