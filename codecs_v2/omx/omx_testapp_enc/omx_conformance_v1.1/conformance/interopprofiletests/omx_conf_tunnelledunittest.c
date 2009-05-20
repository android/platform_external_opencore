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

/** OMX_CONF_TunnelledUnitTest.c
 *
 *  Utility test that runs the component under test until all appropriate end
 *  of stream flags are returned. This test is useful for unit testing a
 *  a component. The user must specify one input file per input port and
 *  may specify as many as one output file per output port (for capturing)
 *  the results of the component's transformation.
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "OMX_OSAL_Interfaces.h"
#include "OMX_CONF_TestHarness.h"
#include "OMX_CONF_StubbedCallbacks.h"
#include "OMX_CONF_TunnelTestComponent.h"

    /* callback data */
    typedef struct TUTDATATYPE
    {
        OMX_STATETYPE eState;
        OMX_HANDLETYPE hStateChangeEvent;
        OMX_HANDLETYPE hEOSEvent;
        OMX_HANDLETYPE hCUT;
        OMX_U32 nTotalStreams;
        OMX_U32 nFinishedStreams;
    } TUTDATATYPE;

    /* Tunnelled Unit Test's implementation of OMX_CALLBACKTYPE.EventHandler */
    OMX_ERRORTYPE TUTEventHandler(
        OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_EVENTTYPE eEvent,
        OMX_IN OMX_U32 nData1,
        OMX_IN OMX_U32 nData2,
        OMX_IN OMX_PTR pEventData)
    {
        TUTDATATYPE* pContext = pAppData;

        UNUSED_PARAMETER(pEventData);

        if (hComponent != pContext->hCUT)
        {
            return OMX_ErrorNone;
        }

        if ((OMX_EventCmdComplete == eEvent) && (OMX_CommandStateSet == nData1))
        {
            pContext->eState = (OMX_STATETYPE)(nData2);
            OMX_OSAL_EventSet(pContext->hStateChangeEvent);
        }
        else if (OMX_EventBufferFlag == eEvent)
        {
            pContext->nFinishedStreams++;
            if (pContext->nFinishedStreams == pContext->nTotalStreams)
            {
                OMX_OSAL_EventSet(pContext->hEOSEvent);
            }
        }

        return OMX_ErrorNone;
    }

    /* Wait for the Component Under Test to change to state and confirm
       it is the one we expect */
    OMX_ERRORTYPE TUTWaitForState(TUTDATATYPE *pAppData, OMX_STATETYPE eState)
    {
        OMX_BOOL bTimedOut = OMX_FALSE;

        OMX_OSAL_EventWait(pAppData->hStateChangeEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimedOut);
        if (bTimedOut || pAppData->eState != eState)
        {
            return OMX_ErrorUndefined;
        }
        return OMX_ErrorNone;
    }

    /* Wait for the Component Under Test to finish processing all streams */
    OMX_ERRORTYPE TUTWaitForEOS(TUTDATATYPE *pAppData)
    {
        OMX_BOOL bTimedOut = OMX_FALSE;

        OMX_OSAL_EventWait(pAppData->hEOSEvent, 100*1000, &bTimedOut);
        if (bTimedOut)
        {
            return OMX_ErrorUndefined;
        }
        return OMX_ErrorNone;
    }

    OMX_ERRORTYPE TUTRun(OMX_HANDLETYPE *phWrappedTTComp, OMX_HANDLETYPE hWrappedComp, TUTDATATYPE *pAppData)
    {
        OMX_ERRORTYPE eError;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Transitioning both components to executing.\n");

        /* transition CUT to idle */
        OMX_OSAL_EventReset(pAppData->hStateChangeEvent);
        if (OMX_ErrorNone != (eError = OMX_SendCommand(hWrappedComp, OMX_CommandStateSet, OMX_StateIdle, 0))) return eError;

        /* transition TTC to idle */
        if (OMX_ErrorNone != (eError = OMX_SendCommand(*phWrappedTTComp, OMX_CommandStateSet, OMX_StateIdle, 0))) return eError;

        /* transition CUT to executing */
        if (OMX_ErrorNone != (eError = TUTWaitForState(pAppData, OMX_StateIdle))) return eError;
        OMX_OSAL_EventReset(pAppData->hStateChangeEvent);
        if (OMX_ErrorNone != (eError = OMX_SendCommand(hWrappedComp, OMX_CommandStateSet, OMX_StateExecuting, 0))) return eError;
        if (OMX_ErrorNone != (eError = TUTWaitForState(pAppData, OMX_StateExecuting))) return eError;

        /* transition TTC to executing  */
        if (OMX_ErrorNone != (eError = OMX_SendCommand(*phWrappedTTComp, OMX_CommandStateSet, OMX_StateExecuting, 0))) return eError;

        if (OMX_ErrorNone != (eError = TUTWaitForEOS(pAppData))) return eError;

        return eError;
    }

    OMX_U32 TUTPortsOfDomainAndDirection(TUTDATATYPE *pAppData,
                                         OMX_PORTDOMAINTYPE eDomain, OMX_DIRTYPE eDir)
    {
        OMX_ERRORTYPE  eError = OMX_ErrorNone;
        OMX_PARAM_PORTDEFINITIONTYPE oCUTPort;
        OMX_U32 i, nTotal = 0;
        OMX_PORT_PARAM_TYPE oParam;

        INIT_PARAM(oParam);
        INIT_PARAM(oCUTPort);

        /* query the component's ports */
        if (OMX_GetParameter(pAppData->hCUT, eDomain, &oParam))
        {
            return 0;
        }

        /* start with first port of domain */
        oCUTPort.nPortIndex = oParam.nStartPortNumber;

        /* for each discovered port */
        for (i = 0;i < oParam.nPorts;i++)
        {
            eError = OMX_GetParameter(pAppData->hCUT, OMX_IndexParamPortDefinition,
                                      &oCUTPort);
            if (eError) return eError;
            if (oCUTPort.eDir == eDir)
            {
                nTotal++;
            }
            oCUTPort.nPortIndex++;
        }

        return nTotal;
    }


    OMX_ERRORTYPE TUTGetTotalStreams(TUTDATATYPE *pAppData)
    {
        OMX_ERRORTYPE  eError = OMX_ErrorNone;

        /* let total streams = total output ports */
        pAppData->nTotalStreams =
            TUTPortsOfDomainAndDirection(pAppData, OMX_IndexParamAudioInit, OMX_DirOutput) +
            TUTPortsOfDomainAndDirection(pAppData, OMX_IndexParamVideoInit, OMX_DirOutput) +
            TUTPortsOfDomainAndDirection(pAppData, OMX_IndexParamImageInit, OMX_DirOutput) +
            TUTPortsOfDomainAndDirection(pAppData, OMX_IndexParamOtherInit, OMX_DirOutput);

        /* if no output ports then let total streams = total input ports */
        if (0 == pAppData->nTotalStreams)
        {
            pAppData->nTotalStreams =
                TUTPortsOfDomainAndDirection(pAppData, OMX_IndexParamAudioInit, OMX_DirInput) +
                TUTPortsOfDomainAndDirection(pAppData, OMX_IndexParamVideoInit, OMX_DirInput) +
                TUTPortsOfDomainAndDirection(pAppData, OMX_IndexParamImageInit, OMX_DirInput) +
                TUTPortsOfDomainAndDirection(pAppData, OMX_IndexParamOtherInit, OMX_DirInput);
        }

        return eError;
    }

    /* Main entrypoint into the Tunnelled Unit Test */
    OMX_ERRORTYPE OMX_CONF_TunnelledUnitTest(OMX_IN OMX_STRING cComponentName)
    {
        OMX_PTR pWrappedAppData;
        OMX_CALLBACKTYPE *pWrappedCallbacks;
        OMX_HANDLETYPE hComp, hWrappedComp, hTTComp, hWrappedTTComp;
        OMX_ERRORTYPE  eTemp, eError = OMX_ErrorNone;
        OMX_CALLBACKTYPE oCallbacks;
        TUTDATATYPE oAppData;

        /* create state change event */
        OMX_OSAL_EventCreate(&oAppData.hStateChangeEvent);
        OMX_OSAL_EventReset(oAppData.hStateChangeEvent);
        OMX_OSAL_EventCreate(&oAppData.hEOSEvent);
        OMX_OSAL_EventReset(oAppData.hEOSEvent);

        /* init component handles */
        hComp = hWrappedComp = hTTComp = hWrappedTTComp = 0;

        oCallbacks.EventHandler    = TUTEventHandler;
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
        oAppData.nFinishedStreams = 0;
        TUTGetTotalStreams(&oAppData);

        /* Acquire tunnel test component handle */
        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_GetTunnelTestComponentHandle(&hTTComp, pWrappedAppData, pWrappedCallbacks));
        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_ComponentTracerCreate(hTTComp, "OMX.CONF.tunnel.test", &hWrappedTTComp));

        /* Connect CUT to TTC */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Connecting all ports.\n");
        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_TTCConnectAllPorts(hWrappedTTComp, hWrappedComp));

        /* Force all CUT ports to be suppliers */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Forcing all component ports to be suppliers.\n");
        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_ForceSuppliers(hWrappedComp, OMX_TRUE));
        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_ForceSuppliers(hWrappedTTComp, OMX_FALSE));

        OMX_CONF_FAIL_IF_ERROR(TUTRun(&hWrappedTTComp, hWrappedComp, &oAppData));

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
            OMX_CONF_REMEMBER_ERROR(TUTWaitForState(&oAppData, OMX_StateIdle));
            OMX_OSAL_EventReset(oAppData.hStateChangeEvent);
            OMX_CONF_REMEMBER_ERROR(OMX_SendCommand(hWrappedComp, OMX_CommandStateSet, OMX_StateLoaded, 0));
            OMX_CONF_REMEMBER_ERROR(TUTWaitForState(&oAppData, OMX_StateLoaded));
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
