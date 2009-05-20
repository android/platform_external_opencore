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

/** OMX_CONF_MinPayloadSizeTest.c
 *  OpenMax IL conformance test validating that all output ports of a component abide by minimum payload size
 *  rules (where applicable).
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "OMX_OSAL_Interfaces.h"
#include "OMX_CONF_TestHarness.h"
#include "OMX_CONF_StubbedCallbacks.h"
#include "OMX_CONF_TunnelTestComponent.h"

    /* Call back data */
    typedef struct MPSTDATATYPE
    {
        OMX_STATETYPE eState;
        OMX_HANDLETYPE hStateChangeEvent;
        OMX_HANDLETYPE hEOSEvent;
        OMX_HANDLETYPE hCUT;
    } MPSTDATATYPE;

    /* Min Payload Size Test's implementation of OMX_CALLBACKTYPE.EventHandler */
    OMX_ERRORTYPE MPSTEventHandler(
        OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_EVENTTYPE eEvent,
        OMX_IN OMX_U32 nData1,
        OMX_IN OMX_U32 nData2,
        OMX_IN OMX_PTR pEventData)
    {
        MPSTDATATYPE* pContext = pAppData;

        UNUSED_PARAMETER(pEventData);

        if (hComponent != pContext->hCUT)
        {
            return OMX_ErrorNone;
        }

        if ((eEvent == OMX_EventCmdComplete) && ((OMX_COMMANDTYPE)(nData1) == OMX_CommandStateSet))
        {
            pContext->eState = (OMX_STATETYPE)(nData2);
            OMX_OSAL_EventSet(pContext->hStateChangeEvent);
        }

        if (eEvent == OMX_EventError && nData1 == OMX_ErrorInvalidState)
        {
            pContext->eState = OMX_StateInvalid;
            OMX_OSAL_EventSet(pContext->hStateChangeEvent);
        }

        if (eEvent == OMX_EventBufferFlag)
        {
            OMX_OSAL_EventSet(pContext->hEOSEvent);
        }
        return OMX_ErrorNone;
    }

    /* Wait for the Component Under Test to change to state and confirm it is the one we expect */
    OMX_ERRORTYPE MPSTWaitForState(MPSTDATATYPE *pAppData, OMX_STATETYPE eState)
    {
        OMX_BOOL bTimedOut = OMX_FALSE;

        OMX_OSAL_EventWait(pAppData->hStateChangeEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimedOut);
        if (bTimedOut || pAppData->eState != eState)
        {
            if (bTimedOut)
                return OMX_ErrorTimeout;
            else
                return OMX_ErrorUndefined;
        }
        return OMX_ErrorNone;
    }

    OMX_U32 g_EmptyThisBufferCalls;	/* counter of # of EmptyThisBufferCalls made to TTC */
    OMX_BOOL g_bInvalidSize;		/* set when an output send the TTC a payload of an invalid size */

    /* Callback for when the TTC detects an invalid payload size */
    OMX_ERRORTYPE MPSTOnInvalidPayloadSize(OMX_U32 nOutPort, OMX_U32 nMin, OMX_U32 nActual)
    {
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Insufficiently sized payload received from port %i.\n", nOutPort);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Expected at least 0x%08x bytes. Received 0x%08x bytes.\n", nMin, nActual);
        g_bInvalidSize = OMX_TRUE;

        return OMX_ErrorNone;
    }

    /* Main entrypoint into the Min Payload Size Test */
    OMX_ERRORTYPE OMX_CONF_MinPayloadSizeTest(OMX_IN OMX_STRING cComponentName)
    {
        OMX_PTR pWrappedAppData;
        OMX_CALLBACKTYPE *pWrappedCallbacks;
        OMX_HANDLETYPE hComp, hWrappedComp, hTTComp, hWrappedTTComp;
        OMX_ERRORTYPE  eTemp, eError = OMX_ErrorNone;
        OMX_CALLBACKTYPE oCallbacks;
        MPSTDATATYPE oAppData;
        OMX_U32 i;

        /* create state change event */
        for (i = 0;i < sizeof(oAppData);i++)((OMX_U8*)&oAppData)[i] = 0;
        OMX_OSAL_EventCreate(&oAppData.hStateChangeEvent);
        OMX_OSAL_EventReset(oAppData.hStateChangeEvent);
        OMX_OSAL_EventCreate(&oAppData.hEOSEvent);
        OMX_OSAL_EventReset(oAppData.hEOSEvent);

        /* init component handles */
        hComp = hWrappedComp = hTTComp = hWrappedTTComp = 0;

        oCallbacks.EventHandler    = MPSTEventHandler;
        oCallbacks.EmptyBufferDone = StubbedEmptyBufferDone;
        oCallbacks.FillBufferDone  = StubbedFillBufferDone;
        eError = OMX_CONF_CallbackTracerCreate(&oCallbacks, (OMX_PTR) & oAppData, cComponentName,
                                               &pWrappedCallbacks, &pWrappedAppData);

        /* Initialize OpenMax */
        eError = OMX_Init();

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Creating component under test and tunnel test component.\n");

        /* Acquire component under test handle */
        OMX_CONF_FAIL_IF_ERROR(OMX_GetHandle(&hComp, cComponentName, pWrappedAppData, pWrappedCallbacks));
        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_ComponentTracerCreate(hComp, cComponentName, &hWrappedComp));
        oAppData.hCUT = hComp;

        /* Acquire tunnel test component handle */
        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_GetTunnelTestComponentHandle(&hTTComp, pWrappedAppData, pWrappedCallbacks));
        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_ComponentTracerCreate(hTTComp, "OMX.CONF.tunnel.test", &hWrappedTTComp));

        g_bInvalidSize = OMX_FALSE;
        OMX_CONF_SetTTCOnInvalidPayloadSize(hTTComp, MPSTOnInvalidPayloadSize);

        /* Connect CUT to TTC */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Connecting all ports.\n");
        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_TTCConnectAllPorts(hWrappedTTComp, hWrappedComp));

        /* Force all CUT ports to be suppliers */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Forcing all CUT ports to be suppliers.\n");
        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_ForceSuppliers(hWrappedComp, OMX_TRUE));
        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_ForceSuppliers(hWrappedTTComp, OMX_FALSE));

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Transitioning both TTC and CUT to executing.\n");

        /* transition CUT to idle */
        OMX_CONF_FAIL_IF_ERROR(OMX_GetState(hWrappedComp, &oAppData.eState));
        OMX_CONF_FAIL_IF_ERROR(OMX_SendCommand(hWrappedComp, OMX_CommandStateSet, OMX_StateIdle, 0));

        /* transition TTC to idle */
        OMX_CONF_FAIL_IF_ERROR(OMX_SendCommand(hWrappedTTComp, OMX_CommandStateSet, OMX_StateIdle, 0));

        /* transition CUT to executing */
        OMX_CONF_FAIL_IF_ERROR(MPSTWaitForState(&oAppData, OMX_StateIdle));
        OMX_OSAL_EventReset(oAppData.hStateChangeEvent);
        OMX_CONF_FAIL_IF_ERROR(OMX_SendCommand(hWrappedComp, OMX_CommandStateSet, OMX_StateExecuting, 0));
        OMX_CONF_FAIL_IF_ERROR(MPSTWaitForState(&oAppData, OMX_StateExecuting));

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "TTC automatically outputs minimally sized buffer payloads.\n");

        /* transition TTC to executing  */
        OMX_CONF_FAIL_IF_ERROR(OMX_SendCommand(hWrappedTTComp, OMX_CommandStateSet, OMX_StateExecuting, 0));

        /* wait for at least 100 calls to EmptyThisBuffer or FillThisBuffer*/
        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_WaitForBufferTraffic(hTTComp));

        if (g_bInvalidSize)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-ERROR: Detected an incorrectly sized buffer payload.\n");
            eError = OMX_ErrorUndefined;
        }
        else
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-OK: All buffer payloads correctly sized.\n");
        }

OMX_CONF_TEST_FAIL:

        /* Cleanup: Return function errors rather than closing errors if appropriate */

        /* transition CUT and TTC to Loaded state */
        OMX_OSAL_EventReset(oAppData.hStateChangeEvent);
        if (hWrappedComp)
            OMX_CONF_REMEMBER_ERROR(OMX_SendCommand(hWrappedComp, OMX_CommandStateSet, OMX_StateIdle, 0));
        if (hWrappedTTComp)
        {
            OMX_CONF_REMEMBER_ERROR(OMX_SendCommand(hWrappedTTComp, OMX_CommandStateSet, OMX_StateIdle, 0));
            OMX_CONF_REMEMBER_ERROR(TTCReleaseBuffers(hTTComp));  /* release any buffers that ttc may be holding to allow CUT to go idle */
        }
        if (hWrappedComp)
        {
            OMX_CONF_REMEMBER_ERROR(MPSTWaitForState(&oAppData, OMX_StateIdle));
            OMX_OSAL_EventReset(oAppData.hStateChangeEvent);
            OMX_CONF_REMEMBER_ERROR(OMX_SendCommand(hWrappedComp, OMX_CommandStateSet, OMX_StateLoaded, 0));
        }
        if (hWrappedTTComp)
            OMX_CONF_REMEMBER_ERROR(OMX_SendCommand(hWrappedTTComp, OMX_CommandStateSet, OMX_StateLoaded, 0));
        if (hWrappedComp)
        {
            OMX_CONF_REMEMBER_ERROR(MPSTWaitForState(&oAppData, OMX_StateLoaded));
            if (OMX_GetState(hWrappedComp, &oAppData.eState) != OMX_ErrorNone || oAppData.eState != OMX_StateLoaded)
            {
                OMX_OSAL_EventReset(oAppData.hStateChangeEvent);
                OMX_CONF_REMEMBER_ERROR(OMX_SendCommand(hWrappedComp, OMX_CommandStateSet, OMX_StateInvalid, 0));
                OMX_CONF_REMEMBER_ERROR(MPSTWaitForState(&oAppData, OMX_StateInvalid));
            }
        }

        /* destroy state change event */
        OMX_OSAL_EventDestroy(oAppData.hStateChangeEvent);
        OMX_OSAL_EventDestroy(oAppData.hEOSEvent);

        if (hWrappedComp)
        {
            OMX_CONF_REMEMBER_ERROR(OMX_CONF_ComponentTracerDestroy(hWrappedComp));
        }
        if (hWrappedTTComp)
        {
            OMX_CONF_REMEMBER_ERROR(OMX_CONF_ComponentTracerDestroy(hWrappedTTComp));
        }

        if (hComp)
        {
            OMX_CONF_REMEMBER_ERROR(OMX_FreeHandle(hComp));
        }

        if (hTTComp)
        {
            OMX_CONF_REMEMBER_ERROR(OMX_CONF_FreeTunnelTestComponentHandle(hTTComp));
        }

        OMX_CONF_REMEMBER_ERROR(OMX_CONF_CallbackTracerDestroy(pWrappedCallbacks, pWrappedAppData));
        OMX_CONF_REMEMBER_ERROR(OMX_Deinit());

        return eError;
    }

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */
