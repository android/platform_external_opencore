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
/**
    @file OMX_testapp\include\omxenctest.h

*/

#ifndef OMXENCTEST_H_INCLUDED
#define OMXENCTEST_H_INCLUDED

#ifndef OMXENCTESTBASE_H_INCLUDED
#include "omxenctestbase.h"
#endif

enum EncTests
{
    GET_ROLES_TEST = 0,
    PARAM_NEGOTIATION_TEST,
    NORMAL_SEQ_TEST ,
    USE_BUFFER_TEST,
    BUFFER_BUSY_TEST,
    PARTIAL_FRAMES_TEST,
    EXTRA_PARTIAL_FRAMES_TEST,
    PAUSE_RESUME_TEST,
    ENDOFSTREAM_MISSING_TEST,
    WITHOUT_MARKER_BIT_TEST,
};


/* Macro to reset the structure for GetParemeter call and also to set the size and version of it*/
#define INIT_GETPARAMETER_STRUCT(name, str)\
    memset(&(str), 0x0, sizeof(name));\
    (str).nSize = sizeof (name);\
    (str).nVersion.s.nVersionMajor = 0x1;\
    (str).nVersion.s.nVersionMinor = 0x0;\
    (str).nVersion.s.nRevision = 0x0;\
    (str).nVersion.s.nStep = 0x0


//Macro to verify whether some error occured in the openmax API or not
//e is the return value of the API and s is the string to be printed with the error
#define CHECK_ERROR(e, s) \
    if (OMX_ErrorNone != (e))\
    {\
        printf("%s Error, Stop the test case\n", s);\
        iState = StateError;\
        RunIfNotReady();\
        break;\
    }


//Macro to verify whether some error occured in memory allocation or not
//m is the pointer to which memory has been allocated and s is the string to be printed in case of error
#define CHECK_MEM(m, s) \
    if (NULL == (m))\
    {\
        printf("%s Memory Allocation Failed\n", s);\
        iState = StateError;\
        RunIfNotReady();\
        break;\
    }



#define PRINT_RESULT
#define MAX_SIZE 4000
#define ROLE_SIZE 20

enum AMR_SETTING_TYPE
{
    INPUT_FILE_NAME_AMR,
    OUTPUT_FILE_NAME_AMR,
    INPUT_BITS_PER_SAMPLE,
    INPUT_SAMPLING_FREQUENCY,
    INPUT_NUMBER_OF_CHANNELS,
    OUTPUT_BAND_MODE,
    OUTPUT_FORMAT,
    REFERENCE_FILE_NAME,
};

enum M4V_SETTING_TYPE
{
    INPUT_FILE_NAME_M4V,
    OUTPUT_FILE_NAME_M4V,
    INPUT_WIDTH_M4V,
    INPUT_HEIGHT_M4V,
    INPUT_FRAMERATE_M4V,
    INPUT_FORMAT_M4V,
    ENCODE_WIDTH_M4V,
    ENCODE_HEIGHT_M4V,
    ENCODE_BITRATE_M4V,
    ENCODE_FRAMERATE_M4V,
    ENCODE_NUM_PFRAMES_M4V,
    ENCODE_CONTENTTYPE,
    ENCODE_RATECONTROLTYPE_M4V,
    ENCODE_IQUANT_M4V,
    ENCODE_PQUANT_M4V,
    ENCODE_SEARCHRANGE_M4V,
    ENCODE_MV8x8,
    ENCODE_INTRAREFRESHTYPE_M4V,
    ENCODE_NUMINTRAMB_M4V,
    ENCODE_PACKETSIZE,
    ENCODE_PROFILE_M4V,
    ENCODE_LEVEL_M4V,
    ENCODE_RESYNC,
    ENCODE_DATAPARTITION,
    ENCODE_SHORTHEADER,
    ENCODE_RESYNC_MARKER_SPACING,
    ENCODE_REVERSIBLE_VLC,
    ENCODE_TIME_INCREMENT_RESOLUTION,
    ENCODE_GOBHEADER_INTERVAL
};


enum AVC_SETTING_TYPE
{
    INPUT_FILE_NAME,
    OUTPUT_FILE_NAME,
    INPUT_WIDTH,
    INPUT_HEIGHT,
    INPUT_FRAMERATE,
    INPUT_FORMAT,
    ENCODE_WIDTH,
    ENCODE_HEIGHT,
    ENCODE_BITRATE,
    ENCODE_FRAMERATE,
    ENCODE_NUM_PFRAMES,
    ENCODE_RATECONTROLTYPE,
    ENCODE_PQUANT,
    ENCODE_SEARCHRANGE,         //sXSearchRange
    ENCODE_INTRAREFRESHTYPE,    //eRefreshMode
    ENCODE_NUMINTRAMB,          //nCirMBs
    ENCODE_PROFILE,
    ENCODE_LEVEL,
    ENCODE_LOOP_FILTER_MODE,    //eLoopFilterMode
    ENCODE_CONST_IPRED,         //bconstIpred flag
    ENCODE_ACCURACY             //eAccuracy
};


enum AAC_SETTING_TYPE
{
    INPUT_FILE_NAME_AAC,
    OUTPUT_FILE_NAME_AAC,
    IP_NUMBER_OF_CHANNELS,
    OP_NUMBER_OF_CHANNELS,
    SAMPLING_FREQ,
    BIT_RATE,
    AUDIO_BANDWIDTH,
    CHANNEL_MODE,
    AAC_PROFILE,
    AAC_STREAM_FORMAT,
    AAC_TOOL
};


//Main AO class for the TestApplication
class OmxComponentEncTest : public OmxEncTestBase
{
    public:
        OmxComponentEncTest() : OmxEncTestBase("OMX_EncTestApp")
        {
            iRole = (OMX_STRING) oscl_malloc(ROLE_SIZE);
            ipConfigFile = NULL;
            ipInputFile = NULL;
            ipOutputFile = NULL;
            ConfigBuffer = (char *)oscl_malloc(MAX_SIZE);
            iInputFileSize = 0;
            iInputBitsPerSample = 16;
            iInputNumberOfChannels = 1;
            iOutputNumberOfChannels = 1;
            iInputSamplingRate = 8000;
            iOutputBandMode = OMX_AUDIO_AMRBandModeNB0;
            iOutputFormat = OMX_AUDIO_AMRFrameFormatIF2;
            iTestStatus = OMX_TRUE;

            iChannelMode = OMX_AUDIO_ChannelModeStereo;
            iAacBandWidth = 0;
            iAacTools = OMX_AUDIO_AACToolAll;
            iAacProfile = OMX_AUDIO_AACObjectLC;
            iAacStreamFormat = OMX_AUDIO_AACStreamFormatMP2ADTS;


        }
        OMX_BOOL Parse(char aConfigFileName[], OMX_STRING role, char Component[]);

        ~OmxComponentEncTest()
        {
            if (iRole)
            {
                oscl_free(iRole);
                iRole = NULL;
            }
            if (ConfigBuffer)
            {
                oscl_free(ConfigBuffer);
                ConfigBuffer = NULL;
            }

        }

    protected:

        bool WriteOutput(OMX_U8* aOutBuff, OMX_U32 aSize);
        void StopOnError();
        void VerifyOutput(OMX_U8 aTestName[]);
        bool GetNextLine(const char *& line_start, const char *& line_end);
        OMX_BOOL ParseAmrLine(const char* line_start,
                              const char* line_end, AMR_SETTING_TYPE line_num);

        OMX_BOOL ParseM4vLine(const char * line_start,
                              const char * line_end, M4V_SETTING_TYPE line_num);

        OMX_BOOL ParseAvcLine(const char * line_start,
                              const char * line_end, AVC_SETTING_TYPE line_num);

        OMX_BOOL ParseAacLine(const char * line_start,
                              const char * line_end, AAC_SETTING_TYPE line_num);

        void Extract(const char * line_start,
                     const char * line_end, char * first, int32& len);

        OMX_ERRORTYPE GetInput();
        OMX_ERRORTYPE GetInputFrame();
        OMX_ERRORTYPE GetInputFrameAMR();

        OMX_STRING  iRole;
        char        iInputFileName[200];
        char        iOutputFileName[200];
        char        iFormat[10];

        OMX_S32     iInputPortIndex;
        OMX_S32     iOutputPortIndex;
        OMX_BOOL    iTestStatus;


        OMX_U32     iInputSamplingRate;
        OMX_U32     iInputBitsPerSample;
        OMX_U32     iInputNumberOfChannels;

        OMX_U32     iOutputNumberOfChannels;

        OMX_AUDIO_AMRFRAMEFORMATTYPE iOutputFormat;
        OMX_AUDIO_AMRBANDMODETYPE iOutputBandMode;

        OMX_AUDIO_PARAM_AACPROFILETYPE iAacParam;
        OMX_U32     iAacBandWidth;
        OMX_U32     iAacTools;
        OMX_AUDIO_AACPROFILETYPE iAacProfile;
        OMX_AUDIO_AACSTREAMFORMATTYPE iAacStreamFormat;
        OMX_AUDIO_CHANNELMODETYPE iChannelMode;

        OMX_S32     iFrameTimeStamp;

        FILE*       ipConfigFile;
        FILE*       ipInputFile;
        FILE*       ipOutputFile;
        char*       ConfigBuffer;
        OMX_U32     iInputFileSize;

        OMX_U32 iFrameWidth; /*mp4*/
        OMX_U32 iFrameHeight;
        OMX_U32 iFrameRate;


        OMX_COLOR_FORMATTYPE iColorFormat;
        OMX_U32 iTgtFrameWidth;
        OMX_U32 iTgtFrameHeight;
        OMX_U32 iTgtBitRate;
        OMX_U32 iTgtFrameRate;
        OMX_U32 iNumPFrames;
        OMX_U32 iCodecMode;
        OMX_VIDEO_CONTROLRATETYPE iRateControlType;
        OMX_U32  iIQuant;
        OMX_U32  iPQuant;
        OMX_S32  iSearchRange;
        OMX_BOOL iFourMV;
        OMX_VIDEO_INTRAREFRESHTYPE iRefreshMode;
        OMX_U32 iNumIntraMB;
        OMX_U32 iMaxPacketSize;
        OMX_VIDEO_MPEG4PROFILETYPE iMpeg4Profile;
        OMX_VIDEO_MPEG4LEVELTYPE iMpeg4Level;
        OMX_BOOL    iResyncFlag;
        OMX_BOOL    iDataPartitioningFlag;
        OMX_BOOL    iShortHeaderFlag;
        OMX_U32     iResynchMarkerSpacing;
        OMX_BOOL    iReversibleVLCFlag;
        OMX_U32     iTimeIncRes;
        OMX_U32     iGobHeaderInterval;

        OMX_VIDEO_PARAM_MPEG4TYPE iMpeg4Type;
        OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE iErrCorrType;
        OMX_VIDEO_PARAM_BITRATETYPE iBitRateType;
        OMX_VIDEO_PARAM_QUANTIZATIONTYPE iQuantParam;
        OMX_VIDEO_PARAM_MOTIONVECTORTYPE iMotionVector;
        OMX_VIDEO_PARAM_INTRAREFRESHTYPE iRefreshParam;
        OMX_VIDEO_PARAM_H263TYPE iH263Type;

        OMX_VIDEO_AVCPROFILETYPE iAvcProfile;
        OMX_VIDEO_AVCLEVELTYPE iAvcLevel;
        OMX_VIDEO_AVCLOOPFILTERTYPE iLoopFilterType;
        OMX_BOOL iIPredictionFlag;
        OMX_VIDEO_MOTIONVECTORTYPE iAccuracy;
        OMX_VIDEO_PARAM_AVCTYPE iAvcType;



    private:
        void Run();


};

class OmxEncTestUseBuffer : public OmxComponentEncTest
{
    public:
        OmxEncTestUseBuffer():
                OmxComponentEncTest() { };
    private:
        void Run();
};

class OmxEncTestEosMissing : public OmxComponentEncTest
{
    public:
        OmxEncTestEosMissing():
                OmxComponentEncTest() { };
    private:
        void Run();

};

class OmxEncTestWithoutMarker : public OmxComponentEncTest
{
    public:
        OmxEncTestWithoutMarker():
                OmxComponentEncTest() { };
    private:
        void Run();
};

class OmxEncTestPartialFrames : public OmxComponentEncTest
{
    public:
        OmxEncTestPartialFrames():
                OmxComponentEncTest() { };
    private:
        void Run();
};

class OmxEncTestExtraPartialFrames : public OmxComponentEncTest
{
    public:
        OmxEncTestExtraPartialFrames():
                OmxComponentEncTest() { };
    private:
        void Run();
};

//TestApplication class to pause the buffer processing inbetween and then resume
class OmxEncTestPauseResume : public OmxComponentEncTest
{
    public:
        OmxEncTestPauseResume():
                OmxComponentEncTest() { };
    private:
        void Run();
        OMX_U32 iFrameCount;
        OMX_BOOL iPauseCommandSent;
};

//TestApplication class for verifying the roles of component
class OmxEncTestCompRole : public OmxComponentEncTest
{
    public:
        OmxEncTestCompRole():
                OmxComponentEncTest() { };
    private:
        void Run();
};

//TestApplication class for verifying the roles of component
class OmxEncTestBufferBusy : public OmxComponentEncTest
{
    public:
        OmxEncTestBufferBusy():
                OmxComponentEncTest() { };
    private:
        void Run();
};

//TestApplication class for buffers negotiation via Get/Set Parameter calls
class OmxEncTestBufferNegotiation : public OmxComponentEncTest
{
    public:
        OmxEncTestBufferNegotiation():
                OmxComponentEncTest()
        {
            iNumInputBuffers = 0;
            iNumOutputBuffers = 0;
        };

    private:
        void Run();
        OMX_BOOL NegotiateParameters();
        OMX_BOOL ParamTestVideoPort();
        OMX_BOOL ParamTestAudioPort();
        OMX_S32 iNumOutputBuffers;
        OMX_S32 iNumInputBuffers;
        OMX_BOOL iIsAudioFormat;
};

#endif  // OMXENCTEST_H_INCLUDED
