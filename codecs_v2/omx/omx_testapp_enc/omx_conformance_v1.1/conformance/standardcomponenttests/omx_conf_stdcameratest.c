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

/** OMX_CONF_StdCameraTest.c
 *  OpenMax IL conformance test - Standard camera Component Test
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "OMX_CONF_StdCompCommon.h"
#include <string.h>

#define TEST_NAME_STRING "StdCameraTest"

    /**************************** G L O B A L S **********************************/

    /*****************************************************************************/

    OMX_ERRORTYPE StdCameraTest_YuvCamera(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_U32 nPortIndex;
        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;

        OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);

        eError = StdComponentTest_SetRole(pCtx, "camera.yuv");
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* For the standard camera component, there must be at least two video domain ports. */
        if (pCtx->sPortParamVideo.nPorts < 2) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* For the standard camera component, there must be at least one other domain port. */
        if (pCtx->sPortParamOther.nPorts < 1) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = VPB + 0; output port, video */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying Camera video output port 0 \n");
        nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 0;
        if (StdComponentTest_IsOutputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonOther_YuvCameraPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify default values specific to VPB+0 */
        sPortDefinition.nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 0;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
        if ((sPortDefinition.eDomain != OMX_PortDomainVideo) ||
                (sPortDefinition.format.video.eCompressionFormat != OMX_VIDEO_CodingUnused) ||
                (sPortDefinition.format.video.eColorFormat != OMX_COLOR_FormatYUV420Planar) ||
                (sPortDefinition.format.video.nFrameWidth != 320) ||
                (sPortDefinition.format.video.nFrameHeight != 240) ||
                (sPortDefinition.format.video.nStride != 640) ||
                (sPortDefinition.format.video.nSliceHeight != 16))
            eError = OMX_ErrorBadParameter;
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = VPB + 1; output port, video  */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying Camera video output port 1 \n");
        nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 1;
        if (StdComponentTest_IsOutputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonOther_YuvCameraPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify default values specific to VPB+1 */
        sPortDefinition.nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 1;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
        if ((sPortDefinition.eDomain != OMX_PortDomainVideo) ||
                (sPortDefinition.format.video.eCompressionFormat != OMX_VIDEO_CodingUnused) ||
                (sPortDefinition.format.video.eColorFormat != OMX_COLOR_FormatYUV420Planar) ||
                (sPortDefinition.format.video.nFrameWidth != 640) ||
                (sPortDefinition.format.video.nFrameHeight != 480) ||
                (sPortDefinition.format.video.nStride != 1280) ||
                (sPortDefinition.format.video.nSliceHeight != 16))
            eError = OMX_ErrorBadParameter;
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = OPB + 0; input port, other format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying Other input port 0 \n");
        nPortIndex = pCtx->sPortParamOther.nStartPortNumber + 0;
        if (StdComponentTest_IsInputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonOther_OtherPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    /* Test a component for compliance with the Standard Camera. */

    OMX_ERRORTYPE OMX_CONF_StdYuvCameraTest(
        OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_CTXTYPE ctx;

        eError = StdComponentTest_IsRoleSupported(cComponentName, "camera.yuv");
        OMX_CONF_BAIL_ON_ERROR(eError);

        memset(&ctx, 0x0, sizeof(TEST_CTXTYPE));

        eError = StdComponentTest_StdComp(cComponentName, &ctx,
                                          (STDCOMPTEST_COMPONENT)StdCameraTest_YuvCamera);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    /* This runs through and tests all standard components of the camera class
       exposed and supported by the component.
    */
    OMX_ERRORTYPE OMX_CONF_StdCameraTest(
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
            if (strstr(sRoles[i], "camera.yuv") != NULL)
            {
                eError = OMX_CONF_StdYuvCameraTest(cComponentName);
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
