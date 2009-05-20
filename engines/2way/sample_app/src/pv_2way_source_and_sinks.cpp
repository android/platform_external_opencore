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
#include "pv_2way_source_and_sinks.h"
#include "pv_2way_video_sink.h"


PV2WaySourceAndSinks::PV2WaySourceAndSinks(PV2WaySourceAndSinksObserver* aObserver,
        PV2Way324InitInfo& aSdkInitInfo)
        : PV2WaySourceAndSinksBase(aObserver, aSdkInitInfo),
        iVideoRender(NULL),
        iVideoRenderCtrl(0),
        iFileParser(NULL),
        bVideoDisplay(false)

{
    iLogger = NULL;
    OSCL_DYNAMIC_CAST(PV2WayVideoSink*, iVideoSink)->SetPlayOnMediaObserver(this);
}

PV2WaySourceAndSinks::~PV2WaySourceAndSinks()
{
    ResetVideoRender();
}


void PV2WaySourceAndSinks::ResetSourceSinkNodes()
{
    iAudioSource->Delete();

    iVideoSource->Delete();

    iVideoSink->Delete();

    iAudioSink->Delete();
}

// this is used to help the GUI.
int PV2WaySourceAndSinks::GetEngineCmdState(PVCommandId aCmdId)
{
    if (iAudioSource->IsAddId(aCmdId))
        return EPVTestCmdAddAudioDataSource;
    else if (iAudioSource->IsRemoveId(aCmdId))
        return EPVTestCmdRemoveAudioDataSource;
    else if (iVideoSource->IsAddId(aCmdId))
        return EPVTestCmdAddVideoDataSource;
    else if (iVideoSource->IsRemoveId(aCmdId))
        return EPVTestCmdRemoveVideoDataSource;
    else if (iAudioSink->IsAddId(aCmdId))
        return EPVTestCmdAddAudioDataSink;
    else if (iAudioSink->IsRemoveId(aCmdId))
        return EPVTestCmdRemoveAudioDataSink;
    else if (iVideoSink->IsAddId(aCmdId))
        return EPVTestCmdAddVideoDataSink;
    else if (iVideoSink->IsRemoveId(aCmdId))
        return EPVTestCmdRemoveVideoDataSink;
    else
        return EPVTestCmdIdle;
}

int32 PV2WaySourceAndSinks::PlayOnMedia(unsigned char* aFrame, uint32 size)
{
    iObserver->StartTimer();
    if (iVideoRender)
    {
        PVMFStatus retVal = iVideoRender->VideoOutputDevicePlay(LPBYTE(aFrame),
                            size);
        bVideoDisplay = true;
        return retVal;
    }
    return 0;
}

void PV2WaySourceAndSinks::MIOParametersValid(bool bValid,
        int32 aWidth,
        int32 aHeight)
{
    if (bValid && iVideoRender)
    {
        iVideoRender->VideoOutputDeviceInit(aWidth, aHeight);
    }
}

void PV2WaySourceAndSinks::CreateVideoRender(HWND aHandle)
{
    iVideoRender = OSCL_NEW(PVVideoRender, (aHandle));
}

bool PV2WaySourceAndSinks::StopVideoRender()
{
    bool retVal = false;
    if (iVideoRender && bVideoDisplay)
    {
        iVideoRender->VideoOutputDeviceStop();
        bVideoDisplay = false;
    }
    return retVal;
}

void PV2WaySourceAndSinks::ResetVideoRender()
{
    if (iVideoRender)
    {
        iVideoRender->VideoOutputDeviceReset();
        OSCL_DELETE(iVideoRender);
        iVideoRender = NULL;
    }
}

