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

/** OMX_CONF_PortBufferSupplierTest.c
 *  OpenMax IL conformance test validating buffer supplying behavior on input ports
 */

#include <stdlib.h>
#include <string.h>
#include "OMX_OSAL_Interfaces.h"
#include "OMX_CONF_TestHarness.h"
#include "OMX_CONF_StubbedCallbacks.h"
#include "OMX_CONF_TunnelTestComponent.h"

typedef struct TestContext
{
    OMX_U32 nPorts;

    OMX_HANDLETYPE hComp;
    OMX_HANDLETYPE hTTComp;
    OMX_STATETYPE eStateComp;
    OMX_STATETYPE eStateTTComp;
    OMX_HANDLETYPE hStateChangeEvent;
    OMX_HANDLETYPE hMutex;

    OMX_HANDLETYPE hOrigComp;
    OMX_HANDLETYPE hOrigTTComp;

    OMX_HANDLETYPE hCUT;
} TestContext;

#define MAX_PORTS 128
#define HASH_PORT(_x_) ((_x_) % MAX_PORTS)

OMX_U32 g_nBuffersTotal;
OMX_U32 g_nBuffers[MAX_PORTS];
OMX_U32 g_nUseBufferCallsTotal;
OMX_U32 g_nUseBufferCalls[MAX_PORTS];
OMX_U32 g_nFreeBufferCallsTotal;
OMX_U32 g_nFreeBufferCalls[MAX_PORTS];
OMX_U32 g_nPortIndex[MAX_PORTS];
OMX_COMPONENTTYPE g_oOrigTTC;


static OMX_ERRORTYPE
Test_UseBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
    OMX_IN OMX_U32 nPortIndex,
    OMX_IN OMX_PTR pAppPrivate,
    OMX_IN OMX_U32 nSizeBytes,
    OMX_IN OMX_U8* pBuffer)
{
    OMX_ERRORTYPE eError = g_oOrigTTC.UseBuffer(hComponent, ppBufferHdr, nPortIndex, pAppPrivate, nSizeBytes, pBuffer);
    if (eError == OMX_ErrorNone || eError == OMX_ErrorNotImplemented)
    {
        ++g_nUseBufferCallsTotal;
        ++g_nUseBufferCalls[HASH_PORT(nPortIndex)];
    }

    return eError;
}

static OMX_ERRORTYPE
Test_FreeBuffer(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_U32 nPortIndex,
    OMX_INOUT OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    OMX_ERRORTYPE eError = g_oOrigTTC.FreeBuffer(hComponent, nPortIndex, pBufferHdr);
    if (eError == OMX_ErrorNone || eError == OMX_ErrorNotImplemented)
    {
        ++g_nFreeBufferCallsTotal;
        ++g_nFreeBufferCalls[HASH_PORT(nPortIndex)];
    }
    return eError;
}

static OMX_ERRORTYPE Test_EventHandler(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_PTR pAppData,
    OMX_IN OMX_EVENTTYPE eEvent,
    OMX_IN OMX_U32 nData1,
    OMX_IN OMX_U32 nData2,
    OMX_IN OMX_PTR pEventData)
{
    TestContext* pContext = pAppData;

    UNUSED_PARAMETER(nData2);
    UNUSED_PARAMETER(pEventData);

    if (hComponent != pContext->hCUT)
    {
        return OMX_ErrorNone;
    }

    if ((eEvent == OMX_EventCmdComplete) && ((OMX_COMMANDTYPE)(nData1) == OMX_CommandStateSet))
    {
        OMX_GetState(pContext->hComp, &pContext->eStateComp);
        OMX_GetState(pContext->hTTComp, &pContext->eStateTTComp);
        OMX_OSAL_EventSet(pContext->hStateChangeEvent);
    }
    return OMX_ErrorNone;
}

static OMX_ERRORTYPE Test_ChangeState(TestContext* pContext, OMX_STATETYPE eState)
{
    OMX_ERRORTYPE  eError = OMX_ErrorNone;
    OMX_BOOL bDone, bTimedOut;

    OMX_OSAL_MutexLock(pContext->hMutex);
    OMX_GetState(pContext->hComp, &pContext->eStateComp);
    OMX_GetState(pContext->hTTComp, &pContext->eStateTTComp);
    OMX_OSAL_MutexUnlock(pContext->hMutex);

    if (eState != pContext->eStateComp)
    {
        eError = OMX_SendCommand(pContext->hComp, OMX_CommandStateSet, eState, NULL);
        OMX_CONF_FAIL_IF_ERROR(eError);
    }

    if (eState != pContext->eStateTTComp)
    {
        eError = OMX_SendCommand(pContext->hTTComp, OMX_CommandStateSet, eState, NULL);
        OMX_CONF_FAIL_IF_ERROR(eError);
    }

    OMX_OSAL_EventReset(pContext->hStateChangeEvent);
    OMX_OSAL_MutexLock(pContext->hMutex);
    OMX_GetState(pContext->hComp, &pContext->eStateComp);
    OMX_GetState(pContext->hTTComp, &pContext->eStateTTComp);
    OMX_OSAL_MutexUnlock(pContext->hMutex);

    bDone = OMX_FALSE;
    bTimedOut = OMX_FALSE;
    do
    {
        OMX_OSAL_MutexLock(pContext->hMutex);
        bDone = (pContext->eStateComp == eState && pContext->eStateTTComp == eState);
        OMX_OSAL_MutexUnlock(pContext->hMutex);
        if (!bDone)
        {
            OMX_OSAL_EventWait(pContext->hStateChangeEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimedOut);
            OMX_OSAL_EventReset(pContext->hStateChangeEvent);
        }
    }
    while (!bDone && !bTimedOut);
    if (bTimedOut)
    {
        OMX_GetState(pContext->hComp, &pContext->eStateComp);
        OMX_GetState(pContext->hTTComp, &pContext->eStateTTComp);
        bDone = (pContext->eStateComp == eState && pContext->eStateTTComp == eState);
    }
    if (bDone)
        return OMX_ErrorNone;
    else
        return OMX_ErrorTimeout;

OMX_CONF_TEST_FAIL:
    return eError;
}

static OMX_ERRORTYPE Test_SetupOnePort(TestContext *pContext, OMX_HANDLETYPE hComp, int nPort, OMX_HANDLETYPE hTTComp,
                                       OMX_BUFFERSUPPLIERTYPE eSupplier)
{
    OMX_ERRORTYPE  eError = OMX_ErrorNone;
    OMX_PARAM_BUFFERSUPPLIERTYPE oSupplier;
    OMX_DIRTYPE eDir;
    OMX_BOOL bTTCompIsSupplier;
    OMX_PARAM_PORTDEFINITIONTYPE oPort;
    int nBuffers = 0;
    int nPortHash = HASH_PORT(pContext->nPorts);

    INIT_PARAM(oPort);
    oPort.nPortIndex = nPort;
    eError = OMX_GetParameter(hComp, OMX_IndexParamPortDefinition, &oPort);
    OMX_CONF_FAIL_IF_ERROR(eError);

    eDir = oPort.eDir;
    nBuffers = oPort.nBufferCountMin;

    TTCInvertBufferSupplier(&g_oOrigTTC, 0);
    eError = TTCConnectPort(&g_oOrigTTC, hComp, nPort);
    OMX_CONF_FAIL_IF_ERROR(eError);

    INIT_PARAM(oSupplier);
    oSupplier.eBufferSupplier = eSupplier;

    if (eDir == OMX_DirOutput)
    {
        /* set up tunnel with CUT output to TTC input */
        eError = OMX_SetupTunnel(hComp, nPort, hTTComp, pContext->nPorts);
        OMX_CONF_FAIL_IF_ERROR(eError);

        oSupplier.nPortIndex = pContext->nPorts;
        eError = OMX_SetParameter(hTTComp, OMX_IndexParamCompBufferSupplier, &oSupplier);
        OMX_CONF_FAIL_IF_ERROR(eError);

        bTTCompIsSupplier = (eSupplier == OMX_BufferSupplyInput);
    }
    else
    {
        /* set up tunnel with TTC output to CUT input */
        eError = OMX_SetupTunnel(hTTComp, pContext->nPorts, hComp, nPort);
        OMX_CONF_FAIL_IF_ERROR(eError);

        oSupplier.nPortIndex = nPort;
        eError = OMX_SetParameter(hComp, OMX_IndexParamCompBufferSupplier, &oSupplier);
        OMX_CONF_FAIL_IF_ERROR(eError);

        bTTCompIsSupplier = (eSupplier == OMX_BufferSupplyOutput);
    }

    if (bTTCompIsSupplier)
    {
        g_nBuffers[nPortHash] = 0;
    }
    else
    {
        g_nBuffers[nPortHash] += nBuffers;
        g_nBuffersTotal += nBuffers;
    }
    g_nUseBufferCalls[nPortHash] = 0;
    g_nFreeBufferCalls[nPortHash] = 0;
    g_nPortIndex[nPortHash] = nPort;
OMX_CONF_TEST_FAIL:
    return eError;
}
static OMX_ERRORTYPE Test_SetupPortsForDomain(
    TestContext *pContext,
    const char * pszDomain,
    OMX_INDEXTYPE nInitIndex,
    OMX_BUFFERSUPPLIERTYPE eSupplier,
    OMX_INOUT OMX_U32 *pnPorts
)
{
    OMX_ERRORTYPE  eError = OMX_ErrorNone;
    OMX_PORT_PARAM_TYPE oInit;
    int i;
    int nEndPortNumber;

    INIT_PARAM(oInit);
    eError = OMX_GetParameter(pContext->hComp, nInitIndex, &oInit);
    if (eError == OMX_ErrorNotImplemented)
        return OMX_ErrorNone;

    OMX_CONF_FAIL_IF_ERROR(eError);

    nEndPortNumber = oInit.nStartPortNumber + oInit.nPorts;
    for (i = oInit.nStartPortNumber; i < nEndPortNumber; i++)
    {
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Testing %s port %d\n", pszDomain, i);

        eError = Test_SetupOnePort(pContext, pContext->hComp, i, pContext->hTTComp, eSupplier);
        OMX_CONF_FAIL_IF_ERROR(eError);
        (*pnPorts)++;
    }

OMX_CONF_TEST_FAIL:
    return eError;
}


static OMX_ERRORTYPE Test_TestAllPorts(TestContext *pContext, OMX_BUFFERSUPPLIERTYPE eSupplier)
{
    OMX_ERRORTYPE eError;
    int nPortHash;
    OMX_HANDLETYPE hTTComp = pContext->hTTComp;

    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Connecting ports\n");
    pContext->nPorts = 0;
    g_nBuffersTotal = 0;
    g_nUseBufferCallsTotal = 0;
    g_nFreeBufferCallsTotal = 0;
    for (nPortHash = 0; nPortHash < MAX_PORTS; nPortHash++)
        g_nBuffers[nPortHash] = 0;

    eError = Test_SetupPortsForDomain(pContext, "audio", OMX_IndexParamAudioInit, eSupplier, &pContext->nPorts);
    OMX_CONF_FAIL_IF_ERROR(eError);
    eError = Test_SetupPortsForDomain(pContext, "video", OMX_IndexParamVideoInit, eSupplier, &pContext->nPorts);
    OMX_CONF_FAIL_IF_ERROR(eError);
    eError = Test_SetupPortsForDomain(pContext, "image", OMX_IndexParamImageInit, eSupplier, &pContext->nPorts);
    OMX_CONF_FAIL_IF_ERROR(eError);
    eError = Test_SetupPortsForDomain(pContext, "other", OMX_IndexParamOtherInit, eSupplier, &pContext->nPorts);
    OMX_CONF_FAIL_IF_ERROR(eError);

    OMX_CONF_FAIL_IF(pContext->nPorts <= 0);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Changing state to idle (%d ports found)\n", pContext->nPorts);
    eError = Test_ChangeState(pContext, OMX_StateIdle);
    OMX_CONF_FAIL_IF_ERROR(eError);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Checking number of UseBuffer calls\n");
    for (nPortHash = 0; nPortHash < MAX_PORTS; nPortHash++)
    {
        OMX_CONF_FAIL_IF_NEQ(g_nUseBufferCalls[nPortHash], g_nBuffers[nPortHash]);
    }
    OMX_CONF_FAIL_IF_NEQ(g_nUseBufferCallsTotal, g_nBuffersTotal);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Changing state to loaded\n");
    eError = Test_ChangeState(pContext, OMX_StateLoaded);
    OMX_CONF_FAIL_IF_ERROR(eError);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Checking number of FreeBuffer calls\n");
    for (nPortHash = 0; nPortHash < MAX_PORTS; nPortHash++)
    {
        OMX_CONF_FAIL_IF_NEQ(g_nFreeBufferCalls[nPortHash], g_nBuffers[nPortHash]);
    }
    OMX_CONF_FAIL_IF_NEQ(g_nFreeBufferCallsTotal, g_nBuffersTotal);
    eError = OMX_ErrorNone;

OMX_CONF_TEST_FAIL:
    Test_ChangeState(pContext, OMX_StateLoaded);
    TTCDisconnectAllPorts(hTTComp);
    return eError;
}

OMX_ERRORTYPE OMX_CONF_PortBufferSupplierTest(OMX_IN OMX_STRING cComponentName)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;
    OMX_ERRORTYPE eCleanupError = OMX_ErrorNone;
    OMX_HANDLETYPE hComp = 0, hWrappedComp = 0, hTTComp = 0, hWrappedTTComp = 0;
    OMX_CALLBACKTYPE oCallbacks;

    TestContext oAppData;
    OMX_CALLBACKTYPE *pWrappedCallbacks;
    OMX_PTR pWrappedAppData;
    TestContext* pContext = &oAppData;

    /* create state change event */
    OMX_OSAL_EventCreate(&oAppData.hStateChangeEvent);
    OMX_OSAL_EventReset(oAppData.hStateChangeEvent);
    OMX_OSAL_MutexCreate(&oAppData.hMutex);

    oCallbacks.EventHandler    = Test_EventHandler;
    oCallbacks.EmptyBufferDone = StubbedEmptyBufferDone;
    oCallbacks.FillBufferDone  = StubbedFillBufferDone;
    eError = OMX_CONF_CallbackTracerCreate(&oCallbacks, (OMX_PTR) & oAppData, cComponentName,
                                           &pWrappedCallbacks, &pWrappedAppData);

    /* Initialize OpenMax */
    eError = OMX_Init();

    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Creating component under test and tunnel test component.\n");

    /* Acquire component under test handle */
    eError = OMX_GetHandle(&hComp, cComponentName, pWrappedAppData, pWrappedCallbacks);
    OMX_CONF_FAIL_IF_ERROR(eError);
    eError = OMX_CONF_ComponentTracerCreate(hComp, cComponentName, &hWrappedComp);
    OMX_CONF_FAIL_IF_ERROR(eError);
    oAppData.hCUT = hComp;

    /* Acquire tunnel test component handle */
    eError = OMX_CONF_GetTunnelTestComponentHandle(&hTTComp, pWrappedAppData, pWrappedCallbacks);
    OMX_CONF_FAIL_IF_ERROR(eError);

    /* remember entry points for original tunnel test component and replace ones we want to monitor */
    memcpy(&g_oOrigTTC, hTTComp, sizeof(OMX_COMPONENTTYPE));
    ((OMX_COMPONENTTYPE *)hTTComp)->UseBuffer = Test_UseBuffer;
    ((OMX_COMPONENTTYPE *)hTTComp)->FreeBuffer = Test_FreeBuffer;

    /* now wrap the result */
    eError = OMX_CONF_ComponentTracerCreate(hTTComp, "OMX.CONF.tunnel.test", &hWrappedTTComp);
    OMX_CONF_FAIL_IF_ERROR(eError);

    pContext->hOrigComp = hComp;
    pContext->hOrigTTComp = hTTComp;
    pContext->hComp = hWrappedComp;
    pContext->hTTComp = hWrappedTTComp;

    OMX_GetState(hComp, &pContext->eStateComp);
    OMX_GetState(hTTComp, &pContext->eStateTTComp);

    eError = Test_TestAllPorts(pContext, OMX_BufferSupplyInput);
    OMX_CONF_FAIL_IF_ERROR(eError);

    eError = Test_TestAllPorts(pContext, OMX_BufferSupplyOutput);
    OMX_CONF_FAIL_IF_ERROR(eError);

OMX_CONF_TEST_FAIL:
    /* Cleanup: Return function errors rather than closing errors if appropriate */

    if (hWrappedComp)
    {
        eCleanupError = OMX_CONF_ComponentTracerDestroy(hWrappedComp);
        if (eError == OMX_ErrorNone) eError = eCleanupError;
    }

    if (hComp)
    {
        eCleanupError = OMX_FreeHandle(hComp);
        if (eError == OMX_ErrorNone) eError = eCleanupError;
    }

    if (hWrappedTTComp)
    {
        eCleanupError = OMX_CONF_ComponentTracerDestroy(hWrappedTTComp);
        if (eError == OMX_ErrorNone) eError = eCleanupError;
    }

    if (hTTComp)
    {
        eCleanupError = OMX_CONF_FreeTunnelTestComponentHandle(hTTComp);
        if (eError == OMX_ErrorNone) eError = eCleanupError;
    }

    eCleanupError = OMX_CONF_CallbackTracerDestroy(pWrappedCallbacks, pWrappedAppData);
    if (eError == OMX_ErrorNone) eError = eCleanupError;

    /* destroy state change event */
    OMX_OSAL_MutexDestroy(oAppData.hMutex);
    OMX_OSAL_EventDestroy(oAppData.hStateChangeEvent);


    eCleanupError = OMX_Deinit();
    if (eError == OMX_ErrorNone) eError = eCleanupError;

    return eError;

}


