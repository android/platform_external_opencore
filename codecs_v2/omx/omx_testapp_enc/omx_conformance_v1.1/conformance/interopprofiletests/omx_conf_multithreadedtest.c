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

/** OMX_CONF_MultiThreadedTest.c
 *  OpenMax IL conformance test validating the interoperability of components created from different
 *  threads.
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "OMX_OSAL_Interfaces.h"
#include "OMX_CONF_TestHarness.h"
#include "OMX_CONF_StubbedCallbacks.h"
#include "OMX_CONF_TunnelTestComponent.h"

    /* Callback data */
    typedef struct MTTDATATYPE
    {
        OMX_STATETYPE eState;
        OMX_HANDLETYPE hStateChangeEvent;
        OMX_HANDLETYPE hCUT;
    } MTTDATATYPE;

    /* Multi-Threaded Test's implementation of OMX_CALLBACKTYPE.EventHandler */
    OMX_ERRORTYPE MTTEventHandler(
        OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_EVENTTYPE eEvent,
        OMX_IN OMX_U32 nData1,
        OMX_IN OMX_U32 nData2,
        OMX_IN OMX_PTR pEventData)
    {
        MTTDATATYPE* pContext = pAppData;

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
        return OMX_ErrorNone;
    }

    /* Wait for the Component Under Test to change to state and confirm it is the one we expect */
    OMX_ERRORTYPE MTTWaitForState(MTTDATATYPE *pAppData, OMX_STATETYPE eState)
    {
        OMX_BOOL bTimedOut = OMX_FALSE;

        OMX_OSAL_EventWait(pAppData->hStateChangeEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimedOut);
        if (bTimedOut || pAppData->eState != eState)
        {
            return OMX_ErrorUndefined;
        }
        return OMX_ErrorNone;
    }

    /* globals to facilitate component creation in a separate thread */
    OMX_IN OMX_STRING g_cComponentName;
    OMX_CALLBACKTYPE *g_pWrappedCallbacks;
    OMX_PTR g_pWrappedAppData;
    OMX_HANDLETYPE g_hComp, g_hWrappedComp, g_hTTComp, g_hWrappedTTComp;
    OMX_HANDLETYPE g_CUTEvent, g_TTCEvent;
    OMX_ERRORTYPE g_eCUTError;
    OMX_ERRORTYPE g_eTTCError;

    /* create the component under test (used as a thread function) */
    OMX_U32 CreateCUT(OMX_PTR pParam)
    {
        UNUSED_PARAMETER(pParam);

        /* Acquire component under test handle */
        g_eCUTError = OMX_GetHandle(&g_hComp, g_cComponentName, g_pWrappedAppData, g_pWrappedCallbacks);
        if (g_eCUTError == OMX_ErrorNone)
        {
            g_eCUTError = OMX_CONF_ComponentTracerCreate(g_hComp, g_cComponentName, &g_hWrappedComp);
        }
        OMX_OSAL_EventSet(g_CUTEvent);
        return 0;
    }

    /* create the tunnel test component (used as a thread function) */
    OMX_U32 CreateTTC(OMX_PTR pParam)
    {
        UNUSED_PARAMETER(pParam);

        /* Acquire tunnel test component handle */
        g_eTTCError = OMX_CONF_GetTunnelTestComponentHandle(&g_hTTComp, g_pWrappedAppData, g_pWrappedCallbacks);
        if (g_eTTCError == OMX_ErrorNone)
        {
            g_eTTCError = OMX_CONF_ComponentTracerCreate(g_hTTComp, "OMX.CONF.tunnel.test", &g_hWrappedTTComp);
        }
        OMX_OSAL_EventSet(g_TTCEvent);
        return 0;
    }

    /* Main entrypoint into the Multi-Threaded Test */
    OMX_ERRORTYPE OMX_CONF_MultiThreadedTest(OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE  eTemp, eError = OMX_ErrorNone;
        OMX_CALLBACKTYPE oCallbacks;
        MTTDATATYPE oAppData;
        OMX_HANDLETYPE hCUTThread, hTTCThread;
        OMX_BOOL bTimedOut;

        g_cComponentName = cComponentName;

        /* setup synchronization objects */
        OMX_OSAL_EventCreate(&g_CUTEvent);
        OMX_OSAL_EventReset(g_CUTEvent);
        OMX_OSAL_EventCreate(&g_TTCEvent);
        OMX_OSAL_EventReset(g_TTCEvent);

        /* create state change event */
        OMX_OSAL_EventCreate(&oAppData.hStateChangeEvent);
        OMX_OSAL_EventReset(oAppData.hStateChangeEvent);

        /* init component handles */
        g_hComp = g_hWrappedComp = g_hTTComp = g_hWrappedTTComp = 0;

        oCallbacks.EventHandler    = MTTEventHandler;
        oCallbacks.EmptyBufferDone = StubbedEmptyBufferDone;
        oCallbacks.FillBufferDone  = StubbedFillBufferDone;
        eError = OMX_CONF_CallbackTracerCreate(&oCallbacks, (OMX_PTR) & oAppData, cComponentName,
                                               &g_pWrappedCallbacks, &g_pWrappedAppData);

        /* Initialize OpenMax */
        eError = OMX_Init();

        /* create two threads that will create the CUT and TTC respectively */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,
                       "Creating two threads that will create the component under test \n and tunnel test component respectively.\n");
        g_cComponentName = cComponentName;
        g_pWrappedCallbacks = g_pWrappedCallbacks;
        g_pWrappedAppData = g_pWrappedAppData;
        OMX_OSAL_ThreadCreate(CreateCUT, 0, 0, &hCUTThread);
        OMX_OSAL_ThreadCreate(CreateTTC, 0, 0, &hTTCThread);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Waiting for threads to instantiate components.\n");
        bTimedOut = OMX_FALSE;
        OMX_OSAL_EventWait(g_TTCEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimedOut);
        if (bTimedOut)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,
                           "Timeout waiting %i ms for thread creating TTC to finish.  Proceeding with test.\n",
                           OMX_CONF_TIMEOUT_EXPECTING_SUCCESS);
        }
        OMX_OSAL_EventWait(g_CUTEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimedOut);
        if (bTimedOut)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,
                           "Timeout waiting %i ms for thread creating TTC to finish.  Proceeding with test.\n",
                           OMX_CONF_TIMEOUT_EXPECTING_SUCCESS);
        }
        oAppData.hCUT = g_hComp;

        OMX_OSAL_ThreadDestroy(hCUTThread);
        OMX_OSAL_ThreadDestroy(hTTCThread);

        OMX_CONF_FAIL_IF_ERROR(g_eCUTError);
        OMX_CONF_FAIL_IF_ERROR(g_eTTCError);

        /* Connect CUT to TTC */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Connecting all ports and transitioning to idle.\n");
        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_TTCConnectAllPorts(g_hWrappedTTComp, g_hWrappedComp));

        /* Force all CUT ports to be suppliers */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Forcing all CUT ports to be suppliers.\n");
        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_ForceSuppliers(g_hWrappedComp, OMX_TRUE));
        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_ForceSuppliers(g_hWrappedTTComp, OMX_FALSE));

        /* transition CUT to idle */
        OMX_OSAL_EventReset(oAppData.hStateChangeEvent);
        OMX_CONF_FAIL_IF_ERROR(OMX_SendCommand(g_hWrappedComp, OMX_CommandStateSet, OMX_StateIdle, 0));

        /* transition TTC to idle */
        OMX_CONF_FAIL_IF_ERROR(OMX_SendCommand(g_hWrappedTTComp, OMX_CommandStateSet, OMX_StateIdle, 0));

        /* wait for transitions to complete */
        OMX_CONF_FAIL_IF_ERROR(MTTWaitForState(&oAppData, OMX_StateIdle));

OMX_CONF_TEST_FAIL:

        /* Cleanup: Return function errors rather than closing errors if appropriate */

        /* transition CUT to Loaded state */
        if (g_hWrappedComp)
        {
            OMX_OSAL_EventReset(oAppData.hStateChangeEvent);
            OMX_CONF_REMEMBER_ERROR(OMX_SendCommand(g_hWrappedComp, OMX_CommandStateSet, OMX_StateLoaded, 0));
        }

        /* transition TTC to Loaded state */
        if (g_hWrappedTTComp)
        {
            OMX_CONF_REMEMBER_ERROR(OMX_SendCommand(g_hWrappedTTComp, OMX_CommandStateSet, OMX_StateLoaded, 0));
        }

        if (g_hWrappedComp)
        {
            OMX_CONF_REMEMBER_ERROR(MTTWaitForState(&oAppData, OMX_StateLoaded));
        }

        /* destroy state change event */
        OMX_OSAL_EventDestroy(oAppData.hStateChangeEvent);

        OMX_CONF_REMEMBER_ERROR(OMX_CONF_CallbackTracerDestroy(g_pWrappedCallbacks, g_pWrappedAppData));

        if (g_hWrappedComp)
        {
            OMX_CONF_REMEMBER_ERROR(OMX_CONF_ComponentTracerDestroy(g_hWrappedComp));
        }

        /* clean up synchronization objects */
        OMX_OSAL_EventDestroy(g_CUTEvent);
        OMX_OSAL_EventDestroy(g_TTCEvent);

        if (g_hComp)
        {
            OMX_CONF_REMEMBER_ERROR(OMX_FreeHandle(g_hComp));
        }

        if (g_hTTComp)
        {
            OMX_CONF_REMEMBER_ERROR(OMX_CONF_FreeTunnelTestComponentHandle(g_hTTComp));
        }

        OMX_CONF_REMEMBER_ERROR(OMX_Deinit());

        return eError;
    }

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */
