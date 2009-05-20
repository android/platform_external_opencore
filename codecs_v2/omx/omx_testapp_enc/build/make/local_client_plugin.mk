# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := test_omxenc_client_plugin

XINCDIRS += ../../../../../extern_libs_v2/khronos/openmax/include


SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := omx_threadsafe_callbacks.cpp \
	omxenctest.cpp \
	omxenctestbase.cpp \
	omxtest_buffer_busy_enc.cpp \
	omxtest_eosmissing_enc.cpp \
	omxtest_extra_partialframes_enc.cpp \
	omxtest_get_role_enc.cpp \
	omxtest_param_negotiation_enc.cpp \
	omxtest_partialframe_enc.cpp \
	omxtest_pause_resume_enc.cpp \
	omxtest_usebuffer_enc.cpp \
	omxtest_without_marker_enc.cpp


LIBS := threadsafe_callback_ao \
        osclio \
        osclproc \
        pvlogger \
        osclutil \
        osclmemory \
        osclerror \
        osclbase 

SYSLIBS += $(SYS_THREAD_LIB)

include $(MK)/prog.mk
