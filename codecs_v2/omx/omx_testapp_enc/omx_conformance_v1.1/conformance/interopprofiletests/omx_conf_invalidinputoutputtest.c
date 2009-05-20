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

/** OMX_CONF_InvalidInputOutputTest.c
 *  OpenMax IL conformance test validating correct component behavior when connecting to input or
 *  output ports that behave incorrectly.
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include <string.h>
#include "OMX_OSAL_Interfaces.h"
#include "OMX_CONF_TestHarness.h"
#include "OMX_CONF_StubbedCallbacks.h"
#include "OMX_CONF_TunnelTestComponent.h"

    /* Callback data */
    typedef struct IIOTDATATYPE
    {
        OMX_STATETYPE eState;
        OMX_HANDLETYPE hStateChangeEvent;
        OMX_HANDLETYPE hCUT;
    } IIOTDATATYPE;

    /* Invalid Input Output Test's implementation of OMX_CALLBACKTYPE.EventHandler */
    OMX_ERRORTYPE IIOTEventHandler(
        OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_EVENTTYPE eEvent,
        OMX_IN OMX_U32 nData1,
        OMX_IN OMX_U32 nData2,
        OMX_IN OMX_PTR pEventData)
    {
        IIOTDATATYPE* pContext = pAppData;

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
    OMX_ERRORTYPE IIOTWaitForState(IIOTDATATYPE *pAppData, OMX_STATETYPE eState)
    {
        OMX_BOOL bTimedOut = OMX_FALSE;

        OMX_OSAL_EventWait(pAppData->hStateChangeEvent, OMX_CONF_TIMEOUT_EXPECTING_FAILURE, &bTimedOut);
        if (bTimedOut || pAppData->eState != eState)
        {
            return OMX_ErrorUndefined;
        }
        OMX_OSAL_EventReset(pAppData->hStateChangeEvent);
        return OMX_ErrorNone;
    }

    /* perform tests on all ports of a particular domain */
    static OMX_ERRORTYPE DoTestAllPortsForDomain(OMX_INDEXTYPE nIndex, OMX_HANDLETYPE hWrappedComp,
            OMX_HANDLETYPE hWrappedTTComp, OMX_DIRTYPE eDir, OMX_U8 bShouldError, OMX_STRING sTestType)
    {
        OMX_PORT_PARAM_TYPE oParam;
        OMX_U32 i;
        OMX_U32 iCUTPort;
        OMX_ERRORTYPE eError;

        INIT_PARAM(oParam);

        /* query the component's other ports */
        if (OMX_ErrorNone != (eError = OMX_GetParameter(hWrappedComp, nIndex, &oParam))) return eError;

        /* for each discovered port */
        for (i = 0;i < oParam.nPorts;i++)
        {
            OMX_PARAM_PORTDEFINITIONTYPE oCUTPort;
            INIT_PARAM(oCUTPort);

            iCUTPort = oParam.nStartPortNumber + i;

            oCUTPort.nPortIndex = iCUTPort;
            eError = OMX_GetParameter(hWrappedComp, OMX_IndexParamPortDefinition, &oCUTPort);
            if (eError != OMX_ErrorNone)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--Invalid IO Tunnel test %s Failed for Port %d\n",
                               sTestType, iCUTPort);
                return eError;
            }
            else if (eDir == oCUTPort.eDir)
            {
                eError = TTCConnectPort(hWrappedTTComp, hWrappedComp, iCUTPort);
                if (bShouldError && eError != OMX_ErrorNone)
                {
                    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--Invalid IO Tunnel test %s Passed for Port %d\n",
                                   sTestType, iCUTPort);
                    eError = OMX_ErrorNone;
                }
                else if (bShouldError)
                {
                    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--Invalid IO Tunnel test %s Failed for Port %d\n",
                                   sTestType, iCUTPort);
                    return OMX_ErrorUndefined;
                }
                else if (eError != OMX_ErrorNone)
                {
                    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--Invalid IO Tunnel test %s Failed for Port %d\n",
                                   sTestType, iCUTPort);
                    return eError;
                }
            }
            else if (!bShouldError)
            {
                /* if we don't expect an error, try to connect the port, but ignore any result so that transitions to idle work normally */
                TTCConnectPort(hWrappedTTComp, hWrappedComp, iCUTPort);
            }

        }
        return eError;
    }

    /* perform tests on all ports */
    static OMX_ERRORTYPE DoTestAllPorts(OMX_HANDLETYPE hWrappedComp, OMX_HANDLETYPE hWrappedTTComp,
                                        OMX_DIRTYPE eDir, OMX_U8 bShouldError, OMX_STRING sTestType)
    {
        OMX_ERRORTYPE eError;

        eError = OMX_ErrorNone;

        /* test audio ports */
        if (OMX_ErrorNone != (eError =
                                  DoTestAllPortsForDomain(OMX_IndexParamAudioInit, hWrappedComp, hWrappedTTComp,
                                                          eDir, bShouldError, sTestType))) return eError;

        /* test video ports */
        if (OMX_ErrorNone != (eError =
                                  DoTestAllPortsForDomain(OMX_IndexParamVideoInit, hWrappedComp, hWrappedTTComp,
                                                          eDir, bShouldError, sTestType))) return eError;

        /* test image ports */
        if (OMX_ErrorNone != (eError =
                                  DoTestAllPortsForDomain(OMX_IndexParamImageInit, hWrappedComp, hWrappedTTComp,
                                                          eDir, bShouldError, sTestType))) return eError;

        /* test other ports */
        if (OMX_ErrorNone != (eError =
                                  DoTestAllPortsForDomain(OMX_IndexParamOtherInit, hWrappedComp, hWrappedTTComp,
                                                          eDir, bShouldError, sTestType))) return eError;


        return eError;
    }

    /* perform all input port tests */
    static OMX_ERRORTYPE TestAllInputPorts(OMX_HANDLETYPE hWrappedComp, OMX_HANDLETYPE hWrappedTTComp, OMX_PTR pAppData)
    {
        OMX_ERRORTYPE eError;
        OMX_COMPONENTTYPE *pComp;

        pComp = (OMX_COMPONENTTYPE *)hWrappedComp;

        // invalid port type
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Input - Testing invalid port types\n");
        TTCCreateInvalidPortTypes(hWrappedTTComp, 1);
        if (OMX_ErrorNone != (eError = DoTestAllPorts(hWrappedComp,
                                       hWrappedTTComp, OMX_DirInput, 1, "'Input - Invalid port type'"))) return eError;
        TTCCreateInvalidPortTypes(hWrappedTTComp, 0);

        // doesn't support IndexParamCompbufferSupplier
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Input - Testing when IndexParamCompBufferSupplier is broken\n");
        TTCDontSupportIndexParamCompBufferSupplier(hWrappedTTComp, 1);
        if (OMX_ErrorNone != (eError = DoTestAllPorts(hWrappedComp,
                                       hWrappedTTComp, OMX_DirInput, 1, "'Input - IndexParamCompBufferSupplier broken'"))) return eError;
        TTCDontSupportIndexParamCompBufferSupplier(hWrappedTTComp, 0);

        // don't support UseBuffer
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Input - Testing when UseBuffer doesn't work\n");
        TTCDontSupportUseBuffer(hWrappedTTComp, 1);
        if (OMX_ErrorNone != (eError = DoTestAllPorts(hWrappedComp,
                                       hWrappedTTComp, OMX_DirInput, 0, "'Input - UseBuffer unsupported'"))) return eError;

        OMX_OSAL_EventReset(((IIOTDATATYPE *)pAppData)->hStateChangeEvent);
        if (OMX_ErrorNone != (eError = OMX_SendCommand(hWrappedComp,
                                       OMX_CommandStateSet, OMX_StateIdle, 0))) return eError;

        if (OMX_ErrorNone == (eError = IIOTWaitForState(pAppData, OMX_StateIdle)))
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--Invalid IO Tunnel test %s Failed\n",
                           "'Input - UseBuffer unsupported'");
            eError = OMX_ErrorPortsNotCompatible;
        }
        else
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--Invalid IO Tunnel test %s Passed\n",
                           "'Input - UseBuffer unsupported'");
            eError = OMX_ErrorNone;
        }

        OMX_OSAL_EventReset(((IIOTDATATYPE *)pAppData)->hStateChangeEvent);
        TTCDontSupportUseBuffer(hWrappedTTComp, 0);

        return eError;
    }

    /* perform all output port tests */
    static OMX_ERRORTYPE TestAllOutputPorts(OMX_HANDLETYPE hWrappedComp, OMX_HANDLETYPE hWrappedTTComp, OMX_PTR pAppData)
    {
        OMX_ERRORTYPE eError;

        // invalid port type
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Output - Testing invalid port types\n");
        TTCCreateInvalidPortTypes(hWrappedTTComp, 1);
        if (OMX_ErrorNone != (eError = DoTestAllPorts(hWrappedComp,
                                       hWrappedTTComp, OMX_DirOutput, 1, "'Output - Invalid port type'"))) return eError;
        TTCCreateInvalidPortTypes(hWrappedTTComp, 0);

        // input port supplier preference opposite of CUT output port supplier preference
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Output - Testing input port supplier preference opposite of CUT output port supplier preference\n");
        TTCInvertBufferSupplier(hWrappedTTComp, 1);
        if (OMX_ErrorNone != (eError = DoTestAllPorts(hWrappedComp,
                                       hWrappedTTComp, OMX_DirOutput, 0, "'Output - Input port supplier preference opposite of CUT output port supplier preference'"))) return eError;
        TTCInvertBufferSupplier(hWrappedTTComp, 0);

        // don't support UseBuffer
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Output - Testing when UseBuffer doesn't work\n");
        TTCDontSupportUseBuffer(hWrappedTTComp, 1);
        if (OMX_ErrorNone != (eError = DoTestAllPorts(hWrappedComp,
                                       hWrappedTTComp, OMX_DirOutput, 0, "'Output - UseBuffer unsupported'"))) return eError;

        OMX_OSAL_EventReset(((IIOTDATATYPE *)pAppData)->hStateChangeEvent);
        if (OMX_ErrorNone != (eError = OMX_SendCommand(hWrappedComp,
                                       OMX_CommandStateSet, OMX_StateIdle, 0))) return eError;

        if (OMX_ErrorNone == IIOTWaitForState(pAppData, OMX_StateIdle))
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--Invalid IO Tunnel test %s Failed\n",
                           "'Output - UseBuffer unsupported'");
            eError = OMX_ErrorPortsNotCompatible;
        }
        else
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--Invalid IO Tunnel test %s Passed\n",
                           "'Output - UseBuffer unsupported'");
            eError = OMX_ErrorNone;
        }

        OMX_OSAL_EventReset(((IIOTDATATYPE *)pAppData)->hStateChangeEvent);
        TTCDontSupportUseBuffer(hWrappedTTComp, 0);

        return eError;
    }

    /* Main entrypoint into the Invalid Input Output Test */
    OMX_ERRORTYPE OMX_CONF_InvalidInputOutputTest(OMX_IN OMX_STRING cComponentName)
    {
        OMX_PTR pWrappedAppData;
        OMX_CALLBACKTYPE *pWrappedCallbacks;
        OMX_HANDLETYPE hComp, hTTComp, hWrappedComp, hWrappedTTComp;
        OMX_ERRORTYPE  eTemp, eError = OMX_ErrorNone;
        OMX_CALLBACKTYPE oCallbacks;
        IIOTDATATYPE oAppData;
        memset(&oAppData, 0, sizeof oAppData);

        oAppData.hStateChangeEvent = 0;
        OMX_OSAL_EventCreate(&oAppData.hStateChangeEvent);

        /* init component handles */
        hComp = hWrappedComp = hTTComp = hWrappedTTComp = 0;

        oCallbacks.EventHandler    = IIOTEventHandler;
        oCallbacks.EmptyBufferDone = StubbedEmptyBufferDone;
        oCallbacks.FillBufferDone  = StubbedFillBufferDone;
        eError = OMX_CONF_CallbackTracerCreate(&oCallbacks, (OMX_PTR) & oAppData, cComponentName,
                                               &pWrappedCallbacks, &pWrappedAppData);

        /* Initialize OpenMax */
        eError = OMX_Init();

        OMX_CONF_FAIL_IF_ERROR(OMX_GetHandle(&hComp, cComponentName, pWrappedAppData, pWrappedCallbacks));
        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_ComponentTracerCreate(hComp, cComponentName, &hWrappedComp));
        oAppData.hCUT = hComp;

        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_GetTunnelTestComponentHandle(&hTTComp, pWrappedAppData, pWrappedCallbacks));
        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_ComponentTracerCreate(hTTComp, "OMX.CONF.tunnel.test", &hWrappedTTComp));

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Invalid Input Tunneling Setup Tests\n");
        OMX_CONF_FAIL_IF_ERROR(TestAllInputPorts(hWrappedComp, hWrappedTTComp, &oAppData));

        if (hWrappedComp)
        {
            OMX_CONF_REMEMBER_ERROR(OMX_CONF_ComponentTracerDestroy(hWrappedComp));
        }

        if (hComp)
        {
            OMX_CONF_REMEMBER_ERROR(OMX_FreeHandle(hComp));
        }

        OMX_CONF_FAIL_IF_ERROR(OMX_GetHandle(&hComp, cComponentName, pWrappedAppData, pWrappedCallbacks));
        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_ComponentTracerCreate(hComp, cComponentName, &hWrappedComp));
        oAppData.hCUT = hComp;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Invalid Output Tunneling Setup Tests\n");
        OMX_CONF_FAIL_IF_ERROR(TestAllOutputPorts(hWrappedComp, hWrappedTTComp, &oAppData));

        OMX_CONF_TEST_FAIL
:

        /* Cleanup: Return function errors rather than closing errors if appropriate */

        OMX_OSAL_EventReset(oAppData.hStateChangeEvent);

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

        if (oAppData.hStateChangeEvent)
        {
            OMX_OSAL_EventDestroy(oAppData.hStateChangeEvent);
        }

        return eError;
    }

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */
