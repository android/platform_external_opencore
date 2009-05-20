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
#include "av_test.h"
#include "pv_2way_source_and_sinks_base.h"

void av_test::test()
{
    fprintf(fileoutput, "\n-------- Start avtest --------\n");
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


void av_test::Run()
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

void av_test::DoCancel()
{
}

void av_test::ConnectSucceeded()
{
}


void av_test::InitFailed()
{
    fprintf(fileoutput, "\n*************** Test FAILED: Init Failed *************** \n");
    test_is_true(false);
    test_base::InitFailed();
}

void av_test::ConnectFailed()
{
}

void av_test::AudioAddSinkCompleted()
{
    iAudioSinkAdded = true;
    if (iAudioSourceAdded && iVideoSourceAdded && iVideoSinkAdded)
        timer->RunIfNotReady(TEST_DURATION);
}

void av_test::AudioAddSourceCompleted()
{
    iAudioSourceAdded = true;
    if (iAudioSinkAdded && iVideoSourceAdded && iVideoSinkAdded)
        timer->RunIfNotReady(TEST_DURATION);
}

void av_test::VideoAddSinkFailed()
{
    VideoAddSinkSucceeded();
}

void av_test::VideoAddSinkSucceeded()
{
    iVideoSinkAdded = true;
    if (iVideoSourceAdded && iAudioSourceAdded && iAudioSinkAdded)
        timer->RunIfNotReady(TEST_DURATION);
}

void av_test::VideoAddSourceSucceeded()
{
    iVideoSourceAdded = true;
    if (iVideoSinkAdded && iAudioSourceAdded && iAudioSinkAdded)
        timer->RunIfNotReady(TEST_DURATION);
}

void av_test::VideoAddSourceFailed()
{
    VideoAddSourceSucceeded();
}

void av_test::RstCmdCompleted()
{
    test_is_true(true);
    test_base::RstCmdCompleted();
}

void av_test::CheckForTimeToDisconnect()
{
    if (!iAudioSourceAdded &&
            !iAudioSinkAdded &&
            !iVideoSourceAdded &&
            !iVideoSinkAdded)
    {
        fprintf(fileoutput, "\nTime to disconnect \n");
        disconnect();
    }
}

void av_test::AudioRemoveSourceCompleted()
{
    fprintf(fileoutput, "\nAudio source removed \n");
    iAudioSourceAdded = false;
    CheckForTimeToDisconnect();
}

void av_test::AudioRemoveSinkCompleted()
{
    fprintf(fileoutput, "\nAudio sink removed \n");
    iAudioSinkAdded = false;
    CheckForTimeToDisconnect();
}

void av_test::VideoRemoveSourceCompleted()
{
    fprintf(fileoutput, "\nVideo source removed \n");
    iVideoSourceAdded = false;
    CheckForTimeToDisconnect();
}

void av_test::VideoRemoveSinkCompleted()
{
    fprintf(fileoutput, "\nVideo sink removed \n");
    iVideoSinkAdded = false;
    CheckForTimeToDisconnect();
}

void av_test::TimerCallback()
{
    int error = 0;
    if (!iAudioSourceAdded ||
            !iAudioSinkAdded ||
            !iVideoSourceAdded ||
            !iVideoSinkAdded)
    {
        // wait longer
        timer->RunIfNotReady(TEST_DURATION);
        return;
    }

    fprintf(fileoutput, "\nRemoving source and sinks \n");
    OSCL_TRY(error, iVideoRemoveSourceId = iSourceAndSinks->RemoveVideoSource());
    if (error)
    {
        iTestStatus &= false;
        disconnect();
    }
    else
    {
        error = 1;
        OSCL_TRY(error, iVideoRemoveSinkId = iSourceAndSinks->RemoveVideoSink());
        if (error)
        {
            iTestStatus &= false;
            disconnect();
        }
    }

    OSCL_TRY(error, iAudioRemoveSourceId = iSourceAndSinks->RemoveAudioSource());
    if (error)
    {
        iTestStatus &= false;
        disconnect();
    }
    else
    {
        error = 1;
        OSCL_TRY(error, iAudioRemoveSinkId = iSourceAndSinks->RemoveAudioSink());
        if (error)
        {
            iTestStatus &= false;
            disconnect();
        }
    }
}

bool av_test::start_async_test()
{
    timer = OSCL_NEW(engine_timer, (this));
    if (timer == NULL)
    {
        fprintf(fileoutput, "\n*************** Test FAILED: no timer *************** \n");
        test_is_true(false);
        return false;
    }
    iAudioSourceAdded = false;
    iAudioSinkAdded = false;

    timer->AddToScheduler();


    return test_base::start_async_test();
}





