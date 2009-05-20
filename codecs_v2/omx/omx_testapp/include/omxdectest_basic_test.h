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
    @file OMX_testapp\include\omxdectest_basic.h

*/

#ifndef OMXDECTEST_BASIC_H_INCLUDED
#define OMXDECTEST_BASIC_H_INCLUDED

#ifndef OMXDECTEST_H_INCLUDED
#include "omxdectest.h"
#endif


//TestApplication class for verifying the roles of component
class OmxDecTestCompRole : public OmxComponentDecTest
{
    public:

        OmxDecTestCompRole(FILE* aConsOutFile, FILE* aInputFile, FILE* aOutputFile, char aOutFileName[],
                           char aRefFileName[], OMX_STRING aName, OMX_STRING aRole,
                           char aFormat[], OMX_U32 aChannels) :

                OmxComponentDecTest(aConsOutFile, aInputFile, aOutputFile, aOutFileName,
                                    aRefFileName, aName, aRole, aFormat,
                                    aChannels)
        { };

    private:

        void Run();
};

//TestApplication class for buffers negotiation via Get/Set Parameter calls
class OmxDecTestBufferNegotiation : public OmxComponentDecTest
{
    public:

        OmxDecTestBufferNegotiation(FILE* aConsOutFile, FILE* aInputFile, FILE* aOutputFile, char aOutFileName[],
                                    char aRefFileName[], OMX_STRING aName, OMX_STRING aRole,
                                    char aFormat[], OMX_U32 aChannels) :

                OmxComponentDecTest(aConsOutFile, aInputFile, aOutputFile,
                                    aOutFileName, aRefFileName, aName,
                                    aRole, aFormat, aChannels)
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

//TestApplication class for dynamic port reconfiguration
class OmxDecTestPortReconfig : public OmxComponentDecTest
{
    public:

        OmxDecTestPortReconfig(FILE* aConsOutFile, FILE* aInputFile, FILE* aOutputFile, char aOutFileName[],
                               char aRefFileName[], OMX_STRING aName, OMX_STRING aRole,
                               char aFormat[], OMX_U32 aChannels) :

                OmxComponentDecTest(aConsOutFile, aInputFile, aOutputFile, aOutFileName,
                                    aRefFileName, aName, aRole, aFormat,
                                    aChannels)
        {
            iTestCompleteFlag = OMX_FALSE;
            iIsVideoFormat = OMX_FALSE;

            iFlagDecodeHeader = OMX_FALSE;
            iFlagDisablePort = OMX_FALSE;
            iEosFlagExecuting = OMX_FALSE;
            iStatusExecuting = OMX_ErrorNone;
            iFlagStopping = OMX_FALSE;
        };

    private:

        bool WriteOutput(OMX_U8* aOutBuff, OMX_U32 aSize);
        void Run();

        OMX_BOOL iTestCompleteFlag;
        OMX_S32  iDefaultWidth;
        OMX_S32  iDefaultHeight;
        OMX_BOOL iIsVideoFormat;

        //Temporary flags, can't be kept static now in the switch case, as it has to be executed twice
        OMX_BOOL        iFlagDecodeHeader;
        OMX_BOOL        iFlagDisablePort;
        OMX_BOOL        iEosFlagExecuting;
        OMX_ERRORTYPE   iStatusExecuting;
        OMX_BOOL        iFlagStopping;
};

//TestApplication class for testing simultaneous port reconfiguraion and state transitions
class OmxDecTestPortReconfigTransitionTest : public OmxComponentDecTest
{
    public:

        OmxDecTestPortReconfigTransitionTest(FILE* aConsOutFile, FILE* aInputFile, FILE* aOutputFile, char aOutFileName[],
                                             char aRefFileName[], OMX_STRING aName, OMX_STRING aRole,
                                             char aFormat[], OMX_U32 aChannels) :

                OmxComponentDecTest(aConsOutFile, aInputFile, aOutputFile, aOutFileName,
                                    aRefFileName, aName, aRole, aFormat,
                                    aChannels)
        { };

    private:

        OMX_ERRORTYPE EventHandler(OMX_OUT OMX_HANDLETYPE aComponent,
                                   OMX_OUT OMX_PTR aAppData,
                                   OMX_OUT OMX_EVENTTYPE aEvent,
                                   OMX_OUT OMX_U32 aData1,
                                   OMX_OUT OMX_U32 aData2,
                                   OMX_OUT OMX_PTR aEventData);


        void Run();
};

//TestApplication class for testing simultaneous port reconfiguraion and state transitions
class OmxDecTestPortReconfigTransitionTest_2 : public OmxComponentDecTest
{
    public:

        OmxDecTestPortReconfigTransitionTest_2(FILE* aConsOutFile, FILE* aInputFile, FILE* aOutputFile, char aOutFileName[],
                                               char aRefFileName[], OMX_STRING aName, OMX_STRING aRole,
                                               char aFormat[], OMX_U32 aChannels) :

                OmxComponentDecTest(aConsOutFile, aInputFile, aOutputFile, aOutFileName,
                                    aRefFileName, aName, aRole, aFormat,
                                    aChannels)
        { };

    private:

        OMX_ERRORTYPE EventHandler(OMX_OUT OMX_HANDLETYPE aComponent,
                                   OMX_OUT OMX_PTR aAppData,
                                   OMX_OUT OMX_EVENTTYPE aEvent,
                                   OMX_OUT OMX_U32 aData1,
                                   OMX_OUT OMX_U32 aData2,
                                   OMX_OUT OMX_PTR aEventData);


        void Run();
};

//TestApplication class for testing simultaneous port reconfiguraion and state transitions
class OmxDecTestPortReconfigTransitionTest_3 : public OmxComponentDecTest
{
    public:

        OmxDecTestPortReconfigTransitionTest_3(FILE* aConsOutFile, FILE* aInputFile, FILE* aOutputFile, char aOutFileName[],
                                               char aRefFileName[], OMX_STRING aName, OMX_STRING aRole,
                                               char aFormat[], OMX_U32 aChannels) :

                OmxComponentDecTest(aConsOutFile, aInputFile, aOutputFile, aOutFileName,
                                    aRefFileName, aName, aRole, aFormat,
                                    aChannels)
        { };

    private:

        OMX_ERRORTYPE EventHandler(OMX_OUT OMX_HANDLETYPE aComponent,
                                   OMX_OUT OMX_PTR aAppData,
                                   OMX_OUT OMX_EVENTTYPE aEvent,
                                   OMX_OUT OMX_U32 aData1,
                                   OMX_OUT OMX_U32 aData2,
                                   OMX_OUT OMX_PTR aEventData);


        void Run();
};

//TestApplication class to flush the ports inbetween buffer processing and then resume it
class OmxDecTestFlushPort : public OmxComponentDecTest
{
    public:

        OmxDecTestFlushPort(FILE* aConsOutFile, FILE* aInputFile, FILE* aOutputFile, char aOutFileName[],
                            char aRefFileName[], OMX_STRING aName, OMX_STRING aRole,
                            char aFormat[], OMX_U32 aChannels) :

                OmxComponentDecTest(aConsOutFile, aInputFile, aOutputFile, aOutFileName,
                                    aRefFileName, aName, aRole, aFormat,
                                    aChannels)
        {
            iFrameCount = 0;
            iFlushCommandSent = OMX_FALSE;
            iFlushCommandCompleted = OMX_FALSE;
        };

    private:

        void Run();

        OMX_U32 iFrameCount;
        OMX_BOOL iFlushCommandSent;
        OMX_BOOL iFlushCommandCompleted;
};


//TestApplication class to flush the ports inbetween buffer processing and then resume it
class OmxDecTestEosAfterFlushPort : public OmxComponentDecTest
{
    public:

        OmxDecTestEosAfterFlushPort(FILE* aConsOutFile, FILE* aInputFile, FILE* aOutputFile, char aOutFileName[],
                                    char aRefFileName[], OMX_STRING aName, OMX_STRING aRole,
                                    char aFormat[], OMX_U32 aChannels) :

                OmxComponentDecTest(aConsOutFile, aInputFile, aOutputFile, aOutFileName,
                                    aRefFileName, aName, aRole, aFormat,
                                    aChannels)
        {
            iFrameCount = 0;
            iFlushCommandSent = OMX_FALSE;
            iFlushCommandCompleted = OMX_FALSE;
        };

    private:

        void Run();

        OMX_U32 iFrameCount;
        OMX_BOOL iFlushCommandSent;
        OMX_BOOL iFlushCommandCompleted;
};


#endif  // OMXDECTEST_BASIC_H_INCLUDED
