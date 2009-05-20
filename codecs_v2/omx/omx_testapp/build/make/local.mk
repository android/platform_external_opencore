# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk

TARGET := test_omx_client

XINCDIRS += ../../../../../extern_libs_v2/khronos/openmax/include


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



LIBS :=  pv_video_config_parser \
        omx_common_lib \
        omx_mp3_component_lib \
        pvmp3 \
        omx_m4venc_component_lib \
	omx_avcenc_component_lib \
	omx_amrenc_component_lib \
	omx_aacenc_component_lib \
        omx_avc_component_lib \
        pvavch264enc \
        pvavcdecoder \
        pv_avc_common_lib \
        omx_amr_component_lib \
        pvencoder_gsmamr \
        pvdecoder_gsmamr \
        pv_amr_nb_common_lib \
        pvamrwbdecoder \
        omx_m4v_component_lib \
        pvm4vencoder \
        pvmp4decoder \
	m4v_config \
        omx_wmv_component_lib \
        wmvdecoder \
	omx_aac_component_lib \
        pv_aac_dec \
        pv_aac_enc \
	omx_wma_component_lib \
        wmadecoder \
        omx_baseclass_lib \
        pv_config_parser \
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
