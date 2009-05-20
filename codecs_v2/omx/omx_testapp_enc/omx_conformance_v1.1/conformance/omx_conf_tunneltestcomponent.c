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

/** OMX_CONF_TunnelTestComponent.c
 *  Common "Tunnel Test Component" as described in OpenMax IL Conformance Test Document used by
 *  Interop tests.
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "OMX_CONF_TunnelTestComponent.h"
#include "OMX_CONF_StubbedComponent.h"
#include "OMX_CONF_TestHarness.h"
#include <stdlib.h>
#include <string.h>

#define NO_MINSIZE 0x7fffffff

OMX_ERRORTYPE TTCCreateInvalidPortTypes(OMX_IN  OMX_HANDLETYPE hTTC, OMX_IN  OMX_U8 bCreateInvalid)
{
    TTCDATATYPE *pData;

    pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hTTC)->pComponentPrivate);
    pData->bCreateInvalidPorts = bCreateInvalid;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE TTCDontSupportIndexParamCompBufferSupplier(OMX_IN  OMX_HANDLETYPE hTTC, OMX_IN  OMX_U8 bSupportIndex)
{
    TTCDATATYPE *pData;

    pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hTTC)->pComponentPrivate);
    pData->bDontSupportIndexParamCompBufferSupplier = bSupportIndex;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE TTCInvertBufferSupplier(OMX_IN  OMX_HANDLETYPE hTTC, OMX_IN  OMX_U8 bInvert)
{
    TTCDATATYPE *pData;

    pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hTTC)->pComponentPrivate);
    pData->bInvertBufferSupplier = bInvert;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE TTCDontSupportUseBuffer(OMX_IN  OMX_HANDLETYPE hTTC, OMX_IN  OMX_U8 bSupportUseBuffer)
{
    TTCDATATYPE *pData;

    pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hTTC)->pComponentPrivate);
    pData->bDontDoUseBuffer = bSupportUseBuffer;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE TTCSetParameter(
        OMX_IN  OMX_HANDLETYPE hComponent, 
        OMX_IN  OMX_INDEXTYPE nIndex,
        OMX_IN  OMX_PTR ComponentParameterStructure);

OMX_ERRORTYPE TTCConnectPort(OMX_HANDLETYPE hTTC, OMX_HANDLETYPE hCUT, OMX_U32 iCUTPort)
{
    OMX_PARAM_PORTDEFINITIONTYPE oTTCPort, oCUTPort;
    OMX_PARAM_BUFFERSUPPLIERTYPE oTTCSupplier;
    OMX_PARAM_BUFFERSUPPLIERTYPE oCUTSupplier;
    OMX_ERRORTYPE eError;
    TTCDATATYPE *pData;

    pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hTTC)->pComponentPrivate);

    INIT_PARAM(oTTCSupplier);
    INIT_PARAM(oCUTSupplier);
    INIT_PARAM(oCUTPort);
    INIT_PARAM(oTTCPort);
    
    /* get CUT port definition */
    oCUTPort.nPortIndex = iCUTPort;
    TTC_RETURN_ANY_ERROR(eError = OMX_GetParameter(hCUT, OMX_IndexParamPortDefinition, &oCUTPort));

    /* Setup the TTC's port to be identical to CUT port except in the opposite direction */
    /* Setup the TTC's port to prefer that the CUT allocate buffers */
    /* We can only do this because this is the tunnel test component. */
    oTTCSupplier.nPortIndex = pData->nUsedPorts;
    oTTCPort = oCUTPort;
    oTTCPort.nPortIndex = pData->nUsedPorts;

    /* Create an invalid type if asked */
    if (pData->bCreateInvalidPorts){
        switch (oCUTPort.eDomain) {
        case OMX_PortDomainAudio:
            oTTCPort.format.audio.eEncoding = OMX_AUDIO_CodingMax;
            break;

        case OMX_PortDomainVideo:
            oTTCPort.format.video.eCompressionFormat = OMX_VIDEO_CodingMax;
            break;

        case OMX_PortDomainImage:
            oTTCPort.format.image.eCompressionFormat = OMX_IMAGE_CodingMax;
            break;
        default:
        case OMX_PortDomainOther:
            oTTCPort.format.other.eFormat = OMX_OTHER_FormatMax;
            break;
        }
    }

    if (oCUTPort.eDir == OMX_DirInput){
        oTTCPort.eDir = OMX_DirOutput;
        oTTCSupplier.eBufferSupplier = OMX_BufferSupplyInput;
    } else { /* OMX_DirOutput */
        oTTCPort.eDir = OMX_DirInput;
        oTTCSupplier.eBufferSupplier = OMX_BufferSupplyOutput;
    }
    if (pData->bInvertBufferSupplier) {
        oCUTSupplier.nSize = sizeof(oCUTSupplier);
        oCUTSupplier.nPortIndex = iCUTPort;
        oCUTSupplier.nVersion = g_OMX_CONF_Version;
        TTC_RETURN_ANY_ERROR(eError = OMX_GetParameter(hCUT, OMX_IndexParamCompBufferSupplier, &oCUTSupplier));
        if (oCUTSupplier.eBufferSupplier == OMX_BufferSupplyUnspecified) {
            oCUTSupplier.eBufferSupplier = OMX_BufferSupplyOutput;
            TTC_RETURN_ANY_ERROR(eError = OMX_SetParameter(hCUT, OMX_IndexParamCompBufferSupplier, &oCUTSupplier));
        }
    }

    TTC_RETURN_ANY_ERROR(eError = TTCSetParameter(hTTC, OMX_IndexParamPortDefinition, &oTTCPort));
    TTCSetParameter(hTTC, OMX_IndexParamCompBufferSupplier, &oTTCSupplier);

    /* create a tunnel between TTC port and CUT port depending on direction */
    if (oCUTPort.eDir == OMX_DirInput)
    {
        /* CUT port is an input: TTC to CUT */
        TTC_RETURN_ANY_ERROR(eError = OMX_SetupTunnel(hTTC, pData->nUsedPorts, hCUT, iCUTPort));
    } 
    else  /* OMX_DirOutput */
    {
        /* CUT port is an output CUT to TTC */
        TTC_RETURN_ANY_ERROR(eError = OMX_SetupTunnel(hCUT, iCUTPort, hTTC, pData->nUsedPorts));
    }
 
    pData->nUsedPorts++;

    return eError;
}

OMX_ERRORTYPE TTCDisconnectAllPorts(OMX_IN  OMX_HANDLETYPE hTTC)
{
    TTCDATATYPE *pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hTTC)->pComponentPrivate);
    OMX_U32 i;
    TTCPORTTYPE *pPort;

    for (i = 0, pPort = &pData->oPort[0]; i < pData->nUsedPorts; i++, pPort++) {
        if (pPort->hTunnelComponent) {
            /* tell the component to disconnect the tunnel if we're still connected */
            ((OMX_COMPONENTTYPE *)pPort->hTunnelComponent)->ComponentTunnelRequest(pPort->hTunnelComponent, pPort->nTunnelPort,
                                                                                   NULL, 0, NULL);
        }
        pPort->hTunnelComponent = 0;
        pPort->nTunnelPort = 0;
        pPort->eSupplierSetting = OMX_BufferSupplyUnspecified;
    }

    pData->nUsedPorts = 0;
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE TTCConnectDomainPorts(OMX_INDEXTYPE eIndexParamDomainInit, OMX_HANDLETYPE hTTC, OMX_HANDLETYPE hCUT)
{
    OMX_PORT_PARAM_TYPE oParam;
    OMX_U32 i;
    OMX_U32 iCUTPort;
    OMX_ERRORTYPE eError;

    INIT_PARAM(oParam);

    /* query the component's other ports */
    TTC_RETURN_ANY_ERROR(eError = OMX_GetParameter(hCUT, eIndexParamDomainInit, &oParam));

    /* for each discovered port */
    for (i=0;i<oParam.nPorts;i++)
    {        
        iCUTPort = oParam.nStartPortNumber + i;
        TTC_RETURN_ANY_ERROR(eError = TTCConnectPort(hTTC, hCUT, iCUTPort));
    }
    return eError;
}

OMX_ERRORTYPE OMX_CONF_TTCConnectAllPorts(    
    OMX_IN  OMX_HANDLETYPE hTunnelTestComponent,
    OMX_IN  OMX_HANDLETYPE hComponentUnderTest)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    if (OMX_ErrorNone != (eError = TTCConnectDomainPorts(OMX_IndexParamAudioInit, hTunnelTestComponent, hComponentUnderTest))) return eError;
    if (OMX_ErrorNone != (eError = TTCConnectDomainPorts(OMX_IndexParamVideoInit, hTunnelTestComponent, hComponentUnderTest))) return eError;
    if (OMX_ErrorNone != (eError = TTCConnectDomainPorts(OMX_IndexParamImageInit, hTunnelTestComponent, hComponentUnderTest))) return eError;
    if (OMX_ErrorNone != (eError = TTCConnectDomainPorts(OMX_IndexParamOtherInit, hTunnelTestComponent, hComponentUnderTest))) return eError;

    return eError;
}

/* Allows a conformance test to specify a callback (back to the test) to be called on EmptyThisBuffer calls */ 
OMX_ERRORTYPE OMX_CONF_SetTTCOnEmptyThisBuffer(    
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_ERRORTYPE (*OnEmptyThisBuffer)(OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
)
{
    TTCDATATYPE *pData;
    pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    pData->OnEmptyThisBuffer = OnEmptyThisBuffer;
    return OMX_ErrorNone;
}

/* Allows a conformance test to specify a callback (back to the test) to be called on FillThisBuffer calls */ 
OMX_ERRORTYPE OMX_CONF_SetTTCOnFillThisBuffer(    
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_ERRORTYPE (*OnFillThisBuffer)(OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
)
{
    TTCDATATYPE *pData;
    pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    pData->OnFillThisBuffer = OnFillThisBuffer;
    return OMX_ErrorNone;
}

OMX_ERRORTYPE OMX_CONF_SetTTCOnInvalidPayloadSize(
    OMX_IN  OMX_HANDLETYPE hComponent,
    OMX_ERRORTYPE (*OnInvalidPayloadSize)(OMX_IN OMX_U32 nOutPort, OMX_IN OMX_U32 nMin, OMX_IN OMX_U32 nActual)
    )
{
    TTCDATATYPE *pData;
    pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    pData->OnInvalidPayloadSize = OnInvalidPayloadSize;
    return OMX_ErrorNone;
}

/* Tunnel Test Component's implementation of OMX_COMPONENTTYPE.SetCallbacks */
OMX_ERRORTYPE TTCSetCallbacks(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_IN  OMX_CALLBACKTYPE* pCallbacks, 
        OMX_IN  OMX_PTR pAppData)
{
    TTCDATATYPE *pData;
    pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    pData->pCallbacks = pCallbacks;
    pData->pAppData = pAppData;

    return OMX_ErrorNone; 
}

static OMX_ERRORTYPE TTCFillInitStructure(OMX_HANDLETYPE hComponent,
    OMX_IN OMX_INDEXTYPE nPortDefParamIndex, OMX_INOUT OMX_PTR pComponentParameterStructure)
{
    OMX_PORT_PARAM_TYPE* pInitStruct = pComponentParameterStructure;
    unsigned i, nFirstPort, nLastPort;
    TTCDATATYPE *pData; 
    pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    nFirstPort = nLastPort = pData->nUsedPorts;
    for (i = 0; i < pData->nUsedPorts; i++) {
        if (pData->oPort[i].nPortDefParamIndex == nPortDefParamIndex)
        {
            if (nFirstPort > i)
                nFirstPort = i;
            nLastPort = i;
        }
        else if (nFirstPort < i)
            break;
    }
    if (nFirstPort < pData->nUsedPorts)
    {
        pInitStruct->nPorts = nLastPort-nFirstPort+1;
        pInitStruct->nStartPortNumber = nFirstPort;
        return OMX_ErrorNone;
    }
    else
    {
        pInitStruct->nPorts = 0;
        pInitStruct->nStartPortNumber = 0;
        return OMX_ErrorNone;
    }
}

/* Tunnel Test Component's implementation of OMX_COMPONENTTYPE.GetParameter */
OMX_ERRORTYPE TTCGetParameter(
        OMX_IN  OMX_HANDLETYPE hComponent, 
        OMX_IN  OMX_INDEXTYPE nParamIndex,  
        OMX_INOUT OMX_PTR ComponentParameterStructure)
{   
    OMX_PARAM_PORTDEFINITIONTYPE *pPortDef;
    OMX_PARAM_BUFFERSUPPLIERTYPE *pSupplier;
    TTCDATATYPE *pData;
    TTCPORTTYPE *pPort;
    pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    switch(nParamIndex)
    {
    case OMX_IndexParamAudioInit: 
        return TTCFillInitStructure(hComponent, OMX_PortDomainAudio, ComponentParameterStructure);
    case OMX_IndexParamOtherInit:
        return TTCFillInitStructure(hComponent, OMX_PortDomainOther, ComponentParameterStructure);
    case OMX_IndexParamVideoInit:
        return TTCFillInitStructure(hComponent, OMX_PortDomainVideo, ComponentParameterStructure);
    case OMX_IndexParamImageInit:
        return TTCFillInitStructure(hComponent, OMX_PortDomainImage, ComponentParameterStructure);

    case OMX_IndexParamPortDefinition:            
        pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *)ComponentParameterStructure;
        *pPortDef = pData->oPort[pPortDef->nPortIndex].oPortDef;
        return OMX_ErrorNone;

    case OMX_IndexParamCompBufferSupplier:
        if (pData->bDontSupportIndexParamCompBufferSupplier)
            return OMX_ErrorNotImplemented;

        pSupplier = (OMX_PARAM_BUFFERSUPPLIERTYPE *)ComponentParameterStructure;
        pPort = &pData->oPort[pSupplier->nPortIndex];
        if (pPort->hTunnelComponent){
            pSupplier->eBufferSupplier = pPort->eSupplierSetting;
        } else {
            pSupplier->eBufferSupplier = pPort->eSupplierPreference;
        }
        return OMX_ErrorNone;

	default:
		return OMX_ErrorBadParameter;
    }
    return OMX_ErrorBadParameter; 
}

/* Tunnel Test Component's implementation of OMX_COMPONENTTYPE.SetParameter */
OMX_ERRORTYPE TTCSetParameter(
        OMX_IN  OMX_HANDLETYPE hComponent, 
        OMX_IN  OMX_INDEXTYPE nIndex,
        OMX_IN  OMX_PTR ComponentParameterStructure)
{   
    OMX_PARAM_PORTDEFINITIONTYPE *pPortDef;
    OMX_PARAM_BUFFERSUPPLIERTYPE *pSupplier;
    OMX_PARAM_BUFFERSUPPLIERTYPE oTunnelBufferSupply;
    TTCDATATYPE *pData;
    TTCPORTTYPE *pPort;
    pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    switch(nIndex)
    {
    case OMX_IndexParamPortDefinition:            
        pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE *)ComponentParameterStructure;
        pPort = &pData->oPort[pPortDef->nPortIndex];
        pPort->nPreferredCount = pPortDef->nBufferCountMin;
        pPort->nPreferredSize = pPortDef->nBufferSize;
        pPort->oPortDef = *pPortDef;
        return OMX_ErrorNone;
    case OMX_IndexParamCompBufferSupplier:
        if (pData->bDontSupportIndexParamCompBufferSupplier)
            return OMX_ErrorNotImplemented;

        pSupplier = (OMX_PARAM_BUFFERSUPPLIERTYPE *)ComponentParameterStructure;
        pPort = &pData->oPort[pSupplier->nPortIndex];
        if (pPort->hTunnelComponent){
            pPort->eSupplierSetting = pSupplier->eBufferSupplier;
            if (pPort->oPortDef.eDir == OMX_DirInput){
                oTunnelBufferSupply = *pSupplier;
                oTunnelBufferSupply.nPortIndex = pPort->nTunnelPort;
                return OMX_SetParameter(pPort->hTunnelComponent, OMX_IndexParamCompBufferSupplier, &oTunnelBufferSupply);
            }
        } else {
            pPort->eSupplierPreference = pSupplier->eBufferSupplier;            
        }
        return OMX_ErrorNone;

	default:
		return OMX_ErrorBadParameter;
    }
    return OMX_ErrorBadParameter; 
}

/* Tunnel Test Component's implementation of OMX_COMPONENTTYPE.DeInit */
OMX_ERRORTYPE TTCDeInit(
        OMX_IN  OMX_HANDLETYPE hComponent)
{ 
    TTCDATATYPE *pData;
    pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);


    OMX_OSAL_EventDestroy(pData->hBufferCountEvent);
    OMX_OSAL_MutexDestroy(pData->hMutex);
    OMX_OSAL_EventDestroy(pData->hHoldingBuffersEvent);

    OMX_OSAL_Free(pData);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE TTCTunnelVerifyConnection(TTCPORTTYPE *pPort, OMX_HANDLETYPE hTunneledComp)
{
    /* 1.4 Check if input port is compatible with output port */
    OMX_PARAM_PORTDEFINITIONTYPE oPortDef;
    OMX_ERRORTYPE error = OMX_ErrorNone;

   oPortDef.nSize = sizeof(oPortDef);
   oPortDef.nVersion = g_OMX_CONF_Version;
   oPortDef.nPortIndex = pPort->nTunnelPort;
   if (OMX_ErrorNone != 
       (error = OMX_GetParameter( hTunneledComp, OMX_IndexParamPortDefinition, &oPortDef))) return error;
   switch(pPort->oPortDef.eDomain)
   {
   case OMX_PortDomainOther:
       if (pPort->oPortDef.format.other.eFormat!= oPortDef.format.other.eFormat) 
        {
            pPort->hTunnelComponent = 0; 
            pPort->nTunnelPort      = 0;
            return OMX_ErrorPortsNotCompatible;
        }
        break;
    case OMX_PortDomainAudio:
        if (pPort->oPortDef.format.audio.eEncoding != oPortDef.format.audio.eEncoding) 
        {
            pPort->hTunnelComponent = 0; 
            pPort->nTunnelPort      = 0;
            return OMX_ErrorPortsNotCompatible;
        }
        break;
    case OMX_PortDomainVideo:
        if (pPort->oPortDef.format.video.eCompressionFormat != oPortDef.format.video.eCompressionFormat) 
        {
            pPort->hTunnelComponent = 0; 
            pPort->nTunnelPort      = 0;
            return OMX_ErrorPortsNotCompatible;
        }
        break;
    case OMX_PortDomainImage:
        if (pPort->oPortDef.format.image.eCompressionFormat != oPortDef.format.image.eCompressionFormat) 
        {
            pPort->hTunnelComponent = 0; 
            pPort->nTunnelPort      = 0;
            return OMX_ErrorPortsNotCompatible;
        }
        break;
    default: 
        pPort->hTunnelComponent = 0;
        pPort->nTunnelPort      = 0;
        return OMX_ErrorPortsNotCompatible; /* Our current port is not set up correctly */
    }
    return error;
}

OMX_ERRORTYPE TTCPortGetMinPayloadSize(TTCPORTTYPE *pPort)
{
    OMX_AUDIO_PARAM_PCMMODETYPE oPcm;

    pPort->nMinBytes = NO_MINSIZE; /* default to a very large size */

    switch(pPort->oPortDef.eDomain){
    case OMX_PortDomainAudio:
        if (pPort->oPortDef.format.audio.eEncoding == OMX_AUDIO_CodingPCM)
        {
            oPcm.nSize = sizeof(oPcm);
            oPcm.nVersion = g_OMX_CONF_Version;
            oPcm.nPortIndex = pPort->nTunnelPort;
            if (OMX_ErrorNone == OMX_GetParameter(pPort->hTunnelComponent, OMX_IndexParamAudioPcm, &oPcm)){
                pPort->nMinBytes = 
                    (oPcm.nBitPerSample * oPcm.nChannels * oPcm.nSamplingRate) /* bits per sec */
                    /( (1000/OMX_MIN_PCMPAYLOAD_MSEC) /* Divide by number of time segments in a second */
                        *8);                          /* and by 8 bits per byte */
            }
        }
        break;
    case OMX_PortDomainVideo:
        if ((pPort->oPortDef.format.video.eCompressionFormat == OMX_VIDEO_CodingUnused) &&
            (pPort->oPortDef.format.video.eColorFormat != OMX_COLOR_FormatUnused))
        {
            pPort->nMinBytes = pPort->oPortDef.format.video.nStride * 
                pPort->oPortDef.format.video.nSliceHeight;

            pPort->nPlaneBytesTotal = pPort->oPortDef.format.video.nStride * 
                pPort->oPortDef.format.video.nFrameHeight;
        }
        break;
    case OMX_PortDomainImage:
        if ((pPort->oPortDef.format.image.eCompressionFormat == OMX_IMAGE_CodingUnused) &&
            (pPort->oPortDef.format.image.eColorFormat != OMX_COLOR_FormatUnused))
        {
            pPort->nMinBytes = pPort->oPortDef.format.image.nStride * 
                pPort->oPortDef.format.image.nSliceHeight; 
        }
        break;               
    default:
    case OMX_PortDomainOther:
        pPort->nMinBytes = 0;
        break;               
    }
    return OMX_ErrorNone;
}

/* Tunnel Test Component's implementation of OMX_COMPONENTTYPE.TunnelRequest */
OMX_ERRORTYPE TTCTunnelRequest(
    OMX_IN  OMX_HANDLETYPE hComp,
    OMX_IN  OMX_U32 nPort,
    OMX_IN  OMX_HANDLETYPE hTunneledComp,
    OMX_IN  OMX_U32 nTunneledPort,
    OMX_INOUT  OMX_TUNNELSETUPTYPE* pTunnelSetup)
{
    OMX_ERRORTYPE error = OMX_ErrorNone;
    TTCDATATYPE *pData;
    TTCPORTTYPE *pPort;
    OMX_PARAM_BUFFERSUPPLIERTYPE oSupplier;

    pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hComp)->pComponentPrivate);
    pPort = &pData->oPort[nPort];

    if (pTunnelSetup == NULL || hTunneledComp == 0) {
        /* cancel previous tunnel */
        pPort->hTunnelComponent = 0;
        pPort->nTunnelPort = 0;
        pPort->eSupplierSetting = OMX_BufferSupplyUnspecified;
    }
    else {
        if (pPort->oPortDef.eDir != OMX_DirInput && pPort->oPortDef.eDir != OMX_DirOutput) return OMX_ErrorBadParameter;

        pPort->hTunnelComponent = hTunneledComp;
        pPort->nTunnelPort = nTunneledPort;

        if (pPort->oPortDef.eDir == OMX_DirOutput) {
            /* first call, where we're the output (source of data) */

            pTunnelSetup->eSupplier = pPort->eSupplierSetting;
        }
        else {
            /* second call, where we're the input (sink of data) */
            error = TTCTunnelVerifyConnection(pPort, hTunneledComp);
            if(OMX_ErrorNone != error)
            {
                /* Invalid connection formats. Return error */
                return OMX_ErrorPortsNotCompatible;
            }
           
            /* If specified obey output port's preferences. Otherwise choose CUT (output). */
            if (pData->bInvertBufferSupplier) {
                if (pTunnelSetup->eSupplier == OMX_BufferSupplyInput)
                    pPort->eSupplierSetting = OMX_BufferSupplyOutput;
                else
                    pPort->eSupplierSetting = OMX_BufferSupplyInput;
            }
            else {
                pPort->eSupplierSetting = pTunnelSetup->eSupplier;
            }
            if (OMX_BufferSupplyUnspecified == pPort->eSupplierSetting){
                pPort->eSupplierSetting = pTunnelSetup->eSupplier = OMX_BufferSupplyOutput;
            }

            /* Tell the output port who the supplier is */
            oSupplier.nSize = sizeof(oSupplier);
            oSupplier.nVersion = g_OMX_CONF_Version;
            oSupplier.nPortIndex = nTunneledPort;
            oSupplier.eBufferSupplier = pPort->eSupplierSetting;
            error = OMX_SetParameter(hTunneledComp, OMX_IndexParamCompBufferSupplier, &oSupplier);

            if (pData->bInvertBufferSupplier) {
                oSupplier.nSize = sizeof(oSupplier);
                oSupplier.nVersion = g_OMX_CONF_Version;
                oSupplier.nPortIndex = nTunneledPort;
                error = OMX_GetParameter(hTunneledComp, OMX_IndexParamCompBufferSupplier, &oSupplier);
                if (oSupplier.eBufferSupplier != pPort->eSupplierSetting) {
                    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "- SetParameter: OMX_IndexParamCompBufferSupplier failed to change setting\n" );
                    return OMX_ErrorUndefined;
                }
            }
        }

        /* determine min playload size if any */
        TTCPortGetMinPayloadSize(pPort);

    }
    return error;
}

/* TTC calls this on transition to idle to allocate buffers for a supplier port */
OMX_ERRORTYPE TTCUpdatePortBufferInfo(TTCPORTTYPE *pPort)
{
    OMX_PARAM_PORTDEFINITIONTYPE oPortDef;

    /* initialize buffer info to this ports preferences first */
    pPort->nBufferCount = pPort->nPreferredCount;
    pPort->nBufferSize = pPort->nPreferredSize;

    oPortDef.nVersion = g_OMX_CONF_Version;
    oPortDef.nSize = sizeof(oPortDef);
    oPortDef.nPortIndex = pPort->nTunnelPort;
    if (OMX_ErrorNone != OMX_GetParameter(pPort->hTunnelComponent, OMX_IndexParamPortDefinition, &oPortDef)){
        return OMX_ErrorUndefined;
    }
    if (oPortDef.nBufferCountMin >= MAX_TTCBUFFERS) {
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "TunnelTestComponent: CUT needs too many buffer (%d min for CUT, %d max for TTC)\n", oPortDef.nBufferCountMin, MAX_TTCBUFFERS);
        return OMX_ErrorUndefined;
    }

    if (pPort->nBufferCount < oPortDef.nBufferCountMin) pPort->nBufferCount = oPortDef.nBufferCountMin;
    if (pPort->nBufferSize < oPortDef.nBufferSize) pPort->nBufferSize = oPortDef.nBufferSize;

   return OMX_ErrorNone;
}

/* Returns true is and only if the port is a supplier */
OMX_BOOL TTCPortIsSupplier(TTCPORTTYPE *pPort)
{
    if ((pPort->oPortDef.eDir == OMX_DirInput && pPort->eSupplierSetting == OMX_BufferSupplyInput) ||
        (pPort->oPortDef.eDir == OMX_DirOutput && pPort->eSupplierSetting == OMX_BufferSupplyOutput))
    {
        return OMX_TRUE;
    }
    return OMX_FALSE;
}

/* Tunnel Test Component's implementation of OMX_COMPONENTTYPE.SendCommand */
OMX_ERRORTYPE TTCSendCommand(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_IN  OMX_COMMANDTYPE Cmd,
        OMX_IN  OMX_U32 nParam1,
        OMX_IN  OMX_PTR pCmdData)
{   
    TTCDATATYPE *pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    TTCPORTTYPE *pPort;
    OMX_ERRORTYPE eError;
    OMX_U32 i,j;

    UNUSED_PARAMETER(pCmdData);

    switch(Cmd){
    case OMX_CommandStateSet:
        /* skip error checking - trust conformance test */

        /* if transitioning to idle then allocate buffers for any supplier ports*/
        if (OMX_StateIdle == (OMX_STATETYPE)nParam1 && pData->eState == OMX_StateLoaded)
        {
            for (i=0;i<MAX_TTCPORTS;i++)
            {
                pPort = &(pData->oPort[i]);
                /* if port is tunneling and is the supplier*/
                if (pPort->hTunnelComponent &&  TTCPortIsSupplier(pPort)) 
                {
                    if (OMX_ErrorNone != (eError = TTCUpdatePortBufferInfo(pPort))) return eError;

                    /* allocate buffers and pass them to non-supplier port */
                    for (j=0;j<pPort->nBufferCount;j++)
                    {
                        pPort->pBuffer[j] = OMX_OSAL_Malloc(pPort->nBufferSize);
                        if (OMX_ErrorNone != (eError = OMX_UseBuffer(pPort->hTunnelComponent, &pPort->pBufferHdr[j],
                            pPort->nTunnelPort, pPort, pPort->nBufferSize, pPort->pBuffer[j]))) return eError;
                    }
                }   
                /* if tunneling with an input open the file that will feed the input*/
                if (pPort->hTunnelComponent){
                    if (pPort->oPortDef.eDir == OMX_DirOutput){
                        eError = OMX_OSAL_OpenInputFile(pPort->nTunnelPort);
                    } else { /* input */ 
                        eError = OMX_OSAL_OpenOutputFile(pPort->nTunnelPort);                    
                    }
                    if (eError != OMX_ErrorNone && eError != OMX_ErrorBadParameter) {
                        OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "TTC: file open on port %d failed\n", pPort->nTunnelPort);
                        return eError;
                    }
                }    
            }
        }
        if (OMX_StateLoaded == (OMX_STATETYPE)nParam1)
        {
            for(i=0;i<MAX_TTCPORTS;i++)
            {
                /* deallocate any buffers if we are supplier - because of the way tests are constructed 
                * (i.e. free component under test first) all buffers should be returned. So we don't
                * check. */
                pPort = &pData->oPort[i];
                if (pPort->hTunnelComponent){
                    if (TTCPortIsSupplier(pPort))
                    {   
                        /* supplier - for each buffer... */
                        for (j=0;j<pPort->nBufferCount;j++)
                        {
                            /* free buffer */
                            if (pPort->pBuffer[j]) {
                                OMX_OSAL_Free(pPort->pBuffer[j]);
                                pPort->pBuffer[j] = 0;              
                            }

                            /* tell the non-supplier to free the buffer header */
                            if (pPort->pBufferHdr[j]) {
                                OMX_FreeBuffer(pPort->hTunnelComponent, pPort->nTunnelPort, pPort->pBufferHdr[j]);
                                pPort->pBufferHdr[j] = 0;
                            }
                        }
                    }
              
                    /* if tunneling with an input close the file that will feed the input*/
                    if (pPort->hTunnelComponent){
                        if (pPort->oPortDef.eDir == OMX_DirOutput) {
                            OMX_OSAL_CloseInputFile(pPort->nTunnelPort);
                        } else { /* input */ 
                            OMX_OSAL_CloseOutputFile(pPort->nTunnelPort);
                        }
                    }
                }
            }        
        }
        if (OMX_StateExecuting == (OMX_STATETYPE)nParam1)
        {
            /* initialize any plane byte counters */
            for(i=0;i<MAX_TTCPORTS;i++)
            {
                pPort = &pData->oPort[i];
                if ((pPort->oPortDef.eDir == OMX_DirOutput) && 
                    (pPort->nMinBytes != NO_MINSIZE) &&
                    (pPort->oPortDef.eDomain == OMX_PortDomainVideo))
                {
                    pPort->nPlaneBytesEmitted = 0;
                }
            }
        }
        pData->eState = (OMX_STATETYPE)nParam1;
        break;
    default:
        return OMX_ErrorNotImplemented; 
    }

    /* call event handler */
/*    pData->pCallbacks->EventHandler(hComponent, pData->pAppData, OMX_EventCmdComplete,
        OMX_CommandStateSet, pData->eState, 0);
*/        
    return OMX_ErrorNone;
}

/* Tunnel Test Component's implementation of OMX_COMPONENTTYPE.UseBuffer */
OMX_ERRORTYPE TTCUseBuffer(
        OMX_IN OMX_HANDLETYPE hComponent,
        OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
        OMX_IN OMX_U32 nPortIndex,
        OMX_IN OMX_PTR pAppPrivate,
        OMX_IN OMX_U32 nSizeBytes,
        OMX_IN OMX_U8* pBuffer)
{   
    TTCDATATYPE *pData; 
    TTCPORTTYPE *pPort;
    OMX_U32 i;
    OMX_U8 *p;
    
    /* Note: we can always assume pAppPrivate is from another port because we're the TTC.
     * I.e. our ports *always* tunnel. */

    pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    if (pData->bDontDoUseBuffer)
        return OMX_ErrorNotImplemented;

    pPort = &pData->oPort[nPortIndex];
    pPort->pBufferHdr[pPort->nBufferCount] = OMX_OSAL_Malloc(sizeof(OMX_BUFFERHEADERTYPE));
    *ppBufferHdr = pPort->pBufferHdr[pPort->nBufferCount]; 

    /* clear buffer header */
    for(p=(OMX_U8*)(*ppBufferHdr),i=0;i<sizeof(OMX_BUFFERHEADERTYPE);i++) *p=0;

    /* set relevant fields */
    (*ppBufferHdr)->nSize = sizeof(OMX_BUFFERHEADERTYPE);
    (*ppBufferHdr)->nVersion = g_OMX_CONF_Version;
    (*ppBufferHdr)->pBuffer = pBuffer;
    (*ppBufferHdr)->nAllocLen = nSizeBytes;

    /* set direction dependent fields */
    if (pPort->oPortDef.eDir == OMX_DirInput){
        (*ppBufferHdr)->nInputPortIndex     = nPortIndex;
        (*ppBufferHdr)->pInputPortPrivate   = pPort;
        (*ppBufferHdr)->nOutputPortIndex    = pPort->nTunnelPort;
        (*ppBufferHdr)->pOutputPortPrivate  = pAppPrivate;
        (*ppBufferHdr)->pAppPrivate         = pAppPrivate;
    } else {
        (*ppBufferHdr)->nInputPortIndex     = pPort->nTunnelPort;
        (*ppBufferHdr)->pInputPortPrivate   = pAppPrivate;
        (*ppBufferHdr)->nOutputPortIndex    = nPortIndex;
        (*ppBufferHdr)->pOutputPortPrivate  = pPort;
        (*ppBufferHdr)->pAppPrivate         = pAppPrivate;
    }
        
    /* increment buffer count */
    pPort->nBufferCount++;

    /* todo: put hook into conformance test */
   
    return OMX_ErrorNone; 
}

OMX_ERRORTYPE TTCHoldThisBuffer(OMX_IN TTCDATATYPE *pData, OMX_IN OMX_BUFFERHEADERTYPE *pBuffer)
{
    TTCPORTTYPE *pPort;
    OMX_U32 i;

    pPort = &pData->oPort[pBuffer->nInputPortIndex];
 
    /* find the buffer in the ports list of buffers and set the corresponding boolean to mark it held */
    for (i=0;i<pPort->nBufferCount;i++){
        if (pBuffer == pPort->pBufferHdr[i]){
            pPort->bHoldingBuffer[i] = OMX_TRUE;
            OMX_OSAL_EventSet(pData->hHoldingBuffersEvent);
            return OMX_ErrorNone;     
        }
    }
    return OMX_ErrorBadParameter;
}

/* Tunnel Test Component's implementation of OMX_COMPONENTTYPE.EmptyThisBuffer */
OMX_ERRORTYPE TTCEmptyThisBuffer(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{   
    TTCDATATYPE *pData;    
    TTCPORTTYPE *pPort;
    pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    /* check for buffer countdown */
    OMX_OSAL_MutexLock(pData->hMutex);
    if (pData->nBuffersLeft)
    {
        /* decrement buffer count and signal if last one */
        pData->nBuffersLeft--;
        if (0==pData->nBuffersLeft){
            OMX_OSAL_EventSet(pData->hBufferCountEvent);
        }
    }
    OMX_OSAL_MutexUnlock(pData->hMutex);

    if (pData->OnInvalidPayloadSize){
        pPort = &pData->oPort[pBuffer->nInputPortIndex];
        if (pPort->nMinBytes > pBuffer->nFilledLen && pPort->nMinBytes != NO_MINSIZE && 0 == (pBuffer->nFlags & OMX_BUFFERFLAG_EOS)){
            pData->OnInvalidPayloadSize(pBuffer->nOutputPortIndex, pPort->nMinBytes , pBuffer->nFilledLen); 
        }
    }

    if (pData->OnEmptyThisBuffer){
        pData->OnEmptyThisBuffer(pBuffer);
    }
    
    /* write to output file */
    OMX_OSAL_WriteToOutputFile(pBuffer->pBuffer+pBuffer->nOffset,
        pBuffer->nFilledLen,pBuffer->nInputPortIndex);

    /* are we holding buffers? */
    if (pData->bHoldBuffers){
        /* then keep the buffer */
        return TTCHoldThisBuffer(pData,pBuffer);
    }

    OMX_FillThisBuffer(pData->oPort[pBuffer->nInputPortIndex].hTunnelComponent,pBuffer);

    /* todo: put hook into conformance test */

    return OMX_ErrorNone; 
}

OMX_ERRORTYPE TTCFillThisBuffer(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{   
    OMX_U32 nReadSize;
    TTCDATATYPE *pData;    
    TTCPORTTYPE *pPort;    
    pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    pPort = &pData->oPort[pBuffer->nOutputPortIndex];

    /* clear relevant buffer header fields */
    pBuffer->nFilledLen = 0;
    pBuffer->nOffset = 0;
    pBuffer->nFlags = 0;
    pBuffer->hMarkTargetComponent = 0;
    pBuffer->pMarkData = 0;
    pBuffer->nTimeStamp = 0;
    pBuffer->nTickCount = 0;

    /* check for buffer countdown */
    OMX_OSAL_MutexLock(pData->hMutex);
    if (pData->nBuffersLeft)
    {
        /* decrement buffer count and signal if last one */
        pData->nBuffersLeft--;
        if (0==pData->nBuffersLeft){
            OMX_OSAL_EventSet(pData->hBufferCountEvent);
        }
    }
    OMX_OSAL_MutexUnlock(pData->hMutex);

    if (pData->OnFillThisBuffer){
        pData->OnFillThisBuffer(pBuffer);
    }   

    /* did we reach the EOS on a previous call? Are we holding buffers?*/
    if (pPort->bEOS || pData->bHoldBuffers){
        /* then keep the buffer */
        return TTCHoldThisBuffer(pData,pBuffer);
    }

    nReadSize = pBuffer->nAllocLen;
    if ((NO_MINSIZE != pPort->nMinBytes) && (pPort->nMinBytes<nReadSize)){ 
        nReadSize = pPort->nMinBytes;
        /* if uncompressed video and a plane boundary */
        if ((pPort->oPortDef.eDomain == OMX_PortDomainVideo) && 
            ((nReadSize + pPort->nPlaneBytesEmitted) > pPort->nPlaneBytesTotal))
        {
            nReadSize = pPort->nPlaneBytesTotal - pPort->nPlaneBytesEmitted;
            pPort->nPlaneBytesEmitted = 0; /* reset counter */
        }
    }

    /* read more data from input file */
    /* if we didn't get as much data as expected then send EOS */ 
    if (nReadSize != (pBuffer->nFilledLen = OMX_OSAL_ReadFromInputFile(pBuffer->pBuffer,
        nReadSize, pBuffer->nInputPortIndex)))
    {
        pBuffer->nFlags |= OMX_BUFFERFLAG_EOS;
        pPort->bEOS = OMX_TRUE;
    }
    OMX_EmptyThisBuffer(pPort->hTunnelComponent,pBuffer);

    return OMX_ErrorNone; 
}

/* Tunnel Test Component's implementation of OMX_COMPONENTTYPE.GetState */
OMX_ERRORTYPE TTCGetState(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_OUT OMX_STATETYPE* pState)
{   
    TTCDATATYPE *pData;
    pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    *pState = pData->eState;
    return OMX_ErrorNone; 
}

OMX_ERRORTYPE TTCHoldBuffers(OMX_IN  OMX_HANDLETYPE hTunnelTestComponent)
{
    OMX_BOOL bTimedOut;
    TTCDATATYPE *pData;
    pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hTunnelTestComponent)->pComponentPrivate);

    /* reset the holding buffers event */
    OMX_OSAL_EventReset(pData->hHoldingBuffersEvent);

    /* tell ports to hold their buffers */
    pData->bHoldBuffers = OMX_TRUE;

    /* wait until we're holding a buffer */
    OMX_OSAL_EventWait(pData->hHoldingBuffersEvent,500, &bTimedOut);

    return bTimedOut ? OMX_ErrorTimeout:OMX_ErrorNone;
}

OMX_ERRORTYPE TTCReleaseBuffers(OMX_IN  OMX_HANDLETYPE hTunnelTestComponent)
{
    OMX_U32 i,j;
    TTCPORTTYPE *pPort;
    TTCDATATYPE *pData;
    pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hTunnelTestComponent)->pComponentPrivate);
    
    /* tell ports not to hold their buffers */
    pData->bHoldBuffers = OMX_FALSE;

    for(i=0;i<pData->nUsedPorts;i++)
    {
        pPort = &pData->oPort[i];
        
        /* if port is an input but not the supplier then return the buffers to the supplier */
        if ((pPort->oPortDef.eDir == OMX_DirInput) && (pPort->eSupplierSetting != OMX_BufferSupplyInput)) {
            for(j=0;j<pPort->nBufferCount;j++){
                if (pPort->bHoldingBuffer[j] ){
                    OMX_FillThisBuffer(pPort->hTunnelComponent,pPort->pBufferHdr[j]);
                }
            }
        }         /* if port is an output but not the supplier then return the buffers to the supplier */
        else if ((pPort->oPortDef.eDir == OMX_DirOutput) && (pPort->eSupplierSetting != OMX_BufferSupplyOutput)){
            for(j=0;j<pPort->nBufferCount;j++){
                if (pPort->bHoldingBuffer[j] ){
                    OMX_EmptyThisBuffer(pPort->hTunnelComponent,pPort->pBufferHdr[j]);
                }
            }
        }
    }
    return OMX_ErrorNone;
}

/* Tunnel Test Component's implementation of OMX_COMPONENTTYPE.FreeBuffer */
OMX_ERRORTYPE TTCFreeBuffer(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_IN  OMX_U32 nPortIndex,
        OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{   
    OMX_U32 i;
    TTCPORTTYPE *pPort;
    TTCDATATYPE *pData;
    pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);
    pPort = &pData->oPort[nPortIndex];

    /* find the buffer header and delete it */
    /* NOTE: the TTC will never have a port connected to the IL client so we can assume tunneling. */
    for (i=0;i<pPort->nBufferCount;i++){
        if (pPort->pBufferHdr[i] == pBuffer){
            OMX_OSAL_Free(pBuffer);
            pPort->pBufferHdr[i] = 0;
            pPort->pBuffer[i] = 0;
        }
    }

    return OMX_ErrorNone; 
}

/* Component Initialization function for Tunnel Test Component. */
OMX_ERRORTYPE TunnelTestComponentInit(OMX_IN  OMX_HANDLETYPE hComponent)
{
    OMX_U32 i,j;
    OMX_COMPONENTTYPE *pComp;
    TTCDATATYPE *pData; 
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    pComp = (OMX_COMPONENTTYPE *)hComponent;

    /* create private data */
    pData = (TTCDATATYPE *)OMX_OSAL_Malloc(sizeof(TTCDATATYPE));
    pData->OnEmptyThisBuffer = NULL;
    pData->OnFillThisBuffer = NULL;
    pData->OnInvalidPayloadSize = NULL;
    pData->nUsedPorts = 0;
    pComp->pComponentPrivate = (OMX_PTR)pData;
    pData->bCreateInvalidPorts = OMX_FALSE;
    pData->bDontSupportIndexParamCompBufferSupplier = OMX_FALSE;
    pData->bInvertBufferSupplier = OMX_FALSE;
    pData->bDontDoUseBuffer = OMX_FALSE;
    pData->bHoldBuffers = OMX_FALSE;
    OMX_OSAL_EventCreate(&pData->hHoldingBuffersEvent);

    OMX_OSAL_EventCreate(&pData->hBufferCountEvent);
    pData->nBuffersLeft = 0;
    OMX_OSAL_MutexCreate(&pData->hMutex);

    /* fill in function pointers */
    pComp->SetCallbacks =           TTCSetCallbacks;
    pComp->GetComponentVersion =    StubbedGetComponentVersion;
    pComp->SendCommand =            TTCSendCommand;
    pComp->GetParameter =           TTCGetParameter;
    pComp->SetParameter =           TTCSetParameter;
    pComp->GetConfig =              StubbedGetConfig;
    pComp->SetConfig =              StubbedSetConfig;
    pComp->GetExtensionIndex =      StubbedGetExtensionIndex;
    pComp->GetState =               TTCGetState;
    pComp->ComponentTunnelRequest = TTCTunnelRequest;
    pComp->UseBuffer =              TTCUseBuffer;
    pComp->AllocateBuffer =         StubbedAllocateBuffer;
    pComp->FreeBuffer =             TTCFreeBuffer;
    pComp->EmptyThisBuffer =        TTCEmptyThisBuffer;
    pComp->FillThisBuffer =         TTCFillThisBuffer;
    pComp->ComponentDeInit =        TTCDeInit;

    /* initialize ports */
    for (i=0;i<MAX_TTCPORTS; i++)
    {
        pData->oPort[i].nPortIndex = i;
        pData->oPort[i].hTunnelComponent = NULL;
        pData->oPort[i].nTunnelPort = 0;
        pData->oPort[i].nPreferredCount = 0;
        pData->oPort[i].nPreferredSize = 0;
        pData->oPort[i].nBufferCount = 0;
        pData->oPort[i].nBufferSize = 0;
        pData->oPort[i].nPortDefParamIndex = 0;
        pData->oPort[i].bEOS = OMX_FALSE;
        pData->oPort[i].eSupplierSetting = pData->oPort[i].eSupplierPreference = OMX_BufferSupplyUnspecified;
        memset(&pData->oPort[i].oPortDef, 0, sizeof(pData->oPort[i].oPortDef));
        for(j=0;j<MAX_TTCBUFFERS;j++) pData->oPort[i].bHoldingBuffer[j] = OMX_FALSE;
    }

    /* initialize state */
    pData->eState = OMX_StateLoaded;

    return eError;
}

OMX_ERRORTYPE OMX_CONF_WaitForBufferTraffic(OMX_IN OMX_HANDLETYPE hComponent)
{
    TTCDATATYPE *pData; 
    OMX_BOOL bTimedOut;
    pData = (TTCDATATYPE *)(((OMX_COMPONENTTYPE*)hComponent)->pComponentPrivate);

    /* trigger buffer countdown */
    OMX_OSAL_MutexLock(pData->hMutex);
    OMX_OSAL_EventReset(pData->hBufferCountEvent);
    pData->nBuffersLeft = OMX_CONF_BUFFERS_OF_TRAFFIC;
    OMX_OSAL_MutexUnlock(pData->hMutex);

    /* wait for the countdown to get to zero */
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, 
        "Waiting for at least %i calls to EmptyThisBuffer or FillThisBuffer.\n", 
        OMX_CONF_BUFFERS_OF_TRAFFIC);
    OMX_OSAL_EventWait(pData->hBufferCountEvent, OMX_CONF_TIMEOUT_BUFFER_TRAFFIC, &bTimedOut);

    /* cancel buffer countdown */
    OMX_OSAL_MutexLock(pData->hMutex);
    pData->nBuffersLeft = 0;
    OMX_OSAL_MutexUnlock(pData->hMutex);

    /* if we timed out then the wait has failed */
    if (bTimedOut){
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,
                       "Expected %i buffers processed in %i ms.  Timed out, proceeding with test.\n",
                       OMX_CONF_BUFFERS_OF_TRAFFIC,
                       OMX_CONF_TIMEOUT_BUFFER_TRAFFIC);
    }        

    return OMX_ErrorNone;
}

OMX_ERRORTYPE OMX_CONF_GetTunnelTestComponentHandle(
        OMX_OUT OMX_HANDLETYPE *pHandle, 
        OMX_IN  OMX_PTR pAppData,
        OMX_IN  OMX_CALLBACKTYPE* pCallBacks)
{
    OMX_ERRORTYPE eError;
    OMX_COMPONENTTYPE *pComp;
    OMX_HANDLETYPE hHandle;

    if (pHandle == NULL || pCallBacks == NULL)
        return OMX_ErrorBadParameter;

    hHandle = 0;
    pComp = (OMX_COMPONENTTYPE *)OMX_OSAL_Malloc(sizeof(OMX_COMPONENTTYPE));
    hHandle = (OMX_HANDLETYPE)pComp;
    pComp->nVersion.s.nVersionMajor    = 1;
    pComp->nVersion.s.nVersionMinor    = 0;
    pComp->nVersion.s.nRevision        = 0;
    pComp->nVersion.s.nStep            = 0;
    pComp->nSize = sizeof(OMX_COMPONENTTYPE);
    eError = TunnelTestComponentInit(hHandle);
    if (eError == OMX_ErrorNone) {
        eError = pComp->SetCallbacks(hHandle,pCallBacks,pAppData);
        if (eError != OMX_ErrorNone)
            TTCDeInit(hHandle);
    }
    if (eError == OMX_ErrorNone) {
        *pHandle = hHandle;
    }
    else {
        *pHandle = NULL;
        OMX_OSAL_Free(pComp);
    }
    return eError;
}

OMX_ERRORTYPE OMX_CONF_FreeTunnelTestComponentHandle(OMX_IN OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_COMPONENTTYPE *pComp;
    OMX_STATETYPE eState;

    if (hComponent == NULL)
        return OMX_ErrorBadParameter;

    eError = OMX_GetState(hComponent, &eState);
    if (eError != OMX_ErrorNone)
        return eError;
    else if (eState != OMX_StateInvalid && eState != OMX_StateLoaded)
        return OMX_ErrorInvalidState;

    pComp = (OMX_COMPONENTTYPE*)hComponent;
    eError = TTCDeInit(hComponent);
    if (eError == OMX_ErrorNone) {
        OMX_OSAL_Free(pComp);
    }

    return eError;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */
