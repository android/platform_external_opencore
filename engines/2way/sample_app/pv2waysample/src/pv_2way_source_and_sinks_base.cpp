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
#include "pv_2way_source_and_sinks_base.h"

#include "pv_2way_audio_sink.h"
#include "pv_2way_audio_source.h"
#include "pv_2way_video_sink.h"
#include "pv_2way_video_source.h"


OSCL_EXPORT_REF PV2WaySourceAndSinksBase::PV2WaySourceAndSinksBase(PV2WaySourceAndSinksObserver* aSourceSinksObserver,
        PV2Way324InitInfo& aSdkInitInfo):
        iObserver(aSourceSinksObserver),
        iAudioStartRecId(-1),
        iAudioStopRecId(-1),
        iVideoStartRecId(-1),
        iVideoStopRecId(-1),
        iSdkInitInfo(aSdkInitInfo),
        iAudioSink(NULL),
        iAudioSource(NULL),
        iVideoSink(NULL),
        iVideoSource(NULL),
        iTerminal(NULL)
{
    iAudioSink = OSCL_NEW(PV2WayAudioSink,
                          (&iSdkInitInfo.iIncomingAudioFormats));
    iAudioSource = OSCL_NEW(PV2WayAudioSource,
                            (&iSdkInitInfo.iOutgoingAudioFormats));
    iVideoSink = OSCL_NEW(PV2WayVideoSink,
                          (&iSdkInitInfo.iIncomingVideoFormats));
    iVideoSource = OSCL_NEW(PV2WayVideoSource,
                            (&iSdkInitInfo.iOutgoingVideoFormats));
}


OSCL_EXPORT_REF PV2WaySourceAndSinksBase::~PV2WaySourceAndSinksBase()
{
    Cleanup();
}

OSCL_EXPORT_REF void PV2WaySourceAndSinksBase::Init()
{
    iAudioSink->Init();
    iAudioSource->Init();
    iVideoSink->Init();
    iVideoSource->Init();
}


OSCL_EXPORT_REF void PV2WaySourceAndSinksBase::Cleanup()
{
    if (iAudioSink)
    {
        OSCL_DELETE(iAudioSink);
        iAudioSink = NULL;
    }
    if (iAudioSource)
    {
        OSCL_DELETE(iAudioSource);
        iAudioSource = NULL;
    }
    if (iVideoSink)
    {
        OSCL_DELETE(iVideoSink);
        iVideoSink = NULL;
    }
    if (iVideoSource)
    {
        OSCL_DELETE(iVideoSource);
        iVideoSource = NULL;
    }
}

OSCL_EXPORT_REF void PV2WaySourceAndSinksBase::RemoveSourceAndSinks()
{
    iAudioSink->Remove();
    iAudioSource->Remove();
    iVideoSink->Remove();
    iVideoSource->Remove();
}

OSCL_EXPORT_REF void PV2WaySourceAndSinksBase::ResetCompleted()
{
    iAudioSink->ResetCompleted();
    iAudioSource->ResetCompleted();
    iVideoSink->ResetCompleted();
    iVideoSource->ResetCompleted();
}

OSCL_EXPORT_REF int PV2WaySourceAndSinksBase::ResetPreferredCodec(TPVDirection aDir,
        PV2WayMediaType aMediaType)
{
    PV2WayMIO* mio = 0;
    mio = GetMIO(aDir, aMediaType);
    if (mio)
    {
        mio->ClearCodecs();
        return 0;
    }
    OutputInfo("PV2WaySourceAndSinksBase::ResetPreferredCodec: Error!  No MIO of given dir, type");
    return -1;
}

PV2WayMIO* PV2WaySourceAndSinksBase::GetMIO(TPVDirection aDir,
        PV2WayMediaType aMediaType)
{
    PV2WayMIO *mio = 0;
    switch (aDir)
    {
        case OUTGOING:
            switch (aMediaType)
            {
                case PV_AUDIO:
                    mio = iAudioSource;
                    break;
                case PV_VIDEO:
                    mio = iVideoSource;
                    break;
                case PV_MEDIA_NONE:
                case PV_CONTROL:
                case PV_DATA:
                case PV_USER_INPUT:
                case PV_MULTIPLEXED:
                case PV_MEDIA_ALL:
                default:
                    break;
            }
            break;

        case INCOMING:
            switch (aMediaType)
            {
                case PV_AUDIO:
                    mio = iAudioSink;
                    break;
                case PV_VIDEO:
                    mio = iVideoSink;
                    break;
                case PV_MEDIA_NONE:
                case PV_CONTROL:
                case PV_DATA:
                case PV_USER_INPUT:
                case PV_MULTIPLEXED:
                case PV_MEDIA_ALL:
                default:
                    break;
            }
            break;
        case PV_DIRECTION_BOTH:
        case PV_DIRECTION_NONE:
        default:
            break;
    }
    return mio;

}

OSCL_EXPORT_REF int PV2WaySourceAndSinksBase::AddPreferredCodec(TPVDirection aDir,
        PV2WayMediaType aMediaType,
        PVMFFormatType aFormat)
{
    PV2WayMIO* mio = GetMIO(aDir, aMediaType);
    if (mio)
    {
        mio->AddCodec(aFormat);
        return 0;
    }
    OutputInfo("PV2WaySourceAndSinksBase::AddPreferredCodec: Error!  No MIO of given dir, type");
    return -1;
}

OSCL_EXPORT_REF int PV2WaySourceAndSinksBase::AddPreferredCodec(TPVDirection aDir,
        PV2WayMediaType aMediaType,
        PvmiMIOFileInputSettings& aFileSettings)
{
    PV2WayMIO* mio = GetMIO(aDir, aMediaType);
    if (mio)
    {
        mio->AddCodec(aFileSettings);
        return 0;
    }
    OutputInfo("PV2WaySourceAndSinksBase::AddPreferredCodec: Error!  No MIO of given dir, type");
    return -1;
}


OSCL_EXPORT_REF int PV2WaySourceAndSinksBase::AddPreferredCodec(TPVDirection aDir,
        PV2WayMediaType aMediaType,
        PVMFFileInputSettings& aFileSettings)
{
    PV2WayMIO* mio = GetMIO(aDir, aMediaType);
    if (mio)
    {
        mio->AddCodec(aFileSettings);
        return 0;
    }
    OutputInfo("PV2WaySourceAndSinksBase::AddPreferredCodec: Error!  No MIO of given dir, type");
    return -1;
}

OSCL_EXPORT_REF void PV2WaySourceAndSinksBase::CommandCompleted(const PVCmdResponse& aResponse)
{
    PVCommandId cmdId = aResponse.GetCmdId();

    if (iAudioSource->IsAddId(cmdId))
    {
        iAudioSource->AddCompleted(aResponse);
    }
    else if (iVideoSource->IsAddId(cmdId))
    {
        iVideoSource->AddCompleted(aResponse);
    }
    else if (iAudioSource->IsRemoveId(cmdId))
    {
        iAudioSource->RemoveCompleted(aResponse);
    }
    else if (iVideoSource->IsRemoveId(cmdId))
    {
        iVideoSource->RemoveCompleted(aResponse);
    }
    else if (iAudioSink->IsAddId(cmdId))
    {
        iAudioSink->AddCompleted(aResponse);
    }
    else if (iVideoSink->IsAddId(cmdId))
    {
        iVideoSink->AddCompleted(aResponse);
    }
    else if (iAudioSink->IsRemoveId(cmdId))
    {
        iAudioSink->RemoveCompleted(aResponse);
    }
    else if (iVideoSink->IsRemoveId(cmdId))
    {
        iVideoSink->RemoveCompleted(aResponse);
    }
}

OSCL_EXPORT_REF void PV2WaySourceAndSinksBase::HandleClosingTrack(const PVAsyncInformationalEvent& aEvent)
{
    TPVDirection dir = (TPVDirection)((PVAsyncInformationalEvent&)aEvent).GetLocalBuffer()[0];
    TPVChannelId id = GetIdFromLocalBuffer((PVAsyncInformationalEvent &) aEvent);
    OutputInfo("PVT_INDICATION_CLOSING_TRACK dir=%d, id %u\n", dir, id);
    if (dir == INCOMING)
    {
        if (iAudioSink->IsChannelId(id))
        {
            iAudioSink->HandleClosingTrack();
        }
        else if (iVideoSink->IsChannelId(id))
        {
            iVideoSink->HandleClosingTrack();
        }
    }
    else
    {
        if (iAudioSource->IsChannelId(id))
        {
            iAudioSource->HandleClosingTrack();
        }
        else if (iVideoSource->IsChannelId(id))
        {
            iVideoSource->HandleClosingTrack();
        }
    }
}


OSCL_EXPORT_REF void PV2WaySourceAndSinksBase::HandleCloseTrack(const PVAsyncInformationalEvent& aEvent)
{

    TPVDirection dir = (TPVDirection)((PVAsyncInformationalEvent&)aEvent).GetLocalBuffer()[0];
    TPVChannelId id = GetIdFromLocalBuffer((PVAsyncInformationalEvent &) aEvent);
    OutputInfo("PVT_INDICATION_CLOSE_TRACK dir=%d, id=%u", dir, id);
    if (dir == INCOMING)
    {
        if (iVideoSink->IsChannelId(id))
        {
            OutputInfo("\nIncoming video track closed\n");
            iVideoSink->HandleCloseTrack();
        }
        else if (iAudioSink->IsChannelId(id))
        {
            OutputInfo("\nIncoming audio track closed\n");
            iAudioSink->HandleCloseTrack();
        }
    }
    else if (dir == OUTGOING)
    {
        if (iVideoSource->IsChannelId(id))
        {
            OutputInfo("\nOutgoing video track closed\n");
            iVideoSource->HandleCloseTrack();
        }
        else if (iAudioSource->IsChannelId(id))
        {
            OutputInfo("\nOutgoing audio track closed\n");
            iAudioSource->HandleCloseTrack();
        }
    }
}

OSCL_EXPORT_REF void PV2WaySourceAndSinksBase::PrintFormatTypes()
{
    OutputInfo("\nAudio Sink Format Types: ");
    iAudioSink->PrintFormatTypes();
    OutputInfo("\nAudio Source Format Types: ");
    iAudioSource->PrintFormatTypes();
    OutputInfo("\nVideo Sink Format Types: ");
    iVideoSink->PrintFormatTypes();
    OutputInfo("\nVideo Source Format Types: ");
    iVideoSource->PrintFormatTypes();
}

#ifdef PERF_TEST
OSCL_EXPORT_REF int PV2WaySourceAndSinksBase::SetPerfFileSettings()
{
    PVMFFileInputSettings audioSourceFileSettings;
    audioSourceFileSettings.iMediaFormat = PVMF_MIME_AMR_IF2;
    audioSourceFileSettings.iLoopInputFile = true;
    audioSourceFileSettings.iFileName = AUDIO_SOURCE_FILENAME_FOR_PERF;
    audioSourceFileSettings.iSamplingFrequency = 8000;
    audioSourceFileSettings.iNumChannels = 1;
    audioSourceFileSettings.iFrameRateSimulation = true;
    iAudioSource->AddCodec(audioSourceFileSettings);

    // create audio sink for performance testing
    PVMFFileInputSettings audioSinkFileSettings;
    audioSinkFileSettings.iFileName = AUDIO_SINK_FILENAME;
    audioSinkFileSettings.iMediaFormat = PVMF_MIME_AMR_IF2;
    iAudioSink->AddCodec(audioSinkFileSettings);

    // create the video source for performance testing
    PVMFFileInputSettings videoSourceFileSettings;
    videoSourceFileSettings.iMediaFormat = PVMF_MIME_H2632000; // console was: PVMF_MIME_H263
    videoSourceFileSettings.iLoopInputFile = true;
    videoSourceFileSettings.iFileName = VIDEO_SOURCE_FILENAME_FOR_PERF;
    videoSourceFileSettings.iTimescale = VIDEO_TIMESCALE;
    videoSourceFileSettings.iFrameHeight = VIDEO_FRAMEHEIGHT;
    videoSourceFileSettings.iFrameWidth = VIDEO_FRAMEWIDTH;
    videoSourceFileSettings.iFrameRate = VIDEO_FRAMERATE;
    videoSourceFileSettings.iFrameRateSimulation = true;
    iVideoSource->AddCodec(videoSourceFileSettings);

    // create the video sink for performance testing
    PVMFFileInputSettings videoSinkFileSettings;
    videoSinkFileSettings.iFileName = VIDEO_SINK_FILENAME;
    videoSinkFileSettings.iMediaFormat = PVMF_MIME_H2632000;
    iVideoSink->AddCodec(videoSinkFileSettings);

    return PVMFSuccess;
}

#endif


