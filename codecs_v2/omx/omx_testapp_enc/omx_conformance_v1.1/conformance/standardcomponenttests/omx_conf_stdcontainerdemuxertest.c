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

/** OMX_CONF_StdContainerDemuxerTest.c
 *  OpenMax IL conformance test - Standard Container Demuxer Class Component Test
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "OMX_CONF_StdCompCommon.h"
#include <string.h>

#define TEST_NAME_STRING "StdContainerDemuxerTest"


    /**************************** G L O B A L S **********************************/

    /*****************************************************************************/

    OMX_ERRORTYPE StdContainerDemuxerTest_3GpDemuxer(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_U32 nPortIndex;
        OMX_PARAM_U32TYPE sTestParam;

        OMX_CONF_INIT_STRUCT(sTestParam, OMX_PARAM_U32TYPE);

        eError = StdComponentTest_SetRole(pCtx, "container_demuxer.3gp");
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* For the standard 3GP Container Demuxer component, there must be at least
           one port each of the audio, video and other domains. */
        if (pCtx->sPortParamAudio.nPorts < 1) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        if (pCtx->sPortParamVideo.nPorts < 1) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        if (pCtx->sPortParamOther.nPorts < 1) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = APB + 0; Output port, format? */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying PCM output port 0 \n");
        nPortIndex = pCtx->sPortParamAudio.nStartPortNumber + 0;
        if (StdComponentTest_IsOutputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonOther_ContainerDemuxerPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify addition APB+0 settings
           No defaults mentioned in the specification. Verify access.
        */
        sTestParam.nPortIndex = pCtx->sPortParamAudio.nStartPortNumber + 0;
        OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamActiveStream,
                                      (OMX_PTR)&sTestParam, eError);
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = OMX_GetParameter(pCtx, OMX_IndexParamNumAvailableStreams,
                                  (OMX_PTR) & sTestParam);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = VPB + 0; output port, format? */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying Raw video output port 0 \n");
        nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 0;
        if (StdComponentTest_IsOutputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonOther_ContainerDemuxerPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify addition VPB+0 settings
           No defaults mentioned in the specification. Verify access.
        */
        sTestParam.nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 0;
        OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamActiveStream,
                                      (OMX_PTR)&sTestParam, eError);
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = OMX_GetParameter(pCtx, OMX_IndexParamNumAvailableStreams,
                                  (OMX_PTR) & sTestParam);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = OPB + 0; input port, OTHER format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying other (media time) input port 0 \n");
        nPortIndex = pCtx->sPortParamOther.nStartPortNumber + 0;
        if (StdComponentTest_IsInputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonOther_OtherPortParameters(pCtx, nPortIndex);
        eError = StdCompCommonOther_ContainerDemuxerPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdContainerDemuxerTest_AsfDemuxer(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_U32 nPortIndex;

        eError = StdComponentTest_SetRole(pCtx, "container_demuxer.asf");
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* For the standard ASF Container Demuxer component, there must be at least
           one port each of the audio, video and other domains. */
        if (pCtx->sPortParamAudio.nPorts < 1) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        if (pCtx->sPortParamVideo.nPorts < 1) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        if (pCtx->sPortParamOther.nPorts < 1) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = APB + 0; Output port, PCM format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying PCM output port 0 \n");
        nPortIndex = pCtx->sPortParamAudio.nStartPortNumber + 0;
        if (StdComponentTest_IsOutputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonAudio_PcmPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* TO DO: Verify additional support for some additional configs. */
        /* OMX_IndexConfigAudioVolume, OMX_IndexConfigAudioMute */
        /* Also need to verify the default value for bInterleaved (OMX_IndexParamAudioPcm). */

        /* Verify - Port Index = VPB + 0; output port, raw format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying Raw video output port 0 \n");
        nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 0;
        if (StdComponentTest_IsOutputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonVideo_RawPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = OPB + 0; input port, OTHER format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying other (media time) input port 0 \n");
        nPortIndex = pCtx->sPortParamOther.nStartPortNumber + 0;
        if (StdComponentTest_IsInputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonOther_OtherPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdContainerDemuxerTest_RealDemuxer(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_U32 nPortIndex;

        eError = StdComponentTest_SetRole(pCtx, "container_demuxer.real");
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* For the standard Real Container Demuxer component, there must be at least
           one port each of the audio, video and other domains. */
        if (pCtx->sPortParamAudio.nPorts < 1) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        if (pCtx->sPortParamVideo.nPorts < 1) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        if (pCtx->sPortParamOther.nPorts < 1) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = APB + 0; Output port, PCM format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying PCM output port 0 \n");
        nPortIndex = pCtx->sPortParamAudio.nStartPortNumber + 0;
        if (StdComponentTest_IsOutputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonAudio_PcmPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* TO DO: Verify additional support for some additional configs. */
        /* OMX_IndexConfigAudioVolume, OMX_IndexConfigAudioMute */
        /* Also need to verify the default value for bInterleaved (OMX_IndexParamAudioPcm). */

        /* Verify - Port Index = VPB + 0; output port, raw format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying Raw video output port 0 \n");
        nPortIndex = pCtx->sPortParamVideo.nStartPortNumber + 0;
        if (StdComponentTest_IsOutputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonVideo_RawPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify - Port Index = OPB + 0; input port, OTHER format */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying other (media time) input port 0 \n");
        nPortIndex = pCtx->sPortParamOther.nStartPortNumber + 0;
        if (StdComponentTest_IsInputPort(pCtx, nPortIndex) == OMX_FALSE) eError = OMX_ErrorUndefined;
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = StdCompCommonOther_OtherPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    /* Test a component for compliance with the Standard 3GP Container Demuxer. */

    OMX_ERRORTYPE OMX_CONF_Std3GpContainerDemuxerTest(
        OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_CTXTYPE ctx;

        eError = StdComponentTest_IsRoleSupported(cComponentName, "container_demuxer.3gp");
        OMX_CONF_BAIL_ON_ERROR(eError);

        memset(&ctx, 0x0, sizeof(TEST_CTXTYPE));

        eError = StdComponentTest_StdComp(cComponentName, &ctx,
                                          (STDCOMPTEST_COMPONENT)StdContainerDemuxerTest_3GpDemuxer);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    /* Test a component for compliance with the Standard ASF Container Demuxer. */

    OMX_ERRORTYPE OMX_CONF_StdAsfContainerDemuxerTest(
        OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_CTXTYPE ctx;

        eError = StdComponentTest_IsRoleSupported(cComponentName, "container_demuxer.asf");
        OMX_CONF_BAIL_ON_ERROR(eError);

        memset(&ctx, 0x0, sizeof(TEST_CTXTYPE));

        eError = StdComponentTest_StdComp(cComponentName, &ctx,
                                          (STDCOMPTEST_COMPONENT)StdContainerDemuxerTest_AsfDemuxer);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    /* Test a component for compliance with the Standard Real Container Demuxer. */

    OMX_ERRORTYPE OMX_CONF_StdRealContainerDemuxerTest(
        OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_CTXTYPE ctx;

        eError = StdComponentTest_IsRoleSupported(cComponentName, "container_demuxer.real");
        OMX_CONF_BAIL_ON_ERROR(eError);

        memset(&ctx, 0x0, sizeof(TEST_CTXTYPE));

        eError = StdComponentTest_StdComp(cComponentName, &ctx,
                                          (STDCOMPTEST_COMPONENT)StdContainerDemuxerTest_RealDemuxer);

OMX_CONF_TEST_BAIL:

        return (eError);
    }

    /*****************************************************************************/

    /* This runs through and tests all standard components of the Container Demuxer class
       exposed and supported by the component.
    */
    OMX_ERRORTYPE OMX_CONF_StdContainerDemuxerTest(
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
            if (strstr(sRoles[i], "container_demuxer.3gp") != NULL)
            {
                eError = OMX_CONF_Std3GpContainerDemuxerTest(cComponentName);
            }
            else if (strstr(sRoles[i], "container_demuxer.asf") != NULL)
            {
                eError = OMX_CONF_StdAsfContainerDemuxerTest(cComponentName);
            }
            else if (strstr(sRoles[i], "container_demuxer.real") != NULL)
            {
                eError = OMX_CONF_StdRealContainerDemuxerTest(cComponentName);
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
