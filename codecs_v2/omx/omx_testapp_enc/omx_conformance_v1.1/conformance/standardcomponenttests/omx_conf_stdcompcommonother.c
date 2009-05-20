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

/** OMX_CONF_StdCompCommonOther.c
 *  OpenMax IL conformance test - Standard Component Test
 *  Contains common code that can be reused by various standard component tests
 *  requiring ports of the OTHER domain.
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "OMX_CONF_StdCompCommon.h"

#include <string.h>

    /**************************** G L O B A L S **********************************/

    /*****************************************************************************/

    OMX_ERRORTYPE StdCompCommonOther_OtherPortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {

        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;
        OMX_OTHER_PARAM_PORTFORMATTYPE sPortFormat;

        OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
        OMX_CONF_INIT_STRUCT(sPortFormat, OMX_OTHER_PARAM_PORTFORMATTYPE);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying OTHER port %i Default parameters\n", nPortIndex);

        /* Verify support for the common standard component port parameters. */
        eError = StdComponentTest_StdPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify Port Definition */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support and default settings for OMX_IndexParamPortDefinition\n");
        sPortDefinition.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
        if ((sPortDefinition.eDomain != OMX_PortDomainOther) ||
                (sPortDefinition.format.other.eFormat != OMX_OTHER_FormatTime))
            eError = OMX_ErrorBadParameter;
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eDomain = %d (OMX_PortDomainOther)\n",
                       sPortDefinition.eDomain);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eFormat = %d (OMX_OTHER_FormatTime)\n",
                       sPortDefinition.format.other.eFormat);

        /* Verify support for OMX_IndexParamOtherPortFormat and verify
           that the port format is as expected.
        */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support and default settings for OMX_IndexParamOtherPortFormat\n");
        sPortFormat.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamOtherPortFormat, (OMX_PTR) & sPortFormat);
        if (sPortFormat.eFormat != OMX_OTHER_FormatTime)
            eError = OMX_ErrorBadParameter;
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eFormat = %d (OMX_OTHER_FormatTime)\n",
                       sPortFormat.eFormat);


OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdCompCommonOther_ContainerDemuxerPortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {

        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_TIME_CONFIG_TIMESTAMPTYPE sTimePosition;
        OMX_TIME_CONFIG_SEEKMODETYPE sTimeSeekMode;

        OMX_CONF_INIT_STRUCT(sTimePosition, OMX_TIME_CONFIG_TIMESTAMPTYPE);
        OMX_CONF_INIT_STRUCT(sTimeSeekMode, OMX_TIME_CONFIG_SEEKMODETYPE);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying Container Demuxer port %i common parameters\n", nPortIndex);

        /* Verify support for the common standard component port parameters. */
        eError = StdComponentTest_StdPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* There are no default values mentioned in the specification.
           Just verify the access at present.
        */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for OMX_IndexConfigTimePosition\n");
        OMX_CONF_CONFIG_REQUIREDGETSET(pCtx, OMX_IndexConfigTimePosition,
                                       (OMX_PTR)&sTimePosition,
                                       eError);                      // R/W

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for OMX_IndexConfigTimeSeekMode \n");
        OMX_CONF_CONFIG_REQUIREDGETSET(pCtx, OMX_IndexConfigTimeSeekMode,
                                       (OMX_PTR)&sTimeSeekMode,
                                       eError);                      // R/W

OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdCompCommonOther_ContainerMuxerPortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {

        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying Container Muxer port %i common parameters\n", nPortIndex);

        /* Verify support for the common standard component port parameters. */
        eError = StdComponentTest_StdPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdCompCommonOther_YuvCameraPortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;
        OMX_VIDEO_PARAM_PORTFORMATTYPE sPortFormat;

        OMX_PARAM_SENSORMODETYPE sSensorMode;            //OMX_IndexParamCommonSensorMode
        OMX_CONFIG_WHITEBALCONTROLTYPE sWhiteBalance;    //OMX_IndexConfigCommonWhiteBalance
        OMX_CONFIG_SCALEFACTORTYPE sScaleFactor;                //OMX_IndexConfigCommonDigitalZoom
        // OMX_CONFIG_EVCOMPENSATIONTYPE sEvComp;           //OMX_IndexConfigCommonEVCompensation
        OMX_CONFIG_BOOLEANTYPE sConfigCapturing;         // OMX_IndexConfigCapturing
        OMX_CONFIG_BOOLEANTYPE sAutoPauseCapture;             //OMX_IndexConfigAutoPauseAfterCapture


        OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
        OMX_CONF_INIT_STRUCT(sPortFormat, OMX_VIDEO_PARAM_PORTFORMATTYPE);

        OMX_CONF_INIT_STRUCT(sSensorMode, OMX_PARAM_SENSORMODETYPE);
        OMX_CONF_INIT_STRUCT(sWhiteBalance, OMX_CONFIG_WHITEBALCONTROLTYPE);
        OMX_CONF_INIT_STRUCT(sScaleFactor, OMX_CONFIG_SCALEFACTORTYPE);
        //OMX_CONF_INIT_STRUCT(sEvComp, OMX_CONFIG_EVCOMPENSATIONTYPE);
        OMX_CONF_INIT_STRUCT(sConfigCapturing, OMX_CONFIG_BOOLEANTYPE);
        OMX_CONF_INIT_STRUCT(sAutoPauseCapture, OMX_CONFIG_BOOLEANTYPE);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying Camera port %i Default parameters\n", nPortIndex);

        /* There are no default values specified for most of the above.
           Just verify access for now.
        */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for OMX_IndexParamCommonSensorMode\n");
        OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamCommonSensorMode, (OMX_PTR)&sSensorMode, eError);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for OMX_IndexConfigCommonWhiteBalance\n");
        OMX_CONF_CONFIG_REQUIREDGETSET(pCtx, OMX_IndexConfigCommonWhiteBalance, (OMX_PTR)&sWhiteBalance, eError);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for OMX_IndexConfigCommonDigitalZoom\n");
        OMX_CONF_CONFIG_REQUIREDGETSET(pCtx, OMX_IndexConfigCommonDigitalZoom, (OMX_PTR)&sScaleFactor, eError);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for OMX_IndexConfigCapturing\n");
        OMX_CONF_CONFIG_REQUIREDGETSET(pCtx, OMX_IndexConfigCapturing, (OMX_PTR)&sConfigCapturing, eError);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for OMX_IndexAutoPauseAfterCapture\n");
        OMX_CONF_CONFIG_REQUIREDGETSET(pCtx, OMX_IndexAutoPauseAfterCapture, (OMX_PTR)&sAutoPauseCapture, eError);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify support for the common standard component port parameters. */
        eError = StdComponentTest_StdPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify Port Definition */
        sPortDefinition.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);

        /* Verify support for OMX_IndexParamVideoPortFormat and verify
           that the port format is as expected.
        */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support and defaults for OMX_IndexParamVideoPortFormat\n");
        sPortFormat.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamVideoPortFormat, (OMX_PTR) & sPortFormat);
        if ((sPortFormat.eCompressionFormat != OMX_VIDEO_CodingUnused) ||
                (sPortFormat.eColorFormat != OMX_COLOR_FormatYUV420Planar))
            eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eCompressionFormat = %d (OMX_VIDEO_CodingUnused)\n",
                       sPortFormat.eCompressionFormat);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eColorFormat = %d (OMX_COLOR_FormatYUV420Planar)\n",
                       sPortFormat.eColorFormat);


OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/


#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */
