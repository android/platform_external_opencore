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
#include "test_engine.h"

#include "alloc_dealloc_test.h"
#include "init_test.h"
#include "init_cancel_test.h"
#ifndef NO_2WAY_324
#include "video_only_test.h"
#include "av_test.h"
#include "user_input_test.h"
#include "connect_test.h"
#include "connect_cancel_test.h"
#include "audio_only_test.h"
#include "av_duplicate_test.h"
#include "acceptable_formats_test.h"
#include "pvmf_fileoutput_factory.h"
#endif

#include "oscl_string_utils.h"
#include "oscl_mem_audit.h"
#include "pv_2way_mio.h"
#include "pv_2way_unittest_source_and_sinks.h"


#include "tsc_h324m_config_interface.h"
#include "test_engine_utility.h"
#include "test_codecs.h"

#include "pv_logger_impl.h"

#define AUDIO_FIRST 0
#define VIDEO_FIRST 1

#define MAX_SIP_TEST 27
#define SIP_TEST_OFFSET 200
#define SIP_TEST_MAP(x) (x+SIP_TEST_OFFSET)
#define NUM_SIP_ARGS 10


int start_test();

FILE* fileoutput;
cmd_line *global_cmd_line;

//Find test range args:
//To run a range of tests by enum ID:
//  -test 17 29

char engine_test::iProfileName[32] = "";
uint32 engine_test::iMediaPorts[2] = { 0, 0 };
char engine_test::iPeerAddress[64] = "";



engine_test_suite::engine_test_suite() : test_case()
{
}

engine_test_suite::~engine_test_suite()
{
}



PV2WayUnitTestSourceAndSinks* engine_test_suite::CreateSourceAndSinks(engine_test* test,
        const char* const aAudSrcFormatType,
        const char* const aAudSinkFormatType,
        const char* const aVidSrcFormatType,
        const char* const aVidSinkFormatType)
{
    // this function sets up the source and sinks class given some format types.
    // note- this function adds specific codecs based on the format.
    // a totally different set of codecs could be used.  (using a different codecs input)
    PV2WayUnitTestSourceAndSinks* sourceAndSinks = OSCL_NEW(PV2WayUnitTestSourceAndSinks,
            (test, test->GetSdkInfo()));
    // Audio Source /Outgoing
    if (oscl_strncmp(PVMF_MIME_PCM16, aAudSrcFormatType, oscl_strlen(PVMF_MIME_PCM16)) == 0)
    {
        sourceAndSinks->AddPreferredCodec(OUTGOING, PV_AUDIO, codecs.iAudioSourceRawFileSettings);
    }
    else if (oscl_strncmp(PVMF_MIME_AMR_IF2, aAudSrcFormatType, oscl_strlen(PVMF_MIME_AMR_IF2)) == 0)
    {
        sourceAndSinks->AddPreferredCodec(OUTGOING, PV_AUDIO, codecs.iAudioSourceFileSettings);
    }

    // Audio Sink   /Incoming
    if (oscl_strncmp(PVMF_MIME_PCM16, aAudSinkFormatType, oscl_strlen(PVMF_MIME_PCM16)) == 0)
    {
        sourceAndSinks->AddPreferredCodec(INCOMING, PV_AUDIO, codecs.iAudioSinkRawFileSettings);
    }
    else if (oscl_strncmp(PVMF_MIME_AMR_IF2, aAudSinkFormatType, oscl_strlen(PVMF_MIME_AMR_IF2)) == 0)
    {
        sourceAndSinks->AddPreferredCodec(INCOMING, PV_AUDIO, codecs.iAudioSinkFileSettings);
    }

    // Video Source /Outgoing
    if (oscl_strncmp(PVMF_MIME_YUV420, aVidSrcFormatType, oscl_strlen(PVMF_MIME_YUV420)) == 0)
    {
        sourceAndSinks->AddPreferredCodec(OUTGOING, PV_VIDEO, codecs.iVideoSourceYUVFileSettings);
    }
    else if (oscl_strncmp(PVMF_MIME_H2632000, aVidSrcFormatType, oscl_strlen(PVMF_MIME_H2632000)) == 0)
    {
        sourceAndSinks->AddPreferredCodec(OUTGOING, PV_VIDEO, codecs.iVideoSourceH263FileSettings);
    }
    else if (oscl_strncmp(PVMF_MIME_M4V, aVidSrcFormatType, oscl_strlen(PVMF_MIME_M4V)) == 0)
    {
        sourceAndSinks->AddPreferredCodec(OUTGOING, PV_VIDEO, codecs.iVideoSourceM4VFileSettings);
    }

    // Video Sink   /Incoming
    if (oscl_strncmp(PVMF_MIME_YUV420, aVidSinkFormatType, oscl_strlen(PVMF_MIME_YUV420)) == 0)
    {
        sourceAndSinks->AddPreferredCodec(INCOMING, PV_VIDEO, codecs.iVideoSinkYUVFileSettings);
    }
    else if (oscl_strncmp(PVMF_MIME_H2632000, aVidSinkFormatType, oscl_strlen(PVMF_MIME_H2632000)) == 0)
    {
        sourceAndSinks->AddPreferredCodec(INCOMING, PV_VIDEO, codecs.iVideoSinkH263FileSettings);
    }
    else if (oscl_strncmp(PVMF_MIME_M4V, aVidSinkFormatType, oscl_strlen(PVMF_MIME_M4V)) == 0)
    {
        sourceAndSinks->AddPreferredCodec(INCOMING, PV_VIDEO, codecs.iVideoSinkM4VFileSettings);
    }

    return sourceAndSinks;
}

// 32 to 40
void engine_test_suite::AddAcceptableFormatsTests(const bool aProxy,
        int32 firstTest,
        int32 lastTest)
{
    // when init is done - check values
    if (firstTest <= 32 && lastTest >= 32)
    {
        // in   PCM
        //      YUV
        // out PCM , IF2
        //      YUV
        // expected: PCM , YUV
        test_base* temp = OSCL_NEW(acceptable_formats_test, (aProxy));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_PCM16,
                PVMF_MIME_PCM16, PVMF_MIME_YUV420, PVMF_MIME_YUV420);
        iSourceAndSinks->AddPreferredCodec(OUTGOING, PV_AUDIO, codecs.iAudioSourceFileSettings);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }
}

void engine_test_suite::AddNegotiatedFormatsTests(const bool aProxy,
        int32 firstTest,
        int32 lastTest)
{
}

void engine_test_suite::AddAudioTests(const bool aProxy,
                                      int32 firstTest,
                                      int32 lastTest)
{
    if (firstTest <= 7 && lastTest >= 7)
    {
        test_base* temp = OSCL_NEW(audio_only_test, (aProxy));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_AMR_IF2,
                PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_YUV420);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }
    if (firstTest <= 8 && lastTest >= 8)
    {
        test_base* temp = OSCL_NEW(audio_only_test, (aProxy));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_PCM16,
                PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_YUV420);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }
    if (firstTest <= 9 && lastTest >= 9)
    {
        test_base* temp = OSCL_NEW(audio_only_test, (aProxy));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp,
                PVMF_MIME_AMR_IF2, PVMF_MIME_PCM16, PVMF_MIME_YUV420, PVMF_MIME_YUV420);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }

    if (firstTest <= 10 && lastTest >= 10)
    {
        test_base* temp = OSCL_NEW(audio_only_test, (aProxy));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_PCM16,
                PVMF_MIME_PCM16, PVMF_MIME_YUV420, PVMF_MIME_YUV420);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }
}

void engine_test_suite::AddVideoTests(const bool aProxy,
                                      int32 firstTest,
                                      int32 lastTest)
{
    if (firstTest <= 11 && lastTest >= 11)
    {
        test_base* temp = OSCL_NEW(video_only_test, (aProxy));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_AMR_IF2,
                PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_YUV420);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }

    if (firstTest <= 12 && lastTest >= 12)
    {
        test_base* temp = OSCL_NEW(video_only_test, (aProxy));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_AMR_IF2,
                PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_H2632000);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }
    if (firstTest <= 13 && lastTest >= 13)
    {
        test_base* temp = OSCL_NEW(video_only_test, (aProxy));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_AMR_IF2,
                PVMF_MIME_AMR_IF2, PVMF_MIME_H2632000, PVMF_MIME_YUV420);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }

    if (firstTest <= 14 && lastTest >= 14)
    {
        test_base* temp = OSCL_NEW(video_only_test, (aProxy));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_AMR_IF2,
                PVMF_MIME_AMR_IF2, PVMF_MIME_M4V, PVMF_MIME_YUV420);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }
    if (firstTest <= 15 && lastTest >= 15)
    {
        test_base* temp = OSCL_NEW(video_only_test, (aProxy));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_AMR_IF2,
                PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_M4V);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }

}

void engine_test_suite::AddBasicAVTests(const bool aProxy,
                                        int32 firstTest,
                                        int32 lastTest)
{
    if (firstTest <= 16 && lastTest >= 16)
    {
        test_base* temp = OSCL_NEW(av_test, (aProxy));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_AMR_IF2,
                PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_YUV420);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }

    if (firstTest <= 17 && lastTest >= 17)
    {
        test_base* temp = OSCL_NEW(av_test, (aProxy));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_PCM16,
                PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_YUV420);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }

    if (firstTest <= 18 && lastTest >= 18)
    {
        test_base* temp = OSCL_NEW(av_test, (aProxy));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_PCM16,
                PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_YUV420);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }

    if (firstTest <= 19 && lastTest >= 19)
    {
        test_base* temp = OSCL_NEW(av_test, (aProxy));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_PCM16,
                PVMF_MIME_PCM16, PVMF_MIME_YUV420, PVMF_MIME_YUV420);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }

    if (firstTest <= 20 && lastTest >= 20)
    {
        test_base* temp = OSCL_NEW(av_test, (aProxy));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_AMR_IF2,
                PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_H2632000);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }
    if (firstTest <= 21 && lastTest >= 21)
    {
        test_base* temp = OSCL_NEW(av_test, (aProxy));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_AMR_IF2,
                PVMF_MIME_AMR_IF2, PVMF_MIME_H2632000, PVMF_MIME_YUV420);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }
    if (firstTest <= 22 && lastTest >= 22)
    {
        test_base* temp = OSCL_NEW(av_test, (aProxy));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_AMR_IF2,
                PVMF_MIME_AMR_IF2, PVMF_MIME_H2632000, PVMF_MIME_H2632000);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }
    if (firstTest <= 23 && lastTest >= 23)
    {
        test_base* temp = OSCL_NEW(av_test, (aProxy));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_AMR_IF2,
                PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_YUV420);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }


    if (firstTest <= 24 && lastTest >= 24)
    {
        test_base* temp = OSCL_NEW(av_test, (aProxy));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_PCM16,
                PVMF_MIME_AMR_IF2, PVMF_MIME_M4V, PVMF_MIME_YUV420);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }
    if (firstTest <= 25 && lastTest >= 25)
    {
        test_base* temp = OSCL_NEW(av_test, (aProxy));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_AMR_IF2,
                PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_M4V);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }

}

void engine_test_suite::AddSetupTests(const bool aProxy,
                                      int32 firstTest,
                                      int32 lastTest)
{
    if (firstTest == 0)
    {
        // alloc_dealloc_test
        test_base* temp = OSCL_NEW(alloc_dealloc_test, (aProxy));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_AMR_IF2,
                PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_YUV420);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }
    if (firstTest <= 2 && lastTest >= 2)
    {
        // init_test
        test_base* temp = OSCL_NEW(init_test, (aProxy, 1));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_AMR_IF2,
                PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_YUV420);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }

    if (firstTest <= 3 && lastTest >= 3)
    {
        // init_test
        test_base* temp = OSCL_NEW(init_test, (aProxy, 2));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_AMR_IF2,
                PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_YUV420);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }

    if (firstTest <= 4 && lastTest >= 4)
    {
        // init_cancel_test
        test_base* temp = OSCL_NEW(init_cancel_test, (aProxy));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_AMR_IF2,
                PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_YUV420);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }

    if (firstTest <= 5 && lastTest >= 5)
    {
        // connect_test
        test_base* temp = OSCL_NEW(connect_test, (aProxy, 1));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_AMR_IF2,
                PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_YUV420);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }

    if (firstTest <= 26 && lastTest >= 26)
    {
        // connect_test
        test_base* temp = OSCL_NEW(connect_test, (aProxy, 1, true));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_AMR_IF2,
                PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_YUV420);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }

    if (firstTest <= 6 && lastTest >= 6)
    {
        // connect_cancel_test
        test_base* temp = OSCL_NEW(connect_cancel_test, (aProxy));
        PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_AMR_IF2,
                PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_YUV420);
        temp->AddSourceAndSinks(iSourceAndSinks);
        adopt_test_case(temp);
    }
}

bool engine_test_suite::proxy_tests(const bool aProxy)
{

    int32 firstTest = 0;
    int32 lastTest = MAX_324_TEST;
    bool alltests = false;


    FindTestRange(global_cmd_line, firstTest, lastTest, fileoutput);

    //Basic 2way tests
    fprintf(fileoutput, "Basic engine tests.  First: %d Last: %d\n", firstTest, lastTest);
    if (firstTest == 0 && lastTest == MAX_324_TEST)
        alltests = true;

    if (!codecs.setvalues())
    {
        fprintf(fileoutput, "ERROR! Could not locate all input files.\n");
        return false;
    }
#ifndef NO_2WAY_324
    // if we loop here we'll run out of memory
    {
        fprintf(fileoutput, "Add Setup tests\n");
        AddSetupTests(aProxy, firstTest, lastTest);
        fprintf(fileoutput, "Add Audio tests\n");

        AddAudioTests(aProxy, firstTest, lastTest);

        ///////////////////////////////////////////////////////////////////////////////
        //          VIDEO TESTS
        fprintf(fileoutput, "Add Video tests\n");
        AddVideoTests(aProxy, firstTest, lastTest);

        ///////////////////////////////////////////////////////////////////////////////
        //          AUDIO-VIDEO TESTS
        fprintf(fileoutput, "Add AV tests\n");
        AddBasicAVTests(aProxy, firstTest, lastTest);

        ///////////////////////////////////////////////////////////////////////////////
        //          AUDIO-VIDEO TESTS: MULTIPLE INPUTS

        fprintf(fileoutput, "Add test 28\n");
        if (firstTest <= 28 && lastTest >= 28)
        {
            // in AMR_IF2, PCM
            //      YUV
            // out AMR_IF2
            //      YUV
            test_base* temp = OSCL_NEW(av_test, (aProxy));
            PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_AMR_IF2,
                    PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_YUV420);
            iSourceAndSinks->AddPreferredCodec(INCOMING, PV_AUDIO, codecs.iAudioSinkRawFileSettings);
            temp->AddSourceAndSinks(iSourceAndSinks);
            adopt_test_case(temp);

        }

        fprintf(fileoutput, "Add test 29\n");
        if (firstTest <= 29 && lastTest >= 29)
        {
            // in AMR_IF2, PCM
            //      YUV
            // out PCM
            //      YUV
            // expected: IF2, YUV
            test_base* temp = OSCL_NEW(av_test, (aProxy));
            PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_PCM16,
                    PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_YUV420);
            iSourceAndSinks->AddPreferredCodec(INCOMING, PV_AUDIO, codecs.iAudioSinkRawFileSettings);
            temp->AddSourceAndSinks(iSourceAndSinks);
            adopt_test_case(temp);
        }

        fprintf(fileoutput, "Add test 30\n");
        // result is PCM, YUV420, PCM, YUV420
        if (firstTest <= 30 && lastTest >= 30)
        {
            // in   PCM
            //      YUV, H263
            // out PCM
            //      YUV
            // expected: PCM, H263
            test_base* temp = OSCL_NEW(av_test, (aProxy));
            PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_PCM16,
                    PVMF_MIME_PCM16, PVMF_MIME_YUV420, PVMF_MIME_YUV420);
            iSourceAndSinks->AddPreferredCodec(INCOMING, PV_VIDEO, codecs.iVideoSinkH263FileSettings);
            temp->AddSourceAndSinks(iSourceAndSinks);
            adopt_test_case(temp);
        }
        fprintf(fileoutput, "Add test 31\n");
        if (firstTest <= 31 && lastTest >= 31)
        {
            // in   PCM
            //      YUV
            // out PCM , IF2
            //      YUV
            // expected: PCM , YUV
            test_base* temp = OSCL_NEW(av_test, (aProxy));
            PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_PCM16,
                    PVMF_MIME_PCM16, PVMF_MIME_YUV420, PVMF_MIME_YUV420);
            iSourceAndSinks->AddPreferredCodec(OUTGOING, PV_AUDIO, codecs.iAudioSourceFileSettings);
            temp->AddSourceAndSinks(iSourceAndSinks);
            adopt_test_case(temp);
        }
        // nope
        fprintf(fileoutput, "Add test 32\n");
        if (firstTest <= 32 && lastTest >= 32)
        {
            // in   IF2
            //      YUV
            // out PCM , IF2
            //      YUV
            // expected: PCM, YUV
            test_base* temp = OSCL_NEW(av_test, (aProxy));
            PV2WayUnitTestSourceAndSinks* iSourceAndSinks = CreateSourceAndSinks(temp, PVMF_MIME_PCM16,
                    PVMF_MIME_AMR_IF2, PVMF_MIME_YUV420, PVMF_MIME_YUV420);
            iSourceAndSinks->AddPreferredCodec(OUTGOING, PV_AUDIO, codecs.iAudioSourceFileSettings);
            temp->AddSourceAndSinks(iSourceAndSinks);
            adopt_test_case(temp);
        }

        //4. All compressed : AMR-IF2,H.263,MPEG-4
        //5. All compressed (invalid test case): AMR-IF2,MPEG-4
        //6. Combination: PCM16,AMR-IF2,YUV420,MPEG-4
        //7. Combination: PCM8,PCM16,AMR-IF2,YUV420,H.263,MPEG-4

        // 32 to 40
        //AddAcceptableFormatsTests(aProxy, firstTest, lastTest);
        //AddNegotiatedFormatsTests(aProxy, firstTest, lastTest);

    }
#endif
    return true;
}


int test_wrapper()
{
    int result;

    OsclErrorTrap::Init();
    OsclScheduler::Init("PV2WayEngineFactory");

    result = start_test();

    OsclScheduler::Cleanup();
    OsclErrorTrap::Cleanup();

    return result;
}


int local_main(FILE* filehandle, cmd_line *command_line)
{
    OSCL_UNUSED_ARG(command_line);
    int result;
    global_cmd_line = command_line;

    fileoutput = filehandle;
    fprintf(fileoutput, "Test Program for PV Engine class.\n");

    CPV2WayEngineFactory::Init();
#ifndef OSCL_BYPASS_MEMMGT
#ifndef NDEBUG
#ifdef MEM_AUDIT_2WAY
    OsclAuditCB auditCB;
    OsclMemInit(auditCB);
    auditCB.pAudit->MM_SetMode(auditCB.pAudit->MM_GetMode() |
                               MM_AUDIT_VALIDATE_ON_FREE_FLAG | MM_AUDIT_ALLOC_NODE_ENABLE_FLAG);
#endif
#endif
#endif
    result = test_wrapper();

    PVLogger::Cleanup();
#ifndef OSCL_BYPASS_MEMMGT
#ifndef NDEBUG
#ifdef MEM_AUDIT_2WAY
    //Check for memory leaks before cleaning up OsclMem.

    uint32 leaks = 0;

    if (auditCB.pAudit)
    {
        MM_Stats_t* stats = auditCB.pAudit->MM_GetStats("");
        if (stats)
        {
            fprintf(fileoutput, "Memory Stats:\n");
            fprintf(fileoutput, "  peakNumAllocs %d\n", stats->peakNumAllocs);
            fprintf(fileoutput, "  peakNumBytes %d\n", stats->peakNumBytes);
            fprintf(fileoutput, "  numAllocFails %d\n", stats->numAllocFails);
            if (stats->numAllocs)
            {
                fprintf(fileoutput, "  ERROR: Memory Leaks! numAllocs %d, numBytes %d\n", stats->numAllocs, stats->numBytes);
            }
        }
        leaks = auditCB.pAudit->MM_GetNumAllocNodes();
        if (leaks != 0)
        {
            fprintf(fileoutput, "ERROR: %d Memory leaks detected!\n", leaks);
            MM_AllocQueryInfo*info = auditCB.pAudit->MM_CreateAllocNodeInfo(leaks);
            uint32 leakinfo = auditCB.pAudit->MM_GetAllocNodeInfo(info, leaks, 0);
            if (leakinfo != leaks)
            {
                fprintf(fileoutput, "ERROR: Leak info is incomplete.\n");
            }
            for (uint32 i = 0; i < leakinfo; i++)
            {
                fprintf(fileoutput, "Leak Info:\n");
                fprintf(fileoutput, "  allocNum %d\n", info[i].allocNum);
                fprintf(fileoutput, "  fileName %s\n", info[i].fileName);
                fprintf(fileoutput, "  lineNo %d\n", info[i].lineNo);
                fprintf(fileoutput, "  size %d\n", info[i].size);
                fprintf(fileoutput, "  pMemBlock 0x%x\n", info[i].pMemBlock);
                fprintf(fileoutput, "  tag %s\n", info[i].tag);
            }
            auditCB.pAudit->MM_ReleaseAllocNodeInfo(info);
        }
    }
#endif
#endif
#endif
    CPV2WayEngineFactory::Cleanup();

    return (result);
}


int start_test()
{
    int32 leave = 0;
    bool result = 0;
    // looping for testing
    uint32 ii = 0;
    //for (ii = 0; ii < 1000; ++ii)
    {
        engine_test_suite engine_tests;
        fprintf(fileoutput, "####  TEST ROUND NUMBER: %d ### \n", ii);
        // setting iProxy
        if (!engine_tests.proxy_tests(true))
        {
            fprintf(fileoutput, "ERROR - unable to setup tests\n");
        }

        OSCL_TRY(leave, engine_tests.run_test());

        if (leave != 0)
            fprintf(fileoutput, "Leave %d\n", leave);

        text_test_interpreter interp;
        _STRING rs = interp.interpretation(engine_tests.last_result());
        fprintf(fileoutput, rs.c_str());
        const test_result the_result = engine_tests.last_result();
        result = (the_result.success_count() != the_result.total_test_count());
    }
    return result;
}

#if (LINUX_MAIN==1)

int main(int argc, char *argv[])
{
    local_main(stdout, NULL);
    return 0;
}

#endif

