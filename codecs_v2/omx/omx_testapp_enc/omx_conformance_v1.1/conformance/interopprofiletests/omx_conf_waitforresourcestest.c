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

/** OMX_CONF_WaitForResourcesTest.c
 *  OpenMax IL conformance test validating proper behavior when a component is waiting for resources.
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include <stdlib.h>
#include "OMX_OSAL_Interfaces.h"
#include "OMX_CONF_TestHarness.h"
#include "OMX_CONF_StubbedCallbacks.h"
#include "OMX_CONF_TunnelTestComponent.h"

#define OMX_RESOURCETEST_PRIORITY_LOW   100
#define OMX_RESOURCETEST_PRIORITY_HIGH  0

    OMX_ERRORTYPE OMX_CONF_ResourcePreemptionTest(OMX_IN OMX_STRING cComponentName);
    OMX_ERRORTYPE OMX_CONF_WaitForResourcesTest(OMX_IN OMX_STRING cComponentName);

    /* callback data */
    typedef struct OMXWFRTESTDATA_T
    {
        OMX_U32 nInstantiation;
        OMX_BOOL bErrorInsufficientResources;   /* Did we get an OMX_ErrorInsufficientResources? */
        OMX_HANDLETYPE hEventStateChange;       /* Waiting for components to go to idle or waiting state */
    } OMXWFRTESTDATA;

    /* convert a component name and instantation # to an instantiation name */
    void GetInstantiationName(OMX_STRING cComponentName, OMX_U32 nInstantation, OMX_STRING szInstantiationName)
    {
        char *pSrc, *pDst;

        pSrc = cComponentName, pDst = szInstantiationName;
        while (*pSrc != '\0')
        {
            *pDst = *pSrc;
            pSrc++;
            pDst++;
        }
        *pDst++ = '[';
        *pDst++ = (char)((int)'0' + (nInstantation / 100));
        nInstantation -= (nInstantation / 100) * 100;
        *pDst++ = (char)((int)'0' + (nInstantation / 10));
        nInstantation -= (nInstantation / 10) * 10;
        *pDst++ = (char)((int)'0' + nInstantation);
        *pDst++ = ']';
        *pDst++ = '\0';
    }

    /* event that is signaled when the component changes state to idle */
    OMX_HANDLETYPE hEventStateChangeIdle;
    OMX_HANDLETYPE hEventStateChangeLoaded;

    static OMX_ERRORTYPE DisableAllPorts(OMX_HANDLETYPE hComp)
    {
        OMX_PORT_PARAM_TYPE          oPortInit;
        OMX_ERRORTYPE eError = OMX_ErrorUndefined;
        OMX_U32 i, d;


        INIT_PARAM(oPortInit);

        for (d = 0; d < 4; d++)
        {
            switch (d)
            {
                case 0:
                    eError = OMX_GetParameter(hComp, OMX_IndexParamOtherInit, &oPortInit);
                    break;
                case 1:
                    eError = OMX_GetParameter(hComp, OMX_IndexParamAudioInit, &oPortInit);
                    break;
                case 2:
                    eError = OMX_GetParameter(hComp, OMX_IndexParamVideoInit, &oPortInit);
                    break;
                case 3:
                    eError = OMX_GetParameter(hComp, OMX_IndexParamImageInit, &oPortInit);
                    break;
                default:
                    return OMX_ErrorUndefined;
            }

            if (OMX_ErrorNone == eError)
            {
                for (i = oPortInit.nStartPortNumber; i < oPortInit.nStartPortNumber + oPortInit.nPorts; i++)
                {
                    eError = OMX_SendCommand(hComp, OMX_CommandPortDisable, i, 0);
                    if (OMX_ErrorNone != eError)
                        return eError;
                }
            }
        }

        return OMX_ErrorNone;
    }

    /* Handle Events */
    OMX_ERRORTYPE WaitResEventHandler(
        OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_EVENTTYPE eEvent,
        OMX_IN OMX_U32 nData1,
        OMX_IN OMX_U32 nData2,
        OMX_IN OMX_PTR pEventData)
    {
        OMXWFRTESTDATA* pContext = pAppData;
        OMX_STATETYPE eState;
        char szState[256];

        UNUSED_PARAMETER(hComponent);
        UNUSED_PARAMETER(pEventData);

        if ((eEvent == OMX_EventCmdComplete) && ((OMX_COMMANDTYPE)(nData1) == OMX_CommandStateSet))
        {
            eState = (OMX_STATETYPE)nData2;

            OMX_CONF_StateToString(eState, szState);

            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,
                           "Instantiation %i changing to state %s\n", pContext->nInstantiation, szState);

            /* Set event change notification */
            OMX_OSAL_EventSet(pContext->hEventStateChange);

            if (eState == OMX_StateIdle)
            {
                OMX_OSAL_EventSet(hEventStateChangeIdle);
            }
            if (eState == OMX_StateLoaded)
            {
                OMX_OSAL_EventSet(hEventStateChangeLoaded);
            }
        }
        if ((eEvent == OMX_EventError) && ((OMX_ERRORTYPE)(nData1) == OMX_ErrorInsufficientResources))
        {

            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,
                           "Instantiation %i sent OMX_ErrorInsufficientResources error\n", pContext->nInstantiation);

            pContext->bErrorInsufficientResources = OMX_TRUE;
            OMX_OSAL_EventSet(pContext->hEventStateChange);
        }
        return OMX_ErrorNone;
    }

    OMX_ERRORTYPE OMX_CONF_ResourcePreemptionTest(OMX_IN OMX_STRING cComponentName)
    {
        /* Interop test 4.6: Resource Manager Preemption */
        /* ------------------------------------------------
         * 1. Load component and set priority to low
         * 2. Transition to idle
         * 3. Repeat from steps 1-3 until transition to idle returns
         *    OMX_ErrorInsufficientResources and transitions to Wait state
         * 4. Switch component in wait state to higher priority (lower number)
         * 5. Verify component with lower priority gives up resources and one with
         *    higher priority gets the necessary resources
         */

        OMX_HANDLETYPE hComp[1000];
        OMX_U32 i, nComp = 0;
        OMX_ERRORTYPE  eError = OMX_ErrorNone;
        OMX_CALLBACKTYPE  oCallbacks;
        OMXWFRTESTDATA    oAppData[1000];
        OMX_STATETYPE     eState = OMX_StateLoaded;
        OMX_BOOL          bTimeout  = OMX_FALSE;
        OMX_PRIORITYMGMTTYPE oPriority;
        OMX_HANDLETYPE hTraceComp[1000];
        OMX_HANDLETYPE hEventStateChange;
        OMX_ERRORTYPE eErrorCleanup;
        OMX_PTR pWrappedAppData[1000];
        OMX_CALLBACKTYPE *pWrappedCallbacks[1000];
        char szInstantiationName[256];

        /* Initialize data */
        oCallbacks.EventHandler         = WaitResEventHandler;
        oCallbacks.EmptyBufferDone      = StubbedEmptyBufferDone;
        oCallbacks.FillBufferDone       = StubbedFillBufferDone;

        /* Begin test */
        eError = OMX_Init();
        if (OMX_ErrorNone != eError) return OMX_ErrorUndefined;

        hEventStateChange       = 0;
        hEventStateChangeIdle   = 0;
        hEventStateChangeLoaded = 0;
        eError = OMX_OSAL_EventCreate(&hEventStateChange);
        OMX_CONF_FAIL_IF_ERROR(eError);
        eError = OMX_OSAL_EventCreate(&hEventStateChangeIdle);
        OMX_CONF_FAIL_IF_ERROR(eError);
        eError = OMX_OSAL_EventCreate(&hEventStateChangeLoaded);
        OMX_CONF_FAIL_IF_ERROR(eError);

        INIT_PARAM(oPriority);
        oPriority.nGroupPriority = OMX_RESOURCETEST_PRIORITY_LOW;

        nComp = 0;
        do
        {
            GetInstantiationName(cComponentName, nComp, szInstantiationName);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Trying to create and idle instantiation %i (priority=%i)\n",
                           nComp, OMX_RESOURCETEST_PRIORITY_LOW);
            if (nComp >= 1000)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Test unable to complete. Too many components requested (%d)\n", nComp);
                eError = OMX_ErrorUndefined;
                goto OMX_CONF_TEST_FAIL;
            }

            hComp[nComp] = hTraceComp[nComp] = NULL;
            oAppData[nComp].bErrorInsufficientResources = OMX_FALSE;
            oAppData[nComp].hEventStateChange      = hEventStateChange;
            oAppData[nComp].nInstantiation = nComp;

            eError = OMX_CONF_CallbackTracerCreate(&oCallbacks, (OMX_PTR) & oAppData[nComp], szInstantiationName,
                                                   &pWrappedCallbacks[nComp], &pWrappedAppData[nComp]);
            OMX_CONF_FAIL_IF_ERROR(eError);

            hComp[nComp] = NULL;
            eError = OMX_GetHandle(&hComp[nComp], cComponentName, pWrappedAppData[nComp], pWrappedCallbacks[nComp]);
            OMX_CONF_FAIL_IF_ERROR(eError); /* Problem acquiring handle. bail. */

            eError = OMX_CONF_ComponentTracerCreate(hComp[nComp], szInstantiationName, &hTraceComp[nComp]);
            OMX_CONF_FAIL_IF_ERROR(eError);

            /* Set priority and transition to idle */
            oPriority.nGroupID       = nComp;
            eError = OMX_SetParameter(hTraceComp[nComp], OMX_IndexParamPriorityMgmt, &oPriority);
            OMX_CONF_FAIL_IF_ERROR(eError);

            /* Disable all ports so we can go idle without buffers */
            eError = DisableAllPorts(hTraceComp[nComp]);
            OMX_CONF_FAIL_IF_ERROR(eError);

            eError = OMX_SendCommand(hTraceComp[nComp], OMX_CommandStateSet, OMX_StateIdle, 0);
            OMX_CONF_FAIL_IF_ERROR(eError);
            OMX_OSAL_EventWait(hEventStateChange, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
            if (bTimeout == OMX_TRUE)
            {
                eError = OMX_ErrorTimeout;
                goto OMX_CONF_TEST_FAIL;
            }
            OMX_OSAL_EventReset(hEventStateChange);

            eError = OMX_GetState(hTraceComp[nComp], &eState);
            OMX_CONF_FAIL_IF_ERROR(eError);
            nComp++;
        }
        while (!oAppData[nComp-1].bErrorInsufficientResources);

        nComp--;

        /* check that the last try stayed in loaded because of insufficient resources */
        if ((eState != OMX_StateLoaded) || !oAppData[nComp].bErrorInsufficientResources)
        {
            eError = OMX_ErrorUndefined;
            goto OMX_CONF_TEST_FAIL;
        }
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Instantiation %i failed to idle (insuffient resources)\n", nComp);

        /* Set priority and transition to idle */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Raising the priority of instantiation %i (priority=%i)\n",
                       nComp, OMX_RESOURCETEST_PRIORITY_HIGH);
        oPriority.nGroupPriority = OMX_RESOURCETEST_PRIORITY_HIGH;
        eError = OMX_SetParameter(hTraceComp[nComp], OMX_IndexParamPriorityMgmt, &oPriority);
        OMX_CONF_FAIL_IF_ERROR(eError);

        /* Disable all ports so we can go idle without buffers */
        eError = DisableAllPorts(hTraceComp[nComp]);
        OMX_CONF_FAIL_IF_ERROR(eError);

        /* Retry transition to idle */
        OMX_OSAL_EventReset(hEventStateChange);
        OMX_OSAL_EventReset(hEventStateChangeIdle);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Trying to idle instantiation %i \n", nComp);
        eError = OMX_SendCommand(hTraceComp[nComp], OMX_CommandStateSet, OMX_StateIdle, 0);
        OMX_CONF_FAIL_IF_ERROR(eError);
        OMX_OSAL_EventWait(hEventStateChange, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
        if (bTimeout == OMX_TRUE)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,
                           "-ERROR: Timed out waiting for state change from instantiation %i\n", nComp);
            eError = OMX_ErrorTimeout;
            goto OMX_CONF_TEST_FAIL;
        }
        if (eState != OMX_StateLoaded)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,
                           "-ERROR: State of pre-empted instantiation is not OMX_StateLoaded\n", nComp);
            eError = OMX_ErrorUndefined;
            goto OMX_CONF_TEST_FAIL;
        }
        OMX_OSAL_EventWait(hEventStateChangeIdle, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
        eError = OMX_GetState(hTraceComp[nComp], &eState);
        OMX_CONF_FAIL_IF_ERROR(eError);
        if (bTimeout == OMX_TRUE)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,
                           "-ERROR: Timed out waiting for state change from instantiation %i\n", nComp);
            eError = OMX_ErrorTimeout;
            goto OMX_CONF_TEST_FAIL;
        }
        if (eState != OMX_StateIdle)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,
                           "-ERROR: State of instantiation %i is not OMX_StateIdle\n", nComp);
            eError = OMX_ErrorUndefined;
            goto OMX_CONF_TEST_FAIL;
        }
OMX_CONF_TEST_FAIL:
        /* Cleanup */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Cleaning up\n");
        for (i = 0;i <= nComp;i++)
        {
            OMX_ERRORTYPE eErrorCleanup;

            if (hTraceComp[i] != NULL)
            {
                OMX_STATETYPE eState = OMX_StateInvalid;

                if (OMX_ErrorNone == OMX_GetState(hTraceComp[i], &eState)
                        && eState != OMX_StateInvalid
                        && eState != OMX_StateLoaded)
                {
                    OMX_OSAL_EventReset(hEventStateChange);
                    eErrorCleanup = OMX_SendCommand(hTraceComp[i], OMX_CommandStateSet, OMX_StateLoaded, 0);
                    if (OMX_ErrorNone == eErrorCleanup)
                    {
                        OMX_OSAL_EventWait(hEventStateChange, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
                    }
                }
            }

        }

        for (i = 0;i <= nComp;i++)
        {
            OMX_OSAL_EventReset(hEventStateChange);
            OMX_OSAL_EventWait(hEventStateChange, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
            if (hComp[i])
                eErrorCleanup = OMX_FreeHandle(hComp[i]);

        }
        for (i = 0;i <= nComp;i++)
        {
            /* unset trace component */
            if (hTraceComp[i] != NULL)
                OMX_CONF_ComponentTracerDestroy(hTraceComp[i]);
        }

        for (i = 0;i <= nComp;i++)
        {
            OMX_CONF_CallbackTracerDestroy(pWrappedCallbacks[i], pWrappedAppData[i]);
        }

        if (hEventStateChange)
        {
            OMX_OSAL_EventDestroy(hEventStateChange);
        }
        if (hEventStateChangeIdle)
        {
            OMX_OSAL_EventDestroy(hEventStateChangeIdle);
        }
        if (hEventStateChangeLoaded)
        {
            OMX_OSAL_EventDestroy(hEventStateChangeLoaded);
        }
        OMX_Deinit();

        return eError;
    }

    OMX_ERRORTYPE OMX_CONF_WaitForResourcesTest(OMX_IN OMX_STRING cComponentName)
    {
        /* Interop test 4.7: Wait For Resources */
        /* ---------------------------------------
         * 1. Load component
         * 2. Transition to idle
         * 3. Repeat from steps 1-3 until transition to idle returns
         *    OMX_ErrorInsufficientResources and transitions to Wait state
         * 4. Switch one of the previous components to the loaded state to free resources
         * 5. Verify the component in the WaitForResources state transitions to idle
         */

        OMX_HANDLETYPE hComp[1000];
        OMX_U32 i, nComp = 0;
        OMX_ERRORTYPE  eError = OMX_ErrorNone;
        OMX_CALLBACKTYPE  oCallbacks;
        OMXWFRTESTDATA    oAppData[1000];
        OMX_STATETYPE     eState = OMX_StateLoaded;
        OMX_BOOL          bTimeout  = OMX_FALSE;
        OMX_PRIORITYMGMTTYPE oPriority;
        OMX_HANDLETYPE hTraceComp[1000];
        OMX_HANDLETYPE hEventStateChange;
        OMX_ERRORTYPE eErrorCleanup;
        OMX_PTR pWrappedAppData[1000];
        OMX_CALLBACKTYPE *pWrappedCallbacks[1000];
        char szInstantiationName[256];

        /* Initialize data */
        oCallbacks.EventHandler         = WaitResEventHandler;
        oCallbacks.EmptyBufferDone      = StubbedEmptyBufferDone;
        oCallbacks.FillBufferDone       = StubbedFillBufferDone;

        /* Begin test */
        eError = OMX_Init();
        if (OMX_ErrorNone != eError) return OMX_ErrorUndefined;

        hEventStateChange       = 0;
        hEventStateChangeIdle   = 0;
        hEventStateChangeLoaded = 0;
        eError = OMX_OSAL_EventCreate(&hEventStateChange);
        OMX_CONF_FAIL_IF_ERROR(eError);
        eError = OMX_OSAL_EventCreate(&hEventStateChangeIdle);
        OMX_CONF_FAIL_IF_ERROR(eError);
        eError = OMX_OSAL_EventCreate(&hEventStateChangeLoaded);
        OMX_CONF_FAIL_IF_ERROR(eError);

        INIT_PARAM(oPriority);
        oPriority.nGroupPriority = OMX_RESOURCETEST_PRIORITY_LOW;
        oPriority.nGroupID       = 1;


        nComp = 0;
        do
        {
            hComp[nComp] = hTraceComp[nComp] = NULL;
            GetInstantiationName(cComponentName, nComp, szInstantiationName);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Trying to create and idle instantiation %i (priority = %i)\n",
                           nComp, OMX_RESOURCETEST_PRIORITY_LOW);
            if (nComp >= 1000)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Test unable to complete. Too many components requested (%d)\n", nComp);
                eError = OMX_ErrorUndefined;
                goto OMX_CONF_TEST_FAIL;
            }

            oAppData[nComp].bErrorInsufficientResources = OMX_FALSE;
            oAppData[nComp].hEventStateChange      = hEventStateChange;
            oAppData[nComp].nInstantiation = nComp;

            eError = OMX_CONF_CallbackTracerCreate(&oCallbacks, (OMX_PTR) & oAppData[nComp], szInstantiationName,
                                                   &pWrappedCallbacks[nComp], &pWrappedAppData[nComp]);
            OMX_CONF_FAIL_IF_ERROR(eError);

            hComp[nComp] = NULL;
            eError = OMX_GetHandle(&hComp[nComp], cComponentName, pWrappedAppData[nComp], pWrappedCallbacks[nComp]);
            OMX_CONF_FAIL_IF_ERROR(eError); /* Problem acquiring handle. bail. */

            eError = OMX_CONF_ComponentTracerCreate(hComp[nComp], szInstantiationName, &hTraceComp[nComp]);
            OMX_CONF_FAIL_IF_ERROR(eError);

            /* Set priority and transition to idle */
            eError = OMX_SetParameter(hTraceComp[nComp], OMX_IndexParamPriorityMgmt, &oPriority);
            OMX_CONF_FAIL_IF_ERROR(eError);

            /* Disable all ports so we can go idle without buffers */
            eError = DisableAllPorts(hTraceComp[nComp]);
            OMX_CONF_FAIL_IF_ERROR(eError);

            eError = OMX_SendCommand(hTraceComp[nComp], OMX_CommandStateSet, OMX_StateIdle, 0);
            OMX_CONF_FAIL_IF_ERROR(eError);
            OMX_OSAL_EventWait(hEventStateChange, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
            if (bTimeout == OMX_TRUE)
            {
                eError = OMX_ErrorTimeout;
                goto OMX_CONF_TEST_FAIL;
            }
            OMX_OSAL_EventReset(hEventStateChange);

            eError = OMX_GetState(hTraceComp[nComp], &eState);
            OMX_CONF_FAIL_IF_ERROR(eError);
            nComp++;

        }
        while (!oAppData[nComp-1].bErrorInsufficientResources);

        nComp--;

        /* check that the last try stayed in loaded because of insufficient resources */
        if ((eState != OMX_StateLoaded) || !oAppData[nComp].bErrorInsufficientResources)
        {
            eError = OMX_ErrorUndefined;
            goto OMX_CONF_TEST_FAIL;
        }

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Instantiation %i failed to idle (insuffient resources)\n", nComp);

        /* tell last try it to wait for resources */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Telling instantiation %i to wait for resources\n", nComp);
        OMX_OSAL_EventReset(hEventStateChange);
        OMX_CONF_FAIL_IF_ERROR(OMX_SendCommand(hTraceComp[nComp], OMX_CommandStateSet, OMX_StateWaitForResources, 0));
        OMX_OSAL_EventWait(hEventStateChange, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
        if (bTimeout == OMX_TRUE)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,
                           "-ERROR: Timed out waiting for state change from instantiation %i\n", nComp);
            eError = OMX_ErrorTimeout;
            goto OMX_CONF_TEST_FAIL;
        }
        OMX_CONF_FAIL_IF_ERROR(OMX_GetState(hTraceComp[nComp], &eState));
        if (eState != OMX_StateWaitForResources)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,
                           "-ERROR: State of instantiation %i is not OMX_StateWaitForResources\n", nComp);
            eError = OMX_ErrorUndefined;
            goto OMX_CONF_TEST_FAIL;
        }

        /* transition first try to loaded */
        OMX_OSAL_EventReset(hEventStateChange);
        OMX_OSAL_EventReset(hEventStateChangeIdle);
        OMX_OSAL_EventReset(hEventStateChangeLoaded);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Telling instantiation 0 to go to loaded\n", nComp);
        OMX_CONF_FAIL_IF_ERROR(OMX_SendCommand(hTraceComp[0], OMX_CommandStateSet, OMX_StateLoaded, 0));
        OMX_OSAL_EventWait(hEventStateChangeLoaded, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
        if (bTimeout == OMX_TRUE)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,
                           "-ERROR: Timed out waiting for state change from instantiation 0\n");
            eError = OMX_ErrorTimeout;
            goto OMX_CONF_TEST_FAIL;
        }
        OMX_CONF_FAIL_IF_ERROR(OMX_GetState(hTraceComp[0], &eState));
        if (eState != OMX_StateLoaded)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,
                           "-ERROR: State of instantiation 0 is not OMX_StateLoaded\n");
            eError = OMX_ErrorUndefined;
            goto OMX_CONF_TEST_FAIL;
        }

        /* wait for last try to get resources */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Waiting for instantiation %i to go to idle\n", nComp);
        OMX_OSAL_EventWait(hEventStateChangeIdle, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
        if (bTimeout == OMX_TRUE)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,
                           "-ERROR: Timed out waiting for state change from instantiation %i\n", nComp);
            eError = OMX_ErrorTimeout;
            goto OMX_CONF_TEST_FAIL;
        }
        OMX_CONF_FAIL_IF_ERROR(OMX_GetState(hTraceComp[nComp], &eState));
        if (eState != OMX_StateIdle)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,
                           "-ERROR: State of instantiation %i is not OMX_StateIdle\n", nComp);
            eError = OMX_ErrorUndefined;
            goto OMX_CONF_TEST_FAIL;
        }

OMX_CONF_TEST_FAIL:
        /* Cleanup */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Cleaning up\n");
        for (i = 1;i <= nComp;i++)
        {
            OMX_ERRORTYPE eErrorCleanup;

            if (hTraceComp[i] != NULL)
            {
                OMX_OSAL_EventReset(hEventStateChange);
                eErrorCleanup = OMX_SendCommand(hTraceComp[i], OMX_CommandStateSet, OMX_StateLoaded, 0);
                if (OMX_ErrorNone == eErrorCleanup)
                {
                    OMX_OSAL_EventWait(hEventStateChange, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
                }
            }
        }

        for (i = 0;i <= nComp;i++)
        {
            OMX_OSAL_EventReset(hEventStateChange);
            OMX_OSAL_EventWait(hEventStateChange, 20, &bTimeout);
            if (hComp[i] != NULL)
                eErrorCleanup = OMX_FreeHandle(hComp[i]);
        }

        for (i = 0;i <= nComp;i++)
        {
            /* unset trace component */
            if (hTraceComp[i] != NULL)
                OMX_CONF_ComponentTracerDestroy(hTraceComp[i]);
        }

        for (i = 0;i <= nComp;i++)
        {
            OMX_CONF_CallbackTracerDestroy(pWrappedCallbacks[i], pWrappedAppData[i]);
        }

        if (hEventStateChange)
        {
            OMX_OSAL_EventDestroy(hEventStateChange);
        }

        if (hEventStateChangeIdle)
        {
            OMX_OSAL_EventDestroy(hEventStateChangeIdle);
        }

        if (hEventStateChangeLoaded)
        {
            OMX_OSAL_EventDestroy(hEventStateChangeLoaded);
        }

        OMX_Deinit();

        return eError;
    }

#ifdef __cplusplus
}
#endif /* __cplusplus */



