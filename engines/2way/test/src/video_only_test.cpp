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
#include "video_only_test.h"

#ifndef PV_MP4_H263_ENC_EXTENSION_H_INCLUDED
#include "pvmp4h263encextension.h"
#endif

#define TRADEOFF_VALUE 5
#define TRADEOFF_VALUE_2 10


void video_only_test::test()
{
    fprintf(fileoutput, "\n-------- Start video only test -------- ");
    fprintf(fileoutput, "\n** Test Number: %d. ** \n", iTestNum);
    fprintf(fileoutput, "\nSETTINGS:\nProxy %d", iUseProxy);
    iSourceAndSinks->PrintFormatTypes();
    fprintf(fileoutput, "\n----------------------------------\n");

    int error = 0;

    scheduler = OsclExecScheduler::Current();

    this->AddToScheduler();

    if (start_async_test())
    {
        OSCL_TRY(error, scheduler->StartScheduler());
        if (error != 0)
        {
            OSCL_LEAVE(error);
        }
    }

    this->RemoveFromScheduler();
}


void video_only_test::Run()
{
    if (terminal)
    {
        if (iUseProxy)
        {
            CPV2WayProxyFactory::DeleteTerminal(terminal);
        }
        else
        {
            CPV2WayEngineFactory::DeleteTerminal(terminal);
        }
        terminal = NULL;
    }

    if (timer)
    {
        OSCL_DELETE(timer);
        timer = NULL;
    }

    scheduler->StopScheduler();
}

void video_only_test::DoCancel()
{
}


void video_only_test::H324MConfigCommandCompletedL(PVMFCmdResp& aResponse)
{
    OSCL_UNUSED_ARG(aResponse);
}

void video_only_test::H324MConfigHandleInformationalEventL(PVMFAsyncEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
}

void video_only_test::DoStuffWithH324MConfig()
{
    H324MConfigInterface * t324Interface = (H324MConfigInterface *)iH324MConfig;
    t324Interface->SetObserver(this);
    iEncIFCommandId = terminal->QueryInterface(PVMp4H263EncExtensionUUID, iVidEncIFace);
}

void video_only_test::VideoAddSinkSucceeded()
{
    iVideoSinkAdded = true;
    if (iVideoSourceAdded)
    {
        VideoNodesAdded();
    }
}

void video_only_test::VideoNodesAdded()
{
    if (iH324MConfig == NULL)
    {
        printf("\n*************** Test FAILED: could not set stack config (after video add sink) *************** \n");
        test_is_true(false);
        disconnect();
    }
    DoStuffWithH324MConfig();
    timer->RunIfNotReady(TEST_DURATION);
    test_is_true(true);
}

void video_only_test::VideoAddSourceSucceeded()
{
    iVideoSourceAdded = true;
    if (iVideoSinkAdded)
    {
        VideoNodesAdded();
    }
}

void video_only_test::VideoAddSourceFailed()
{
    if (iVidSrcFormatType == PVMF_MIME_M4V)
    {
        test_is_true(true);
    }
    else
    {
        printf("\n*************** Test FAILED: error adding video source *************** \n");
        test_is_true(false);
    }
    disconnect();
}

void video_only_test::EncoderIFSucceeded()
{
    PVMp4H263EncExtensionInterface *iface = (PVMp4H263EncExtensionInterface *)iVidEncIFace;
    iface->RequestIFrame();
}

void video_only_test::EncoderIFFailed()
{
    EncoderIFSucceeded();
}

void video_only_test::DisCmdFailed()
{
    DisCmdSucceeded();
}

void video_only_test::DisCmdSucceeded()
{
    printf("Finished disconnecting \n");
    if (iH324MConfig)
        iH324MConfig->removeRef();
    iH324MConfig = NULL;
    reset();
}

void video_only_test::InitFailed()
{
    printf("\n*************** Test FAILED: Init Failed *************** \n");
    test_is_true(false);
    test_base::InitFailed();
}

void video_only_test::ConnectSucceeded()
{
}

void video_only_test::ConnectFailed()
{
}

void video_only_test::TimerCallback()
{
    int error = 1;

    if (!iVideoSourceAdded || !iVideoSinkAdded)
    {
        // not finished yet
        // wait longer
        timer->RunIfNotReady(TEST_DURATION);
        return;
    }

    OSCL_TRY(error, iVideoRemoveSourceId = iSourceAndSinks->RemoveVideoSource());
    if (error)
    {
        printf("\n*************** Test FAILED: error removing video source *************** \n");
        test_is_true(false);
        disconnect();
    }

}

bool video_only_test::start_async_test()
{
    timer = OSCL_NEW(engine_timer, (this));
    if (timer == NULL)
    {
        printf("\n*************** Test FAILED: timer could not be created *************** \n");
        test_is_true(false);
        return false;
    }
    timer->AddToScheduler();


    return test_base::start_async_test();
}





