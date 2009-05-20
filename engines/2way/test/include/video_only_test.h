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
#ifndef VIDEO_ONLY_TEST_H_INCLUDED
#define VIDEO_ONLY_TEST_H_INCLUDED

#include "test_base.h"
#include "tsc_h324m_config_interface.h"

class video_only_test : public test_base
{
    public:
        video_only_test(bool aUseProxy = false)
                : test_base(aUseProxy),
                iTradeOffCmd(0),
                iIncomingVideo(0)

        {
            iUsingVideo = true;
        }

        ~video_only_test()
        {
        }

        void test();

        void Run();

        void DoCancel();


        void TimerCallback();

        void H324MConfigCommandCompletedL(PVMFCmdResp& aResponse);

        void H324MConfigHandleInformationalEventL(PVMFAsyncEvent& aNotification);

    private:
        void VideoNodesAdded();
        void DoStuffWithH324MConfig();
        virtual void VideoAddSinkSucceeded();
        virtual void VideoAddSourceSucceeded();
        virtual void VideoAddSourceFailed();
        virtual void EncoderIFSucceeded();
        virtual void EncoderIFFailed();
        virtual void DisCmdFailed();
        virtual void DisCmdSucceeded();
        virtual void InitFailed();
        virtual void ConnectSucceeded();
        virtual void ConnectFailed();
        bool start_async_test();
        PVInterface *iVidEncIFace;
        PVCommandId i324mIFCommandId, iTradeOffCmd, iTradeOffInd, iEncIFCommandId;
        PVTrackId iIncomingVideo, iOutgoingVideo;
};


#endif


