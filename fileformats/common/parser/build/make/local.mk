# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvfileparserutils
XCXXFLAGS := $(FLAG_COMPILE_WARNINGS_AS_ERRORS)






SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvfile.cpp \
	pvmi_datastreamsyncinterface_ref_factory.cpp \
	pvmi_datastreamsyncinterface_ref_impl.cpp \
	pvmi_external_download_datastream_factory.cpp \
	pvmi_external_download_datastream_impl.cpp \
	pvmi_external_download_file_monitor.cpp \
	pvmi_external_download_simulator.cpp


HDRS := pvfile.h \
	pvmi_datastreamsyncinterface_ref_factory.h \
	virtual_buffer.h \
	pvmi_external_download_extension_interfaces.h \
	pvmi_external_download_datastream_factory.h \
	pvmi_external_download_datastream_impl.h \
	pvmi_external_download_file_monitor.h \
	pvmi_external_download_simulator.h

 
include $(MK)/library.mk
install:: headers-install

 
