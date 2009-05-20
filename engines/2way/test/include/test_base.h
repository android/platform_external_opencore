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
#ifndef TEST_BASE_H_INCLUDED
#define TEST_BASE_H_INCLUDED

#ifndef TEST_ENGINE_H_INCLUDED
#include "test_engine.h"
#endif

#include "pvlogger_stderr_appender.h"
#include "pvlogger_file_appender.h"

#ifndef PV_2WAY_PROXY_FACTORY_H_INCLUDED
#include "pv_2way_proxy_factory.h"
#endif

#define TEST_DURATION 800

class test_base : public engine_test,
        public H324MConfigObserver
{
    public:

        test_base(bool aUseProxy = false,
                  int aMaxRuns = 1,
                  bool isSIP = false) :
                engine_test(aUseProxy, aMaxRuns),
                iH324MConfig(NULL),
                iComponentInterface(NULL),
                iEncoderIFCommandId(-1),
                i324mIFCommandId(-1),
                iCancelCmdId(-1),
                iQueryInterfaceCmdId(-1),
                iStackIFSet(false),
                iEncoderIFSet(false),
                iSIP(isSIP),
                iTempH324MConfigIterface(NULL),
                iSourceAndSinks(NULL),
                iUsingAudio(false),
                iUsingVideo(false)
        {
            iTestNum = iTestCounter;
            test_base::iTestCounter++;
        }

        virtual ~test_base()
        {
            cleanup();
            if (iSourceAndSinks)
            {
                OSCL_DELETE(iSourceAndSinks);
            }
        }
        void AddSourceAndSinks(PV2WayUnitTestSourceAndSinks* aSourceAndSinks)
        {
            iSourceAndSinks = aSourceAndSinks;
            if (iSourceAndSinks && terminal)
            {
                iSourceAndSinks->SetTerminal(terminal);
            }
        }
        void StartTimer()
        {
        }

        void cleanup()
        {
            if (iH324MConfig)
            {
                iH324MConfig->removeRef();
                iH324MConfig = NULL;
            }
            if (iComponentInterface)
            {
                iComponentInterface->removeRef();
                iComponentInterface = NULL;
            }
            if (iTempH324MConfigIterface)
            {
                iTempH324MConfigIterface->removeRef();
                iTempH324MConfigIterface = NULL;
            }
            engine_test::cleanup();
        }

    protected:
        template<class DestructClass>
        class AppenderDestructDealloc : public OsclDestructDealloc
        {
            public:
                virtual void destruct_and_dealloc(OsclAny *ptr)
                {
                    OSCL_DELETE((DestructClass*)ptr);
                }
        };
        void InitializeLogs();
        void HandleInformationalEvent(const PVAsyncInformationalEvent& aEvent);

        virtual bool Init();
        virtual void CommandCompleted(const PVCmdResponse& aResponse);

        void CreateH324Component(bool aCreateH324 = true);

        void H324MConfigCommandCompletedL(PVMFCmdResp& aResponse);
        void H324MConfigHandleInformationalEventL(PVMFAsyncEvent& aNotification);

        virtual void QueryInterfaceSucceeded();

        //------------------- Functions overridden in test classes for specific behavior------
        virtual bool start_async_test();

        virtual void InitSucceeded();
        virtual void InitFailed();
        virtual void InitCancelled();

        virtual void ConnectSucceeded();
        virtual void ConnectFailed();
        virtual void ConnectCancelled();

        virtual void CancelCmdCompleted();


        virtual void RstCmdCompleted();
        virtual void DisCmdSucceeded();
        virtual void DisCmdFailed();

        virtual void EncoderIFSucceeded();
        virtual void EncoderIFFailed();


        // audio
        virtual void AudioAddSinkCompleted();
        virtual void AudioAddSourceCompleted();
        virtual void AudioRemoveSourceCompleted();
        virtual void AudioRemoveSinkCompleted();

        // video
        virtual void VideoAddSinkSucceeded();
        virtual void VideoAddSinkFailed();
        virtual void VideoAddSourceSucceeded();
        virtual void VideoAddSourceFailed();
        virtual void VideoRemoveSourceCompleted();
        virtual void VideoRemoveSinkCompleted();

        virtual void CreateParts();

        //------------------- END Functions overridden in test classes for specific behavior------

        H324MConfigInterface* iH324MConfig;
        PVInterface *iComponentInterface;
        PVCommandId iEncoderIFCommandId;
        PVCommandId i324mIFCommandId;
        PVCommandId iCancelCmdId;
        PVCommandId iQueryInterfaceCmdId;

        bool iStackIFSet;
        bool iEncoderIFSet;
        bool iSIP;

        PVMFFormatType iAudSrcFormatType, iAudSinkFormatType;
        PVMFFormatType iVidSrcFormatType, iVidSinkFormatType;
        PVInterface* iTempH324MConfigIterface;

        PV2WayUnitTestSourceAndSinks* iSourceAndSinks;

        bool iUsingAudio;
        bool iUsingVideo;
        static uint32 iTestCounter;
        int iTestNum;

};


#endif


