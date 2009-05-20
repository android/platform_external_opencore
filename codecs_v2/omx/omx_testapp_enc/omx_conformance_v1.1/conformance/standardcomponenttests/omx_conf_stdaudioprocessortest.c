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
 * Copyright (c) 2006 The Khronos Group Inc.
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

/** OMX_CONF_StdAudioProcessorTest.c
 *  OpenMax IL conformance test - Standard Audio Processor Component Test
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "OMX_CONF_StdAudioProcessorTest.h"
#include <string.h>

#define TEST_NAME_STRING "StdAudioProcessorTest"

    /* StdCompAudioProcessorEndianness gives the native endianness of the
     * platform under which the tests are run
     */
    OMX_ENDIANTYPE StdCompAudioProcessorEndianness(void)
    {
        union
        {
            OMX_U32 u32;
            OMX_U8 bytes[4];
        } endian_union;

        endian_union.u32 = 1;
        return endian_union.bytes[0] ? OMX_EndianLittle : OMX_EndianBig;
    }

    /* Common port test for standard audio processors. Verifies port count,
     *  domain and directions. */
    OMX_ERRORTYPE StdAudioProcessorTest_Common(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_U32 nPortIndex;

        /* For the standard audio processor component, there must be at least
         *  two audio domain ports. */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying that there are two audio domain ports\n");
        if (pCtx->sPortParamAudio.nPorts < 2) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = APB + 0; input port, pcm format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying pcm input port 0 \n");
        nPortIndex = pCtx->sPortParamAudio.nStartPortNumber + 0;
        if (StdComponentTest_IsInputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " port direction correct\n");

        /* Verify - Port Index = APB + 1; output port, pcm format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying pcm output port 1 \n");
        nPortIndex = pCtx->sPortParamAudio.nStartPortNumber + 1;
        if (StdComponentTest_IsOutputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " port direction correct\n");

OMX_CONF_TEST_BAIL:

        return (eError);
    }



    /* PCM port parameter test checks the common parameters for pcm ports of the
     * standard audio processor components.
     * @param pCtx test context pointer
     * @param nPcmSamplingRateArrayLength length of the sampling rate array
     * @param pPcmSamplingRate the sampling rate array to be tested
     */
    OMX_ERRORTYPE StdCompCommonAudioProcessor_PcmPortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPcmSamplingRateArrayLength,
        OMX_U32 *pPcmSamplingRate)
    {
        OMX_U32 i, nStartPort;
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;
        OMX_AUDIO_PARAM_PORTFORMATTYPE sPortFormat;
        OMX_AUDIO_PARAM_PCMMODETYPE sFormatPCM;

        OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
        OMX_CONF_INIT_STRUCT(sPortFormat, OMX_AUDIO_PARAM_PORTFORMATTYPE);
        OMX_CONF_INIT_STRUCT(sFormatPCM, OMX_AUDIO_PARAM_PCMMODETYPE);

        nStartPort = pCtx->sPortParamAudio.nStartPortNumber;

        /* Verify support for the common standard component port parameters. */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying common standard component parameters for both ports\n");
        eError = StdComponentTest_StdPortParameters(pCtx, nStartPort);
        OMX_CONF_BAIL_ON_ERROR(eError);

        eError = StdComponentTest_StdPortParameters(pCtx, nStartPort + 1);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify Port Definitions */

        for (i = nStartPort; i <= nStartPort + 1; i++)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying from the port %d definition that the port domain is audio and format is pcm\n", i);
            sPortDefinition.nPortIndex = i;
            eError = OMX_GetParameter(pCtx->hWrappedComp,
                                      OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
            if ((sPortDefinition.eDomain != OMX_PortDomainAudio) ||
                    (sPortDefinition.format.audio.eEncoding != OMX_AUDIO_CodingPCM))
                eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
            OMX_CONF_BAIL_ON_ERROR(eError);
        }

        /* Verify support for OMX_IndexParamAudioPortFormat and verify
           that the port format is as expected.
        */
        for (i = nStartPort; i <= nStartPort + 1; i++)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for OMX_IndexParamAudioPortFormat, and that the port %d coding is pcm\n", i);
            sPortFormat.nPortIndex = i;
            eError = OMX_GetParameter(pCtx->hWrappedComp,
                                      OMX_IndexParamAudioPortFormat, (OMX_PTR) & sPortFormat);
            if (sPortFormat.eEncoding != OMX_AUDIO_CodingPCM)
                eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
            OMX_CONF_BAIL_ON_ERROR(eError);
        }

        /* Verify default settings for PCM */
        for (i = nStartPort; i <= nStartPort + 1; i++)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying the default PCM settings for port %d\n", i);
            OMX_CONF_INIT_STRUCT(sFormatPCM, OMX_AUDIO_PARAM_PCMMODETYPE);
            sFormatPCM.nPortIndex = i;
            eError = OMX_GetParameter(pCtx->hWrappedComp,
                                      OMX_IndexParamAudioPcm, (OMX_PTR) & sFormatPCM);
            if ((sFormatPCM.nChannels != 2) ||
                    (sFormatPCM.eNumData != OMX_NumericalDataSigned) ||
                    (sFormatPCM.nSamplingRate != 48000) ||
                    (sFormatPCM.ePCMMode != OMX_AUDIO_PCMModeLinear) ||
                    (sFormatPCM.eEndian != StdCompAudioProcessorEndianness()) ||
                    (sFormatPCM.bInterleaved != OMX_TRUE) ||
                    (sFormatPCM.nBitPerSample != 16) ||
                    (sFormatPCM.eChannelMapping[0] != OMX_AUDIO_ChannelLF) ||
                    (sFormatPCM.eChannelMapping[1] != OMX_AUDIO_ChannelRF))
                eError = OMX_ErrorBadParameter;/*Need a more informative error value. */
            OMX_CONF_BAIL_ON_ERROR(eError);
        }

        /* Verify all possible settings for OMX_IndexParamAudioPcm now. */
        /* Verify all allowed sample rates can be set. */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying that all allowed sample rates can be set for the input port\n");
        for (i = 0; i < nPcmSamplingRateArrayLength; i++)
        {
            sFormatPCM.nPortIndex = nStartPort;
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " %d Hz\n", pPcmSamplingRate[i]);
            OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamAudioPcm,
                                             (OMX_PTR)&sFormatPCM,
                                             sFormatPCM.nSamplingRate,
                                             pPcmSamplingRate[i], eError);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " Verifying that the output port sampling rate changed automatically\n");
            sFormatPCM.nSamplingRate = -1; // invalidate memory
            sFormatPCM.nPortIndex = nStartPort + 1;
            eError = OMX_GetParameter(pCtx->hWrappedComp,
                                      OMX_IndexParamAudioPcm, (OMX_PTR) & sFormatPCM);
            if (sFormatPCM.nSamplingRate != pPcmSamplingRate[i])
            {
                eError = OMX_ErrorBadParameter;/*Need a more informative error value. */
            }
            OMX_CONF_BAIL_ON_ERROR(eError);
        }

OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /* Equalizer test implementation */
    OMX_ERRORTYPE StdAudioProcessorTest_Equalizer(TEST_CTXTYPE *pCtx)
    {
        OMX_U32 PcmSamplingRate[2] = { 44100, 48000 };
        OMX_BOOL TrueFalse[2] = { OMX_TRUE, OMX_FALSE };
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_AUDIO_CONFIG_EQUALIZERTYPE sEqualizer;
        OMX_AUDIO_CONFIG_LOUDNESSTYPE sLoudness;
        OMX_AUDIO_CONFIG_BASSTYPE sBass;
        OMX_AUDIO_CONFIG_TREBLETYPE sTreble;
        OMX_S32 i;
        OMX_U32 band;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Setting role audio_processor.pcm.equalizer\n");
        eError = StdComponentTest_SetRole(pCtx, "audio_processor.pcm.equalizer");
        OMX_CONF_BAIL_ON_ERROR(eError);

        OMX_CONF_INIT_STRUCT(sEqualizer, OMX_AUDIO_CONFIG_EQUALIZERTYPE);
        OMX_CONF_INIT_STRUCT(sLoudness, OMX_AUDIO_CONFIG_LOUDNESSTYPE);
        OMX_CONF_INIT_STRUCT(sBass, OMX_AUDIO_CONFIG_BASSTYPE);
        OMX_CONF_INIT_STRUCT(sTreble, OMX_AUDIO_CONFIG_TREBLETYPE);

        eError = StdAudioProcessorTest_Common(pCtx);
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonAudioProcessor_PcmPortParameters(pCtx,
                 2, PcmSamplingRate);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify support for OMX_IndexConfigAudioEqualizer */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for OMX_IndexConfigAudioEqualizer\n");
        sEqualizer.nPortIndex = pCtx->sPortParamAudio.nStartPortNumber + 1;
        /* set values to such that will fail the default tests */
        sEqualizer.bEnable = OMX_TRUE;
        sTreble.bEnable = OMX_TRUE;
        sBass.bEnable = OMX_TRUE;
        sLoudness.bLoudness = OMX_TRUE;
        eError = OMX_GetConfig(pCtx->hWrappedComp,
                               OMX_IndexConfigAudioEqualizer, (OMX_PTR) & sEqualizer);
        OMX_CONF_BAIL_ON_ERROR(eError);
        /* verify defaults */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying defaults values for OMX_IndexConfigAudioEqualizer\n");
        if (sEqualizer.bEnable != OMX_FALSE)
            eError = OMX_ErrorBadParameter;/*Need a more informative error value. */
        OMX_CONF_BAIL_ON_ERROR(eError);

        sBass.nPortIndex = pCtx->sPortParamAudio.nStartPortNumber + 1;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for OMX_IndexConfigAudioBass\n");
        eError = OMX_GetConfig(pCtx->hWrappedComp,
                               OMX_IndexConfigAudioBass, (OMX_PTR) & sBass);
        OMX_CONF_BAIL_ON_ERROR(eError);
        /* verify defaults */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying defaults values for OMX_IndexConfigAudioBass\n");
        if (sBass.bEnable != OMX_FALSE)
            eError = OMX_ErrorBadParameter;/*Need a more informative error value. */
        OMX_CONF_BAIL_ON_ERROR(eError);

        sTreble.nPortIndex = pCtx->sPortParamAudio.nStartPortNumber + 1;
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for OMX_IndexConfigAudioTreble\n");
        eError = OMX_GetConfig(pCtx->hWrappedComp,
                               OMX_IndexConfigAudioTreble, (OMX_PTR) & sTreble);
        OMX_CONF_BAIL_ON_ERROR(eError);
        /* verify defaults */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying defaults values for OMX_IndexConfigAudioTreble\n");
        if (sTreble.bEnable != OMX_FALSE)
            eError = OMX_ErrorBadParameter;/*Need a more informative error value. */
        OMX_CONF_BAIL_ON_ERROR(eError);

        sLoudness.nPortIndex = pCtx->sPortParamAudio.nStartPortNumber + 1;
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for OMX_IndexConfigAudioLoudness\n");
        eError = OMX_GetConfig(pCtx->hWrappedComp,
                               OMX_IndexConfigAudioLoudness, (OMX_PTR) & sLoudness);
        OMX_CONF_BAIL_ON_ERROR(eError);
        /* verify defaults */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying defaults values for OMX_IndexConfigAudioLoudness\n");
        if (sLoudness.bLoudness != OMX_FALSE)
            eError = OMX_ErrorBadParameter;/*Need a more informative error value. */
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify true and false can be set for bEnabled in all structs. */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying true and false can be set for bEnabled in EQ, Bass, Treble and Loudness\n");
        for (i = 0; i < 2; i++)
        {
            OMX_CONF_CONFIG_READ_WRITE_VERIFY(pCtx, OMX_IndexConfigAudioEqualizer,
                                              (OMX_PTR)&sEqualizer,
                                              sEqualizer.bEnable,
                                              TrueFalse[i], !(TrueFalse[i]), eError);

            OMX_CONF_CONFIG_READ_WRITE_VERIFY(pCtx, OMX_IndexConfigAudioLoudness,
                                              (OMX_PTR)&sLoudness,
                                              sLoudness.bLoudness,
                                              TrueFalse[i], !(TrueFalse[i]), eError);

            OMX_CONF_CONFIG_READ_WRITE_VERIFY(pCtx, OMX_IndexConfigAudioBass,
                                              (OMX_PTR)&sBass,
                                              sBass.bEnable,
                                              TrueFalse[i], !(TrueFalse[i]), eError);

            OMX_CONF_CONFIG_READ_WRITE_VERIFY(pCtx, OMX_IndexConfigAudioTreble,
                                              (OMX_PTR)&sTreble,
                                              sTreble.bEnable,
                                              TrueFalse[i], !(TrueFalse[i]), eError);
        }

        /* verify that each EQ band allows setting at least the -1200 to 1200
         * millibel range */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying all EQ bands support -1200 to 1200 millibel range\n");
        for (band = sEqualizer.sBandIndex.nMin; band <= sEqualizer.sBandIndex.nMax;
                band++)
        {
            for (i = -1200; i <= 1200; i++)
            {
                sEqualizer.sBandIndex.nValue = band;
                OMX_CONF_CONFIG_READ_WRITE_VERIFY(pCtx,
                                                  OMX_IndexConfigAudioEqualizer,
                                                  (OMX_PTR)&sEqualizer,
                                                  sEqualizer.sBandLevel.nValue,
                                                  i,  10000, eError);
            }
        }

        /* verify that bass and treble allow setting the -100 to 100 range */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying bass and treble support -100 to 100 range\n");
        for (i = -100; i <= 100; i++)
        {
            OMX_CONF_CONFIG_READ_WRITE_VERIFY(pCtx, OMX_IndexConfigAudioBass,
                                              (OMX_PTR)&sBass,
                                              sBass.nBass,
                                              i, 10000, eError);

            OMX_CONF_CONFIG_READ_WRITE_VERIFY(pCtx, OMX_IndexConfigAudioTreble,
                                              (OMX_PTR)&sTreble,
                                              sTreble.nTreble,
                                              i, 10000, eError);
        }
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "End of StdAudioProcessorTest_Equalizer\n");

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /* Chorus test implementation */
    OMX_ERRORTYPE StdAudioProcessorTest_Chorus(TEST_CTXTYPE *pCtx)
    {
        OMX_U32 PcmSamplingRate[2] = { 44100, 48000 };
        OMX_BOOL TrueFalse[2] = { OMX_TRUE, OMX_FALSE };
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_AUDIO_CONFIG_CHORUSTYPE sChorus;
        OMX_U32 i;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Setting role audio_processor.pcm.chorus\n");
        eError = StdComponentTest_SetRole(pCtx, "audio_processor.pcm.chorus");
        OMX_CONF_BAIL_ON_ERROR(eError);

        OMX_CONF_INIT_STRUCT(sChorus, OMX_AUDIO_CONFIG_CHORUSTYPE);

        eError = StdAudioProcessorTest_Common(pCtx);
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonAudioProcessor_PcmPortParameters(pCtx,
                 2, PcmSamplingRate);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify support for OMX_IndexConfigAudioChorus */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for OMX_IndexConfigAudioChorus\n");
        sChorus.nPortIndex = pCtx->sPortParamAudio.nStartPortNumber + 1;
        /* set values to such that will fail the default test */
        sChorus.bEnable = OMX_TRUE;
        eError = OMX_GetConfig(pCtx->hWrappedComp,
                               OMX_IndexConfigAudioChorus, (OMX_PTR) & sChorus);
        OMX_CONF_BAIL_ON_ERROR(eError);
        /* verify defaults */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying defaults values for OMX_IndexConfigAudioChorus\n");
        if (sChorus.bEnable != OMX_FALSE)
            eError = OMX_ErrorBadParameter;/*Need a more informative error value. */
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify true and false can be set for bEnabled. */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying true and false can be set for bEnabled\n");
        for (i = 0; i < 2; i++)
        {
            OMX_CONF_CONFIG_READ_WRITE_VERIFY(pCtx, OMX_IndexConfigAudioChorus,
                                              (OMX_PTR)&sChorus,
                                              sChorus.bEnable,
                                              TrueFalse[i], !(TrueFalse[i]), eError);
        }
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "End of StdAudioProcessorTest_Chorus\n");

OMX_CONF_TEST_BAIL:
        return (eError);
    }

    /* Reverberation test implementation */
    OMX_ERRORTYPE StdAudioProcessorTest_Reverberation(TEST_CTXTYPE *pCtx)
    {
        OMX_U32 PcmSamplingRate[2] = { 44100, 48000 };
        OMX_BOOL TrueFalse[2] = { OMX_TRUE, OMX_FALSE };
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_AUDIO_CONFIG_REVERBERATIONTYPE sReverberation;
        OMX_U32 i;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Setting role audio_processor.pcm.reverberation\n");
        eError = StdComponentTest_SetRole(pCtx, "audio_processor.pcm.reverberation");
        OMX_CONF_BAIL_ON_ERROR(eError);

        OMX_CONF_INIT_STRUCT(sReverberation, OMX_AUDIO_CONFIG_REVERBERATIONTYPE);

        eError = StdAudioProcessorTest_Common(pCtx);
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonAudioProcessor_PcmPortParameters(pCtx,
                 2, PcmSamplingRate);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify support for OMX_IndexConfigAudioReverberation */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for OMX_IndexConfigAudioReverberation\n");
        sReverberation.nPortIndex = pCtx->sPortParamAudio.nStartPortNumber + 1;
        /* set values to such that will fail the default test */
        sReverberation.bEnable = OMX_TRUE;
        eError = OMX_GetConfig(pCtx->hWrappedComp,
                               OMX_IndexConfigAudioReverberation, (OMX_PTR) & sReverberation);
        OMX_CONF_BAIL_ON_ERROR(eError);
        /* verify defaults */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying defaults values for OMX_IndexConfigAudioReverberation\n");
        if (sReverberation.bEnable != OMX_FALSE)
            eError = OMX_ErrorBadParameter;/*Need a more informative error value. */
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify true and false can be set for bEnabled. */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying true and false can be set for bEnabled\n");
        for (i = 0; i < 2; i++)
        {
            OMX_CONF_CONFIG_READ_WRITE_VERIFY(pCtx,
                                              OMX_IndexConfigAudioReverberation,
                                              (OMX_PTR)&sReverberation,
                                              sReverberation.bEnable,
                                              TrueFalse[i], !(TrueFalse[i]), eError);
        }
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "End of StdAudioProcessorTest_Reverberation\n");

OMX_CONF_TEST_BAIL:
        return (eError);
    }

    /* Headphone Stereo Widening test implementation */
    OMX_ERRORTYPE StdAudioProcessorTest_StereoWideningHeadphones(
        TEST_CTXTYPE *pCtx)
    {
        OMX_U32 PcmSamplingRate[6] = { 16000, 22050, 24000, 32000, 44100, 48000 };
        OMX_BOOL TrueFalse[2] = { OMX_TRUE, OMX_FALSE };
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_AUDIO_CONFIG_STEREOWIDENINGTYPE sStereoWidening;
        OMX_U32 i;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Setting role audio_processor.pcm.stereo_widening_headphones\n");
        eError = StdComponentTest_SetRole(pCtx, "audio_processor.pcm.stereo_widening_headphones");
        OMX_CONF_BAIL_ON_ERROR(eError);

        OMX_CONF_INIT_STRUCT(sStereoWidening, OMX_AUDIO_CONFIG_STEREOWIDENINGTYPE);

        eError = StdAudioProcessorTest_Common(pCtx);
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonAudioProcessor_PcmPortParameters(pCtx,
                 6, PcmSamplingRate);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify support for OMX_IndexConfigAudioStereoWidening */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for OMX_IndexConfigAudioStereoWidening\n");
        sStereoWidening.nPortIndex = pCtx->sPortParamAudio.nStartPortNumber + 1;
        /* set values to such that will fail the default test */
        sStereoWidening.bEnable = OMX_TRUE;
        sStereoWidening.eWideningType = OMX_AUDIO_StereoWideningLoudspeakers;
        eError = OMX_GetConfig(pCtx->hWrappedComp,
                               OMX_IndexConfigAudioStereoWidening, (OMX_PTR) & sStereoWidening);
        OMX_CONF_BAIL_ON_ERROR(eError);
        /* verify defaults */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying defaults values for OMX_IndexConfigAudioStereoWidening\n");
        if ((sStereoWidening.bEnable != OMX_FALSE) ||
                (sStereoWidening.eWideningType != OMX_AUDIO_StereoWideningHeadphones))
            eError = OMX_ErrorBadParameter;/*Need a more informative error value. */
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify true and false can be set for bEnabled. */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying true and false can be set for bEnabled\n");
        for (i = 0; i < 2; i++)
        {
            OMX_CONF_CONFIG_READ_WRITE_VERIFY(pCtx,
                                              OMX_IndexConfigAudioStereoWidening,
                                              (OMX_PTR)&sStereoWidening,
                                              sStereoWidening.bEnable,
                                              TrueFalse[i], !(TrueFalse[i]), eError);
        }

        /* Verify the value range endpoints for nStereoWidening can be sent.
         * The actual value may not change as expected. Spec says it can be rounded
         * to nearest supported value. */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying the value range endpoints for nStereoWidening can be sent\n");
        sStereoWidening.nStereoWidening = 0;
        eError = OMX_SetConfig(pCtx->hWrappedComp,
                               OMX_IndexConfigAudioStereoWidening, (OMX_PTR) & sStereoWidening);
        OMX_CONF_BAIL_ON_ERROR(eError);
        sStereoWidening.nStereoWidening = 100;
        eError = OMX_SetConfig(pCtx->hWrappedComp,
                               OMX_IndexConfigAudioStereoWidening, (OMX_PTR) & sStereoWidening);
        OMX_CONF_BAIL_ON_ERROR(eError);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "End of StdAudioProcessorTest_StereoWideningHeadphones\n");

OMX_CONF_TEST_BAIL:
        return (eError);
    }

    /* Loudspeakers Stereo Widening test implementation */
    OMX_ERRORTYPE StdAudioProcessorTest_StereoWideningLoudspeakers(
        TEST_CTXTYPE *pCtx)
    {
        OMX_U32 PcmSamplingRate[6] = { 16000, 22050, 24000, 32000, 44100, 48000 };
        OMX_BOOL TrueFalse[2] = { OMX_TRUE, OMX_FALSE };
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_AUDIO_CONFIG_STEREOWIDENINGTYPE sStereoWidening;
        OMX_U32 i;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Setting role audio_processor.pcm.stereo_widening_loudspeakers\n");
        eError = StdComponentTest_SetRole(pCtx, "audio_processor.pcm.stereo_widening_loudspeakers");
        OMX_CONF_BAIL_ON_ERROR(eError);

        OMX_CONF_INIT_STRUCT(sStereoWidening, OMX_AUDIO_CONFIG_STEREOWIDENINGTYPE);

        eError = StdAudioProcessorTest_Common(pCtx);
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonAudioProcessor_PcmPortParameters(pCtx,
                 6, PcmSamplingRate);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify support for OMX_IndexConfigAudioStereoWidening */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for OMX_IndexConfigAudioStereoWidening\n");
        sStereoWidening.nPortIndex = pCtx->sPortParamAudio.nStartPortNumber + 1;
        /* set values to such that will fail the default test */
        sStereoWidening.bEnable = OMX_TRUE;
        sStereoWidening.eWideningType = OMX_AUDIO_StereoWideningHeadphones;
        eError = OMX_GetConfig(pCtx->hWrappedComp,
                               OMX_IndexConfigAudioStereoWidening, (OMX_PTR) & sStereoWidening);
        OMX_CONF_BAIL_ON_ERROR(eError);
        /* verify defaults */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying defaults values for OMX_IndexConfigAudioStereoWidening\n");
        if ((sStereoWidening.bEnable != OMX_FALSE) ||
                (sStereoWidening.eWideningType != OMX_AUDIO_StereoWideningLoudspeakers))
            eError = OMX_ErrorBadParameter;/*Need a more informative error value. */
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify true and false can be set for bEnabled. */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying true and false can be set for bEnabled\n");
        for (i = 0; i < 2; i++)
        {
            OMX_CONF_CONFIG_READ_WRITE_VERIFY(pCtx,
                                              OMX_IndexConfigAudioStereoWidening,
                                              (OMX_PTR)&sStereoWidening,
                                              sStereoWidening.bEnable,
                                              TrueFalse[i], !(TrueFalse[i]), eError);
        }

        /* Verify the value range endpoints for nStereoWidening can be sent.
         * The actual value may not change as expected. Spec says it can be rounded
         * to nearest supported value. */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying the value range endpoints for nStereoWidening can be sent\n");
        sStereoWidening.nStereoWidening = 0;
        eError = OMX_SetConfig(pCtx->hWrappedComp,
                               OMX_IndexConfigAudioStereoWidening, (OMX_PTR) & sStereoWidening);
        OMX_CONF_BAIL_ON_ERROR(eError);
        sStereoWidening.nStereoWidening = 100;
        eError = OMX_SetConfig(pCtx->hWrappedComp,
                               OMX_IndexConfigAudioStereoWidening, (OMX_PTR) & sStereoWidening);
        OMX_CONF_BAIL_ON_ERROR(eError);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "End of StdAudioProcessorTest_StereoWideningLoudspeakers\n");

OMX_CONF_TEST_BAIL:
        return (eError);
    }

    /* Test a component for compliance with the Standard Audio Processor
     * Equalizer. */
    OMX_ERRORTYPE OMX_CONF_StdAudioProcessorEqualizerTest(
        OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_CTXTYPE ctx;

        eError = StdComponentTest_IsRoleSupported(cComponentName, "audio_processor.pcm.equalizer");
        OMX_CONF_BAIL_ON_ERROR(eError);

        memset(&ctx, 0x0, sizeof(TEST_CTXTYPE));

        eError = StdComponentTest_StdComp(cComponentName, &ctx,
                                          (STDCOMPTEST_COMPONENT)StdAudioProcessorTest_Equalizer);

OMX_CONF_TEST_BAIL:
        return (eError);
    }

    /* Test a component for compliance with the Standard Audio Processor
     * Reverberation. */
    OMX_ERRORTYPE OMX_CONF_StdAudioProcessorReverberationTest(
        OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_CTXTYPE ctx;

        eError = StdComponentTest_IsRoleSupported(cComponentName, "audio_processor.pcm.reverberation");
        OMX_CONF_BAIL_ON_ERROR(eError);

        memset(&ctx, 0x0, sizeof(TEST_CTXTYPE));

        eError = StdComponentTest_StdComp(cComponentName, &ctx,
                                          (STDCOMPTEST_COMPONENT)StdAudioProcessorTest_Reverberation);

OMX_CONF_TEST_BAIL:
        return (eError);
    }

    /* Test a component for compliance with the Standard Audio Processor
     * Chorus. */
    OMX_ERRORTYPE OMX_CONF_StdAudioProcessorChorusTest(
        OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_CTXTYPE ctx;

        eError = StdComponentTest_IsRoleSupported(cComponentName, "audio_processor.pcm.chorus");
        OMX_CONF_BAIL_ON_ERROR(eError);

        memset(&ctx, 0x0, sizeof(TEST_CTXTYPE));

        eError = StdComponentTest_StdComp(cComponentName, &ctx,
                                          (STDCOMPTEST_COMPONENT)StdAudioProcessorTest_Chorus);

OMX_CONF_TEST_BAIL:
        return (eError);
    }

    /* Test a component for compliance with the Standard Audio Processor
     * StereoWideningHeadphones. */
    OMX_ERRORTYPE OMX_CONF_StdAudioProcessorStereoWideningHeadphonesTest(
        OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_CTXTYPE ctx;

        eError = StdComponentTest_IsRoleSupported(cComponentName, "audio_processor.pcm.stereo_widening_headphones");
        OMX_CONF_BAIL_ON_ERROR(eError);

        memset(&ctx, 0x0, sizeof(TEST_CTXTYPE));

        eError = StdComponentTest_StdComp(cComponentName, &ctx,
                                          (STDCOMPTEST_COMPONENT)StdAudioProcessorTest_StereoWideningHeadphones);

OMX_CONF_TEST_BAIL:
        return (eError);
    }

    /* Test a component for compliance with the Standard Audio Processor
     * StereoWideningLoudspeakers. */
    OMX_ERRORTYPE OMX_CONF_StdAudioProcessorStereoWideningLoudspeakersTest(
        OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_CTXTYPE ctx;

        eError = StdComponentTest_IsRoleSupported(cComponentName, "audio_processor.pcm.stereo_widening_loudspeakers");
        OMX_CONF_BAIL_ON_ERROR(eError);

        memset(&ctx, 0x0, sizeof(TEST_CTXTYPE));

        eError = StdComponentTest_StdComp(cComponentName, &ctx,
                                          (STDCOMPTEST_COMPONENT)StdAudioProcessorTest_StereoWideningLoudspeakers);

OMX_CONF_TEST_BAIL:
        return (eError);
    }

    /* This runs through and tests all standard components of the audio processor
     * class exposed and supported by the component. */
    OMX_ERRORTYPE OMX_CONF_StdAudioProcessorTest(
        OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_U32 nNumRoles;
        OMX_STRING sRoles[MAX_COMPONENT_ROLES] = {NULL};
        OMX_U32 i;

        /* The following utility function calls OMX_GetRolesOfComponent,
           allocates memory, and populates strings.
        */
        eError = StdComponentTest_PopulateRolesArray(cComponentName, &nNumRoles, sRoles);
        OMX_CONF_BAIL_ON_ERROR(eError);

        eError = OMX_ErrorComponentNotFound;
        for (i = 0; i < nNumRoles; i++)
        {
            if (strstr(sRoles[i], "audio_processor.pcm.chorus") != NULL)
            {
                eError = OMX_CONF_StdAudioProcessorChorusTest(cComponentName);
            }
            else if (strstr(sRoles[i], "audio_processor.pcm.equalizer") != NULL)
            {
                eError = OMX_CONF_StdAudioProcessorEqualizerTest(cComponentName);
            }
            else if (strstr(sRoles[i], "audio_processor.pcm.equalizer") != NULL)
            {
                eError = OMX_CONF_StdAudioProcessorEqualizerTest(cComponentName);
            }
            else if (strstr(sRoles[i], "audio_processor.pcm.reverberation") != NULL)
            {
                eError = OMX_CONF_StdAudioProcessorReverberationTest(cComponentName);
            }
            else if (strstr(sRoles[i], "audio_processor.pcm.stereo_widening_headphones") != NULL)
            {
                eError = OMX_CONF_StdAudioProcessorStereoWideningHeadphonesTest(cComponentName);
            }
            else if (strstr(sRoles[i], "audio_processor.pcm.stereo_widening_loudspeakers") != NULL)
            {
                eError = OMX_CONF_StdAudioProcessorStereoWideningLoudspeakersTest(cComponentName);
            }
            else
            {
                continue;
            }
            OMX_CONF_BAIL_ON_ERROR(eError);
        }

OMX_CONF_TEST_BAIL:
        StdComponentTest_FreeRolesArray(nNumRoles, sRoles);
        return (eError);
    }

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */
