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
#ifndef TEST_PV_PLAYER_ENGINE_H_INCLUDED
#define TEST_PV_PLAYER_ENGINE_H_INCLUDED

#ifndef TEST_CASE_H_INCLUDED
#include "test_case.h"
#endif

#ifndef TEXT_TEST_INTERPRETER_H_INCLUDED
#include "text_test_interpreter.h"
#endif

#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif

#ifndef OSCL_EXCEPTION_H_INCLUDE
#include "oscl_exception.h"
#endif

#ifndef PV_PLAYER_FACTORY_H_INCLUDED
#include "pv_player_factory.h"
#endif

#ifndef PV_PLAYER_INTERFACE_H_INCLUDE
#include "pv_player_interface.h"
#endif

#ifndef PV_ENGINE_OBSERVER_H_INCLUDED
#include "pv_engine_observer.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef PVLOGGER_STDERR_APPENDER_H_INCLUDED
#include "pvlogger_stderr_appender.h"
#endif

#ifndef PVLOGGER_TIME_AND_ID_LAYOUT_H_INCLUDED
#include "pvlogger_time_and_id_layout.h"
#endif

#ifndef PVMI_MEDIA_IO_FILEOUTPUT_H_INCLUDED
#include "pvmi_media_io_fileoutput.h"
#endif

#ifndef TEST_PV_PLAYER_ENGINE_CONFIG_H_INCLUDED
#include "test_pv_player_engine_config.h"
#endif

#ifndef MEDIA_CLOCK_CONVERTER
#include "media_clock_converter.h"
#endif

template<class DestructClass>
class LogAppenderDestructDealloc : public OsclDestructDealloc
{
    public:
        virtual void destruct_and_dealloc(OsclAny *ptr)
        {
            delete((DestructClass*)ptr);
        }
};

class pvplayer_engine_test_suite : public test_case
{
    public:
        pvplayer_engine_test_suite(char *aFilename,
                                   PVMFFormatType aFiletype,
                                   int32 aFirstTest,
                                   int32 aLastTest,
                                   bool aCompV,
                                   bool aCompA,
                                   bool aFInput,
                                   bool aBCS,
                                   int32 aLogLevel,
                                   int32 aLogNode,
                                   int32 aLogText,
                                   int32 aLogMem,
                                   int32 aFileFormatType,
                                   bool  aProxyEnabled,
                                   uint32 aDownloadRateInKbps);
};


// Observer class for pvPlayer async test to notify completion of test
class pvplayer_async_test_observer
{
    public:
        // Signals completion of test. Test instance can be deleted after this callback completes.
        virtual void TestCompleted(test_case &) = 0;
};


class PVPlayerAsyncTestParam
{
    public:
        pvplayer_async_test_observer* iObserver;
        test_case* iTestCase;
        FILE* iTestMsgOutputFile;
        const char* iFileName;
        PVMFFormatType iFileType;
        bool iCompressedVideo;
        bool iCompressedAudio;
        bool iFileInput;
        bool iBCS;
        int iCurrentTestNumber;
        bool iProxyEnabled;
        //explicit copy constructor
        void Copy(const PVPlayerAsyncTestParam& aParam)
        {
            iObserver = aParam.iObserver;
            iTestCase = aParam.iTestCase;
            iTestMsgOutputFile = aParam.iTestMsgOutputFile;
            iFileName = aParam.iFileName;
            iFileType = aParam.iFileType;
            iCompressedVideo = aParam.iCompressedVideo;
            iCompressedAudio = aParam.iCompressedAudio;
            iFileInput = aParam.iFileInput;
            iBCS = aParam.iBCS;
            iCurrentTestNumber = aParam.iCurrentTestNumber;
            iProxyEnabled = aParam.iProxyEnabled;
        }
} ;

#define PVPATB_TEST_IS_TRUE(condition) (iTestCase->test_is_true_stub( (condition), (#condition), __FILE__, __LINE__ ))

typedef enum
{
    STATE_CREATE,
    STATE_QUERYINTERFACE,
    STATE_ADDDATASOURCE,
    STATE_CONFIGPARAMS,
    STATE_INIT,
    STATE_QUERYLICENSEACQIF,
    STATE_ACQUIRELICENSE,
    STATE_CANCEL_ACQUIRELICENSE,
    STATE_INIT2,
    STATE_GETMETADATAKEYLIST,
    STATE_GETMETADATAVALUELIST,
    STATE_ADDDATASINK_VIDEO,
    STATE_ADDDATASINK_AUDIO,
    STATE_PREPARE,
    STATE_WAIT_FOR_DATAREADY,
    STATE_WAIT_FOR_BUFFCOMPLETE,
    STATE_CANCELALL,
    STATE_WAIT_FOR_CANCELALL,
    STATE_START,
    STATE_SETPLAYBACKRANGE,
    STATE_PAUSE,
    STATE_RESUME,
    STATE_EOSNOTREACHED,
    STATE_STOP,
    STATE_REMOVEDATASINK_VIDEO,
    STATE_REMOVEDATASINK_AUDIO,
    STATE_RESET,
    STATE_REMOVEDATASOURCE,
    STATE_CLEANUPANDCOMPLETE,
    STATE_PROTOCOLROLLOVER,
    STATE_RECONNECT
} PVTestState;

/*!
** PVPlayerTestMioFactory: MIO Factory functions
*/
class PvmiMIOControl;
class PVPlayerTestMioFactory
{
    public:
        static PVPlayerTestMioFactory* Create();
        virtual ~PVPlayerTestMioFactory() {}

        virtual PvmiMIOControl* CreateAudioOutput(OsclAny* aParam) = 0;
        virtual PvmiMIOControl* CreateAudioOutput(OsclAny* aParam, MediaType aMediaType, bool aCompressedAudio = false)
        {
            OSCL_UNUSED_ARG(aParam);
            OSCL_UNUSED_ARG(aMediaType);
            OSCL_UNUSED_ARG(aCompressedAudio);
            return 0;
        };
        virtual PvmiMIOControl* CreateAudioOutput(OsclAny* aParam, PVRefFileOutputTestObserver* aObserver, bool aActiveTiming, uint32 aQueueLimit, bool aSimFlowControl, bool logStrings = true) = 0;
        virtual void DestroyAudioOutput(PvmiMIOControl* aMio) = 0;
        virtual PvmiMIOControl* CreateVideoOutput(OsclAny* aParam) = 0;
        virtual PvmiMIOControl* CreateVideoOutput(OsclAny* aParam, MediaType aMediaType, bool aCompressedVideo = false)
        {
            OSCL_UNUSED_ARG(aParam);
            OSCL_UNUSED_ARG(aMediaType);
            OSCL_UNUSED_ARG(aCompressedVideo);
            return 0;
        };
        virtual PvmiMIOControl* CreateVideoOutput(OsclAny* aParam, PVRefFileOutputTestObserver* aObserver, bool aActiveTiming, uint32 aQueueLimit, bool aSimFlowControl, bool logStrings = true) = 0;
        virtual void DestroyVideoOutput(PvmiMIOControl* aMio) = 0;
        virtual PvmiMIOControl* CreateTextOutput(OsclAny* aParam) = 0;
        virtual PvmiMIOControl* CreateTextOutput(OsclAny* aParam, MediaType aMediaType)
        {
            OSCL_UNUSED_ARG(aParam);
            OSCL_UNUSED_ARG(aMediaType);
            return 0;
        };
        virtual void DestroyTextOutput(PvmiMIOControl* aMio) = 0;
};

// The base class for all pvplayer engine asynchronous tests
class pvplayer_async_test_base : public OsclTimerObject,
        public PVCommandStatusObserver,
        public PVInformationalEventObserver,
        public PVErrorEventObserver
{
    public:
        pvplayer_async_test_base(PVPlayerAsyncTestParam aTestParam) :
                OsclTimerObject(OsclActiveObject::EPriorityNominal, "PVPlayerEngineAsyncTestBase")
        {
            OSCL_ASSERT(aTestParam.iObserver != NULL);
            OSCL_ASSERT(aTestParam.iTestCase != NULL);
            iObserver = aTestParam.iObserver;
            iTestCase = aTestParam.iTestCase;
            iTestMsgOutputFile = aTestParam.iTestMsgOutputFile;
            iFileName = aTestParam.iFileName;
            iFileType = aTestParam.iFileType;
            iCompressedVideo = aTestParam.iCompressedVideo;
            iCompressedAudio = aTestParam.iCompressedAudio;
            iFileInput = aTestParam.iFileInput;
            iBCS = aTestParam.iBCS;
            iTestCaseName = _STRLIT_CHAR(" ");
            iTestNumber = aTestParam.iCurrentTestNumber;
            iProxyEnabled = aTestParam.iProxyEnabled;

            // Initialize the variables to use for context data testing
            iContextObjectRefValue = 0x5C7A; // some random number
            iContextObject = iContextObjectRefValue;

            iMioFactory = PVPlayerTestMioFactory::Create();
            OSCL_ASSERT(iMioFactory);
        }

        virtual ~pvplayer_async_test_base()
        {
            if (iMioFactory)
            {
                delete iMioFactory;
                iMioFactory = NULL;
            }
        }

        virtual void StartTest() = 0;

        virtual void CommandCompleted(const PVCmdResponse& /*aResponse*/) {}
        virtual void HandleErrorEvent(const PVAsyncErrorEvent& /*aEvent*/) {}
        virtual void HandleInformationalEvent(const PVAsyncInformationalEvent& /*aEvent*/) {}

        // Utility function to retrieve the filename from string and replace ',' with '_'
        void RetrieveFilename(const oscl_wchar* aSource, OSCL_wHeapString<OsclMemAllocator>& aFilename)
        {
            if (aSource == NULL)
            {
                return;
            }

            // Find the last '\' or '/' in the string
            oscl_wchar* lastslash = (oscl_wchar*)aSource;
            bool foundlastslash = false;
            while (!foundlastslash)
            {
                const oscl_wchar* tmp1 = oscl_strstr(lastslash, _STRLIT_WCHAR("\\"));
                const oscl_wchar* tmp2 = oscl_strstr(lastslash, _STRLIT_WCHAR("/"));
                if (tmp1 != NULL)
                {
                    lastslash = (oscl_wchar*)tmp1 + 1;
                }
                else if (tmp2 != NULL)
                {
                    lastslash = (oscl_wchar*)tmp2 + 1;
                }
                else
                {
                    foundlastslash = true;
                }
            }

            // Now copy the filename
            if (lastslash)
            {
                aFilename = lastslash;
            }

            // Replace each '.' in filename with '_'
            bool finishedreplace = false;
            while (!finishedreplace)
            {
                oscl_wchar* tmp = OSCL_CONST_CAST(oscl_wchar*, oscl_strstr(aFilename.get_cstr(), _STRLIT_WCHAR(".")));
                if (tmp != NULL)
                {
                    oscl_strncpy(tmp, _STRLIT_WCHAR("_"), 1);
                }
                else
                {
                    finishedreplace = true;
                }
            }
        }

        pvplayer_async_test_observer* iObserver;
        test_case* iTestCase;
        FILE* iTestMsgOutputFile;
        const char *iFileName;
        PVMFFormatType iFileType;
        bool iCompressedVideo;
        bool iCompressedAudio;
        bool iProxyEnabled;
        bool iFileInput;
        bool iBCS;

        OSCL_HeapString<OsclMemAllocator> iTestCaseName;

        uint32 iContextObject;
        uint32 iContextObjectRefValue;

        int32 iTestNumber;

        // Media IO Factory
        PVPlayerTestMioFactory* iMioFactory;
};


// test_base-based class which will run async tests on pvPlayer engine
class pvplayer_engine_test : public test_case,
        public pvplayer_async_test_observer
{
    public:
        pvplayer_engine_test(char *aFileName,
                             PVMFFormatType aFileType,
                             int32 aFirstTest,
                             int32 aLastTest,
                             bool aCompV,
                             bool aCompA,
                             bool aFInput,
                             bool aBCS,
                             int32 aLogLevel,
                             int32 aLogNode,
                             int32 aLogText,
                             int32 aLogMem,
                             int32 aFileFormatType,
                             bool aProxyEnabled,
                             uint32 aDownloadRateInKbps);
        ~pvplayer_engine_test();

        // Note: for command line options to work, the local tests need to be 0-99,
        // Download tests 100-199,
        // Streaming tests 200-299.
        // Interactive test 800-899

        /**
         *  @page test Test List
         * This is extra text for the Test List page.
         * The tests should be executed from the same directory as the test content.
         * @code
         *    testapp [-test <testcase number>]
         * @endcode
         * where <testcase number> is one of the test cases listed below.
         *
         */

        enum PVPlayerEngineAsyncTests
        {
            /**
            * @test (0) New Delete Test. No input.
                */
            NewDeleteTest = 0,
            OpenPlayStopResetTest,
            OpenPlayStopResetCPMTest,
            MetaDataTest,
            TimingTest,
            InvalidStateTest,
            PreparedStopTest,
            VideoOnlyPlay7Seconds,
            Play5StopPlay10StopReset,
            PauseResume,

            PlayPauseStop = 10,
            OutsideNodeForVideoSink,
            GetPlayerState,
            GetCurrentPosition,
            PlaySetStopPosition,
            PlaySetStopPositionVidFrameNum,
            SetStartPositionPlayStop,
            SetPlayRangePlay,
            SetPlayRangeVidFrameNumPlay,
            PlaySetPlayRangeStop,

            PlaySetPlayRangeVidFrameNumStop = 20,
            TrackLevelInfoTest,
            SetPlaybackRate2X,
            SetPlaybackRateFifth,
            CapConfigInterfaceTest,
            QueuedCommandsTest,
            LoopingTest,
            WaitForEOSTest,
            MultiplePauseResumeTest,
            MultipleRepositionTest, // Start of local tests using media IO node

            MediaIONodeOpenPlayStopTest = 30,
            MediaIONodePlayStopPlayTest,
            MediaIONodePauseResumeTest,
            MediaIONodePlaySetPlaybackRangeTest,
            MediaIONodeSetPlaybackRate3XTest,
            MediaIONodeSetPlaybackRateHalfTest,
            MediaIONodeLoopingTest,
            MediaIONodeWaitForEOSTest,
            MediaIOMultiplePauseResumeTest,
            MediaIOMultipleRepositionTest,

            MediaIORepositionConfigTest = 40,
            MediaIONodeEOSLoopingTest,
            MediaIONodeRepositionDuringPreparedTest,
            MediaIONodePlaySetPlaybackRangeStopPlayTest,
            MediaIONodePlayStopSetPlaybackRangePlayStopTest,
            MediaIONodeSetPlaybackRangeNearEndStartTest,
            MediaIONodePlayRepositionNearEndOfClipTest,
            MediaIONodeForwardStepToEOSTest,
            MediaIONodeForwardStepTest,
            MediaIONodeForwardStepActiveAudioTest,
            MediaIONodeBackwardTest,

            /**
            * @test (51) Open-Play-Stop Test.
            * Default MP4 file with MPEG-4 + AMR tracks.
            * No input. Output YUV and PCM data files.
            */
            MP4M4VAMRFileOpenPlayStopTest = 51, // Start of testing various local files
            /**
            * @test (52) Play-Stop-Play-Stop Test.
            * Default MP4 file with MPEG-4 + AMR tracks.
            * No input. Output YUV and PCM data files.
            */
            MP4M4VAMRFilePlayStopPlayStopTest,
            /**
            * @test (53) Open-Play-Stop Test.
            * Default MP4 file with H.263 + AMR tracks.
            * No input. Output YUV and PCM data files.
            */
            MP4H263AMRFileOpenPlayStopTest,
            /**
            * @test (54) Play-Stop-Play-Stop Test.
            * Default MP4 file with H.263 + AMR tracks.
            * No input. Output YUV and PCM data files.
            */
            MP4H263AMRFilePlayStopPlayStopTest,
            /**
            * @test (55) Open-Play-Stop Test.
            * Default MP4 file with AVC + AMR tracks.
            * No input. Output YUV and PCM data files.
            */
            MP4AVCAMRFileOpenPlayStopTest,
            /**
            * @test (56) Play-Stop-Play-Stop Test.
            * Default MP4 file with AVC + AMR tracks.
            * No input. Output YUV and PCM data file.
            */
            MP4AVCAMRFilePlayStopPlayStopTest,
            /**
            * @test (57) Open-Play-Stop Test.
            * Default MP4 file with AMR track.
            * No input. Output PCM data file.
            */
            MP4AMRFileOpenPlayStopTest,
            /**
            * @test (58) Play-Stop-Play-Stop Test.
            * DDefault MP4 file with AMR track.
            * No input. Output PCM data file.
            */
            MP4AMRFilePlayStopPlayStopTest,
            /**
            * @test (59) Open-Play-Stop Test.
            * Default MP4 file with AAC track.
            * No input. Output PCM data file.
            */
            MP4AACFileOpenPlayStopTest,
            /**
            * @test (60) Play-Stop-Play-Stop Test.
            * Default MP4 file with AAC track.
            * No input. Output PCM data file.
            */
            MP4AACFilePlayStopPlayStopTest = 60,
            /**
            * @test (61) Open-Play-Stop Test.
            * Default MP4 file with AMR + text track.
            * No input. Output PCM data file.
            */
            MP4M4VAMRTextFileOpenPlayStopTest,
            /**
            * @test (62) Play-Stop-Play-Stop Test.
            * Default MP4 file with AMR + text track.
            * No input. Output PCM data file.
            */
            MP4M4VAMRTextFilePlayStopPlayStopTest,
            /**
            * @test (63) Open-Play-Stop Test.
            * Default AMR IETF format file.
            * No input. Output PCM data files.
            */
            AMRIETFFileOpenPlayStopTest,
            /**
            * @test (64) Play-Stop-Play-Stop Test.
            * Default AMR IETF format file.
            * No input. Output PCM data files.
            */
            AMRIETFFilePlayStopPlayStopTest,
            /**
            * @test (65) Open-Play-Stop Test.
            * Default AMR IF2 format file.
            * No input. Output PCM data files.
            */
            AMRIF2FileOpenPlayStopTest,
            /**
            * @test (66) Play-Stop-Play-Stop Test.
            * Default AMR IF2 format file.
            * No input. Output PCM data files.
            */
            AMRIF2FilePlayStopPlayStopTest,
            /**
            * @test (67) Open-Play-Stop Test.
            * Default AAC ADTS format file.
            * No input. Output PCM data files.
            */
            AACADTSFileOpenPlayStopTest,
            /**
            * @test (68) Play-Stop-Play-Stop Test.
            * Default AAC ADTS format file.
            * No input. Output PCM data files.
            */
            AACADTSFilePlayStopPlayStopTest,
            /**
            * @test (69) Open-Play-Stop Test.
            * Default AAC ADIF format file.
            * No input. Output PCM data files.
            */
            AACADIFFileOpenPlayStopTest,
            /**
            * @test (70) Play-Stop-Play-Stop Test.
            * Default AAC ADIF format file.
            * No input. Output PCM data files.
            */
            AACADIFFilePlayStopPlayStopTest = 70,
            /**
            * @test (71) Open-Play-Stop Test.
            * Default raw AAC format file.
            * No input. Output PCM data files.
            */
            AACRawFileOpenPlayStopTest,
            /**
            * @test (72) Play-Stop-Play-Stop Test.
            * Default raw AAC format file.
            * No input. Output PCM data files.
            */
            AACRawFilePlayStopPlayStopTest,
            /**
            * @test (73) Open-Play-Stop Test.
            * Default MP3 constant bitrate (CBR) format file.
            * No input. Output PCM data files.
            */
            MP3CBRFileOpenPlayStopTest,
            /**
            * @test (74) Play-Stop-Play-Stop Test.
            * Default MP3 constant bitrate (CBR) format file.
            * No input. Output PCM data files.
            */
            MP3CBRFilePlayStopPlayStopTest,
            /**
            * @test (75) Open-Play-Stop Test.
            * Default MP3 variable bitrate (VBR) format file.
            * No input. Output PCM data files.
            */
            MP3VBRFileOpenPlayStopTest,
            /**
            * @test (76) Play-Stop-Play-Stop Test.
            * Default MP3 variable bitrate (VBR) format file.
            * No input. Output PCM data files.
            */
            MP3VBRFilePlayStopPlayStopTest,
            /**
            * @test (77) Open-Play-Stop Test.
            * Default WAV format file.
            * No input. Output PCM data files.
            */
            WAVFileOpenPlayStopTest,
            /**
            * @test (78) Play-Stop-Play-Stop Test.
            * Default WAV format file.
            * No input. Output PCM data files.
            */
            WAVFilePlayStopPlayStopTest,
            /**
            * @test (79) Open-Play-Stop Test.
            * Default ASF format file with WMV and WMA tracks.
            * No input. Output YUV and PCM data files.
            */
            ASFFileOpenPlayStopTest,
            /**
            * @test (80) Play-Stop-Play-Stop Test.
            * Default ASF format file with WMV and WMA tracks.
            * No input. Output YUV and PCM data files.
            */
            ASFFilePlayStopPlayStopTest = 80,

            //real audio test case
            /**
            * @test (81) Open-Play-Stop Test.
            * Default Real format file with Real Video and Real Audio tracks.
            * No input. Output YUV and PCM data files.
            */
            RealAudioFileOpenPlayStopTest = 81,
            SetPlaybackAfterPrepare,

            MediaIONodeBackwardForwardTest = 83,
            MediaIONodePauseNearEOSBackwardResumeTest,
            MediaIONodeMultiplePauseSetPlaybackRateResumeTest,
            MediaIONodeBackwardNearEOSForwardNearStartTest,

            MultiplePauseSeekResumeTest = 87,
            MultipleSetStartPositionPlayStopTest,

            OpenPlayStopResetCPMRecognizeTest = 89, //Start of testing recognizer using DataStream input

            LastLocalTest,//placeholder

            FirstDownloadTest = 100,  //placeholder

            /**
            * @test (101) FastTrack Download Open Play Stop Test.
            * Input PVX file (session establishment file for FastTrack).
            * Output (partially) downloaded media file and YUV and PCM data files from 10 seconds of Playback.
            */
            FTDownloadOpenPlayStopTest = 101,
            /**
            * @test (102) Progressive Download Play ASAP Test.
            * Input http url to a progressively downloadable file.
            * Output (partially) downloaded media file and YUV and PCM data files from 10 seconds of Playback.
            */
            ProgDownloadPlayAsapTest, //102
            /**
            * @test (103) Progressive Download Then Play Test.
            * Input http url to a progressively downloadable file.
            * Output (partially) downloaded media file and YUV and PCM data files from Playback.
            */
            ProgDownloadDownloadThenPlayTest, //103
            /**
            * @test (104) Progressive Download Only Test.
            * Input http url to a progressively downloadable file.
            * Output downloaded media file.
            */
            ProgDownloadDownloadOnlyTest, //104
            /**
            * @test (105) Progressive Download Cancel During Init Test.
            * Input http url to a progressively downloadable file.
            * Output (partially) downloaded media file.
            */
            ProgDownloadCancelDuringInitTest, //105
            /**
            * @test (106) Progressive Download Content Too Large Test.
            * Download the file whose size is larger than the maximum file size which the player can download.
            * Input http url to a progressively downloadable file.
            * No output.
            */
            ProgDownloadContentTooLarge, //106
            /**
            * @test (107) Progressive Download Truncated Test.
            * Start the playback until PVMFInfoContentTruncated or EOS error occurs.
            * Input special http url to a download simulator.
            * Output (partially) downloaded media file and YUV and PCM data files from Playback.
            */
            ProgDownloadTruncated, //107
            /**
            * @test (108) Progressive Download Protocol Rollover Test.
            * Set Protocol rollover and continue download
            * Input http url to a progressively downloadable file.
            * Output (partially) downloaded media file and YUV and PCM data files from Playback.
            */
            ProgDownloadProtocolRolloverTest, //108
            /**
            * @test (109) Progressive Download Set Playback Range Test.
            * Input http url to a progressively downloadable file.
            * Output (partially) downloaded media file and YUV and PCM data files from Playback.
            */
            ProgDownloadSetPlayBackRangeTest, //109
            /**
            * @test (110) Progressive Download Play Until EOS Test.
            * Input http url to a progressively downloadable file.
            * Output (partially) downloaded media file and YUV and PCM data files from Playback.
            */
            ProgDownloadPlayUtilEOSTest, //110
            /**
            * @test (111) FastTrack Download Open Play Until EOS Test.
            * Input PVX file (session establishment file for FastTrack).
            * Output (partially) downloaded media file and YUV and PCM data files from Playback.
            */
            FTDownloadOpenPlayUntilEOSTest, //111
            /**
            * @test (112) Progressive Download Then Play-Pause Test.
            * Input http url to a progressively downloadable file.
            * Output (partially) downloaded media file and YUV and PCM data files from Playback.
            */
            ProgDownloadDownloadThenPlayPauseTest,//112
            /**
            * @test (113) Progressive Download Then Play Reposition Test.
            * Input http url to a progressively downloadable file.
            * Output (partially) downloaded media file and YUV and PCM data files from Playback.
            */
            ProgDownloadDownloadThenPlayRepositionTest,//113
            /**
            * @test (114) Progressive Download Cancel During Init Delay Test.
            * Input http url to a progressively downloadable file.
            * Output (partially) downloaded media file and YUV and PCM data files from Playback.
            */
            ProgDownloadCancelDuringInitDelayTest, //114
            /**
            * @test (115) Progressive Download Resume After Underflow Test.
            * Input http url to a progressively downloadable file.
            * Output (partially) downloaded media file and YUV and PCM data files from Playback.
            */
            ProgDownloadPauseResumeAfterUnderflowTest, //115
            /**
            * @test (116) FastTrack Download Play-Stop-Play Test.
            * Input PVX file (session establishment file for FastTrack).
            * Output (partially) downloaded media file and YUV and PCM data files from Playback.
            */
            FTDownloadPlayStopPlayTest, //116
            /**
            * @test (117) Progressive Download Play-Stop-Play Test.
            * Input http url to a progressively downloadable file.
            * Output (partially) downloaded media file and YUV and PCM data files from Playback.
            */
            ProgDownloadPlayStopPlayTest, //117

            LastDownloadTest, //placeholder

            /**
            * @test (150) Progressive Playback Until EOS Test
            * Input http url to a progressively downloadable MP4 or MP3 file.
            * Default url is an http url to an MP4 file.
            * Output YUV and PCM data files from Playback.
            */
            ProgPlaybackMP4UntilEOSTest = 150,
            /**
            * @test (151) Progressive Playback Short Test
            * Input http url to a progressively downloadable MP4 or MP3 file.
            * Default url is an http url to an MP4 file.
            * Output YUV and PCM data files from Playback.
            */
            ProgPlaybackMP4ShortTest, //151
            /**
            * @test (152) Progressive Playback Short Pause-Resume Test
            * Input http url to a progressively downloadable MP4 or MP3 file.
            * Default url is an http url to an MP4 file.
            * Output YUV and PCM data files from Playback.
            */
            ProgPlaybackMP4ShortPauseResumeTest, //152
            /**
            * @test (153) Progressive Playback Long Pause-Resume Test
            * Input http url to a progressively downloadable MP4 or MP3 file.
            * Default url is an http url to an MP4 file.
            * Output YUV and PCM data files from Playback.
            */
            ProgPlaybackMP4LongPauseResumeTest, //153
            /**
            * @test (154) Progressive Playback Start-Pause-Seek-Resume-Twice Test
            * Input http url to a progressively downloadable MP4 or MP3 file.
            * Default url is an http url to an MP4 file.
            * Output YUV and PCM data files from Playback.
            */
            ProgPlaybackMP4StartPauseSeekResumeTwiceTest, //154
            /**
            * @test (155) Progressive Playback Seek-Start Test
            * Input http url to a progressively downloadable MP4 or MP3 file.
            * Default url is an http url to an MP4 file.
            * Output YUV and PCM data files from Playback.
            */
            ProgPlaybackMP4SeekStartTest, //155
            /**
            * @test (156) Progressive Playback Start-Pause-Seek-Resume-Loop Test
            * Input http url to a progressively downloadable MP4 or MP3 file.
            * Default url is an http url to an MP4 file.
            * Output YUV and PCM data files from Playback.
            */
            ProgPlaybackMP4StartPauseSeekResumeLoopTest, //156
            /**
            * @test (157) Progressive Playback Seek-Forward-Step-Loop Test
            * Input http url to a progressively downloadable MP4 or MP3 file.
            * Default url is an http url to an MP4 file.
            * Output YUV and PCM data files from Playback.
            */
            ProgPlaybackMP4SeekForwardStepLoopTest, //157
            /**
            * @test (158) Progressive Playback Play-Stop-Play Test
            * Input http url to a progressively downloadable MP4 or MP3 file.
            * Default url is an http url to an MP4 file.
            * Output YUV and PCM data files from Playback.
            */
            ProgPlaybackPlayStopPlayTest, //158
            /**
            * @test (159) Progressive Playback Seek to beginning of clip after Download Complete Test
            * Input http url to a progressively downloadable MP4 or MP3 file.
            * Default url is an http url to an MP4 file.
            * Output YUV and PCM data files from Playback.
            */
            ProgPlaybackMP4SeekToBOCAfterDownloadCompleteTest, //159
            /**
            * @test (160) Progressive Playback Seek in cache after Download Complete Test
            * Input http url to a progressively downloadable MP4 or MP3 file.
            * Default url is an http url to an MP4 file.
            * Output YUV and PCM data files from Playback.
            */
            ProgPlaybackMP4SeekInCacheAfterDownloadCompleteTest, //160
            /**
            * @test (161) Progressive Playback EOS Stop Play Again Test
            * Input http url to a progressively downloadable MP4 or MP3 file.
            * Default url is an http url to an MP4 file.
            * Output YUV and PCM data files from Playback.
            */
            ProgPlaybackMP4EOSStopPlayAgainTest, //161

            LastProgressivePlaybackTest, //placeholder

            /**
            * @test (180) Shoutcast Playback 5 minute Test
            * Input http url to a Shoutcast url
            * Default url is a Shoutcast url to an MP3 file.
            * Output PCM data file from Playback.
            */
            ShoutcastPlayback5MinuteTest = 180,
            /**
            * @test (181) Shoutcast Playback Pause-Resume Test
            * Input http url to a Shoutcast url
            * Default url is a Shoutcast url to an MP3 file.
            * Output PCM data file from Playback.
            */
            ShoutcastPlaybackPauseResumeTest = 181,
            /**
            * @test (182) Shoutcast Playback Play-Stop-Play Test
            * Input http url to a Shoutcast url
            * Default url is a Shoutcast url to an MP3 file.
            * Output PCM data file from Playback.
            */
            ShoutcastPlaybackPlayStopPlayTest = 182,

            LastShoutcastPlaybackTest, // placeholder

            FirstStreamingTest = 200, //placeholder

            /**
            * @test (201) Streaming Open-Play-Stop Test
            * Stream content for 20 seconds only
            * Input rtsp url, SDP file, or http url to SDP or ASF file
            * Default input is an SDP file
            * Output YUV and PCM data files from Playback.
            */
            StreamingOpenPlayStopTest, //201
            /**
            * @test (202) Streaming Open-Play-Pause-Play-Stop Test
            * Stream content for 20 seconds - Pause - Stream 20 seconds more.
            * Input rtsp url, SDP file, or http url to SDP or ASF file
            * Default input is an SDP file
            * Output YUV and PCM data files from Playback.
            */
            StreamingOpenPlayPausePlayStopTest, //202
            /**
            * @test (203) Streaming Open-Play-Seek-Stop Test
            * Stream content for 20 seconds - Seek - Stop.
            * Input rtsp url, SDP file, or http url to SDP or ASF file
            * Default input is an SDP file
            * Output YUV and PCM data files from Playback.
            */
            StreamingOpenPlaySeekStopTest, //203
            /**
            * @test (204) Streaming Cancel During Prepare Test
            * Input rtsp url, SDP file, or http url to SDP or ASF file
            * Default input is an SDP file
            * Output YUV and PCM data files from Playback.
            */
            StreamingCancelDuringPrepareTest, //204

            LastStreamingTest, //placeholder

            //Multiple Instance tests.
            MultipleInstanceOpenPlayStopTest = 300,
            MultipleThreadOpenPlayStopTest = 301,
            //this range reserved for future multiple instance tests.
            LastMultipleInstanceTest = 325,//placeholder

            FirstProjTest = 700, // placeholder
            // Project specific unit tests should have numbers 701 to 799
            LastProjTest = 799,

            FirstInteractiveTest = 800, // placeholder

            PrintMetadataTest = 801,
            PrintMemStatsTest,
            PlayUntilEOSTest,
            ReleaseMetadataTest,

            /**
             * @test (805) ExternalDownloadPlayUntilEOSTest
             * This testcase will test external download provided the source node in pvPlayerSDK has been modified to take in external datastream.
             * If one provides a local file (say mp4, asf etc) then this testcase instantiates a download sim, configures it
             * (PUT has a new command line option "-downloadrate" to specify the desired download simulate rate in kbps).
             * It also instantiate the external DS factory and passes the same to engine in AddDataSource.
             * If one provides this testcase with HTTP URL and if the player test configuration has RUN_PVNETWORK_DOWNLOAD_TESTCASES set to 1,
             * then this testcase will instantiate "HTTPDownload" class to do the download.
             * This class is part of the "http_retriever" library located in \protocols\.
             * Testcase also instantiate the external DS factory and passes the same to engine in AddDataSource.
             * This testcase does not accept, RTSP URLs, SDP files, PVX files as input. If RUN_PVNETWORK_DOWNLOAD_TESTCASES is set to 0
             * then HTTP URL inputs will not work as well.
             * This testcase does not have default arguments built in. It must always be run with "-source" arg.
             */
            ExternalDownloadPlayUntilEOSTest = 805,

            /**
             * @test (806) MultiProcessExternalDownloadPlayUntilEOSTest
             * This testcase will test external download provided the source node in pvPlayerSDK has been modified to take in external datastream.
             * This testcase uses the HTTPDownload class to perform the download and pass the downloading file name to filemonitor class that
             * the file as it downloads. This is to simulate usecases where donwnload is happenning in a completely different process space.
             * It also instantiate the external DS factory and passes the same to engine in AddDataSource.
             * This testcase only accepts HTTP URLs and is valid only if the player test configuration has RUN_PVNETWORK_DOWNLOAD_TESTCASES set to 1,
             * This testcase does not accept, local files (asf, mp4, mp3 etc), RTSP URLs, SDP files, PVX files as input.
             * If RUN_PVNETWORK_DOWNLOAD_TESTCASES is set to 0 then this testcase is not enabled.
             * This testcase does not have default arguments built in. It must always be run with "-source" arg.
             */
            MultiProcessExternalDownloadPlayUntilEOSTest = 806,
            PlayUntilEOSUsingExternalFileHandleTest = 807,

            StreamingOpenPlayUntilEOSTest = 851,//851
            StreamingOpenPlayPausePlayUntilEOSTest,//852
            StreamingOpenPlaySeekPlayUntilEOSTest, //853
            StreamingJitterBufferAdjustUntilEOSTest, //854
            StreamingCloakingOpenPlayUntilEOSTest, //855
            StreamingPlayBitStreamSwitchPlayUntilEOSTest,//856
            StreamingMultiplePlayUntilEOSTest,//857
            StreamingMultipleCloakingPlayUntilEOSTest, //858
            StreamingProtocolRollOverTest, //859
            StreamingProtocolRollOverTestWithUnknownURLType, //860
            StreamingPlayListSeekTest, //861
            StreamingSeekAfterEOSTest, //862
            /*
             * TC 863: This test case checks the engine behavior when the server responds
             * with any of the following error codes - 404 (URL Not Found), 415 (Media Unsupported),
             * 457 (Invalid Range). In all cases, the test should not error out, and complete as in TC 861
             */
            StreamingPlayListErrorCodeTest, // 863
            StreamingOpenPlayMultipleSeekToEndOfClipUntilEOSTest, //864


            StreamingOpenPlayMultiplePausePlayUntilEOSTest = 875, //875

            DVBH_StreamingOpenPlayStopTest = 876, //876
            DVBH_StreamingOpenPlayUntilEOSTest, //877
            StreamingLongPauseTest, //878

            CPM_DLA_OMA1PASSTRHU_OpenFailAuthPlayStopResetTest, //879
            CPM_DLA_OMA1PASSTRHU_OpenPlayStopResetTest, //880
            CPM_DLA_OMA1PASSTRHU_UnknownContentOpenPlayStopResetTest, //881

            StreamingOpenPlayUntilEOSTestWithFileHandle,//882
            //883-888 available

            //GetLicense returns commandCompleted before CancelLic could be triggered
            CPM_DLA_OMA1PASSTRHU_CancelAcquireLicenseTooLate_CancelFails = 889, //889
            //GetLicense does not commandComplete, cancelLic is triggered
            CPM_DLA_OMA1PASSTRHU_CancelAcquireLicense_CancelSucceeds, //890
            CPM_DLA_OMA1PASSTRHU_ContentNotSupported, //891

            StreamingOpenPlayForwardPlayUntilEOSTest, //892

            //Multiple CPM Plugins
            OpenPlayStop_MultiCPMTest, //893
            StreamingLongPauseSeekTest, //894

            /**
            * @test (900) GenericReset_AddDataSource Test
            */
            GenericReset_AddDataSource = 900,
            /**
            * @test (901) GenericReset_Init Test
            */
            GenericReset_Init,
            /**
            * @test (902) GenericReset_AddDataSinkVideo Test
            */
            GenericReset_AddDataSinkVideo,
            /**
            * @test (903) GenericReset_AddDataSinkAudio Test
            */
            GenericReset_AddDataSinkAudio,
            /**
            * @test (904) GenericReset_Prepare Test
            */
            GenericReset_Prepare,
            /**
            * @test (905) GenericReset_Start Test
            */
            GenericReset_Start,
            /**
            * @test (906) GenericReset_Pause Test
            */
            GenericReset_Pause,
            /**
            * @test (907) GenericReset_Resume Test
            */
            GenericReset_Resume,
            /**
            * @test (908) GenericReset_Stop Test
            */
            GenericReset_Stop,
            //GenericReset_Reset,
            /**
            * @test (909) GenericReset_SetPlaybackRange Test
            */
            GenericReset_SetPlaybackRange,
            /**
            * @test (910) GenericDelete_AddDataSource Test
            */
            GenericDelete_AddDataSource = 910,
            /**
            * @test (911) GenericDelete_Init Test
            */
            GenericDelete_Init,
            /**
            * @test (912) GenericDelete_AddDataSinkVideo Test
            */
            GenericDelete_AddDataSinkVideo,
            /**
            * @test (913) GenericDelete_AddDataSinkAudio Test
            */
            GenericDelete_AddDataSinkAudio,
            /**
            * @test (914) GenericDelete_Prepare Test
            */
            GenericDelete_Prepare,
            /**
            * @test (915) GenericDelete_Start Test
            */
            GenericDelete_Start,
            /**
            * @test (916) GenericDelete_Pause Test
            */
            GenericDelete_Pause,
            /**
            * @test (917) GenericDelete_Resume Test
            */
            GenericDelete_Resume,
            /**
            * @test (918) GenericDelete_Stop Test
            */
            GenericDelete_Stop,
            //GenericDelete_Reset,
            /**
            * @test (919) GenericDelete_SetPlaybackRange Test
            */
            GenericDelete_SetPlaybackRange,
            /**
            * @test (920) GenericDeleteWhileProc_AddDataSource Test
            */
            GenericDeleteWhileProc_AddDataSource = 920,
            /**
            * @test (921) GenericDeleteWhileProc_Init Test
            */
            GenericDeleteWhileProc_Init,
            /**
            * @test (922) GenericDeleteWhileProc_AddDataSinkVideo Test
            */
            GenericDeleteWhileProc_AddDataSinkVideo,
            /**
            * @test (923) GenericDeleteWhileProc_AddDataSinkAudio Test
            */
            GenericDeleteWhileProc_AddDataSinkAudio,
            /**
            * @test (924) GenericDeleteWhileProc_Prepare Test
            */
            GenericDeleteWhileProc_Prepare,
            /**
            * @test (925) GenericDeleteWhileProc_Start Test
            */
            GenericDeleteWhileProc_Start,
            /**
            * @test (926) GenericDeleteWhileProc_Pause Test
            */
            GenericDeleteWhileProc_Pause,
            /**
            * @test (927) GenericDeleteWhileProc_Resume Test
            */
            GenericDeleteWhileProc_Resume,
            /**
            * @test (928) GenericDeleteWhileProc_Stop Test
            */
            GenericDeleteWhileProc_Stop,
            //GenericDeleteWhileProc_Reset,
            /**
            * @test (929) GenericDeleteWhileProc_SetPlaybackRange Test
            */
            GenericDeleteWhileProc_SetPlaybackRange,

            /**
            * @test (930) GenericCancelAll_AddDataSource Test
            */
            GenericCancelAll_AddDataSource = 930,
            /**
            * @test (931) GenericCancelAll_Init Test
            */
            GenericCancelAll_Init,
            /**
            * @test (932) GenericCancelAll_AddDataSinkVideo Test
            */
            GenericCancelAll_AddDataSinkVideo,
            /**
            * @test (933) GenericCancelAll_AddDataSinkAudio Test
            */
            GenericCancelAll_AddDataSinkAudio,
            /**
            * @test (934) GenericCancelAll_Prepare Test
            */
            GenericCancelAll_Prepare,
            /**
            * @test (935) GenericCancelAll_Start Test
            */
            GenericCancelAll_Start,
            /**
            * @test (936) GenericCancelAll_Pause Test
            */
            GenericCancelAll_Pause,
            /**
            * @test (937) GenericCancelAll_Resume Test
            */
            GenericCancelAll_Resume,
            /**
            * @test (938) GenericCancelAll_Stop Test
            */
            GenericCancelAll_Stop,
            //GenericCancelAll_Reset,
            /**
            * @test (939) GenericCancelAll_SetPlaybackRange Test
            */
            GenericCancelAll_SetPlaybackRange,

            /**
            * @test (940) GenericCancelAllWhileProc_AddDataSource Test
            */
            GenericCancelAllWhileProc_AddDataSource = 940,
            /**
            * @test (941) GenericCancelAllWhileProc_Init Test
            */
            GenericCancelAllWhileProc_Init,
            /**
            * @test (942) GenericCancelAllWhileProc_AddDataSinkVideo Test
            */
            GenericCancelAllWhileProc_AddDataSinkVideo,
            /**
            * @test (943) GenericCancelAllWhileProc_AddDataSinkAudio Test
            */
            GenericCancelAllWhileProc_AddDataSinkAudio,
            /**
            * @test (944) GenericCancelAllWhileProc_Prepare Test
            */
            GenericCancelAllWhileProc_Prepare,
            /**
            * @test (945) GenericCancelAllWhileProc_Start Test
            */
            GenericCancelAllWhileProc_Start,
            /**
            * @test (946) GenericCancelAllWhileProc_Pause Test
            */
            GenericCancelAllWhileProc_Pause,
            /**
            * @test (947) GenericCancelAllWhileProc_Resume Test
            */
            GenericCancelAllWhileProc_Resume,
            /**
            * @test (948) GenericCancelAllWhileProc_Stop Test
            */
            GenericCancelAllWhileProc_Stop,
            //GenericCancelAllWhileProc_Reset,
            /**
            * @test (949) GenericCancelAllWhileProc_SetPlaybackRange Test
            */
            GenericCancelAllWhileProc_SetPlaybackRange,

            // ACCESS DRM plugin tests
            /**
            * @test (960) FirstAccessCPMTest Test
            */
            FirstAccessCPMTest = 960,
            /**
            * @test (961) QueryEngine_AccessCPMTest Test
            */
            QueryEngine_AccessCPMTest,          //961
            /**
            * @test (962) OpenPlayStop_AccessCPMTest Test
            */
            OpenPlayStop_AccessCPMTest,         //962
            /**
            * @test (963) PlayStopPlayStop_AccessCPMTest Test
            */
            PlayStopPlayStop_AccessCPMTest,     //963
            /**
            * @test (964) StartupMeasurement_AccessCPMTest Test
            */
            StartupMeasurement_AccessCPMTest,   //964
            /**
            * @test (965) LastAccessCPMTest Test
            */
            LastAccessCPMTest,

            /**
            * @test (1051) GenericNetworkDisconnect_AddDataSource Test
            */
            GenericNetworkDisconnect_AddDataSource = 1051,
            /**
            * @test (1052) GenericNetworkDisconnect_Init Test
            */
            GenericNetworkDisconnect_Init,
            /**
            * @test (1053) GenericNetworkDisconnect_AddDataSinkVideo Test
            */
            GenericNetworkDisconnect_AddDataSinkVideo,
            /**
            * @test (1054) GenericNetworkDisconnect_AddDataSinkAudio Test
            */
            GenericNetworkDisconnect_AddDataSinkAudio,
            /**
            * @test (1055) GenericNetworkDisconnect_Prepare Test
            */
            GenericNetworkDisconnect_Prepare,
            /**
            * @test (1056) GenericNetworkDisconnect_Start Test
            */
            GenericNetworkDisconnect_Start,
            /**
            * @test (1057) GenericNetworkDisconnect_Pause Test
            */
            GenericNetworkDisconnect_Pause,
            /**
            * @test (1058) GenericNetworkDisconnect_Resume Test
            */
            GenericNetworkDisconnect_Resume,
            /**
            * @test (1059) GenericNetworkDisconnect_Stop Test
            */
            GenericNetworkDisconnect_Stop,
            /**
            * @test (1060) GenericNetworkDisconnect_SetPlaybackRange Test
            */
            GenericNetworkDisconnect_SetPlaybackRange,

            /**
            * @test (1061) GenericNetworkDisconnectWhileProc_AddDataSource Test
            */
            GenericNetworkDisconnectWhileProc_AddDataSource = 1061,
            /**
            * @test (1062) GenericNetworkDisconnectWhileProc_Init Test
            */
            GenericNetworkDisconnectWhileProc_Init,
            /**
            * @test (1063) GenericNetworkDisconnectWhileProc_AddDataSinkVideo Test
            */
            GenericNetworkDisconnectWhileProc_AddDataSinkVideo,
            /**
            * @test (1064) GenericNetworkDisconnectWhileProc_AddDataSinkAudio Test
            */
            GenericNetworkDisconnectWhileProc_AddDataSinkAudio,
            /**
            * @test (1065) GenericNetworkDisconnectWhileProc_Prepare Test
            */
            GenericNetworkDisconnectWhileProc_Prepare,
            /**
            * @test (1066) GenericNetworkDisconnectWhileProc_Start Test
            */
            GenericNetworkDisconnectWhileProc_Start,
            /**
            * @test (1067) GenericNetworkDisconnectWhileProc_Pause Test
            */
            GenericNetworkDisconnectWhileProc_Pause,
            /**
            * @test (1068) GenericNetworkDisconnectWhileProc_Resume Test
            */
            GenericNetworkDisconnectWhileProc_Resume,
            /**
            * @test (1069) GenericNetworkDisconnectWhileProc_Stop Test
            */
            GenericNetworkDisconnectWhileProc_Stop,
            /**
            * @test (1070) GenericNetworkDisconnectWhileProc_SetPlaybackRange Test
            */
            GenericNetworkDisconnectWhileProc_SetPlaybackRange,

            /**
            * @test (1071) GenericNetworkDisconnectReconnect_AddDataSource Test
            */
            GenericNetworkDisconnectReconnect_AddDataSource = 1071,
            /**
             * @test (1072) GenericNetworkDisconnectReconnect_Init Test
             */
            GenericNetworkDisconnectReconnect_Init,
            /**
             * @test (1073) GenericNetworkDisconnectReconnect_AddDataSinkVideo Test
             */
            GenericNetworkDisconnectReconnect_AddDataSinkVideo,
            /**
             * @test (1074) GenericNetworkDisconnectReconnect_AddDataSinkAudio Test
             */
            GenericNetworkDisconnectReconnect_AddDataSinkAudio,
            /**
             * @test (1075) GenericNetworkDisconnectReconnect_Prepare Test
             */
            GenericNetworkDisconnectReconnect_Prepare,
            /**
             * @test (1076) GenericNetworkDisconnectReconnect_Start Test
             */
            GenericNetworkDisconnectReconnect_Start,
            /**
             * @test (1077) GenericNetworkDisconnectReconnect_Pause Test
             */
            GenericNetworkDisconnectReconnect_Pause,
            /**
             * @test (1078) GenericNetworkDisconnectReconnect_Resume Test
             */
            GenericNetworkDisconnectReconnect_Resume,
            /**
             * @test (1079) GenericNetworkDisconnectReconnect_Stop Test
             */
            GenericNetworkDisconnectReconnect_Stop,
            /**
             * @test (1080) GenericNetworkDisconnectReconnect_SetPlaybackRange Test
             */
            GenericNetworkDisconnectReconnect_SetPlaybackRange,

            /**
             * @test (1081) GenericNetworkDisconnectReconnectWhileProc_AddDataSource Test
             */
            GenericNetworkDisconnectReconnectWhileProc_AddDataSource = 1081,
            /**
             * @test (1082) GenericNetworkDisconnectReconnectWhileProc_Init Test
             */
            GenericNetworkDisconnectReconnectWhileProc_Init,
            /**
             * @test (1083) GenericNetworkDisconnectReconnectWhileProc_AddDataSinkVideo Test
             */
            GenericNetworkDisconnectReconnectWhileProc_AddDataSinkVideo,
            /**
             * @test (1084) GenericNetworkDisconnectReconnectWhileProc_AddDataSinkAudio Test
             */
            GenericNetworkDisconnectReconnectWhileProc_AddDataSinkAudio,
            /**
             * @test (1085) GenericNetworkDisconnectReconnectWhileProc_Prepare Test
             */
            GenericNetworkDisconnectReconnectWhileProc_Prepare,
            /**
             * @test (1086) GenericNetworkDisconnectReconnectWhileProc_Start Test
             */
            GenericNetworkDisconnectReconnectWhileProc_Start,
            /**
             * @test (1087) GenericNetworkDisconnectReconnectWhileProc_Pause Test
             */
            GenericNetworkDisconnectReconnectWhileProc_Pause,
            /**
             * @test (1088) GenericNetworkDisconnectReconnectWhileProc_Resume Test
             */
            GenericNetworkDisconnectReconnectWhileProc_Resume,
            /**
             * @test (1089) GenericNetworkDisconnectReconnectWhileProc_Stop Test
             */
            GenericNetworkDisconnectReconnectWhileProc_Stop,
            /**
             * @test (1090) GenericNetworkDisconnectReconnectWhileProc_SetPlaybackRange Test
             */
            GenericNetworkDisconnectReconnectWhileProc_SetPlaybackRange,
            /**
             * @test (1091) GenericNetworkDisconnectCancelAll_AddDataSource Test
             */
            GenericNetworkDisconnectCancelAll_AddDataSource = 1091,
            /**
             * @test (1092) GenericNetworkDisconnectCancelAll_Init Test
             */
            GenericNetworkDisconnectCancelAll_Init,
            /**
             * @test (1093) GenericNetworkDisconnectCancelAll_AddDataSinkVideo Test
             */
            GenericNetworkDisconnectCancelAll_AddDataSinkVideo,
            /**
             * @test (1094) GenericNetworkDisconnectCancelAll_AddDataSinkAudio Test
             */
            GenericNetworkDisconnectCancelAll_AddDataSinkAudio,
            /**
             * @test (1095) GenericNetworkDisconnectCancelAll_Prepare Test
             */
            GenericNetworkDisconnectCancelAll_Prepare,
            /**
             * @test (1096) GenericNetworkDisconnectCancelAll_Start Test
             */
            GenericNetworkDisconnectCancelAll_Start,
            /**
             * @test (1097) GenericNetworkDisconnectCancelAll_Pause Test
             */
            GenericNetworkDisconnectCancelAll_Pause,
            /**
             * @test (1098) GenericNetworkDisconnectCancelAll_Resume Test
             */
            GenericNetworkDisconnectCancelAll_Resume,
            /**
             * @test (1099) GenericNetworkDisconnectCancelAll_Stop Test
             */
            GenericNetworkDisconnectCancelAll_Stop,
            /**
             * @test (1100) GenericNetworkDisconnectCancelAll_SetPlaybackRange Test
             */
            GenericNetworkDisconnectCancelAll_SetPlaybackRange,

            /**
             * @test (1101) GenericNetworkDisconnectCancelAllWhileProc_AddDataSource Test
             */
            GenericNetworkDisconnectCancelAllWhileProc_AddDataSource = 1101,
            /**
             * @test (1102) GenericNetworkDisconnectCancelAllWhileProc_Init Test
             */
            GenericNetworkDisconnectCancelAllWhileProc_Init,
            /**
             * @test (1103) GenericNetworkDisconnectCancelAllWhileProc_AddDataSinkVideo Test
             */
            GenericNetworkDisconnectCancelAllWhileProc_AddDataSinkVideo,
            /**
             * @test (1104) GenericNetworkDisconnectCancelAllWhileProc_AddDataSinkAudio Test
             */
            GenericNetworkDisconnectCancelAllWhileProc_AddDataSinkAudio,
            /**
             * @test (1105) GenericNetworkDisconnectCancelAllWhileProc_Prepare Test
             */
            GenericNetworkDisconnectCancelAllWhileProc_Prepare,
            /**
             * @test (1106) GenericNetworkDisconnectCancelAllWhileProc_Start Test
             */
            GenericNetworkDisconnectCancelAllWhileProc_Start,
            /**
             * @test (1107) GenericNetworkDisconnectCancelAllWhileProc_Pause Test
             */
            GenericNetworkDisconnectCancelAllWhileProc_Pause,
            /**
             * @test (1108) GenericNetworkDisconnectCancelAllWhileProc_Resume Test
             */
            GenericNetworkDisconnectCancelAllWhileProc_Resume,
            /**
             * @test (1109) GenericNetworkDisconnectCancelAllWhileProc_Stop Test
             */
            GenericNetworkDisconnectCancelAllWhileProc_Stop,
            /**
             * @test (1110) GenericNetworkDisconnectCancelAllWhileProc_SetPlaybackRange Test
             */
            GenericNetworkDisconnectCancelAllWhileProc_SetPlaybackRange,

            /**
             * @test (1111) GenericNetworkDisconnectCancelAllWhileProtocolRollover Test
             */
            GenericNetworkDisconnectCancelAllWhileProtocolRollover,
            /**
             * @test (1112) GenericOpenPlayMultiplePauseResumeUntilEOSTest Test
             */
            GenericOpenPlayMultiplePauseResumeUntilEOSTest = 1112,
            /**
             * @test (1113) GenericOpenPlayMultipleSeekUntilEOSTest Test
             */
            GenericOpenPlayMultipleSeekUntilEOSTest,

            /**
             * @test (1114) GenericOpenPlayStop_SleepAddDataSource Test
             */
            GenericOpenPlayStop_SleepAddDataSource = 1114,
            /**
             * @test (1115) GenericOpenPlayStop_SleepInit Test
             */
            GenericOpenPlayStop_SleepInit,
            /**
             * @test (1116) GenericOpenPlayStop_SleepAddDataSinkVideo Test
             */
            GenericOpenPlayStop_SleepAddDataSinkVideo,
            /**
             * @test (1117) GenericOpenPlayStop_SleepAddDataSinkAudio Test
             */
            GenericOpenPlayStop_SleepAddDataSinkAudio,
            /**
             * @test (1118) GenericOpenPlayStop_SleepPrepare Test
             */
            GenericOpenPlayStop_SleepPrepare,
            /**
             * @test (1119) GenericOpenPlayStop_SleepGetMetaDataValueList Test
             */
            GenericOpenPlayStop_SleepGetMetaDataValueList,
            /**
             * @test (1120) GenericOpenPlayStop_SleepStop Test
             */
            GenericOpenPlayStop_SleepStop,

            /**
             * @test (1125) GenericOpenPlayPauseResumeSeekStopProfiling Test
             */
            GenericOpenPlayPauseResumeSeekStopProfiling = 1125,
            /**
             * @test (1126) GenericOpenPlayPauseRepositionResumeUntilEOSTest Test
             */
            GenericOpenPlayPauseRepositionResumeUntilEOSTest,
            /**
             * @test (1127) GenericOpenPlayPauseRepositionResumeNetworkDisconnectCancelAllTest Test
             */
            GenericOpenPlayPauseRepositionResumeNetworkDisconnectCancelAllTest,
            /**
             * @test (1128) GenericOpenSetPlaybackRangeStartPlayStopTest Test
             */
            GenericOpenSetPlaybackRangeStartPlayStopTest,
            /**
             * @test (1129) GenericOpenPlayRepositionToEndTest Test
             */
            GenericOpenPlayRepositionToEndTest,
            /**
             * @test (1130) GenericPVMFErrorCorruptReNotified Test
             */
            GenericPVMFErrorCorruptReNotified,
            /**
             * @test (1131) GenericOpenPlayPauseGetMetaDataUntilEOSTest Test
             */
            GenericOpenPlayPauseGetMetaDataUntilEOSTest,
            /**
             * @test (1132) GenericOpenGetMetaDataPicTest Test
             */
            GenericOpenGetMetaDataPicTest,//1132

            //1133-1149 available.

            //BEGIN JANUS CPM TESTS
            /**
             * @test (1150) CleanDrmData_JanusCPMTest Test
             */
            CleanDrmData_JanusCPMTest = 1150,
            /**
             * @test (1151) LoadLicense_JanusCPMTest Test
             */
            LoadLicense_JanusCPMTest,//1151
            /**
             * @test (1152) OpenPlayStop_JanusCPMTest Test
             */
            OpenPlayStop_JanusCPMTest, //1152
            /**
             * @test (1153) PlayStopPlayStop_JanusCPMTest Test
             */
            PlayStopPlayStop_JanusCPMTest,//1153
            /**
             * @test (1154) QueryEngine_JanusCPMTest Test
             */
            QueryEngine_JanusCPMTest,//1154
            /**
             * @test (1155) StartupMeasurement_JanusCPMTest Test
             */
            StartupMeasurement_JanusCPMTest, //1155
            //Janus DLA tests
            /**
             * @test (1156) DLA_CleanDrmData_JanusCPMTest Test
             */
            DLA_CleanDrmData_JanusCPMTest,//1156
            /**
             * @test (1157) DLA_OpenPlayStop_JanusCPMTest Test
             */
            DLA_OpenPlayStop_JanusCPMTest,//1157
            /**
             * @test (1158) DLA_LicenseCapture_JanusCPMTest Test
             */
            DLA_LicenseCapture_JanusCPMTest,//1158
            /**
             * @test (1159) DLA_CancelAcquireLicense_JanusCPMTest Test
             */
            DLA_CancelAcquireLicense_JanusCPMTest,//1159
            //Janus streaming tests.
            /**
             * @test (1160) DLA_StreamingOpenPlayUntilEOST_JanusCPMTest Test
             */
            DLA_StreamingOpenPlayUntilEOST_JanusCPMTest,//1160
            /**
             * @test (1161) DLA_StreamingOpenPlayPausePlayUntilEOS_JanusCPMTest Test
             */
            DLA_StreamingOpenPlayPausePlayUntilEOS_JanusCPMTest,//1161
            /**
             * @test (1162) DLA_StreamingOpenPlaySeekPlayUntilEOS_JanusCPMTest Test
             */
            DLA_StreamingOpenPlaySeekPlayUntilEOS_JanusCPMTest,//1162
            /**
             * @test (1163) DLA_StreamingMultiplePlayUntilEOS_JanusCPMTest Test
             */
            DLA_StreamingMultiplePlayUntilEOS_JanusCPMTest,//1163
            /**
             * @test (1164) DLA_StreamingProtocolRollOverTest_JanusCPMTest Test
             */
            DLA_StreamingProtocolRollOverTest_JanusCPMTest,//1164
            /**
             * @test (1165) DLA_StreamingProtocolRollOverTestWithUnknownURLType_JanusCPMTest Test
             */
            DLA_StreamingProtocolRollOverTestWithUnknownURLType_JanusCPMTest,//1165
            /**
             * @test (1166) DLA_StreamingCancelAcquireLicense_JanusCPMTest Test
             */
            DLA_StreamingCancelAcquireLicense_JanusCPMTest,//1166
            //Janus PDL tests
            /**
             * @test (1167) DLA_PDL_OpenPlayUntilEOS_JanusCPMTest Test
             */
            DLA_PDL_OpenPlayUntilEOS_JanusCPMTest,//1167

            //this range RESERVED for future Janus tests.

            /**
             * @test (1200) FirstDLAStreamingTest Test
             */
            FirstDLAStreamingTest = 1200, //placeholder
            //note these are all Janus CPM tests

            /**
             * @test (1201) DLA_StreamingCancelAll_AddDataSource Test
             */
            DLA_StreamingCancelAll_AddDataSource = 1201,
            /**
             * @test (1202) DLA_StreamingCancelAll_Init Test
             */
            DLA_StreamingCancelAll_Init,
            /**
             * @test (1203) DLA_StreamingCancelAll_LicenseAcquired Test
             */
            DLA_StreamingCancelAll_LicenseAcquired,
            /**
             * @test (1204) DLA_StreamingCancelAll_AddDataSinkVideo Test
             */
            DLA_StreamingCancelAll_AddDataSinkVideo,
            /**
             * @test (1205) DLA_StreamingCancelAll_AddDataSinkAudio Test
             */
            DLA_StreamingCancelAll_AddDataSinkAudio,
            /**
             * @test (1206) DLA_StreamingCancelAll_Prepare Test
             */
            DLA_StreamingCancelAll_Prepare,
            /**
             * @test (1207) DLA_StreamingCancelAll_Start Test
             */
            DLA_StreamingCancelAll_Start,
            /**
             * @test (1208) DLA_StreamingCancelAll_Pause Test
             */
            DLA_StreamingCancelAll_Pause,
            /**
             * @test (1209) DLA_StreamingCancelAll_Resume Test
             */
            DLA_StreamingCancelAll_Resume,
            /**
             * @test (1210) DLA_StreamingCancelAll_Stop Test
             */
            DLA_StreamingCancelAll_Stop,
            /**
             * @test (1211) DLA_StreamingCancelAll_SetPlaybackRange Test
             */
            DLA_StreamingCancelAll_SetPlaybackRange,

            /**
             * @test (1212) DLA_StreamingCancelAllWhileProc_AddDataSource Test
             */
            DLA_StreamingCancelAllWhileProc_AddDataSource = 1212,
            /**
             * @test (1213) DLA_StreamingCancelAllWhileProc_Init Test
             */
            DLA_StreamingCancelAllWhileProc_Init,
            /**
             * @test (1214) DLA_StreamingCancelAllWhileProc_LicenseAcquired Test
             */
            DLA_StreamingCancelAllWhileProc_LicenseAcquired,
            /**
             * @test (1215) DLA_StreamingCancelAllWhileProc_AddDataSinkVideo Test
             */
            DLA_StreamingCancelAllWhileProc_AddDataSinkVideo,
            /**
             * @test (1216) DLA_StreamingCancelAllWhileProc_AddDataSinkAudio Test
             */
            DLA_StreamingCancelAllWhileProc_AddDataSinkAudio,
            /**
             * @test (1217) DLA_StreamingCancelAllWhileProc_Prepare Test
             */
            DLA_StreamingCancelAllWhileProc_Prepare,
            /**
             * @test (1218) DLA_StreamingCancelAllWhileProc_Start Test
             */
            DLA_StreamingCancelAllWhileProc_Start,
            /**
             * @test (1219) DLA_StreamingCancelAllWhileProc_Pause Test
             */
            DLA_StreamingCancelAllWhileProc_Pause,
            /**
             * @test (1220) DLA_StreamingCancelAllWhileProc_Resume Test
             */
            DLA_StreamingCancelAllWhileProc_Resume,
            /**
             * @test (1221) DLA_StreamingCancelAllWhileProc_Stop Test
             */
            DLA_StreamingCancelAllWhileProc_Stop,
            /**
             * @test (1222) DLA_StreamingCancelAllWhileProc_SetPlaybackRange Test
             */
            DLA_StreamingCancelAllWhileProc_SetPlaybackRange,

            /**
             * @test (1223) DLA_StreamingNetworkDisconnect_AddDataSource Test
             */
            DLA_StreamingNetworkDisconnect_AddDataSource = 1223,
            /**
             * @test (1224) DLA_StreamingNetworkDisconnect_Init Test
             */
            DLA_StreamingNetworkDisconnect_Init,
            /**
             * @test (1225) DLA_StreamingNetworkDisconnect_LicenseAcquired Test
             */
            DLA_StreamingNetworkDisconnect_LicenseAcquired,
            /**
             * @test (1226) DLA_StreamingNetworkDisconnect_AddDataSinkVideo Test
             */
            DLA_StreamingNetworkDisconnect_AddDataSinkVideo,
            /**
             * @test (1227) DLA_StreamingNetworkDisconnect_AddDataSinkAudio Test
             */
            DLA_StreamingNetworkDisconnect_AddDataSinkAudio,
            /**
             * @test (1228) DLA_StreamingNetworkDisconnect_Prepare Test
             */
            DLA_StreamingNetworkDisconnect_Prepare,
            /**
             * @test (1229) DLA_StreamingNetworkDisconnect_Start Test
             */
            DLA_StreamingNetworkDisconnect_Start,
            /**
             * @test (1230) DLA_StreamingNetworkDisconnect_Pause Test
             */
            DLA_StreamingNetworkDisconnect_Pause,
            /**
             * @test (1231) DLA_StreamingNetworkDisconnect_Resume Test
             */
            DLA_StreamingNetworkDisconnect_Resume,
            /**
             * @test (1232) DLA_StreamingNetworkDisconnect_Stop Test
             */
            DLA_StreamingNetworkDisconnect_Stop,
            /**
             * @test (1233) DLA_StreamingNetworkDisconnect_SetPlaybackRange Test
             */
            DLA_StreamingNetworkDisconnect_SetPlaybackRange,

            /**
             * @test (1234) DLA_StreamingNetworkDisconnectWhileProc_AddDataSource Test
             */
            DLA_StreamingNetworkDisconnectWhileProc_AddDataSource = 1234,
            /**
             * @test (1235) DLA_StreamingNetworkDisconnectWhileProc_Init Test
             */
            DLA_StreamingNetworkDisconnectWhileProc_Init,
            /**
             * @test (1236) DLA_StreamingNetworkDisconnectWhileProc_LicenseAcquired Test
             */
            DLA_StreamingNetworkDisconnectWhileProc_LicenseAcquired,
            /**
             * @test (1237) DLA_StreamingNetworkDisconnectWhileProc_AddDataSinkVideo Test
             */
            DLA_StreamingNetworkDisconnectWhileProc_AddDataSinkVideo,
            /**
             * @test (1238) DLA_StreamingNetworkDisconnectWhileProc_AddDataSinkAudio Test
             */
            DLA_StreamingNetworkDisconnectWhileProc_AddDataSinkAudio,
            /**
             * @test (1239) DLA_StreamingNetworkDisconnectWhileProc_Prepare Test
             */
            DLA_StreamingNetworkDisconnectWhileProc_Prepare,
            /**
             * @test (1240) DLA_StreamingNetworkDisconnectWhileProc_Start Test
             */
            DLA_StreamingNetworkDisconnectWhileProc_Start,
            /**
             * @test (1241) DLA_StreamingNetworkDisconnectWhileProc_Pause Test
             */
            DLA_StreamingNetworkDisconnectWhileProc_Pause,
            /**
             * @test (1242) DLA_StreamingNetworkDisconnectWhileProc_Resume Test
             */
            DLA_StreamingNetworkDisconnectWhileProc_Resume,
            /**
             * @test (1243) DLA_StreamingNetworkDisconnectWhileProc_Stop Test
             */
            DLA_StreamingNetworkDisconnectWhileProc_Stop,
            /**
             * @test (1244) DLA_StreamingNetworkDisconnectWhileProc_SetPlaybackRange Test
             */
            DLA_StreamingNetworkDisconnectWhileProc_SetPlaybackRange,

            /**
             * @test (1245) DLA_StreamingNetworkDisconnectReconnect_AddDataSource Test
             */
            DLA_StreamingNetworkDisconnectReconnect_AddDataSource = 1245,
            /**
             * @test (1246) DLA_StreamingNetworkDisconnectReconnect_Init Test
             */
            DLA_StreamingNetworkDisconnectReconnect_Init,
            /**
             * @test (1247) DLA_StreamingNetworkDisconnectReconnect_LicenseAcquired Test
             */
            DLA_StreamingNetworkDisconnectReconnect_LicenseAcquired,
            /**
             * @test (1248) DLA_StreamingNetworkDisconnectReconnect_AddDataSinkVideo Test
             */
            DLA_StreamingNetworkDisconnectReconnect_AddDataSinkVideo,
            /**
             * @test (1249) DLA_StreamingNetworkDisconnectReconnect_AddDataSinkAudio Test
             */
            DLA_StreamingNetworkDisconnectReconnect_AddDataSinkAudio,
            /**
             * @test (1250) DLA_StreamingNetworkDisconnectReconnect_Prepare Test
             */
            DLA_StreamingNetworkDisconnectReconnect_Prepare,
            /**
             * @test (1251) DLA_StreamingNetworkDisconnectReconnect_Start Test
             */
            DLA_StreamingNetworkDisconnectReconnect_Start,
            /**
             * @test (1252) DLA_StreamingNetworkDisconnectReconnect_Pause Test
             */
            DLA_StreamingNetworkDisconnectReconnect_Pause,
            /**
             * @test (1253) DLA_StreamingNetworkDisconnectReconnect_Resume Test
             */
            DLA_StreamingNetworkDisconnectReconnect_Resume,
            /**
             * @test (1254) DLA_StreamingNetworkDisconnectReconnect_Stop Test
             */
            DLA_StreamingNetworkDisconnectReconnect_Stop,
            /**
             * @test (1255) DLA_StreamingNetworkDisconnectReconnect_SetPlaybackRange Test
             */
            DLA_StreamingNetworkDisconnectReconnect_SetPlaybackRange,
            /**
             * @test (1256) DLA_StreamingNetworkDisconnectReconnectWhileProc_AddDataSource Test
             */
            DLA_StreamingNetworkDisconnectReconnectWhileProc_AddDataSource = 1256,
            /**
             * @test (1257) DLA_StreamingNetworkDisconnectReconnectWhileProc_Init Test
             */
            DLA_StreamingNetworkDisconnectReconnectWhileProc_Init,
            /**
             * @test (1258) DLA_StreamingNetworkDisconnectReconnectWhileProc_LicenseAcquired Test
             */
            DLA_StreamingNetworkDisconnectReconnectWhileProc_LicenseAcquired,
            /**
             * @test (1259) DLA_StreamingNetworkDisconnectReconnectWhileProc_AddDataSinkVideo Test
             */
            DLA_StreamingNetworkDisconnectReconnectWhileProc_AddDataSinkVideo,
            /**
             * @test (1260) DLA_StreamingNetworkDisconnectReconnectWhileProc_AddDataSinkAudio Test
             */
            DLA_StreamingNetworkDisconnectReconnectWhileProc_AddDataSinkAudio,
            /**
             * @test (1261) DLA_StreamingNetworkDisconnectReconnectWhileProc_Prepare Test
             */
            DLA_StreamingNetworkDisconnectReconnectWhileProc_Prepare,
            /**
             * @test (1262) DLA_StreamingNetworkDisconnectReconnectWhileProc_Start Test
             */
            DLA_StreamingNetworkDisconnectReconnectWhileProc_Start,
            /**
             * @test (1263) DLA_StreamingNetworkDisconnectReconnectWhileProc_Pause Test
             */
            DLA_StreamingNetworkDisconnectReconnectWhileProc_Pause,
            /**
             * @test (1264) DLA_StreamingNetworkDisconnectReconnectWhileProc_Resume Test
             */
            DLA_StreamingNetworkDisconnectReconnectWhileProc_Resume,
            /**
             * @test (1265) DLA_StreamingNetworkDisconnectReconnectWhileProc_Stop Test
             */
            DLA_StreamingNetworkDisconnectReconnectWhileProc_Stop,
            /**
             * @test (1266) DLA_StreamingNetworkDisconnectReconnectWhileProc_SetPlaybackRange Test
             */
            DLA_StreamingNetworkDisconnectReconnectWhileProc_SetPlaybackRange,
            /**
             * @test (1267) DLA_StreamingNetworkDisconnectCancelAll_AddDataSource Test
             */
            DLA_StreamingNetworkDisconnectCancelAll_AddDataSource = 1267,
            /**
             * @test (1268) DLA_StreamingNetworkDisconnectCancelAll_Init Test
             */
            DLA_StreamingNetworkDisconnectCancelAll_Init,
            /**
             * @test (1269) DLA_StreamingNetworkDisconnectCancelAll_LicenseAcquired Test
             */
            DLA_StreamingNetworkDisconnectCancelAll_LicenseAcquired,
            /**
             * @test (1270) DLA_StreamingNetworkDisconnectCancelAll_AddDataSinkVideo Test
             */
            DLA_StreamingNetworkDisconnectCancelAll_AddDataSinkVideo,
            /**
             * @test (1271) DLA_StreamingNetworkDisconnectCancelAll_AddDataSinkAudio Test
             */
            DLA_StreamingNetworkDisconnectCancelAll_AddDataSinkAudio,
            /**
             * @test (1272) DLA_StreamingNetworkDisconnectCancelAll_Prepare Test
             */
            DLA_StreamingNetworkDisconnectCancelAll_Prepare,
            /**
             * @test (1273) DLA_StreamingNetworkDisconnectCancelAll_Start Test
             */
            DLA_StreamingNetworkDisconnectCancelAll_Start,
            /**
             * @test (1274) DLA_StreamingNetworkDisconnectCancelAll_Pause Test
             */
            DLA_StreamingNetworkDisconnectCancelAll_Pause,
            /**
             * @test (1275) DLA_StreamingNetworkDisconnectCancelAll_Resume Test
             */
            DLA_StreamingNetworkDisconnectCancelAll_Resume,
            /**
             * @test (1276) DLA_StreamingNetworkDisconnectCancelAll_Stop Test
             */
            DLA_StreamingNetworkDisconnectCancelAll_Stop,
            /**
             * @test (1277) DLA_StreamingNetworkDisconnectCancelAll_SetPlaybackRange Test
             */
            DLA_StreamingNetworkDisconnectCancelAll_SetPlaybackRange,
            /**
             * @test (1278) DLA_StreamingNetworkDisconnectCancelAllWhileProc_AddDataSource Test
             */
            DLA_StreamingNetworkDisconnectCancelAllWhileProc_AddDataSource = 1278,
            /**
             * @test (1279) DLA_StreamingNetworkDisconnectCancelAllWhileProc_Init Test
             */
            DLA_StreamingNetworkDisconnectCancelAllWhileProc_Init,
            /**
             * @test (1280) DLA_StreamingNetworkDisconnectCancelAllWhileProc_LicenseAcquired Test
             */
            DLA_StreamingNetworkDisconnectCancelAllWhileProc_LicenseAcquired,
            /**
             * @test (1281) DLA_StreamingNetworkDisconnectCancelAllWhileProc_AddDataSinkVideo Test
             */
            DLA_StreamingNetworkDisconnectCancelAllWhileProc_AddDataSinkVideo,
            /**
             * @test (1282) DLA_StreamingNetworkDisconnectCancelAllWhileProc_AddDataSinkAudio Test
             */
            DLA_StreamingNetworkDisconnectCancelAllWhileProc_AddDataSinkAudio,
            /**
             * @test (1283) DLA_StreamingNetworkDisconnectCancelAllWhileProc_Prepare Test
             */
            DLA_StreamingNetworkDisconnectCancelAllWhileProc_Prepare,
            /**
             * @test (1284) DLA_StreamingNetworkDisconnectCancelAllWhileProc_Start Test
             */
            DLA_StreamingNetworkDisconnectCancelAllWhileProc_Start,
            /**
             * @test (1285) DLA_StreamingNetworkDisconnectCancelAllWhileProc_Pause Test
             */
            DLA_StreamingNetworkDisconnectCancelAllWhileProc_Pause,
            /**
             * @test (1286) DLA_StreamingNetworkDisconnectCancelAllWhileProc_Resume Test
             */
            DLA_StreamingNetworkDisconnectCancelAllWhileProc_Resume,
            /**
             * @test (1287) DLA_StreamingNetworkDisconnectCancelAllWhileProc_Stop Test
             */
            DLA_StreamingNetworkDisconnectCancelAllWhileProc_Stop,
            /**
             * @test (1288) DLA_StreamingNetworkDisconnectCancelAllWhileProc_SetPlaybackRange Test
             */
            DLA_StreamingNetworkDisconnectCancelAllWhileProc_SetPlaybackRange,

            /**
             * @test (1289) DLA_StreamingOpenPlayMultiplePauseResumeUntilEOSTest Test
             */
            DLA_StreamingOpenPlayMultiplePauseResumeUntilEOSTest = 1289,
            /**
             * @test (1290) DLA_StreamingOpenPlayMultipleSeekUntilEOSTest Test
             */
            DLA_StreamingOpenPlayMultipleSeekUntilEOSTest,
            /**
             * @test (1291) DLA_StreamingOpenPlayStop_SleepAddDataSource Test
             */
            DLA_StreamingOpenPlayStop_SleepAddDataSource = 1291,
            /**
             * @test (1292) DLA_StreamingOpenPlayStop_SleepInit Test
             */
            DLA_StreamingOpenPlayStop_SleepInit,
            /**
             * @test (1293) DLA_StreamingOpenPlayStop_SleepLicenseAcquired Test
             */
            DLA_StreamingOpenPlayStop_SleepLicenseAcquired,
            /**
             * @test (1294) DLA_StreamingOpenPlayStop_SleepAddDataSinkVideo Test
             */
            DLA_StreamingOpenPlayStop_SleepAddDataSinkVideo,
            /**
             * @test (1295) DLA_StreamingOpenPlayStop_SleepAddDataSinkAudio Test
             */
            DLA_StreamingOpenPlayStop_SleepAddDataSinkAudio,
            /**
             * @test (1296) DLA_StreamingOpenPlayStop_SleepPrepare Test
             */
            DLA_StreamingOpenPlayStop_SleepPrepare,
            /**
             * @test (1301) DLA_StreamingOpenPlayStop_SleepStop Test
             */
            DLA_StreamingOpenPlayStop_SleepStop = 1301,
            //END JANUS CPM TESTS

            /**
             * @test (1302) GenericOpenPlaySeekBeyondClipDurationTest Test
             */
            GenericOpenPlaySeekBeyondClipDurationTest = 1302,

            //tests various modes of track selection
            //refer to test_pv_player_engine_testset_apptrackselection.h
            //for definition of modes
            ApplicationInvolvedTrackSelectionTestDefault = 1303,
            ApplicationInvolvedTrackSelectionTestPassthru = 1304,
            ApplicationInvolvedTrackSelectionTestAudioOnly = 1305,
            ApplicationInvolvedTrackSelectionTestVideoOnly = 1306,
            ApplicationInvolvedTrackSelectionTestTextOnly = 1307,
            ApplicationInvolvedTrackSelectionTestNoTracks = 1308,

            //Metadata Query tests
            DLA_QueryEngine_JupiterCPMTest_v2_WMA = 1400,
            DLA_QueryEngine_JupiterCPMTest_v24_WMA,//1401
            DLA_QueryEngine_JupiterCPMTest_v4_Enveloped_ASF,//1402
            DLA_QueryEngine_JupiterCPMTest_v4_Enveloped_MP4,//1403
            DLA_QueryEngine_JupiterCPMTest_v4_Enveloped_Image,//1404
            //Enveloped image tests
            DLA_EnvelopeRead_JupiterCPMTest_v4_Enveloped_Image,//1405
            //Multi-media format tests
            DLA_OpenPlayStop_JupiterCPMTest_v2_WMA,//1406
            DLA_OpenPlayStop_JupiterCPMTest_v2_WMV,//1407
            DLA_OpenPlayStop_JupiterCPMTest_v24_WMA,//1408
            DLA_OpenPlayStop_JupiterCPMTest_v24_WMV,//1409
            DLA_OpenPlayStop_JupiterCPMTest_v4_WMA,//1410
            DLA_OpenPlayStop_JupiterCPMTest_v4_WMV,//1411
            DLA_OpenPlayStop_JupiterCPMTest_v4_AAC,//1412
            DLA_OpenPlayStop_JupiterCPMTest_v4_H264,//1413
            DLA_OpenPlayStop_JupiterCPMTest_v4_Enveloped_ASF,//1414
            DLA_OpenPlayStop_JupiterCPMTest_v4_Enveloped_MP4,//1415
            DLA_OpenPlayStop_JupiterCPMTest_unprotected_H264,//1416
            DLA_OpenPlayStop_JupiterCPMTest_unprotected_AAC,//1417
            //Special license protocol sequence tests
            DLA_OpenPlayStop_JupiterCPMTest_redirect,//1418
            DLA_OpenPlayStop_JupiterCPMTest_v24_WMA_fallback,//1419
            DLA_OpenPlayStop_JupiterCPMTest_v4_WMA_ringtone,//1420
            DLA_OpenPlayStop_JupiterCPMTest_v4_WMA_domain,//1421
            DLA_OpenPlayStop_JupiterCPMTest_v4_WMA_domain_renew,//1422
            DLA_OpenPlayStop_JupiterCPMTest_v4_WMA_domain_offline,//1423
            DLA_OpenPlayStop_JupiterCPMTest_v4_WMA_domain_history,//1424
            //Jupiter utility tests
            DLA_JoinDomain_JupiterCPMTest,//1425
            DLA_LeaveDomain_JupiterCPMTest,//1426
            DLA_DeleteLicense_JupiterCPMTest_v2_Content,//1427
            DLA_DeleteLicense_JupiterCPMTest_v24_Content,//1428
            DLA_MeteringByCert_JupiterCPMTest,//1429
            DLA_MeteringByMID_JupiterCPMTest,//1430
            DLA_MeteringAll_JupiterCPMTest,//1431
            DLA_LicenseUpdateAll_JupiterCPMTest,//1432
            DLA_LicenseUpdateExpired_JupiterCPMTest,//1433
            //Jupiter CancelAcquireLicense tests
            DLA_CancelAcquireLicense_JupiterCPMTest_v4_Enveloped_Image,//1434
            DLA_CancelAcquireLicense_JupiterCPMTest_v2_Content,//1435
            DLA_CancelAcquireLicense_JupiterCPMTest_v24_Content,//1436
            DLA_CancelAcquireLicense_JupiterCPMTest_v4_Enveloped_MP4,//1437
            //Jupiter streaming tests.
            DLA_StreamingOpenPlayUntilEOST_JupiterCPMTest,//1438
            DLA_StreamingOpenPlayPausePlayUntilEOS_JupiterCPMTest,//1439
            DLA_StreamingOpenPlaySeekPlayUntilEOS_JupiterCPMTest,//1440
            DLA_StreamingMultiplePlayUntilEOS_JupiterCPMTest,//1441
            DLA_StreamingCancelAcquireLicense_JupiterCPMTest,//1442
            DLA_StreamingProtocolRollOverTest_JupiterCPMTest,//1443
            DLA_StreamingProtocolRollOverTestWithUnknownURLType_JupiterCPMTest,//1444
            //RESERVED FOR FUTURE JUPITER CPM TESTS.
            LastJupiterCPMTest = 1599,//placeholder
            //End JUPITER CPM TESTS.
            /*
             * Note: Starting PVR tests from 1600 since the number of tests exceed the range
             * from 895 and 900 (the earlier location of the tests)
             */
            /*
             * =========== Basic PVR tests ===========
             */


            /**
            PVR Memory Live Buffer tests start at 1600
            */

            /**
             * @test (1600) PVR MLB:Play->pause->resume->stop test
             * Description: Self-explanatory
             */
            PVR_MLB_StreamingOpenPlayLivePausePlayStopTest = 1600,//1600
            /**
             * @test (1601) PVR MLB:Reposition after a play pause
             * Description: See TC 1611
             */
            PVR_MLB_StreamingOpenPlayLivePausePlaySeekStopTest,//1601
            /**
             * @test (1602) PVR MLB:Reposition after a play pause
             * Description: See TC 1611
             */
            PVR_MLB_StreamingOpenPlayLiveBufferBoundaryCheckTest,//1602
            /**
             * @test (1602) PVR MLB:Check live buffer boundary
             * Description: To maintain TC numbers, this test case
             *              does a play followed by a stop.
             */

            /*
             * =========== Live pause tests ===========
             */

            /**
             * @test (1603) PVR MLB:Quick successive pause/resume
             * Description: This test case will call pause/resume in a quick and
             *              repetitive sequence to test quick successive pause/resume stress condition.
             * Input:       1. repeat count
             */
            PVR_MLB_StreamingOpenPlayMultipleLivePausePlayTest,//1603
            /**
             * @test (1604) PVR MLB:Pause with random duration
             * Description: This test case will call pause/resume repeatedly.
             *              Each pause has random duration in a range.
             * Inputs:      1. repeat count.
             *              2. pause duration range
             */
            PVR_MLB_StreamingOpenPlayMultipleLivePauseRandomDurationPlayTest,//1604
            /**
             * @test (1605) PVR MLB:Long pause
             * Description: This test case will pause long period of time (> live buffer duration)
             * Input:       1. pause duration
             */
            PVR_MLB_StreamingOpenPlayLongLivePausePlayTest,//1605
            /**
             * @test (1606) PVR MLB:Stop during live pause
             * Description: This test case stop the session during live pause.
             *              Then it starts the same session again.
             */
            PVR_MLB_StreamingOpenPlayLivePauseStopTest,//1606

            /*
             * =========== Random position tests ===========
             */

            /**
             * @test (1607) PVR MLB:Jump to live after pause
             * Description: This test case jumps to live session (not buffer) after a pause.
             *              The above step is repeated several times, after pausing for a random duration.
             * Inputs:      1. Repeat count
             *              2. Pause duration range
             */
            PVR_MLB_StreamingOpenPauseJumpToLiveStopTest, //1607
            /**
             * @test (1608) PVR MLB:Jump to live after pause/resume.
             * Description: This test case pauses a live session for a random duration, resumes
             *              a for random duration that is less than the pause duration, and then
             *              jumps to live. This is repeated several times.
             * Inputs:      1. Repeat count
             *              2. Pause duration range
             *              3. Resume duration range
             */
            PVR_MLB_StreamingOpenPauseResumeJumpToLiveStopTest, //1608
            /*
             * 1609:        Stop during playing back from live buffer
             * Description: This test case pauses a live session for a random duration, resumes
             *              and then stops the session while playing from the live buffer. The same
             *              session is started after the stop.
             */
            PVR_MLB_StreamingOpenPauseResumeStopTest, //1609
            /**
             * @test (1610) PVR MLB:PVR RTSP URL test
             * Description: This test case uses an RTSP URL as the input for the streaming session.
             *              Both live and VOD cases are handled.
             */
            PVR_MLB_StreamingRTSPUrlPauseResumeStopTest, //1610

            /**
             * @test (1611) PVR MLB:PVR Repositioning test
             * Description: This test case does a sequence of repositioning requests within the live buffer.
             *              The followed sequence is: start, pause, resume, repositioning sequence, stop.
                            The repos. sequence consists of 6 repositioning requests back and forward.
             */
            PVR_MLB_StreamingPauseResumeRepositionStopTest, //1611

            /**
             * @test (1612) PVR MLB:PVR Repositioning test
             * Description: This test case does a sequence of repositioning requests within the live buffer.
             *              The followed sequence is: start, pause, repositioning, resume. This sequence is
                        done six times, followed by a stop.
            */
            PVR_MLB_StreamingPauseRepositionResumeStopTest, //1612

            /**
             * @test (1613) PVR MLB:RTSP END_OF_STREAM test for memory live buffer.
             * Description: This test is design to test the case where and RTSP END_OF_STREAM arrives
             *              during a live pause. Total playback time must equal the cumulative playlist clip
             *              duration.
             * Input:
             */
            PVR_MLB_StreamingOpenPlayLivePauseWaitForRTSPEOSResumeTest = 1613,//1613


            /**
             * @test (1614) PVR MLB:Playlist switch with a full live buffer after JTL
             * Description: This test case makes a live pause it resumes once the live buffer gets full, and makes
             *              a JTL followed by a channel switch, the channel switch time is measured.
            */
            PVR_MLB_StreamingOpenPauseJmpToLiveChannelSwitchTest, //1614

            /**
             * @test (1615) PVR MLB:Bitrate estimation test
            * Description: Test case for mismatch between advertised and real bitrate,
            *              the root of the problem with higher-than-advertised bitrates is that it
            *              may cause overwrites in the LiveBuffer, which is then reflected in quality artifacts.
            *
            *              PLEASE USE A SDP FILE THAT ANOUNCES A SMALLER BITRATE THAN THE REAL ONE
            */
            PVR_MLB_StreamingBitrateEstimationTest = 1615,

            /**
            PVR FileSystem Live Buffer tests start at 1630
            */
            PVR_FSLB_StreamingOpenPlayLivePausePlayStopTest = 1630,//1630
            PVR_FSLB_StreamingOpenPlayLivePausePlaySeekStopTest,//1631
            PVR_FSLB_StreamingOpenPlayLiveBufferBoundaryCheckTest,//1632
            /*
             * =========== Live pause tests ===========
             */

            /*
             * 1633:        Quick successive pause/resume
             * Description: This test case will call pause/resume in a quick and
             *              repetitive sequence to test quick successive pause/resume stress condition.
             * Input:       1. repeat count
             */
            PVR_FSLB_StreamingOpenPlayMultipleLivePausePlayTest,//1633
            /*
             * 1634:        Pause with random duration
             * Description: This test case will call pause/resume repeatedly.
             *              Each pause has random duration in a range.
             * Inputs:      1. repeat count.
             *              2. pause duration range
             */
            PVR_FSLB_StreamingOpenPlayMultipleLivePauseRandomDurationPlayTest,//1634
            /*
             * 1635:        Long pause
             * Description: This test case will pause long period of time (> live buffer duration)
             * Input:       1. pause duration
             */
            PVR_FSLB_StreamingOpenPlayLongLivePausePlayTest,//1635
            /*
             * 1636:        Stop during live pause
             * Description: This test case stop the session during live pause.
             *              Then it starts the same session again.
             */
            PVR_FSLB_StreamingOpenPlayLivePauseStopTest,//1636

            /*
             * =========== Random position tests ===========
             */

            /*
             * 1637:        Jump to live after pause
             * Description: This test case jumps to live session (not buffer) after a pause.
             *              The above step is repeated several times, after pausing for a random duration.
             * Inputs:      1. Repeat count
             *              2. Pause duration range
             */
            PVR_FSLB_StreamingOpenPauseJumpToLiveStopTest, //1637
            /*
             * 1638:        Jump to live after pause/resume
             * Description: This test case pauses a live session for a random duration, resumes
             *              a for random duration that is less than the pause duration, and then
             *              jumps to live. This is repeated several times.
             * Inputs:      1. Repeat count
             *              2. Pause duration range
             *              3. Resume duration range
             */
            PVR_FSLB_StreamingOpenPauseResumeJumpToLiveStopTest, //1638
            /*
             * 1639:        Stop during playing back from live buffer
             * Description: This test case pauses a live session for a random duration, resumes
             *              and then stops the session while playing from the live buffer. The same
             *              session is started after the stop.
             */
            PVR_FSLB_StreamingOpenPauseResumeStopTest, //1639

            /*
             * 1640:        RTSP URL test
             * Description: This test case uses an RTSP URL as the input for the streaming session.
             *              Both live and VOD cases are handled.
             */
            PVR_FSLB_StreamingRTSPUrlPauseResumeStopTest, //1640

            /*
             * 1641:        PVR Repositioning test
             * Description: This test case does a sequence of repositioning requests within the live buffer.
             *              The followed sequence is: start, pause, resume, repositioning sequence, stop.
                            The repos. sequence consists of 6 repositioning requests back and forward.
             */
            PVR_FSLB_StreamingPauseResumeRepositionStopTest, //1641

            /*
            * 1642:        PVR Repositioning test
            * Description: This test case does a sequence of repositioning requests within the live buffer.
            *              The followed sequence is: start, pause, repositioning, resume. This sequence is
                        done six times, followed by a stop.
            */
            PVR_FSLB_StreamingPauseRepositionResumeStopTest, //1642

            /*
            * 1643:        RTSP END_OF_STREAM test for file system live buffer.
            * Description: Same as 1643.
            * Input:
            */

            PVR_FSLB_StreamingOpenPlayLivePauseWaitForRTSPEOSResumeTest, //1643

            /*
            * 1644:        PVR Playlist switch with a full live buffer after JTL
            * Description: This test case makes a live pause it resumes once the live buffer gets full, and makes
            *              a JTL followed by a channel switch, the channel switch time is measured.

            */
            PVR_FSLB_StreamingOpenPauseJmpToLiveChannelSwitchTest, //1644


            /**
            PVR local file playback tests start at 1660
            */

            PVR_FILEPLAYBACK_OpenPlayUntilEOFTest = 1660,// 1660

            /*
            * 1661       PVR Local Playback Open-Play-Reposition-Play-Stop
            * Description: This test case plays a local pvr file for 10 seconds, then
            *              it repositions to 50, 75, 40, 0, 60 and 95% of the local file
            *              and then it stops playback.
            *

            */

            PVR_FILEPLAYBACK_OpenPlayRepositionPlayStopTest,  // 1661


            /*
            * 1662       PVR Local Playback Open-Play-Pause-Reposition
            * Description: This test case plays a local pvr file for 10 seconds, then pauses playback
            *              it repositions to 50, 75, 40, 0, 60 and 95% of the local file
            *              and then it stops playback.
            *

            */

            PVR_FILEPLAYBACK_OpenPlayPauseRepositionTest,  // 1662

            /*
            * 1663       PVR Local Playback Retrieved duration is correct
            * Description: This test case plays a local pvr file for 3 seconds and
            *              verifies if the retrieved duration is correct
            *

            */

            PVR_FILEPLAYBACK_OpenPlayVerifyDurationTest,  // 1663




            /*===========================PVR Recorder Test cases============================*/

            /*
            * 1680       PVR_Recorder_RecordOnStartUp
            * Description: - Recording will start from NPT zero until playback is stopped.
            *              - Pass criteria will be: No playback or recording errors.
            */

            PVR_Recorder_RecordOnStartUp = 1680,

            /*
            * 1681       PVR_Recorder_RecordOnDemand
            * Description: - Recording will start from an arbitrary NPT different than zero to a point before playback is stopped.
            *              - Pass criteria will be: No playback or recording errors.
            */

            PVR_Recorder_RecordOnDemand,

            /*
            * 1682       PVR_Recorder_MultiplePause
            * Description: - Recording will start from NPT zero.
            *              - Multiple pause/resume resume will be performed
            *              - Pass criteria will be: No playback or recording errors.
            */

            PVR_Recorder_MultiplePause,

            /*
            * 1683       PVR_Recorder_MultiplePauseJTL
            * Description: - Recording will start from NPT zero.
            *              - Multiple pause/resume will be performed
            *              - After last pause JTL
            *              - Pass criteria will be: No playback or recording errors.
            */

            PVR_Recorder_MultiplePauseJTL,

            /*
            * 1684       PVR_Recorder_MultipleRepos
            * Description: - Recording will start from NPT zero.
            *              - Multiple Reposition/resume will be performed
            *              - Pass criteria will be: No playback or recording errors.
            */

            PVR_Recorder_MultipleRepos,

            /*
            * 1685       PVR_Recorder_MultipleRecord
            * Description: - Recording will start from an arbitrary NPT different than zero.
            *              - Multiple StartRecord/StopRecord will be performed
            *              - Pass criteria will be: No playback or recording errors
            */

            PVR_Recorder_MultipleRecord,

            LastInteractiveTest = 2000, // placeholder


            BeyondLastTest = 9999 //placeholder
        };

        // From test_case
        virtual void test();

        // From pvplayer_async_test_observer
        void TestCompleted(test_case&);

        //Validating file format specific player test cases
        bool ValidateTestCase(int& aCurrentTestCaseNumber);

        void SetupLoggerScheduler();

    private:
        const char *iFileName;
        PVMFFormatType iFileType;

        int iCurrentTestNumber;
        pvplayer_async_test_base* iCurrentTest;
        int32 iFirstTest;
        int32 iLastTest;
        bool iCompressedVideoOutput;
        bool iCompressedAudioOutput;
        bool iFileInput;
        bool iBCS;
        //for proxy support
        bool iProxyEnabled;

        // For test results
        int iTotalSuccess;
        int iTotalError;
        int iTotalFail;

        // For logging
        int32 iLogLevel;
        int32 iLogNode;
        int32 iLogFile;
        int32 iLogMem;

        // For memory statistics
        uint32 iTotalAlloc;
        uint32 iTotalBytes;
        uint32 iAllocFails;
        uint32 iNumAllocs;
        //Flag for checking the file type
        int32 iFileFormatType;

        uint32 iDownloadRateInKbps;
};

/**
 * Factory class for project specific unit test cases
 */
class PVPlayerProjTestFactory
{
    public:
        /**
         * Creates customer project test case object as specified by test case number.  Default
         * implementation returns NULL.  Customer projects that wish to extend pvPlayer unit
         * test to implement customer specific test cases should implement this function
         *
         * @param aTestCaseNumber - Unit test case number
         * @param aTestParam - Parameters to the unit test case
         * @return Newly created project specific test case object, or NULL if the test case
         *              number is not supported.
         */
        virtual pvplayer_async_test_base* GetProjTest(uint aTestCaseNumber, PVPlayerAsyncTestParam aTestParam)
        {
            OSCL_UNUSED_ARG(aTestCaseNumber);
            OSCL_UNUSED_ARG(aTestParam);
            return NULL;
        }
};

#endif


