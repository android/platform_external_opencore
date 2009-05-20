# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvmp3ffrecognizer

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvmp3ffrec_factory.cpp \
       pvmp3ffrec_plugin.cpp

HDRS := pvmp3ffrec_factory.h

include $(MK)/library.mk
