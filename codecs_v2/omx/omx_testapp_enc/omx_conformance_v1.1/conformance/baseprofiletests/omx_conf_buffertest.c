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

/** OMX_CONF_BufferTest.c
 *  OpenMax IL conformance test - Buffer Test
 */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "OMX_OSAL_Interfaces.h"
#include "OMX_CONF_TestHarness.h"
#include "OMX_CONF_StubbedCallbacks.h"

#include <string.h>

    /*
     *  E X T E R N A L   F U N C T I O N S
     */
    OMX_ERRORTYPE OMX_CONF_CommandToString(
        OMX_IN OMX_COMMANDTYPE eCommand,
        OMX_OUT OMX_STRING sName);
    OMX_ERRORTYPE OMX_CONF_StateToString(
        OMX_IN OMX_STATETYPE eState,
        OMX_OUT OMX_STRING sName);


    /*
     *  D E C L A R A T I O N S
     */



#define TEST_NAME_STRING "BufferTest"
#define TEST_COMPONENT_NAME_SIZE 128

#define NUM_DOMAINS 0x4

    static char szDesc[256];

    /*
     *     M A C R O S
     */

#define OMX_CONF_INIT_STRUCT(_s_, _name_)\
    memset(&(_s_), 0x0, sizeof(_name_));\
    (_s_).nSize = sizeof(_name_);\
    (_s_).nVersion.s.nVersionMajor = 0x1;\
    (_s_).nVersion.s.nVersionMinor = 0x0;\
    (_s_).nVersion.s.nRevision = 0x0;\
    (_s_).nVersion.s.nStep = 0x0

#define OMX_CONF_BAIL_ON_ERROR(_e_) \
    if (OMX_ErrorNone != (_e_))\
    {\
        goto OMX_CONF_TEST_BAIL;\
    }

#define OMX_CONF_SET_ERROR_BAIL(_e_, _c_, _s_)\
    _e_ = _c_;\
    OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, _s_);\
    goto OMX_CONF_TEST_BAIL

#define OMX_CONF_ASSERT(_e_, _a_, _r_) \
    if (!(_a_))\
    {\
        OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "%s\n", (_r_));\
        (_e_) = OMX_ErrorUndefined;\
        goto OMX_CONF_TEST_BAIL;\
    }

#define LIFO_ALLOC(_p_, _n_)\
    _p_ = (LIFOTYPE*)OMX_OSAL_Malloc(sizeof(LIFOTYPE) + (sizeof(OMX_PTR) * (_n_ + 1)));\
    if (0x0 != _p_)\
    {\
        _p_->nNumEntries = _n_;\
        _p_->nEnd = 0;\
    }

#define LIFO_FREE(_p_)\
    OMX_OSAL_Free(_p_);\
    _p_ = 0x0

#define LIFO_ADD(_p_, _e_)\
    _p_->aEntires[_p_->nEnd] = (OMX_PTR)_e_;\
    _p_->nEnd++

#define LIFO_REMOVE(_p_, _e_)\
    _e_ = 0x0;\
    if (0x0 != _p_->nEnd)\
    {\
        _p_->nEnd--;\
        _e_ = _p_->aEntires[_p_->nEnd];\
    }

#define LIFO_INQUEUE(_p_) _p_->nEnd



    /*
     *     D E F I N I T I O N S
     */


    typedef struct _LIFOTYPE
    {
        OMX_U32 nNumEntries;
        OMX_U32 nEnd;
        OMX_PTR aEntires[1];

    } LIFOTYPE;

    typedef struct _TEST_PORTTYPE
    {
        OMX_PARAM_PORTDEFINITIONTYPE sPortDef;
        LIFOTYPE *pLifo;

    } TEST_PORTTYPE;

    typedef struct _TEST_CTXTYPE
    {
        OMX_STATETYPE eState;
        OMX_HANDLETYPE hWrappedComp;
        OMX_U32 nNumPorts;
        OMX_PORT_PARAM_TYPE sPortParam[NUM_DOMAINS];
        OMX_HANDLETYPE hStateChangeEvent;
        OMX_HANDLETYPE hPortDisableEvent;
        OMX_HANDLETYPE hPortEnableEvent;
        OMX_HANDLETYPE hPortErrorEvent;
        OMX_U32 nNumPortsStopped;
        OMX_BOOL bStopAllPorts;
        OMX_U32 nNumPortsRestarted;
        OMX_BOOL bRestartAllPorts;
        OMX_U32 nRestartPort;
        OMX_U32 nStopPort;
        TEST_PORTTYPE *aPorts;

    } TEST_CTXTYPE;



    /*
     *     F U N C T I O N S
     */

    /*****************************************************************************/
    OMX_ERRORTYPE BufferTest_EventHandler(
        OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_EVENTTYPE eEvent,
        OMX_IN OMX_U32 nData1,
        OMX_IN OMX_U32 nData2,
        OMX_IN OMX_PTR pEventData)
    {
        TEST_CTXTYPE *pCtx;

        UNUSED_PARAMETER(hComponent);
        UNUSED_PARAMETER(pEventData);

        if (0x0 == pAppData) return(OMX_ErrorNone);
        pCtx = (TEST_CTXTYPE*)pAppData;

        if (OMX_EventCmdComplete == eEvent)
        {
            switch ((OMX_COMMANDTYPE)(nData1))
            {
                case OMX_CommandStateSet:
                    OMX_CONF_StateToString((OMX_STATETYPE)(nData2), szDesc);
                    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Component transitioned to %s\n", szDesc);
                    pCtx->eState = (OMX_STATETYPE)(nData2);
                    OMX_OSAL_EventSet(pCtx->hStateChangeEvent);
                    break;
                case OMX_CommandPortDisable:
                    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i completed disable\n", nData2);
                    if (OMX_TRUE == pCtx->bStopAllPorts)
                    {
                        pCtx->nNumPortsStopped++;
                        if (pCtx->nNumPortsStopped == pCtx->nNumPorts)
                        {
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "All ports completed disable\n");
                            OMX_OSAL_EventSet(pCtx->hPortDisableEvent);
                        }

                    }
                    else if (pCtx->nStopPort == nData2)
                    {
                        OMX_OSAL_EventSet(pCtx->hPortDisableEvent);
                    }
                    break;
                case OMX_CommandPortEnable:
                    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i completed enable\n", nData2);
                    if (OMX_TRUE == pCtx->bRestartAllPorts)
                    {
                        pCtx->nNumPortsRestarted++;
                        if (pCtx->nNumPortsRestarted == pCtx->nNumPorts)
                        {
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "All ports completed enable\n");
                            OMX_OSAL_EventSet(pCtx->hPortEnableEvent);
                        }
                    }
                    else if (pCtx->nRestartPort == nData2)
                    {
                        OMX_OSAL_EventSet(pCtx->hPortEnableEvent);
                    }
                    break;
                case OMX_EventBufferFlag:
                    break;
                default:
                    break;
            }

        }
        else if (OMX_EventError == eEvent)
        {
            OMX_CONF_ErrorToString((OMX_ERRORTYPE)nData1, szDesc);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Component reported error %s (0x%x)\n",
                           szDesc, nData2);
            if (OMX_ErrorPortUnpopulated == nData1)
            {
                OMX_OSAL_EventSet(pCtx->hPortErrorEvent);
            }
        }

        return OMX_ErrorNone;
    }


    /*****************************************************************************/
    OMX_ERRORTYPE BufferTest_PortAllocateNumBuffers(
        TEST_CTXTYPE *pCtx,
        TEST_PORTTYPE *pPort,
        OMX_U32 nNumBuffers)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_BUFFERHEADERTYPE *pBufHdr;

        while (0x0 != nNumBuffers)
        {
            eError = OMX_AllocateBuffer(pCtx->hWrappedComp, &pBufHdr,
                                        pPort->sPortDef.nPortIndex,
                                        0x0, pPort->sPortDef.nBufferSize);
            OMX_CONF_BAIL_ON_ERROR(eError);
            LIFO_ADD(pPort->pLifo, pBufHdr);
            nNumBuffers--;
        }


OMX_CONF_TEST_BAIL:

        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE BufferTest_PortFreeNumBuffers(
        TEST_CTXTYPE *pCtx,
        TEST_PORTTYPE *pPort,
        OMX_U32 nNumBuffers)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_BUFFERHEADERTYPE *pBufHdr;

        while ((0x0 != LIFO_INQUEUE(pPort->pLifo)) && (0x0 != nNumBuffers))
        {
            LIFO_REMOVE(pPort->pLifo, pBufHdr);
            eError = OMX_FreeBuffer(pCtx->hWrappedComp, pPort->sPortDef.nPortIndex, pBufHdr);
            OMX_CONF_BAIL_ON_ERROR(eError);
            nNumBuffers--;
        }


OMX_CONF_TEST_BAIL:

        return(eError);

    }


    /*****************************************************************************/
    OMX_ERRORTYPE BufferTest_AllocatePortStructures(
        TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_PORTTYPE *pPort;
        OMX_U32 nSize;
        OMX_U32 i, j;
        OMX_U32 nPortIndex;

        /* allocate port structures */
        nSize = (sizeof(TEST_PORTTYPE) * pCtx->nNumPorts);
        pCtx->aPorts = (TEST_PORTTYPE*)OMX_OSAL_Malloc(nSize);
        if (0x0 == pCtx->aPorts)
        {
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "OMX_OSAL_Malloc failed\n");
        }

        memset(pCtx->aPorts, 0x0, nSize);

        pPort = pCtx->aPorts;
        /* initialize port structures */
        for (i = 0; i < NUM_DOMAINS; i++)
        {
            for (j = 0x0; j < pCtx->sPortParam[i].nPorts; j++)
            {
                nPortIndex = pCtx->sPortParam[i].nStartPortNumber + j;
                OMX_CONF_INIT_STRUCT(pPort->sPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
                pPort->sPortDef.nPortIndex = nPortIndex;
                eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & pPort->sPortDef);
                OMX_CONF_BAIL_ON_ERROR(eError);

                if (0x0 == pPort->sPortDef.nBufferCountMin)
                {
                    /* a buffer count of 0x0 is not allowed */
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined,
                                            "PortDefinition nBufferCountMin incorrect\n");
                }

                if (pPort->sPortDef.nBufferCountMin > pPort->sPortDef.nBufferCountActual)
                {
                    /* a buffer count of 0x0 is not allowed */
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined,
                                            "PortDefinition nBufferCountActual incorrect\n");
                }

                /* set the actual buffer count to min + 1 */
                pPort->sPortDef.nBufferCountActual = pPort->sPortDef.nBufferCountMin + 1;

                /* set actual buffer count on the port */
                eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & pPort->sPortDef);
                OMX_CONF_BAIL_ON_ERROR(eError);

                /* allocate LIFO with one more entry that the required minimum
                   as the test will allocate 1 more than needed to exercise
                   allocating buffers during the IDLE state */
                LIFO_ALLOC(pPort->pLifo, (pPort->sPortDef.nBufferCountActual));
                if (0x0 == pPort->pLifo)
                {
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined,
                                            "memory allocation failure\n");
                }

                pPort++;
            }

        }


OMX_CONF_TEST_BAIL:

        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE BufferTest_FreePortStructures(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_PORTTYPE *pPort;
        OMX_U32 i;

        if (0x0 != pCtx->aPorts)
        {
            pPort = pCtx->aPorts;

            for (i = 0; i < pCtx->nNumPorts; i++)
            {
                /* free all allocated buffers */
                if (OMX_ErrorNone == eError)
                {
                    /* record the return code, but don't exit on failure as the
                       test has memory it must free */
                    eError = BufferTest_PortFreeNumBuffers(pCtx, pPort, LIFO_INQUEUE(pPort->pLifo));

                }
                else
                {
                    /* preserve the first failure from freeing buffers */
                    BufferTest_PortFreeNumBuffers(pCtx, pPort, LIFO_INQUEUE(pPort->pLifo));
                }

                /* free LIFO */
                LIFO_FREE(pPort->pLifo);
                pPort->pLifo = 0x0;
                pPort++;
            }

            OMX_OSAL_Free(pCtx->aPorts);
            pCtx->aPorts = 0x0;
        }


        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE BufferTest_AllocateAllButOne(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_PORTTYPE *pPort;
        OMX_U32 i;

        pPort = pCtx->aPorts;

        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Allocating all buffers but one on port %i\n",
                           pPort->sPortDef.nPortIndex);
            /* allocate all but one buffer on each port */
            eError = BufferTest_PortAllocateNumBuffers(pCtx, pPort,
                     (pPort->sPortDef.nBufferCountMin - 1));
            OMX_CONF_BAIL_ON_ERROR(eError);
            pPort++;
        }


OMX_CONF_TEST_BAIL:

        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE BufferTest_AllocateAllPlusOne(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_PORTTYPE *pPort;
        OMX_U32 i;
        OMX_U32 nCount;

        pPort = pCtx->aPorts;

        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Allocating all buffers plus one on port %i\n",
                           pPort->sPortDef.nPortIndex);
            /* allocate all buffers on each port, but plus buffer */
            nCount = pPort->sPortDef.nBufferCountActual;
            nCount -= LIFO_INQUEUE(pPort->pLifo);
            eError = BufferTest_PortAllocateNumBuffers(pCtx, pPort, nCount);
            OMX_CONF_BAIL_ON_ERROR(eError);
            pPort++;
        }


OMX_CONF_TEST_BAIL:

        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE BufferTest_FreeAll(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_PORTTYPE *pPort;
        OMX_U32 i;

        pPort = pCtx->aPorts;

        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Freeing all buffers on port %i\n",
                           pPort->sPortDef.nPortIndex);
            /* free all buffers on each port, but one buffer */
            eError = BufferTest_PortFreeNumBuffers(pCtx, pPort,
                                                   LIFO_INQUEUE(pPort->pLifo));
            OMX_CONF_BAIL_ON_ERROR(eError);
            pPort++;
        }


OMX_CONF_TEST_BAIL:

        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE BufferTest_FreeAllButOne(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_PORTTYPE *pPort;
        OMX_U32 i;

        pPort = pCtx->aPorts;

        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Freeing all buffers but one on port %i\n",
                           pPort->sPortDef.nPortIndex);
            /* free all buffers on each port, but one buffer */
            eError = BufferTest_PortFreeNumBuffers(pCtx, pPort,
                                                   (LIFO_INQUEUE(pPort->pLifo) - 1));
            OMX_CONF_BAIL_ON_ERROR(eError);
            pPort++;
        }


OMX_CONF_TEST_BAIL:

        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE OMX_CONF_BufferTest(OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_ERRORTYPE eCleanupError = OMX_ErrorNone;
        TEST_CTXTYPE ctx;
        TEST_CTXTYPE *pCtx;
        OMX_HANDLETYPE hComp  = 0x0;
        OMX_CALLBACKTYPE oCallbacks;
        OMX_HANDLETYPE hWrappedComp = 0x0;
        OMX_CALLBACKTYPE *pWrappedCallbacks;
        OMX_PTR pWrappedAppData;
        OMX_BOOL bTimeout;
        TEST_PORTTYPE *pPort;
        OMX_U32 i;
        OMX_U32 nPortIndex;
        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;

        oCallbacks.EventHandler    =  BufferTest_EventHandler;
        oCallbacks.EmptyBufferDone =  StubbedEmptyBufferDone;
        oCallbacks.FillBufferDone  =  StubbedFillBufferDone;

        pCtx = &ctx;
        pCtx->hStateChangeEvent = 0x0;
        pCtx->hPortDisableEvent = 0x0;
        pCtx->hPortEnableEvent = 0x0;
        pCtx->hPortErrorEvent = 0x0;
        pCtx->aPorts = 0x0;
        pCtx->bRestartAllPorts = OMX_FALSE;
        pCtx->bStopAllPorts = OMX_FALSE;

        /* initialize events to track callbacks */
        OMX_OSAL_EventCreate(&pCtx->hStateChangeEvent);
        OMX_OSAL_EventReset(pCtx->hStateChangeEvent);
        OMX_OSAL_EventCreate(&pCtx->hPortDisableEvent);
        OMX_OSAL_EventReset(pCtx->hPortDisableEvent);
        OMX_OSAL_EventCreate(&pCtx->hPortEnableEvent);
        OMX_OSAL_EventReset(pCtx->hPortEnableEvent);
        OMX_OSAL_EventCreate(&pCtx->hPortErrorEvent);
        OMX_OSAL_EventReset(pCtx->hPortErrorEvent);

        eError = OMX_CONF_CallbackTracerCreate(&oCallbacks, (OMX_PTR)pCtx, cComponentName,
                                               &pWrappedCallbacks, &pWrappedAppData);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Initialize OpenMax */
        eError = OMX_Init();
        OMX_CONF_BAIL_ON_ERROR(eError);

        eError = OMX_GetHandle(&hComp, cComponentName, pWrappedAppData, pWrappedCallbacks);
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = OMX_CONF_ComponentTracerCreate(hComp, cComponentName, &hWrappedComp);
        OMX_CONF_BAIL_ON_ERROR(eError);

        pCtx->hWrappedComp = hWrappedComp;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "detected all port on component %s\n", cComponentName);
        /* inspect component's ports */
        OMX_CONF_INIT_STRUCT(pCtx->sPortParam[0], OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(hWrappedComp, OMX_IndexParamAudioInit, (OMX_PTR) & pCtx->sPortParam[0]);
        if (OMX_ErrorUnsupportedIndex == eError)  eError = OMX_ErrorNone;
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "detected %i audio ports starting at %i \n",
                       pCtx->sPortParam[0].nPorts, pCtx->sPortParam[0].nStartPortNumber);

        /* detect all video ports on the component */
        OMX_CONF_INIT_STRUCT(pCtx->sPortParam[1], OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(hWrappedComp, OMX_IndexParamVideoInit, (OMX_PTR) & pCtx->sPortParam[1]);
        if (OMX_ErrorUnsupportedIndex == eError)  eError = OMX_ErrorNone;
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "detected %i video ports starting at %i \n",
                       pCtx->sPortParam[1].nPorts, pCtx->sPortParam[1].nStartPortNumber);

        /* detect all image ports on the component */
        OMX_CONF_INIT_STRUCT(pCtx->sPortParam[2], OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(hWrappedComp, OMX_IndexParamImageInit, (OMX_PTR) & pCtx->sPortParam[2]);
        if (OMX_ErrorUnsupportedIndex == eError)  eError = OMX_ErrorNone;
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "detected %i image ports starting at %i \n",
                       pCtx->sPortParam[2].nPorts, pCtx->sPortParam[2].nStartPortNumber);

        /* detect all other ports on the component */
        OMX_CONF_INIT_STRUCT(pCtx->sPortParam[3], OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(hWrappedComp, OMX_IndexParamOtherInit, (OMX_PTR) & pCtx->sPortParam[3]);
        if (OMX_ErrorUnsupportedIndex == eError)  eError = OMX_ErrorNone;
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "detected %i other ports starting at %i \n",
                       pCtx->sPortParam[3].nPorts, pCtx->sPortParam[3].nStartPortNumber);

        /* record total number of ports and allocate port structures */
        pCtx->nNumPorts = pCtx->sPortParam[0].nPorts +
                          pCtx->sPortParam[1].nPorts +
                          pCtx->sPortParam[2].nPorts +
                          pCtx->sPortParam[3].nPorts;

        if (0x0 == pCtx->nNumPorts)
        {
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Component has no ports\n");
        }

        /* allocate port structures */
        eError = BufferTest_AllocatePortStructures(pCtx);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* transition component to idle, verifying that the component does not
           go to IDLE within a certain timeout */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Command component to OMX_StateIdle\n");
        OMX_OSAL_EventReset(pCtx->hStateChangeEvent);
        eError = OMX_SendCommand(hWrappedComp, OMX_CommandStateSet, OMX_StateIdle, 0x0);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* allocate some buffers on the ports, vertifying the component does
           not transition to idle */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Allocate some buffers on all ports\n");
        eError = BufferTest_AllocateAllButOne(pCtx);
        OMX_CONF_BAIL_ON_ERROR(eError);

        OMX_OSAL_EventWait(pCtx->hStateChangeEvent, OMX_CONF_TIMEOUT_EXPECTING_FAILURE, &bTimeout);
        if (OMX_FALSE == bTimeout)
        {
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Unexpected state transition to OMX_StateIdle\n");
        }

        /* allocate all buffers on the ports, vertifying the component does
           transition to idle */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Allocate remaining buffers on all ports\n");
        eError = BufferTest_AllocateAllPlusOne(pCtx);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_EventWait(pCtx->hStateChangeEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
        if (OMX_TRUE == bTimeout)
        {
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "State transition to OMX_StateIdle never occured\n");
        }

        /* transition the component to from idle to loaded, verifying the component
           does not transition */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Command component to OMX_StateLoaded\n");
        OMX_OSAL_EventReset(pCtx->hStateChangeEvent);
        eError = OMX_SendCommand(hWrappedComp, OMX_CommandStateSet, OMX_StateLoaded, 0x0);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* free some buffers */
        eError = BufferTest_FreeAllButOne(pCtx);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_EventWait(pCtx->hStateChangeEvent, OMX_CONF_TIMEOUT_EXPECTING_FAILURE, &bTimeout);
        if (OMX_FALSE == bTimeout)
        {
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Unexpected state transition to OMX_StateLoaded\n");
        }

        /* free all buffers and confirm component transitions to loaded */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Free buffers on all ports\n");
        eError = BufferTest_FreeAll(pCtx);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_EventWait(pCtx->hStateChangeEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
        if (OMX_TRUE == bTimeout)
        {
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Expected state transition to OMX_StateLoaded\n");
        }

        /* stop all ports */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Command all ports to disabled\n");
        pCtx->bStopAllPorts = OMX_TRUE;
        pCtx->nNumPortsStopped = 0x0;
        OMX_OSAL_EventReset(pCtx->hPortDisableEvent);
        eError = OMX_SendCommand(hWrappedComp, OMX_CommandPortDisable, OMX_ALL, 0x0);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_EventWait(pCtx->hPortDisableEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
        if (OMX_TRUE == bTimeout)
        {
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "All ports not disabled\n");
        }
        pCtx->bStopAllPorts = OMX_FALSE;

        /* set the component to idle */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Command component to OMX_StateIdle\n");
        OMX_OSAL_EventReset(pCtx->hStateChangeEvent);
        eError = OMX_SendCommand(hWrappedComp, OMX_CommandStateSet, OMX_StateIdle, 0x0);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_EventWait(pCtx->hStateChangeEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
        if (OMX_TRUE == bTimeout)
        {
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Expected state transition to OMX_StateIdle\n");
        }

        pPort = pCtx->aPorts;

        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            /* verify port enable and populated status */
            nPortIndex =  pPort->sPortDef.nPortIndex;
            OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
            sPortDefinition.nPortIndex = nPortIndex;
            eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition,
                                      (OMX_PTR) & sPortDefinition);
            OMX_CONF_BAIL_ON_ERROR(eError);

            OMX_CONF_ASSERT(eError, (OMX_FALSE == sPortDefinition.bEnabled),
                            "Incorrect value in PortDefinition bEnabled field\n");
            OMX_CONF_ASSERT(eError, (OMX_FALSE == sPortDefinition.bPopulated),
                            "Incorrect value in PortDefinition bPopulated field\n");

            /* enable port, and verify it does not restart */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Command port %i to enabled\n", nPortIndex);
            OMX_OSAL_EventReset(pCtx->hPortEnableEvent);
            pCtx->nRestartPort = nPortIndex;
            eError = OMX_SendCommand(hWrappedComp, OMX_CommandPortEnable, nPortIndex, 0x0);
            OMX_CONF_BAIL_ON_ERROR(eError);
            OMX_OSAL_EventWait(pCtx->hPortEnableEvent, OMX_CONF_TIMEOUT_EXPECTING_FAILURE, &bTimeout);
            if (OMX_FALSE == bTimeout)
            {
                OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Port enabled unexpectedly\n");
            }

            /* verify port enabled, but not populated */
            OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
            sPortDefinition.nPortIndex = nPortIndex;
            eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition,
                                      (OMX_PTR) & sPortDefinition);
            OMX_CONF_BAIL_ON_ERROR(eError);

            OMX_CONF_ASSERT(eError, (OMX_TRUE == sPortDefinition.bEnabled),
                            "Incorrect value in PortDefinition bEnabled field\n");
            OMX_CONF_ASSERT(eError, (OMX_FALSE == sPortDefinition.bPopulated),
                            "Incorrect value in PortDefinition bPopulated field\n");

            /* allocate buffers on the port, and verify it restarts */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Allocate buffers on port %i\n", nPortIndex);
            eError = BufferTest_PortAllocateNumBuffers(pCtx, pPort, (pPort->sPortDef.nBufferCountActual));
            OMX_CONF_BAIL_ON_ERROR(eError);
            OMX_OSAL_EventWait(pCtx->hPortEnableEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
            if (OMX_TRUE == bTimeout)
            {
                OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Port never enabled\n");
            }

            /* verify port enabled and populated */
            OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
            sPortDefinition.nPortIndex = nPortIndex;
            eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition,
                                      (OMX_PTR) & sPortDefinition);
            OMX_CONF_BAIL_ON_ERROR(eError);

            OMX_CONF_ASSERT(eError, (OMX_TRUE == sPortDefinition.bEnabled),
                            "Incorrect value in PortDefinition bEnabled field\n");
            OMX_CONF_ASSERT(eError, (OMX_TRUE == sPortDefinition.bPopulated),
                            "Incorrect value in PortDefinition bPopulated field\n");

            /* stop port, verifying it does not stop */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Command port %i to disabled\n", nPortIndex);
            OMX_OSAL_EventReset(pCtx->hPortDisableEvent);
            pCtx->nStopPort = nPortIndex;
            eError = OMX_SendCommand(hWrappedComp, OMX_CommandPortDisable, nPortIndex, 0x0);
            OMX_CONF_BAIL_ON_ERROR(eError);
            OMX_OSAL_EventWait(pCtx->hPortDisableEvent, OMX_CONF_TIMEOUT_EXPECTING_FAILURE, &bTimeout);
            if (OMX_FALSE == bTimeout)
            {
                OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Port disabled unexpectedly\n");
            }

            /* free buffers on the port, verifying it stops */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Free buffers on port %i\n", nPortIndex);
            eError = BufferTest_PortFreeNumBuffers(pCtx, pPort, LIFO_INQUEUE(pPort->pLifo));
            OMX_CONF_BAIL_ON_ERROR(eError);
            OMX_OSAL_EventWait(pCtx->hPortDisableEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
            if (OMX_TRUE == bTimeout)
            {
                OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Port never disabled\n");
            }

            /* verify port not enabled, not populated */
            OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
            sPortDefinition.nPortIndex = nPortIndex;
            eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition,
                                      (OMX_PTR) & sPortDefinition);
            OMX_CONF_BAIL_ON_ERROR(eError);

            OMX_CONF_ASSERT(eError, (OMX_FALSE == sPortDefinition.bEnabled),
                            "Incorrect value in PortDefinition bEnabled field\n");
            OMX_CONF_ASSERT(eError, (OMX_FALSE == sPortDefinition.bPopulated),
                            "Incorrect value in PortDefinition bPopulated field\n");

            /* enable the port and allocate buffers */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Command port %i to enabled\n", nPortIndex);
            OMX_OSAL_EventReset(pCtx->hPortEnableEvent);
            eError = OMX_SendCommand(hWrappedComp, OMX_CommandPortEnable, nPortIndex, 0x0);
            OMX_CONF_BAIL_ON_ERROR(eError);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Allocate buffers on port %i\n", nPortIndex);
            eError = BufferTest_PortAllocateNumBuffers(pCtx, pPort, (pPort->sPortDef.nBufferCountActual));
            OMX_CONF_BAIL_ON_ERROR(eError);
            OMX_OSAL_EventWait(pCtx->hPortEnableEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
            if (OMX_TRUE == bTimeout)
            {
                OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Port never enabled\n");
            }

            /* test the port errors when buffers are freed */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Free buffers on port %i\n", nPortIndex);
            OMX_OSAL_EventReset(pCtx->hPortErrorEvent);
            eError = BufferTest_PortFreeNumBuffers(pCtx, pPort, LIFO_INQUEUE(pPort->pLifo));
            OMX_CONF_BAIL_ON_ERROR(eError);

            /* waiting for component to report error due to lost buffers */
            OMX_OSAL_EventWait(pCtx->hPortErrorEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
            if (OMX_TRUE == bTimeout)
            {
                OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined,
                                        "Component did not report OMX_ErrorPortUnpopulated\n");
            }

            /* verify port enabled, but not populated */
            OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
            sPortDefinition.nPortIndex = nPortIndex;
            eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition,
                                      (OMX_PTR) & sPortDefinition);
            OMX_CONF_BAIL_ON_ERROR(eError);

            OMX_CONF_ASSERT(eError, (OMX_TRUE == sPortDefinition.bEnabled),
                            "Incorrect value in PortDefinition bEnabled field\n");
            OMX_CONF_ASSERT(eError, (OMX_FALSE == sPortDefinition.bPopulated),
                            "Incorrect value in PortDefinition bPopulated field\n");

            /* stop the port */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Command port %i to disabled\n", nPortIndex);
            OMX_OSAL_EventReset(pCtx->hPortDisableEvent);
            pCtx->nStopPort = nPortIndex;
            eError = OMX_SendCommand(hWrappedComp, OMX_CommandPortDisable, nPortIndex, 0x0);
            OMX_OSAL_EventWait(pCtx->hPortDisableEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
            if (OMX_TRUE == bTimeout)
            {
                OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Port never disabled\n");
            }

            /* verify port not enabled, but not populated */
            OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
            sPortDefinition.nPortIndex = nPortIndex;
            eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition,
                                      (OMX_PTR) & sPortDefinition);
            OMX_CONF_BAIL_ON_ERROR(eError);

            OMX_CONF_ASSERT(eError, (OMX_FALSE == sPortDefinition.bEnabled),
                            "Incorrect value in PortDefinition bEnabled field\n");
            OMX_CONF_ASSERT(eError, (OMX_FALSE == sPortDefinition.bPopulated),
                            "Incorrect value in PortDefinition bPopulated field\n");

            pPort++;
        }

        /* transition component from idle to loaded */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Command component to OMX_StateLoaded\n");
        OMX_OSAL_EventReset(pCtx->hStateChangeEvent);
        eError = OMX_SendCommand(hWrappedComp, OMX_CommandStateSet, OMX_StateLoaded, 0x0);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_EventWait(pCtx->hStateChangeEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
        if (OMX_TRUE == bTimeout)
        {
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Expected state transition to OMX_StateLoaded\n");
        }

        /* restart all ports */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Command all ports to enable\n");
        pCtx->bRestartAllPorts = OMX_TRUE;
        pCtx->nNumPortsRestarted = 0x0;
        OMX_OSAL_EventReset(pCtx->hPortEnableEvent);
        eError = OMX_SendCommand(hWrappedComp, OMX_CommandPortEnable, OMX_ALL, 0x0);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_EventWait(pCtx->hPortEnableEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
        if (OMX_TRUE == bTimeout)
        {
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "All ports not enabled\n");
        }
        pCtx->bRestartAllPorts = OMX_FALSE;


OMX_CONF_TEST_BAIL:

        eCleanupError = BufferTest_FreePortStructures(pCtx);

        if (hWrappedComp)
        {
            OMX_CONF_ComponentTracerDestroy(hWrappedComp);
        }

        if (hComp)
        {
            if (OMX_ErrorNone == eCleanupError)
            {
                eCleanupError = OMX_FreeHandle(hComp);

            }
            else
            {
                OMX_FreeHandle(hComp);
            }
        }

        if (OMX_ErrorNone != eCleanupError)
        {
            OMX_Deinit();

        }
        else
        {
            eCleanupError = OMX_Deinit();
        }

        OMX_OSAL_EventDestroy(pCtx->hStateChangeEvent);
        OMX_OSAL_EventDestroy(pCtx->hPortDisableEvent);
        OMX_OSAL_EventDestroy(pCtx->hPortEnableEvent);
        OMX_OSAL_EventDestroy(pCtx->hPortErrorEvent);

        if (OMX_ErrorNone == eError)
        {
            /* if there were no failures during the test, report any errors found
               during cleanup */
            eError = eCleanupError;
        }

        return(eError);
    }


#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */

