# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := colorconvert

XCXXFLAGS := $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

OPTIMIZE_FOR_PERFORMANCE_OVER_SIZE := true

XCPPFLAGS += -DFALSE=false

SRCDIR := ../../src
INCSRCDIR := ../../include

SRCS :=	ccrgb16toyuv420.cpp \
	ccyuv422toyuv420.cpp \
	cczoomrotation16.cpp \
	cczoomrotationbase.cpp \
	ccrgb24toyuv420.cpp \
	ccrgb12toyuv420.cpp \
	ccyuv420semiplnrtoyuv420plnr.cpp \
	ccyuv420toyuv420semi.cpp

ifneq ($(ARCHITECTURE),android)
SRCS += cczoomrotation12.cpp \
        cczoomrotation24.cpp \
        cczoomrotation32.cpp \
	ccrgb24torgb16.cpp \
	cpvvideoblend.cpp
endif

HDRS :=  cczoomrotationbase.h \
	cczoomrotation16.h \
	ccrgb16toyuv420.h \
	ccyuv422toyuv420.h \
	colorconv_config.h \
	ccrgb24toyuv420.h \
	ccrgb12toyuv420.h \
	ccyuv420semitoyuv420.h \
	ccyuv420toyuv420semi.h

ifneq ($(ARCHITECTURE),android)
SRCS += cczoomrotation12.h \
        cczoomrotation24.h \
        cczoomrotation32.h \
	ccrgb24torgb16.h \
	pvvideoblend.h
endif

include $(MK)/library.mk
