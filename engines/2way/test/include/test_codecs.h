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
#ifndef TESTCODECS_H_INCLUDE
#define TESTCODECS_H_INCLUDE

class TestCodecs
{
    public:

        TestCodecs()
        {
        }
        ~TestCodecs() {};
        bool FileExists(const oscl_wchar* afileName)
        {
            Oscl_FileServer fs;
            fs.Connect();
            Oscl_File temp_file;
            if (!(temp_file.Open(afileName, Oscl_File::MODE_READ, fs)))
            {
                temp_file.Close();
                fs.Close();
                return true;
            }
            fs.Close();
            char filename[FILENAME_LEN];
            if (0 == oscl_UnicodeToUTF8(afileName, oscl_strlen(afileName),
                                        filename, FILENAME_LEN))
            {
                fprintf(fileoutput, "ERROR!!!! Could not locate input file!\n");
                return false;
            }
            fprintf(fileoutput, "\nERROR!!!! Could not locate file %s!!!!\n\n", filename);
            return false;
        }
        bool setvalues()
        {

            // create the audio source
            iAudioSourceFileSettings.iMediaFormat = PVMF_MIME_AMR_IF2;
            iAudioSourceFileSettings.iLoopInputFile = true;
            iAudioSourceFileSettings.iFileName = AUDIO_SOURCE_FILENAME;
            iAudioSourceFileSettings.iSamplingFrequency = 8000;
            iAudioSourceFileSettings.iNumChannels = 1;
            if (!FileExists(AUDIO_SOURCE_FILENAME))
            {
                return false;
            }

            iAudioSourceRawFileSettings.iMediaFormat = PVMF_MIME_PCM16;
            iAudioSourceRawFileSettings.iLoopInputFile = true;
            iAudioSourceRawFileSettings.iFileName = AUDIO_SOURCE_RAW_FILENAME;
            iAudioSourceRawFileSettings.iSamplingFrequency = 8000;
            iAudioSourceRawFileSettings.iNumChannels = 1;
            if (!FileExists(AUDIO_SOURCE_RAW_FILENAME))
            {
                return false;
            }


            iAudioSource2FileSettings.iMediaFormat = PVMF_MIME_AMR_IF2;
            iAudioSource2FileSettings.iLoopInputFile = true;
            iAudioSource2FileSettings.iFileName = AUDIO_SOURCE_FILENAME;
            iAudioSource2FileSettings.iSamplingFrequency = 8000;
            iAudioSource2FileSettings.iNumChannels = 1;

            iAudioSource3FileSettings.iMediaFormat = PVMF_MIME_AMR_IETF;
            iAudioSource3FileSettings.iLoopInputFile = true;
            iAudioSource3FileSettings.iFileName = AUDIO_SOURCE3_FILENAME;
            iAudioSource3FileSettings.iSamplingFrequency = 8000;
            iAudioSource3FileSettings.iNum20msFramesPerChunk = 1;
            iAudioSource3FileSettings.iNumChannels = 1;
            if (!FileExists(AUDIO_SOURCE3_FILENAME))
            {
                return false;
            }

            // create the audio sinks

            iAudioSinkFileSettings.iFileName = AUDIO_SINK_FILENAME;
            iAudioSinkFileSettings.iMediaFormat = PVMF_MIME_AMR_IF2;

            iAudioSinkRawFileSettings.iFileName = AUDIO_SINK_RAW_FILENAME;
            iAudioSinkRawFileSettings.iMediaFormat = PVMF_MIME_PCM16;


            iAudioSink2FileSettings.iFileName = AUDIO_SINK2_FILENAME;
            iAudioSink2FileSettings.iMediaFormat = PVMF_MIME_AMR_IETF;

            // create the video sources
            iVideoSourceYUVFileSettings.iMediaFormat = PVMF_MIME_YUV420;
            iVideoSourceYUVFileSettings.iLoopInputFile = true;
            iVideoSourceYUVFileSettings.iFileName = VIDEO_SOURCE_YUV_FILENAME;
            iVideoSourceYUVFileSettings.iTimescale = 1000;
            iVideoSourceYUVFileSettings.iFrameHeight = 144;
            iVideoSourceYUVFileSettings.iFrameWidth = 176;
            iVideoSourceYUVFileSettings.iFrameRate = 5;
            if (!FileExists(VIDEO_SOURCE_YUV_FILENAME))
            {
                return false;
            }



            iVideoSourceH263FileSettings.iMediaFormat = PVMF_MIME_H2632000;
            iVideoSourceH263FileSettings.iLoopInputFile = true;
            iVideoSourceH263FileSettings.iFileName = VIDEO_SOURCE_H263_FILENAME;
            iVideoSourceH263FileSettings.iTimescale = 1000;
            iVideoSourceH263FileSettings.iFrameHeight = 144;
            iVideoSourceH263FileSettings.iFrameWidth = 176;
            iVideoSourceH263FileSettings.iFrameRate = 5;
            if (!FileExists(VIDEO_SOURCE_H263_FILENAME))
            {
                return false;
            }


            // create another video source
            iVideoSourceM4VFileSettings.iMediaFormat = PVMF_MIME_M4V;
            iVideoSourceM4VFileSettings.iLoopInputFile = true;
            iVideoSourceM4VFileSettings.iFileName = VIDEO_SOURCE_M4V_FILENAME;
            iVideoSourceM4VFileSettings.iTimescale = 1000;
            iVideoSourceM4VFileSettings.iFrameHeight = 144;
            iVideoSourceM4VFileSettings.iFrameWidth = 176;
            iVideoSourceM4VFileSettings.iFrameRate = 5;
            if (!FileExists(VIDEO_SOURCE_M4V_FILENAME))
            {
                return false;
            }


            // create the video sinks
            iVideoSinkYUVFileSettings.iFileName = VIDEO_SINK_YUV_FILENAME;
            iVideoSinkYUVFileSettings.iMediaFormat = PVMF_MIME_YUV420;

            iVideoSinkH263FileSettings.iFileName = VIDEO_SINK_H263_FILENAME;
            iVideoSinkH263FileSettings.iMediaFormat = PVMF_MIME_H2632000;


            iVideoSinkM4VFileSettings.iFileName = VIDEO_SINK_M4V_FILENAME;
            iVideoSinkM4VFileSettings.iMediaFormat = PVMF_MIME_M4V;
            return true;
        }

        PvmiMIOFileInputSettings iAudioSourceRawFileSettings;
        PvmiMIOFileInputSettings iAudioSource2FileSettings;
        PvmiMIOFileInputSettings iAudioSource3FileSettings;
        PvmiMIOFileInputSettings iAudioSourceFileSettings;

        PvmiMIOFileInputSettings iVideoSourceYUVFileSettings;
        PvmiMIOFileInputSettings iVideoSourceH263FileSettings;
        PvmiMIOFileInputSettings iVideoSourceM4VFileSettings;

        PvmiMIOFileInputSettings iAudioSinkRawFileSettings;
        PvmiMIOFileInputSettings iAudioSink2FileSettings;
        PvmiMIOFileInputSettings iAudioSinkFileSettings;

        PvmiMIOFileInputSettings iVideoSinkYUVFileSettings;
        PvmiMIOFileInputSettings iVideoSinkH263FileSettings;
        PvmiMIOFileInputSettings iVideoSinkM4VFileSettings;
};

#endif
