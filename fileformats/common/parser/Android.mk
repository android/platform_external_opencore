LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/pvfile.cpp \
 	src/pvmi_datastreamsyncinterface_ref_factory.cpp \
 	src/pvmi_datastreamsyncinterface_ref_impl.cpp \
 	src/pvmi_external_download_datastream_factory.cpp \
 	src/pvmi_external_download_datastream_impl.cpp \
 	src/pvmi_external_download_file_monitor.cpp \
 	src/pvmi_external_download_simulator.cpp


LOCAL_MODULE := libpvfileparserutils

LOCAL_CFLAGS :=  $(PV_CFLAGS)



LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_C_INCLUDES := \
	$(PV_TOP)/fileformats/common/parser/src \
 	$(PV_TOP)/fileformats/common/parser/include \
 	$(PV_INCLUDES)

LOCAL_COPY_HEADERS_TO := $(PV_COPY_HEADERS_TO)

LOCAL_COPY_HEADERS := \
	include/pvfile.h \
 	include/pvmi_datastreamsyncinterface_ref_factory.h \
 	include/virtual_buffer.h \
 	include/pvmi_external_download_extension_interfaces.h \
 	include/pvmi_external_download_datastream_factory.h \
 	include/pvmi_external_download_datastream_impl.h \
 	include/pvmi_external_download_file_monitor.h \
 	include/pvmi_external_download_simulator.h

include $(BUILD_STATIC_LIBRARY)
