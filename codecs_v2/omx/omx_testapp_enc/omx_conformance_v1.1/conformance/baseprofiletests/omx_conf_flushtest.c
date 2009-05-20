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

/** OMX_CONF_FlushTest.c
 *  OpenMax IL conformance test - Flush Test
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
     *  D E C L A R A T I O N S
     */

#define TEST_NAME_STRING "FlushTest"
#define TEST_COMPONENT_NAME_SIZE 128
#define TEST_NUM_BUFFERS_TO_PROCESS 100

#define NUM_DOMAINS 0x4

    static char szDesc[256];
    static char szState[256];



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

#define OMX_CONF_WAIT_STATE(_p_, _s_, _e_)\
{\
    OMX_BOOL _bTimeout_;\
    OMX_OSAL_EventWait((_p_)->hStateChangeEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &_bTimeout_);\
    if (OMX_TRUE == _bTimeout_)\
    {\
        OMX_CONF_StateToString(_s_, szState);\
        OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "State transition to %s never occured\n", szState);\
        _e_ = OMX_ErrorUndefined;\
        goto OMX_CONF_TEST_BAIL;\
    }\
    _e_ = OMX_GetState((_p_)->hWrappedComp, &(_p_)->eState);\
    OMX_CONF_BAIL_ON_ERROR(_e_);\
    if (_s_ != (_p_)->eState)\
    {\
        OMX_CONF_StateToString(_s_, szState);\
        OMX_CONF_StateToString((_p_)->eState, szDesc);\
        OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "OMX_GetState reported %s, expecting %s\n",\
                       szDesc, szState);\
        _e_ = OMX_ErrorUndefined;\
        goto OMX_CONF_TEST_BAIL;\
    }\
}

#define OMX_CONF_SET_STATE(_p_, _s_, _e_)\
{\
    OMX_CONF_StateToString(_s_, szState);\
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Command component to %s\n", szState);\
    OMX_OSAL_EventReset((_p_)->hStateChangeEvent);\
    _e_ = OMX_SendCommand((_p_)->hWrappedComp, OMX_CommandStateSet, _s_, 0x0);\
    OMX_CONF_BAIL_ON_ERROR(_e_);\
}

#define OMX_CONF_SET_STATE_AND_WAIT(_p_, _s_, _e_)\
    OMX_CONF_SET_STATE(_p_, _s_, _e_);\
    OMX_CONF_WAIT_STATE(_p_, _s_, _e_)

#define Q_ALLOC(_p_, _n_)\
    _p_ = (QTYPE*)OMX_OSAL_Malloc(sizeof(QTYPE) + (sizeof(OMX_PTR) * (_n_ + 1)));\
    if (0x0 != _p_)\
    {\
        _p_->nNumEntries = _n_;\
        _p_->nEnd = 0;\
        _p_->nStart = 0;\
        _p_->nInQueue = 0;\
    }

#define Q_FREE(_p_)\
    if (0x0 != _p_)\
    {\
        OMX_OSAL_Free(_p_);\
    }\
    _p_ = 0x0

#define Q_ADD(_p_, _e_)\
    _p_->aEntires[_p_->nStart++] = (OMX_PTR)_e_;\
    if (_p_->nStart == _p_->nNumEntries) _p_->nStart = 0;\
    _p_->nInQueue++

#define Q_PEEK(_p_, _e_)\
    if (0x0 != _p_->nInQueue)\
    {\
        _e_ = _p_->aEntires[_p_->nEnd];\
    }

#define Q_REMOVE(_p_, _e_)\
    if (0x0 != _p_->nInQueue)\
    {\
        _e_ = _p_->aEntires[_p_->nEnd++];\
        if (_p_->nEnd == _p_->nNumEntries) _p_->nEnd = 0;\
        _p_->nInQueue--;\
    }

#define Q_INQUEUE(_p_) _p_->nInQueue

    /*
     *     D E F I N I T I O N S
     */

    typedef struct _TEST_CTXTYPE TEST_CTXTYPE;

    typedef struct _QTYPE
    {
        OMX_U32 nNumEntries;
        OMX_U32 nEnd;
        OMX_U32 nStart;
        OMX_U32 nInQueue;
        OMX_PTR aEntires[1];

    } QTYPE;

    typedef struct _TEST_PORTTYPE
    {
        TEST_CTXTYPE *pCtx;
        OMX_PARAM_PORTDEFINITIONTYPE sPortDef;
        QTYPE *pQ;
        OMX_U32 nBuffersOutstanding;
        OMX_BOOL bEOS;
        OMX_BOOL bOpenFile;
        OMX_U32 nNumSaveBufferOrder;
        OMX_PTR *aSaveBufferOrder;

    } TEST_PORTTYPE;

    struct _TEST_CTXTYPE
    {
        OMX_STATETYPE eState;
        OMX_HANDLETYPE hWrappedComp;
        OMX_U32 nNumPorts;
        OMX_PORT_PARAM_TYPE sPortParam[NUM_DOMAINS];
        OMX_HANDLETYPE hStateChangeEvent;
        OMX_HANDLETYPE hPortStopEvent;
        OMX_HANDLETYPE hPortRestartEvent;
        OMX_HANDLETYPE hPortFlushEvent;
        OMX_HANDLETYPE hBufferCallbackEvent;
        OMX_U32 nBuffersProcessed;
        OMX_U32 nNumPortsStopped;
        OMX_BOOL bStopAllPorts;
        OMX_U32 nStopPort;
        OMX_U32 nNumPortsRestarted;
        OMX_BOOL bRestartAllPorts;
        OMX_U32 nRestartPort;
        OMX_U32 nNumPortsFlushed;
        OMX_BOOL bFlushAllPorts;
        OMX_U32 nFlushPort;
        TEST_PORTTYPE *aPorts;

    };


    /*
     *  E X T E R N A L   F U N C T I O N S
     */

    OMX_ERRORTYPE OMX_CONF_CommandToString(
        OMX_IN OMX_COMMANDTYPE eCommand,
        OMX_OUT OMX_STRING sName);

    OMX_ERRORTYPE OMX_CONF_StateToString(
        OMX_IN OMX_STATETYPE eState,
        OMX_OUT OMX_STRING sName);

    OMX_ERRORTYPE FlushTest_FindPortFromIndex(
        TEST_CTXTYPE *pCtx,
        TEST_PORTTYPE **ppPort,
        OMX_U32 nPortIndex);


    /*****************************************************************************/
    OMX_ERRORTYPE FlushTest_EmptyBufferDone(
        OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_BUFFERHEADERTYPE* pBufHdr)
    {
        TEST_CTXTYPE *pCtx;
        TEST_PORTTYPE *pPort;

        UNUSED_PARAMETER(hComponent);
        UNUSED_PARAMETER(pAppData);

        if (0x0 != pBufHdr->pAppPrivate)
        {
            pPort = (TEST_PORTTYPE*)pBufHdr->pAppPrivate;
            pCtx = pPort->pCtx;

            /* EmptyBufferDone can only be called from an input port */
            if (OMX_DirInput == pPort->sPortDef.eDir)
            {
                /* when the queue is full, don't add another buffer header
                   as something is wrong with the component.  It is
                   incorrectly returning more buffers than sent in */
                if (Q_INQUEUE(pPort->pQ) < pPort->sPortDef.nBufferCountActual)
                {
                    Q_ADD(pPort->pQ, pBufHdr);
                    pPort->nBuffersOutstanding--;
                    pCtx->nBuffersProcessed++;
                    OMX_OSAL_EventSet(pCtx->hBufferCallbackEvent);
                }

            }

        }

        return OMX_ErrorNone;
    }


    /*****************************************************************************/
    OMX_ERRORTYPE FlushTest_FillBufferDone(
        OMX_OUT OMX_HANDLETYPE hComponent,
        OMX_OUT OMX_PTR pAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* pBufHdr)
    {
        TEST_CTXTYPE *pCtx;
        TEST_PORTTYPE *pPort;

        UNUSED_PARAMETER(hComponent);
        UNUSED_PARAMETER(pAppData);

        if (0x0 != pBufHdr->pAppPrivate)
        {
            pPort = (TEST_PORTTYPE*)pBufHdr->pAppPrivate;
            pCtx = pPort->pCtx;

            /* FillBufferDone can only be called from an output port */
            if (OMX_DirOutput == pPort->sPortDef.eDir)
            {
                /* when the queue is full, don't add another buffer header
                   as something is wrong with the component.  It is
                   incorrectly returning more buffers than sent in */
                if (Q_INQUEUE(pPort->pQ) < pPort->sPortDef.nBufferCountActual)
                {
                    /* write to file if enabled */
                    if (OMX_TRUE == pPort->bOpenFile)
                    {
                        OMX_OSAL_WriteToOutputFile((pBufHdr->pBuffer + pBufHdr->nOffset),
                                                   pBufHdr->nFilledLen, pPort->sPortDef.nPortIndex);
                    }

                    Q_ADD(pPort->pQ, pBufHdr);
                    pPort->nBuffersOutstanding--;
                    pCtx->nBuffersProcessed++;
                    OMX_OSAL_EventSet(pCtx->hBufferCallbackEvent);
                }

            }

        }

        return OMX_ErrorNone;
    }


    /*****************************************************************************/
    OMX_ERRORTYPE FlushTest_EventHandler(
        OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_EVENTTYPE eEvent,
        OMX_IN OMX_U32 nData1,
        OMX_IN OMX_U32 nData2,
        OMX_IN OMX_PTR pEventData)
    {
        TEST_CTXTYPE *pCtx;
        TEST_PORTTYPE *pPort;

        UNUSED_PARAMETER(hComponent);
        UNUSED_PARAMETER(pEventData);

        if (0x0 == pAppData) return(OMX_ErrorNone);
        pCtx = (TEST_CTXTYPE*)pAppData;


        if (eEvent == OMX_EventCmdComplete)
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
                    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i completed stopping\n", nData2);
                    if (OMX_TRUE == pCtx->bStopAllPorts)
                    {
                        pCtx->nNumPortsStopped++;
                        if (pCtx->nNumPortsStopped == pCtx->nNumPorts)
                        {
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "All ports completed stopping\n");
                            OMX_OSAL_EventSet(pCtx->hPortStopEvent);
                        }

                    }
                    else if (pCtx->nStopPort == nData2)
                    {
                        OMX_OSAL_EventSet(pCtx->hPortStopEvent);
                    }
                    break;
                case OMX_CommandPortEnable:
                    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port completed %i restart\n", nData2);
                    if (OMX_TRUE == pCtx->bRestartAllPorts)
                    {
                        pCtx->nNumPortsRestarted++;
                        if (pCtx->nNumPortsRestarted == pCtx->nNumPorts)
                        {
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "All ports completed restart\n");
                            OMX_OSAL_EventSet(pCtx->hPortRestartEvent);
                        }

                    }
                    else if (pCtx->nRestartPort == nData2)
                    {
                        OMX_OSAL_EventSet(pCtx->hPortRestartEvent);
                    }
                    break;
                case OMX_CommandFlush:
                    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i reported flush complete\n", nData2);
                    if (OMX_TRUE == pCtx->bFlushAllPorts)
                    {
                        /* only record the por flush command if all buffers from the port were
                           received as part of the flush */
                        if (OMX_ErrorNone == FlushTest_FindPortFromIndex(pCtx, &pPort, nData2))
                        {
                            if (Q_INQUEUE(pPort->pQ) == pPort->sPortDef.nBufferCountActual)
                            {
                                pCtx->nNumPortsFlushed++;
                                if (pCtx->nNumPortsFlushed == pCtx->nNumPorts)
                                {
                                    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "All ports completed flush\n");
                                    OMX_OSAL_EventSet(pCtx->hPortFlushEvent);
                                }

                            }
                            else
                            {
                                OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR,
                                               "Port reported flush complete without flushing all buffers\n");
                            }

                        }

                    }
                    else if (pCtx->nFlushPort == nData2)
                    {
                        if (OMX_ErrorNone == FlushTest_FindPortFromIndex(pCtx, &pPort, nData2))
                        {
                            if (Q_INQUEUE(pPort->pQ) == pPort->sPortDef.nBufferCountActual)
                            {
                                OMX_OSAL_EventSet(pCtx->hPortFlushEvent);

                            }
                            else
                            {
                                OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR,
                                               "Port reported flush complete without flushing all buffers\n");
                            }

                        }

                    }
                    break;
                case OMX_EventBufferFlag:
                    break;
                default:
                    break;
            }
        }

        return OMX_ErrorNone;
    }


    /*****************************************************************************/
    OMX_ERRORTYPE FlushTest_FindPortFromIndex(
        TEST_CTXTYPE *pCtx,
        TEST_PORTTYPE **ppPort,
        OMX_U32 nPortIndex)
    {
        OMX_ERRORTYPE eError = OMX_ErrorUndefined;
        TEST_PORTTYPE *pPort;
        OMX_U32 i;

        *ppPort = 0x0;
        pPort = pCtx->aPorts;

        if (0x0 != pPort)
        {
            for (i = 0; i < pCtx->nNumPorts; i++)
            {
                if (pPort->sPortDef.nPortIndex == nPortIndex)
                {
                    *ppPort = pPort;
                    eError = OMX_ErrorNone;
                    break;
                }

                pPort++;
            }

        }

        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE FlushTest_FindBogusPortIndex(
        TEST_CTXTYPE *pCtx,
        OMX_U32 *pPortIndex)
    {
        TEST_PORTTYPE *pPort;
        OMX_U32 i;
        OMX_U32 nBogusPort = 0x0;

        if ((0x0 == pCtx->nNumPorts) || (0x0 == pCtx->aPorts))
        {
            return(OMX_ErrorUndefined);
        }

        pPort = pCtx->aPorts;

        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            /* loop through all the port index's to find the maximum port
               index value, and use that +1 as a invalid port index */
            if (nBogusPort < pPort->sPortDef.nPortIndex)
            {
                nBogusPort = pPort->sPortDef.nPortIndex;
            }

            pPort++;
        }
        nBogusPort++;
        *pPortIndex = nBogusPort;

        return(OMX_ErrorNone);
    }

    /* JVW-NVIDIA: Reset port stream for http://cvs.khronos.org/bugzilla/show_bug.cgi?id=223 */

    /*****************************************************************************/
    OMX_ERRORTYPE FlushTest_ResetPortStream(
        TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_PORTTYPE *pPort;
        OMX_U32 i;

        pPort = pCtx->aPorts;

        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            pPort->bEOS = OMX_FALSE;

            if (OMX_TRUE == pPort->bOpenFile)
            {
                /* only close and reopen files for input ports */
                if (OMX_DirInput == pPort->sPortDef.eDir)
                {
                    eError = OMX_OSAL_CloseInputFile(pPort->sPortDef.nPortIndex);
                    OMX_CONF_BAIL_ON_ERROR(eError);

                    pPort->bOpenFile = OMX_FALSE;
                    eError = OMX_OSAL_OpenInputFile(pPort->sPortDef.nPortIndex);
                    OMX_CONF_BAIL_ON_ERROR(eError);
                    pPort->bOpenFile = OMX_TRUE;
                }

            }

            pPort++;
        }


OMX_CONF_TEST_BAIL:

        return(eError);
    }

    /*****************************************************************************/
    OMX_ERRORTYPE FlushTest_SaveBufferOrder(
        TEST_PORTTYPE *pPort)
    {
        OMX_BUFFERHEADERTYPE *pBufHdr = NULL;
        OMX_BUFFERHEADERTYPE **pBufOrder;
        OMX_U32 i;

        if ((0x0 != pPort) && (0x0 != pPort->aSaveBufferOrder))
        {
            pPort->nNumSaveBufferOrder = Q_INQUEUE(pPort->pQ);
            pBufOrder = (OMX_BUFFERHEADERTYPE**)pPort->aSaveBufferOrder;
            for (i = 0; i < pPort->nNumSaveBufferOrder; i++)
            {
                Q_REMOVE(pPort->pQ, pBufHdr);
                *pBufOrder = pBufHdr;
                pBufOrder++;
                Q_ADD(pPort->pQ, pBufHdr);
            }

        }

        return(OMX_ErrorNone);

    }


    /*****************************************************************************/
    OMX_ERRORTYPE FlushTest_CheckBufferOrder(
        TEST_PORTTYPE *pPort)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_BUFFERHEADERTYPE *pBufHdr = NULL;
        OMX_BUFFERHEADERTYPE **pBufOrder;
        OMX_U32 i;

        if ((0x0 != pPort) && (0x0 != pPort->aSaveBufferOrder))
        {
            pBufOrder = (OMX_BUFFERHEADERTYPE**)pPort->aSaveBufferOrder;
            for (i = 0; i < pPort->nNumSaveBufferOrder; i++)
            {
                Q_REMOVE(pPort->pQ, pBufHdr);
                if (*pBufOrder != pBufHdr)
                {
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "buffers returned out of order\n");
                }
                pBufOrder++;
                Q_ADD(pPort->pQ, pBufHdr);
            }

        }


OMX_CONF_TEST_BAIL:

        return(eError);
    }

    /* JVW-NVIDIA: Use real buffer not local copy to fix http://cvs.khronos.org/bugzilla/show_bug.cgi?id=222 */

    /*****************************************************************************/
    OMX_ERRORTYPE FlushTest_SendBogusBuffers(
        TEST_CTXTYPE *pCtx,
        TEST_PORTTYPE *pPort)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_BUFFERHEADERTYPE *pBufHdr = NULL;
        OMX_U32 nBogusPortIndex;
        OMX_U32 nVersion;
        OMX_U32 nPortIndex;

        /* construct bogus buffer header */
        Q_PEEK(pPort->pQ, pBufHdr);

        /* check bogus size in buffer header structure */
        pBufHdr->nSize -= 4;

        if (OMX_DirInput == pPort->sPortDef.eDir)
        {
            eError = OMX_EmptyThisBuffer(pCtx->hWrappedComp, pBufHdr);

        }
        else
        {
            eError = OMX_FillThisBuffer(pCtx->hWrappedComp, pBufHdr);

        }

        if (OMX_ErrorNone == eError) eError = OMX_ErrorUndefined;
        if (OMX_ErrorBadParameter == eError) eError = OMX_ErrorNone;
        OMX_CONF_BAIL_ON_ERROR(eError);
        pBufHdr->nSize += 4;

        /* check bogus version in buffer header structure */
        nVersion = pBufHdr->nVersion.nVersion;
        pBufHdr->nVersion.nVersion = 0x01020304;
        if (OMX_DirInput == pPort->sPortDef.eDir)
        {
            eError = OMX_EmptyThisBuffer(pCtx->hWrappedComp, pBufHdr);

        }
        else
        {
            eError = OMX_FillThisBuffer(pCtx->hWrappedComp, pBufHdr);

        }

        if (OMX_ErrorNone == eError) eError = OMX_ErrorUndefined;
        if (OMX_ErrorVersionMismatch == eError) eError = OMX_ErrorNone;
        pBufHdr->nVersion.nVersion = nVersion;

        /* check bogus port in buffer header structure */
        eError = FlushTest_FindBogusPortIndex(pCtx, &nBogusPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        if (OMX_DirInput == pPort->sPortDef.eDir)
        {
            nPortIndex = pBufHdr->nInputPortIndex;
            pBufHdr->nInputPortIndex = nBogusPortIndex;
            eError = OMX_EmptyThisBuffer(pCtx->hWrappedComp, pBufHdr);
            pBufHdr->nInputPortIndex = nPortIndex;
        }
        else
        {
            nPortIndex = pBufHdr->nOutputPortIndex;
            pBufHdr->nOutputPortIndex = nBogusPortIndex;
            eError = OMX_FillThisBuffer(pCtx->hWrappedComp, pBufHdr);
            pBufHdr->nOutputPortIndex = nPortIndex;
        }

        if (OMX_ErrorNone == eError) eError = OMX_ErrorUndefined;
        if (OMX_ErrorBadPortIndex == eError) eError = OMX_ErrorNone;
        OMX_CONF_BAIL_ON_ERROR(eError);

OMX_CONF_TEST_BAIL:

        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE FlushTest_SendNBuffers(
        TEST_CTXTYPE *pCtx,
        TEST_PORTTYPE *pPort,
        OMX_U32 nNumBuffers,
        OMX_BOOL bReadFromInputFile)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_BUFFERHEADERTYPE *pBufHdr = NULL;

        if (OMX_DirInput == pPort->sPortDef.eDir)
        {
            while ((0x0 != Q_INQUEUE(pPort->pQ)) && (0x0 != nNumBuffers))
            {
                if (OMX_TRUE == pPort->bEOS)
                {
                    /* no more buffers to process, so fail */
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined,
                                            "End of stream on port, no more data to send\n");

                }

                Q_REMOVE(pPort->pQ, pBufHdr);
                if (OMX_TRUE == bReadFromInputFile)
                {
                    // todo - determine correct size for sending buffers
                    pBufHdr->nFilledLen = OMX_OSAL_ReadFromInputFile(pBufHdr->pBuffer,
                                          pBufHdr->nAllocLen,
                                          pBufHdr->nInputPortIndex);
                    if (pBufHdr->nFilledLen != pBufHdr->nAllocLen)
                    {
                        pBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
                        pPort->bEOS = OMX_TRUE;
                        /* JVW-NVIDIA: Reset port stream for http://cvs.khronos.org/bugzilla/show_bug.cgi?id=223 */
                        if (pBufHdr->nFlags)
                        {
                            eError = FlushTest_ResetPortStream(pCtx);
                            OMX_CONF_BAIL_ON_ERROR(eError);
                        }

                    }

                }
                eError = OMX_EmptyThisBuffer(pCtx->hWrappedComp, pBufHdr);
                OMX_CONF_BAIL_ON_ERROR(eError);
                pPort->nBuffersOutstanding++;
                nNumBuffers--;
            }

        }
        else
        {
            while ((0x0 != Q_INQUEUE(pPort->pQ)) && (0x0 != nNumBuffers))
            {
                Q_REMOVE(pPort->pQ, pBufHdr);

                /* mark buffer header as empty on OMX_FillThisBuffer calls */
                pBufHdr->nFlags = 0x0;
                pBufHdr->nFilledLen = 0x0;
                pBufHdr->hMarkTargetComponent = 0x0;
                pBufHdr->pMarkData = 0x0;
                eError = OMX_FillThisBuffer(pCtx->hWrappedComp, pBufHdr);
                OMX_CONF_BAIL_ON_ERROR(eError);
                pPort->nBuffersOutstanding++;
                nNumBuffers--;
            }

        }

OMX_CONF_TEST_BAIL:

        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE FlushTest_ProcessNBuffers(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nBufToProcess)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_PORTTYPE *pPort;
        OMX_BOOL bTimeout;
        OMX_U32 i;
        OMX_U32 nToSend;
        OMX_U32 nSent;

        /* reset buffers to process counter */
        pCtx->nBuffersProcessed = 0x0;

        while (nBufToProcess > pCtx->nBuffersProcessed)
        {
            nSent = 0x0;
            pPort = pCtx->aPorts;
            for (i = 0; i < pCtx->nNumPorts; i++)
            {
                nToSend = Q_INQUEUE(pPort->pQ);
                eError = FlushTest_SendNBuffers(pCtx, pPort, nToSend, OMX_TRUE);
                OMX_CONF_BAIL_ON_ERROR(eError);
                nSent += nToSend;

                pPort++;
            }

            if (0x0 == nSent)
            {
                /* no buffers were sent, so the component hasn't returned any
                   buffers, so wait until the component does return a buffer */
                OMX_OSAL_EventWait(pCtx->hBufferCallbackEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
                OMX_OSAL_EventReset(pCtx->hBufferCallbackEvent);
                if (OMX_TRUE == bTimeout)
                {
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined,
                                            "Component unresponsive to processing buffers\n");
                }

                // todo - add means to test to timeout on failure
            }

        }


OMX_CONF_TEST_BAIL:

        return(eError);

    }

    /*****************************************************************************/
    OMX_ERRORTYPE FlushTest_PortAllocateNumBuffers(
        TEST_CTXTYPE *pCtx,
        TEST_PORTTYPE *pPort,
        OMX_U32 nNumBuffers)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_BUFFERHEADERTYPE *pBufHdr = NULL;

        while (0x0 != nNumBuffers)
        {
            eError = OMX_AllocateBuffer(pCtx->hWrappedComp, &pBufHdr,
                                        pPort->sPortDef.nPortIndex,
                                        pPort, pPort->sPortDef.nBufferSize);
            OMX_CONF_BAIL_ON_ERROR(eError);
            Q_ADD(pPort->pQ, pBufHdr);
            if (pBufHdr->pAppPrivate != (OMX_PTR)pPort)
            {
                OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined,
                                        "OMX_AllocateBuffer did not populate pAppPrivate\n");
            }

            nNumBuffers--;
        }


OMX_CONF_TEST_BAIL:

        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE FlushTest_PortFreeNumBuffers(
        TEST_CTXTYPE *pCtx,
        TEST_PORTTYPE *pPort,
        OMX_U32 nNumBuffers)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_BUFFERHEADERTYPE *pBufHdr = NULL;

        while ((0x0 != Q_INQUEUE(pPort->pQ)) && (0x0 != nNumBuffers))
        {
            Q_REMOVE(pPort->pQ, pBufHdr);
            eError = OMX_FreeBuffer(pCtx->hWrappedComp, pPort->sPortDef.nPortIndex, pBufHdr);
            OMX_CONF_BAIL_ON_ERROR(eError);
            nNumBuffers--;
        }


OMX_CONF_TEST_BAIL:

        return(eError);

    }


    /*****************************************************************************/
    OMX_ERRORTYPE FlushTest_AllocatePortStructures(
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
                pPort->pCtx = pCtx;
                pPort->bEOS = OMX_FALSE;
                pPort->bOpenFile = OMX_FALSE;
                nPortIndex = pCtx->sPortParam[i].nStartPortNumber + j;
                OMX_CONF_INIT_STRUCT(pPort->sPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
                pPort->sPortDef.nPortIndex = nPortIndex;
                eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & pPort->sPortDef);
                OMX_CONF_BAIL_ON_ERROR(eError);
                if (OMX_DirInput == pPort->sPortDef.eDir)
                {
                    eError = OMX_OSAL_OpenInputFile(pPort->sPortDef.nPortIndex);
                    OMX_CONF_BAIL_ON_ERROR(eError);
                    pPort->bOpenFile = OMX_TRUE;

                }
                else
                {
                    if (OMX_ErrorNone == OMX_OSAL_OpenOutputFile(pPort->sPortDef.nPortIndex))
                    {
                        /* test does not fail when output file does not open */
                        pPort->bOpenFile = OMX_TRUE;
                    }

                }

                if (0x0 == pPort->sPortDef.nBufferCountActual)
                {
                    /* a buffer count of 0x0 is not allowed */
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined,
                                            "PortDefinition nBufferCount incorrect\n");
                }

                if (pPort->sPortDef.nBufferCountMin > pPort->sPortDef.nBufferCountActual)
                {
                    /* a buffer count of 0x0 is not allowed */
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined,
                                            "PortDefinition nBufferCountActual incorrect\n");
                }

                /* allocate LIFO with that or the actual required buffers */
                Q_ALLOC(pPort->pQ, (pPort->sPortDef.nBufferCountActual));
                if (0x0 == pPort->pQ)
                {
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined,
                                            "memory allocation failure\n");
                }


                pPort->aSaveBufferOrder = OMX_OSAL_Malloc(sizeof(OMX_PTR) * (pPort->sPortDef.nBufferCountActual));
                if (0x0 == pPort->aSaveBufferOrder)
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
    OMX_ERRORTYPE FlushTest_FreePortStructures(TEST_CTXTYPE *pCtx)
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
                if (0x0 != pPort->pQ)
                {
                    if (OMX_ErrorNone == eError)
                    {
                        /* record the return code, but don't exit on failure as the
                           test has memory it must free */
                        eError = FlushTest_PortFreeNumBuffers(pCtx, pPort, Q_INQUEUE(pPort->pQ));

                    }
                    else
                    {
                        /* preserve the first failure from freeing buffers */
                        FlushTest_PortFreeNumBuffers(pCtx, pPort, Q_INQUEUE(pPort->pQ));
                    }

                    /* free LIFO */
                    Q_FREE(pPort->pQ);
                    pPort->pQ = 0x0;

                }

                if (0x0 != pPort->aSaveBufferOrder)
                {
                    OMX_OSAL_Free(pPort->aSaveBufferOrder);
                }
                pPort->aSaveBufferOrder = 0x0;

                if (OMX_TRUE == pPort->bOpenFile)
                {
                    if (OMX_DirInput == pPort->sPortDef.eDir)
                    {
                        eError = OMX_OSAL_CloseInputFile(pPort->sPortDef.nPortIndex);

                    }
                    else
                    {
                        eError = OMX_OSAL_CloseOutputFile(pPort->sPortDef.nPortIndex);
                    }
                    pPort->bOpenFile = OMX_FALSE;
                }

                pPort++;
            }

            OMX_OSAL_Free(pCtx->aPorts);
            pCtx->aPorts = 0x0;
        }


        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE FlushTest_AllocateAllBuffers(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_PORTTYPE *pPort;
        OMX_U32 i;
        OMX_U32 nCount;

        pPort = pCtx->aPorts;

        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Allocating %i buffers on port %i\n",
                           pPort->sPortDef.nBufferCountActual, pPort->sPortDef.nPortIndex);
            /* allocate all buffers on each port */
            nCount = pPort->sPortDef.nBufferCountActual;
            nCount -= Q_INQUEUE(pPort->pQ);
            eError = FlushTest_PortAllocateNumBuffers(pCtx, pPort, nCount);
            OMX_CONF_BAIL_ON_ERROR(eError);
            pPort++;
        }


OMX_CONF_TEST_BAIL:

        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE FlushTest_FreeAllBuffers(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_PORTTYPE *pPort;
        OMX_U32 i;

        pPort = pCtx->aPorts;

        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Freeing all buffers on port %i\n",
                           pPort->sPortDef.nPortIndex);
            eError = FlushTest_PortFreeNumBuffers(pCtx, pPort,
                                                  Q_INQUEUE(pPort->pQ));
            OMX_CONF_BAIL_ON_ERROR(eError);
            pPort++;
        }


OMX_CONF_TEST_BAIL:

        return(eError);
    }

    /*****************************************************************************/
    OMX_ERRORTYPE OMX_CONF_FlushTest(OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_ERRORTYPE eCleanupError = OMX_ErrorNone;
        OMX_HANDLETYPE hComp  = 0x0;
        OMX_CALLBACKTYPE oCallbacks;
        OMX_HANDLETYPE hWrappedComp = 0x0;
        OMX_CALLBACKTYPE *pWrappedCallbacks;
        OMX_PTR pWrappedAppData;
        TEST_CTXTYPE ctx;
        TEST_CTXTYPE *pCtx;
        TEST_PORTTYPE *pPort;
        OMX_BOOL bTimeout;
        OMX_U32 nPortIndex;
        OMX_U32 i;

        oCallbacks.EventHandler    =  FlushTest_EventHandler;
        oCallbacks.EmptyBufferDone =  FlushTest_EmptyBufferDone;
        oCallbacks.FillBufferDone  =  FlushTest_FillBufferDone;

        pCtx = &ctx;
        memset(pCtx, 0x0, sizeof(TEST_CTXTYPE));

        /* initialize events to track callbacks */
        OMX_OSAL_EventCreate(&pCtx->hStateChangeEvent);
        OMX_OSAL_EventReset(pCtx->hStateChangeEvent);
        OMX_OSAL_EventCreate(&pCtx->hPortStopEvent);
        OMX_OSAL_EventReset(pCtx->hPortStopEvent);
        OMX_OSAL_EventCreate(&pCtx->hPortRestartEvent);
        OMX_OSAL_EventReset(pCtx->hPortRestartEvent);
        OMX_OSAL_EventCreate(&pCtx->hPortFlushEvent);
        OMX_OSAL_EventReset(pCtx->hPortFlushEvent);
        OMX_OSAL_EventCreate(&pCtx->hBufferCallbackEvent);
        OMX_OSAL_EventReset(pCtx->hBufferCallbackEvent);

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
        eError = FlushTest_AllocatePortStructures(pCtx);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* transition component from loaded->idle */
        OMX_CONF_SET_STATE(pCtx, OMX_StateIdle, eError);

        /* allocate buffers on all ports */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Allocate buffers on all ports\n");
        eError = FlushTest_AllocateAllBuffers(pCtx);
        OMX_CONF_WAIT_STATE(pCtx, OMX_StateIdle, eError);

        /* transition component to executing */
        OMX_CONF_SET_STATE_AND_WAIT(pCtx, OMX_StateExecuting, eError);

        /* bogus buffer header processing */
        pPort = pCtx->aPorts;
        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            nPortIndex = pPort->sPortDef.nPortIndex;
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Sendng bogus buffers to port %i\n", nPortIndex);
            eError = FlushTest_SendBogusBuffers(pCtx, pPort);
            OMX_CONF_BAIL_ON_ERROR(eError);
            pPort++;
        }

        /* process some buffers */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Processing %i buffers\n", TEST_NUM_BUFFERS_TO_PROCESS);
        eError = FlushTest_ProcessNBuffers(pCtx, TEST_NUM_BUFFERS_TO_PROCESS);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* pause component */
        OMX_CONF_SET_STATE_AND_WAIT(pCtx, OMX_StatePause, eError);

        /* feed in more buffers */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Processing additional buffers while paused\n");

        pPort = pCtx->aPorts;
        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            nPortIndex = pPort->sPortDef.nPortIndex;
            /* feed in more buffers to fill the receive queues on each port of the
               component */
            eError = FlushTest_SendNBuffers(pCtx, pPort, Q_INQUEUE(pPort->pQ), OMX_TRUE);
            OMX_CONF_BAIL_ON_ERROR(eError);
            pPort++;
        }

        /* flush buffers from all ports */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Commanding all ports to flush\n");
        pCtx->nNumPortsFlushed = 0x0;
        pCtx->bFlushAllPorts = OMX_TRUE;
        OMX_OSAL_EventReset(pCtx->hPortFlushEvent);
        eError = OMX_SendCommand(hWrappedComp, OMX_CommandFlush, OMX_ALL, 0x0);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* confirm all buffers returned on each flushed port */
        OMX_OSAL_EventWait(pCtx->hPortFlushEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
        if (OMX_TRUE == bTimeout)
        {
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Not all ports report flush completed\n");
        }

        pPort = pCtx->aPorts;
        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            nPortIndex = pPort->sPortDef.nPortIndex;
            /* record buffer order */
            eError = FlushTest_SaveBufferOrder(pPort);
            OMX_CONF_BAIL_ON_ERROR(eError);

            /* feed in more buffers one at a time */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Sending %i buffers into port %i\n",
                           pPort->sPortDef.nBufferCountActual, nPortIndex);
            eError = FlushTest_SendNBuffers(pCtx, pPort, pPort->sPortDef.nBufferCountActual, OMX_FALSE);
            OMX_CONF_BAIL_ON_ERROR(eError);

            /* confirm none of the buffers were returned to the IL Client */
            if (0x0 != Q_INQUEUE(pPort->pQ))
            {
                OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Port returned buffers unexpectedly\n");
            }

            /* flush each port one at a time */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Commanding port %i to flush\n", nPortIndex);
            pCtx->bFlushAllPorts = OMX_FALSE;
            pCtx->nFlushPort = nPortIndex;
            OMX_OSAL_EventReset(pCtx->hPortFlushEvent);
            eError = OMX_SendCommand(hWrappedComp, OMX_CommandFlush, nPortIndex, 0x0);
            OMX_CONF_BAIL_ON_ERROR(eError);
            OMX_OSAL_EventWait(pCtx->hPortFlushEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
            if (OMX_TRUE == bTimeout)
            {
                OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Port did report flush completed\n");
            }

            /* check buffer order */
            eError = FlushTest_CheckBufferOrder(pPort);
            OMX_CONF_BAIL_ON_ERROR(eError);

            pPort++;
        }

        /* test invalid port number */
        eError = FlushTest_FindBogusPortIndex(pCtx, &nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Commanding bogus port %i to flush\n", nPortIndex);
        eError = OMX_SendCommand(hWrappedComp, OMX_CommandFlush, nPortIndex, 0x0);
        if (OMX_ErrorBadPortIndex != eError)
        {
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined,
                                    "Failure to detect bogus port index and return OMX_ErrorBadPortIndex\n");
        }

        /* feed back in buffers that were flushed, but contained data to be processed */
        pPort = pCtx->aPorts;
        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            nPortIndex = pPort->sPortDef.nPortIndex;
            /* feed in more buffers to fill the receive queues on each port of the
               component */
            eError = FlushTest_SendNBuffers(pCtx, pPort, pPort->sPortDef.nBufferCountActual, OMX_FALSE);
            OMX_CONF_BAIL_ON_ERROR(eError);
            pPort++;
        }

        /* transition back to executing to continue processing more buffers */
        OMX_CONF_SET_STATE_AND_WAIT(pCtx, OMX_StateExecuting, eError);

        /* processing more buffers to validate component can continue from a pause
           back to executing */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Processing %i additional buffers\n", TEST_NUM_BUFFERS_TO_PROCESS);
        eError = FlushTest_ProcessNBuffers(pCtx, TEST_NUM_BUFFERS_TO_PROCESS);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* transition component to idle */
        OMX_CONF_SET_STATE_AND_WAIT(pCtx, OMX_StateIdle, eError);

        /* transition to loaded */
        OMX_CONF_SET_STATE(pCtx, OMX_StateLoaded, eError);

        /* free all buffers */
        eError = FlushTest_FreeAllBuffers(pCtx);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_CONF_WAIT_STATE(pCtx, OMX_StateLoaded, eError);


OMX_CONF_TEST_BAIL:

        eCleanupError = FlushTest_FreePortStructures(pCtx);

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
        OMX_OSAL_EventDestroy(pCtx->hPortStopEvent);
        OMX_OSAL_EventDestroy(pCtx->hPortRestartEvent);
        OMX_OSAL_EventDestroy(pCtx->hPortFlushEvent);
        OMX_OSAL_EventDestroy(pCtx->hBufferCallbackEvent);

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

