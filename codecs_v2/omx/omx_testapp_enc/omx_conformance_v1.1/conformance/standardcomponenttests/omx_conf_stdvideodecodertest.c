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

/** OMX_CONF_StdVideoDecoderTest.c
 *  OpenMax IL conformance test - Standard Video Decoder Component Test
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "OMX_CONF_StdCompCommon.h"
#include <string.h>

#define TEST_NAME_STRING "StdVideoDecoderTest"

    /**************************** G L O B A L S **********************************/

    /*****************************************************************************/

    OMX_ERRORTYPE StdVideoDecoderTest_WmvDecoder(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_U32 nPortIndex;

        eError = StdComponentTest_SetRole(pCtx, "video_decoder.wmv");
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* For the standard MPEG-4 decoder component, there must be at least two video domain ports. */
        if (pCtx->sPortParamVideo.nPorts < 2) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = VPB + 0; input port, MPEG-4 format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying MPEG-4 input port 0 \n");
        nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 0;
        if (StdComponentTest_IsInputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonVideo_WmvPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = VPB + 1; output port, raw video format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying Raw video output port 1 \n");
        nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 1;
        if (StdComponentTest_IsOutputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonVideo_RawPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdVideoDecoderTest_RvDecoder(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_U32 nPortIndex;

        eError = StdComponentTest_SetRole(pCtx, "video_decoder.rv");
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* For the standard MPEG-4 decoder component, there must be at least two video domain ports. */
        if (pCtx->sPortParamVideo.nPorts < 2) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = VPB + 0; input port, real video format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying Real Video input port 0 \n");
        nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 0;
        if (StdComponentTest_IsInputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonVideo_RvPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = VPB + 1; output port, raw video format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying Raw video output port 1 \n");
        nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 1;
        if (StdComponentTest_IsOutputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonVideo_RawPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdVideoDecoderTest_Mpeg4Decoder(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_U32 nPortIndex;

        eError = StdComponentTest_SetRole(pCtx, "video_decoder.mpeg4");
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* For the standard MPEG-4 decoder component, there must be at least two video domain ports. */
        if (pCtx->sPortParamVideo.nPorts < 2) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = VPB + 0; input port, MPEG-4 format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying MPEG-4 input port 0 \n");
        nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 0;
        if (StdComponentTest_IsInputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonVideo_Mpeg4PortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = VPB + 1; output port, raw video format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying Raw video output port 1 \n");
        nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 1;
        if (StdComponentTest_IsOutputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonVideo_RawPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdVideoDecoderTest_H263Decoder(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_U32 nPortIndex;

        eError = StdComponentTest_SetRole(pCtx, "video_decoder.h263");
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* For the standard H263 decoder component, there must be at least two video domain ports. */
        if (pCtx->sPortParamVideo.nPorts < 2) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = VPB + 0; input port, H263 format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying H.263 input port 0 \n");
        nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 0;
        if (StdComponentTest_IsInputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonVideo_H263PortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = VPB + 1; output port, raw video format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying Raw video output port 1 \n");
        nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 1;
        if (StdComponentTest_IsOutputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonVideo_RawPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdVideoDecoderTest_AvcDecoder(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_U32 nPortIndex;

        eError = StdComponentTest_SetRole(pCtx, "video_decoder.avc");
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* For the standard AVC decoder component, there must be at least two video domain ports. */
        if (pCtx->sPortParamVideo.nPorts < 2) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = VPB + 0; input port, AVC format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying AVC input port 0 \n");
        nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 0;
        if (StdComponentTest_IsInputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonVideo_AvcPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = VPB + 1; output port, raw video format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying Raw video output port 1 \n");
        nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 1;
        if (StdComponentTest_IsOutputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonVideo_RawPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    /* Test a component for compliance with the Standard AVC Decoder. */

    OMX_ERRORTYPE OMX_CONF_StdAvcDecoderTest(
        OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_CTXTYPE ctx;

        eError = StdComponentTest_IsRoleSupported(cComponentName, "video_decoder.avc");
        OMX_CONF_BAIL_ON_ERROR(eError);

        memset(&ctx, 0x0, sizeof(TEST_CTXTYPE));

        eError = StdComponentTest_StdComp(cComponentName, &ctx,
                                          (STDCOMPTEST_COMPONENT)StdVideoDecoderTest_AvcDecoder);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    /* Test a component for compliance with the Standard H263 Decoder. */

    OMX_ERRORTYPE OMX_CONF_StdH263DecoderTest(
        OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_CTXTYPE ctx;

        eError = StdComponentTest_IsRoleSupported(cComponentName, "video_decoder.h263");
        OMX_CONF_BAIL_ON_ERROR(eError);

        memset(&ctx, 0x0, sizeof(TEST_CTXTYPE));

        eError = StdComponentTest_StdComp(cComponentName, &ctx,
                                          (STDCOMPTEST_COMPONENT)StdVideoDecoderTest_H263Decoder);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    /* Test a component for compliance with the Standard MPEG-4 Decoder. */

    OMX_ERRORTYPE OMX_CONF_StdMpeg4DecoderTest(
        OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_CTXTYPE ctx;

        eError = StdComponentTest_IsRoleSupported(cComponentName, "video_decoder.mpeg4");
        OMX_CONF_BAIL_ON_ERROR(eError);

        memset(&ctx, 0x0, sizeof(TEST_CTXTYPE));

        eError = StdComponentTest_StdComp(cComponentName, &ctx,
                                          (STDCOMPTEST_COMPONENT)StdVideoDecoderTest_Mpeg4Decoder);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    /* Test a component for compliance with the Standard Real Video Decoder. */

    OMX_ERRORTYPE OMX_CONF_StdRvDecoderTest(
        OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_CTXTYPE ctx;

        eError = StdComponentTest_IsRoleSupported(cComponentName, "video_decoder.rv");
        OMX_CONF_BAIL_ON_ERROR(eError);

        memset(&ctx, 0x0, sizeof(TEST_CTXTYPE));

        eError = StdComponentTest_StdComp(cComponentName, &ctx,
                                          (STDCOMPTEST_COMPONENT)StdVideoDecoderTest_RvDecoder);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    /* Test a component for compliance with the Standard WMV Decoder. */

    OMX_ERRORTYPE OMX_CONF_StdWmvDecoderTest(
        OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_CTXTYPE ctx;

        eError = StdComponentTest_IsRoleSupported(cComponentName, "video_decoder.wmv");
        OMX_CONF_BAIL_ON_ERROR(eError);

        memset(&ctx, 0x0, sizeof(TEST_CTXTYPE));

        eError = StdComponentTest_StdComp(cComponentName, &ctx,
                                          (STDCOMPTEST_COMPONENT)StdVideoDecoderTest_WmvDecoder);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    /* This runs through and tests all standard components of the video decoder class
       exposed and supported by the component.
    */
    OMX_ERRORTYPE OMX_CONF_StdVideoDecoderTest(
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
            if (strstr(sRoles[i], "video_decoder.h263") != NULL)
            {
                eError = OMX_CONF_StdH263DecoderTest(cComponentName);
            }
            else if (strstr(sRoles[i], "video_decoder.avc") != NULL)
            {
                eError = OMX_CONF_StdAvcDecoderTest(cComponentName);
            }
            else if (strstr(sRoles[i], "video_decoder.mpeg4") != NULL)
            {
                eError = OMX_CONF_StdMpeg4DecoderTest(cComponentName);
            }
            else if (strstr(sRoles[i], "video_decoder.rv") != NULL)
            {
                eError = OMX_CONF_StdRvDecoderTest(cComponentName);
            }
            else if (strstr(sRoles[i], "video_decoder.wmv") != NULL)
            {
                eError = OMX_CONF_StdWmvDecoderTest(cComponentName);
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
