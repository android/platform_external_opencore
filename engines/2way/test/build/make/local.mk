# Get the current local path as the first operation
LOCAL_PATH := $(call get_makefile_dir)

# Clear out the variables used in the local makefiles
include $(MK)/clear.mk


ifeq ($(USING_MONA),1)
TARGET := pv2way_mona_engine_test
TWOWAY_TARGET := pv2way_mona_engine_test
else
ifeq ($(USING_OMX),1)
TARGET := pv2way_omx_engine_test
TWOWAY_TARGET := pv2way_omx_engine_test
else
TARGET := pv2way_engine_test
TWOWAY_TARGET := pv2way_engine_test
endif
endif


ifeq ($(USING_MONA),1)
MONA_FLAGS = -DUSING_MONA
XCPPFLAGS += -DPV_USE_AMR_CODECS $(SIPCPPFLAGS) $(MONA_FLAGS) $(OMX_FLAGS)
else
XCPPFLAGS += -DPV_USE_AMR_CODECS $(SIPCPPFLAGS) $(OMX_FLAGS)
endif

XCXXFLAGS += $(FLAG_COMPILE_WARNINGS_AS_ERRORS)

ifeq ($(USING_MONA),1)
XINCDIRS +=  ../../h223/mona/include  ../../h324/tsc/mona/include 
endif

SRCDIR := ../../src
INCSRCDIR := ../../include
XINCDIRS +=  ../../../pvlogger/src \
             ../../../sample_app/pv2waysample/include \
             ../../../../../nodes/pvdummyinputnode/include \
             ../../../include

SRCS := alloc_dealloc_test.cpp \
	av_duplicate_test.cpp \
	test_engine.cpp \
	test_engine_utility.cpp \
	engine_test.cpp \
	init_cancel_test.cpp \
	init_test.cpp \
	test_base.cpp \
	pv_2way_unittest_source_and_sinks.cpp \
	../../pvlogger/src/pv_logger_impl.cpp

SRCS_324 = av_test.cpp \
	acceptable_formats_test.cpp \
	connect_cancel_test.cpp \
	connect_test.cpp \
	audio_only_test.cpp \
	video_only_test.cpp \
	user_input_test.cpp

SRCS += $(SRCS_324)



BASE_LIBS = pv2waysample \
   pv2wayengine \
   pv324m 

END_LIBS = pvdummyinputnode pvdummyoutputnode \
   pvclientserversocketnode \
   pvmediainputnode \
   pvmediaoutputnode \
   pvmiofileinput \
   pvmiofileoutput\
   pvmf \
   pvvideoparsernode \
   colorconvert \
   pvdecoder_gsmamr \
   pvencoder_gsmamr \
   pvmediadatastruct \
   pv_amr_nb_common_lib \
   pvamrwbdecoder \
   pvmp4decoder \
   pvm4vencoder \
   pvcommsionode \
   pvgendatastruct \
   pvgeneraltools \
   pvmimeutils \
   pvthreadmessaging \
   pvlatmpayloadparser \
   pvmio_comm_loopback \
   unit_test \
   osclio \
   osclproc \
   osclutil \
   osclmemory \
   osclerror \
   osclbase


ifeq ($(USING_OMX),1)
ifeq ($(pv2wayengine_lib),y)
LIBS = \
   pvomxvideodecnode \
   pvomxaudiodecnode \
   pvomxencnode \
   pvomxbasedecnode \
   omx_common_lib \
   omx_m4v_component_lib \
   omx_amr_component_lib \
   omx_amrenc_component_lib \
   omx_m4venc_component_lib \
   omx_baseclass_lib \
   m4v_config \
   pvomx_proxy_lib \
   omx_queue_lib \
   pv_config_parser \
   getactualaacconfig \
   osclregcli \
   osclregserv \
   threadsafe_callback_ao

FULL_LIBS := $(BASE_LIBS) + $(LIBS) + $(END_LIBS)

else
ifeq ($(pv2wayengine_lib),m)
FULL_LIBS =  opencore_2way \
   pvomxvideodecnode \
   pvomxaudiodecnode \
   pvomxencnode \
   pvomxbasedecnode \
   omx_common_lib \
   omx_m4v_component_lib \
   omx_m4venc_component_lib \
   omx_baseclass_lib \
   pvomx_proxy_lib \
   omx_queue_lib \
   unit_test \
   opencore_common 
	     
endif
endif

else
# NON-OMX
LIBS =  pvvideodecnode pvamrencnode gsmamrdecnode \
	pvvideoencnode 

FULL_LIBS := $(BASE_LIBS) + $(LIBS) + $(END_LIBS)

endif



LIBS := $(FULL_LIBS)


ifneq ($(HOST_ARCH),win32)
SYSLIBS += $(SYS_THREAD_LIB)
endif

include $(MK)/prog.mk





ifeq ($(HOST_ARCH),win32)
TWOWAY_TARGET := ${TARGET}.exe
TWOWAYFULL_TARGET := ${TWOWAY_TARGET}
TWOWAY_TEST_DIR := build\2way_test
RUNPREF := 
else
TWOWAY_TEST_DIR := ${BUILD_ROOT}/2way_test
TWOWAYFULL_TARGET := ./${TWOWAY_TARGET}
endif
 

run_2way_test:: $(REALTARGET) default
	$(quiet) ${RM} -r $(TWOWAY_TEST_DIR)
	$(quiet) ${MKDIR} -p $(TWOWAY_TEST_DIR)
	$(quiet) $(CP) $(SRC_ROOT)/tools_v2/build/package/opencore/elem/common/pvplayer.cfg $(TWOWAY_TEST_DIR)
	$(quiet) $(CP) $(SRC_ROOT)/engines/2way/pvlogger/config/pvlogger.ini $(TWOWAY_TEST_DIR)
	$(quiet) $(CP) ${BUILD_ROOT}/bin/${BUILD_ARCH}/$(TWOWAY_TARGET) $(TWOWAY_TEST_DIR)
	$(quiet) $(CP) -r $(SRC_ROOT)/engines/2way/test/test_data/* $(TWOWAY_TEST_DIR)
ifeq ($(HOST_ARCH),win32)
	$(quiet) $(CP) ${BUILD_ROOT}/installed_lib/${BUILD_ARCH}/* $(TWOWAY_TEST_DIR)
	cd $(TWOWAY_TEST_DIR) && $(TWOWAYFULL_TARGET) $(TEST_ARGS) $(SOURCE_ARGS)
else
	$(quiet) export LD_LIBRARY_PATH=${BUILD_ROOT}/installed_lib/${BUILD_ARCH} && \
	cd $(TWOWAY_TEST_DIR) && $(TWOWAYFULL_TARGET) $(TEST_ARGS) $(SOURCE_ARGS)
endif
