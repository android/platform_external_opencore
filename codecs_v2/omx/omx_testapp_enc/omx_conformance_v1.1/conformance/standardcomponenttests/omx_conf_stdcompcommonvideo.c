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

/** OMX_CONF_StdCompCommonVideo.c
 *  OpenMax IL conformance test - Standard Component Test
 *  Contains common code that can be reused by various standard video component tests.
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "OMX_CONF_StdCompCommon.h"
#include <string.h>

    /**************************** G L O B A L S **********************************/

    /*****************************************************************************/

    static OMX_ERRORTYPE StdCompCommonVideo_PortFormatSupported(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex,
        OMX_VIDEO_CODINGTYPE eCompressionFormat,
        OMX_COLOR_FORMATTYPE eColorFormat)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_VIDEO_PARAM_PORTFORMATTYPE sPortFormat;

        OMX_CONF_INIT_STRUCT(sPortFormat, OMX_VIDEO_PARAM_PORTFORMATTYPE);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying port format support\n");

        sPortFormat.nPortIndex = nPortIndex;

        for (sPortFormat.nIndex = 0; ; sPortFormat.nIndex++)
        {
            eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamVideoPortFormat, (OMX_PTR) & sPortFormat);
            if (OMX_ErrorNoMore == eError)
                eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
            OMX_CONF_BAIL_ON_ERROR(eError);

            if ((sPortFormat.eCompressionFormat == eCompressionFormat) &&
                    (sPortFormat.eColorFormat == eColorFormat))
                break;
        }

OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/

    static OMX_ERRORTYPE StdCompCommonVideo_CommonPortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex,
        OMX_DIRTYPE eDir)
    {
        OMX_U32 i;
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_VIDEO_PARAM_PROFILELEVELTYPE sProfileLevelRead;
        OMX_VIDEO_PARAM_PROFILELEVELTYPE sProfileLevelWrite;
        OMX_VIDEO_PARAM_BITRATETYPE sBitRate;
        OMX_CONFIG_FRAMERATETYPE sFrameRate;
        OMX_VIDEO_CONFIG_BITRATETYPE sConfigBitRate;

        OMX_CONF_INIT_STRUCT(sProfileLevelRead, OMX_VIDEO_PARAM_PROFILELEVELTYPE);
        OMX_CONF_INIT_STRUCT(sProfileLevelWrite, OMX_VIDEO_PARAM_PROFILELEVELTYPE);
        OMX_CONF_INIT_STRUCT(sBitRate, OMX_VIDEO_PARAM_BITRATETYPE);
        OMX_CONF_INIT_STRUCT(sFrameRate, OMX_CONFIG_FRAMERATETYPE);
        OMX_CONF_INIT_STRUCT(sConfigBitRate, OMX_VIDEO_CONFIG_BITRATETYPE);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying port %i common default parameters\n", nPortIndex);

        /* Verify default settings and support for OMX_IndexParamVideoProfileLevelQuerySupported and OMX_IndexParamVideoProfileLevelCurrent */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support and default setting for OMX_IndexParamVideoProfileLevelQuerySupported and OMX_IndexParamVideoProfileLevelCurrent\n");
        sProfileLevelRead.nPortIndex = nPortIndex;
        sProfileLevelWrite.nPortIndex = nPortIndex;

        for (i = 0; ; i++)
        {
            sProfileLevelWrite.nProfileIndex = i;
            eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamVideoProfileLevelQuerySupported, (OMX_PTR) & sProfileLevelWrite);
            if (eError == OMX_ErrorNoMore)
            {
                eError = OMX_ErrorNone;
                break;
            }
            OMX_CONF_BAIL_ON_ERROR(eError);

            eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamVideoProfileLevelCurrent, (OMX_PTR) & sProfileLevelWrite);
            OMX_CONF_BAIL_ON_ERROR(eError);

            eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamVideoProfileLevelCurrent, (OMX_PTR) & sProfileLevelRead);
            OMX_CONF_BAIL_ON_ERROR(eError);

            if ((sProfileLevelRead.eProfile != sProfileLevelWrite.eProfile) ||
                    (sProfileLevelRead.eLevel != sProfileLevelWrite.eLevel))
            {
                eError = OMX_ErrorBadParameter;
                OMX_CONF_BAIL_ON_ERROR(eError);
            }
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eProfile = %d, eLevel = %d\n",
                           sProfileLevelRead.eProfile, sProfileLevelRead.eLevel);
        }

        /* Parameter verification for encoders */
        if (eDir == OMX_DirOutput)
        {
            /* Verify default settings and support for OMX_IndexParamVideoBitrate */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support and default settings for OMX_IndexParamVideoBitrate\n");
            sBitRate.nPortIndex = nPortIndex;
            eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamVideoBitrate, (OMX_PTR) & sBitRate);
            if ((sBitRate.eControlRate != OMX_Video_ControlRateConstant) ||
                    (sBitRate.nTargetBitrate != 64000))
                eError = OMX_ErrorBadParameter;
            OMX_CONF_BAIL_ON_ERROR(eError);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eControlRate = OMX_Video_ControlRateConstant\n");
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nTargetBitrate = 64000\n");

            /* Verify default setting for OMX_IndexConfigVideoFramerate */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support and default settings for OMX_IndexConfigVideoFramerate\n");
            sFrameRate.nPortIndex = nPortIndex;
            eError = OMX_GetConfig(pCtx->hWrappedComp, OMX_IndexConfigVideoFramerate, (OMX_PTR) & sFrameRate);
            if (sFrameRate.xEncodeFramerate != (15 << 16))
                eError = OMX_ErrorBadParameter;
            OMX_CONF_BAIL_ON_ERROR(eError);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " xEncodeFramerate = %d (15 << 16)\n", sFrameRate.xEncodeFramerate);

            /* Verify default setting for OMX_IndexConfigVideoBitrate */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support and default settings for OMX_IndexConfigVideoBitrate\n");
            sConfigBitRate.nPortIndex = nPortIndex;
            eError = OMX_GetConfig(pCtx->hWrappedComp, OMX_IndexConfigVideoBitrate, (OMX_PTR) & sConfigBitRate);
            if (sConfigBitRate.nEncodeBitrate != 64000)
                eError = OMX_ErrorBadParameter;
            OMX_CONF_BAIL_ON_ERROR(eError);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " xEncodeBitrate = 64000\n");
        }

OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdCompCommonVideo_H263PortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;
        OMX_VIDEO_PARAM_PORTFORMATTYPE sPortFormat;
        OMX_VIDEO_PARAM_H263TYPE sFormatH263;
        OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE sErrorCorrection;

        OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
        OMX_CONF_INIT_STRUCT(sPortFormat, OMX_VIDEO_PARAM_PORTFORMATTYPE);
        OMX_CONF_INIT_STRUCT(sFormatH263, OMX_VIDEO_PARAM_H263TYPE);
        OMX_CONF_INIT_STRUCT(sErrorCorrection, OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying H.263 port %i Default parameters\n", nPortIndex);

        /* Verify support for the common standard component port parameters. */
        eError = StdComponentTest_StdPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify Port Definition */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support and defaults for OMX_IndexParamPortDefinition\n");
        sPortDefinition.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
        if ((sPortDefinition.eDomain != OMX_PortDomainVideo) ||
                (sPortDefinition.format.video.eCompressionFormat != OMX_VIDEO_CodingH263) ||
                (sPortDefinition.format.video.eColorFormat != OMX_COLOR_FormatUnused) ||
                (sPortDefinition.format.video.nFrameWidth != 176) ||
                (sPortDefinition.format.video.nFrameHeight != 144) ||
                (sPortDefinition.format.video.nBitrate != 64000) ||
                (sPortDefinition.format.video.xFramerate != (15 << 16)))
            eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eDomain = OMX_PortDomainVideo\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eCompressionFormat = OMX_VIDEO_CodingH263\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eColorFormat = OMX_COLOR_FormatUnused\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nFrameWidth = 176\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nFrameHeight = 144  \n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nBitrate = 64000  \n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " xFramerate = %d (15 << 16))\n", sPortDefinition.format.video.xFramerate);

        /* Verify support for OMX_IndexParamVideoPortFormat and verify that the port format is as expected */
        eError = StdCompCommonVideo_PortFormatSupported(pCtx, nPortIndex, OMX_VIDEO_CodingH263, OMX_COLOR_FormatUnused);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eCompressionFormat = OMX_VIDEO_CodingH263\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eColorFormat = OMX_COLOR_FormatUnused\n");

        /* Verify default settings for H263 */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support and defaults for OMX_IndexParamVideoH263\n");
        sFormatH263.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamVideoH263, (OMX_PTR) & sFormatH263);
        if ((sFormatH263.eProfile != OMX_VIDEO_H263ProfileBaseline) ||
                (sFormatH263.eLevel != OMX_VIDEO_H263Level10) ||
                (sFormatH263.bPLUSPTYPEAllowed != OMX_FALSE) ||
                (sFormatH263.bForceRoundingTypeToZero != OMX_TRUE))
            eError = OMX_ErrorBadParameter;
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eProfile = OMX_VIDEO_H263ProfileBaseLine\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eLevel = OMX_VIDEO_H263Level10\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " bPLUSPTYPEAllowed = OMX_FALSE\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " bForceRoundingTypeToZero = OMX_TRUE\n");

        /* Parameter verification for encoders */
        if (sPortDefinition.eDir == OMX_DirOutput)
        {
            /* Verify settings for OMX_IndexParamVideoH263 */
            if ((sFormatH263.nGOBHeaderInterval < 1) ||
                    (sFormatH263.nGOBHeaderInterval > 9))
                eError = OMX_ErrorBadParameter;
            OMX_CONF_BAIL_ON_ERROR(eError);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " 1 < nGOBHeaderInterval < 9\n");

            /* Verify default setting for OMX_IndexParamVideoErrorCorrection */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support and defaults for OMX_IndexParamVideoErrorCorrection \n");
            sErrorCorrection.nPortIndex = nPortIndex;
            eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamVideoErrorCorrection, (OMX_PTR) & sErrorCorrection);
            if ((sErrorCorrection.bEnableHEC != OMX_TRUE) ||
                    (sErrorCorrection.bEnableResync != OMX_TRUE))
                eError = OMX_ErrorBadParameter;
            OMX_CONF_BAIL_ON_ERROR(eError);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " bEnableHEC = OMX_TRUE\n");
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " bEnableResync = OMX_TRUE\n");
        }

        /* Verify default settings for common parameters */
        eError = StdCompCommonVideo_CommonPortParameters(pCtx, nPortIndex, sPortDefinition.eDir);
        OMX_CONF_BAIL_ON_ERROR(eError);

OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdCompCommonVideo_AvcPortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;
        OMX_VIDEO_PARAM_PORTFORMATTYPE sPortFormat;
        OMX_VIDEO_PARAM_AVCTYPE sFormatAvc;

        OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
        OMX_CONF_INIT_STRUCT(sPortFormat, OMX_VIDEO_PARAM_PORTFORMATTYPE);
        OMX_CONF_INIT_STRUCT(sFormatAvc, OMX_VIDEO_PARAM_AVCTYPE);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying AVC port %i Default parameters\n", nPortIndex);

        /* Verify support for the common standard component port parameters. */
        eError = StdComponentTest_StdPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify Port Definition */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support and defaults for OMX_IndexParamPortDefinition\n");
        sPortDefinition.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
        if ((sPortDefinition.eDomain != OMX_PortDomainVideo) ||
                (sPortDefinition.format.video.eCompressionFormat != OMX_VIDEO_CodingAVC) ||
                (sPortDefinition.format.video.eColorFormat != OMX_COLOR_FormatUnused) ||
                (sPortDefinition.format.video.nFrameWidth != 176) ||
                (sPortDefinition.format.video.nFrameHeight != 144) ||
                (sPortDefinition.format.video.nBitrate != 64000) ||
                (sPortDefinition.format.video.xFramerate != (15 << 16)))
            eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eDomain = OMX_PortDomainVideo\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eCompressionFormat = OMX_VIDEO_CodingAVC\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eColorFormat = OMX_COLOR_FormatUnused\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nFrameWidth = 176\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nFrameHeight = 144  \n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nBitrate = 64000  \n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " xFramerate = %d (15 << 16))\n", sPortDefinition.format.video.xFramerate);

        /* Verify support for OMX_IndexParamVideoPortFormat and verify that the port format is as expected */
        eError = StdCompCommonVideo_PortFormatSupported(pCtx, nPortIndex, OMX_VIDEO_CodingAVC, OMX_COLOR_FormatUnused);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eCompressionFormat = OMX_VIDEO_CodingAVC\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eColorFormat = OMX_COLOR_FormatUnused\n");

        /* Verify default settings for AVC */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support and defaults for OMX_IndexParamVideoAVC\n");
        sFormatAvc.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamVideoAvc, (OMX_PTR) & sFormatAvc);
        if ((sFormatAvc.eProfile != OMX_VIDEO_AVCProfileBaseline) ||
                (sFormatAvc.eLevel != OMX_VIDEO_AVCLevel1))
            eError = OMX_ErrorBadParameter;
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eProfile = OMX_VIDEO_AVCProfileBaseLine\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eLevel = OMX_VIDEO_AVCLevel1\n");

        /* Parameter verification for encoders */
        if (sPortDefinition.eDir == OMX_DirOutput)
        {
            /* Verify settings for OMX_IndexParamVideoAvc */
            if ((sFormatAvc.bUseHadamard != OMX_TRUE) ||
                    (sFormatAvc.nRefFrames != 1) ||
                    (sFormatAvc.bEnableFMO != OMX_FALSE) ||
                    (sFormatAvc.bEnableASO != OMX_FALSE) ||
                    (sFormatAvc.bWeightedPPrediction != OMX_FALSE) ||
                    (sFormatAvc.bconstIpred != OMX_FALSE))
                eError = OMX_ErrorBadParameter;
            OMX_CONF_BAIL_ON_ERROR(eError);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " bUseHadamard = OMX_TRUE\n");
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nRefFrames = 1\n");
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " bEnableFMO = OMX_FALSE\n");
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " bEnableASO = OMX_FALSE\n");
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " bWeightedPPrediction = OMX_FALSE\n");
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " bconstIpred = OMX_FALSE\n");
        }

        /* Verify default settings for common parameters */
        eError = StdCompCommonVideo_CommonPortParameters(pCtx, nPortIndex, sPortDefinition.eDir);
        OMX_CONF_BAIL_ON_ERROR(eError);

OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdCompCommonVideo_Mpeg4PortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;
        OMX_VIDEO_PARAM_PORTFORMATTYPE sPortFormat;
        OMX_VIDEO_PARAM_MPEG4TYPE sFormatMpeg4;
        OMX_VIDEO_PARAM_BITRATETYPE sVideoBitRate;     // for encoder

        OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
        OMX_CONF_INIT_STRUCT(sPortFormat, OMX_VIDEO_PARAM_PORTFORMATTYPE);
        OMX_CONF_INIT_STRUCT(sFormatMpeg4, OMX_VIDEO_PARAM_MPEG4TYPE);
        OMX_CONF_INIT_STRUCT(sVideoBitRate, OMX_VIDEO_PARAM_BITRATETYPE);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying MPEG4 port %i Default parameters\n", nPortIndex);

        /* Verify support for the common standard component port parameters. */
        eError = StdComponentTest_StdPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify Port Definition */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support and defaults for OMX_IndexParamPortDefinition\n");
        sPortDefinition.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
        if ((sPortDefinition.eDomain != OMX_PortDomainVideo) ||
                (sPortDefinition.format.video.eCompressionFormat != OMX_VIDEO_CodingMPEG4) ||
                (sPortDefinition.format.video.eColorFormat != OMX_COLOR_FormatUnused) ||
                (sPortDefinition.format.video.nFrameWidth != 176) ||
                (sPortDefinition.format.video.nFrameHeight != 144) ||
                (sPortDefinition.format.video.nBitrate != 64000) ||
                (sPortDefinition.format.video.xFramerate != (15 << 16)))
            eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eDomain = OMX_PortDomainVideo\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eCompressionFormat = OMX_VIDEO_CodingMPEG4\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eColorFormat = OMX_COLOR_FormatUnused\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nFrameWidth = 176\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nFrameHeight = 144  \n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nBitrate = 64000  \n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " xFramerate = %d (15 << 16))\n", sPortDefinition.format.video.xFramerate);

        /* Verify support for OMX_IndexParamVideoPortFormat and verify that the port format is as expected */
        eError = StdCompCommonVideo_PortFormatSupported(pCtx, nPortIndex, OMX_VIDEO_CodingMPEG4, OMX_COLOR_FormatUnused);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eCompressionFormat = OMX_VIDEO_CodingMPEG4\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eColorFormat = OMX_COLOR_FormatUnused\n");

        /* Verify default settings and support for OMX_IndexParamVideoMpeg4 */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support and defaults for OMX_IndexParamVideoMpeg4\n");
        sFormatMpeg4.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamVideoMpeg4, (OMX_PTR) & sFormatMpeg4);
        if ((sFormatMpeg4.eProfile != OMX_VIDEO_MPEG4ProfileSimple) ||
                (sFormatMpeg4.eLevel != OMX_VIDEO_MPEG4Level1))
            eError = OMX_ErrorBadParameter;
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eProfile = OMX_VIDEO_MPEG4ProfileSimple\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eLevel = OMX_VIDEO_MPEG4Level1\n");

        /* Parameter verification for encoders */
        if (sPortDefinition.eDir == OMX_DirOutput)
        {
            /* Verify settings for OMX_IndexParamVideoMpeg4*/
            if ((sFormatMpeg4.bSVH != OMX_FALSE) ||
                    (sFormatMpeg4.nIDCVLCThreshold != 0) ||
                    (sFormatMpeg4.bACPred != OMX_TRUE) ||
                    (sFormatMpeg4.bReversibleVLC != OMX_FALSE) ||
                    (sFormatMpeg4.nHeaderExtension < 0) ||
                    (sFormatMpeg4.nHeaderExtension > 99))
                eError = OMX_ErrorBadParameter;
            OMX_CONF_BAIL_ON_ERROR(eError);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " bSVH = OMX_FALSE\n");
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nIDCVLCThreshold = 0\n");
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " bACPred = OMX_TRUE\n");
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " bReversibleVLC = OMX_FALSE\n");
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nHeaderExtension = %d\n");

            /* Verify support for OMX_IndexParamVideoBitrate and verify the default value. */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support and defaults for OMX_IndexParamVideoBitrate\n");
            sVideoBitRate.nPortIndex = nPortIndex;
            eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamVideoBitrate, (OMX_PTR) & sVideoBitRate);
            if (sVideoBitRate.eControlRate != OMX_Video_ControlRateConstant)
                eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
            OMX_CONF_BAIL_ON_ERROR(eError);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_Video_ControlRateConstant (Default)\n");

            /* Verify this value can be changed.*/
            OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamVideoBitrate,
                                             (OMX_PTR)&sVideoBitRate,
                                             sVideoBitRate.eControlRate,
                                             OMX_Video_ControlRateDisable, eError);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_Video_ControlRateDisable\n");
            OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamVideoBitrate,
                                             (OMX_PTR)&sVideoBitRate,
                                             sVideoBitRate.eControlRate,
                                             OMX_Video_ControlRateVariable, eError);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_Video_ControlRateVariable\n");
            OMX_CONF_PARAM_READ_WRITE_VERIFY(pCtx, OMX_IndexParamVideoBitrate,
                                             (OMX_PTR)&sVideoBitRate,
                                             sVideoBitRate.eControlRate,
                                             OMX_Video_ControlRateConstant, eError);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_Video_ControlRateConstant\n");
        }

        /* Verify default settings for common parameters */
        eError = StdCompCommonVideo_CommonPortParameters(pCtx, nPortIndex, sPortDefinition.eDir);
        OMX_CONF_BAIL_ON_ERROR(eError);

OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdCompCommonVideo_RvPortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;
        OMX_VIDEO_PARAM_PORTFORMATTYPE sPortFormat;
        OMX_VIDEO_PARAM_BITRATETYPE sBitRate;
        OMX_VIDEO_PARAM_PROFILELEVELTYPE sProfileLevelRead;
        OMX_VIDEO_PARAM_PROFILELEVELTYPE sProfileLevelWrite;
        OMX_VIDEO_PARAM_RVTYPE sFormatRv;
        OMX_VIDEO_RVFORMATTYPE eFormat;
        OMX_BOOL bEnablePostFilter;
        OMX_BOOL bEnableLatencyMode;

        OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
        OMX_CONF_INIT_STRUCT(sPortFormat, OMX_VIDEO_PARAM_PORTFORMATTYPE);
        OMX_CONF_INIT_STRUCT(sBitRate, OMX_VIDEO_PARAM_BITRATETYPE);
        OMX_CONF_INIT_STRUCT(sProfileLevelRead, OMX_VIDEO_PARAM_PROFILELEVELTYPE);
        OMX_CONF_INIT_STRUCT(sProfileLevelWrite, OMX_VIDEO_PARAM_PROFILELEVELTYPE);
        OMX_CONF_INIT_STRUCT(sFormatRv, OMX_VIDEO_PARAM_RVTYPE);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying Real Video port %i Default parameters\n", nPortIndex);

        /* Verify support for the common standard component port parameters. */
        eError = StdComponentTest_StdPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify Port Definition */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support and defaults for OMX_IndexParamPortDefinition\n");
        sPortDefinition.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
        if ((sPortDefinition.eDomain != OMX_PortDomainVideo) ||
                (sPortDefinition.format.video.eCompressionFormat != OMX_VIDEO_CodingRV) ||
                (sPortDefinition.format.video.eColorFormat != OMX_COLOR_FormatUnused) ||
                (sPortDefinition.format.video.nFrameWidth != 176) ||
                (sPortDefinition.format.video.nFrameHeight != 144) ||
                (sPortDefinition.format.video.nBitrate != 64000) ||
                (sPortDefinition.format.video.xFramerate != (15 << 16)))
            eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eDomain = OMX_PortDomainVideo\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eCompressionFormat = OMX_VIDEO_CodingRV\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eColorFormat = OMX_COLOR_FormatUnused\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nFrameWidth = 176\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nFrameHeight = 144  \n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nBitrate = 64000  \n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " xFramerate = %d (15 << 16))\n", sPortDefinition.format.video.xFramerate);

        /* Verify support for OMX_IndexParamVideoPortFormat and verify that the port format is as expected */
        eError = StdCompCommonVideo_PortFormatSupported(pCtx, nPortIndex, OMX_VIDEO_CodingRV, OMX_COLOR_FormatUnused);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eCompressionFormat = OMX_VIDEO_CodingRV\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eColorFormat = OMX_COLOR_FormatUnused\n");

        /* Verify default settings and support for OMX_IndexParamVideoRv*/
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support and defaults for OMX_IndexParamVideoRv\n");
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamVideoRv, (OMX_PTR) & sFormatRv);
        if ((sFormatRv.eFormat != OMX_VIDEO_RVFormat9) ||
                (sFormatRv.bEnablePostFilter != OMX_FALSE) ||
                (sFormatRv.bEnableLatencyMode != OMX_FALSE))
            eError = OMX_ErrorBadParameter;
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "eFormat = OMX_VIDEO_RVFormat9\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " bEnablePostFilter = OMX_FALSE\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " bEnableLatencyMode = OMX_FALSE\n");

        /* Verify combination of settings */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying all combinations of eFormat,"
                       " bEnablePostFilter, bEnableLatencyMode\n");
        for (eFormat = OMX_VIDEO_RVFormat8; ; eFormat = OMX_VIDEO_RVFormat9)
        {
            for (bEnablePostFilter = OMX_FALSE; ; bEnablePostFilter = OMX_TRUE)
            {
                for (bEnableLatencyMode = OMX_FALSE; ; bEnableLatencyMode = OMX_TRUE)
                {
                    sFormatRv.eFormat = eFormat;
                    sFormatRv.bEnablePostFilter = bEnablePostFilter;
                    sFormatRv.bEnableLatencyMode = bEnableLatencyMode;
                    eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamVideoRv, (OMX_PTR) & sFormatRv);
                    OMX_CONF_BAIL_ON_ERROR(eError);

                    eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamVideoRv, (OMX_PTR) & sFormatRv);
                    if ((eFormat != sFormatRv.eFormat) ||
                            (bEnablePostFilter != sFormatRv.bEnablePostFilter) ||
                            (bEnableLatencyMode != sFormatRv.bEnableLatencyMode))
                        eError = OMX_ErrorBadParameter;
                    OMX_CONF_BAIL_ON_ERROR(eError);

                    if (OMX_TRUE == bEnableLatencyMode) break;
                }

                if (OMX_TRUE == bEnablePostFilter) break;
            }

            if (OMX_VIDEO_RVFormat9 == eFormat) break;
        }

OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdCompCommonVideo_WmvPortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;
        OMX_VIDEO_PARAM_PORTFORMATTYPE sPortFormat;
        OMX_VIDEO_PARAM_BITRATETYPE sBitRate;
        OMX_VIDEO_PARAM_PROFILELEVELTYPE sProfileLevelRead;
        OMX_VIDEO_PARAM_PROFILELEVELTYPE sProfileLevelWrite;
        OMX_VIDEO_PARAM_WMVTYPE sFormatWmv;

        OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
        OMX_CONF_INIT_STRUCT(sPortFormat, OMX_VIDEO_PARAM_PORTFORMATTYPE);
        OMX_CONF_INIT_STRUCT(sBitRate, OMX_VIDEO_PARAM_BITRATETYPE);
        OMX_CONF_INIT_STRUCT(sProfileLevelRead, OMX_VIDEO_PARAM_PROFILELEVELTYPE);
        OMX_CONF_INIT_STRUCT(sProfileLevelWrite, OMX_VIDEO_PARAM_PROFILELEVELTYPE);
        OMX_CONF_INIT_STRUCT(sFormatWmv, OMX_VIDEO_PARAM_WMVTYPE);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying WMV Video port %i Default parameters\n", nPortIndex);

        /* Verify support for the common standard component port parameters. */
        eError = StdComponentTest_StdPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify Port Definition */
        sPortDefinition.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
        if ((sPortDefinition.eDomain != OMX_PortDomainVideo) ||
                (sPortDefinition.format.video.eCompressionFormat != OMX_VIDEO_CodingWMV) ||
                (sPortDefinition.format.video.eColorFormat != OMX_COLOR_FormatUnused) ||
                (sPortDefinition.format.video.nFrameWidth != 176) ||
                (sPortDefinition.format.video.nFrameHeight != 144) ||
                (sPortDefinition.format.video.nBitrate != 64000) ||
                (sPortDefinition.format.video.xFramerate != (15 << 16)))
            eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eDomain = OMX_PortDomainVideo\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eCompressionFormat = OMX_VIDEO_CodingWMV\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eColorFormat = OMX_COLOR_FormatUnused\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nFrameWidth = 176\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nFrameHeight = 144  \n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nBitrate = 64000  \n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " xFramerate = %d (15 << 16))\n", sPortDefinition.format.video.xFramerate);

        /* Verify support for OMX_IndexParamVideoPortFormat and verify that the port format is as expected */
        eError = StdCompCommonVideo_PortFormatSupported(pCtx, nPortIndex, OMX_VIDEO_CodingWMV, OMX_COLOR_FormatUnused);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eCompressionFormat = OMX_VIDEO_CodingWMV\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eColorFormat = OMX_COLOR_FormatUnused\n");

        /* Verify default settings and support for OMX_IndexParamVideoWmv*/
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support and defaults for OMX_IndexParamVideoWmv\n");
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamVideoWmv, (OMX_PTR) & sFormatWmv);
        if (sFormatWmv.eFormat != OMX_VIDEO_WMVFormat9)
            eError = OMX_ErrorBadParameter;
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_VIDEO_WMVFormat9 (Default)\n");

        OMX_CONF_PARAM_WRITE_VERIFY(pCtx, OMX_IndexParamVideoWmv,
                                    (OMX_PTR)&sFormatWmv, sFormatWmv.eFormat, OMX_VIDEO_WMVFormat7, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_VIDEO_WMVFormat7 (Default)\n");

        OMX_CONF_PARAM_WRITE_VERIFY(pCtx, OMX_IndexParamVideoWmv,
                                    (OMX_PTR)&sFormatWmv, sFormatWmv.eFormat, OMX_VIDEO_WMVFormat8, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_VIDEO_WMVFormat8 (Default)\n");

OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdCompCommonVideo_RawPortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {

        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;
        OMX_VIDEO_PARAM_PORTFORMATTYPE sPortFormat;

        OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
        OMX_CONF_INIT_STRUCT(sPortFormat, OMX_VIDEO_PARAM_PORTFORMATTYPE);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying Raw Video port %i Default parameters\n", nPortIndex);

        /* Verify support for the common standard component port parameters. */
        eError = StdComponentTest_StdPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify Port Definition */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support and defaults for OMX_IndexParamPortDefinition\n");
        sPortDefinition.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
        if ((sPortDefinition.eDomain != OMX_PortDomainVideo) ||
                (sPortDefinition.format.video.eCompressionFormat != OMX_VIDEO_CodingUnused) ||
                (sPortDefinition.format.video.eColorFormat != OMX_COLOR_FormatYUV420Planar) ||
                (sPortDefinition.format.video.nFrameWidth != 176) ||
                (sPortDefinition.format.video.nFrameHeight != 144) ||
                (sPortDefinition.format.video.nBitrate != 64000) ||
                (sPortDefinition.format.video.xFramerate != (15 << 16)))
            eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eDomain = OMX_PortDomainVideo\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eCompression = OMX_VIDEO_CodingUnused\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eColorFormat = OMX_COLOR_FormatYUV420Planar\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nFrameWidth = 176\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nFrameHeight = 144  \n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nBitrate = 64000  \n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " xFramerate = %d (15 << 16))\n", sPortDefinition.format.video.xFramerate);

        /* Verify support for OMX_IndexParamVideoPortFormat and verify that the port format is as expected */
        eError = StdCompCommonVideo_PortFormatSupported(pCtx, nPortIndex, OMX_VIDEO_CodingUnused, OMX_COLOR_FormatYUV420Planar);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eCompressionFormat = OMX_VIDEO_CodingUnused\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eColorFormat = OMX_COLOR_FormatYUV420Planar\n");

OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdCompCommonVideo_IVRendererPortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {
        OMX_U32 i;
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_CONFIG_ROTATIONTYPE sRotation;
        OMX_CONFIG_MIRRORTYPE sMirror;
        OMX_CONFIG_SCALEFACTORTYPE sScale;
        OMX_CONFIG_RECTTYPE sRect;
        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;

        OMX_CONF_INIT_STRUCT(sRotation, OMX_CONFIG_ROTATIONTYPE);
        OMX_CONF_INIT_STRUCT(sMirror, OMX_CONFIG_MIRRORTYPE);
        OMX_CONF_INIT_STRUCT(sScale, OMX_CONFIG_SCALEFACTORTYPE);
        OMX_CONF_INIT_STRUCT(sRect, OMX_CONFIG_RECTTYPE);
        OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying common video renderer input port parameters\n");

        /* Verify support for OMX_IndexConfigCommonRotate */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying default setting for OMX_IndexConfigCommonRotate\n");
        sRotation.nPortIndex = nPortIndex;
        eError = OMX_GetConfig(pCtx->hWrappedComp, OMX_IndexConfigCommonRotate, (OMX_PTR) & sRotation);
        if (sRotation.nRotation != 0)
            eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "nRotation = %d\n", sRotation.nRotation);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for other values of OMX_IndexConfigCommonRotate\n");
        OMX_CONF_CONFIG_WRITE_VERIFY(pCtx, OMX_IndexConfigCommonRotate,
                                     (OMX_PTR)&sRotation, sRotation.nRotation, 90, eError);

        OMX_CONF_CONFIG_WRITE_VERIFY(pCtx, OMX_IndexConfigCommonRotate,
                                     (OMX_PTR)&sRotation, sRotation.nRotation, 180, eError);

        OMX_CONF_CONFIG_WRITE_VERIFY(pCtx, OMX_IndexConfigCommonRotate,
                                     (OMX_PTR)&sRotation, sRotation.nRotation, 270, eError);

        /* Verify support for OMX_IndexConfigCommonMirror */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying default setting for OMX_IndexConfigCommonMirror\n");
        sMirror.nPortIndex = nPortIndex;
        eError = OMX_GetConfig(pCtx->hWrappedComp, OMX_IndexConfigCommonMirror, (OMX_PTR) & sMirror);
        if (sMirror.eMirror != OMX_MirrorNone)
            eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " %d (OMX_MirrorNone)\n", sMirror.eMirror);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support for other values of OMX_IndexConfigCommonMirror\n");
        OMX_CONF_CONFIG_WRITE_VERIFY(pCtx, OMX_IndexConfigCommonMirror,
                                     (OMX_PTR)&sMirror, sMirror.eMirror, OMX_MirrorVertical, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " %d (OMX_MirrorVertical)\n", sMirror.eMirror);

        OMX_CONF_CONFIG_WRITE_VERIFY(pCtx, OMX_IndexConfigCommonMirror,
                                     (OMX_PTR)&sMirror, sMirror.eMirror, OMX_MirrorHorizontal, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " %d (OMX_MirrorHorizontal)\n", sMirror.eMirror);

        OMX_CONF_CONFIG_WRITE_VERIFY(pCtx, OMX_IndexConfigCommonMirror,
                                     (OMX_PTR)&sMirror, sMirror.eMirror, OMX_MirrorBoth, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " %d (OMX_MirrorBoth)\n", sMirror.eMirror);

        /* Verify support for OMX_IndexConfigCommonScale */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying settings for OMX_IndexConfigCommonScale\n");
        sScale.nPortIndex = nPortIndex;

        OMX_CONF_CONFIG_READ_WRITE_VERIFY(pCtx, OMX_IndexConfigCommonScale,
                                          (OMX_PTR)&sScale, sScale.xWidth, (1 << 15), 0, eError);

        OMX_CONF_CONFIG_WRITE_VERIFY(pCtx, OMX_IndexConfigCommonScale,
                                     (OMX_PTR)&sScale, sScale.xHeight, (1 << 15), eError);

        /* Get frame width and height */
        sPortDefinition.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);

        /* Verify support for OMX_IndexConfigCommonInputCrop */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying default settings for OMX_IndexConfigCommonInputCrop\n");
        sRect.nPortIndex = nPortIndex;
        eError = OMX_GetConfig(pCtx->hWrappedComp, OMX_IndexConfigCommonInputCrop, (OMX_PTR) & sRect);
        if ((sRect.nLeft != 0) ||
                (sRect.nTop != 0) ||
                (sRect.nWidth != sPortDefinition.format.video.nFrameWidth) ||
                (sRect.nHeight != sPortDefinition.format.video.nFrameHeight))
            eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
        OMX_CONF_BAIL_ON_ERROR(eError);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying other settings for OMX_IndexConfigCommonInputCrop\n");
        OMX_CONF_CONFIG_WRITE_VERIFY(pCtx, OMX_IndexConfigCommonInputCrop,
                                     (OMX_PTR)&sRect, sRect.nLeft, 1, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nLeft = %d\n", sRect.nLeft);

        OMX_CONF_CONFIG_WRITE_VERIFY(pCtx, OMX_IndexConfigCommonInputCrop,
                                     (OMX_PTR)&sRect, sRect.nTop, 1, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nTop = %d\n", sRect.nTop);

        i = sRect.nWidth >> 1;
        OMX_CONF_CONFIG_WRITE_VERIFY(pCtx, OMX_IndexConfigCommonInputCrop,
                                     (OMX_PTR)&sRect, sRect.nWidth, i, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nWidth/2 = %d\n", sRect.nWidth);

        i = sRect.nHeight >> 1;
        OMX_CONF_CONFIG_WRITE_VERIFY(pCtx, OMX_IndexConfigCommonInputCrop,
                                     (OMX_PTR)&sRect, sRect.nHeight, i, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nHeight/2 = %d\n", sRect.nHeight);

OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdCompCommonVideo_QcifPlusPortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex,
        OMX_COLOR_FORMATTYPE eColorFormat)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;
        OMX_VIDEO_PARAM_PORTFORMATTYPE sPortFormat;

        OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
        OMX_CONF_INIT_STRUCT(sPortFormat, OMX_VIDEO_PARAM_PORTFORMATTYPE);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying QCIF+ video port %i default parameters\n", nPortIndex);

        /* Verify support for the common standard component port parameters */
        eError = StdComponentTest_StdPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify default port definition settings */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support and default settings for QcifPlus port definition\n");
        sPortDefinition.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
        if ((sPortDefinition.eDomain != OMX_PortDomainVideo) ||
                (sPortDefinition.format.video.nFrameWidth != 176) ||
                (sPortDefinition.format.video.nFrameHeight != 220) ||
                (sPortDefinition.format.video.nStride != ((OMX_COLOR_FormatYUV420Planar == eColorFormat) ? 176 : 352)) ||
                (sPortDefinition.format.video.nSliceHeight != 16) ||
                (sPortDefinition.format.video.eCompressionFormat != OMX_VIDEO_CodingUnused) ||
                (sPortDefinition.format.video.eColorFormat != eColorFormat))
            eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eDomain = %d (OMX_PortDomainVideo)\n", sPortDefinition.eDomain);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nFrameWidth = %d, nFrameHeight = %d\n",
                       sPortDefinition.format.video.nFrameWidth, sPortDefinition.format.video.nFrameHeight);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nStride = %d, nSliceHeight = %d\n",
                       sPortDefinition.format.video.nStride, sPortDefinition.format.video.nSliceHeight);

        /* Verify support for OMX_IndexParamVideoPortFormat and verify that the port format is as expected */
        eError = StdCompCommonVideo_PortFormatSupported(pCtx, nPortIndex, OMX_VIDEO_CodingUnused, eColorFormat);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eCompressionFormat = OMX_VIDEO_CodingUnused\n");
        if (OMX_COLOR_FormatYUV420Planar == eColorFormat)
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eColorFormat = OMX_COLOR_FormatYUV420Planar\n");
        else if (OMX_COLOR_Format16bitRGB565 == eColorFormat)
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eColorFormat = OMX_COLOR_Format16bitRGB565\n");

        eError = StdCompCommonVideo_IVRendererPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/


#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */
