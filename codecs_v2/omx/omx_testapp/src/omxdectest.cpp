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
#include "omxdectest.h"
#include "omxdectest_basic_test.h"

#ifndef OSCL_STRING_UTILS_H_INCLUDED
#include "oscl_string_utils.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef __UNIT_TEST_TEST_ARGS__
#include "unit_test_args.h"
#endif

#ifndef PVLOGGER_STDERR_APPENDER_H_INCLUDED
#include "pvlogger_stderr_appender.h"
#endif

#ifndef PVLOGGER_FILE_APPENDER_H_INCLUDED
#include "pvlogger_file_appender.h"
#endif

#ifndef PVLOGGER_TIME_AND_ID_LAYOUT_H_INCLUDED
#include "pvlogger_time_and_id_layout.h"
#endif

#ifndef OSCL_UTF8CONV_H
#include "oscl_utf8conv.h"
#endif

#define PV_ARGSTR_LENGTH 128

#define PV_OMX_MAX_COMPONENT_NAME_LEN 128
#define FRAME_SIZE_FIELD  4

#define SYNC_WORD_LENGTH_MP3   11
#define INBUF_ARRAY_INDEX_SHIFT  (3)
#define INBUF_BIT_WIDTH         (1<<(INBUF_ARRAY_INDEX_SHIFT))
#define SYNC_WORD         (OMX_S32)0x7ff

#define NUMBER_TEST_CASES  23

#define module(x, POW2)   (x&(POW2-1))

/* MPEG Header Definitions - ID Bit Values */

#define MPEG_1              0
#define MPEG_2              1
#define MPEG_2_5            2
#define INVALID_VERSION     -1

OMX_BOOL SilenceInsertionEnable = OMX_FALSE;

#define AAC_MONO_SILENCE_FRAME_SIZE 10
#define AAC_STEREO_SILENCE_FRAME_SIZE 11

static const OMX_U8 AAC_MONO_SILENCE_FRAME[]   = {0x01, 0x40, 0x20, 0x06, 0x4F, 0xDE, 0x02, 0x70, 0x0C, 0x1C};      // 10 bytes
static const OMX_U8 AAC_STEREO_SILENCE_FRAME[] = {0x21, 0x10, 0x05, 0x00, 0xA0, 0x19, 0x33, 0x87, 0xC0, 0x00, 0x7E}; // 11 bytes)


// The string to prepend to source filenames
#define SOURCENAME_PREPEND_STRING ""
#define SOURCENAME_PREPEND_WSTRING _STRLIT_WCHAR("")

// The string to prepend to output filenames
#define OUTPUTNAME_PREPEND_STRING " "
#define OUTPUTNAME_PREPEND_WSTRING _STRLIT_WCHAR("")

template<class DestructClass>
class LogAppenderDestructDealloc : public OsclDestructDealloc
{
    public:
        virtual void destruct_and_dealloc(OsclAny *ptr)
        {
            delete((DestructClass*)ptr);
        }
};

void getCmdLineArg(bool cmdline_iswchar, cmd_line* command_line, char* argstr, int arg_index, int num_elements)
{
    if (cmdline_iswchar)
    {
        oscl_wchar* argwstr = NULL;
        command_line->get_arg(arg_index, argwstr);
        oscl_UnicodeToUTF8(argwstr, oscl_strlen(argwstr), argstr, 128);
        argstr[127] = '\0';
    }
    else
    {
        char* tmpstr = NULL;
        command_line->get_arg(arg_index, tmpstr);
        int32 tmpstrlen = oscl_strlen(tmpstr) + 1;
        if (tmpstrlen > 128)
        {
            tmpstrlen = 128;
        }
        oscl_strncpy(argstr, tmpstr, tmpstrlen);
        argstr[tmpstrlen-1] = '\0';
    }
}

void usage(FILE* filehandle)
{
    fprintf(filehandle, "Usage: Input_file {options}\n");
    fprintf(filehandle, "Option{} \n");
    fprintf(filehandle, "-output {console out}: Output file for console messages - Default is Stderr\n");
    fprintf(filehandle, "-o      {fileout}: Output file to be generated \n");
    fprintf(filehandle, "-i      {input_file2}: 2nd input file for dynamic reconfig, etc.\n");
    fprintf(filehandle, "-r      {ref_file}:   Reference file for output verification\n");
    fprintf(filehandle, "-c/-n   {CodecType/ComponentName}\n");
    fprintf(filehandle, "        CodecType could be avc, aac, mpeg4, h263, wmv, amr, mp3, wma\n");
    fprintf(filehandle, "-t x y  {A range of test cases to run}\n ");
    fprintf(filehandle, "        To run one test case use same index for x and y - Default is ALL\n");
    fprintf(filehandle, "-l      log to file\n");
}


// local_main entry point for the code
int local_main(FILE* filehandle, cmd_line* command_line)
{
    OSCL_UNUSED_ARG(filehandle);

    FILE* pInputFile = NULL;
    FILE* pOutputFile = NULL;

    char InFileName[200] = "\0";
    char InFileName2[200] = "\0";
    char OutFileName[200] = "\0";
    char pRefFileName[200] = "\0";
    char ComponentFormat[10] = "\0";

    OMX_STRING ComponentName = NULL, Role = NULL;
    OMX_S32 ArgIndex = 0, FirstTest, LastTest;
    OMX_U32 Channels = 2;
    OMX_BOOL InitSchedulerFlag = OMX_FALSE;

    //File format and band mode for Without marker test case, to be specified by user as ip argument
    OMX_AUDIO_AMRFRAMEFORMATTYPE AmrInputFileType;
    OMX_AUDIO_AMRBANDMODETYPE AMRFileBandMode;

    OMX_BOOL AmrFormatFlag = OMX_FALSE;
    OMX_BOOL AmrModeFlag = OMX_FALSE;

    //Whether log commands should go in a file
    OMX_BOOL IsLogFile = OMX_FALSE;

    // OSCL Initializations
    OsclBase::Init();
    OsclErrorTrap::Init();

    OsclMutex mem_lock_mutex; //create mutex to use as mem lock
    mem_lock_mutex.Create();

    OsclMem::Init();
    PVLogger::Init();

    bool cmdline_iswchar = command_line->is_wchar();

    int32 count = command_line->get_count();

    char argstr[128];

    if (count > 1)
    {
        ArgIndex = 0;
        getCmdLineArg(cmdline_iswchar, command_line, argstr, ArgIndex, PV_ARGSTR_LENGTH);

        oscl_strncpy(InFileName, argstr, oscl_strlen(argstr) + 1);
        pInputFile = fopen(InFileName, "rb");

        //default is to run all tests.
        FirstTest = 0;
        LastTest = NUMBER_TEST_CASES;

        ArgIndex = 1;

        while (ArgIndex < count)
        {
            getCmdLineArg(cmdline_iswchar, command_line, argstr, ArgIndex, PV_ARGSTR_LENGTH);

            if ('-' == *(argstr))
            {
                switch (argstr[1])
                {
                    case 'o':
                    {
                        ArgIndex++;
                        getCmdLineArg(cmdline_iswchar, command_line, argstr, ArgIndex, PV_ARGSTR_LENGTH);
                        oscl_strncpy(OutFileName, argstr, oscl_strlen(argstr) + 1);
                        ArgIndex++;
                        pOutputFile = fopen(OutFileName, "wb");
                    }
                    break;

                    case 'i':
                    {
                        ArgIndex++;
                        getCmdLineArg(cmdline_iswchar, command_line, argstr, ArgIndex, PV_ARGSTR_LENGTH);
                        oscl_strncpy(InFileName2, argstr, oscl_strlen(argstr) + 1);
                        ArgIndex++;
                    }
                    break;

                    case 'r':
                    {
                        ArgIndex++;
                        getCmdLineArg(cmdline_iswchar, command_line, argstr, ArgIndex, PV_ARGSTR_LENGTH);
                        oscl_strncpy(pRefFileName, argstr, oscl_strlen(argstr) + 1);
                        ArgIndex++;
                    }
                    break;

                    case 't':
                    {
                        ArgIndex++;
                        getCmdLineArg(cmdline_iswchar, command_line, argstr, ArgIndex, PV_ARGSTR_LENGTH);
                        PV_atoi(argstr, 'd', (uint32&)FirstTest);
                        ArgIndex++;
                        getCmdLineArg(cmdline_iswchar, command_line, argstr, ArgIndex, PV_ARGSTR_LENGTH);
                        PV_atoi(argstr, 'd', (uint32&)LastTest);
                        ArgIndex++;
                    }
                    break;

                    //Below two input arguments are required in the WihtoutMarker bit test case
                    case 'f':
                    {
                        ArgIndex++;
                        getCmdLineArg(cmdline_iswchar, command_line, argstr, ArgIndex, PV_ARGSTR_LENGTH);
                        if (0 == oscl_strcmp("rtp", argstr))
                        {
                            AmrInputFileType = OMX_AUDIO_AMRFrameFormatRTPPayload;
                        }
                        else if (0 == oscl_strcmp("fsf", argstr))
                        {
                            AmrInputFileType = OMX_AUDIO_AMRFrameFormatFSF;
                        }
                        else if (0 == oscl_strcmp("if2", argstr))
                        {
                            AmrInputFileType = OMX_AUDIO_AMRFrameFormatIF2;
                        }
                        else if (0 == oscl_strcmp("ets", argstr))
                        {
                            AmrInputFileType = OMX_AUDIO_AMRFrameFormatConformance;
                        }
                        else
                        {
                            /* Invalid input format type */
                            fprintf(filehandle, "Invalid AMR input format specified!\n");
                            return 1;
                        }

                        AmrFormatFlag = OMX_TRUE;
                        ArgIndex++;
                    }
                    break;

                    case 'b':
                    {
                        ArgIndex++;
                        getCmdLineArg(cmdline_iswchar, command_line, argstr, ArgIndex, PV_ARGSTR_LENGTH);
                        if (0 == oscl_strcmp("nb", argstr))
                        {
                            //Narrow band Mode
                            AMRFileBandMode = OMX_AUDIO_AMRBandModeNB0;
                        }
                        else if (0 == oscl_strcmp("wb", argstr))
                        {
                            //Wide band Mode
                            AMRFileBandMode = OMX_AUDIO_AMRBandModeWB0;
                        }
                        else
                        {
                            /* Invalid input format type */
                            fprintf(filehandle, "Invalid AMR band mode specified!\n");
                            return 1;
                        }
                        AmrModeFlag = OMX_TRUE;
                        ArgIndex++;
                    }
                    break;

                    case 'n':
                    case 'c':
                    {
                        char* Result;
                        ArgIndex++;
                        getCmdLineArg(cmdline_iswchar, command_line, argstr, ArgIndex, PV_ARGSTR_LENGTH);
                        if ('n' == argstr[1])
                        {
                            ComponentName = (OMX_STRING) oscl_malloc(PV_OMX_MAX_COMPONENT_NAME_LENGTH * sizeof(OMX_U8));
                            getCmdLineArg(cmdline_iswchar, command_line, argstr, ArgIndex, PV_ARGSTR_LENGTH);
                            oscl_strncpy(ComponentName, argstr, oscl_strlen(argstr) + 1);
                        }

                        Role = (OMX_STRING) oscl_malloc(PV_OMX_MAX_COMPONENT_NAME_LENGTH * sizeof(OMX_U8));

                        getCmdLineArg(cmdline_iswchar, command_line, argstr, ArgIndex, PV_ARGSTR_LENGTH);
                        if (NULL != (Result = (OMX_STRING) oscl_strstr(argstr, "avc")))
                        {
                            oscl_strncpy(Role, "video_decoder.avc", oscl_strlen("video_decoder.avc") + 1);
                            oscl_strncpy(ComponentFormat, "H264", oscl_strlen("H264") + 1);
                        }
                        else if (NULL != (Result = (OMX_STRING) oscl_strstr(argstr, "mpeg4")))
                        {
                            oscl_strncpy(Role, "video_decoder.mpeg4", oscl_strlen("video_decoder.mpeg4") + 1);
                            oscl_strncpy(ComponentFormat, "M4V", oscl_strlen("M4V") + 1);
                        }
                        else if (NULL != (Result = (OMX_STRING) oscl_strstr(argstr, "h263")))
                        {
                            oscl_strncpy(Role, "video_decoder.h263", oscl_strlen("video_decoder.h263") + 1);
                            oscl_strncpy(ComponentFormat, "H263", oscl_strlen("H263") + 1);
                        }
                        else if (NULL != (Result = (OMX_STRING) oscl_strstr(argstr, "aac")))
                        {
                            oscl_strncpy(Role, "audio_decoder.aac", oscl_strlen("video_decoder.aac") + 1);
                            oscl_strncpy(ComponentFormat, "AAC", oscl_strlen("AAC") + 1);
                        }
                        else if (NULL != (Result = (OMX_STRING) oscl_strstr(argstr, "wmv")))
                        {
                            oscl_strncpy(Role, "video_decoder.wmv", oscl_strlen("video_decoder.wmv") + 1);
                            oscl_strncpy(ComponentFormat, "WMV", oscl_strlen("WMV") + 1);
                        }
                        else if (NULL != (Result = (OMX_STRING) oscl_strstr(argstr, "amr")))
                        {
                            oscl_strncpy(Role, "audio_decoder.amr", oscl_strlen("audio_decoder.amr") + 1);
                            oscl_strncpy(ComponentFormat, "AMR", oscl_strlen("AMR") + 1);
                        }
                        else if (NULL != (Result = (OMX_STRING) oscl_strstr(argstr, "mp3")))
                        {
                            oscl_strncpy(Role, "audio_decoder.mp3", oscl_strlen("audio_decoder.mp3") + 1);
                            oscl_strncpy(ComponentFormat, "MP3", oscl_strlen("MP3") + 1);
                        }
                        else if (NULL != (Result = (OMX_STRING) oscl_strstr(argstr, "wma")))
                        {
                            oscl_strncpy(Role, "audio_decoder.wma", oscl_strlen("audio_decoder.wma") + 1);
                            oscl_strncpy(ComponentFormat, "WMA", oscl_strlen("WMA") + 1);
                        }
                        else
                        {
                            fprintf(filehandle, "Unsupported component type\n");
                            return 1;
                        }

                        ArgIndex++;
                    }
                    break;

                    //Same case body
                    case 'm':
                    case 'M':
                    {
                        ArgIndex++;
                        Channels = 1;
                    }
                    break;

                    case 'l':
                    {
                        ArgIndex++;
                        IsLogFile = OMX_TRUE;
                    }
                    break;

                    default:
                    {
                        usage(filehandle);

                        // Clean OSCL
                        PVLogger::Cleanup();
                        OsclErrorTrap::Cleanup();
                        OsclMem::Cleanup();
                        OsclBase::Cleanup();

                        return -1;
                    }
                    break;
                }
            }
            else
            {
                usage(filehandle);

                // Clean OSCL
                PVLogger::Cleanup();
                OsclErrorTrap::Cleanup();
                OsclMem::Cleanup();
                OsclBase::Cleanup();
                mem_lock_mutex.Close();
                return -1;
            }
        }
    }
    else
    {
        fprintf(filehandle, "\nArguments insufficient\n\n");
        usage(filehandle);

        return -1;
    }


    //Verify both input & output files are specified
    if ((NULL == pInputFile) || (NULL == pOutputFile))
    {
        fprintf(filehandle, "One of the input/output file missing/corrupted, exit from here \n");

        if (ComponentName)
        {
            oscl_free(ComponentName);
        }

        if (Role)
        {
            oscl_free(Role);
        }

        // Clean OSCL
        PVLogger::Cleanup();
        OsclErrorTrap::Cleanup();
        OsclMem::Cleanup();
        OsclBase::Cleanup();
        mem_lock_mutex.Close();
        return -1;
    }

    //If no component specified, return from here
    if ((NULL == ComponentName) && (NULL == Role))
    {
        fprintf(filehandle, "User didn't specify any of the component to instantiate, Exit from here \n");
        // Clean OSCL
        PVLogger::Cleanup();
        OsclErrorTrap::Cleanup();
        OsclMem::Cleanup();
        OsclBase::Cleanup();
        mem_lock_mutex.Close();
        return -1;
    }

    //This scope operator will make sure that LogAppenderDestructDealloc is called before the Logger cleanup

    {
        //Enable the following code for logging
        PVLoggerAppender *appender = NULL;
        OsclRefCounter *refCounter = NULL;

        if (IsLogFile)
        {
            OSCL_wHeapString<OsclMemAllocator> LogFileName(OUTPUTNAME_PREPEND_WSTRING);
            LogFileName += _STRLIT_WCHAR("logfile.txt");
            appender = (PVLoggerAppender*)TextFileAppender<TimeAndIdLayout, 1024>::CreateAppender(LogFileName.get_str());
            OsclRefCounterSA<LogAppenderDestructDealloc<TextFileAppender<TimeAndIdLayout, 1024> > > *appenderRefCounter =
                new OsclRefCounterSA<LogAppenderDestructDealloc<TextFileAppender<TimeAndIdLayout, 1024> > >(appender);
            refCounter = appenderRefCounter;
        }
        else
        {
            appender = new StdErrAppender<TimeAndIdLayout, 1024>();
            OsclRefCounterSA<LogAppenderDestructDealloc<StdErrAppender<TimeAndIdLayout, 1024> > > *appenderRefCounter =
                new OsclRefCounterSA<LogAppenderDestructDealloc<StdErrAppender<TimeAndIdLayout, 1024> > >(appender);
            refCounter = appenderRefCounter;

        }

        OsclSharedPtr<PVLoggerAppender> appenderPtr(appender, refCounter);

        //Log all the loggers
        PVLogger *rootnode = PVLogger::GetLoggerObject("");
        rootnode->AddAppender(appenderPtr);
        rootnode->SetLogLevel(PVLOGMSG_DEBUG);


        //Run the tests
        OMX_S32 CurrentTestNumber = FirstTest;
        OmxComponentDecTest* pCurrentTest = NULL;

        while (CurrentTestNumber <= LastTest)
        {
            // Shutdown PVLogger and scheduler before checking mem stats

            switch (CurrentTestNumber)
            {
                case GET_ROLES_TEST:
                {
                    fprintf(filehandle, "\nStarting test %4d: GET_ROLES_TEST \n", CurrentTestNumber);

                    pCurrentTest =  OSCL_NEW(OmxDecTestCompRole, (filehandle, pInputFile, pOutputFile,
                                             OutFileName, pRefFileName, ComponentName, Role,
                                             ComponentFormat, Channels));

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pCurrentTest->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pCurrentTest->StartTestApp();
                    OSCL_DELETE(pCurrentTest);

                    pCurrentTest = NULL;
                    CurrentTestNumber++;

                    fclose(pInputFile);
                    pInputFile = NULL;
                    fclose(pOutputFile);
                    pOutputFile = NULL;
                }
                break;

                case BUFFER_NEGOTIATION_TEST:
                {
                    fprintf(filehandle, "\nStarting test %4d: BUFFER_NEGOTIATION_TEST \n", CurrentTestNumber);

                    pCurrentTest = OSCL_NEW(OmxDecTestBufferNegotiation, (filehandle, pInputFile, pOutputFile,
                                            OutFileName, pRefFileName, ComponentName, Role,
                                            ComponentFormat, Channels));

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pCurrentTest->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pCurrentTest->StartTestApp();

                    OSCL_DELETE(pCurrentTest);

                    pCurrentTest = NULL;
                    CurrentTestNumber++;

                }
                break;

                case DYNAMIC_PORT_RECONFIG:
                {
                    OMX_BOOL iCallbackFlag1, iCallbackFlag2;

                    /* Run the testcase for FILE 1 */
                    fprintf(filehandle, "\nStarting test %4d: DYNAMIC_PORT_RECONFIG for %s\n", CurrentTestNumber, InFileName);

                    pInputFile = fopen(InFileName, "rb");
                    pOutputFile = fopen(OutFileName, "wb");

                    pCurrentTest = OSCL_NEW(OmxDecTestPortReconfig, (filehandle, pInputFile, pOutputFile,
                                            OutFileName, pRefFileName, ComponentName, Role,
                                            ComponentFormat, Channels));

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pCurrentTest->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pCurrentTest->StartTestApp();

                    iCallbackFlag1 = pCurrentTest->iPortSettingsFlag;

                    OSCL_DELETE(pCurrentTest);
                    pCurrentTest = NULL;

                    fclose(pInputFile);
                    pInputFile = NULL;
                    fclose(pOutputFile);
                    pOutputFile = NULL;


                    /* Run the test case for FILE 2 */
                    if (0 != oscl_strcmp(InFileName2, "\0"))
                    {
                        pInputFile = fopen(InFileName2, "rb");
                    }

                    if (NULL == pInputFile)
                    {
                        fprintf(filehandle, "Cannot run this test for second input bitstream File Open Error\n");
                        fprintf(filehandle, "DYNAMIC_PORT_RECONFIG Fail\n");
                        CurrentTestNumber++;
                        break;
                    }

                    pOutputFile = fopen(OutFileName, "wb");
                    fprintf(filehandle, "\nStarting test %4d: DYNAMIC_PORT_RECONFIG for %s\n", CurrentTestNumber, InFileName2);

                    pCurrentTest = OSCL_NEW(OmxDecTestPortReconfig, (filehandle, pInputFile, pOutputFile,
                                            OutFileName, pRefFileName, ComponentName, Role,
                                            ComponentFormat, Channels));

                    pCurrentTest->StartTestApp();

                    iCallbackFlag2 = pCurrentTest->iPortSettingsFlag;
                    OSCL_DELETE(pCurrentTest);
                    pCurrentTest = NULL;

                    fclose(pInputFile);
                    pInputFile = NULL;

                    fclose(pOutputFile);
                    pOutputFile = NULL;


                    /*Verify the test case */
                    if ((OMX_TRUE == iCallbackFlag1) || (OMX_TRUE == iCallbackFlag2))
                    {
                        fprintf(filehandle, "\nDYNAMIC_PORT_RECONFIG Success\n");
                    }
                    else
                    {
                        fprintf(filehandle, "\n No Port Settings Changed Callback arrived for either of the input bit-streams\n");
                        fprintf(filehandle, "DYNAMIC_PORT_RECONFIG Fail\n");
                    }

                    CurrentTestNumber++;
                }
                break;

                case PORT_RECONFIG_TRANSITION_TEST:
                {
                    fprintf(filehandle, "\nStarting test %4d: PORT_RECONFIG_TRANSITION_TEST\n", CurrentTestNumber);

                    pInputFile = fopen(InFileName, "rb");
                    pOutputFile = fopen(OutFileName, "wb");

                    pCurrentTest = OSCL_NEW(OmxDecTestPortReconfigTransitionTest, (filehandle, pInputFile,
                                            pOutputFile, OutFileName, pRefFileName, ComponentName,
                                            Role, ComponentFormat, Channels));

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pCurrentTest->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pCurrentTest->StartTestApp();

                    OSCL_DELETE(pCurrentTest);
                    pCurrentTest = NULL;

                    fclose(pInputFile);
                    pInputFile = NULL;

                    fclose(pOutputFile);
                    pOutputFile = NULL;

                    CurrentTestNumber++;
                }
                break;

                case PORT_RECONFIG_TRANSITION_TEST_2:
                {
                    fprintf(filehandle, "\nStarting test %4d: PORT_RECONFIG_TRANSITION_TEST_2\n", CurrentTestNumber);

                    pInputFile = fopen(InFileName, "rb");
                    pOutputFile = fopen(OutFileName, "wb");

                    pCurrentTest = OSCL_NEW(OmxDecTestPortReconfigTransitionTest_2, (filehandle, pInputFile,
                                            pOutputFile, OutFileName, pRefFileName, ComponentName,
                                            Role, ComponentFormat, Channels));

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pCurrentTest->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pCurrentTest->StartTestApp();

                    OSCL_DELETE(pCurrentTest);
                    pCurrentTest = NULL;

                    fclose(pInputFile);
                    pInputFile = NULL;

                    fclose(pOutputFile);
                    pOutputFile = NULL;
                    CurrentTestNumber++;
                }
                break;

                case PORT_RECONFIG_TRANSITION_TEST_3:
                {
                    fprintf(filehandle, "\nStarting test %4d: PORT_RECONFIG_TRANSITION_TEST_3\n", CurrentTestNumber);

                    pInputFile = fopen(InFileName, "rb");
                    pOutputFile = fopen(OutFileName, "wb");

                    pCurrentTest = OSCL_NEW(OmxDecTestPortReconfigTransitionTest_3, (filehandle, pInputFile,
                                            pOutputFile, OutFileName, pRefFileName, ComponentName,
                                            Role, ComponentFormat, Channels));

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pCurrentTest->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pCurrentTest->StartTestApp();

                    OSCL_DELETE(pCurrentTest);
                    pCurrentTest = NULL;

                    fclose(pInputFile);
                    pInputFile = NULL;

                    fclose(pOutputFile);
                    pOutputFile = NULL;

                    CurrentTestNumber++;
                }
                break;

                case FLUSH_PORT_TEST:
                {
                    fprintf(filehandle, "\nStarting test %4d: FLUSH_PORT_TEST \n", CurrentTestNumber);

                    pInputFile = fopen(InFileName, "rb");
                    pOutputFile = fopen(OutFileName, "wb");

                    pCurrentTest = OSCL_NEW(OmxDecTestFlushPort, (filehandle, pInputFile, pOutputFile,
                                            OutFileName, pRefFileName, ComponentName, Role,
                                            ComponentFormat, Channels));

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pCurrentTest->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pCurrentTest->StartTestApp();

                    OSCL_DELETE(pCurrentTest);
                    pCurrentTest = NULL;

                    fclose(pInputFile);
                    pInputFile = NULL;

                    fclose(pOutputFile);
                    pOutputFile = NULL;

                    CurrentTestNumber++;
                }
                break;

                case EOS_AFTER_FLUSH_PORT_TEST:
                {
                    fprintf(filehandle, "\nStarting test %4d: EOS_AFTER_FLUSH_PORT_TEST \n", CurrentTestNumber);

                    pInputFile = fopen(InFileName, "rb");
                    pOutputFile = fopen(OutFileName, "wb");

                    pCurrentTest = OSCL_NEW(OmxDecTestEosAfterFlushPort, (filehandle, pInputFile, pOutputFile,
                                            OutFileName, pRefFileName, ComponentName, Role,
                                            ComponentFormat, Channels));

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pCurrentTest->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pCurrentTest->StartTestApp();

                    OSCL_DELETE(pCurrentTest);
                    pCurrentTest = NULL;

                    fclose(pInputFile);
                    pInputFile = NULL;

                    fclose(pOutputFile);
                    pOutputFile = NULL;
                    CurrentTestNumber++;
                }
                break;


                case NORMAL_SEQ_TEST:
                {
                    fprintf(filehandle, "\nStarting test %4d: NORMAL_SEQ_TEST \n", CurrentTestNumber);

                    pInputFile = fopen(InFileName, "rb");
                    pOutputFile = fopen(OutFileName, "wb");

                    pCurrentTest = OSCL_NEW(OmxComponentDecTest, (filehandle, pInputFile, pOutputFile,
                                            OutFileName, pRefFileName, ComponentName, Role,
                                            ComponentFormat, Channels));

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pCurrentTest->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pCurrentTest->StartTestApp();

                    OSCL_DELETE(pCurrentTest);
                    pCurrentTest = NULL;

                    fclose(pInputFile);
                    pInputFile = NULL;

                    CurrentTestNumber++;
                }
                break;

                case NORMAL_SEQ_TEST_USEBUFF:
                {
                    fprintf(filehandle, "\nStarting test %4d: NORMAL_SEQ_TEST_USEBUFF \n", CurrentTestNumber);

                    pInputFile = fopen(InFileName, "rb");
                    pOutputFile = fopen(OutFileName, "wb");

                    pCurrentTest = OSCL_NEW(OmxDecTestUseBuffer, (filehandle, pInputFile, pOutputFile,
                                            OutFileName, pRefFileName, ComponentName, Role,
                                            ComponentFormat, Channels));

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pCurrentTest->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pCurrentTest->StartTestApp();

                    OSCL_DELETE(pCurrentTest);
                    pCurrentTest = NULL;

                    fclose(pInputFile);
                    pInputFile = NULL;

                    CurrentTestNumber++;
                }
                break;

                case ENDOFSTREAM_MISSING_TEST:
                {
                    fprintf(filehandle, "\nStarting test %4d: ENDOFSTREAM_MISSING_TEST \n", CurrentTestNumber);

                    pInputFile = fopen(InFileName, "rb");
                    pOutputFile = fopen(OutFileName, "wb");

                    pCurrentTest = OSCL_NEW(OmxDecTestEosMissing, (filehandle, pInputFile, pOutputFile,
                                            OutFileName, pRefFileName, ComponentName, Role,
                                            ComponentFormat, Channels));

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pCurrentTest->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pCurrentTest->StartTestApp();

                    OSCL_DELETE(pCurrentTest);
                    pCurrentTest = NULL;

                    fclose(pInputFile);
                    pInputFile = NULL;

                    pOutputFile = NULL;

                    CurrentTestNumber++;
                }
                break;


                case PARTIAL_FRAMES_TEST:
                {
                    fprintf(filehandle, "\nStarting test %4d: PARTIAL_FRAMES_TEST \n", CurrentTestNumber);

                    pInputFile = fopen(InFileName, "rb");
                    pOutputFile = fopen(OutFileName, "wb");

                    pCurrentTest = OSCL_NEW(OmxDecTestPartialFrames, (filehandle, pInputFile, pOutputFile,
                                            OutFileName, pRefFileName, ComponentName, Role,
                                            ComponentFormat, Channels));

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pCurrentTest->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pCurrentTest->StartTestApp();

                    OSCL_DELETE(pCurrentTest);
                    pCurrentTest = NULL;
                    fclose(pInputFile);
                    pInputFile = NULL;
                    CurrentTestNumber++;
                }
                break;

                case EXTRA_PARTIAL_FRAMES_TEST:
                {
                    fprintf(filehandle, "\nStarting test %4d: EXTRA_PARTIAL_FRAMES_TEST \n", CurrentTestNumber);

                    pInputFile = fopen(InFileName, "rb");
                    pOutputFile = fopen(OutFileName, "wb");

                    pCurrentTest = OSCL_NEW(OmxDecTestExtraPartialFrames, (filehandle, pInputFile, pOutputFile,
                                            OutFileName, pRefFileName, ComponentName, Role,
                                            ComponentFormat, Channels));

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pCurrentTest->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pCurrentTest->StartTestApp();

                    OSCL_DELETE(pCurrentTest);
                    pCurrentTest = NULL;

                    fclose(pInputFile);
                    pInputFile = NULL;

                    CurrentTestNumber++;
                }
                break;

                case INPUT_OUTPUT_BUFFER_BUSY_TEST:
                {
                    fprintf(filehandle, "\nStarting test %4d: INPUT_OUTPUT_BUFFER_BUSY_TEST \n", CurrentTestNumber);

                    pInputFile = fopen(InFileName, "rb");
                    pOutputFile = fopen(OutFileName, "wb");

                    pCurrentTest = OSCL_NEW(OmxDecTestBufferBusy, (filehandle, pInputFile, pOutputFile,
                                            OutFileName, pRefFileName, ComponentName, Role,
                                            ComponentFormat, Channels));

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pCurrentTest->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pCurrentTest->StartTestApp();

                    OSCL_DELETE(pCurrentTest);
                    pCurrentTest = NULL;

                    fclose(pInputFile);
                    pInputFile = NULL;

                    CurrentTestNumber++;
                }
                break;


                case PAUSE_RESUME_TEST:
                {
                    fprintf(filehandle, "\nStarting test %4d: PAUSE_RESUME_TEST \n", CurrentTestNumber);

                    pInputFile = fopen(InFileName, "rb");
                    pOutputFile = fopen(OutFileName, "wb");

                    pCurrentTest = OSCL_NEW(OmxDecTestPauseResume, (filehandle, pInputFile, pOutputFile,
                                            OutFileName, pRefFileName, ComponentName, Role,
                                            ComponentFormat, Channels));

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pCurrentTest->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pCurrentTest->StartTestApp();

                    OSCL_DELETE(pCurrentTest);
                    pCurrentTest = NULL;

                    fclose(pInputFile);
                    pInputFile = NULL;

                    CurrentTestNumber++;
                }
                break;

                case REPOSITIONING_TEST:
                {
                    fprintf(filehandle, "\nStarting test %4d: REPOSITIONING_TEST \n", CurrentTestNumber);

                    pInputFile = fopen(InFileName, "rb");
                    pOutputFile = fopen(OutFileName, "wb");

                    pCurrentTest = OSCL_NEW(OmxDecTestReposition, (filehandle, pInputFile, pOutputFile,
                                            OutFileName, pRefFileName, ComponentName, Role,
                                            ComponentFormat, Channels));

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pCurrentTest->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pCurrentTest->StartTestApp();

                    OSCL_DELETE(pCurrentTest);
                    pCurrentTest = NULL;

                    fclose(pInputFile);
                    pInputFile = NULL;

                    CurrentTestNumber++;
                }
                break;

                case MISSING_NAL_TEST:
                {
                    fprintf(filehandle, "\nStarting test %4d: MISSING_NAL_TEST\n", CurrentTestNumber);

                    pInputFile = fopen(InFileName, "rb");
                    pOutputFile = fopen(OutFileName, "wb");

                    pCurrentTest = OSCL_NEW(OmxDecTestMissingNALTest, (filehandle, pInputFile, pOutputFile,
                                            OutFileName, pRefFileName, ComponentName, Role,
                                            ComponentFormat, Channels));

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pCurrentTest->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pCurrentTest->StartTestApp();

                    OSCL_DELETE(pCurrentTest);
                    pCurrentTest = NULL;

                    fclose(pInputFile);
                    pInputFile = NULL;

                    fclose(pOutputFile);
                    pOutputFile = NULL;

                    CurrentTestNumber++;
                }
                break;

                case CORRUPT_NAL_TEST:
                {
                    fprintf(filehandle, "\nStarting test %4d: CORRUPT_NAL_TEST\n", CurrentTestNumber);

                    pInputFile = fopen(InFileName, "rb");
                    pOutputFile = fopen(OutFileName, "wb");

                    pCurrentTest = OSCL_NEW(OmxDecTestCorruptNALTest, (filehandle, pInputFile, pOutputFile,
                                            OutFileName, pRefFileName, ComponentName, Role,
                                            ComponentFormat, Channels));

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pCurrentTest->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pCurrentTest->StartTestApp();

                    OSCL_DELETE(pCurrentTest);
                    pCurrentTest = NULL;

                    fclose(pInputFile);
                    pInputFile = NULL;

                    fclose(pOutputFile);
                    pOutputFile = NULL;

                    CurrentTestNumber++;
                }
                break;

                case INCOMPLETE_NAL_TEST:
                {
                    fprintf(filehandle, "\nStarting test %4d: INCOMPLETE_NAL_TEST\n", CurrentTestNumber);

                    pInputFile = fopen(InFileName, "rb");
                    pOutputFile = fopen(OutFileName, "wb");

                    pCurrentTest = OSCL_NEW(OmxDecTestIncompleteNALTest, (filehandle, pInputFile, pOutputFile,
                                            OutFileName, pRefFileName, ComponentName, Role,
                                            ComponentFormat, Channels));

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pCurrentTest->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pCurrentTest->StartTestApp();

                    OSCL_DELETE(pCurrentTest);
                    pCurrentTest = NULL;

                    fclose(pInputFile);
                    pInputFile = NULL;

                    fclose(pOutputFile);
                    pOutputFile = NULL;

                    CurrentTestNumber++;
                }
                break;

                default:
                {
                    // just skip the count
                    CurrentTestNumber++;
                }
                break;
            }
        }

        if (ComponentName)
        {
            oscl_free(ComponentName);
        }

        if (Role)
        {
            oscl_free(Role);
        }

        if (pInputFile)
        {
            fclose(pInputFile);
        }

    }

    // Clean OSCL
    PVLogger::Cleanup();
    OsclErrorTrap::Cleanup();
    OsclMem::Cleanup();
    OsclBase::Cleanup();
    mem_lock_mutex.Close();

    return 0;
}


OMX_ERRORTYPE OmxComponentDecTest::GetInput()
{
    OMX_S32 Index;
    OMX_S32 ReadCount;
    OMX_S32 Size;
    OMX_ERRORTYPE Status;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::GetInput() - IN"));

    if (OMX_TRUE == iInputReady)
    {
        if (OMX_TRUE == iInputWasRefused)
        {
            Index = iInIndex; //use previously found and saved Index, do not read the file (this was done earlier)
            iInputWasRefused = OMX_FALSE; // reset the flag
        }
        else
        {
            Index = 0;
            while (OMX_FALSE == ipInputAvail[Index] && Index < iInBufferCount)
            {
                Index++;
            }

            if (iInBufferCount == Index)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::GetInput() - Input buffer not available, OUT"));
                return OMX_ErrorNone;
            }

            if (!iFragmentInProgress)
            {
                iCurFrag = 0;
                Size = iInBufferSize;
                for (OMX_S32 kk = 0; kk < iNumSimFrags - 1; kk++)
                {
                    iSimFragSize[kk] = Size / iNumSimFrags;
                }
                iSimFragSize[iNumSimFrags-1] = Size - (iNumSimFrags - 1) * (Size / iNumSimFrags); // last fragment
            }

            Size = iSimFragSize[iCurFrag];
            iCurFrag++;
            if (iCurFrag == iNumSimFrags)
            {
                iFragmentInProgress = OMX_FALSE;
            }
            else
            {
                iFragmentInProgress = OMX_TRUE;
            }

            ReadCount = fread(ipInBuffer[Index]->pBuffer, 1, Size, ipInputFile);
            if (0 == ReadCount)
            {
                iStopProcessingInput = OMX_TRUE;
                iInIndex = Index;
                iLastInputFrame = iFrameNum;
                iFragmentInProgress = OMX_FALSE;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInput() - Input file has ended, OUT"));
                return OMX_ErrorNone;
            }
            else if (ReadCount < Size)
            {
                iStopProcessingInput = OMX_TRUE;
                iInIndex = Index;
                iLastInputFrame = iFrameNum;
                iFragmentInProgress = OMX_FALSE;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::GetInput() - Last piece of data in input file"));
                Size = ReadCount;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::GetInput() - Input data of %d bytes read from the file", Size));

            if (iFragmentInProgress)
            {
                ipInBuffer[Index]->nFlags  = 0;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInput() - Input buffer of index %d without any marker", Index));
            }
            else
            {
                iFrameNum++;
            }

            ipInBuffer[Index]->nFilledLen = Size;
        }

        ipInBuffer[Index]->nOffset = 0;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "OmxComponentDecTest::GetInput() - Calling EmptyThisBuffer with Alloc Len %d, Filled Len %d Offset %d TimeStamp %d",
                         ipInBuffer[Index]->nAllocLen, ipInBuffer[Index]->nFilledLen, ipInBuffer[Index]->nOffset, ipInBuffer[Index]->nTimeStamp));

        Status = OMX_EmptyThisBuffer(ipAppPriv->Handle, ipInBuffer[Index]);

        if (OMX_ErrorNone == Status)
        {
            ipInputAvail[Index] = OMX_FALSE; // mark unavailable

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxComponentDecTest::GetInput() - Sent the input buffer sucessfully, OUT"));
            return OMX_ErrorNone;
        }
        else if (OMX_ErrorInsufficientResources == Status)
        {
            iInIndex = Index; // save the current input buffer, wait for call back
            iInputReady = OMX_FALSE;
            iInputWasRefused = OMX_TRUE;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInput() - EmptyThisBuffer failed with Error OMX_ErrorInsufficientResources, OUT"));
            return OMX_ErrorInsufficientResources;
        }
        else
        {
            iStopProcessingInput = OMX_TRUE;
            iLastInputFrame = iFrameNum;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInput() - Error, EmptyThisBuffer failed, OUT"));
            return OMX_ErrorNone;
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInput() - Error, Input is not ready to be sent, OUT"));
        return OMX_ErrorInsufficientResources;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::GetInput() - OUT"));
    return OMX_ErrorNone;
}



// Read appropriate input bytes from the file & copy it in the buffer to pass to the component.
// For AVC component
OMX_ERRORTYPE OmxComponentDecTest::GetInputFrameAvc()
{
    OMX_S32 Index;
    OMX_S32 Size;
    OMX_ERRORTYPE Status;
    OMX_S32 TempSize;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::GetInputFrameAvc() - IN"));

    if (OMX_TRUE == iInputReady)
    {
        if (OMX_TRUE == iInputWasRefused)
        {
            Index = iInIndex; //use previously found and saved Index, do not read the file (this was done earlier)
            iInputWasRefused = OMX_FALSE; // reset the flag
        }
        else
        {
            Index = 0;
            while (OMX_FALSE == ipInputAvail[Index] && Index < iInBufferCount)
            {
                Index++;
            }

            if (iInBufferCount == Index)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::GetInputFrameAvc() - Input buffer not available, OUT"));
                return OMX_ErrorNone;
            }

            if (iHeaderSent)
            {
                int inserted_size = 0;

                if (!iFragmentInProgress && !iDivideBuffer)
                {
                    if (AVCOMX_FAIL == (ipAVCBSO->GetNextFullNAL(&ipNalBuffer, &iNalSize)))
                    {
                        iStopProcessingInput = OMX_TRUE;
                        iInIndex = Index;
                        iLastInputFrame = iFrameNum;
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameAvc() - Error GetNextFullNAL failed, OUT"));
                        return OMX_ErrorNone;
                    }

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxComponentDecTest::GetInputFrameAvc() - Next NAL identified of size %d", iNalSize));

                    Size = iNalSize;
                    iCurFrag = 0;

                    for (OMX_S32 kk = 0; kk < iNumSimFrags - 1; kk++)
                    {
                        iSimFragSize[kk] = Size / iNumSimFrags;
                    }

                    iSimFragSize[iNumSimFrags - 1] = Size - (iNumSimFrags - 1) * (Size / iNumSimFrags); // last fragment
                }

                if (OMX_FALSE == iDivideBuffer)
                {
                    Size = iSimFragSize[iCurFrag];

                    iCurFrag++;

#ifdef INSERT_NAL_START_CODE
                    // insert start code only in the first piece of a divided buffer of the 1st fragment
                    if (iCurFrag == 1) // frag was already incremented
                    {
                        oscl_memcpy(ipInBuffer[Index]->pBuffer, NAL_START_CODE, NAL_START_CODE_SIZE);
                        inserted_size = NAL_START_CODE_SIZE;
                    }
                    else
                    {
                        inserted_size = 0;
                    }
#endif
                    if (iCurFrag == iNumSimFrags)
                    {
                        iFragmentInProgress = OMX_FALSE;
                    }
                    else
                    {
                        iFragmentInProgress = OMX_TRUE;
                    }

                    if ((Size + inserted_size) > iInBufferSize)
                    {
                        // break up frame (fragment) into multiple buffers
                        iDivideBuffer = OMX_TRUE;

                        TempSize = iInBufferSize - inserted_size;
                        iRemainingFrameSize = Size - TempSize;
                        Size = TempSize;
                    }
                }
                else
                {
                    if ((Size = iRemainingFrameSize) < iInBufferSize)
                    {
                        // this is the last piece of a broken up frame
                        iRemainingFrameSize = 0;
                        iDivideBuffer = OMX_FALSE;
                    }
                    else
                    {
                        iRemainingFrameSize = Size - iInBufferSize;
                        Size = iInBufferSize;
                    }
                }


                oscl_memcpy(ipInBuffer[Index]->pBuffer + inserted_size, ipNalBuffer, Size);
                ipInBuffer[Index]->nFilledLen = Size + inserted_size;
                ipInBuffer[Index]->nOffset = 0;  // for now all data starts at the beginning of the buffer

                ipNalBuffer += Size;
            }
            else
            {
                ipAVCBSO->GetNextFullNAL(&ipNalBuffer, &iNalSize);

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameAvc() - Next NAL identified of size %d", iNalSize));

#ifdef INSERT_NAL_START_CODE

                oscl_memcpy(ipInBuffer[Index]->pBuffer, NAL_START_CODE, NAL_START_CODE_SIZE);
                oscl_memcpy(ipInBuffer[Index]->pBuffer + NAL_START_CODE_SIZE, ipNalBuffer, iNalSize);
                ipInBuffer[Index]->nFilledLen = iNalSize + NAL_START_CODE_SIZE;

#else
                oscl_memcpy(ipInBuffer[Index]->pBuffer, ipNalBuffer, iNalSize);
                ipInBuffer[Index]->nFilledLen = iNalSize;
#endif
                ipInBuffer[Index]->nOffset = 0;  // for now all data starts at the beginning of the buffer
                iHeaderSent = OMX_TRUE;
            }

            if (iFragmentInProgress || iDivideBuffer)
            {
                ipInBuffer[Index]->nFlags  = 0;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameAvc() - Input buffer of index %d without any marker", Index));
            }
            else
            {
                iFrameNum++;
                ipInBuffer[Index]->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameAvc() - Input buffer of index %d with OMX_BUFFERFLAG_ENDOFFRAME flag marked", Index));
            }
        }

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "OmxComponentDecTest::GetInputFrameAvc() - Calling EmptyThisBuffer with Alloc Len %d, Filled Len %d Offset %d TimeStamp %d",
                         ipInBuffer[Index]->nAllocLen, ipInBuffer[Index]->nFilledLen, ipInBuffer[Index]->nOffset, ipInBuffer[Index]->nTimeStamp));

        Status = OMX_EmptyThisBuffer(ipAppPriv->Handle, ipInBuffer[Index]);

        if (OMX_ErrorNone == Status)
        {
            ipInputAvail[Index] = OMX_FALSE; // mark unavailable
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxComponentDecTest::GetInputFrameAvc() - Sent the input buffer sucessfully, OUT"));

            return OMX_ErrorNone;
        }
        else if (OMX_ErrorInsufficientResources == Status)
        {
            iInIndex = Index; // save the current input buffer, wait for call back
            iInputReady = OMX_FALSE;
            iInputWasRefused = OMX_TRUE;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameAvc() - EmptyThisBuffer failed with Error OMX_ErrorInsufficientResources, OUT"));

            return OMX_ErrorInsufficientResources;
        }
        else
        {
            iStopProcessingInput = OMX_TRUE;
            iLastInputFrame = iFrameNum;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameAvc() - ERROR, EmptyThisBuffer failed, OUT"));
            return OMX_ErrorNone;
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameAvc() - ERROR, Input is not ready to be sent, OUT"));
        return OMX_ErrorInsufficientResources;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::GetInputFrameAvc() - OUT"));

    return OMX_ErrorNone;
}

int32 omx_m4v_getVideoHeader(int32 layer, uint8 *buf, int32 max_size);
int32 omx_m4v_getNextVideoSample(int32 layer_id, uint8 **buf, int32 max_buffer_size, uint32 *ts);

static uint8 OMX_VOSH_START_CODE[] = { 0x00, 0x00, 0x01, 0xB0 };
static uint8 OMX_VO_START_CODE[] = { 0x00, 0x00, 0x01, 0x00 };
static uint8 OMX_VOP_START_CODE[] = { 0x00, 0x00, 0x01, 0xB6 };
static uint8 OMX_H263_START_CODE[] = { 0x00, 0x00, 0x80};

static bool omx_short_video_header = false;

int32 omx_m4v_getVideoHeader(int32 layer, uint8 *buf, int32 max_size)
{
    OSCL_UNUSED_ARG(layer);

    int32 count = 0;
    char my_sc[4];

    uint8 *tmp_bs = buf;

    memcpy(my_sc, tmp_bs, 4);
    my_sc[3] &= 0xf0;

    if (max_size >= 4)
    {
        if (oscl_memcmp(my_sc, OMX_VOSH_START_CODE, 4) && oscl_memcmp(my_sc, OMX_VO_START_CODE, 4))
        {
            count = 0;
            omx_short_video_header = true;
        }
        else
        {
            count = 0;
            omx_short_video_header = false;
            while (oscl_memcmp(tmp_bs + count, OMX_VOP_START_CODE, 4))
            {
                count++;
                if (count > 1000)
                {
                    omx_short_video_header = true;
                    break;
                }
            }
            if (omx_short_video_header == true)
            {
                count = 0;
                while (oscl_memcmp(tmp_bs + count, OMX_H263_START_CODE, 3))
                {
                    count++;
                }
            }
        }
    }
    return count;
}

int32 omx_m4v_getNextVideoSample(int32 layer, uint8 **buf, int32 max_buffer_size, uint32 *timestamp)
{
    OSCL_UNUSED_ARG(layer);

    uint8 *tmp_bs = *buf;
    int32 nBytesRead = -1;
    int32 skip;
    int count = 0;
    int32 i, size;
    uint8 *ptr;

    if (max_buffer_size > 0)
    {
        skip = 1;
        do
        {
            if (omx_short_video_header)
            {
                size = max_buffer_size - skip;
                ptr = tmp_bs + skip;

                i = size;
                if (size < 1)
                {
                    nBytesRead = 0;
                    break;
                }

                while (i--)
                {
                    if ((count > 1) && ((*ptr & 0xFC) == 0x80))
                    {
                        i += 2;
                        break;
                    }

                    if (*ptr++)
                        count = 0;
                    else
                        count++;
                }
                nBytesRead = (size - (i + 1));


                //nBytesRead = PVLocateH263FrameHeader(tmp_bs+skip, max_buffer_size-skip);
            }
            else
            {
                size = max_buffer_size - skip;
                ptr = tmp_bs + skip;

                i = size;
                if (size < 1)
                {
                    nBytesRead = 0;
                    break;
                }

                while (i--)
                {
                    if ((count > 1) && (*ptr == 0x01))
                    {
                        i += 2;
                        break;
                    }

                    if (*ptr++)
                        count = 0;
                    else
                        count++;
                }
                nBytesRead = (size - (i + 1));


            }
            //nBytesRead = PVLocateFrameHeader(tmp_bs+skip, max_buffer_size-skip);


            if (nBytesRead == 0) skip++;

        }
        while (nBytesRead == 0);
        if (nBytesRead < 0)
        {
            nBytesRead = max_buffer_size - skip;
        }
        if (nBytesRead > 0)
        {
            nBytesRead += skip;
            max_buffer_size -= nBytesRead;
            if (max_buffer_size < 0)  max_buffer_size = 0;
            timestamp[0] = -1;
        }
    }
    return nBytesRead;
}


//Read the data from input file & pass it to the MPEG4 component
OMX_ERRORTYPE OmxComponentDecTest::GetInputFrameMpeg4()
{
    OMX_S32 Index, ReadCount, Size, BitstreamSize, TempSize;
    OMX_ERRORTYPE Status;
    uint32 TimeStamp;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::GetInputFrameMpeg4() - IN"));

    if (OMX_TRUE == iInputReady)
    {
        if (OMX_TRUE == iInputWasRefused)
        {
            Index = iInIndex; //use previously found and saved Index, do not read the file (this was done earlier)
            iInputWasRefused = OMX_FALSE; // reset the flag
        }
        else
        {
            Index = 0;
            while (OMX_FALSE == ipInputAvail[Index] && Index < iInBufferCount)
            {
                Index++;
            }

            if (iInBufferCount == Index)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::GetInputFrameMpeg4() - Input buffer not available, OUT"));
                return OMX_ErrorNone;
            }

            if (iHeaderSent)
            {
                if (!iFragmentInProgress && !iDivideBuffer)
                {
                    TimeStamp = -1;
                    Size = omx_m4v_getNextVideoSample(0, &ipBitstreamBuffer, iMaxSize, &TimeStamp);

                    iMaxSize -= Size;

                    iCurFrag = 0;

                    if (Size <= 0)
                    {
                        iStopProcessingInput = OMX_TRUE;
                        iInIndex = Index;
                        iLastInputFrame = iFrameNum;

                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameMpeg4() - Error Video sample of size 0, OUT"));
                        return OMX_ErrorNone;
                    }

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxComponentDecTest::GetInputFrameMpeg4() - Next Video sample identified of size %d", Size));

                    for (OMX_S32 kk = 0; kk < iNumSimFrags - 1; kk++)
                    {
                        iSimFragSize[kk] = Size / iNumSimFrags;
                    }

                    iSimFragSize[iNumSimFrags - 1] = Size - (iNumSimFrags - 1) * (Size / iNumSimFrags); // last fragment
                }

                if (OMX_FALSE == iDivideBuffer)
                {
                    Size = iSimFragSize[iCurFrag];
                    iCurFrag++;

                    if (iCurFrag == iNumSimFrags)
                    {
                        iFragmentInProgress = OMX_FALSE;
                    }
                    else
                    {
                        iFragmentInProgress = OMX_TRUE;
                    }

                    if (Size > iInBufferSize)
                    {
                        iDivideBuffer = OMX_TRUE;
                        TempSize = iInBufferSize;

                        iRemainingFrameSize = Size - TempSize;
                        Size = TempSize;
                    }
                }
                else
                {
                    if ((Size = iRemainingFrameSize) < iInBufferSize)
                    {
                        iRemainingFrameSize = 0;
                        iDivideBuffer = OMX_FALSE;
                    }
                    else
                    {

                        iRemainingFrameSize = Size - iInBufferSize;
                        Size = iInBufferSize;
                    }
                }


                oscl_memcpy(ipInBuffer[Index]->pBuffer, ipBitstreamBuffer, Size);
                ipInBuffer[Index]->nFilledLen = Size;
                ipInBuffer[Index]->nOffset = 0;
                ipBitstreamBuffer += Size;
            }

            else
            {
                ReadCount = fread(ipBitstreamBuffer, 1, BIT_BUFF_SIZE,  ipInputFile);

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::GetInputFrameMpeg4() - Input file read into a big bitstream buffer OUT"));

                BitstreamSize = ReadCount;
                iMaxSize = ReadCount;

                Size = omx_m4v_getVideoHeader(0, ipBitstreamBuffer, iMaxSize);

                if (0 == Size)
                {
                    ipInBuffer[Index]->pBuffer = ipBitstreamBuffer;
                    ipInBuffer[Index]->nFilledLen = 32;
                    ipInBuffer[Index]->nOffset = 0;
                }
                else
                {
                    ipInBuffer[Index]->nFilledLen = Size;
                    oscl_memcpy(ipInBuffer[Index]->pBuffer, ipBitstreamBuffer, Size);
                    ipBitstreamBuffer += Size;
                    ipInBuffer[Index]->nOffset = 0;
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameMpeg4() - Next Video sample identified of size %d", Size));

                iMaxSize -= Size;
                iHeaderSent = OMX_TRUE;
            }

            if (iFragmentInProgress || iDivideBuffer)
            {
                ipInBuffer[Index]->nFlags  = 0;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameMpeg4() - Input buffer of index %d without any marker", Index));
            }
            else
            {
                iFrameNum++;
                ipInBuffer[Index]->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameMpeg4() - Input buffer of index %d with OMX_BUFFERFLAG_ENDOFFRAME flag marked", Index));
            }
        }

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "OmxComponentDecTest::GetInputFrameMpeg4() - Calling EmptyThisBuffer with Alloc Len %d, Filled Len %d Offset %d TimeStamp %d",
                         ipInBuffer[Index]->nAllocLen, ipInBuffer[Index]->nFilledLen, ipInBuffer[Index]->nOffset, ipInBuffer[Index]->nTimeStamp));

        Status = OMX_EmptyThisBuffer(ipAppPriv->Handle, ipInBuffer[Index]);
        if (OMX_ErrorNone == Status)
        {
            ipInputAvail[Index] = OMX_FALSE; // mark unavailable

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxComponentDecTest::GetInputFrameMpeg4() - Sent the input buffer sucessfully, OUT"));
            return OMX_ErrorNone;
        }
        else if (OMX_ErrorInsufficientResources == Status)
        {
            iInIndex = Index; // save the current input buffer, wait for call back
            iInputReady = OMX_FALSE;
            iInputWasRefused = OMX_TRUE;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameMpeg4() - EmptyThisBuffer failed with Error OMX_ErrorInsufficientResources, OUT"));
            return OMX_ErrorInsufficientResources;
        }
        else
        {
            iStopProcessingInput = OMX_TRUE;
            iLastInputFrame = iFrameNum;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameMpeg4() - ERROR, EmptyThisBuffer failed, OUT"));
            return OMX_ErrorNone;

        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameMpeg4() - ERROR, Input is not ready to be sent, OUT"));
        return OMX_ErrorInsufficientResources;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::GetInputFrameMpeg4() - OUT"));

    return OMX_ErrorNone;
}


//Read the data from input file & pass it to the H263 component
OMX_ERRORTYPE OmxComponentDecTest::GetInputFrameH263()
{
    OMX_S32 Index, ReadCount, Size, BitstreamSize, TempSize;
    OMX_ERRORTYPE Status;
    uint32 TimeStamp;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::GetInputFrameH263() - IN"));

    if (OMX_TRUE == iInputReady)
    {
        if (OMX_TRUE == iInputWasRefused)
        {
            Index = iInIndex; //use previously found and saved Index, do not read the file (this was done earlier)
            iInputWasRefused = OMX_FALSE; // reset the flag
        }
        else
        {
            Index = 0;
            while (OMX_FALSE == ipInputAvail[Index] && Index < iInBufferCount)
            {
                Index++;
            }

            if (iInBufferCount == Index)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::GetInputFrameH263() - Input buffer not available, OUT"));
                return OMX_ErrorNone;
            }

            if (iHeaderSent)
            {
                if (!iFragmentInProgress && !iDivideBuffer)
                {
                    TimeStamp = -1;

                    Size = omx_m4v_getNextVideoSample(0, &ipBitstreamBuffer, iMaxSize, &TimeStamp);

                    iMaxSize -= Size;

                    iCurFrag = 0;

                    if (Size <= 0)
                    {
                        iStopProcessingInput = OMX_TRUE;
                        iInIndex = Index;
                        iLastInputFrame = iFrameNum;

                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameH263() - Error Video sample of size 0, OUT"));
                        return OMX_ErrorNone;
                    }

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxComponentDecTest::GetInputFrameH263() - Next Video sample identified of size %d", Size));

                    for (OMX_S32 kk = 0; kk < iNumSimFrags - 1; kk++)
                    {
                        iSimFragSize[kk] = Size / iNumSimFrags;
                    }

                    iSimFragSize[iNumSimFrags - 1] = Size - (iNumSimFrags - 1) * (Size / iNumSimFrags); // last fragment
                }

                if (OMX_FALSE == iDivideBuffer)
                {
                    Size = iSimFragSize[iCurFrag];
                    iCurFrag++;

                    if (iCurFrag == iNumSimFrags)
                    {
                        iFragmentInProgress = OMX_FALSE;
                    }
                    else
                    {
                        iFragmentInProgress = OMX_TRUE;
                    }

                    if (Size > iInBufferSize)
                    {
                        iDivideBuffer = OMX_TRUE;

                        TempSize = iInBufferSize;

                        iRemainingFrameSize = Size - TempSize;
                        Size = TempSize;

                    }
                }
                else
                {
                    if ((Size = iRemainingFrameSize) < iInBufferSize)
                    {
                        // the last piece of a broken up frame
                        iRemainingFrameSize = 0;
                        iDivideBuffer = OMX_FALSE;
                    }
                    else
                    {

                        iRemainingFrameSize = Size - iInBufferSize;
                        Size = iInBufferSize;
                    }
                }


                oscl_memcpy(ipInBuffer[Index]->pBuffer, ipBitstreamBuffer, Size);
                ipInBuffer[Index]->nFilledLen = Size;
                ipInBuffer[Index]->nOffset = 0;
                ipBitstreamBuffer += Size;
            }
            else
            {
                ReadCount = fread(ipBitstreamBuffer, 1, BIT_BUFF_SIZE,  ipInputFile);
                BitstreamSize = ReadCount;
                iMaxSize = ReadCount;
                //Only call this to set internal short_video_header flag as true
                omx_m4v_getVideoHeader(0, ipBitstreamBuffer, iMaxSize);
                iHeaderSent = OMX_TRUE;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::GetInputFrameH263() - Input file read into a big bitstream buffer OUT"));
                return OMX_ErrorNone;
            }

            if (iFragmentInProgress || iDivideBuffer)
            {
                ipInBuffer[Index]->nFlags  = 0;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameH263() - Input buffer of index %d without any marker", Index));
            }
            else
            {
                iFrameNum++;
                ipInBuffer[Index]->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameH263() - Input buffer of index %d with OMX_BUFFERFLAG_ENDOFFRAME flag marked", Index));
            }
        }

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "OmxComponentDecTest::GetInputFrameH263() - Calling EmptyThisBuffer with Alloc Len %d, Filled Len %d Offset %d TimeStamp %d",
                         ipInBuffer[Index]->nAllocLen, ipInBuffer[Index]->nFilledLen, ipInBuffer[Index]->nOffset, ipInBuffer[Index]->nTimeStamp));

        Status = OMX_EmptyThisBuffer(ipAppPriv->Handle, ipInBuffer[Index]);
        if (OMX_ErrorNone == Status)
        {
            ipInputAvail[Index] = OMX_FALSE; // mark unavailable

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxComponentDecTest::GetInputFrameH263() - Sent the input buffer sucessfully, OUT"));
            return OMX_ErrorNone;
        }
        else if (OMX_ErrorInsufficientResources == Status)
        {
            iInIndex = Index; // save the current input buffer, wait for call back
            iInputReady = OMX_FALSE;
            iInputWasRefused = OMX_TRUE;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameH263() - EmptyThisBuffer failed with Error OMX_ErrorInsufficientResources, OUT"));
            return OMX_ErrorInsufficientResources;
        }
        else
        {
            iStopProcessingInput = OMX_TRUE;
            iLastInputFrame = iFrameNum;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameH263() - ERROR, EmptyThisBuffer failed, OUT"));
            return OMX_ErrorNone;

        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameH263() - ERROR, Input is not ready to be sent, OUT"));
        return OMX_ErrorInsufficientResources;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::GetInputFrameH263() - OUT"));

    return OMX_ErrorNone;
}



//Read the data from input file & pass it to the AAC component
OMX_ERRORTYPE OmxComponentDecTest::GetInputFrameAac()
{
    OMX_S32 Index;
    OMX_S32 Size;
    OMX_ERRORTYPE Status;
    OMX_S32 ReadCount;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::GetInputFrameAac() - IN"));

    if (OMX_TRUE == iInputReady)
    {
        if (OMX_TRUE == iInputWasRefused)
        {
            Index = iInIndex; //use previously found and saved Index, do not read the file (this was done earlier)
            iInputWasRefused = OMX_FALSE; // reset the flag
        }
        else
        {
            Index = 0;
            while (OMX_FALSE == ipInputAvail[Index] && Index < iInBufferCount)
            {
                Index++;
            }

            if (iInBufferCount == Index)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::GetInputFrameAac() - Input buffer not available, OUT"));
                return OMX_ErrorNone;
            }


            if (iHeaderSent)
            {
                if (!iFragmentInProgress)
                {
                    ReadCount = fread(&Size, 1, FRAME_SIZE_FIELD, ipInputFile); // read in 1st 4 bytes
                    iCurFrag = 0;

                    if (ReadCount < FRAME_SIZE_FIELD)
                    {
                        iStopProcessingInput = OMX_TRUE;
                        iInIndex = Index;
                        iLastInputFrame = iFrameNum;

                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameAac() - Input file has ended, OUT"));
                        return OMX_ErrorNone;
                    }

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::GetInputFrameAac() - Next frame size is %d", Size));

                    ReadCount = fread(&iFrameTimeStamp, 1, FRAME_TIME_STAMP_FIELD, ipInputFile); // read in 2nd 4 bytes

                    if (ReadCount < FRAME_TIME_STAMP_FIELD)
                    {
                        iStopProcessingInput = OMX_TRUE;
                        iInIndex = Index;
                        iLastInputFrame = iFrameNum;

                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameAac() - Input file has ended, OUT"));
                        return OMX_ErrorNone;
                    }

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::GetInputFrameAac() - Next frame time stamp is %d", iFrameTimeStamp));


                    for (OMX_S32 kk = 0; kk < iNumSimFrags - 1; kk++)
                    {
                        iSimFragSize[kk] = Size / iNumSimFrags;
                    }

                    iSimFragSize[iNumSimFrags - 1] = Size - (iNumSimFrags - 1) * (Size / iNumSimFrags); // last fragment
                }

                Size = iSimFragSize[iCurFrag];
                iCurFrag++;

                if (iCurFrag == iNumSimFrags)
                {
                    iFragmentInProgress = OMX_FALSE;
                }
                else
                {
                    iFragmentInProgress = OMX_TRUE;
                }

                ReadCount = fread(ipInBuffer[Index]->pBuffer, 1, Size, ipInputFile);

                if (ReadCount < Size)
                {
                    iStopProcessingInput = OMX_TRUE;
                    iInIndex = Index;
                    iLastInputFrame = iFrameNum;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameAac() - Input file read error, OUT"));

                    return OMX_ErrorNone;   // corrupted file !!
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameAac() - Bytes read from input file %d", Size));

                ipInBuffer[Index]->nFilledLen = Size;
                ipInBuffer[Index]->nOffset = 0;  // for now all data starts at the beginning of the buffer

            }
            else
            {
                ReadCount = fread(&Size, 1, FRAME_SIZE_FIELD, ipInputFile); // read in 1st 4 bytes
                if (ReadCount < FRAME_SIZE_FIELD)
                {
                    iStopProcessingInput = OMX_TRUE;
                    iInIndex = Index;
                    iLastInputFrame = iFrameNum;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameAac() - Input file has ended, OUT"));
                    return OMX_ErrorNone;
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::GetInputFrameAac() - Next frame size is %d", Size));

                ReadCount = fread(&iFrameTimeStamp, 1, FRAME_TIME_STAMP_FIELD, ipInputFile); // read in 2nd 4 bytes
                if (ReadCount < FRAME_TIME_STAMP_FIELD)
                {
                    iStopProcessingInput = OMX_TRUE;
                    iInIndex = Index;
                    iLastInputFrame = iFrameNum;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameAac() - Input file has ended, OUT"));
                    return OMX_ErrorNone;
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::GetInputFrameAac() - Next frame time stamp is %d", iFrameTimeStamp));

                ReadCount = fread(ipInBuffer[Index]->pBuffer, 1, Size, ipInputFile);
                if (ReadCount < Size)
                {
                    iStopProcessingInput = OMX_TRUE;
                    iInIndex = Index;
                    iLastInputFrame = iFrameNum;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameAac() - Input file read error, OUT"));
                    return OMX_ErrorNone;   // corrupted file !!
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameAac() - Bytes read from input file %d", Size));

                ipInBuffer[Index]->nFilledLen = Size;
                ipInBuffer[Index]->nOffset = 0;

                if (1 == iFrameNum)
                {
                    iHeaderSent = OMX_TRUE;
                }
            }

            if (iFragmentInProgress)
            {
                ipInBuffer[Index]->nFlags  = 0;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameAac() - Input buffer of index %d without any marker", Index));
            }
            else
            {
                iFrameNum++;
                ipInBuffer[Index]->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameAac() - Input buffer of index %d with OMX_BUFFERFLAG_ENDOFFRAME flag marked", Index));
            }

            //Testing Silence insertion at random places
            if (0 == (iFrameNum % 15) && (OMX_TRUE == SilenceInsertionEnable) && (1 == iNumSimFrags))
            {
                //Do silence insertion  // Stereo silence frame
                if (iNumberOfChannels > 1)
                {
                    Size = AAC_STEREO_SILENCE_FRAME_SIZE;
                    oscl_memcpy(ipInBuffer[Index]->pBuffer, &AAC_STEREO_SILENCE_FRAME, Size);
                    ipInBuffer[Index]->nFilledLen = AAC_STEREO_SILENCE_FRAME_SIZE;
                    ipInBuffer[Index]->nOffset = 0;
                }
                else
                {
                    Size = AAC_MONO_SILENCE_FRAME_SIZE;
                    oscl_memcpy(ipInBuffer[Index]->pBuffer, &AAC_MONO_SILENCE_FRAME, Size);
                    ipInBuffer[Index]->nFilledLen = AAC_MONO_SILENCE_FRAME_SIZE;
                    ipInBuffer[Index]->nOffset = 0;
                }
            }
        }


        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "OmxComponentDecTest::GetInputFrameAac() - Calling EmptyThisBuffer with Alloc Len %d, Filled Len %d Offset %d TimeStamp %d",
                         ipInBuffer[Index]->nAllocLen, ipInBuffer[Index]->nFilledLen, ipInBuffer[Index]->nOffset, ipInBuffer[Index]->nTimeStamp));

        ipInBuffer[Index]->nTimeStamp = (OMX_TICKS)iFrameTimeStamp;
        Status = OMX_EmptyThisBuffer(ipAppPriv->Handle, ipInBuffer[Index]);
        if (OMX_ErrorNone == Status)
        {
            ipInputAvail[Index] = OMX_FALSE; // mark unavailable

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxComponentDecTest::GetInputFrameAac() - Sent the input buffer sucessfully, OUT"));
            return OMX_ErrorNone;
        }
        else if (OMX_ErrorInsufficientResources == Status)
        {
            iInIndex = Index; // save the current input buffer, wait for call back
            iInputReady = OMX_FALSE;
            iInputWasRefused = OMX_TRUE;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameAac() - EmptyThisBuffer failed with Error OMX_ErrorInsufficientResources, OUT"));
            return OMX_ErrorInsufficientResources;
        }
        else
        {
            iStopProcessingInput = OMX_TRUE;
            iLastInputFrame = iFrameNum;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameAac() - ERROR, EmptyThisBuffer failed, OUT"));
            return OMX_ErrorNone;
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameAac() - ERROR, Input is not ready to be sent, OUT"));
        return OMX_ErrorInsufficientResources;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::GetInputFrameAac() - OUT"));
    return OMX_ErrorNone;
}


//Read the data from input file & pass it to the AMR component
OMX_ERRORTYPE OmxComponentDecTest::GetInputFrameAmr()
{
    OMX_S32 Index;
    OMX_S32 Size;
    OMX_ERRORTYPE Status;
    OMX_S32 ReadCount;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::GetInputFrameAmr() - IN"));

    if (OMX_TRUE == iInputReady)
    {
        if (OMX_TRUE == iInputWasRefused)
        {
            Index = iInIndex; //use previously found and saved Index, do not read the file (this was done earlier)
            iInputWasRefused = OMX_FALSE; // reset the flag
        }
        else
        {
            Index = 0;
            while (OMX_FALSE == ipInputAvail[Index] && Index < iInBufferCount)
            {
                Index++;
            }

            if (iInBufferCount == Index)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::GetInputFrameAmr() - Input buffer not available, OUT"));
                return OMX_ErrorNone;
            }


            if (iHeaderSent)
            {
                if (!iFragmentInProgress)
                {
                    //First four bytes for the size of next frame
                    ReadCount = fread(&Size, 1, FRAME_SIZE_FIELD, ipInputFile); // read in 1st 4 bytes
                    iCurFrag = 0;

                    if (ReadCount < FRAME_SIZE_FIELD)
                    {
                        iStopProcessingInput = OMX_TRUE;
                        iInIndex = Index;
                        iLastInputFrame = iFrameNum;

                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameAmr() - Input file has ended, OUT"));
                        return OMX_ErrorNone;
                    }

                    //Next four bytes for the timestamp of the frame
                    ReadCount = fread(&iFrameTimeStamp, 1, FRAME_SIZE_FIELD, ipInputFile);

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxComponentDecTest::GetInputFrameAmr() - Next frame size is %d and timestamp %d", Size, iFrameTimeStamp));

                    for (OMX_S32 kk = 0; kk < iNumSimFrags - 1; kk++)
                    {
                        iSimFragSize[kk] = Size / iNumSimFrags;
                    }

                    iSimFragSize[iNumSimFrags - 1] = Size - (iNumSimFrags - 1) * (Size / iNumSimFrags); // last fragment
                }

                Size = iSimFragSize[iCurFrag];
                iCurFrag++;

                if (iCurFrag == iNumSimFrags)
                {
                    iFragmentInProgress = OMX_FALSE;
                }
                else
                {
                    iFragmentInProgress = OMX_TRUE;
                }

                ReadCount = fread(ipInBuffer[Index]->pBuffer, 1, Size, ipInputFile);

                if (ReadCount < Size)
                {
                    iStopProcessingInput = OMX_TRUE;
                    iInIndex = Index;
                    iLastInputFrame = iFrameNum;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "OmxComponentDecTest::GetInputFrameAmr() - Input file read error, OUT"));

                    return OMX_ErrorNone;   // corrupted file !!
                }

                ipInBuffer[Index]->nFilledLen = Size;
                ipInBuffer[Index]->nOffset = 0;  // for now all data starts at the beginning of the buffer

            }
            else
            {
                ReadCount = fread(&Size, 1, FRAME_SIZE_FIELD, ipInputFile); // read in 1st 4 bytes
                if (ReadCount < FRAME_SIZE_FIELD)
                {
                    iStopProcessingInput = OMX_TRUE;
                    iInIndex = Index;
                    iLastInputFrame = iFrameNum;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "OmxComponentDecTest::GetInputFrameAmr() - Input file has ended, OUT"));

                    return OMX_ErrorNone;
                }

                //Next four bytes for the timestamp of the frame
                ReadCount = fread((OMX_S32*) & iFrameTimeStamp, 1, FRAME_SIZE_FIELD, ipInputFile);

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameAmr() - Next frame size is %d and timestamp %d", Size, iFrameTimeStamp));


                ReadCount = fread(ipInBuffer[Index]->pBuffer, 1, Size, ipInputFile);
                if (ReadCount < Size)
                {
                    iStopProcessingInput = OMX_TRUE;
                    iInIndex = Index;
                    iLastInputFrame = iFrameNum;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "OmxComponentDecTest::GetInputFrameAmr() - Input file read error, OUT"));
                    return OMX_ErrorNone;   // corrupted file !!
                }

                ipInBuffer[Index]->nFilledLen = Size;
                ipInBuffer[Index]->nOffset = 0;

                iHeaderSent = OMX_TRUE;
            }

            if (iFragmentInProgress)
            {
                ipInBuffer[Index]->nFlags  = 0;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameAmr() - Input buffer of index %d without any marker", Index));
            }
            else
            {
                iFrameNum++;
                ipInBuffer[Index]->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameAmr() - Input buffer of index %d with OMX_BUFFERFLAG_ENDOFFRAME flag marked", Index));
            }
        }

        ipInBuffer[Index]->nTimeStamp = iFrameTimeStamp;
        Status = OMX_EmptyThisBuffer(ipAppPriv->Handle, ipInBuffer[Index]);
        if (OMX_ErrorNone == Status)
        {
            ipInputAvail[Index] = OMX_FALSE; // mark unavailable

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxComponentDecTest::GetInputFrameAmr() - Sent the input buffer sucessfully, OUT"));
            return OMX_ErrorNone;
        }
        else if (OMX_ErrorInsufficientResources == Status)
        {
            iInIndex = Index; // save the current input buffer, wait for call back
            iInputReady = OMX_FALSE;
            iInputWasRefused = OMX_TRUE;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameAmr() - EmptyThisBuffer failed with Error OMX_ErrorInsufficientResources, OUT"));
            return OMX_ErrorInsufficientResources;
        }
        else
        {
            iStopProcessingInput = OMX_TRUE;
            iLastInputFrame = iFrameNum;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameAmr() - ERROR, EmptyThisBuffer failed, OUT"));
            return OMX_ErrorNone;
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameAmr() - ERROR, Input is not ready to be sent, OUT"));
        return OMX_ErrorInsufficientResources;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::GetInputFrameAmr() - OUT"));

    return OMX_ErrorNone;
}


//Read the data from input file & pass it to the WMV component
OMX_ERRORTYPE OmxComponentDecTest::GetInputFrameWmv()
{
    OMX_S32 Index;
    OMX_S32 Size;
    OMX_ERRORTYPE Status;
    OMX_S32 ReadCount;
    OMX_S32 TempSize;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::GetInputFrameWmv() - IN"));

    if (OMX_TRUE == iInputReady)
    {
        if (OMX_TRUE == iInputWasRefused)
        {
            Index = iInIndex; //use previously found and saved Index, do not read the file (this was done earlier)
            iInputWasRefused = OMX_FALSE; // reset the flag
        }
        else
        {
            Index = 0;
            while (OMX_FALSE == ipInputAvail[Index] && Index < iInBufferCount)
            {
                Index++;
            }

            if (iInBufferCount == Index)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::GetInputFrameWmv() - Input buffer not available, OUT"));
                return OMX_ErrorNone;
            }

            if (iHeaderSent)
            {
                if (!iFragmentInProgress && !iDivideBuffer)
                {
                    ReadCount = fread(&Size, 1, FRAME_SIZE_FIELD, ipInputFile); // read in 1st 4 bytes
                    iCurFrag = 0;

                    if (ReadCount < FRAME_SIZE_FIELD)
                    {
                        iStopProcessingInput = OMX_TRUE;
                        iInIndex = Index;
                        iLastInputFrame = iFrameNum;

                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameWmv() - Input file has ended, OUT"));
                        return OMX_ErrorNone;
                    }

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::GetInputFrameWmv() - Next frame size is %d", Size));

                    for (OMX_S32 kk = 0; kk < iNumSimFrags - 1; kk++)
                    {
                        iSimFragSize[kk] = Size / iNumSimFrags;
                    }

                    iSimFragSize[iNumSimFrags - 1] = Size - (iNumSimFrags - 1) * (Size / iNumSimFrags); // last fragment
                }


                if (OMX_FALSE == iDivideBuffer)
                {
                    Size = iSimFragSize[iCurFrag];
                    iCurFrag++;

                    if (iCurFrag == iNumSimFrags)
                    {
                        iFragmentInProgress = OMX_FALSE;
                    }
                    else
                    {
                        iFragmentInProgress = OMX_TRUE;
                    }

                    if (Size > iInBufferSize)
                    {
                        iDivideBuffer = OMX_TRUE;

                        TempSize = iInBufferSize;

                        iRemainingFrameSize = Size - TempSize;
                        Size = TempSize;

                    }
                }
                else
                {
                    if ((Size = iRemainingFrameSize) < iInBufferSize)
                    {
                        iRemainingFrameSize = 0;
                        iDivideBuffer = OMX_FALSE;
                    }
                    else
                    {

                        iRemainingFrameSize = Size - iInBufferSize;
                        Size = iInBufferSize;
                    }
                }


                ReadCount = fread(ipInBuffer[Index]->pBuffer, 1, Size, ipInputFile);

                if (ReadCount < Size)
                {
                    iStopProcessingInput = OMX_TRUE;
                    iInIndex = Index;
                    iLastInputFrame = iFrameNum;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameWmv() - Input file read error, OUT"));

                    return OMX_ErrorNone;   // corrupted file !!
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameWmv() - Bytes read from input file %d", Size));

                ipInBuffer[Index]->nFilledLen = Size;
                ipInBuffer[Index]->nOffset = 0;  // for now all data starts at the beginning of the buffer
            }
            else
            {
                ReadCount = fread(&Size, 1, FRAME_SIZE_FIELD, ipInputFile); // read in 1st 4 bytes
                if (ReadCount < FRAME_SIZE_FIELD)
                {
                    iStopProcessingInput = OMX_TRUE;
                    iInIndex = Index;
                    iLastInputFrame = iFrameNum;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameWmv() - Input file has ended, OUT"));
                    return OMX_ErrorNone;
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::GetInputFrameWmv() - Next frame size is %d", Size));

                ReadCount = fread(ipInBuffer[Index]->pBuffer, 1, Size, ipInputFile);
                if (ReadCount < Size)
                {
                    iStopProcessingInput = OMX_TRUE;
                    iInIndex = Index;
                    iLastInputFrame = iFrameNum;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameWmv() - Input file read error, OUT"));
                    return OMX_ErrorNone;   // corrupted file !!
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameWmv() - Bytes read from input file %d", Size));

                ipInBuffer[Index]->nFilledLen = Size;
                ipInBuffer[Index]->nOffset = 0;
                iHeaderSent = OMX_TRUE;
            }

            if (iFragmentInProgress || iDivideBuffer)
            {
                ipInBuffer[Index]->nFlags  = 0;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameWmv() - Input buffer of index %d without any marker", Index));
            }
            else
            {
                iFrameNum++;
                ipInBuffer[Index]->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameWmv() - Input buffer of index %d with OMX_BUFFERFLAG_ENDOFFRAME flag marked", Index));
            }
        }


        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "OmxComponentDecTest::GetInputFrameWmv() - Calling EmptyThisBuffer with Alloc Len %d, Filled Len %d Offset %d TimeStamp %d",
                         ipInBuffer[Index]->nAllocLen, ipInBuffer[Index]->nFilledLen, ipInBuffer[Index]->nOffset, ipInBuffer[Index]->nTimeStamp));

        Status = OMX_EmptyThisBuffer(ipAppPriv->Handle, ipInBuffer[Index]);
        if (OMX_ErrorNone == Status)
        {
            ipInputAvail[Index] = OMX_FALSE; // mark unavailable

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxComponentDecTest::GetInputFrameWmv() - Sent the input buffer sucessfully, OUT"));
            return OMX_ErrorNone;
        }
        else if (OMX_ErrorInsufficientResources == Status)
        {
            iInIndex = Index; // save the current input buffer, wait for call back
            iInputReady = OMX_FALSE;
            iInputWasRefused = OMX_TRUE;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameWmv() - EmptyThisBuffer failed with Error OMX_ErrorInsufficientResources, OUT"));
            return OMX_ErrorInsufficientResources;
        }
        else
        {
            iStopProcessingInput = OMX_TRUE;
            iLastInputFrame = iFrameNum;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameWmv() - ERROR, EmptyThisBuffer failed, OUT"));
            return OMX_ErrorNone;
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameWmv() - ERROR, Input is not ready to be sent, OUT"));
        return OMX_ErrorInsufficientResources;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::GetInputFrameWmv() - OUT"));
    return OMX_ErrorNone;
}



//Read the data from input file & pass it to the Mp3 component
OMX_ERRORTYPE OmxComponentDecTest::GetInputFrameMp3()
{
    OMX_S32 Index;
    OMX_S32 Size;
    OMX_ERRORTYPE Status;
    OMX_S32 FrameSize;

    OMX_S32 bytes_read;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::GetInputFrameMp3() - IN"));

    if (OMX_TRUE == iInputReady)
    {
        if (OMX_TRUE == iInputWasRefused)
        {
            Index = iInIndex; //use previously found and saved Index, do not read the file (this was done earlier)
            iInputWasRefused = OMX_FALSE; // reset the flag
        }
        else
        {
            Index = 0;
            while (OMX_FALSE == ipInputAvail[Index] && Index < iInBufferCount)
            {
                Index++;
            }

            if (iInBufferCount == Index)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::GetInputFrameMp3() - Input buffer not available, OUT"));
                return OMX_ErrorNone;
            }

            if (iHeaderSent)
            {
                if (!iFragmentInProgress)
                {
                    do
                    {
                        bytes_read = ipMp3Bitstream->DecodeReadInput();
                        if (!bytes_read)
                        {
                            if (0 == ipMp3Bitstream->iInputBufferCurrentLength)
                            {
                                iStopProcessingInput = OMX_TRUE;
                                iInIndex = Index;
                                iLastInputFrame = iFrameNum;

                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                (0, "OmxComponentDecTest::GetInputFrameMp3() - ERROR, Frame boundaries can not be found, OUT"));
                                return OMX_ErrorNone;
                            }
                            else
                            {
                                FrameSize = ipMp3Bitstream->iInputBufferCurrentLength;
                                break;
                            }
                        }
                    }
                    while (ipMp3Bitstream->Mp3FrameSynchronization(&FrameSize));

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxComponentDecTest::GetInputFrameMp3() - Next frame size is %d", FrameSize));

                    Size = FrameSize;
                    iCurFrag = 0;
                    ipMp3Bitstream->iFragmentSizeRead = ipMp3Bitstream->iInputBufferUsedLength;
                    ipMp3Bitstream->iInputBufferUsedLength += FrameSize;

                    for (OMX_S32 kk = 0; kk < iNumSimFrags - 1; kk++)
                    {
                        iSimFragSize[kk] = Size / iNumSimFrags;
                    }

                    iSimFragSize[iNumSimFrags - 1] = Size - (iNumSimFrags - 1) * (Size / iNumSimFrags); // last fragment
                }

                Size = iSimFragSize[iCurFrag];
                iCurFrag++;

                if (iCurFrag == iNumSimFrags)
                {
                    iFragmentInProgress = OMX_FALSE;
                }
                else
                {
                    iFragmentInProgress = OMX_TRUE;
                }

                oscl_memcpy(ipInBuffer[Index]->pBuffer, &ipMp3Bitstream->ipBuffer[ipMp3Bitstream->iFragmentSizeRead], Size);
                ipInBuffer[Index]->nFilledLen = Size;
                ipInBuffer[Index]->nOffset = 0;  // for now all data starts at the beginning of the buffer

                ipMp3Bitstream->iFragmentSizeRead += Size;
            }
            else
            {
                do
                {
                    bytes_read = ipMp3Bitstream->DecodeReadInput();
                    if (!bytes_read)
                    {
                        if (0 == ipMp3Bitstream->iInputBufferCurrentLength)
                        {
                            iStopProcessingInput = OMX_TRUE;
                            iInIndex = Index;
                            iLastInputFrame = iFrameNum;

                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                            (0, "OmxComponentDecTest::GetInputFrameMp3() - ERROR, Frame boundaries can not be found, OUT"));

                            return OMX_ErrorNone;
                        }
                        else
                        {
                            FrameSize = ipMp3Bitstream->iInputBufferCurrentLength;
                            break;
                        }
                    }
                }
                while (ipMp3Bitstream->Mp3FrameSynchronization(&FrameSize));


                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameMp3() - Next frame size is %d", FrameSize));

                ipInBuffer[Index]->nFilledLen = FrameSize;
                ipInBuffer[Index]->nOffset = 0;  // for now all data starts at the beginning of the buffer
                oscl_memcpy(ipInBuffer[Index]->pBuffer, &ipMp3Bitstream->ipBuffer[ipMp3Bitstream->iInputBufferUsedLength], FrameSize);
                iHeaderSent = OMX_TRUE;
                ipMp3Bitstream->iInputBufferUsedLength += FrameSize;
            }

            if (iFragmentInProgress)
            {
                ipInBuffer[Index]->nFlags  = 0;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameMp3() - Input buffer of index %d without any marker", Index));
            }
            else
            {
                iFrameNum++;
                ipInBuffer[Index]->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameMp3() - Input buffer of index %d with OMX_BUFFERFLAG_ENDOFFRAME flag marked", Index));

                if (ipMp3Bitstream->DecodeAdjustInput())
                {
                    iStopProcessingInput = OMX_TRUE;
                    iInIndex = Index;
                    iLastInputFrame = iFrameNum;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameMp3() - Input file has ended, OUT"));
                    return OMX_ErrorNone;
                }
            }
        }


        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "OmxComponentDecTest::GetInputFrameMp3() - Calling EmptyThisBuffer with Alloc Len %d, Filled Len %d Offset %d TimeStamp %d",
                         ipInBuffer[Index]->nAllocLen, ipInBuffer[Index]->nFilledLen, ipInBuffer[Index]->nOffset, ipInBuffer[Index]->nTimeStamp));

        Status = OMX_EmptyThisBuffer(ipAppPriv->Handle, ipInBuffer[Index]);
        if (OMX_ErrorNone == Status)
        {
            ipInputAvail[Index] = OMX_FALSE; // mark unavailable

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxComponentDecTest::GetInputFrameMp3() - Sent the input buffer sucessfully, OUT"));
            return OMX_ErrorNone;
        }
        else if (OMX_ErrorInsufficientResources == Status)
        {
            iInIndex = Index; // save the current input buffer, wait for call back
            iInputReady = OMX_FALSE;
            iInputWasRefused = OMX_TRUE;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameMp3() - EmptyThisBuffer failed with Error OMX_ErrorInsufficientResources, OUT"));
            return OMX_ErrorInsufficientResources;
        }
        else
        {
            iStopProcessingInput = OMX_TRUE;
            iLastInputFrame = iFrameNum;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameMp3() - ERROR, EmptyThisBuffer failed, OUT"));
            return OMX_ErrorNone;
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameMp3() - ERROR, Input is not ready to be sent, OUT"));
        return OMX_ErrorInsufficientResources;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::GetInputFrameMp3() - OUT"));
    return OMX_ErrorNone;
}



OMX_ERRORTYPE OmxComponentDecTest::GetInputFrameWma()
{
    OMX_S32 Index;
    OMX_S32 Size;
    OMX_ERRORTYPE Status;
    OMX_S32 ReadCount;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::GetInputFrameWma() - IN"));

    if (OMX_TRUE == iInputReady)
    {
        if (OMX_TRUE == iInputWasRefused)
        {
            Index = iInIndex; //use previously found and saved Index, do not read the file (this was done earlier)
            iInputWasRefused = OMX_FALSE; // reset the flag
        }
        else
        {
            Index = 0;
            while (OMX_FALSE == ipInputAvail[Index] && Index < iInBufferCount)
            {
                Index++;
            }

            if (iInBufferCount == Index)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::GetInputFrameWma() - Input buffer not available, OUT"));
                return OMX_ErrorNone;
            }


            if (iHeaderSent)
            {
                if (!iFragmentInProgress)
                {
                    ReadCount = fread(&Size, 1, FRAME_SIZE_FIELD, ipInputFile); // read in 1st 4 bytes
                    iCurFrag = 0;

                    if (ReadCount < FRAME_SIZE_FIELD)
                    {
                        iStopProcessingInput = OMX_TRUE;
                        iInIndex = Index;
                        iLastInputFrame = iFrameNum;

                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameWma() - Input file has ended, OUT"));
                        return OMX_ErrorNone;
                    }

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::GetInputFrameWma() - Next frame size is %d", Size));

                    for (OMX_S32 kk = 0; kk < iNumSimFrags - 1; kk++)
                    {
                        iSimFragSize[kk] = Size / iNumSimFrags;
                    }

                    iSimFragSize[iNumSimFrags - 1] = Size - (iNumSimFrags - 1) * (Size / iNumSimFrags); // last fragment
                }

                Size = iSimFragSize[iCurFrag];
                iCurFrag++;

                if (iCurFrag == iNumSimFrags)
                {
                    iFragmentInProgress = OMX_FALSE;
                }
                else
                {
                    iFragmentInProgress = OMX_TRUE;
                }

                ReadCount = fread(ipInBuffer[Index]->pBuffer, 1, Size, ipInputFile);

                if (ReadCount < Size)
                {
                    iStopProcessingInput = OMX_TRUE;
                    iInIndex = Index;
                    iLastInputFrame = iFrameNum;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameWma() - Input file read error, OUT"));

                    return OMX_ErrorNone;   // corrupted file !!
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameWma() - Bytes read from input file %d", Size));

                ipInBuffer[Index]->nFilledLen = Size;
                ipInBuffer[Index]->nOffset = 0;  // for now all data starts at the beginning of the buffer

            }
            else
            {
                ReadCount = fread(&Size, 1, FRAME_SIZE_FIELD, ipInputFile); // read in 1st 4 bytes
                if (ReadCount < FRAME_SIZE_FIELD)
                {
                    iStopProcessingInput = OMX_TRUE;
                    iInIndex = Index;
                    iLastInputFrame = iFrameNum;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameWma() - Input file has ended, OUT"));
                    return OMX_ErrorNone;
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::GetInputFrameWma() - Next frame size is %d", Size));

                ReadCount = fread(ipInBuffer[Index]->pBuffer, 1, Size, ipInputFile);
                if (ReadCount < Size)
                {
                    iStopProcessingInput = OMX_TRUE;
                    iInIndex = Index;
                    iLastInputFrame = iFrameNum;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameWma() - Input file read error, OUT"));
                    return OMX_ErrorNone;   // corrupted file !!
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameWma() - Bytes read from input file %d", Size));

                ipInBuffer[Index]->nFilledLen = Size;
                ipInBuffer[Index]->nOffset = 0;

                if (1 == iFrameNum)
                {
                    iHeaderSent = OMX_TRUE;
                }
            }

            if (iFragmentInProgress)
            {
                ipInBuffer[Index]->nFlags  = 0;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameWma() - Input buffer of index %d without any marker", Index));
            }
            else
            {
                iFrameNum++;
                ipInBuffer[Index]->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::GetInputFrameWma() - Input buffer of index %d with OMX_BUFFERFLAG_ENDOFFRAME flag marked", Index));
            }
        }


        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "OmxComponentDecTest::GetInputFrameWma() - Calling EmptyThisBuffer with Alloc Len %d, Filled Len %d Offset %d TimeStamp %d",
                         ipInBuffer[Index]->nAllocLen, ipInBuffer[Index]->nFilledLen, ipInBuffer[Index]->nOffset, ipInBuffer[Index]->nTimeStamp));

        ipInBuffer[Index]->nTimeStamp = 0;
        Status = OMX_EmptyThisBuffer(ipAppPriv->Handle, ipInBuffer[Index]);
        if (OMX_ErrorNone == Status)
        {
            ipInputAvail[Index] = OMX_FALSE; // mark unavailable

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxComponentDecTest::GetInputFrameWma() - Sent the input buffer sucessfully, OUT"));
            return OMX_ErrorNone;
        }
        else if (OMX_ErrorInsufficientResources == Status)
        {
            iInIndex = Index; // save the current input buffer, wait for call back
            iInputReady = OMX_FALSE;
            iInputWasRefused = OMX_TRUE;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameWma() - EmptyThisBuffer failed with Error OMX_ErrorInsufficientResources, OUT"));
            return OMX_ErrorInsufficientResources;
        }
        else
        {
            iStopProcessingInput = OMX_TRUE;
            iLastInputFrame = iFrameNum;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameWma() - ERROR, EmptyThisBuffer failed, OUT"));
            return OMX_ErrorNone;
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::GetInputFrameWma() - ERROR, Input is not ready to be sent, OUT"));
        return OMX_ErrorInsufficientResources;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::GetInputFrameWma() - OUT"));
    return OMX_ErrorNone;
}



bool OmxComponentDecTest::WriteOutput(OMX_U8* aOutBuff, OMX_U32 aSize)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::WriteOutput() called, Num of bytes %d", aSize));

    OMX_S32 BytesWritten = fwrite(aOutBuff, sizeof(OMX_U8), aSize, ipOutputFile);
    return (BytesWritten == aSize);
}


/*
 * Active Object class's Run () function
 * Control all the states of AO & sends openmax API's to the component
 */
static OMX_BOOL DisableRun = OMX_FALSE;

void OmxComponentDecTest::Run()
{
    switch (iState)
    {
        case StateUnLoaded:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::Run() - StateUnLoaded IN"));

            OMX_ERRORTYPE Err;
            OMX_U32 PortIndex, ii;

            if (!iCallbacks->initCallbacks())
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::Run() - ERROR initCallbacks failed, OUT"));
                StopOnError();
                break;
            }

            ipAppPriv = (AppPrivateType*) oscl_malloc(sizeof(AppPrivateType));
            CHECK_MEM(ipAppPriv, "Component_Handle");

            //Allocate bitstream buffer for AVC component
            if (0 == oscl_strcmp(iFormat, "H264"))
            {
                ipAVCBSO = OSCL_NEW(AVCBitstreamObject, (ipInputFile));
                CHECK_MEM(ipAVCBSO, "Bitstream_Buffer");
            }

            //Allocate bitstream buffer for MPEG4/H263 component
            if (0 == oscl_strcmp(iFormat, "M4V") || 0 == oscl_strcmp(iFormat, "H263"))
            {
                ipBitstreamBuffer = (OMX_U8*) oscl_malloc(BIT_BUFF_SIZE);
                CHECK_MEM(ipBitstreamBuffer, "Bitstream_Buffer")
                ipBitstreamBufferPointer = ipBitstreamBuffer;
            }

            //Allocate bitstream buffer for MP3 component
            if (0 == oscl_strcmp(iFormat, "MP3"))
            {
                ipMp3Bitstream = OSCL_NEW(Mp3BitstreamObject, (ipInputFile));
                CHECK_MEM(ipMp3Bitstream, "Bitstream_Buffer");
            }

            //This should be the first call to the component to load it.
            Err = OMX_Init();
            CHECK_ERROR(Err, "OMX_Init");
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::Run() - OMX_Init done"));


            if (NULL != iName)
            {
                Err = OMX_GetHandle(&ipAppPriv->Handle, iName, (OMX_PTR) this , iCallbacks->getCallbackStruct());
                CHECK_ERROR(Err, "GetHandle");
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::Run() - Got Handle for the component %s", iName));
            }
            else if (NULL != iRole)
            {
                //Determine the component first & then get the handle
                OMX_U32 NumComps = 0;
                OMX_STRING* pCompOfRole;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::Run() - Finding out the components that can support the role %s", iRole));

                // call once to find out the number of components that can fit the role
                Err = OMX_GetComponentsOfRole(iRole, &NumComps, NULL);

                if (OMX_ErrorNone != Err || NumComps < 1)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::Run() - ERROR, No component can handle the specified role %s", iRole));
                    StopOnError();
                    ipAppPriv->Handle = NULL;
                    break;
                }

                pCompOfRole = (OMX_STRING*) oscl_malloc(NumComps * sizeof(OMX_STRING));
                CHECK_MEM(pCompOfRole, "ComponentRoleArray");

                for (ii = 0; ii < NumComps; ii++)
                {
                    pCompOfRole[ii] = (OMX_STRING) oscl_malloc(PV_OMX_MAX_COMPONENT_NAME_LENGTH * sizeof(OMX_U8));
                    CHECK_MEM(pCompOfRole[ii], "ComponentRoleArray");
                }

                if (StateError == iState)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "OmxComponentDecTest::Run() - Error occured in this state, StateUnLoaded OUT"));
                    RunIfNotReady();
                    break;
                }

                // call 2nd time to get the component names
                Err = OMX_GetComponentsOfRole(iRole, &NumComps, (OMX_U8**) pCompOfRole);
                CHECK_ERROR(Err, "GetComponentsOfRole");

                for (ii = 0; ii < NumComps; ii++)
                {
                    // try to create component
                    Err = OMX_GetHandle(&ipAppPriv->Handle, (OMX_STRING) pCompOfRole[ii], (OMX_PTR) this, iCallbacks->getCallbackStruct());
                    // if successful, no need to continue
                    if ((OMX_ErrorNone == Err) && (NULL != ipAppPriv->Handle))
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::Run() - Got Handle for the component %s", pCompOfRole[ii]));
                        break;
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::Run() - ERROR, Cannot get component %s handle, try another if possible", pCompOfRole[ii]));
                    }

                }

                // whether successful or not, need to free CompOfRoles
                for (ii = 0; ii < NumComps; ii++)
                {
                    oscl_free(pCompOfRole[ii]);
                    pCompOfRole[ii] = NULL;
                }
                oscl_free(pCompOfRole);
                pCompOfRole = NULL;

                // check if there was a problem
                CHECK_ERROR(Err, "GetHandle");
                CHECK_MEM(ipAppPriv->Handle, "ComponentHandle");

            }

            //This will initialize the size and version of the iPortInit structure
            INIT_GETPARAMETER_STRUCT(OMX_PORT_PARAM_TYPE, iPortInit);

            if (0 == oscl_strcmp(iFormat, "AAC") || 0 == oscl_strcmp(iFormat, "AMR")
                    || 0 == oscl_strcmp(iFormat, "MP3") || 0 == oscl_strcmp(iFormat, "WMA"))
            {
                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamAudioInit, &iPortInit);
            }
            else
            {
                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamVideoInit, &iPortInit);
            }

            CHECK_ERROR(Err, "GetParameter_Audio/Video_Init");

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::Run() - GetParameter called for OMX_IndexParamAudioInit/OMX_IndexParamVideoInit"));

            for (ii = 0; ii < iPortInit.nPorts; ii++)
            {
                PortIndex = iPortInit.nStartPortNumber + ii;

                //This will initialize the size and version of the iParamPort structure
                INIT_GETPARAMETER_STRUCT(OMX_PARAM_PORTDEFINITIONTYPE, iParamPort);
                iParamPort.nPortIndex = PortIndex;

                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamPortDefinition, &iParamPort);
                CHECK_ERROR(Err, "GetParameter_IndexParamPortDefinition");
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::Run() - GetParameter called for OMX_IndexParamPortDefinition on port %d", PortIndex));

                if (0 == iParamPort.nBufferCountMin)
                {
                    /* a buffer count of 0 is not allowed */
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::Run() - Error, GetParameter for OMX_IndexParamPortDefinition returned 0 min buffer count"));
                    StopOnError();
                    break;
                }

                if (iParamPort.nBufferCountMin > iParamPort.nBufferCountActual)
                {
                    /* Min buff count can't be more than actual buff count */
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "OmxComponentDecTest::Run() - ERROR, GetParameter for OMX_IndexParamPortDefinition returned actual buffer count %d less than min buffer count %d", iParamPort.nBufferCountActual, iParamPort.nBufferCountMin));
                    StopOnError();
                    break;
                }

                if (OMX_DirInput == iParamPort.eDir)
                {
                    iInputPortIndex = PortIndex;

                    iInBufferSize = iParamPort.nBufferSize;
                    iInBufferCount = iParamPort.nBufferCountActual;
                    iParamPort.nBufferCountActual = iInBufferCount;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxComponentDecTest::Run() - GetParameter returned Num of input buffers %d with Size %d", iInBufferCount, iInBufferSize));

                }
                else if (OMX_DirOutput == iParamPort.eDir)
                {
                    iOutputPortIndex = PortIndex;

                    iOutBufferSize = iParamPort.nBufferSize;
                    iOutBufferCount = iParamPort.nBufferCountActual;

                    iParamPort.nBufferCountActual = iOutBufferCount;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxComponentDecTest::Run() - GetParameter returned Num of output buffers %d with Size %d", iOutBufferCount, iOutBufferSize));
                }

                //Take the buffer parameters of what component has specified
                iParamPort.nPortIndex = PortIndex;
                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamPortDefinition, &iParamPort);
                CHECK_ERROR(Err, "SetParameter_OMX_IndexParamPortDefinition");
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::Run() - SetParameter called for OMX_IndexParamPortDefinition on port %d", PortIndex));
            }

            if (StateError == iState)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "OmxComponentDecTest::Run() - Error, Exiting the test case, OUT"));
                RunIfNotReady();
                break;
            }

#if PROXY_INTERFACE
            ipThreadSafeHandlerEventHandler = OSCL_NEW(EventHandlerThreadSafeCallbackAO, (this, EVENT_HANDLER_QUEUE_DEPTH, "EventHandlerAO"));
            ipThreadSafeHandlerEmptyBufferDone = OSCL_NEW(EmptyBufferDoneThreadSafeCallbackAO, (this, iInBufferCount, "EmptyBufferDoneAO"));
            ipThreadSafeHandlerFillBufferDone = OSCL_NEW(FillBufferDoneThreadSafeCallbackAO, (this, iOutBufferCount, "FillBufferDoneAO"));

            if ((NULL == ipThreadSafeHandlerEventHandler) ||
                    (NULL == ipThreadSafeHandlerEmptyBufferDone) ||
                    (NULL == ipThreadSafeHandlerFillBufferDone))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "OmxComponentDecTest::Run() - Error, ThreadSafe Callback Handler initialization failed, OUT"));

                iState = StateUnLoaded;
                OsclExecScheduler* sched = OsclExecScheduler::Current();
                sched->StopScheduler();
            }
#endif
            //Decide about the getinput call for the correct compoent

            if (0 == oscl_strcmp(iFormat, "H264"))
            {
                pGetInputFrame = &OmxComponentDecTest::GetInputFrameAvc;
            }
            else if (0 == oscl_strcmp(iFormat, "M4V"))
            {
                pGetInputFrame = &OmxComponentDecTest::GetInputFrameMpeg4;
            }
            else if (0 == oscl_strcmp(iFormat, "H263"))
            {
                pGetInputFrame = &OmxComponentDecTest::GetInputFrameH263;
            }
            else if (0 == oscl_strcmp(iFormat, "AAC"))
            {
                pGetInputFrame = &OmxComponentDecTest::GetInputFrameAac;

                //iPcmMode.nSize = sizeof (OMX_AUDIO_PARAM_PCMMODETYPE);
                INIT_GETPARAMETER_STRUCT(OMX_AUDIO_PARAM_PCMMODETYPE, iPcmMode);
                iPcmMode.nPortIndex = iOutputPortIndex;

                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamAudioPcm, &iPcmMode);
                CHECK_ERROR(Err, "GetParameter_AAC_IndexParamAudioPcm");
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::Run() - GetParameter called for OMX_IndexParamAudioPcm for AAC on port %d", iOutputPortIndex));

                iPcmMode.nPortIndex = iOutputPortIndex;
                /* Pass the number of channel information for AAC component
                 * from input arguments to the component */
                iPcmMode.nChannels = iNumberOfChannels;
                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamAudioPcm, &iPcmMode);
                CHECK_ERROR(Err, "SetParameter_AAC_IndexParamAudioPcm");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::Run() - SetParameter called for OMX_IndexParamAudioPcm for AAC on port %d", iOutputPortIndex));
            }
            else if (0 == oscl_strcmp(iFormat, "WMV"))
            {
                pGetInputFrame = &OmxComponentDecTest::GetInputFrameWmv;
            }
            else if (0 == oscl_strcmp(iFormat, "AMR"))
            {
                /* Determine the file format type for AMR file */

                OMX_S32 AmrFileByte = 0;

                fread(&AmrFileByte, 1, 4, ipInputFile); // read in 1st 4 bytes

                if (2 == AmrFileByte)
                {
                    iAmrFileType = OMX_AUDIO_AMRFrameFormatFSF;
                }
                else if (4 == AmrFileByte)
                {
                    iAmrFileType = OMX_AUDIO_AMRFrameFormatIF2;
                }
                else if (6 == AmrFileByte)
                {
                    iAmrFileType = OMX_AUDIO_AMRFrameFormatRTPPayload;
                }
                else if (0 == AmrFileByte)
                {
                    iAmrFileType = OMX_AUDIO_AMRFrameFormatConformance;
                }
                else if (7 == AmrFileByte)
                {
                    iAmrFileType = OMX_AUDIO_AMRFrameFormatRTPPayload;
                    iAmrFileMode = OMX_AUDIO_AMRBandModeWB0;

                }
                else if (8 == AmrFileByte)
                {
                    iAmrFileType = OMX_AUDIO_AMRFrameFormatFSF;
                    iAmrFileMode = OMX_AUDIO_AMRBandModeWB0;
                }
                else
                {
                    /* Invalid input format type */
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "OmxComponentDecTest::Run() - Error, Invalid AMR input format %d, OUT", AmrFileByte));
                    StopOnError();
                }


                pGetInputFrame = &OmxComponentDecTest::GetInputFrameAmr;

                /* Pass the input format type information to the AMR component*/

                //iAmrParam.nSize = sizeof (OMX_AUDIO_PARAM_AMRTYPE);
                INIT_GETPARAMETER_STRUCT(OMX_AUDIO_PARAM_AMRTYPE, iAmrParam);
                iAmrParam.nPortIndex = iInputPortIndex;

                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamAudioAmr, &iAmrParam);
                CHECK_ERROR(Err, "GetParameter_AMR_IndexParamAudioAmr");
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::Run() - GetParameter called for OMX_IndexParamAudioAmr for AMR on port %d", iInputPortIndex));

                iAmrParam.nPortIndex = iInputPortIndex;
                iAmrParam.eAMRFrameFormat = iAmrFileType;
                iAmrParam.eAMRBandMode = iAmrFileMode;

                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamAudioAmr, &iAmrParam);
                CHECK_ERROR(Err, "SetParameter_AMR_IndexParamAudioAmr");
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::Run() - SetParameter called for OMX_IndexParamAudioAmr for AMR on port %d", iInputPortIndex));

            }
            else if (0 == oscl_strcmp(iFormat, "MP3"))
            {
                pGetInputFrame = &OmxComponentDecTest::GetInputFrameMp3;

                //iPcmMode.nSize = sizeof (OMX_AUDIO_PARAM_PCMMODETYPE);
                INIT_GETPARAMETER_STRUCT(OMX_AUDIO_PARAM_PCMMODETYPE, iPcmMode);
                iPcmMode.nPortIndex = iOutputPortIndex;

                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamAudioPcm, &iPcmMode);
                CHECK_ERROR(Err, "GetParameter_MP3_IndexParamAudioPcm");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::Run() - GetParameter called for OMX_IndexParamAudioPcm for MP3 on port %d", iOutputPortIndex));

                iPcmMode.nPortIndex = iOutputPortIndex;
                /* Pass the number of channel information from input arguments to the component */
                iPcmMode.nChannels = iNumberOfChannels;
                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamAudioPcm, &iPcmMode);
                CHECK_ERROR(Err, "SetParameter_MP3_IndexParamAudioPcm");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::Run() - SetParameter called for OMX_IndexParamAudioPcm for MP3 on port %d", iOutputPortIndex));
            }
            else if (0 == oscl_strcmp(iFormat, "WMA"))
            {
                pGetInputFrame = &OmxComponentDecTest::GetInputFrameWma;
            }


            if (StateError != iState)
            {
                iState = StateLoaded;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::Run() - StateUnLoaded OUT, moving to next state"));
            }

            RunIfNotReady();
        }
        break;

        case StateLoaded:
        {
            OMX_ERRORTYPE Err;
            OMX_S32 ii;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::Run() - StateLoaded IN"));

            // allocate memory for ipInBuffer
            ipInBuffer = (OMX_BUFFERHEADERTYPE**) oscl_malloc(sizeof(OMX_BUFFERHEADERTYPE*) * iInBufferCount);
            CHECK_MEM(ipInBuffer, "InputBufferHeader");

            ipInputAvail = (OMX_BOOL*) oscl_malloc(sizeof(OMX_BOOL) * iInBufferCount);
            CHECK_MEM(ipInputAvail, "InputBufferFlag");

            /* Initialize all the buffers to NULL */
            for (ii = 0; ii < iInBufferCount; ii++)
            {
                ipInBuffer[ii] = NULL;
            }

            //allocate memory for output buffer
            ipOutBuffer = (OMX_BUFFERHEADERTYPE**) oscl_malloc(sizeof(OMX_BUFFERHEADERTYPE*) * iOutBufferCount);
            CHECK_MEM(ipOutBuffer, "OutputBuffer");

            ipOutReleased = (OMX_BOOL*) oscl_malloc(sizeof(OMX_BOOL) * iOutBufferCount);
            CHECK_MEM(ipOutReleased, "OutputBufferFlag");

            /* Initialize all the buffers to NULL */
            for (ii = 0; ii < iOutBufferCount; ii++)
            {
                ipOutBuffer[ii] = NULL;
            }

            Err = OMX_SendCommand(ipAppPriv->Handle, OMX_CommandStateSet, OMX_StateIdle, NULL);
            CHECK_ERROR(Err, "SendCommand Loaded->Idle");

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxComponentDecTest::Run() - Sent State Transition Command from Loaded->Idle"));

            iPendingCommands = 1;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxComponentDecTest::Run() - Allocating %d input and %d output buffers", iInBufferCount, iOutBufferCount));

            //These calls are required because the control of in & out buffer should be with the testapp.
            for (ii = 0; ii < iInBufferCount; ii++)
            {
                Err = OMX_AllocateBuffer(ipAppPriv->Handle, &ipInBuffer[ii], iInputPortIndex, NULL, iInBufferSize);
                CHECK_ERROR(Err, "AllocateBuffer_Input");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::Run() - Called AllocateBuffer for buffer index %d on port %d", ii, iInputPortIndex));

                ipInputAvail[ii] = OMX_TRUE;
                ipInBuffer[ii]->nInputPortIndex = iInputPortIndex;
            }

            if (StateError == iState)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "OmxComponentDecTest::Run() - AllocateBuffer Error, StateLoaded OUT"));
                RunIfNotReady();
                break;
            }

            for (ii = 0; ii < iOutBufferCount; ii++)
            {
                Err = OMX_AllocateBuffer(ipAppPriv->Handle, &ipOutBuffer[ii], iOutputPortIndex, NULL, iOutBufferSize);
                CHECK_ERROR(Err, "AllocateBuffer_Output");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::Run() - Called AllocateBuffer for buffer index %d on port %d", ii, iOutputPortIndex));

                ipOutReleased[ii] = OMX_TRUE;

                ipOutBuffer[ii]->nOutputPortIndex = iOutputPortIndex;
                ipOutBuffer[ii]->nInputPortIndex = 0;
            }

            if (StateError == iState)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "OmxComponentDecTest::Run() - AllocateBuffer Error, StateLoaded OUT"));
                RunIfNotReady();
                break;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::Run() - StateLoaded OUT, Moving to next state"));

        }
        break;

        case StateIdle:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::Run() - StateIdle IN"));

            OMX_ERRORTYPE Err = OMX_ErrorNone;

            /*Send an output buffer before dynamic reconfig */
            Err = OMX_FillThisBuffer(ipAppPriv->Handle, ipOutBuffer[0]);
            CHECK_ERROR(Err, "FillThisBuffer");

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxComponentDecTest::Run() - FillThisBuffer command called for initiating dynamic port reconfiguration"));

            ipOutReleased[0] = OMX_FALSE;

            Err = OMX_SendCommand(ipAppPriv->Handle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
            CHECK_ERROR(Err, "SendCommand Idle->Executing");

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxComponentDecTest::Run() - Sent State Transition Command from Idle->Executing"));

            iPendingCommands = 1;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::Run() - StateIdle OUT"));
        }
        break;

        case StateDecodeHeader:
        {
            static OMX_BOOL FlagTemp = OMX_FALSE;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "OmxComponentDecTest::Run() - StateDecodeHeader IN, Sending configuration input buffers to the component to start dynamic port reconfiguration"));

            if (!FlagTemp)
            {
                (*this.*pGetInputFrame)();
                //For AAC component , send one more frame apart from the config frame, so that we can receive the callback
                if (0 == oscl_strcmp(iFormat, "AAC") || 0 == oscl_strcmp(iFormat, "AMR"))
                {
                    (*this.*pGetInputFrame)();
                }
                FlagTemp = OMX_TRUE;

                //Proceed to executing state and if Port settings changed callback comes,
                //then do the dynamic port reconfiguration
                iState = StateExecuting;

                RunIfNotReady();
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::Run() - StateDecodeHeader OUT"));
        }
        break;

        case StateDisablePort:
        {
            static OMX_BOOL FlagTemp = OMX_FALSE;
            OMX_ERRORTYPE Err = OMX_ErrorNone;
            OMX_S32 ii;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::Run() - StateDisablePort IN"));

            if (!DisableRun)
            {
                if (!FlagTemp)
                {
                    Err = OMX_SendCommand(ipAppPriv->Handle, OMX_CommandPortDisable, iOutputPortIndex, NULL);
                    CHECK_ERROR(Err, "SendCommand_PortDisable");

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxComponentDecTest::Run() - Sent Command for OMX_CommandPortDisable on port %d as a part of dynamic port reconfiguration", iOutputPortIndex));

                    iPendingCommands = 1;
                    FlagTemp = OMX_TRUE;
                    RunIfNotReady();
                }
                else
                {
                    //Wait for all the buffers to be returned on output port before freeing them
                    //This wait is required because of the queueing delay in all the Callbacks
                    for (ii = 0; ii < iOutBufferCount; ii++)
                    {
                        if (OMX_FALSE == ipOutReleased[ii])
                        {
                            break;
                        }
                    }

                    if (ii != iOutBufferCount)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                        (0, "OmxComponentDecTest::Run() - Not all the output buffers returned by component yet, wait for it"));
                        RunIfNotReady();
                        break;
                    }

                    for (ii = 0; ii < iOutBufferCount; ii++)
                    {
                        if (ipOutBuffer[ii])
                        {
                            Err = OMX_FreeBuffer(ipAppPriv->Handle, iOutputPortIndex, ipOutBuffer[ii]);
                            CHECK_ERROR(Err, "FreeBuffer_Output_DynamicReconfig");
                            ipOutBuffer[ii] = NULL;

                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                            (0, "OmxComponentDecTest::Run() - Called FreeBuffer for buffer index %d on port %d", ii, iOutputPortIndex));
                        }
                    }

                    if (StateError == iState)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "OmxComponentDecTest::Run() - Error occured in this state, StateDisablePort OUT"));
                        RunIfNotReady();
                        break;
                    }
                    DisableRun = OMX_TRUE;
                }
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::Run() - StateDisablePort OUT"));
        }
        break;

        case StateDynamicReconfig:
        {
            OMX_ERRORTYPE Err = OMX_ErrorNone;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::Run() - StateDynamicReconfig IN"));

            INIT_GETPARAMETER_STRUCT(OMX_PARAM_PORTDEFINITIONTYPE, iParamPort);
            iParamPort.nPortIndex = iOutputPortIndex;
            Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamPortDefinition, &iParamPort);
            CHECK_ERROR(Err, "GetParameter_DynamicReconfig");

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentDecTest::Run() - GetParameter called for OMX_IndexParamPortDefinition on port %d", iParamPort.nPortIndex));

            Err = OMX_SendCommand(ipAppPriv->Handle, OMX_CommandPortEnable, iOutputPortIndex, NULL);
            CHECK_ERROR(Err, "SendCommand_PortEnable");

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxComponentDecTest::Run() - Sent Command for OMX_CommandPortEnable on port %d as a part of dynamic port reconfiguration", iOutputPortIndex));

            iPendingCommands = 1;

            if (0 == oscl_strcmp(iFormat, "H264") || 0 == oscl_strcmp(iFormat, "H263")
                    || 0 == oscl_strcmp(iFormat, "M4V"))
            {
                iOutBufferSize = ((iParamPort.format.video.nFrameWidth + 15) & ~15) * ((iParamPort.format.video.nFrameHeight + 15) & ~15) * 3 / 2;
            }
            else if (0 == oscl_strcmp(iFormat, "WMV"))
            {
                iOutBufferSize = ((iParamPort.format.video.nFrameWidth + 3) & ~3) * ((iParamPort.format.video.nFrameHeight + 3) & ~3) * 3 / 2;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxComponentDecTest::Run() - Allocating buffer again after port reconfigutauion has been complete"));

            for (OMX_S32 ii = 0; ii < iOutBufferCount; ii++)
            {
                Err = OMX_AllocateBuffer(ipAppPriv->Handle, &ipOutBuffer[ii], iOutputPortIndex, NULL, iOutBufferSize);
                CHECK_ERROR(Err, "AllocateBuffer_Output_DynamicReconfig");
                ipOutReleased[ii] = OMX_TRUE;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::Run() - AllocateBuffer called for buffer index %d on port %d", ii, iOutputPortIndex));
            }

            if (StateError == iState)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "OmxComponentDecTest::Run() - Error occured in this state, StateDynamicReconfig OUT"));
                RunIfNotReady();
                break;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::Run() - StateDynamicReconfig OUT"));
        }
        break;

        case StateExecuting:
        {
            static OMX_BOOL EosFlag = OMX_FALSE;
            static OMX_ERRORTYPE Status;
            OMX_S32 Index;
            OMX_BOOL MoreOutput;
            OMX_ERRORTYPE Err = OMX_ErrorNone;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::Run() - StateExecuting IN"));

            MoreOutput = OMX_TRUE;
            while (MoreOutput)
            {
                Index = 0;
                while (OMX_FALSE == ipOutReleased[Index] && Index < iOutBufferCount)
                {
                    Index++;
                }

                if (Index != iOutBufferCount)
                {
                    //This call is being made only once per frame
                    Err = OMX_FillThisBuffer(ipAppPriv->Handle, ipOutBuffer[Index]);
                    CHECK_ERROR(Err, "FillThisBuffer");
                    //Make this flag OMX_TRUE till u receive the callback for output buffer free
                    ipOutReleased[Index] = OMX_FALSE;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxComponentDecTest::Run() - FillThisBuffer command called for output buffer index %d", Index));
                }
                else
                {
                    MoreOutput = OMX_FALSE;
                }
            }


            if (!iStopProcessingInput || (OMX_ErrorInsufficientResources == Status))
            {
                // find available input buffer
                Index = 0;
                while (OMX_FALSE == ipInputAvail[Index] && Index < iInBufferCount)
                {
                    Index++;
                }

                if (Index != iInBufferCount)
                {
                    Status = (*this.*pGetInputFrame)();
                }
            }
            else if (OMX_FALSE == EosFlag)
            {
                //Only send one successful dummy buffer with flag set to signal EOS
                Index = 0;
                while (OMX_FALSE == ipInputAvail[Index] && Index < iInBufferCount)
                {
                    Index++;
                }

                if (Index != iInBufferCount)
                {
                    ipInBuffer[Index]->nFlags |= OMX_BUFFERFLAG_EOS;
                    ipInBuffer[Index]->nFilledLen = 0;
                    Err = OMX_EmptyThisBuffer(ipAppPriv->Handle, ipInBuffer[Index]);

                    CHECK_ERROR(Err, "EmptyThisBuffer_EOS");

                    ipInputAvail[Index] = OMX_FALSE; // mark unavailable
                    EosFlag = OMX_TRUE;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxComponentDecTest::Run() - Input buffer sent to the component with OMX_BUFFERFLAG_EOS flag set"));
                }
            }
            else
            {
                //nothing to do here
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::Run() - StateExecuting OUT"));
            RunIfNotReady();
        }
        break;

        case StateStopping:
        {
            static OMX_BOOL FlagTemp = OMX_FALSE;
            OMX_ERRORTYPE Err = OMX_ErrorNone;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::Run() - StateStopping IN"));

            //stop execution by state transition to Idle state.
            if (!FlagTemp)
            {
                Err = OMX_SendCommand(ipAppPriv->Handle, OMX_CommandStateSet, OMX_StateIdle, NULL);
                CHECK_ERROR(Err, "SendCommand Executing->Idle");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::Run() - Sent State Transition Command from Executing->Idle"));

                iPendingCommands = 1;
                FlagTemp = OMX_TRUE;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::Run() - StateStopping OUT"));
        }
        break;

        case StateCleanUp:
        {
            OMX_S32 ii;
            OMX_ERRORTYPE Err = OMX_ErrorNone;
            static OMX_BOOL FlagTemp = OMX_FALSE;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::Run() - StateCleanUp IN"));


            if (!FlagTemp)
            {
                if (OMX_FALSE == VerifyAllBuffersReturned())
                {
                    // not all buffers have been returned yet, reschedule
                    RunIfNotReady();
                    break;
                }

                //Destroy the component by state transition to Loaded state
                Err = OMX_SendCommand(ipAppPriv->Handle, OMX_CommandStateSet, OMX_StateLoaded, NULL);
                CHECK_ERROR(Err, "SendCommand Idle->Loaded");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentDecTest::Run() - Sent State Transition Command from Idle->Loaded"));

                iPendingCommands = 1;

                if (ipInBuffer)
                {
                    for (ii = 0; ii < iInBufferCount; ii++)
                    {
                        if (ipInBuffer[ii])
                        {
                            Err = OMX_FreeBuffer(ipAppPriv->Handle, iInputPortIndex, ipInBuffer[ii]);
                            CHECK_ERROR(Err, "FreeBuffer_Input");
                            ipInBuffer[ii] = NULL;

                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                            (0, "OmxComponentDecTest::Run() - Called FreeBuffer for buffer index %d on port %d", ii, iInputPortIndex));
                        }
                    }

                    oscl_free(ipInBuffer);
                    ipInBuffer = NULL;
                }

                if (ipInputAvail)
                {
                    oscl_free(ipInputAvail);
                    ipInputAvail = NULL;
                }


                if (ipOutBuffer)
                {
                    for (ii = 0; ii < iOutBufferCount; ii++)
                    {
                        if (ipOutBuffer[ii])
                        {
                            Err = OMX_FreeBuffer(ipAppPriv->Handle, iOutputPortIndex, ipOutBuffer[ii]);
                            CHECK_ERROR(Err, "FreeBuffer_Output");
                            ipOutBuffer[ii] = NULL;

                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                            (0, "OmxComponentDecTest::Run() - Called FreeBuffer for buffer index %d on port %d", ii, iOutputPortIndex));
                        }
                    }
                    oscl_free(ipOutBuffer);
                    ipOutBuffer = NULL;

                }

                if (ipOutReleased)
                {
                    oscl_free(ipOutReleased);
                    ipOutReleased = NULL;
                }

                FlagTemp = OMX_TRUE;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::Run() - StateCleanUp OUT"));

        }
        break;


        /********* FREE THE HANDLE & CLOSE FILES FOR THE COMPONENT ********/
        case StateStop:
        {
            OMX_U8 TestName[] = "NORMAL_SEQ_TEST";
            OMX_ERRORTYPE Err = OMX_ErrorNone;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::Run() - StateStop IN"));

            if (ipAppPriv)
            {
                if (ipAppPriv->Handle)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxComponentDecTest::Run() - Free the Component Handle"));

                    Err = OMX_FreeHandle(ipAppPriv->Handle);
                    if (OMX_ErrorNone != Err)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::Run() - FreeHandle Error"));
                        iTestStatus = OMX_FALSE;
                    }
                }
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxComponentDecTest::Run() - De-initialize the omx component"));

            Err = OMX_Deinit();
            if (OMX_ErrorNone != Err)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentDecTest::Run() - OMX_Deinit Error"));
                iTestStatus = OMX_FALSE;
            }

            if (0 == oscl_strcmp(iFormat, "H264"))
            {
                if (ipAVCBSO)
                {
                    OSCL_DELETE(ipAVCBSO);
                    ipAVCBSO = NULL;
                }
            }

            if (0 == oscl_strcmp(iFormat, "M4V") || 0 == oscl_strcmp(iFormat, "H263"))
            {
                if (ipBitstreamBuffer)
                {
                    oscl_free(ipBitstreamBufferPointer);
                    ipBitstreamBuffer = NULL;
                    ipBitstreamBufferPointer = NULL;
                }
            }

            if (0 == oscl_strcmp(iFormat, "MP3"))
            {
                if (ipMp3Bitstream)
                {
                    OSCL_DELETE(ipMp3Bitstream);
                    ipMp3Bitstream = NULL;
                }
            }

            if (ipAppPriv)
            {
                oscl_free(ipAppPriv);
                ipAppPriv = NULL;
            }

#if PROXY_INTERFACE
            OSCL_DELETE(ipThreadSafeHandlerEventHandler);
            ipThreadSafeHandlerEventHandler = NULL;

            OSCL_DELETE(ipThreadSafeHandlerEmptyBufferDone);
            ipThreadSafeHandlerEmptyBufferDone = NULL;

            OSCL_DELETE(ipThreadSafeHandlerFillBufferDone);
            ipThreadSafeHandlerFillBufferDone = NULL;
#endif

            VerifyOutput(TestName);

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::Run() - StateStop OUT"));

            iState = StateUnLoaded;
            OsclExecScheduler* sched = OsclExecScheduler::Current();
            sched->StopScheduler();
        }
        break;

        case StateError:
        {
            //Do all the cleanup's and exit from here
            OMX_S32 ii;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::Run() - StateError IN"));

            iTestStatus = OMX_FALSE;

            if (ipInBuffer)
            {
                for (ii = 0; ii < iInBufferCount; ii++)
                {
                    if (ipInBuffer[ii])
                    {
                        OMX_FreeBuffer(ipAppPriv->Handle, iInputPortIndex, ipInBuffer[ii]);
                        ipInBuffer[ii] = NULL;

                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                        (0, "OmxComponentDecTest::Run() - Called FreeBuffer for buffer index %d on port %d", ii, iInputPortIndex));
                    }
                }
                oscl_free(ipInBuffer);
                ipInBuffer = NULL;
            }

            if (ipInputAvail)
            {
                oscl_free(ipInputAvail);
                ipInputAvail = NULL;
            }

            if (ipOutBuffer)
            {
                for (ii = 0; ii < iOutBufferCount; ii++)
                {
                    if (ipOutBuffer[ii])
                    {
                        OMX_FreeBuffer(ipAppPriv->Handle, iOutputPortIndex, ipOutBuffer[ii]);
                        ipOutBuffer[ii] = NULL;

                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                        (0, "OmxComponentDecTest::Run() - Called FreeBuffer for buffer index %d on port %d", ii, iOutputPortIndex));
                    }
                }
                oscl_free(ipOutBuffer);
                ipOutBuffer = NULL;
            }

            if (ipOutReleased)
            {
                oscl_free(ipOutReleased);
                ipOutReleased = NULL;
            }

            iState = StateStop;
            RunIfNotReady();

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::Run() - StateError OUT"));

        }
        break;

        default:
        {
            break;
        }
    }
    return ;
}


void OmxComponentDecTest::VerifyOutput(OMX_U8 aTestName[])
{
    FILE* pRefFilePointer = NULL;
    FILE* pOutFilePointer = NULL;
    OMX_S32 FileSize1, FileSize2, Count = 0;
    OMX_S16 Num1, Num2, Diff;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::VerifyOutput() IN"));

    //Check the output against the reference file specified
    fclose(ipOutputFile);

    if (('\0' != iRefFile[0]) && (OMX_TRUE == iTestStatus))
    {
        pRefFilePointer = fopen(iRefFile, "rb");
        pOutFilePointer = fopen(iOutFileName, "rb");

        if (NULL != pRefFilePointer && NULL != pOutFilePointer)
        {
            fseek(pOutFilePointer, 0, SEEK_END);
            FileSize1 = ftell(pOutFilePointer);
            rewind(pOutFilePointer);

            fseek(pRefFilePointer, 0, SEEK_END);
            FileSize2 = ftell(pRefFilePointer);
            rewind(pRefFilePointer);

            if (FileSize1 == FileSize2)
            {
                while (!feof(pOutFilePointer))
                {
                    fread(&Num1, 1, sizeof(OMX_S16), pOutFilePointer);
                    fread(&Num2, 1, sizeof(OMX_S16), pRefFilePointer);

                    Diff = OSCL_ABS(Num2 - Num1);
                    if (0 != Diff)
                    {
                        break;
                    }
                }

                if (0 != Diff)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                                    (0, "OmxComponentDecTest::VerifyOutput() - %s : Fail {Output not bit-exact}", aTestName));

#ifdef PRINT_RESULT
                    fprintf(iConsOutFile, "%s: Fail {Output not bit-exact}\n", aTestName);
#endif
                }
                else
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                                    (0, "OmxComponentDecTest::VerifyOutput() - %s : Success {Output Bit-exact}", aTestName));

#ifdef PRINT_RESULT
                    fprintf(iConsOutFile, "%s: Success {Output Bit-exact}\n", aTestName);
#endif
                }

            }
            else
            {
#ifdef PRINT_RESULT
                fprintf(iConsOutFile, "%s: Fail {Output not bit-exact}\n", aTestName);
#endif
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                                (0, "OmxComponentDecTest::VerifyOutput() - %s : Fail {Output not bit-exact}", aTestName));
            }
        }
        else
        {
#ifdef PRINT_RESULT
            fprintf(iConsOutFile, "Cannot open reference/output file for verification\n");
#endif
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "OmxComponentDecTest::VerifyOutput() - Cannot open reference/output file for verification"));

            if (OMX_FALSE == iTestStatus)
            {
#ifdef PRINT_RESULT
                fprintf(iConsOutFile, "%s: Fail \n", aTestName);
#endif

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                                (0, "OmxComponentDecTest::VerifyOutput() - %s : Fail", aTestName));
            }
            else
            {
#ifdef PRINT_RESULT
                fprintf(iConsOutFile, "%s: Success {Ref file not available} \n", aTestName);
#endif

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                                (0, "OmxComponentDecTest::VerifyOutput() - %s : Success {Ref file not available}", aTestName));
            }
        }
    }
    else
    {
        if ('\0' == iRefFile[0])
        {
#ifdef PRINT_RESULT
            fprintf(iConsOutFile, "Reference file not available\n");
#endif
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "OmxComponentDecTest::VerifyOutput() - Reference file not available"));
        }

        if (OMX_FALSE == iTestStatus)
        {
#ifdef PRINT_RESULT
            fprintf(iConsOutFile, "%s: Fail \n", aTestName);
#endif
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "OmxComponentDecTest::VerifyOutput() - %s : Fail", aTestName));
        }
        else
        {
#ifdef PRINT_RESULT
            fprintf(iConsOutFile, "%s: Success {Ref file not available} \n", aTestName);
#endif
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                            (0, "OmxComponentDecTest::VerifyOutput() - %s : Success {Ref file not available}", aTestName));
        }
    }

    if (pRefFilePointer)
    {
        fclose(pRefFilePointer);
    }
    if (pOutFilePointer)
    {
        fclose(pOutFilePointer);
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::VerifyOutput() OUT"));
    return;
}

void OmxComponentDecTest::StopOnError()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentDecTest::StopOnError() called"));
    iState = StateError;
    RunIfNotReady();
}


OMX_S32 AVCBitstreamObject::Refill()
{
    OMX_S32 RemainBytes = iActualSize - iPos;

    if (RemainBytes > 0)
    {
        // move the remaining stuff to the beginning of iBuffer
        oscl_memcpy(&ipBuffer[0], &ipBuffer[iPos], RemainBytes);
        iPos = 0;
    }

    // read data
    if ((iActualSize = fread(&ipBuffer[RemainBytes], 1, iBufferSize - RemainBytes, ipAVCFile)) == 0)
    {
        iActualSize += RemainBytes;
        iPos = 0;
        return -1;
    }

    iActualSize += RemainBytes;
    iPos = 0;
    return 0;
}


AVCOMX_Status AVCBitstreamObject::GetNextFullNAL(OMX_U8** aNalBuffer, OMX_S32* aNalSize)
{
    // First check if the remaining size of the buffer can hold one full NAL
    if (iActualSize - iPos < MIN_NAL_SIZE)
    {
        if (Refill())
        {
            return AVCOMX_FAIL;
        }
    }

    OMX_U8* pBuff = ipBuffer + iPos;

    *aNalSize = iActualSize - iPos;

    AVCOMX_Status RetVal = AVCAnnexBGetNALUnit(pBuff, aNalBuffer, (int32*) aNalSize);

    if (AVCOMX_FAIL == RetVal)
    {
        return AVCOMX_FAIL;
    }

    if (AVCOMX_NO_NEXT_SC == RetVal)
    {
        Refill();
        pBuff = ipBuffer + iPos;

        *aNalSize = iActualSize - iPos;
        RetVal = AVCAnnexBGetNALUnit(pBuff, aNalBuffer, (int32*) aNalSize);
        if (AVCOMX_FAIL == RetVal)
        {
            return AVCOMX_FAIL; // After refill(), the data should be enough
        }
    }

    iPos += ((*aNalSize) + (OMX_S32)(*aNalBuffer - pBuff));
    return AVCOMX_SUCCESS;
}

AVCOMX_Status AVCBitstreamObject::AVCAnnexBGetNALUnit(uint8 *bitstream, uint8 **nal_unit,
        int *size)
{
    int i, j, FoundStartCode = 0;
    int end;

    i = 0;
    while (bitstream[i] == 0 && i < *size)
    {
        i++;
    }
    if (i >= *size)
    {
        *nal_unit = bitstream;
        return AVCOMX_FAIL; /* cannot find any start_code_prefix. */
    }
    else if (bitstream[i] != 0x1)
    {
        i = -1;  /* start_code_prefix is not at the beginning, continue */
    }

    i++;
    *nal_unit = bitstream + i; /* point to the beginning of the NAL unit */

    j = end = i;
    while (!FoundStartCode)
    {
        while ((j + 1 < *size) && (bitstream[j] != 0 || bitstream[j+1] != 0))  /* see 2 consecutive zero bytes */
        {
            j++;
        }
        end = j;   /* stop and check for start code */
        while (j + 2 < *size && bitstream[j+2] == 0) /* keep reading for zero byte */
        {
            j++;
        }
        if (j + 2 >= *size)
        {
            *size -= i;
            return AVCOMX_NO_NEXT_SC;  /* cannot find the second start_code_prefix */
        }
        if (bitstream[j+2] == 0x1)
        {
            FoundStartCode = 1;
        }
        else
        {
            /* could be emulation code 0x3 */
            j += 2; /* continue the search */
        }
    }

    *size = end - i;

    return AVCOMX_SUCCESS;
}


void AVCBitstreamObject::ResetInputStream()
{
    iPos = 0;
    iActualSize = 0;
}



/* Bitstream read function for MP3 component. */
int32 Mp3BitstreamObject::DecodeReadInput()
{
    int32      possibleElements = 0;
    int32      actualElements;
    int32      start;

    start = iInputBufferCurrentLength;

    if (iInputBufferMaxLength > start)
    {
        possibleElements = iInputBufferMaxLength - start;
    }
    else if (start)
    {
        /*
         *  Synch search need to expand the seek over fresh data
         *  then increase the data read according to teh size of the size
         *  of the frame plus 2 extra bytes to read next sync word
         */
        iInputBufferMaxLength = iCurrentFrameLength;

        if (iInputBufferMaxLength > BIT_BUFF_SIZE_MP3)
        {
            iInputBufferMaxLength = BIT_BUFF_SIZE_MP3;
        }
        possibleElements = iInputBufferMaxLength - start;
    }

    actualElements = fread(&(ipBuffer[start]), sizeof(ipBuffer[0]),
                           possibleElements, ipMp3File);

    /*
     * Increment the number of valid array elements by the actual amount
     * read in.
     */

    if (!actualElements)
    {
        oscl_memset(&(ipBuffer[start]), 0,
                    possibleElements * sizeof(ipBuffer[0]));
    }

    iInputBufferCurrentLength +=  actualElements;

    return actualElements;
}


int32 Mp3BitstreamObject::DecodeAdjustInput()
{
    int32   start;
    int32   length;

    start = iInputBufferUsedLength;

    length = iInputBufferMaxLength - start;

    if (length > 0)
    {
        oscl_memmove(&(ipBuffer[0]),                 /* destination */
                     &(ipBuffer[start]),            /* source      */
                     length * sizeof(ipBuffer[0])); /* length      */
    }

    iInputBufferUsedLength = 0;
    iInputBufferCurrentLength -= start;

    if (iInputBufferCurrentLength < 0)
    {
        return 1;
    }

    return 0;
}


int32 Mp3BitstreamObject::Mp3FrameSynchronization(OMX_S32* aFrameSize)
{
    uint16 val;
    OMX_BOOL Err;
    int32 retval;
    int32 numBytes;

    pVars->pBuffer = ipBuffer;
    pVars->usedBits = (iInputBufferUsedLength << 3); // in bits
    pVars->inputBufferCurrentLength = (iInputBufferCurrentLength);
    Err = HeaderSync();

    if (OMX_TRUE == Err)
    {
        /* validate synchronization by checking two consecutive sync words
         * to avoid multiple bitstream accesses*/

        uint32 temp = GetNbits(21);
        // put back whole header
        pVars->usedBits -= 21 + SYNC_WORD_LENGTH_MP3;

        int32  version;

        switch (temp >> 19)
        {
            case 0:
                version = MPEG_2_5;
                break;
            case 2:
                version = MPEG_2;
                break;
            case 3:
                version = MPEG_1;
                break;
            default:
                version = INVALID_VERSION;
                break;
        }

        int32 freq_index = (temp << 20) >> 30;

        if (version != INVALID_VERSION && (freq_index != 3))
        {
            numBytes = (int32)(((int64)(Mp3Bitrate[version][(temp<<16)>>28] << 20) * InvFreq[freq_index]) >> 28);

            numBytes >>= (20 - version);

            if (version != MPEG_1)
            {
                numBytes >>= 1;
            }
            if ((temp << 22) >> 31)
            {
                numBytes++;
            }

            if (numBytes > (int32)pVars->inputBufferCurrentLength)
            {
                /* frame should account for padding and 2 bytes to check sync */
                iCurrentFrameLength = numBytes + 3;
                return 1;
            }
            else if (numBytes == (int32)pVars->inputBufferCurrentLength)
            {
                /* No enough data to validate, but current frame appears to be correct ( EOF case) */
                iInputBufferUsedLength = pVars->usedBits >> 3;
                *aFrameSize = numBytes;
                return 0;
            }
            else
            {
                int32 offset = pVars->usedBits + ((numBytes) << 3);

                offset >>= INBUF_ARRAY_INDEX_SHIFT;
                uint8    *pElem  = pVars->pBuffer + offset;
                uint16 tmp1 = *(pElem++);
                uint16 tmp2 = *(pElem);

                val = (tmp1 << 3);
                val |= (tmp2 >> 5);
            }
        }
        else
        {
            val = 0; // force mismatch
        }

        if (val == SYNC_WORD)
        {
            iInputBufferUsedLength = pVars->usedBits >> 3; ///  !!!!!
            *aFrameSize = numBytes;
            retval = 0;
        }
        else
        {
            iInputBufferCurrentLength = 0;
            *aFrameSize = 0;
            retval = 1;
        }
    }
    else
    {
        iInputBufferCurrentLength = 0;
    }

    return(retval);

}


OMX_BOOL Mp3BitstreamObject::HeaderSync()
{
    uint16 val;
    uint32    offset;
    uint32    bitIndex;
    uint8     Elem;
    uint8     Elem1;
    uint8     Elem2;
    uint32   returnValue;
    uint32 availableBits = (pVars->inputBufferCurrentLength << 3); // in bits

    // byte aligment
    pVars->usedBits = (pVars->usedBits + 7) & 8;

    offset = (pVars->usedBits) >> INBUF_ARRAY_INDEX_SHIFT;

    Elem  = *(pVars->pBuffer + offset);
    Elem1 = *(pVars->pBuffer + offset + 1);
    Elem2 = *(pVars->pBuffer + offset + 2);


    returnValue = (((uint32)(Elem)) << 16) |
                  (((uint32)(Elem1)) << 8) |
                  ((uint32)(Elem2));

    /* Remove extra high bits by shifting up */
    bitIndex = module(pVars->usedBits, INBUF_BIT_WIDTH);

    pVars->usedBits += SYNC_WORD_LENGTH_MP3;
    /* This line is faster than to mask off the high bits. */
    returnValue = 0xFFFFFF & (returnValue << (bitIndex));

    /* Move the field down. */

    val = (uint32)(returnValue >> (24 - SYNC_WORD_LENGTH_MP3));

    while (((val & SYNC_WORD) != SYNC_WORD) && (pVars->usedBits < availableBits))
    {
        val <<= 8;
        val |= GetUpTo9Bits(8);
    }

    if (SYNC_WORD == (val & SYNC_WORD))
    {
        return(OMX_TRUE);
    }
    else
    {
        return(OMX_FALSE);
    }
}

OMX_U32 Mp3BitstreamObject::GetNbits(OMX_S32 NeededBits)
{
    uint32    offset;
    uint32    bitIndex;
    uint8    *pElem;         /* Needs to be same type as pInput->pBuffer */
    uint8    *pElem1;
    uint8    *pElem2;
    uint8    *pElem3;
    uint32   returnValue = 0;

    if (!NeededBits)
    {
        return (returnValue);
    }

    offset = (pVars->usedBits) >> INBUF_ARRAY_INDEX_SHIFT;

    pElem  = pVars->pBuffer + offset;;
    pElem1 = pVars->pBuffer + offset + 1;
    pElem2 = pVars->pBuffer + offset + 2;
    pElem3 = pVars->pBuffer + offset + 3;


    returnValue = (((uint32) * (pElem)) << 24) |
                  (((uint32) * (pElem1)) << 16) |
                  (((uint32) * (pElem2)) << 8) |
                  ((uint32) * (pElem3));

    /* Remove extra high bits by shifting up */
    bitIndex = module(pVars->usedBits, INBUF_BIT_WIDTH);

    /* This line is faster than to mask off the high bits. */
    returnValue <<= bitIndex;

    /* Move the field down. */
    returnValue >>= 32 - NeededBits;

    pVars->usedBits += NeededBits;

    return (returnValue);
}

uint16 Mp3BitstreamObject::GetUpTo9Bits(int32 neededBits) /* number of bits to read from the bit stream 2 to 9 */
{

    uint32   offset;
    uint32   bitIndex;
    uint8    Elem;         /* Needs to be same type as pInput->pBuffer */
    uint8    Elem1;
    uint16   returnValue;

    offset = (pVars->usedBits) >> INBUF_ARRAY_INDEX_SHIFT;

    Elem  = *(pVars->pBuffer + offset);
    Elem1 = *(pVars->pBuffer + offset + 1);


    returnValue = (((uint16)(Elem)) << 8) |
                  ((uint16)(Elem1));

    /* Remove extra high bits by shifting up */
    bitIndex = module(pVars->usedBits, INBUF_BIT_WIDTH);

    pVars->usedBits += neededBits;
    /* This line is faster than to mask off the high bits. */
    returnValue = (returnValue << (bitIndex));

    /* Move the field down. */

    return (uint16)(returnValue >> (16 - neededBits));

}


void Mp3BitstreamObject::ResetInputStream()
{
    iInputBufferUsedLength = 0;
    iInputBufferCurrentLength = 0;
    iCurrentFrameLength = 0;
}
