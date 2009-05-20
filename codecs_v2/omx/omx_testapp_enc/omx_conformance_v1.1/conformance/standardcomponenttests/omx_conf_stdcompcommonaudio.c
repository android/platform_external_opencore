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
/*
 * Copyright (c) 2005 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/** OMX_CONF_StdCompCommonAudio.c
 *  OpenMax IL conformance test - Standard Component Test
 *  Contains common code that can be reused by various standard audio component tests.
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "OMX_CONF_StdCompCommon.h"

#include <string.h>

    /**************************** G L O B A L S **********************************/

    /*****************************************************************************/

    OMX_ERRORTYPE StdCompCommonAudio_Mp3PortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {
        const OMX_U32 Mp3SampleRate[9] =
            {8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000};

        OMX_U32 i;
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;
        OMX_AUDIO_PARAM_PORTFORMATTYPE sPortFormat;
        OMX_AUDIO_PARAM_MP3TYPE sFormatMP3;
        //This structure is not a part of new headrer file
        //OMX_AUDIO_PARAM_MP3STREAMFORMATTYPE sMp3StreamFormat;

        OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
        OMX_CONF_INIT_STRUCT(sPortFormat, OMX_AUDIO_PARAM_PORTFORMATTYPE);
        OMX_CONF_INIT_STRUCT(sFormatMP3, OMX_AUDIO_PARAM_MP3TYPE);
        //This structure is not a part of new headrer file
        //OMX_CONF_INIT_STRUCT(sMp3StreamFormat, OMX_AUDIO_PARAM_MP3STREAMFORMATTYPE);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying MP3 port %i Default parameters\n", nPortIndex);

        /* Verify support for the common standard component port parameters. */
        eError = StdComponentTest_StdPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify Port Definition */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying MP3 port definition\n");
        sPortDefinition.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
        if ((sPortDefinition.eDomain != OMX_PortDomainAudio) ||
                (sPortDefinition.format.audio.eEncoding != OMX_AUDIO_CodingMP3))
            eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eEncoding = OMX_AUDIO_CodingMP3\n");

        /* Verify support for OMX_IndexParamAudioPortFormat and verify
           that the port format is as expected.
        */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying component supports OMX_IndexParamAudioPortFormat\n");
        sPortFormat.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamAudioPortFormat, (OMX_PTR) & sPortFormat);
        if (sPortFormat.eEncoding != OMX_AUDIO_CodingMP3)
            eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eEncoding = OMX_AUDIO_CodingMP3\n");

        /* Verify default settings for MP3 */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying default values for MP3\n");
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamAudioMp3, (OMX_PTR) & sFormatMP3);
        if ((sFormatMP3.nChannels != 2) ||
                (sFormatMP3.nSampleRate != 44100) ||
                (sFormatMP3.eChannelMode != OMX_AUDIO_ChannelModeStereo))
            eError = OMX_ErrorBadParameter;  /* Need a more informative error value. */
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nChannels = %d, nSampleRate = %d,"
                       "eChannelMode = %d\n", sFormatMP3.nChannels,
                       sFormatMP3.nSampleRate, sFormatMP3.eChannelMode);

        /* Verify all possible settings for OMX_IndexParamAudioMp3 now. */

        /* Verify all allowed channels could be specified. */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying number of audio channels supported\n");
        for (i = 0; i < 2; i++)
        {
            OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioMp3,
                                             (OMX_PTR)&sFormatMP3,
                                             sFormatMP3.nChannels,
                                             i + 1, eError);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " %d \n", sFormatMP3.nChannels);
        }

        /* Verify all allowed sample rates can be set. */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for sample rates =\n");
        for (i = 0; i < 9; i++)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " %d ", Mp3SampleRate[i]);
            OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioMp3,
                                             (OMX_PTR)&sFormatMP3,
                                             sFormatMP3.nSampleRate,
                                             Mp3SampleRate[i], eError);
        }
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\n ");

        /* Verify all allowed AudioBandWidth values can be set. */
        for (i = 0; i < 2; i++)
        {
            OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioMp3,
                                             (OMX_PTR)&sFormatMP3,
                                             sFormatMP3.nAudioBandWidth,
                                             i, eError);
        }

        /* Verify all allowed eChannelMode values can be set. */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for eChannelMode values\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_AUDIO_ChannelModeStereo\n");
        OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioMp3,
                                         (OMX_PTR)&sFormatMP3,
                                         sFormatMP3.eChannelMode,
                                         OMX_AUDIO_ChannelModeStereo, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_AUDIO_ChannelModeJointStereo\n");
        OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioMp3,
                                         (OMX_PTR)&sFormatMP3,
                                         sFormatMP3.eChannelMode,
                                         OMX_AUDIO_ChannelModeJointStereo, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_AUDIO_ChannelModeDual\n");
        OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioMp3,
                                         (OMX_PTR)&sFormatMP3,
                                         sFormatMP3.eChannelMode,
                                         OMX_AUDIO_ChannelModeDual, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_AUDIO_ChannelModeMono\n");
        OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioMp3,
                                         (OMX_PTR)&sFormatMP3,
                                         sFormatMP3.eChannelMode,
                                         OMX_AUDIO_ChannelModeMono, eError);
//This structure is not a part of new headrer file

OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdCompCommonAudio_AacPortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {
        const OMX_U32 AacSampleRate[9] =
            {8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000};

        OMX_U32 i;
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;
        OMX_AUDIO_PARAM_PORTFORMATTYPE sPortFormat;
        OMX_AUDIO_PARAM_AACPROFILETYPE sFormatAAC;

        OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
        OMX_CONF_INIT_STRUCT(sPortFormat, OMX_AUDIO_PARAM_PORTFORMATTYPE);
        OMX_CONF_INIT_STRUCT(sFormatAAC, OMX_AUDIO_PARAM_AACPROFILETYPE);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying AAC Port %i Default parameters\n", nPortIndex);

        /* Verify support for the common standard component port parameters. */
        eError = StdComponentTest_StdPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify Port Definition */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying AAC port definition\n");
        sPortDefinition.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
        if ((sPortDefinition.eDomain != OMX_PortDomainAudio) ||
                (sPortDefinition.format.audio.eEncoding != OMX_AUDIO_CodingAAC))
            eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eEncoding = OMX_AUDIO_CodingAAC\n");

        /* Verify support for OMX_IndexParamAudioPortFormat and verify
           that the port format is as expected.
        */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying component supports OMX_IndexParamAudioPortFormat\n");
        sPortFormat.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamAudioPortFormat, (OMX_PTR) & sPortFormat);
        if (sPortFormat.eEncoding != OMX_AUDIO_CodingAAC)
            eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eEncoding = OMX_AUDIO_CodingAAC\n");

        /* Verify default settings for AAC */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying default values for AAC Profile\n");
        OMX_CONF_INIT_STRUCT(sFormatAAC, OMX_AUDIO_PARAM_AACPROFILETYPE);
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamAudioAac, (OMX_PTR) & sFormatAAC);
        if ((sFormatAAC.nChannels != 2) ||
                (sFormatAAC.nSampleRate != 44100) ||
                (sFormatAAC.eAACProfile != OMX_AUDIO_AACObjectLC) ||
                (sFormatAAC.eAACStreamFormat != OMX_AUDIO_AACStreamFormatMP2ADTS) ||
                (sFormatAAC.eChannelMode != OMX_AUDIO_ChannelModeStereo))
            eError = OMX_ErrorBadParameter;  /* Need a more informative error value. */
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nChannels = %d, nSampleRate = %d, "
                       "eAACProfile = %d, eChannelMode = %d, eAACStreamFormat = %d\n",
                       sFormatAAC.nChannels, sFormatAAC.nSampleRate,
                       sFormatAAC.eAACProfile, sFormatAAC.eAACStreamFormat,
                       sFormatAAC.eChannelMode);

        /* Verify all possible settings for OMX_IndexParamAudioAac now. */

        /* Verify all allowed channels could be specified. */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying number of audio channels supported\n");
        for (i = 0; i < 2; i++)
        {
            OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioAac,
                                             (OMX_PTR)&sFormatAAC,
                                             sFormatAAC.nChannels,
                                             i + 1, eError);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " %d \n", sFormatAAC.nChannels);
        }

        /* Verify all allowed sample rates can be set. */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for sample rates =\n");
        for (i = 0; i < 10; i++)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " %d ", AacSampleRate[i]);
            OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioAac,
                                             (OMX_PTR)&sFormatAAC,
                                             sFormatAAC.nSampleRate,
                                             AacSampleRate[i], eError);
        }
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\n ");

        /* Verify all allowed AudioBandWidth values can be set. */
        for (i = 0; i < 2; i++)
        {
            OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioAac,
                                             (OMX_PTR)&sFormatAAC,
                                             sFormatAAC.nAudioBandWidth,
                                             i, eError);
        }

        /* Verify all allowed eAACProfile values can be set. */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for eAACProfile values\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_AUDIO_AACObjectLC\n");
        OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioAac,
                                         (OMX_PTR)&sFormatAAC,
                                         sFormatAAC.eAACProfile,
                                         OMX_AUDIO_AACObjectLC, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_AUDIO_AACObjectHE\n");
        OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioAac,
                                         (OMX_PTR)&sFormatAAC,
                                         sFormatAAC.eAACProfile,
                                         OMX_AUDIO_AACObjectHE, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_AUDIO_AACObjectHE_PS\n");
        OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioAac,
                                         (OMX_PTR)&sFormatAAC,
                                         sFormatAAC.eAACProfile,
                                         OMX_AUDIO_AACObjectHE_PS, eError);

        /* Verify all allowed eAACStreamFormat values can be set. */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for eAACStreamFormat values\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_AUDIO_AACStreamFormatMP2ADTS\n");
        OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioAac,
                                         (OMX_PTR)&sFormatAAC,
                                         sFormatAAC.eAACStreamFormat,
                                         OMX_AUDIO_AACStreamFormatMP2ADTS, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_AUDIO_AACStreamFormatMP4ADTS\n");
        OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioAac,
                                         (OMX_PTR)&sFormatAAC,
                                         sFormatAAC.eAACStreamFormat,
                                         OMX_AUDIO_AACStreamFormatMP4ADTS, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_AUDIO_AACStreamFormatADIF\n");
        OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioAac,
                                         (OMX_PTR)&sFormatAAC,
                                         sFormatAAC.eAACStreamFormat,
                                         OMX_AUDIO_AACStreamFormatADIF, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_AUDIO_AACStreamFormatRAW\n");
        OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioAac,
                                         (OMX_PTR)&sFormatAAC,
                                         sFormatAAC.eAACStreamFormat,
                                         OMX_AUDIO_AACStreamFormatRAW, eError);

        /* Verify all allowed eChannelMode values can be set. */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for eChannelMode values\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_AUDIO_ChannelModeStereo\n");
        OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioAac,
                                         (OMX_PTR)&sFormatAAC,
                                         sFormatAAC.eChannelMode,
                                         OMX_AUDIO_ChannelModeStereo, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_AUDIO_ChannelModeMono\n");
        OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioAac,
                                         (OMX_PTR)&sFormatAAC,
                                         sFormatAAC.eChannelMode,
                                         OMX_AUDIO_ChannelModeMono, eError);

OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdCompCommonAudio_RealAudioPortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        UNUSED_PARAMETER(pCtx);
        UNUSED_PARAMETER(nPortIndex);

        // Placeholder for actual real audio code.

        //OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdCompCommonAudio_WmaPortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        UNUSED_PARAMETER(pCtx);
        UNUSED_PARAMETER(nPortIndex);

        // Placeholder for actual real audio code.

        //OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdCompCommonAudio_PcmPortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {
        const OMX_U32 PcmSamplingRate[9] =
            {8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000};

        OMX_U32 i;
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;
        OMX_AUDIO_PARAM_PORTFORMATTYPE sPortFormat;
        OMX_AUDIO_PARAM_PCMMODETYPE sFormatPCM;

        OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
        OMX_CONF_INIT_STRUCT(sPortFormat, OMX_AUDIO_PARAM_PORTFORMATTYPE);
        OMX_CONF_INIT_STRUCT(sFormatPCM, OMX_AUDIO_PARAM_PCMMODETYPE);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying PCM Port %i Default parameters\n", nPortIndex);

        /* Verify support for the common standard component port parameters. */
        eError = StdComponentTest_StdPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify Port Definition */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying PCM port definition\n");
        sPortDefinition.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
        if ((sPortDefinition.eDomain != OMX_PortDomainAudio) ||
                (sPortDefinition.format.audio.eEncoding != OMX_AUDIO_CodingPCM))
            eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eEncoding = OMX_AUDIO_CodingPCM\n");

        /* Verify support for OMX_IndexParamAudioPortFormat and verify
           that the port format is as expected.
        */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying component supports OMX_IndexParamAudioPortFormat\n");
        sPortFormat.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamAudioPortFormat, (OMX_PTR) & sPortFormat);
        if (sPortFormat.eEncoding != OMX_AUDIO_CodingPCM)
            eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eEncoding = OMX_AUDIO_CodingPCM\n");

        /* Verify default settings for PCM */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying default values for PCM Param\n");
        OMX_CONF_INIT_STRUCT(sFormatPCM, OMX_AUDIO_PARAM_PCMMODETYPE);
        sFormatPCM.nPortIndex = nPortIndex; // Make sure asking for the right port!
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamAudioPcm, (OMX_PTR) & sFormatPCM);
        if ((sFormatPCM.nChannels != 2) ||
                (sFormatPCM.eNumData != OMX_NumericalDataSigned) ||
                (sFormatPCM.bInterleaved != OMX_TRUE) ||
                (sFormatPCM.nBitPerSample != 16) ||
                (sFormatPCM.nSamplingRate != 44100) ||
                (sFormatPCM.ePCMMode != OMX_AUDIO_PCMModeLinear) ||
                (sFormatPCM.eChannelMapping[0] != OMX_AUDIO_ChannelLF) ||
                (sFormatPCM.eChannelMapping[1] != OMX_AUDIO_ChannelRF))
            eError = OMX_ErrorBadParameter;  /* Need a more informative error value. */
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nChannels = %d, eNumData = %d, "
                       "bInterleaved = %d, nBitPerSample = %d, "
                       "nSamplingRate = %d, ePCMMode = %d\n",
                       sFormatPCM.nChannels, sFormatPCM.eNumData,
                       sFormatPCM.bInterleaved, sFormatPCM.nBitPerSample,
                       sFormatPCM.nSamplingRate, sFormatPCM.ePCMMode);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eChannelMapping[0] = %d, eChannelMapping[1] = %d\n",
                       sFormatPCM.eChannelMapping[0], sFormatPCM.eChannelMapping[1]);

        /* Verify all possible settings for OMX_IndexParamAudioPcm now. */
        /* Verify all allowed channels could be specified. */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying number of audio channels supported\n");
        for (i = 0; i < 2; i++)
        {
            OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioPcm,
                                             (OMX_PTR)&sFormatPCM,
                                             sFormatPCM.nChannels,
                                             i + 1, eError);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " %d \n", sFormatPCM.nChannels);
        }

        /* Verify all allowed sample rates can be set. */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for sample rates\n");
        for (i = 0; i < 9; i++)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " %d ", PcmSamplingRate[i]);
            OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioPcm,
                                             (OMX_PTR)&sFormatPCM,
                                             sFormatPCM.nSamplingRate,
                                             PcmSamplingRate[i], eError);
        }
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\n ");

        /* Verify all allowed eNumData values can be set. */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for eNumData values\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_NumericalDataSigned\n");
        OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioPcm,
                                         (OMX_PTR)&sFormatPCM,
                                         sFormatPCM.eNumData,
                                         OMX_NumericalDataSigned, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_NumericalDataUnsigned\n");
        OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioPcm,
                                         (OMX_PTR)&sFormatPCM,
                                         sFormatPCM.eNumData,
                                         OMX_NumericalDataUnsigned, eError);

        /* Verify all allowed ePCMMode values can be set. */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for ePCMMode values\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_AUDIO_PCMModeLinear\n");
        OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioPcm,
                                         (OMX_PTR)&sFormatPCM,
                                         sFormatPCM.ePCMMode,
                                         OMX_AUDIO_PCMModeLinear, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_AUDIO_PCMModeALaw\n");
        OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioPcm,
                                         (OMX_PTR)&sFormatPCM,
                                         sFormatPCM.ePCMMode,
                                         OMX_AUDIO_PCMModeALaw, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_AUDIO_PCMModeMULaw\n");
        OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioPcm,
                                         (OMX_PTR)&sFormatPCM,
                                         sFormatPCM.ePCMMode,
                                         OMX_AUDIO_PCMModeMULaw, eError);

OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/


#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */
