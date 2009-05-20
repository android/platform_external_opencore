# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := test_omxenc_client

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



LIBS :=  pv_video_config_parser \
        omx_common_lib \
	omx_m4venc_component_lib \
	omx_avcenc_component_lib \
	omx_amrenc_component_lib \
	omx_aacenc_component_lib \
	omx_aac_component_lib \
	omx_amr_component_lib \
	omx_mp3_component_lib \
	omx_wma_component_lib \
	omx_avc_component_lib \
	omx_m4v_component_lib \
	omx_wmv_component_lib \
	omx_baseclass_lib \
	pv_config_parser \
	getactualaacconfig \
	m4v_config \
	pvmp4decoder \
	pvm4vencoder \
	pvavcdecoder \
	pvavch264enc \
	pv_avc_common_lib \
	pvamrwbdecoder \
	pvdecoder_gsmamr \
	pvencoder_gsmamr \
	pv_amr_nb_common_lib \
	pv_aac_enc \
	pv_aac_dec \
	pvmp3 \
	wmadecoder \
	wmvdecoder \
	colorconvert \
	unit_test \
	pvomx_proxy_lib \
	omx_queue_lib \
	threadsafe_callback_ao \
	osclio \
	osclproc \
	pvlogger \
	osclutil \
	osclmemory \
	osclerror \
        osclbase 
           

SYSLIBS += $(SYS_THREAD_LIB)

include $(MK)/prog.mk
