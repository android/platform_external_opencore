LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvmf_clientserver_socket_factory.cpp \
 	src/pvmf_clientserver_socket_node.cpp \
 	src/pvmf_clientserver_socket_port.cpp


LOCAL_MODULE := libpvclientserversocketnode

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/nodes/pvclientserversocketnode/src \
 	$(PV_TOP)/nodes/pvclientserversocketnode/include \
 	$(PV_TOP)/nodes/pvclientserversocketnode/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvmf_clientserver_socket_factory.h \
 	include/pvmf_clientserver_socket_node.h \
 	include/pvmf_clientserver_socket_port.h \
 	include/pvmf_clientserver_socket_tuneables.h \
 	include/pvmf_socket_buffer_allocators.h

include $(BUILD_STATIC_LIBRARY)
