# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := test_omx_client

XINCDIRS += ../../../../common/include ../../src/single_core ../../config/android ../../../../../pvmi/pvmf/include ../../../../../nodes/common/include ../../../../../extern_libs_v2/khronos/openmax/include


SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := omx_threadsafe_callbacks.cpp \
	omxdectest.cpp \
	omxdectestbase.cpp \
	omxtest_buffer_busy.cpp \
	omxtest_corrupt_nal.cpp \
	omxtest_dynamic_reconfig.cpp \
	omxtest_eos_missing.cpp \
	omxtest_extra_partialframes.cpp \
	omxtest_flush_eos.cpp \
	omxtest_flush_port.cpp \
	omxtest_get_role.cpp \
	omxtest_incomplete_nal.cpp \
	omxtest_missing_nal.cpp \
	omxtest_param_negotiation.cpp \
	omxtest_partialframes.cpp \
	omxtest_pause_resume.cpp \
	omxtest_portreconfig_transit_1.cpp \
	omxtest_portreconfig_transit_2.cpp \
	omxtest_portreconfig_transit_3.cpp \
	omxtest_reposition.cpp \
	omxtest_usebuffer.cpp \
	omxtest_without_marker.cpp



LIBS := unit_test opencore_player opencore_common 
        

SYSLIBS += $(SYS_THREAD_LIB)

include $(MK)/prog.mk
