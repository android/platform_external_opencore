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

/** OMX_CONF_StdCompCommonImage.c
 *  OpenMax IL conformance test - Standard Component Test
 *  Contains common code that can be reused by various standard image component tests.
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "OMX_CONF_StdCompCommon.h"
#include <string.h>

    /**************************** G L O B A L S **********************************/

    static OMX_ERRORTYPE StdCompCommonImage_PortFormatSupported(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex,
        OMX_IMAGE_CODINGTYPE eCompressionFormat,
        OMX_COLOR_FORMATTYPE eColorFormat)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_IMAGE_PARAM_PORTFORMATTYPE sPortFormat;

        OMX_CONF_INIT_STRUCT(sPortFormat, OMX_IMAGE_PARAM_PORTFORMATTYPE);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying port format support\n");

        sPortFormat.nPortIndex = nPortIndex;

        for (sPortFormat.nIndex = 0; ; sPortFormat.nIndex++)
        {
            eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamImagePortFormat, (OMX_PTR) & sPortFormat);
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

    OMX_ERRORTYPE StdCompCommonImage_JpegPortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;
        OMX_IMAGE_PARAM_PORTFORMATTYPE sPortFormat;
        OMX_IMAGE_PARAM_QUANTIZATIONTABLETYPE sQuantTable;
        OMX_IMAGE_PARAM_HUFFMANTTABLETYPE sHuffTable;

        OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
        OMX_CONF_INIT_STRUCT(sPortFormat, OMX_IMAGE_PARAM_PORTFORMATTYPE);
        OMX_CONF_INIT_STRUCT(sQuantTable, OMX_IMAGE_PARAM_QUANTIZATIONTABLETYPE);
        OMX_CONF_INIT_STRUCT(sHuffTable, OMX_IMAGE_PARAM_HUFFMANTTABLETYPE);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying JPEG port %i Default parameters\n", nPortIndex);

        /* Verify support for the common standard component port parameters */
        eError = StdComponentTest_StdPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify default port definition settings */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying JPEG port definition\n");
        sPortDefinition.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
        if ((sPortDefinition.eDomain != OMX_PortDomainImage) ||
                (sPortDefinition.format.image.nFrameWidth != 640) ||
                (sPortDefinition.format.image.nFrameHeight != 480) ||
                (sPortDefinition.format.image.eCompressionFormat != OMX_IMAGE_CodingJPEG) ||
                (sPortDefinition.format.image.eColorFormat != OMX_COLOR_FormatUnused))
            eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "eDomain = %d, nFrameWidth = %d, nFrameHeight = %d, "
                       "eCompressionFormat= %d, eColorFormat = %d\n", sPortDefinition.eDomain,
                       sPortDefinition.format.image.nFrameWidth, sPortDefinition.format.image.nFrameHeight,
                       sPortDefinition.format.image.eCompressionFormat, sPortDefinition.format.image.eColorFormat);

        /* Verify default settings for OMX_IndexParamImagePortFormat */
        eError = StdCompCommonImage_PortFormatSupported(pCtx, nPortIndex, OMX_IMAGE_CodingJPEG, OMX_COLOR_FormatUnused);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eCompressionFormat = OMX_IMAGE_CodingJPEG\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eColorFormat = OMX_COLOR_FormatUnused\n");

        /* Verify default settings for OMX_IndexParamQuantizationTable */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying default settings for OMX_IndexParamQuantizationTable\n");
        sQuantTable.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamQuantizationTable, (OMX_PTR) & sQuantTable);
        if (sQuantTable.eQuantizationTable != OMX_IMAGE_QuantizationTableLuma)
            eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_IMAGE_QuantizationTableLuma\n");

        /* Verify setting alternate values for OMX_IndexParamQuantization */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying other settings for OMX_IndexParamQuantizationTable\n");
        OMX_CONF_PARAM_WRITE_VERIFY(pCtx, OMX_IndexParamQuantizationTable,
                                    (OMX_PTR)&sQuantTable, sQuantTable.eQuantizationTable, OMX_IMAGE_QuantizationTableChroma, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_IMAGE_QuantizationTableChroma\n");

        /* Parameter verification for decoders*/
        if (sPortDefinition.eDir == OMX_DirInput)
        {
            /* Verify default settings for OMX_IndexParamHuffmanTable */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying default settings for OMX_IndexParamHuffmanTable\n");
            sHuffTable.nPortIndex = nPortIndex;
            eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamHuffmanTable, (OMX_PTR) & sHuffTable);
            if (sHuffTable.eHuffmanTable != OMX_IMAGE_HuffmanTableAC)
                eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
            OMX_CONF_BAIL_ON_ERROR(eError);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_IMAGE_HuffmanTableAC\n");

            /* Verify setting alternate values for OMX_IndexParamHuffman*/
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying other settings for OMX_IndexParamHuffmanTable\n");
            OMX_CONF_PARAM_WRITE_VERIFY(pCtx, OMX_IndexParamHuffmanTable,
                                        (OMX_PTR)&sHuffTable, sHuffTable.eHuffmanTable, OMX_IMAGE_HuffmanTableDC, eError);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " OMX_IMAGE_HuffmanTableDC\n");
        }

OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdCompCommonImage_RawPortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;
        OMX_IMAGE_PARAM_PORTFORMATTYPE sPortFormat;

        OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
        OMX_CONF_INIT_STRUCT(sPortFormat, OMX_IMAGE_PARAM_PORTFORMATTYPE);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying Raw Image port %i Default parameters\n", nPortIndex);

        /* Verify support for the common standard component port parameters */
        eError = StdComponentTest_StdPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify default port definition settings */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying Raw port definition\n");
        sPortDefinition.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
        if ((sPortDefinition.eDomain != OMX_PortDomainImage) ||
                (sPortDefinition.format.image.nFrameWidth != 640) ||
                (sPortDefinition.format.image.nFrameHeight != 480) ||
                (sPortDefinition.format.image.eCompressionFormat != OMX_IMAGE_CodingUnused) ||
                (sPortDefinition.format.image.eColorFormat != OMX_COLOR_FormatYUV420Planar))
            eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "eDomain = %d, nFrameWidth = %d, nFrameHeight = %d, "
                       "eCompressionFormat= %d, eColorFormat = %d\n", sPortDefinition.eDomain,
                       sPortDefinition.format.image.nFrameWidth, sPortDefinition.format.image.nFrameHeight,
                       sPortDefinition.format.image.eCompressionFormat, sPortDefinition.format.image.eColorFormat);

        /* Verify default settings for OMX_IndexParamImagePortFormat */
        eError = StdCompCommonImage_PortFormatSupported(pCtx, nPortIndex, OMX_IMAGE_CodingUnused, OMX_COLOR_FormatYUV420Planar);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eCompressionFormat = OMX_IMAGE_CodingUnused\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eColorFormat = OMX_COLOR_FormatYUV420Planar\n");

OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdCompCommonImage_IVRendererPortParameters(
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

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying common image renderer input port parameters\n");

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
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " %d\n", sRotation.nRotation);

        OMX_CONF_CONFIG_WRITE_VERIFY(pCtx, OMX_IndexConfigCommonRotate,
                                     (OMX_PTR)&sRotation, sRotation.nRotation, 180, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " %d\n", sRotation.nRotation);

        OMX_CONF_CONFIG_WRITE_VERIFY(pCtx, OMX_IndexConfigCommonRotate,
                                     (OMX_PTR)&sRotation, sRotation.nRotation, 270, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " %d\n", sRotation.nRotation);

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
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " xWidth = ox%x (1 << 15)\n", sScale.xWidth);

        OMX_CONF_CONFIG_WRITE_VERIFY(pCtx, OMX_IndexConfigCommonScale,
                                     (OMX_PTR)&sScale, sScale.xHeight, (1 << 15), eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " xHeight = ox%x (1 << 15)\n", sScale.xHeight);

        /* Get frame width and height */
        sPortDefinition.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);

        /* Verify support for OMX_IndexConfigCommonInputCrop */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying default settings for OMX_IndexConfigCommonInputCrop\n");
        sRect.nPortIndex = nPortIndex;
        eError = OMX_GetConfig(pCtx->hWrappedComp, OMX_IndexConfigCommonInputCrop, (OMX_PTR) & sRect);
        if ((sRect.nLeft != 0) ||
                (sRect.nTop != 0) ||
                (sRect.nWidth != sPortDefinition.format.image.nFrameWidth) ||
                (sRect.nHeight != sPortDefinition.format.image.nFrameHeight))
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

    OMX_ERRORTYPE StdCompCommonImage_QcifPlusPortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex,
        OMX_COLOR_FORMATTYPE eColorFormat)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;
        OMX_IMAGE_PARAM_PORTFORMATTYPE sPortFormat;

        OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
        OMX_CONF_INIT_STRUCT(sPortFormat, OMX_IMAGE_PARAM_PORTFORMATTYPE);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying QCIF+ image port %i default parameters\n", nPortIndex);

        /* Verify support for the common standard component port parameters */
        eError = StdComponentTest_StdPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Verify default port definition settings */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Verifying support and default settings for QcifPlus port definition\n");
        sPortDefinition.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
        if ((sPortDefinition.eDomain != OMX_PortDomainImage) ||
                (sPortDefinition.format.image.nFrameWidth != 176) ||
                (sPortDefinition.format.image.nFrameHeight != 220) ||
                (sPortDefinition.format.image.nStride != 352) ||
                (sPortDefinition.format.image.nSliceHeight != 16) ||
                (sPortDefinition.format.image.eCompressionFormat != OMX_IMAGE_CodingUnused) ||
                (sPortDefinition.format.image.eColorFormat != eColorFormat))
            eError = OMX_ErrorBadParameter;  // OMX_ErrorBadPortFormatEncoding
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eDomain = %d (OMX_PortDomainImage)\n", sPortDefinition.eDomain);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nFrameWidth = %d, nFrameHeight = %d\n",
                       sPortDefinition.format.image.nFrameWidth, sPortDefinition.format.image.nFrameHeight);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " nStride = %d, nSliceHeight = %d\n",
                       sPortDefinition.format.image.nStride, sPortDefinition.format.image.nSliceHeight);

        /* Verify default settings for OMX_IndexParamImagePortFormat */
        eError = StdCompCommonImage_PortFormatSupported(pCtx, nPortIndex, OMX_IMAGE_CodingUnused, eColorFormat);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eCompressionFormat = OMX_VIDEO_CodingUnused\n");
        if (OMX_COLOR_FormatYUV420Planar == eColorFormat)
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eColorFormat = OMX_COLOR_FormatYUV420Planar\n");
        else if (OMX_COLOR_Format16bitRGB565 == eColorFormat)
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, " eColorFormat = OMX_COLOR_Format16bitRGB565\n");

        eError = StdCompCommonImage_IVRendererPortParameters(pCtx, nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */

