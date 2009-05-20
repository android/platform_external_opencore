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

/** OMX_CONF_StdAudioProcessorTest.h
 *  OpenMax IL conformance test - Standard Audio Processor Component Test
 *  header
 */

#ifndef OMX_CONF_STDAUDIOPROCESSORTEST_H_
#define OMX_CONF_STDAUDIOPROCESSORTEST_H_

#include "OMX_CONF_StdCompCommon.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /* Common port test for standard audio processors. Verifies port count,
     * domain and directions. */
    OMX_ERRORTYPE StdAudioProcessorTest_Common(TEST_CTXTYPE *pCtx);

    /* StdCompAudioProcessorEndianness gives the native endianness of the
     * platform under which the tests are run */
    OMX_ENDIANTYPE StdCompAudioProcessorEndianness(void);

    /* PCM port parameter test checks the common parameters for pcm ports of the
     * standard audio processor components.
     * @param pCtx test context pointer
     * @param nPcmSamplingRateArrayLength length of the sampling rate array
     * @param pPcmSamplingRate the sampling rate array to be tested
     */
    OMX_ERRORTYPE StdCompCommonAudioProcessor_PcmPortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPcmSamplingRateArrayLength, OMX_U32 *pPcmSamplingRate);

    /* Equalizer test */
    OMX_ERRORTYPE StdAudioProcessorTest_Equalizer(TEST_CTXTYPE *pCtx);

    /* Chorus test */
    OMX_ERRORTYPE StdAudioProcessorTest_Chorus(TEST_CTXTYPE *pCtx);

    /* Reverberation test */
    OMX_ERRORTYPE StdAudioProcessorTest_Reverberation(TEST_CTXTYPE *pCtx);

    /* Headphone Stereo Widening test */
    OMX_ERRORTYPE StdAudioProcessorTest_StereoWideningHeadphones(
        TEST_CTXTYPE *pCtx);

    /* Loudspeaker Stereo Widening test */
    OMX_ERRORTYPE StdAudioProcessorTest_StereoWideningLoudspeakers(
        TEST_CTXTYPE *pCtx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*OMX_CONF_STDAUDIOPROCESSORTEST_H_*/
