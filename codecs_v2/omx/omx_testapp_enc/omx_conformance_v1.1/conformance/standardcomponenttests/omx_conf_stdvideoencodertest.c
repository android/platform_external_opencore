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

/** OMX_CONF_StdVideoEncoderTest.c
 *  OpenMax IL conformance test - Standard Video Encoder Component Test
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "OMX_CONF_StdCompCommon.h"
#include <string.h>

#define TEST_NAME_STRING "StdVideoEncoderTest"


    /**************************** G L O B A L S **********************************/

    /*****************************************************************************/

    OMX_ERRORTYPE StdVideoEncoderTest_H263Encoder(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_U32 nPortIndex;

        eError = StdComponentTest_SetRole(pCtx, "video_encoder.h263");
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* For the standard H.263 encoder component, there must be at least two video domain ports. */
        if (pCtx->sPortParamVideo.nPorts < 2) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = VPB + 0; input port, raw video format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying Raw video input port 0 \n");
        nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 0;
        if (StdComponentTest_IsInputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonVideo_RawPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = VPB + 1; output port, H.263 format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying H.263 output port 1 \n");
        nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 1;
        if (StdComponentTest_IsOutputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonVideo_H263PortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);


OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdVideoEncoderTest_AvcEncoder(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_U32 nPortIndex;

        eError = StdComponentTest_SetRole(pCtx, "video_encoder.avc");
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* For the standard AVC encoder component, there must be at least two video domain ports. */
        if (pCtx->sPortParamVideo.nPorts < 2) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = VPB + 0; input port, raw video format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying Raw video input port 0 \n");
        nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 0;
        if (StdComponentTest_IsInputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonVideo_RawPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = VPB + 1; output port, AVC format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying AVC output port 1 \n");
        nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 1;
        if (StdComponentTest_IsOutputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonVideo_AvcPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);


OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdVideoEncoderTest_Mpeg4Encoder(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_U32 nPortIndex;

        OMX_CONFIG_FRAMERATETYPE sVideoFrameRate;
        OMX_VIDEO_CONFIG_BITRATETYPE sVideoBitRate;

        OMX_CONF_INIT_STRUCT(sVideoFrameRate, OMX_CONFIG_FRAMERATETYPE);
        OMX_CONF_INIT_STRUCT(sVideoBitRate, OMX_VIDEO_CONFIG_BITRATETYPE);

        eError = StdComponentTest_SetRole(pCtx, "video_encoder.mpeg4");
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* For the standard MPEG4 encoder component, there must be at least two video domain ports. */
        if (pCtx->sPortParamVideo.nPorts < 2) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = VPB + 0; input port, raw video format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying Raw video input port 0 \n");
        nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 0;
        if (StdComponentTest_IsInputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonVideo_RawPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = VPB + 1; output port, MPEG4 format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying MPEG4 output port 1 \n");
        nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 1;
        if (StdComponentTest_IsOutputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonVideo_Mpeg4PortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify support for OMX_IndexConfigVideoFramerate and verify the default value. */
        sVideoFrameRate.nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 1;
        eError = OMX_GetConfig(pCtx->hWrappedComp, OMX_IndexConfigVideoFramerate, (OMX_PTR) & sVideoFrameRate);
        if (sVideoFrameRate.xEncodeFramerate != (15 << 16))
            eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify this value can be changed.*/
        OMX_CONF_CONFIG_READ_WRITE_VERIFY(pCtx, OMX_IndexConfigVideoFramerate,
                                          (OMX_PTR)&sVideoFrameRate,
                                          sVideoFrameRate.xEncodeFramerate,
                                          (5 << 16), (10 << 16), eError);

        /* Verify support for OMX_IndexConfigVideoBitrate and verify the default value. */
        sVideoBitRate.nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 1;
        eError = OMX_GetConfig(pCtx->hWrappedComp, OMX_IndexConfigVideoBitrate, (OMX_PTR) & sVideoBitRate);
        if (sVideoBitRate.nEncodeBitrate != 64000)
            eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify this value can be changed.*/
        OMX_CONF_CONFIG_READ_WRITE_VERIFY(pCtx, OMX_IndexConfigVideoBitrate,
                                          (OMX_PTR)&sVideoBitRate,
                                          sVideoBitRate.nEncodeBitrate,
                                          32000, 11111, eError);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    /* Test a component for compliance with the Standard H.263 Encoder. */

    OMX_ERRORTYPE OMX_CONF_StdH263EncoderTest(
        OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_CTXTYPE ctx;

        eError = StdComponentTest_IsRoleSupported(cComponentName, "video_encoder.h263");
        OMX_CONF_BAIL_ON_ERROR(eError);

        memset(&ctx, 0x0, sizeof(TEST_CTXTYPE));

        eError = StdComponentTest_StdComp(cComponentName, &ctx,
                                          (STDCOMPTEST_COMPONENT)StdVideoEncoderTest_H263Encoder);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    /* Test a component for compliance with the Standard AVC Encoder. */

    OMX_ERRORTYPE OMX_CONF_StdAvcEncoderTest(
        OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_CTXTYPE ctx;

        eError = StdComponentTest_IsRoleSupported(cComponentName, "video_encoder.avc");
        OMX_CONF_BAIL_ON_ERROR(eError);

        memset(&ctx, 0x0, sizeof(TEST_CTXTYPE));

        eError = StdComponentTest_StdComp(cComponentName, &ctx,
                                          (STDCOMPTEST_COMPONENT)StdVideoEncoderTest_AvcEncoder);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    /* Test a component for compliance with the Standard MPEG4 Encoder. */

    OMX_ERRORTYPE OMX_CONF_StdMpeg4EncoderTest(
        OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_CTXTYPE ctx;

        eError = StdComponentTest_IsRoleSupported(cComponentName, "video_encoder.mpeg4");
        OMX_CONF_BAIL_ON_ERROR(eError);

        memset(&ctx, 0x0, sizeof(TEST_CTXTYPE));

        eError = StdComponentTest_StdComp(cComponentName, &ctx,
                                          (STDCOMPTEST_COMPONENT)StdVideoEncoderTest_Mpeg4Encoder);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    /* This runs through and tests all standard components of the video encoder class
       exposed and supported by the component.
    */
    OMX_ERRORTYPE OMX_CONF_StdVideoEncoderTest(
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
            if (strstr(sRoles[i], "video_encoder.h263") != NULL)
            {
                eError = OMX_CONF_StdH263EncoderTest(cComponentName);
            }
            else if (strstr(sRoles[i], "video_encoder.avc") != NULL)
            {
                eError = OMX_CONF_StdAvcEncoderTest(cComponentName);
            }
            else if (strstr(sRoles[i], "video_encoder.mpeg4") != NULL)
            {
                eError = OMX_CONF_StdMpeg4EncoderTest(cComponentName);
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
