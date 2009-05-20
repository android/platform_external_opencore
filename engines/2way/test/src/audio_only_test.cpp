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
#include "audio_only_test.h"


void audio_only_test::test()
{
    fprintf(fileoutput, "\n-------- Start audio only test -------- ");
    fprintf(fileoutput, "\n** Test Number: %d. ** \n", iTestNum);
    fprintf(fileoutput, "\nSETTINGS:\nProxy %d", iUseProxy);
    iSourceAndSinks->PrintFormatTypes();
    fprintf(fileoutput, "\n-------------------------------------\n");

    int error = 0;

    scheduler = OsclExecScheduler::Current();

    this->AddToScheduler();

    if (start_async_test())
    {
        OSCL_TRY(error, scheduler->StartScheduler());
        if (error != 0)
        {
            printf("\n*************** Test FAILED: error starting scheduler *************** \n");
            test_is_true(false);
            OSCL_LEAVE(error);
        }
    }

    this->RemoveFromScheduler();
    test_is_true(iTestStatus);
}


void audio_only_test::Run()
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

void audio_only_test::DoCancel()
{
}

void audio_only_test::ConnectSucceeded()
{
}

void audio_only_test::ConnectFailed()
{
    reset();
}


void audio_only_test::TimerCallback()
{
    int error = 1;
    if (!iAudioSourceAdded || !iAudioSinkAdded)
    {
        // not finished yet
        // wait longer
        timer->RunIfNotReady(TEST_DURATION);
        return;
    }
    timer_elapsed = true;
    OSCL_TRY(error, iAudioRemoveSourceId = iSourceAndSinks->RemoveAudioSource());
    if (error)
    {
        printf("\n*************** Test FAILED: error removing audio source *************** \n");
        iTestStatus &= false;
        disconnect();
    }
    else
    {
        error = 1;
        OSCL_TRY(error, iAudioRemoveSinkId = iSourceAndSinks->RemoveAudioSink());
        if (error)
        {
            printf("\n*************** Test FAILED: error removing audio sink *************** \n");
            iTestStatus &= false;
            disconnect();
        }
    }

}


bool audio_only_test::start_async_test()
{
    timer = OSCL_NEW(engine_timer, (this));
    if (timer == NULL)
    {
        printf("\n*************** Test FAILED: could not create timer *************** \n");
        iTestStatus &= false;
        return false;
    }

    iAudioSourceAdded = false;
    iAudioSinkAdded = false;
    isFirstSink = true;
    isFirstSrc = true;

    timer->AddToScheduler();


    return test_base::start_async_test();
}


