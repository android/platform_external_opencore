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

#include "pv_2way_console_source_and_sinks.h"

#ifndef PVMF_MEDIA_INPUT_NODE_FACTORY_H_INCLUDED
#include "pvmf_media_input_node_factory.h"
#endif

#include "pvmf_format_type.h"
#include "oscl_utf8conv.h"

#define AUDIO_SAMPLING_FREQUENCY 8000
#define VIDEO_TIMESCALE 1000
#define VIDEO_FRAMEHEIGHT 144
#define VIDEO_FRAMEWIDTH 176
#define VIDEO_FRAMERATE 5

#define TEST_COMPRESSED
#ifdef TEST_COMPRESSED
#define LAYOUT_BUFFER_SIZE 1024
#define AUDIO_SAMPLING_FREQUENCY 8000
#define VIDEO_TIMESCALE 1000
#define VIDEO_FRAMEHEIGHT 144
#define VIDEO_FRAMEWIDTH 176
#define VIDEO_FRAMERATE 5


#define AUDIO_SOURCE_FILENAME _STRLIT("audio_in.if2")
#define AUDIO_SINK_FILENAME _STRLIT("audio_out.if2")
#define VIDEO_SOURCE_FILENAME _STRLIT("holmes.h263")
#define VIDEO_SINK_FILENAME _STRLIT("holmes_out.h263")
#else
#define LAYOUT_BUFFER_SIZE 1024
#define AUDIO_SAMPLING_FREQUENCY 8000
#define VIDEO_TIMESCALE 1000
#define VIDEO_FRAMEHEIGHT 144
#define VIDEO_FRAMEWIDTH 176
#define VIDEO_FRAMERATE 5

#define AUDIO_SOURCE_FILENAME _STRLIT("pcm16testinput.pcm")
#define AUDIO_SINK_FILENAME _STRLIT("pcm16testoutput.pcm")
#define VIDEO_SOURCE_FILENAME _STRLIT("yuv420video.dat")
#define VIDEO_SINK_FILENAME _STRLIT("video_out.yuv420")
#endif

PV2WayConsoleSourceAndSinks::PV2WayConsoleSourceAndSinks(PV2WaySourceAndSinksObserver* aObserver,
        PV2Way324InitInfo& aSdkInitInfo) :
        PV2WaySourceAndSinksBase(aObserver, aSdkInitInfo)
{
    iSdkInitInfo.iMultiplexingDelayMs = 0;
    // incoming, outgoing formats set in engine handler
}

PV2WayConsoleSourceAndSinks::~PV2WayConsoleSourceAndSinks()
{
}

bool FileExists(const oscl_wchar* fileName)
{
    Oscl_FileServer fs;
    fs.Connect();
    Oscl_File temp_file;
    if (!(temp_file.Open(fileName, Oscl_File::MODE_READ, fs)))
    {
        temp_file.Close();
        fs.Close();
        return true;
    }
    fs.Close();
    return false;
}

int PV2WayConsoleSourceAndSinks::CreateCodecs()
{
    ///////////////////////////////////////////////////
    // create the audio source
#ifdef TEST_COMPRESSED
    iAudioSourceFileSettings.iMediaFormat = PVMF_MIME_AMR_IF2;
#else
    iAudioSourceFileSettings.iMediaFormat = PVMF_MIME_PCM16;
#endif
    iAudioSourceFileSettings.iLoopInputFile = true;
    iAudioSourceFileSettings.iFileName = AUDIO_SOURCE_FILENAME;
    iAudioSourceFileSettings.iSamplingFrequency = AUDIO_SAMPLING_FREQUENCY;
    iAudioSourceFileSettings.iNumChannels = 1;
    if (!FileExists(AUDIO_SOURCE_FILENAME))
    {
        char filename[FILENAME_LEN];
        if (0 == oscl_UnicodeToUTF8(AUDIO_SOURCE_FILENAME, oscl_strlen(AUDIO_SOURCE_FILENAME),
                                    filename, FILENAME_LEN))
        {
            OutputInfo("\nError! Cannot find audio source file!\n");
            return PVMFFailure;
        }
        OutputInfo("\nError! Cannot find audio source file: %s!\n", filename);
        return PVMFFailure;
    }

    iAudioSource->AddCodec(iAudioSourceFileSettings);

    ///////////////////////////////////////////////////
    // create the audio sink

    iAudioSinkFileSettings.iFileName = AUDIO_SINK_FILENAME;
#ifdef TEST_COMPRESSED
    iAudioSinkFileSettings.iMediaFormat = PVMF_MIME_AMR_IF2;
#else
    iAudioSinkFileSettings.iMediaFormat = PVMF_MIME_PCM16;
#endif
    iAudioSink->AddCodec(iAudioSinkFileSettings);

    ///////////////////////////////////////////////////
    // create the video source
#ifdef TEST_COMPRESSED
    iVideoSourceFileSettings.iMediaFormat = PVMF_MIME_H2632000;
#else
    iVideoSourceFileSettings.iMediaFormat = PVMF_MIME_YUV420;
#endif
    iVideoSourceFileSettings.iLoopInputFile = true;
    iVideoSourceFileSettings.iFileName = VIDEO_SOURCE_FILENAME;
    iVideoSourceFileSettings.iTimescale = VIDEO_TIMESCALE;
    iVideoSourceFileSettings.iFrameHeight = VIDEO_FRAMEHEIGHT;
    iVideoSourceFileSettings.iFrameWidth = VIDEO_FRAMEWIDTH;
    iVideoSourceFileSettings.iFrameRate = VIDEO_FRAMERATE;
    if (!FileExists(VIDEO_SOURCE_FILENAME))
    {
        char filename[FILENAME_LEN];
        if (!oscl_UnicodeToUTF8(VIDEO_SOURCE_FILENAME, oscl_strlen(VIDEO_SOURCE_FILENAME),
                                filename, FILENAME_LEN))
        {
            OutputInfo("\nError! Cannot find video source file!\n");
            return PVMFFailure;
        }
        OutputInfo("\nError! Cannot find video source file: %s!\n", filename);
        return PVMFFailure;
    }
    iVideoSource->AddCodec(iVideoSourceFileSettings);

    ///////////////////////////////////////////////////
    // create the video sink
    iVideoSinkFileSettings.iFileName = VIDEO_SINK_FILENAME;
#ifdef TEST_COMPRESSED
    iVideoSinkFileSettings.iMediaFormat = PVMF_MIME_H2632000;
#else
    iVideoSinkFileSettings.iMediaFormat = PVMF_MIME_YUV420;
#endif
    iVideoSink->AddCodec(iVideoSinkFileSettings);
    return PVMFSuccess;
}



