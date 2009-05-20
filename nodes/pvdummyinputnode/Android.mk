LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmf_dummy_fileinput_node.cpp \
 	src/pvmf_fileinput_port.cpp


LOCAL_MODULE := libpvdummyinputnode

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/pvdummyinputnode/src \
 	$(PV_TOP)/nodes/pvdummyinputnode/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvmf_fileinput_node_extension.h \
 	include/pvmf_dummy_fileinput_node_factory.h \
 	include/pvmf_dummy_fileinput_node.h \
 	include/pvmf_fileinput_node_internal.h \
 	include/pvmf_fileinput_port.h \
 	include/pvmf_fileinput_settings.h

include $(BUILD_STATIC_LIBRARY)
