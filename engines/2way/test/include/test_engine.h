/* ------------------------------------------------------------------
 * Copyright (C) 1998-2009 PacketVideo
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 * -------------------------------------------------------------------
 */
#ifndef TEST_ENGINE_H_INCLUDED
#define TEST_ENGINE_H_INCLUDED

#include "oscl_error.h"
#include "oscl_timer.h"
#include "oscl_mem.h"
#include "oscl_scheduler.h"
#include "oscl_utf8conv.h"
#include "pvlogger.h"
#include "pvlogger_stderr_appender.h"
#include "pvlogger_file_appender.h"
#include "pvlogger_time_and_id_layout.h"
#include "test_case.h"
#include "text_test_interpreter.h"
#include "pv_2way_interface.h"
#include "pv_2way_engine_factory.h"
#include "pv_2way_proxy_factory.h"
#include "pvmf_media_input_node_factory.h"
#include "pv_media_output_node_factory.h"
#include "pvmi_media_io_fileoutput.h"
#include "pvmi_mio_fileinput_factory.h"
#include "pv_engine_observer.h"
#include "pv_engine_observer_message.h"
#include "tsc_h324m_config_interface.h"
#include "engine_test.h"

#ifndef NO_2WAY_324
#include "pv_comms_io_node_factory.h"
#include "pvmi_mio_comm_loopback_factory.h"
#endif

#include "test_codecs.h"

#define RX_LOGGER_TAG _STRLIT_CHAR("pvcommionode.rx.bin")
#define TX_LOGGER_TAG _STRLIT_CHAR("pvcommionode.tx.bin")
#define PVSIP2WAY_PROFILE _STRLIT_CHAR("pvSIP2Way")
#define PVSIPDEMO_ADDRESS _STRLIT_CHAR("sip:pvSIPDemo@")
#define PVSIPDEMO2_ADDRESS _STRLIT_CHAR("sip:pvSIPDemo2@")
#define PVSIP_DEFAULT_REALM _STRLIT_CHAR("pvrealm")


#define TEST_RX_LOG_FILENAME _STRLIT("commrx.bin")
#define TEST_TX_LOG_FILENAME _STRLIT("commtx.bin")
#define TEST_LOG_FILENAME _STRLIT("test2way.log")
#define AUDIO_SOURCE_FILENAME _STRLIT("audio_in.if2")
#define AUDIO_SOURCE3_FILENAME _STRLIT("audio_in.amr")
#define AUDIO_SOURCE_RAW_FILENAME _STRLIT("pcm16testinput.pcm")
#define AUDIO_SINK_FILENAME _STRLIT("audio_if2_out.dat")
#define AUDIO_SINK_RAW_FILENAME _STRLIT("audio_pcm16_out.dat")
#define AUDIO_SINK2_FILENAME _STRLIT("audio_ietf_out.dat")
#define VIDEO_SOURCE_YUV_FILENAME _STRLIT("yuv420video.yuv")
#define VIDEO_SOURCE_H263_FILENAME _STRLIT("h263video.h263")
#define VIDEO_SOURCE_M4V_FILENAME _STRLIT("m4vvideo.m4v")
#define VIDEO_SINK_YUV_FILENAME _STRLIT("video_yuv_out.dat")
#define VIDEO_SINK_H263_FILENAME _STRLIT("video_h263_out.dat")
#define VIDEO_SINK_M4V_FILENAME _STRLIT("video_m4v_out.dat")
#define VIDEO_PREVIEW_FILENAME _STRLIT("video_preview_out.dat")
#define RECORDED_CALL_FILENAME _STRLIT("recorded_call.mp4")
#define AUDIO_ONLY_PLAY_FILENAME _STRLIT("pv-amr-122_novisual.3gp")
#define AUDIO_H263_PLAY_FILENAME _STRLIT("pv-amr-122_h263-64.3gp")
#define AUDIO_MPEG4_PLAY_FILENAME _STRLIT("pv2-amr122_mpeg4-rvlcs-64.3gp")
#define H263_ONLY_PLAY_FILENAME _STRLIT("pv-noaudio_h263-64.3gp")
#define MPEG4_ONLY_PLAY_FILENAME _STRLIT("pv2-noaudio_mpeg4-rvlcs-64.3gp")
#define SQCIF_PLAY_FILENAME _STRLIT("sqcif1.3gp")
#define QVGA_PLAY_FILENAME _STRLIT("qvga.3gp")



#define LOG_FILE_NAME _STRLIT("pvlog.txt")



class engine_test_suite : public test_case
{
    public:
        engine_test_suite();
        PV2WayUnitTestSourceAndSinks* CreateSourceAndSinks(engine_test* test,
                const char* const aAudSrcFormatType,
                const char* const aAudSinkFormatType,
                const char* const aVidSrcFormatType,
                const char* const aVidSinkFormatType);
        bool proxy_tests(const bool aProxy);

        ~engine_test_suite();

    private:
        TestCodecs codecs;
        void AddSetupTests(const bool aProxy, int32 firstTest, int32 lastTest);
        void AddAudioTests(const bool aProxy, int32 firstTest, int32 lastTest);
        void AddVideoTests(const bool aProxy, int32 firstTest, int32 lastTest);
        void AddBasicAVTests(const bool aProxy, int32 firstTest, int32 lastTest);
        void AddAcceptableFormatsTests(const bool aProxy, int32 firstTest, int32 lastTest);
        void AddNegotiatedFormatsTests(const bool aProxy, int32 firstTest, int32 lastTest);

        void play_from_file_tests(const bool aProxy,
                                  const OSCL_wString& aFilename,
                                  const bool aHasAudio,
                                  const bool aHasVideo);
};

class engine_timer;


class engine_timer : public OsclTimerObject
{
    public:
        engine_timer(engine_test *aObserver) : OsclTimerObject(OsclActiveObject::EPriorityNominal, "Test Engine Timer"),
                iObserver(aObserver)
        {};

        ~engine_timer()
        {
            Cancel();
        }

    protected:
        void Run()
        {
            iObserver->TimerCallback();
        }
        void DoCancel()
        {
            OsclTimerObject::DoCancel();
        };

        engine_test *iObserver;

};



#endif


