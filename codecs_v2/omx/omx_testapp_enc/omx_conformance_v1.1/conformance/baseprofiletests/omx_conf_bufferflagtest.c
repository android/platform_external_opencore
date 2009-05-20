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

/** OMX_CONF_BufferFlagTest.c
 *  OpenMax IL conformance test - Buffer Flag Test
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

#define TEST_NAME_STRING "BufferFlagTest"
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
    _e_ = 0x0;\
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
        OMX_BOOL bSendMark;
        OMX_BOOL bPropogateMark;
        OMX_BOOL bMarkReturned;
        OMX_HANDLETYPE hMarkTargetComponent;
        OMX_PTR pMarkData;

    } TEST_PORTTYPE;

    struct _TEST_CTXTYPE
    {
        OMX_STATETYPE eState;
        OMX_HANDLETYPE hWrappedComp;
        OMX_HANDLETYPE hCompOriginal;
        OMX_U32 nNumPorts;
        OMX_U32 nNumOutputPorts;
        OMX_U32 nNumInputPorts;
        OMX_PORT_PARAM_TYPE sPortParam[NUM_DOMAINS];
        OMX_BOOL bForceEOS;
        OMX_HANDLETYPE hStateChangeEvent;
        OMX_HANDLETYPE hBufferCallbackEvent;
        OMX_HANDLETYPE hEOSEvent;
        OMX_HANDLETYPE hBufferMarkEvent;
        OMX_HANDLETYPE hBufferFlagEvent;
        OMX_U32 nPortEOSCount;
        OMX_U32 nBufferFlagCount;
        OMX_U32 nBuffersProcessed;
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

    OMX_ERRORTYPE BufferFlagTest_FindPortFromIndex(
        TEST_CTXTYPE *pCtx,
        TEST_PORTTYPE **ppPort,
        OMX_U32 nPortIndex);


    /*****************************************************************************/
    OMX_ERRORTYPE BufferFlagTest_EmptyBufferDone(
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
                    pBufHdr->hMarkTargetComponent = 0x0;
                    pBufHdr->pMarkData = 0x0;
                    pBufHdr->nFlags = 0x0; /* JVW-NVIDIA: Reset flags for http://cvs.khronos.org/bugzilla/show_bug.cgi?id=221*/
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
    OMX_ERRORTYPE BufferFlagTest_FillBufferDone(
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
                if (OMX_BUFFERFLAG_EOS & pBufHdr->nFlags)
                {
                    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "EOS received from port %i\n",
                                   pPort->sPortDef.nPortIndex);
                    pPort->bEOS = OMX_TRUE;
                    pCtx->nPortEOSCount--;
                    if (0x0 == pCtx->nPortEOSCount)
                    {
                        OMX_OSAL_EventSet(pCtx->hEOSEvent);
                    }

                }

                if ((pBufHdr->hMarkTargetComponent == pCtx->hWrappedComp) &&
                        (pBufHdr->pMarkData == pCtx))
                {
                    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Buffer mark propogated on output port %i\n",
                                   pPort->sPortDef.nPortIndex);
                    pPort->bMarkReturned = OMX_TRUE;
                }

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
    OMX_ERRORTYPE BufferFlagTest_EventHandler(
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
                default:
                    break;
            }
        }
        else if (OMX_EventMark == eEvent)
        {
            if (0x0 != pEventData)
            {
                pPort = (TEST_PORTTYPE*)pEventData;
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Buffer mark from port %i returned\n",
                               pPort->sPortDef.nPortIndex);
                pPort->bMarkReturned = OMX_TRUE;
            }

        }
        else if (OMX_EventBufferFlag == eEvent)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i reported buffer flag\n", nData1);
            if (OMX_ErrorNone == BufferFlagTest_FindPortFromIndex(pCtx, &pPort, nData1))
            {
                if (nData2 & OMX_BUFFERFLAG_EOS)
                {
                    if ((OMX_DirInput != pPort->sPortDef.eDir) || (0x0 == pCtx->nNumOutputPorts))
                    {
                        pCtx->nBufferFlagCount--;
                        if (0x0 == pCtx->nBufferFlagCount)
                        {
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "All OMX_EventBufferFlag completed\n");
                            OMX_OSAL_EventSet(pCtx->hBufferFlagEvent);
                        }

                    }

                }
                else
                {
                    OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR,
                                   "OMX_EventBufferFlag event without EOS flag set\n");
                }

            }

        }

        return OMX_ErrorNone;
    }


    /*****************************************************************************/
    OMX_ERRORTYPE BufferFlagTest_FindPortFromIndex(
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
    OMX_ERRORTYPE BufferFlagTest_FindBogusPortIndex(
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


    /*****************************************************************************/
    OMX_ERRORTYPE BufferFlagTest_SendNBuffers(
        TEST_CTXTYPE *pCtx,
        TEST_PORTTYPE *pPort,
        OMX_U32 nNumBuffers,
        OMX_BOOL bReadFromInputFile)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_BUFFERHEADERTYPE *pBufHdr;

        if (OMX_DirInput == pPort->sPortDef.eDir)
        {
            while ((0x0 != Q_INQUEUE(pPort->pQ)) && (0x0 != nNumBuffers))
            {
                if (OMX_TRUE == pCtx->bForceEOS)
                {
                    /* when forcing end of stream, set the flag only once */
                    Q_REMOVE(pPort->pQ, pBufHdr);
                    pBufHdr->nFilledLen = 0x0;
                    if (OMX_TRUE != pPort->bEOS)
                    {
                        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "EOS on port %i\n",
                                       pPort->sPortDef.nPortIndex);
                        pBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
                        pPort->bEOS = OMX_TRUE;
                    }

                }
                else
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
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "EOS on port %i\n",
                                           pPort->sPortDef.nPortIndex);
                            pBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
                            pPort->bEOS = OMX_TRUE;
                        }

                    }

                }

                if (OMX_TRUE == pPort->bSendMark)
                {
                    /* send only a single buffer mark down to the component using
                       it's port structure pointer as the data */
                    pBufHdr->hMarkTargetComponent = pCtx->hCompOriginal;
                    pBufHdr->pMarkData = pPort;
                    pPort->bSendMark = OMX_FALSE;
                    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "BufferMark on port %i\n",
                                   pPort->sPortDef.nPortIndex);

                }
                else if (OMX_TRUE == pPort->bPropogateMark)
                {
                    /* send only a single buffer mark down to the component using
                       it's port structure pointer as the data */
                    pBufHdr->hMarkTargetComponent = pCtx->hWrappedComp;
                    pBufHdr->pMarkData = pCtx;
                    pPort->bPropogateMark = OMX_FALSE;
                    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "BufferMark propogated on port %i\n",
                                   pPort->sPortDef.nPortIndex);
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
    OMX_ERRORTYPE BufferFlagTest_ProcessNBuffers(
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
                eError = BufferFlagTest_SendNBuffers(pCtx, pPort, nToSend, OMX_TRUE);
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
    OMX_ERRORTYPE BufferFlagTest_PortAllocateNumBuffers(
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
    OMX_ERRORTYPE BufferFlagTest_PortFreeNumBuffers(
        TEST_CTXTYPE *pCtx,
        TEST_PORTTYPE *pPort,
        OMX_U32 nNumBuffers)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_BUFFERHEADERTYPE *pBufHdr;

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
    OMX_ERRORTYPE BufferFlagTest_ResetPortStream(
        TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_PORTTYPE *pPort;
        OMX_U32 i;

        pPort = pCtx->aPorts;

        pCtx->bForceEOS = OMX_FALSE;

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
    OMX_ERRORTYPE BufferFlagTest_AllocatePortStructures(
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
                pPort->bSendMark = OMX_FALSE;
                pPort->bMarkReturned = OMX_FALSE;
                nPortIndex = pCtx->sPortParam[i].nStartPortNumber + j;
                OMX_CONF_INIT_STRUCT(pPort->sPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
                pPort->sPortDef.nPortIndex = nPortIndex;
                eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & pPort->sPortDef);
                OMX_CONF_BAIL_ON_ERROR(eError);
                if (OMX_DirInput == pPort->sPortDef.eDir)
                {
                    /* count number of input ports */
                    pCtx->nNumInputPorts++;

                    eError = OMX_OSAL_OpenInputFile(pPort->sPortDef.nPortIndex);
                    OMX_CONF_BAIL_ON_ERROR(eError);
                    pPort->bOpenFile = OMX_TRUE;

                }
                else
                {
                    /* count number of output ports */
                    pCtx->nNumOutputPorts++;

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

                pPort++;
            }

        }


OMX_CONF_TEST_BAIL:

        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE BufferFlagTest_FreePortStructures(TEST_CTXTYPE *pCtx)
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
                        eError = BufferFlagTest_PortFreeNumBuffers(pCtx, pPort, Q_INQUEUE(pPort->pQ));

                    }
                    else
                    {
                        /* preserve the first failure from freeing buffers */
                        BufferFlagTest_PortFreeNumBuffers(pCtx, pPort, Q_INQUEUE(pPort->pQ));
                    }

                    /* free LIFO */
                    Q_FREE(pPort->pQ);
                    pPort->pQ = 0x0;

                }

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
    OMX_ERRORTYPE BufferFlagTest_AllocateAllBuffers(TEST_CTXTYPE *pCtx)
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
            eError = BufferFlagTest_PortAllocateNumBuffers(pCtx, pPort, nCount);
            OMX_CONF_BAIL_ON_ERROR(eError);
            pPort++;
        }


OMX_CONF_TEST_BAIL:

        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE BufferFlagTest_FreeAllBuffers(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_PORTTYPE *pPort;
        OMX_U32 i;

        pPort = pCtx->aPorts;

        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Freeing all buffers on port %i\n",
                           pPort->sPortDef.nPortIndex);
            eError = BufferFlagTest_PortFreeNumBuffers(pCtx, pPort,
                     Q_INQUEUE(pPort->pQ));
            OMX_CONF_BAIL_ON_ERROR(eError);
            pPort++;
        }


OMX_CONF_TEST_BAIL:

        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE OMX_CONF_BufferFlagTest(OMX_IN OMX_STRING cComponentName)
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
        OMX_MARKTYPE sMark;

        oCallbacks.EventHandler    =  BufferFlagTest_EventHandler;
        oCallbacks.EmptyBufferDone =  BufferFlagTest_EmptyBufferDone;
        oCallbacks.FillBufferDone  =  BufferFlagTest_FillBufferDone;

        pCtx = &ctx;
        pCtx->nNumPorts = 0x0;
        pCtx->nNumOutputPorts = 0x0;
        pCtx->nNumInputPorts = 0x0;
        pCtx->bForceEOS = OMX_FALSE;
        pCtx->aPorts = 0x0;

        /* initialize events to track callbacks */
        OMX_OSAL_EventCreate(&pCtx->hStateChangeEvent);
        OMX_OSAL_EventReset(pCtx->hStateChangeEvent);
        OMX_OSAL_EventCreate(&pCtx->hBufferCallbackEvent);
        OMX_OSAL_EventReset(pCtx->hBufferCallbackEvent);
        OMX_OSAL_EventCreate(&pCtx->hEOSEvent);
        OMX_OSAL_EventReset(pCtx->hEOSEvent);
        OMX_OSAL_EventCreate(&pCtx->hBufferMarkEvent);
        OMX_OSAL_EventReset(pCtx->hBufferMarkEvent);
        OMX_OSAL_EventCreate(&pCtx->hBufferFlagEvent);
        OMX_OSAL_EventReset(pCtx->hBufferFlagEvent);

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
        pCtx->hCompOriginal = hComp;

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
        eError = BufferFlagTest_AllocatePortStructures(pCtx);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* transition component from loaded->idle */
        OMX_CONF_SET_STATE(pCtx, OMX_StateIdle, eError);

        /* allocate buffers on all ports */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Allocate buffers on all ports\n");
        eError = BufferFlagTest_AllocateAllBuffers(pCtx);
        OMX_CONF_WAIT_STATE(pCtx, OMX_StateIdle, eError);

        if (0x0 == pCtx->nNumInputPorts)
        {
            /* without input ports on the component, the test cannot
               force a EOS ... hence pass this test for components
               without an input port */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "BufferFlag portion of test, component has no input ports\n");
            goto OMX_CONF_TEST_SKIP_BUFFERFLAGTEST;
        }

        /* transition component to executing */
        OMX_CONF_SET_STATE_AND_WAIT(pCtx, OMX_StateExecuting, eError);

        /* process some buffers */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Processing %i buffers\n", TEST_NUM_BUFFERS_TO_PROCESS);
        eError = BufferFlagTest_ProcessNBuffers(pCtx, TEST_NUM_BUFFERS_TO_PROCESS);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* send EOS on each input port, and wait for EOS on each output port */
        pCtx->bForceEOS = OMX_TRUE;
        pCtx->nPortEOSCount = pCtx->nNumOutputPorts;
        if (0x0 == pCtx->nNumOutputPorts)
        {
            /* component has no output ports, so the component will report
               the same number of buffer events as input ports */
            pCtx->nBufferFlagCount = pCtx->nNumInputPorts;

        }
        else
        {
            /* each EOS on a buffer sent from an output port MUST trigger
               the buffer flag event */
            pCtx->nBufferFlagCount = pCtx->nNumOutputPorts;
        }

        OMX_OSAL_EventReset(pCtx->hBufferFlagEvent);
        OMX_OSAL_EventReset(pCtx->hEOSEvent);

        /* send EOS to each input port */
        pPort = pCtx->aPorts;
        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            nPortIndex = pPort->sPortDef.nPortIndex;
            if (OMX_DirInput == pPort->sPortDef.eDir)
            {
                BufferFlagTest_SendNBuffers(pCtx, pPort, 0x1, OMX_FALSE);
            }
            pPort++;
        }

        /* process enough buffers on the output port for EOS to be propogated */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Processing %i buffers\n", TEST_NUM_BUFFERS_TO_PROCESS);
        eError = BufferFlagTest_ProcessNBuffers(pCtx, TEST_NUM_BUFFERS_TO_PROCESS);
        OMX_CONF_BAIL_ON_ERROR(eError);

        if (0x0 != pCtx->nNumOutputPorts)
        {
            /* only wait for EOS to show up in buffers from output ports
               if there are output ports */
            OMX_OSAL_EventWait(pCtx->hEOSEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
            if (OMX_TRUE == bTimeout)
            {
                OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Not all ports propogated EOS\n");
            }

        }

        OMX_OSAL_EventWait(pCtx->hBufferFlagEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
        if (OMX_TRUE == bTimeout)
        {
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Not all ports reported EOS\n");
        }

        /* transition component to idle */
        OMX_CONF_SET_STATE_AND_WAIT(pCtx, OMX_StateIdle, eError);

        /* reset input file stream */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Resetting input data stream to start of file\n");
        eError = BufferFlagTest_ResetPortStream(pCtx);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* test propogation of buffer marks from input port through an output port */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Testing component reports OMX_EventMark\n");
        pPort = pCtx->aPorts;
        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            nPortIndex = pPort->sPortDef.nPortIndex;
            pPort->bMarkReturned = OMX_FALSE;
            if (OMX_DirInput == pPort->sPortDef.eDir)
            {
                pPort->bPropogateMark = OMX_TRUE;
            }
            pPort++;
        }

        /* transition component to executing process buffers */
        OMX_CONF_SET_STATE_AND_WAIT(pCtx, OMX_StateExecuting, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Processing %i buffers\n", TEST_NUM_BUFFERS_TO_PROCESS);
        eError = BufferFlagTest_ProcessNBuffers(pCtx, TEST_NUM_BUFFERS_TO_PROCESS);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* transition component to idle */
        OMX_CONF_SET_STATE_AND_WAIT(pCtx, OMX_StateIdle, eError);

        /* check that each buffer mark on each input port was returned by the
           component */
        pPort = pCtx->aPorts;
        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            nPortIndex = pPort->sPortDef.nPortIndex;
            if (OMX_DirOutput == pPort->sPortDef.eDir)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Validating buffer mark propogated through output port %i\n",
                               nPortIndex);
                if (OMX_TRUE != pPort->bMarkReturned)
                {
                    /* buffer mark not returned from input port */
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Buffer mark wasn't propogated\n");
                }

            }
            pPort++;
        }

        /* reset input file stream */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Resetting input data stream to start of file\n");
        eError = BufferFlagTest_ResetPortStream(pCtx);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* testing component can set a mark into a buffer and propogate it out
           of an output port */
        OMX_CONF_SET_STATE_AND_WAIT(pCtx, OMX_StateExecuting, eError);

        /* test sending bad port index to buffer mark */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Testing OMX_CommandMarkBuffer with bogus port index\n");
        eError = BufferFlagTest_FindBogusPortIndex(pCtx, &nPortIndex);
        OMX_CONF_BAIL_ON_ERROR(eError);

        sMark.hMarkTargetComponent = pCtx->hWrappedComp;
        sMark.pMarkData = pCtx;
        eError = OMX_SendCommand(pCtx->hWrappedComp, OMX_CommandMarkBuffer, nPortIndex, &sMark);
        if (OMX_ErrorBadPortIndex != eError)
        {
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined,
                                    "Component did not detect bogus port index\n");
        }
        eError = OMX_ErrorNone;

        /* send individual buffer mark to each port */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Testing component with OMX_CommandMarkBuffer\n");
        sMark.hMarkTargetComponent = pCtx->hWrappedComp;
        sMark.pMarkData = pCtx;
        pPort = pCtx->aPorts;
        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            nPortIndex = pPort->sPortDef.nPortIndex;
            pPort->bMarkReturned = OMX_FALSE;
            if (OMX_DirInput == pPort->sPortDef.eDir)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "OMX_CommandMarkBuffer on port %i\n",
                               nPortIndex);
                eError = OMX_SendCommand(pCtx->hWrappedComp, OMX_CommandMarkBuffer, nPortIndex, &sMark);
                if (OMX_ErrorNone != eError)
                {
                    /* transition to IDLE, and bail */
                    OMX_CONF_SET_STATE_AND_WAIT(pCtx, OMX_StateExecuting, eCleanupError);

                }
                OMX_CONF_BAIL_ON_ERROR(eError);

            }
            pPort++;
        }

        /* process buffers */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Processing %i buffers\n", TEST_NUM_BUFFERS_TO_PROCESS);
        eError = BufferFlagTest_ProcessNBuffers(pCtx, TEST_NUM_BUFFERS_TO_PROCESS);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* transition component to idle */
        OMX_CONF_SET_STATE_AND_WAIT(pCtx, OMX_StateIdle, eError);

        /* check that each buffer mark on each input port was returned by the
           component */
        pPort = pCtx->aPorts;
        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            nPortIndex = pPort->sPortDef.nPortIndex;
            if (OMX_DirOutput == pPort->sPortDef.eDir)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Validating buffer mark propogated through output port %i\n",
                               nPortIndex);
                if (OMX_TRUE != pPort->bMarkReturned)
                {
                    /* buffer mark not returned from input port */
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Buffer mark wasn't propogated\n");
                }

            }
            pPort++;
        }


        /* reset input file stream */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Resetting input data stream to start of file\n");
        eError = BufferFlagTest_ResetPortStream(pCtx);
        OMX_CONF_BAIL_ON_ERROR(eError);

        OMX_CONF_SET_STATE_AND_WAIT(pCtx, OMX_StateExecuting, eError);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Testing OMX_CommandMarkBuffer does not overwrite already marked buffers\n");

        sMark.hMarkTargetComponent = pCtx->hWrappedComp;
        sMark.pMarkData = pCtx;

        /* send individual buffer mark to each port */
        pPort = pCtx->aPorts;
        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            nPortIndex = pPort->sPortDef.nPortIndex;
            pPort->bMarkReturned = OMX_FALSE;
            if (OMX_DirInput == pPort->sPortDef.eDir)
            {
                pPort->bPropogateMark = OMX_TRUE;
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "OMX_CommandMarkBuffer on port %i\n",
                               nPortIndex);
                eError = OMX_SendCommand(pCtx->hWrappedComp, OMX_CommandMarkBuffer, nPortIndex, &sMark);
                if (OMX_ErrorNone != eError)
                {
                    /* transition to IDLE, and bail */
                    OMX_CONF_SET_STATE_AND_WAIT(pCtx, OMX_StateExecuting, eCleanupError);

                }
                OMX_CONF_BAIL_ON_ERROR(eError);
            }
            pPort++;
        }

        /* test component does not overwrite buffer marks */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Processing %i buffers\n", TEST_NUM_BUFFERS_TO_PROCESS);
        eError = BufferFlagTest_ProcessNBuffers(pCtx, TEST_NUM_BUFFERS_TO_PROCESS);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* transition component to idle */
        OMX_CONF_SET_STATE_AND_WAIT(pCtx, OMX_StateIdle, eError);

        /* check that each buffer mark on each input port was returned by the
           component */
        pPort = pCtx->aPorts;
        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            nPortIndex = pPort->sPortDef.nPortIndex;
            if (OMX_DirOutput == pPort->sPortDef.eDir)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Validating buffer mark propogated through output port %i\n",
                               nPortIndex);
                if (OMX_TRUE != pPort->bMarkReturned)
                {
                    /* buffer mark not returned from input port */
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Buffer mark wasn't propogated\n");
                }

            }
            pPort++;
        }


        /* this label is used only for components that have no output ports */
OMX_CONF_TEST_SKIP_BUFFERFLAGTEST:

        /* reset input file stream */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Resetting input data stream to start of file\n");
        eError = BufferFlagTest_ResetPortStream(pCtx);
        OMX_CONF_BAIL_ON_ERROR(eError);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Testing component reports OMX_EventMark\n");
        pPort = pCtx->aPorts;
        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            nPortIndex = pPort->sPortDef.nPortIndex;
            pPort->bMarkReturned = OMX_FALSE;
            if (OMX_DirInput == pPort->sPortDef.eDir)
            {
                pPort->bSendMark = OMX_TRUE;
            }
            pPort++;
        }

        /* transition component to executing process buffers */
        OMX_CONF_SET_STATE_AND_WAIT(pCtx, OMX_StateExecuting, eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Processing %i buffers\n", TEST_NUM_BUFFERS_TO_PROCESS);
        eError = BufferFlagTest_ProcessNBuffers(pCtx, TEST_NUM_BUFFERS_TO_PROCESS);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* transition component to idle */
        OMX_CONF_SET_STATE_AND_WAIT(pCtx, OMX_StateIdle, eError);

        /* check that each buffer mark on each input port was returned by the
           component */
        pPort = pCtx->aPorts;
        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            nPortIndex = pPort->sPortDef.nPortIndex;
            if (OMX_DirInput == pPort->sPortDef.eDir)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Validating buffer mark on port %i returned via EventHandler\n",
                               nPortIndex);
                if (OMX_TRUE != pPort->bMarkReturned)
                {
                    /* buffer mark not returned from input port */
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Buffer mark never returned\n");
                }

            }
            pPort++;
        }

        /* transition to loaded */
        OMX_CONF_SET_STATE(pCtx, OMX_StateLoaded, eError);

        /* free all buffers */
        eError = BufferFlagTest_FreeAllBuffers(pCtx);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_CONF_WAIT_STATE(pCtx, OMX_StateLoaded, eError);


OMX_CONF_TEST_BAIL:

        eCleanupError = BufferFlagTest_FreePortStructures(pCtx);

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
        OMX_OSAL_EventDestroy(pCtx->hBufferCallbackEvent);
        OMX_OSAL_EventDestroy(pCtx->hEOSEvent);
        OMX_OSAL_EventDestroy(pCtx->hBufferMarkEvent);
        OMX_OSAL_EventDestroy(pCtx->hBufferFlagEvent);

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

