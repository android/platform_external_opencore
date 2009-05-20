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

/** OMX_CONF_TunnelComponent.h
 *  Definition of tunneled component. Useful for stress testing tunneling features of OpenMax
 */
#ifndef OMX_CONF_TunnelTestComponent_h
#define OMX_CONF_TunnelTestComponent_h

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include <OMX_Types.h>
#include <OMX_Component.h>
#include <OMX_Core.h>
#include <OMX_Index.h>
#include <OMX_Image.h>
#include <OMX_Audio.h>
#include <OMX_Video.h>
#include <OMX_IVCommon.h>
#include <OMX_Other.h>

#include "OMX_OSAL_Interfaces.h"

    /* todo: dynamically allocate ports and buffer pointers instead of statically allocating them */
#define MAX_TTCPORTS 100
#define MAX_TTCBUFFERS 256

    /* Tunnel Test Component Port Context */
    typedef struct TTCPORTTYPE
    {
        OMX_HANDLETYPE hTunnelComponent;
        OMX_U32 nTunnelPort;
        OMX_BUFFERSUPPLIERTYPE eSupplierSetting;
        OMX_BUFFERSUPPLIERTYPE eSupplierPreference;
        OMX_U32 nPortIndex;
        OMX_BOOL bEOS;

        /* buffer info */
        OMX_U32 nPreferredCount;
        OMX_U32 nPreferredSize;
        OMX_U32 nBufferCount;
        OMX_U32 nBufferSize;
        OMX_U8 *pBuffer[MAX_TTCBUFFERS];
        OMX_BUFFERHEADERTYPE *pBufferHdr[MAX_TTCBUFFERS];

        OMX_INDEXTYPE nPortDefParamIndex;

        OMX_PARAM_PORTDEFINITIONTYPE oPortDef;

        OMX_BOOL bHoldingBuffer[MAX_TTCBUFFERS];

        OMX_U32 nMinBytes;
        OMX_U32 nPlaneBytesEmitted;
        OMX_U32 nPlaneBytesTotal;

    } TTCPORTTYPE;

    /** Tunnel Test Component Context */
    typedef struct TTCDATATYPE
    {
        OMX_STATETYPE eState;
        TTCPORTTYPE oPort[MAX_TTCPORTS];
        OMX_U32 nUsedPorts;
        OMX_CALLBACKTYPE *pCallbacks;
        OMX_PTR pAppData;
        OMX_ERRORTYPE(*OnEmptyThisBuffer)(OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);
        OMX_ERRORTYPE(*OnFillThisBuffer)(OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer);
        OMX_ERRORTYPE(*OnInvalidPayloadSize)(OMX_IN OMX_U32 nOutPort, OMX_IN OMX_U32 nMin, OMX_IN OMX_U32 nActual);
        OMX_BOOL bCreateInvalidPorts;
        OMX_BOOL bDontSupportIndexParamCompBufferSupplier;
        OMX_BOOL bInvertBufferSupplier;
        OMX_BOOL bDontDoUseBuffer;
        OMX_BOOL bHoldBuffers;
        OMX_HANDLETYPE hHoldingBuffersEvent;

        OMX_HANDLETYPE hBufferCountEvent;
        OMX_U32 nBuffersLeft;
        OMX_HANDLETYPE hMutex;
    } TTCDATATYPE;

#define TTC_RETURN_ANY_ERROR(__X) \
{ \
    OMX_ERRORTYPE __eErr; \
    if (OMX_ErrorNone != (__eErr = (__X))) \
        return __eErr; \
}

    /* Sets the Tunnel Test Component's OnEmptyThisBuffer callback. If this callback is non-NULL
     * the TTC calls it on a EmptyThisBuffer calls */
    OMX_ERRORTYPE OMX_CONF_SetTTCOnEmptyThisBuffer(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_ERRORTYPE(*OnEmptyThisBuffer)(OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
    );

    /* Sets the Tunnel Test Component's OnFillThisBuffer callback. If this callback is non-NULL
     * the TTC calls it on a OnFillThisBuffer calls */
    OMX_ERRORTYPE OMX_CONF_SetTTCOnFillThisBuffer(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_ERRORTYPE(*OnFillThisBuffer)(OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
    );

    /* Sets the Tunnel Test Components OnInvalidPayloadSize callback. If this callback is non-NULL
     * the TTC checks the payload size on EmptyThisBuffer calls. If the size is invalid it calls this
     * function. */
    OMX_ERRORTYPE OMX_CONF_SetTTCOnInvalidPayloadSize(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_ERRORTYPE(*OnInvalidPayloadSize)(OMX_IN OMX_U32 nOutPort, OMX_IN OMX_U32 nMin, OMX_IN OMX_U32 nActual)
    );

    /* Connects a single port of any domain type from a component under test to the tunnel test component. The
     * TTC will query and clone the CUT port (flipping direction). */
    OMX_ERRORTYPE TTCConnectPort(
        OMX_HANDLETYPE hTTC,
        OMX_HANDLETYPE hCUT,
        OMX_U32 iCUTPort);

    /* Connects all of the component under test's ports for all domains to the tunnel test component. The
     * TTC will query and clone all CUT ports (flipping direction). */
    OMX_ERRORTYPE OMX_CONF_TTCConnectAllPorts(
        OMX_IN  OMX_HANDLETYPE hTunnelTestComponent,
        OMX_IN  OMX_HANDLETYPE hComponentUnderTest);

    /* Connects all of the component under test's audio ports to the tunnel test component. The
     * TTC will query and clone all CUT ports (flipping direction). */
    OMX_ERRORTYPE OMX_CONF_TTCConnectAudioPorts(
        OMX_IN  OMX_HANDLETYPE hTunnelTestComponent,
        OMX_IN  OMX_HANDLETYPE hComponentUnderTest);
    /* Connects all of the component under test's video ports to the tunnel test component. The
     * TTC will query and clone all CUT ports (flipping direction). */
    OMX_ERRORTYPE OMX_CONF_TTCConnectVideoPorts(
        OMX_IN  OMX_HANDLETYPE hTunnelTestComponent,
        OMX_IN  OMX_HANDLETYPE hComponentUnderTest);
    /* Connects all of the component under test's image ports to the tunnel test component. The
     * TTC will query and clone all CUT ports (flipping direction). */
    OMX_ERRORTYPE OMX_CONF_TTCConnectImagePorts(
        OMX_IN  OMX_HANDLETYPE hTunnelTestComponent,
        OMX_IN  OMX_HANDLETYPE hComponentUnderTest);
    /* Connects all of the component under test's otheer ports to the tunnel test component. The
     * TTC will query and clone all CUT ports (flipping direction). */
    OMX_ERRORTYPE OMX_CONF_TTCConnectOtherPorts(
        OMX_IN  OMX_HANDLETYPE hTunnelTestComponent,
        OMX_IN  OMX_HANDLETYPE hComponentUnderTest);

    /* Disconnect all of the ports connected with above: effectively resets for another test */
    OMX_ERRORTYPE TTCDisconnectAllPorts(OMX_IN  OMX_HANDLETYPE hTunnelTestComponent);

    /* Tells the 'ConnectXPort' functions to select an invalid encoding type (instead of a matching one) */
    OMX_ERRORTYPE TTCCreateInvalidPortTypes(
        OMX_IN  OMX_HANDLETYPE hTTC,
        OMX_IN  OMX_U8 bCreateInvalid);

    /* Makes the TTC not support IndexParamCompBufferSupplier */
    OMX_ERRORTYPE TTCDontSupportIndexParamCompBufferSupplier(
        OMX_IN  OMX_HANDLETYPE hTTC,
        OMX_IN  OMX_U8 bSupportIndex);

    /* Makes the TTC input ports pick the opposite buffer supplier from the output ports */
    OMX_ERRORTYPE TTCInvertBufferSupplier(
        OMX_IN  OMX_HANDLETYPE hTTC,
        OMX_IN  OMX_U8 bSupportIndex);

    /* Makes the TTC's UseBuffer implementation fail */
    OMX_ERRORTYPE TTCDontSupportUseBuffer(
        OMX_IN  OMX_HANDLETYPE hTTC,
        OMX_IN  OMX_U8 bSupportUseBuffer);

    /* Freeze processing and hold at least one buffer */
    OMX_ERRORTYPE TTCHoldBuffers(OMX_IN  OMX_HANDLETYPE hTunnelTestComponent);

    /* Unfreeze processing and release all held buffers*/
    OMX_ERRORTYPE TTCReleaseBuffers(OMX_IN  OMX_HANDLETYPE hTunnelTestComponent);

    /* Wait for some some buffers to be exchanged */
    OMX_ERRORTYPE OMX_CONF_WaitForBufferTraffic(OMX_IN OMX_HANDLETYPE hComponent);

    /* TTC GetHandle and FreeHandle functions exposed here so that solutions that do not statically link
       components may still create and destroy the Tunnel Test Component */
    OMX_ERRORTYPE OMX_CONF_GetTunnelTestComponentHandle(
        OMX_OUT OMX_HANDLETYPE *pHandle,
        OMX_IN  OMX_PTR pAppData,
        OMX_IN  OMX_CALLBACKTYPE* pCallBacks);
    OMX_ERRORTYPE OMX_CONF_FreeTunnelTestComponentHandle(
        OMX_IN OMX_HANDLETYPE hComponent);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* OMX_CONF_TunnelTestComponent_h */

/* File EOF */


