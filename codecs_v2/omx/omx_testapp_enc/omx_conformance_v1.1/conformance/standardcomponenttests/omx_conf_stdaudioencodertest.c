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

/** OMX_CONF_StdAudioEncoderTest.c
 *  OpenMax IL conformance test - Standard Audio Encoder Component Test
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "OMX_CONF_StdCompCommon.h"
#include <string.h>

#define TEST_NAME_STRING "StdAudioEncoderTest"


    /**************************** G L O B A L S **********************************/

    /*****************************************************************************/
    OMX_ERRORTYPE StdAudioEncoderTest_Mp3Encoder(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_U32 nPortIndex;

        eError = StdComponentTest_SetRole(pCtx, "audio_encoder.mp3");
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* For the standard MP3 encoder component, there must be at least two audio domain ports. */
        if (pCtx->sPortParamAudio.nPorts < 2) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = APB + 0; input port, pcm format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying pcm input port 0 \n");
        nPortIndex = pCtx->sPortParamAudio.nStartPortNumber + 0;
        if (StdComponentTest_IsInputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonAudio_PcmPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = APB + 1; outpt port, mp3 format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying Mp3 output port 1 \n");
        nPortIndex = pCtx->sPortParamAudio.nStartPortNumber + 1;
        if (StdComponentTest_IsOutputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonAudio_Mp3PortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdAudioEncoderTest_AacEncoder(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_U32 nPortIndex;

        eError = StdComponentTest_SetRole(pCtx, "audio_encoder.aac");
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* For the standard AAC encoder component, there must be at least two audio domain ports. */
        if (pCtx->sPortParamAudio.nPorts < 2) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = APB + 0; input port, PCM format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying PCM input port 0 \n");
        nPortIndex = pCtx->sPortParamAudio.nStartPortNumber + 0;
        if (StdComponentTest_IsInputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonAudio_PcmPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = APB + 1; outpt port, AAC format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying AAC output port 1 \n");
        nPortIndex = pCtx->sPortParamAudio.nStartPortNumber + 1;
        if (StdComponentTest_IsOutputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonAudio_AacPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    /* Test a component for compliance with the Standard MP3 Encoder. */

    OMX_ERRORTYPE OMX_CONF_StdMp3EncoderTest(
        OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_CTXTYPE ctx;

        eError = StdComponentTest_IsRoleSupported(cComponentName, "audio_encoder.mp3");
        OMX_CONF_BAIL_ON_ERROR(eError);

        memset(&ctx, 0x0, sizeof(TEST_CTXTYPE));

        eError = StdComponentTest_StdComp(cComponentName, &ctx,
                                          (STDCOMPTEST_COMPONENT)StdAudioEncoderTest_Mp3Encoder);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    /* Test a component for compliance with the Standard AAC Encoder. */

    OMX_ERRORTYPE OMX_CONF_StdAacEncoderTest(
        OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_CTXTYPE ctx;

        eError = StdComponentTest_IsRoleSupported(cComponentName, "audio_encoder.aac");
        OMX_CONF_BAIL_ON_ERROR(eError);

        memset(&ctx, 0x0, sizeof(TEST_CTXTYPE));

        eError = StdComponentTest_StdComp(cComponentName, &ctx,
                                          (STDCOMPTEST_COMPONENT)StdAudioEncoderTest_AacEncoder);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    /* This runs through and tests all standard components of the audio encoder class
       exposed and supported by the component.
    */
    OMX_ERRORTYPE OMX_CONF_StdAudioEncoderTest(
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
            if (strstr(sRoles[i], "audio_encoder.mp3") != NULL)
            {
                eError = OMX_CONF_StdMp3EncoderTest(cComponentName);
            }
            else if (strstr(sRoles[i], "audio_encoder.aac") != NULL)
            {
                eError = OMX_CONF_StdAacEncoderTest(cComponentName);
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

    /*****************************************************************************/



#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */
