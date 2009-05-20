# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvwavffrecognizer

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS := pvwavffrec_factory.cpp \
       pvwavffrec_plugin.cpp

HDRS := pvwavffrec_factory.h

include $(MK)/library.mk
