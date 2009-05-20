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

/** OMX_CONF_SeekTest.c
 *  OpenMax IL conformance test validating the proper behavior of a seeking component.
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "OMX_OSAL_Interfaces.h"
#include "OMX_CONF_TestHarness.h"
#include "OMX_CONF_StubbedCallbacks.h"
#include "OMX_CONF_TunnelTestComponent.h"

    OMX_BOOL g_StartTimeDetected;

    /* Callback data */
    typedef struct SCTDATATYPE
    {
        OMX_STATETYPE eState;
        OMX_HANDLETYPE hStateChangeEvent;
        OMX_HANDLETYPE hEOSEvent;
        OMX_HANDLETYPE hCUT;
    } SCTDATATYPE;

    /* Callback called when the Tunnel Test Component recieves a OnEmptythisBuffer call */
    OMX_ERRORTYPE SCTOnEmptyThisBuffer(OMX_BUFFERHEADERTYPE *pHdr)
    {
        if (pHdr->nFlags & OMX_BUFFERFLAG_STARTTIME) g_StartTimeDetected = OMX_TRUE;
        return OMX_ErrorNone;
    }

    /* Seek Component's implementation of OMX_CALLBACKTYPE.EventHandler */
    OMX_ERRORTYPE SCTEventHandler(
        OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_EVENTTYPE eEvent,
        OMX_IN OMX_U32 nData1,
        OMX_IN OMX_U32 nData2,
        OMX_IN OMX_PTR pEventData)
    {
        SCTDATATYPE* pContext = pAppData;

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

        if (eEvent == OMX_EventBufferFlag)
        {
            OMX_OSAL_EventSet(pContext->hEOSEvent);
        }
        return OMX_ErrorNone;
    }

    /* Wait for the Component Under Test to change to state and confirm it is the one we expect */
    OMX_ERRORTYPE SCTWaitForState(SCTDATATYPE *pAppData, OMX_STATETYPE eState)
    {
        OMX_BOOL bTimedOut = OMX_FALSE;

        OMX_OSAL_EventWait(pAppData->hStateChangeEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimedOut);
        if (bTimedOut || pAppData->eState != eState)
        {
            return bTimedOut ? OMX_ErrorTimeout : OMX_ErrorUndefined;
        }
        return OMX_ErrorNone;
    }

    /* Main entrypoint into the Seeking Component Test */
    OMX_ERRORTYPE OMX_CONF_SeekingComponentTest(OMX_IN OMX_STRING cComponentName)
    {
        OMX_TIME_CONFIG_SEEKMODETYPE oSeekMode;
        OMX_TIME_CONFIG_TIMESTAMPTYPE oTimeStamp;
        OMX_PTR pWrappedAppData;
        OMX_CALLBACKTYPE *pWrappedCallbacks;
        OMX_HANDLETYPE hComp, hWrappedComp, hTTComp, hWrappedTTComp;
        OMX_ERRORTYPE  eTemp, eError = OMX_ErrorNone;
        OMX_CALLBACKTYPE oCallbacks;
        SCTDATATYPE oAppData;

        /* create state change event */
        OMX_OSAL_EventCreate(&oAppData.hStateChangeEvent);
        OMX_OSAL_EventReset(oAppData.hStateChangeEvent);
        OMX_OSAL_EventCreate(&oAppData.hEOSEvent);
        OMX_OSAL_EventReset(oAppData.hEOSEvent);

        /* init component handles */
        hComp = hWrappedComp = hTTComp = hWrappedTTComp = 0;

        oCallbacks.EventHandler    = SCTEventHandler;
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

        /* Set the OnEmptyThisBuffer callback */
        OMX_CONF_SetTTCOnEmptyThisBuffer(hTTComp, SCTOnEmptyThisBuffer);

        /* Connect CUT to TTC */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Connecting all ports.\n");
        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_TTCConnectAllPorts(hWrappedTTComp, hWrappedComp));

        /* Force all CUT ports to be suppliers */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Forcing all component ports to be suppliers.\n");
        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_ForceSuppliers(hWrappedComp, OMX_TRUE));
        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_ForceSuppliers(hWrappedTTComp, OMX_FALSE));

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Transitioning to executing.\n");

        /* transition CUT to idle */
        OMX_CONF_FAIL_IF_ERROR(OMX_SendCommand(hWrappedComp, OMX_CommandStateSet, OMX_StateIdle, 0));

        /* transition TTC to idle */
        OMX_CONF_FAIL_IF_ERROR(OMX_SendCommand(hWrappedTTComp, OMX_CommandStateSet, OMX_StateIdle, 0));

        /* transition CUT to executing */
        OMX_CONF_FAIL_IF_ERROR(SCTWaitForState(&oAppData, OMX_StateIdle));
        OMX_OSAL_EventReset(oAppData.hStateChangeEvent);
        OMX_CONF_FAIL_IF_ERROR(OMX_SendCommand(hWrappedComp, OMX_CommandStateSet, OMX_StateExecuting, 0));
        OMX_CONF_FAIL_IF_ERROR(SCTWaitForState(&oAppData, OMX_StateExecuting));

        /* transition TTC to executing  */
        OMX_CONF_FAIL_IF_ERROR(OMX_SendCommand(hWrappedTTComp, OMX_CommandStateSet, OMX_StateExecuting, 0));

        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_WaitForBufferTraffic(hTTComp));

        /* Get/Set OMX_IndexConfigTimeSeekMode */
        INIT_CONFIG(oSeekMode);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Confirm support for OMX_IndexConfigTimeSeekMode\n");
        OMX_CONF_FAIL_IF_ERROR(OMX_GetConfig(hComp, OMX_IndexConfigTimeSeekMode, (OMX_PTR)&oSeekMode));
        oSeekMode.eType = OMX_TIME_SeekModeAccurate;
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeSeekMode, (OMX_PTR)&oSeekMode));

        /* Get OMX_IndexConfigTimePosition */
        INIT_CONFIG(oTimeStamp);
        oTimeStamp.nPortIndex = 0; /* ignored - but we clear it anyway */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Confirm support for OMX_IndexConfigTimePosition\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Get the current position (remember it for later).\n");
        OMX_CONF_FAIL_IF_ERROR(OMX_GetConfig(hComp, OMX_IndexConfigTimePosition, (OMX_PTR)&oTimeStamp));

        /* Set OMX_IndexConfigTimePosition */
        /* Wait for the position to move on and then return to it */
        g_StartTimeDetected = OMX_FALSE;
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Run for a while.\n");
        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_WaitForBufferTraffic(hTTComp));
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Seek to the remembered position.\n");
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimePosition, (OMX_PTR)&oTimeStamp));
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Run for a while - wait for seek (i.e. a OMX_BUFFERFLAG_STARTTIME).\n");
        OMX_CONF_WaitForBufferTraffic(hTTComp);

        if (g_StartTimeDetected)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--OK: Saw OMX_BUFFERFLAG_STARTTIME.\n");
        }
        else
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--ERROR: No OMX_BUFFERFLAG_STARTTIME.\n");
            eError = OMX_ErrorUndefined;
        }

OMX_CONF_TEST_FAIL:

        /* Cleanup: Return function errors rather than closing errors if appropriate */

        /* transition CUT and TTC to Loaded state */
        if (hWrappedComp)
        {
            OMX_OSAL_EventReset(oAppData.hStateChangeEvent);
            OMX_CONF_REMEMBER_ERROR(OMX_SendCommand(hWrappedComp, OMX_CommandStateSet, OMX_StateIdle, 0));
        }
        if (hWrappedTTComp)
        {
            OMX_CONF_REMEMBER_ERROR(OMX_SendCommand(hWrappedTTComp, OMX_CommandStateSet, OMX_StateIdle, 0));
            TTCReleaseBuffers(hTTComp);
        }
        if (hWrappedComp)
        {
            OMX_CONF_REMEMBER_ERROR(SCTWaitForState(&oAppData, OMX_StateIdle));
            OMX_OSAL_EventReset(oAppData.hStateChangeEvent);
            OMX_CONF_REMEMBER_ERROR(OMX_SendCommand(hWrappedComp, OMX_CommandStateSet, OMX_StateLoaded, 0));
            OMX_CONF_REMEMBER_ERROR(SCTWaitForState(&oAppData, OMX_StateLoaded));
        }
        if (hWrappedTTComp)
        {
            OMX_CONF_REMEMBER_ERROR(OMX_SendCommand(hWrappedTTComp, OMX_CommandStateSet, OMX_StateLoaded, 0));
        }

        /* destroy state change event */
        OMX_OSAL_EventDestroy(oAppData.hStateChangeEvent);
        OMX_OSAL_EventDestroy(oAppData.hEOSEvent);

        if (hWrappedComp)
        {
            OMX_CONF_REMEMBER_ERROR(OMX_CONF_ComponentTracerDestroy(hWrappedComp));
        }

        if (hComp)
        {
            OMX_CONF_REMEMBER_ERROR(OMX_FreeHandle(hComp));
        }

        if (hTTComp)
        {
            OMX_CONF_REMEMBER_ERROR(OMX_CONF_FreeTunnelTestComponentHandle(hTTComp));
            OMX_CONF_REMEMBER_ERROR(OMX_CONF_ComponentTracerDestroy(hWrappedTTComp));
        }

        OMX_CONF_REMEMBER_ERROR(OMX_CONF_CallbackTracerDestroy(pWrappedCallbacks, pWrappedAppData));
        OMX_CONF_REMEMBER_ERROR(OMX_Deinit());

        return eError;
    }

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */
