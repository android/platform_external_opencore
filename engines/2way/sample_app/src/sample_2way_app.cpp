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
#ifndef SAMPLE_2WAY_APP_H_INCLUDED
#include "sample_2way_app.h"
#endif

#ifndef OSCL_MEM_H_INCLUDED
#include "oscl_mem.h"
#endif

#ifndef OSCL_MEM_AUDIT_H_INCLUDED
#include "oscl_mem_audit.h"
#endif

#ifndef OSCL_SCHEDULER_H_INCLUDED
#include "oscl_scheduler.h"
#endif

#ifndef OSCL_UTF8CONV_H
#include "oscl_utf8conv.h"
#endif

#ifndef PV_MP4_H263_ENC_EXTENSION_H_INCLUDED
#include "pvmp4h263encextension.h"
#endif

#ifndef PVMF_FILEOUTPUT_CONFIG_H_INCLUDED
#include "pvmf_fileoutput_config.h"
#endif

#ifndef PVMF_COMPOSER_SIZE_AND_DURATION_H_INCLUDED
#include "pvmf_composer_size_and_duration.h"
#endif

#ifndef PVMFAMRENCNODE_EXTENSION_H_INCLUDED
#include "pvmfamrencnode_extension.h"
#endif

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef PVMF_MEDIA_INPUT_NODE_FACTORY_H_INCLUDED
#include "pvmf_media_input_node_factory.h"
#endif

#ifndef PVMI_MIO_FILEINPUT_FACTORY_H_INCLUDED
#include "pvmi_mio_fileinput_factory.h"
#endif

#ifndef PV_COMMS_IO_NODE_FACTORY_H_INCLUDED
#include "pv_comms_io_node_factory.h"
#endif

#ifndef PV_MEDIA_OUTPUT_NODE_FACTORY_H_INCLUDED
#include "pv_media_output_node_factory.h"
#endif

// video at QCIF
#define KVideoFrameWidth        176
#define KVideoFrameHeight       144
// video at 15 frames per second
#define KVideoFrameRate         15
// video time scale in milliseconds
#define KVideoTimescale         1000

// Default file settings for audio and video source
#define KYUVTestInput           _STRLIT_WCHAR("../../../test_input/yuvtestinput.yuv")

// If PV_USE_AMR_CODECS is disabled in 2way/build/make/makefile, turn USE_IF2_FILE on
#define USE_IF2_FILE    1
#ifdef USE_IF2_FILE
#define KAudioTestInput         _STRLIT_WCHAR("../../../data/audio_in.if2")
#else
#define KAudioTestInput         _STRLIT_WCHAR("../../../test_input/pcm16testinput.pcm")
#endif

// Default audio input settings
#define KAudioTimescale         8000
#define KAudioNumChannels       1
// 10 20ms audio frames per chunk
#define KNum20msFramesPerChunk      1

// record duration default to 30 seconds
#define KRecordDuration         30000000

#define KAudioSinkFileName _STRLIT("../../../data/audio_out.dat")
#define KVideoSinkFileName _STRLIT("../../../data/video_out.dat")

// forward declaration
int _local_main();

// printing to console
FILE* file = stdout;

// Entry point for sample program
int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    fprintf(file, "\nSingle threaded sample application for pv2way engine\n");

    //Init Oscl
    OsclBase::Init();
    OsclErrorTrap::Init();
    OsclMem::Init();

    //Run the test under a trap
    int result = -1;
    int32 err;

    OSCL_TRY(err, result = _local_main(););

    //Show any exception.
    if (err != 0)
    {
        fprintf(file, "Error!  Leave %d\n", err);
    }

    //Cleanup
#if !(OSCL_BYPASS_MEMMGT)
    //Check for memory leaks before cleaning up OsclMem.
    OsclAuditCB auditCB;
    OsclMemInit(auditCB);
    if (auditCB.pAudit)
    {
        MM_Stats_t* stats = auditCB.pAudit->MM_GetStats("");
        if (stats)
        {
            fprintf(file, "\nAfter 2way Memory Stats:\n");
            fprintf(file, "  peakNumAllocs %d\n", stats->peakNumAllocs);
            fprintf(file, "  peakNumBytes %d\n", stats->peakNumBytes);
            fprintf(file, "  totalNumAllocs %d\n", stats->totalNumAllocs);
            fprintf(file, "  totalNumBytes %d\n", stats->totalNumBytes);
            fprintf(file, "  numAllocFails %d\n", stats->numAllocFails);
            if (stats->numAllocs)
            {
                fprintf(file, "  ERROR: Memory Leaks! numAllocs %d, numBytes %d\n", stats->numAllocs, stats->numBytes);
            }
        }
        uint32 leaks = auditCB.pAudit->MM_GetNumAllocNodes();
        if (leaks != 0)
        {
            fprintf(file, "ERROR: %d Memory leaks detected!\n", leaks);
            MM_AllocQueryInfo*info = auditCB.pAudit->MM_CreateAllocNodeInfo(leaks);
            uint32 leakinfo = auditCB.pAudit->MM_GetAllocNodeInfo(info, leaks, 0);
            if (leakinfo != leaks)
            {
                fprintf(file, "ERROR: Leak info is incomplete.\n");
            }
            for (uint32 i = 0; i < leakinfo; i++)
            {
                fprintf(file, "Leak Info:\n");
                fprintf(file, "  allocNum %d\n", info[i].allocNum);
                fprintf(file, "  fileName %s\n", info[i].fileName);
                fprintf(file, "  lineNo %d\n", info[i].lineNo);
                fprintf(file, "  size %d\n", info[i].size);
                fprintf(file, "  pMemBlock 0x%x\n", info[i].pMemBlock);
                fprintf(file, "  tag %s\n", info[i].tag);
            }
            auditCB.pAudit->MM_ReleaseAllocNodeInfo(info);
        }
    }
#endif

    OsclMem::Cleanup();
    OsclErrorTrap::Cleanup();
    OsclBase::Cleanup();

    fprintf(file, "\nExiting sample application\n\n");

    return result;
}

////////

int _local_main()
{
    // default video input file is "yuvtestinput.yuv"
    // default audio input file is "pcm16testinput.pcm"
    // default video codec is H263
    // default audio codec is AMR
    // default output file format is 3GP
    // default encoded output file is "amr_h263_in_av_test.3gp"

    fprintf(file, "\nFile Settings:\n");
    fprintf(file, "  Input audio input file name pcm16testinput.pcm \n");
    fprintf(file, "  Input video input file name yuvtestinput.yuv \n");

    // instantiate 2way engine
    pv2way_engine_interface *engine_interface = OSCL_NEW(pv2way_engine_interface, ());
    if (engine_interface)
    {
#if !(OSCL_BYPASS_MEMMGT)
        // Obtain the current mem stats before running the test case
        OsclAuditCB auditCB;
        OsclMemInit(auditCB);
        if (auditCB.pAudit)
        {
            MM_Stats_t* stats = auditCB.pAudit->MM_GetStats("");
            if (stats)
            {
                fprintf(file, "\nBefore 2way Memory Stats:\n");
                fprintf(file, "  totalNumAllocs %d\n", stats->totalNumAllocs);
                fprintf(file, "  totalNumBytes %d\n", stats->totalNumBytes);
                fprintf(file, "  numAllocFails %d\n", stats->numAllocFails);
                fprintf(file, "  numAllocs %d\n", stats->numAllocs);
            }
            else
            {
                fprintf(file, "Retrieving memory statistics before running test case failed! Memory statistics result would be invalid.\n");
            }
        }
        else
        {
            fprintf(file, "Memory audit not available! Memory statistics result would be invalid.\n");
        }
#endif

        fprintf(file, "\nStarting 2way\n ");
        // Enable the following code for logging
        PVLogger::Init();
        PVLoggerAppender *appender = NULL;
        OsclRefCounter *refCounter = NULL;

        // Log all nodes, for errors only, to the stdout
        appender = OSCL_ARRAY_NEW(StdErrAppender<TimeAndIdLayout, 1024>, ());
        OsclRefCounterSA<LogAppenderDestructDealloc<StdErrAppender<TimeAndIdLayout, 1024> > > *appenderRefCounter =
            OSCL_NEW(OsclRefCounterSA<LogAppenderDestructDealloc<StdErrAppender<TimeAndIdLayout, 1024> > >,
                     (appender));
        refCounter = appenderRefCounter;

        OsclSharedPtr<PVLoggerAppender> appenderPtr(appender, refCounter);

        PVLogger *rootnode = PVLogger::GetLoggerObject("");
        rootnode->AddAppender(appenderPtr);
        rootnode->SetLogLevel(PVLOGMSG_ERR);

        // Construct and install the active scheduler
        OsclScheduler::Init("PV2wayEngineScheduler");

        OsclExecScheduler *sched = OsclExecScheduler::Current();
        if (sched)
        {
            // Start 2way
            engine_interface->Start2way();

            // Start the scheduler
            // control is transfered to the 2way engine when StartScheduler() is called
            // control is regained when StopScheduler() is called
            fprintf(file, "\nStarting Scheduler\n");

            // Have PV scheduler use its own implementation of the scheduler
            int error = 0;
            OSCL_TRY(error, sched->StartScheduler());
            if (error != 0)
            {
                OSCL_LEAVE(error);
            }
            engine_interface->RemoveFromScheduler();
        }
        else
        {
            fprintf(file, "ERROR! Scheduler is not available. Test case could not run.");
        }

        // when 2way completes, StopScheduler is called
        // and control is returned to this location

        // Shutdown PVLogger and scheduler before checking mem stats
        OsclScheduler::Cleanup();
        PVLogger::Cleanup();

        // done with playback, free resources
        OSCL_DELETE(engine_interface);
        engine_interface = NULL;

        return 0;
    }
    else
    {
        fprintf(file, "ERROR! pv2way_engine_interface could not be instantiated.\n");
        return 1;
    }
}


////////

pv2way_engine_interface::pv2way_engine_interface() : OsclTimerObject(OsclActiveObject::EPriorityNominal, "PV2wayEngine"),
        iState(STATE_UNKNOWN),
        iTerminal(NULL),
        iAudioSrcNode(NULL),
        iVideoSrcNode(NULL),
        isConnected(false),
        videoDataSrcAdded(false),
        audioDataSrcAdded(false),
        audioSinkAdded(false),
        videoSinkAdded(false)
{
    // Initialize the variables with some random number to use for context data testing
    iContextObject = iContextObjectRefValue = 0x5C7A;
}

////////

pv2way_engine_interface::~pv2way_engine_interface()
{
}


////////

void pv2way_engine_interface::Start2way()
{
    fprintf(file, "\npv2way_engine_interface::Start2way()\n");
    // add this timer object to the scheduler
    AddToScheduler();
    // use iState to drive the 2way
    iState = STATE_CREATE;
    // schedule this timer object to run immediately
    RunIfNotReady();
}

////////

void pv2way_engine_interface::RemoveVideoSink()
{
    fprintf(file, "\npv2way_engine_interface::RemoveVideoSink()\n");
    if (iVideoSinkNode)
    {
        PVMediaOutputNodeFactory::DeleteMediaOutputNode(iVideoSinkNode);
        iVideoSinkNode = NULL;
    }
    if (iVideoSinkIOControl)
    {
        OSCL_DELETE(iVideoSinkIOControl);
        iVideoSinkIOControl = NULL;
    }
}

void pv2way_engine_interface::RemoveVideoSrc()
{
    fprintf(file, "\npv2way_engine_interface::RemoveVideoSrc()\n");
    if (iVideoSrcNode)
    {
        PvmfMediaInputNodeFactory::Delete(iVideoSrcNode);
        iVideoSrcNode = NULL;
    }
    if (videomio)
    {
        PvmiMIOFileInputFactory::Delete(videomio);
        videomio = NULL;
    }
    if (iVideoSettings)
    {
        OSCL_DELETE((PvmiMIOFileInputSettings*) iVideoSettings);
        iVideoSettings = NULL;
    }
}

////////

void pv2way_engine_interface::RemoveAudioSink()
{
    fprintf(file, "\npv2way_engine_interface::RemoveAudioSink()\n");
    if (iAudioSinkNode)
    {
        PVMediaOutputNodeFactory::DeleteMediaOutputNode(iAudioSinkNode);
        iAudioSinkNode = NULL;
    }
    if (iAudioSinkIOControl)
    {
        OSCL_DELETE(iAudioSinkIOControl);
        iAudioSinkIOControl = NULL;
    }
}

void pv2way_engine_interface::RemoveAudioSrc()
{
    fprintf(file, "\npv2way_engine_interface::RemoveAudioSrc()\n");
    if (iAudioSrcNode)
    {
        PvmfMediaInputNodeFactory::Delete(iAudioSrcNode);
        iAudioSrcNode = NULL;
    }
    if (audiomio)
    {
        PvmiMIOFileInputFactory::Delete(audiomio);
        audiomio = NULL;
    }
    if (iAudioSettings)
    {
        OSCL_DELETE((PvmiMIOFileInputSettings*) iAudioSettings);
        iAudioSettings = NULL;
    }
}

////////

// this is called by the PV scheduler
void pv2way_engine_interface::Run()
{
    fprintf(file, "\nRun state: %d", iState);
    int error = 0;

    // in this simple app, state transition is used to drive the engine
    // in proper deployment, user requests may be used instead to drive engine
    // end user requests may be queued by in a separate thread and dequeued here
    switch (iState)
    {
        case STATE_CREATE:
            fprintf(file, "\n   STATE_CREATE\n");
            {
                iTerminal = NULL;
                iAudioSrcNode = NULL;
                iVideoSrcNode = NULL;
                // this call to PV is synchronous command
                OSCL_TRY(error, iTerminal = CPV2WayEngineFactory::CreateTerminal(PV_324M, this, this, this));
                if (error)
                {
                    // clean up
                    fprintf(file, "\nStopping Scheduler\n");

                    // Stop the scheduler
                    OsclExecScheduler *sched = OsclExecScheduler::Current();
                    if (sched)
                    {
                        sched->StopScheduler();
                    }
                }
                else
                {
                    // 2way created successfully
                    // next step is to initialize
                    iState = STATE_INIT;
                    // schedule to run immediately
                    RunIfNotReady();
                }
            }
            break;

        case STATE_INIT:
            fprintf(file, "\n   STATE_INIT\n");
            {
                // initialize the 2way engine
                OSCL_TRY(error, iCurrentCmdId = iTerminal->Init(iInitInfo, (OsclAny*)iTerminal));
                OSCL_FIRST_CATCH_ANY(error, iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            }
            break;

        case STATE_CONNECT:
            fprintf(file, "\n   STATE_CONNECT\n");
            {
                iCommSettings.iMediaFormat = PVMF_MULTIPLEXED_FORMAT;
                iCommSettings.iTestObserver = NULL;
                iCommServerIOControl = PvmiMIOCommLoopbackFactory::Create(iCommSettings);
                iCommServer = PVCommsIONodeFactory::Create(iCommServerIOControl);

                // connect the 2way engine
                iConnectOptions.iLoopbackMode = PV_LOOPBACK_MUX;
                OSCL_TRY(error, iCurrentCmdId = iTerminal->Connect(iConnectOptions, iCommServer));
                OSCL_FIRST_CATCH_ANY(error, iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            }
            break;

        case STATE_30SECONDS:
            fprintf(file, "\n   STATE_30SECONDS\n");
            iState = STATE_DISCONNECT;
            fprintf(file, "\n30 seconds have gone by, disconnecting\n");

        case STATE_DISCONNECT:
            fprintf(file, "\n   STATE_DISCONNECT\n");
            {
                // disconnect the 2way session
                TPVPostDisconnectOption iDisconnectOptions;
                OSCL_TRY(error, iCurrentCmdId = iTerminal->Disconnect(iDisconnectOptions, (OsclAny*)iTerminal));
                OSCL_FIRST_CATCH_ANY(error, iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            }
            break;

        case STATE_RESET:
            fprintf(file, "\n   STATE_RESET\n");
            {
                // reset the 2way engine
                OSCL_TRY(error, iCurrentCmdId = iTerminal->Reset((OsclAny*)iTerminal));
                OSCL_FIRST_CATCH_ANY(error, iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
            }
            break;

        case STATE_CLEANUPANDCOMPLETE:
            fprintf(file, "\n   STATE_CLEANUPANDCOMPLETE\n");
            {
                if (iTerminal)
                {
                    CPV2WayEngineFactory::DeleteTerminal(iTerminal);
                    iTerminal = NULL;
                }

                // delete the data sources after deleting the engine.
                RemoveVideoSrc();
                RemoveVideoSink();

                RemoveAudioSrc();
                RemoveAudioSink();

                PVCommsIONodeFactory::Delete(iCommServer);
                PvmiMIOCommLoopbackFactory::Delete(iCommServerIOControl);

                fprintf(file, "\nStopping Scheduler\n");

                // Stop the scheduler
                OsclExecScheduler *sched = OsclExecScheduler::Current();
                if (sched)
                {
                    sched->StopScheduler();
                }
            }
            break;

        default:
            break;

    }
}

////////

// callback by PV engine when a request has been completed
void pv2way_engine_interface::CommandCompleted(const PVCmdResponse& aResponse)
{
    fprintf(file, "\n********** CommandCompleted: command id: %d", aResponse.GetCmdId());
    if (aResponse.GetCmdStatus() != PVMFSuccess)
    {
        fprintf(file, "\n********** CommandCompleted: command status not success: %d", aResponse.GetCmdStatus());
        // Treat as unrecoverable error
        if (isConnected)
        {
            iState = STATE_DISCONNECT;
        }
        else
        {
            iState = STATE_CLEANUPANDCOMPLETE;
        }
        RunIfNotReady();
        return;
    }

    switch (iState)
    {
        case STATE_INIT:
            // 2way engine initialized
            // Connect
            iState = STATE_CONNECT;
            break;

        case STATE_CONNECT:
            // 2way engine connected
            isConnected = true;
            // Get session parameters
            iState = STATE_UNKNOWN;
            return;
            break;

        case STATE_ADDSOURCE_SINK:
            if (aResponse.GetCmdId() == iAddAudioSrcCmdId)
            {
                // Audio data source added
                audioDataSrcAdded = true;
            }
            else if (aResponse.GetCmdId() == iAddVideoSrcCmdId)
            {
                // Video data source added
                videoDataSrcAdded = true;
            }
            else if (aResponse.GetCmdId() == iAddAudioSinkCmdId)
            {
                // Audio data sink added
                audioSinkAdded = true;
            }
            else if (aResponse.GetCmdId() == iAddVideoSinkCmdId)
            {
                // Video data sink added
                videoSinkAdded = true;
            }
            if (audioDataSrcAdded && audioSinkAdded && videoDataSrcAdded && videoSinkAdded)
            {
                iState = STATE_30SECONDS;

                // schedule this object to run in 30 secs
                fprintf(file, "\nWaiting 30 seconds");
                RunIfNotReady(KRecordDuration);
            }
            return;
            break;

        case STATE_DISCONNECT:
            // 2way engine disconnected
            isConnected = false;
            // Reset
            iState = STATE_RESET;
            break;

        case STATE_RESET:
            // 2way engine reset
            // Free resources
            iState = STATE_CLEANUPANDCOMPLETE;
            break;

        default:
            // engine error if this is reached
            fprintf(file, "\n********** CommandCompleted unknown state %d\n ", iState);
            iState = STATE_CLEANUPANDCOMPLETE;
            break;
    }

    // schedule this object to run immediately
    RunIfNotReady();
}

////////

// callback by PV engine when an error is encountered
void pv2way_engine_interface::HandleErrorEvent(const PVAsyncErrorEvent& aEvent)
{
    fprintf(file, "\n****************  HandleErrorEvent ********************");
    fprintf(file, "\nSTATE = %d EventType = %d ResponseType = %d\n", iState, aEvent.GetEventType(), aEvent.GetResponseType());

    if (isConnected)
    {
        iState = STATE_DISCONNECT;
    }
    else
    {
        iState = STATE_RESET;
    }
    // schedule this object to run immediately
    RunIfNotReady();
}

////////

// callback by PV engine when information is available
void pv2way_engine_interface::HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent)
{
    fprintf(file, "\n ************ InformationalEvent: %d\n", aEvent.GetEventType());
    switch (aEvent.GetEventType())
    {
        case PVT_INDICATION_OUTGOING_TRACK:
            // Add data source now
        {
            TPVChannelId *channel_id = (TPVChannelId *)(&aEvent.GetLocalBuffer()[4]);
            fprintf(file, "Indication of outgoing track with logical channel #%d ", *channel_id);
            if (aEvent.GetLocalBuffer()[0] == PV_AUDIO && !iAudioSrcNode)
            {
                // create an audio data source
                // use a file data source
                // Settings for audio file input MIO module
                PvmiMIOFileInputSettings* settings = OSCL_NEW(PvmiMIOFileInputSettings, ());
                settings->iFileName = KAudioTestInput;
#ifdef USE_IF2_FILE
                settings->iMediaFormat = PVMF_AMR_IF2;
#else
                settings->iMediaFormat = PVMF_PCM16;
                settings->iNum20msFramesPerChunk = KNum20msFramesPerChunk;
#endif
                settings->iLoopInputFile = true;
                settings->iSamplingFrequency = KAudioTimescale;
                settings->iNumChannels = KAudioNumChannels;
                iAudioSettings = (OsclAny*)settings;
                audiomio = PvmiMIOFileInputFactory::Create(*settings);

                iAudioSrcNode = PvmfMediaInputNodeFactory::Create(audiomio);
                if (iAudioSrcNode != NULL)
                {
                    int error = 0;
                    OSCL_TRY(error, iAddAudioSrcCmdId = iTerminal->AddDataSource(*channel_id, *iAudioSrcNode));
                    OSCL_FIRST_CATCH_ANY(error, iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
                    iState = STATE_ADDSOURCE_SINK;
                    printf("\nAudio");
                    fprintf(file, "\n Audio - add data source - cmdID: %d\n", iAddAudioSrcCmdId);
                }
            }
            else if (aEvent.GetLocalBuffer()[0] == PV_VIDEO && !iVideoSrcNode)
            {
                // create a video data source
                // use a file data source
                // settings for video file input MIO module
                PvmiMIOFileInputSettings* settings = OSCL_NEW(PvmiMIOFileInputSettings, ());
                settings->iFileName = KYUVTestInput;
                settings->iMediaFormat = PVMF_YUV420;
                settings->iLoopInputFile = true;
                settings->iTimescale = KVideoTimescale;
                settings->iFrameHeight = KVideoFrameHeight;
                settings->iFrameWidth = KVideoFrameWidth;
                settings->iFrameRate = KVideoFrameRate;
                iVideoSettings = (OsclAny*)settings;
                videomio = PvmiMIOFileInputFactory::Create(*settings);

                iVideoSrcNode = PvmfMediaInputNodeFactory::Create(videomio);
                if (iVideoSrcNode != NULL)
                {
                    int error = 0;
                    OSCL_TRY(error, iAddVideoSrcCmdId = iTerminal->AddDataSource(*channel_id, *iVideoSrcNode));
                    OSCL_FIRST_CATCH_ANY(error, iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
                    iState = STATE_ADDSOURCE_SINK;
                    fprintf(file, "\n Video - add data source - cmdID: %d\n", iAddVideoSrcCmdId);
                }
            }
            else
            {
                printf("unknown type");
            }
        }
        break;
        case PVT_INDICATION_INCOMING_TRACK:
            // Add data sink now
        {
            TPVChannelId *channel_id = (TPVChannelId *)(&aEvent.GetLocalBuffer()[4]);
            fprintf(file, "Indication of incoming track with logical channel #%d ", *channel_id);
            if (aEvent.GetLocalBuffer()[0] == PV_AUDIO && !iAudioSinkNode)
            {
                // create the audio sink
                iAudioSinkFileName = KAudioSinkFileName;
                iAudioSinkIOControl = OSCL_NEW(PVRefFileOutput, (iAudioSinkFileName));
#ifdef USE_IF2_FILE
                iAudioSinkIOControl->setFormatMask(PVMF_COMPRESSED_AUDIO_FORMAT);
#else
                iAudioSinkIOControl->setFormatMask(PVMF_UNCOMPRESSED_AUDIO_FORMAT);
#endif
                iAudioSinkNode = PVMediaOutputNodeFactory::CreateMediaOutputNode(iAudioSinkIOControl);
                if (iAudioSinkNode != NULL)
                {
                    int error = 0;
                    OSCL_TRY(error, iAddAudioSinkCmdId = iTerminal->AddDataSink(*channel_id, *iAudioSinkNode));
                    OSCL_FIRST_CATCH_ANY(error, iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
                    iState = STATE_ADDSOURCE_SINK;
                    fprintf(file, "\n Audio - add data sink - cmdID: %d\n", iAddAudioSinkCmdId);
                }
            }
            else if (aEvent.GetLocalBuffer()[0] == PV_VIDEO && !iVideoSinkNode)
            {
                // create the video sink
                iVideoSinkFileName = KVideoSinkFileName;
                iVideoSinkIOControl = OSCL_NEW(PVRefFileOutput, (iVideoSinkFileName));
                iVideoSinkIOControl->setFormatMask(PVMF_UNCOMPRESSED_VIDEO_FORMAT);
                iVideoSinkNode = PVMediaOutputNodeFactory::CreateMediaOutputNode(iVideoSinkIOControl);
                if (iVideoSinkNode != NULL)
                {
                    int error = 0;
                    OSCL_TRY(error, iAddVideoSinkCmdId = iTerminal->AddDataSink(*channel_id, *iVideoSinkNode));
                    OSCL_FIRST_CATCH_ANY(error, iState = STATE_CLEANUPANDCOMPLETE; RunIfNotReady());
                    iState = STATE_ADDSOURCE_SINK;
                    fprintf(file, "\n Video - add data sink - cmdID: %d\n", iAddVideoSinkCmdId);
                }
            }
            else
            {
                printf("unknown type");
            }
            break;
        }

        case PVT_INDICATION_DISCONNECT:
            audioDataSrcAdded = false;
            videoDataSrcAdded = false;
            audioSinkAdded = false;
            videoSinkAdded = false;
            break;

        case PVT_INDICATION_CLOSE_TRACK:
            break;

        case PVT_INDICATION_INTERNAL_ERROR:
            break;

        default:
            break;
    }
}

