# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := pvoma1passthruplugininterface

SRCDIR := ../../src

SRCS := pvmf_oma1_passthru_plugin_module.cpp

include $(MK)/library.mk

