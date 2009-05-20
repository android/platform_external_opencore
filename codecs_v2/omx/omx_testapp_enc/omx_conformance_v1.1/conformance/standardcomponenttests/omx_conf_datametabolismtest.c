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

/** OMX_CONF_DataMetabolismTest.c
 *  OpenMax IL conformance test - Data Metabolism Test
 */


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "OMX_OSAL_Interfaces.h"
#include "OMX_CONF_TestHarness.h"
#include "OMX_CONF_StubbedCallbacks.h"

#include <string.h>
#include <stdio.h>

    /*
     *  D E C L A R A T I O N S
     */

#define TEST_NAME_STRING "DataMetabolismTest"
#define TEST_COMPONENT_NAME_SIZE 128

#define NUM_DOMAINS 0x4

    static char szDesc[256];
    static char szState[256];

    OMX_U32 nNumOfBufferToProcess;
    OMX_U32 nStartTime;

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

#define OMX_CONF_ASSERT(_e_, _a_, _r_) \
    if (!(_a_))\
    {\
        OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "%s\n", (_r_));\
        (_e_) = OMX_ErrorUndefined;\
        goto OMX_CONF_TEST_BAIL;\
    }

#define OMX_CONF_SET_ERROR_BAIL(_e_, _c_, _s_)\
    _e_ = _c_;\
    OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, _s_);\
    goto OMX_CONF_TEST_BAIL

#define OMX_CONF_TIMEOUT_STATE_CHANGE 2000 /* duration of event timeout in msec when changing states */

#define OMX_CONF_WAIT_STATE(_p_, _s_, _e_)\
{\
    OMX_BOOL _bTimeout_;\
    OMX_OSAL_EventWait((_p_)->hStateChangeEvent, OMX_CONF_TIMEOUT_STATE_CHANGE, &_bTimeout_);\
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
        OMX_U32 nNumBuffer;
        OMX_U32 *nBufferVarArray;
        OMX_U32 nBufferIndex;
        FILE *pFile;
        OMX_STRING sFileName;
    } TEST_PORTTYPE;

    struct _TEST_CTXTYPE
    {
        OMX_STATETYPE eState;
        OMX_HANDLETYPE hWrappedComp;
        OMX_U32 nNumPorts;
        OMX_PORT_PARAM_TYPE sPortParam[NUM_DOMAINS];
        OMX_HANDLETYPE hStateChangeEvent;
        OMX_HANDLETYPE hBufferCallbackEvent;
        OMX_U32 nBuffersProcessed;
        TEST_PORTTYPE *aPorts;
        OMX_STRING *sRoleArray;
        OMX_U32 nNumBuffer;

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

    OMX_ERRORTYPE DataMetabolismTest_FindPortFromIndex(
        TEST_CTXTYPE *pCtx,
        TEST_PORTTYPE **ppPort,
        OMX_U32 nPortIndex);

    OMX_ERRORTYPE OMX_OSAL_OpenBufferVarFile(OMX_INOUT FILE** pFile, OMX_IN OMX_STRING sInFileName, OMX_STRING sMode);
    OMX_U32 OMX_OSAL_ReadFromBufferVarDataInputFile(OMX_OUT OMX_PTR pData, OMX_IN OMX_U32 nBytes, OMX_INOUT FILE* pFile);
    OMX_ERRORTYPE OMX_OSAL_CloseBufferVarFile(OMX_INOUT FILE* pFile);
    OMX_U32 OMX_OSAL_ReadFromBufferVarInputFile(OMX_OUT OMX_PTR pData, OMX_IN OMX_U32 nBytes, OMX_INOUT FILE* pInFile);
    OMX_U32 OMX_OSAL_WriteToBufferVarOutputFile(OMX_OUT OMX_PTR pData, OMX_IN OMX_U32 nBytes, OMX_IN FILE* pInFile);
    OMX_ERRORTYPE OMX_OSAL_ReadBufferVarianceFile(OMX_OUT OMX_U8 *nPortIndex,
            OMX_OUT OMX_STRING sParamName,
            OMX_OUT OMX_STRING sFieldName,
            OMX_OUT OMX_U32 *nFieldValue,
            OMX_OUT OMX_U32 *nArraySize,
            OMX_OUT OMX_U32* nMaxBufSize,
            OMX_INOUT OMX_U32 **nBufferVarArray,
            OMX_IN FILE* pInFile);


    /*****************************************************************************/
    OMX_ERRORTYPE DataMetabolismTest_EmptyBufferDone(
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
    OMX_ERRORTYPE DataMetabolismTest_FillBufferDone(
        OMX_OUT OMX_HANDLETYPE hComponent,
        OMX_OUT OMX_PTR pAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* pBufHdr)
    {
        TEST_CTXTYPE *pCtx;
        TEST_PORTTYPE *pPort;
        OMX_U32 nMsec;

        UNUSED_PARAMETER(hComponent);
        UNUSED_PARAMETER(pAppData);

        if (0x0 != pBufHdr->pAppPrivate)
        {
            pPort = (TEST_PORTTYPE*)pBufHdr->pAppPrivate;
            pCtx = pPort->pCtx;

            /* FillBufferDone can only be called from an output port */
            if (OMX_DirOutput == pPort->sPortDef.eDir)
            {
                nMsec = OMX_OSAL_GetTime();
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Received Output Buffer nFilledLen=%d,Time Offset=%d ms\n", pBufHdr->nFilledLen, nMsec - nStartTime);
                /* when the queue is full, don't add another buffer header
                   as something is wrong with the component.  It is
                   incorrectly returning more buffers than sent in */
                if (Q_INQUEUE(pPort->pQ) < pPort->sPortDef.nBufferCountActual)
                {
                    /* write to file if enabled */
                    if (OMX_TRUE == pPort->bOpenFile)
                    {
                        OMX_OSAL_WriteToBufferVarOutputFile((pBufHdr->pBuffer + pBufHdr->nOffset), pBufHdr->nFilledLen, pPort->pFile);
                    }

                    Q_ADD(pPort->pQ, pBufHdr);
                    OMX_OSAL_EventSet(pCtx->hBufferCallbackEvent);
                }

            }

        }

        return OMX_ErrorNone;
    }


    /*****************************************************************************/
    OMX_ERRORTYPE DataMetabolismTest_EventHandler(
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
                    break;
                case OMX_CommandPortEnable:
                    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port completed %i restart\n", nData2);
                    break;
                case OMX_CommandFlush:
                    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i reported flush complete\n", nData2);
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
    OMX_ERRORTYPE DataMetabolismTest_FindPortFromIndex(
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
    OMX_ERRORTYPE DataMetabolismTest_ResetPortStream(
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
                    eError = OMX_OSAL_CloseBufferVarFile(pPort->pFile);
                    OMX_CONF_BAIL_ON_ERROR(eError);

                    pPort->bOpenFile = OMX_FALSE;

                    eError = OMX_OSAL_OpenBufferVarFile(&pPort->pFile, pPort->sFileName, "r");
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
    OMX_ERRORTYPE DataMetabolismTest_SendNBuffers(
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
                    pBufHdr->nFilledLen = OMX_OSAL_ReadFromBufferVarInputFile(pBufHdr->pBuffer,
                                          pPort->nBufferVarArray[pPort->nBufferIndex],
                                          pPort->pFile);
                    if (pBufHdr->nFilledLen != pPort->nBufferVarArray[pPort->nBufferIndex])
                    {
                        pBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
                        pPort->bEOS = OMX_TRUE;
                        if (pBufHdr->nFlags)
                        {
                            eError = DataMetabolismTest_ResetPortStream(pCtx);
                            OMX_CONF_BAIL_ON_ERROR(eError);
                        }

                    }
                    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Buffer Index=%d\n", pPort->nBufferIndex);
                    pPort->nBufferIndex++;

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
                nNumBuffers--;
            }

        }

OMX_CONF_TEST_BAIL:

        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE DataMetabolismTest_ProcessNBuffers(
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

        /* reset start time*/
        nStartTime = OMX_OSAL_GetTime();

        /*For the last buffer sent pCtx->nBuffersProcessed is incremented after getting callback.
        So we reduce nBufToProcess by 1 so that we don't send one more buffer */
        while ((nBufToProcess - 1) > pCtx->nBuffersProcessed)
        {
            nSent = 0x0;
            pPort = pCtx->aPorts;
            for (i = 0; i < pCtx->nNumPorts; i++)
            {
                nToSend = Q_INQUEUE(pPort->pQ);
                eError = DataMetabolismTest_SendNBuffers(pCtx, pPort, nToSend, OMX_TRUE);
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
            }

        }
OMX_CONF_TEST_BAIL:

        return(eError);

    }

    /*****************************************************************************/
    OMX_ERRORTYPE DataMetabolismTest_PortAllocateNumBuffers(
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
    OMX_ERRORTYPE DataMetabolismTest_PortFreeNumBuffers(
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
    OMX_ERRORTYPE DataMetabolismTest_AllocatePortStructures(
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
    OMX_ERRORTYPE DataMetabolismTest_FreePortStructures(TEST_CTXTYPE *pCtx)
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
                        eError = DataMetabolismTest_PortFreeNumBuffers(pCtx, pPort, Q_INQUEUE(pPort->pQ));

                    }
                    else
                    {
                        /* preserve the first failure from freeing buffers */
                        DataMetabolismTest_PortFreeNumBuffers(pCtx, pPort, Q_INQUEUE(pPort->pQ));
                    }

                    /* free LIFO */
                    Q_FREE(pPort->pQ);
                    pPort->pQ = 0x0;

                }

                pPort++;
            }

            OMX_OSAL_Free(pCtx->aPorts);
            pCtx->aPorts = 0x0;
        }


        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE DataMetabolismTest_AllocateAllBuffers(TEST_CTXTYPE *pCtx)
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
            eError = DataMetabolismTest_PortAllocateNumBuffers(pCtx, pPort, nCount);
            OMX_CONF_BAIL_ON_ERROR(eError);
            pPort++;
        }


OMX_CONF_TEST_BAIL:

        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE DataMetabolismTest_FreeAllBuffers(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        TEST_PORTTYPE *pPort;
        OMX_U32 i;

        pPort = pCtx->aPorts;

        for (i = 0; i < pCtx->nNumPorts; i++)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Freeing all buffers on port %i\n",
                           pPort->sPortDef.nPortIndex);
            eError = DataMetabolismTest_PortFreeNumBuffers(pCtx, pPort,
                     Q_INQUEUE(pPort->pQ));
            OMX_CONF_BAIL_ON_ERROR(eError);
            pPort++;
        }


OMX_CONF_TEST_BAIL:

        return(eError);
    }

    OMX_ERRORTYPE datametabolismtest_videoport(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex,
        OMX_U32 nRoleIndex)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_ERRORTYPE eOtherError = OMX_ErrorNone;
        OMX_STRING sBufVarFileName;
        FILE *pInFile;
        OMX_U8 sReadPortIndex[4];
        OMX_U8 sPortIndex[4];
        OMX_STRING sParamName;
        OMX_STRING sFieldName;
        OMX_U32 nFieldValue;
        OMX_U32 nMaxBufSize;
        TEST_PORTTYPE *pPort;

        DataMetabolismTest_FindPortFromIndex(pCtx, &pPort, nPortIndex);

        pPort->nBufferIndex = 0;

        /*Create File Name <role_name>_<port_specifier>min.bin*/
        sBufVarFileName = (OMX_STRING)OMX_OSAL_Malloc(sizeof(OMX_U8) * (128 + 16));
        pPort->sFileName = (OMX_STRING)OMX_OSAL_Malloc(sizeof(OMX_U8) * (128 + 16));
        sParamName      = (OMX_STRING)OMX_OSAL_Malloc(sizeof(OMX_U8) * (128));
        sFieldName      = (OMX_STRING)OMX_OSAL_Malloc(sizeof(OMX_U8) * (128));


        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Inspecting video port %i\n", nPortIndex);

        if (pPort->sPortDef.eDir == OMX_DirInput)
        {

            if (!strcmp(pCtx->sRoleArray[nRoleIndex], "video_reader.binary"))
            {
                /* allocate buffers on all ports */
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "No Data Metabolism Test Applicable\n");
                eError = OMX_ErrorNone;
                goto OMX_CONF_TEST_BAIL;
            }

            sprintf(sBufVarFileName, "%s%s%lu%s", pCtx->sRoleArray[nRoleIndex], "_v", nPortIndex, "def.txt");
            sprintf(pPort->sFileName, "%s%s%lu%s", pCtx->sRoleArray[nRoleIndex], "_v", nPortIndex, "def.bin");

            eError = OMX_OSAL_OpenBufferVarFile(&pInFile, sBufVarFileName, "r");
            OMX_CONF_BAIL_ON_ERROR(eError);

            eError = OMX_OSAL_ReadBufferVarianceFile(&sReadPortIndex[0], sParamName, sFieldName, &nFieldValue, &pPort->nNumBuffer, &nMaxBufSize, &pPort->nBufferVarArray, pInFile);
            OMX_CONF_BAIL_ON_ERROR(eError);

            eError = OMX_OSAL_CloseBufferVarFile(pInFile);
            OMX_CONF_BAIL_ON_ERROR(eError);

            sprintf((OMX_STRING) &sPortIndex[0], "%c%lu", 'v', nPortIndex);
            if (strncmp((OMX_STRING) &sPortIndex[0], (OMX_STRING) &sReadPortIndex[0], strlen((OMX_STRING) &sPortIndex[0])) != 0)
            {
                OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Port Index mismatch in input file\n");
            }

            if (nMaxBufSize > pPort->sPortDef.nBufferSize)
            {
                OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Buffer Size Greater Than Allowed in the port\n");
            }

            /*This File should be closed somewhere*/
            eError = OMX_OSAL_OpenBufferVarFile(&pPort->pFile, pPort->sFileName, "rb");
            OMX_CONF_BAIL_ON_ERROR(eError);

            pPort->bOpenFile = OMX_TRUE;

            /*Assuming number of buffer sent on each port are equal*/
            nNumOfBufferToProcess = pPort->nNumBuffer;

            /* No variance settings for video port*/
        }
        else if (pPort->sPortDef.eDir == OMX_DirOutput)
        {
            sprintf(pPort->sFileName, "%s%s%lu%s", pCtx->sRoleArray[nRoleIndex], "_v", nPortIndex, "def_out.bin");

            /*This File should be closed somewhere*/
            eOtherError = OMX_OSAL_OpenBufferVarFile(&pPort->pFile, pPort->sFileName, "wb");
            if (eOtherError != OMX_ErrorNone)
            {
                pPort->pFile = NULL;
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Can't Open output file on this port=%d\n", nPortIndex);
            }
            else
                pPort->bOpenFile = OMX_TRUE;
        }

        OMX_OSAL_Free(sBufVarFileName);
        OMX_OSAL_Free(sParamName);
        OMX_OSAL_Free(sFieldName);

OMX_CONF_TEST_BAIL:

        return(eError);
    }
    OMX_ERRORTYPE datametabolismtest_imageport(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex,
        OMX_U32 nRoleIndex)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_ERRORTYPE eOtherError = OMX_ErrorNone;

        OMX_STRING sBufVarFileName;
        FILE *pInFile;
        OMX_U8 sReadPortIndex[4];
        OMX_U8 sPortIndex[4];
        OMX_STRING sParamName;
        OMX_STRING sFieldName;
        OMX_U32 nFieldValue;
        OMX_U32 nMaxBufSize;
        TEST_PORTTYPE *pPort;

        DataMetabolismTest_FindPortFromIndex(pCtx, &pPort, nPortIndex);

        pPort->nBufferIndex = 0;

        /*Create File Name <role_name>_<port_specifier>min.bin*/
        sBufVarFileName = (OMX_STRING)OMX_OSAL_Malloc(sizeof(OMX_U8) * (128 + 16));
        pPort->sFileName = (OMX_STRING)OMX_OSAL_Malloc(sizeof(OMX_U8) * (128 + 16));
        sParamName      = (OMX_STRING)OMX_OSAL_Malloc(sizeof(OMX_U8) * (128));
        sFieldName      = (OMX_STRING)OMX_OSAL_Malloc(sizeof(OMX_U8) * (128));


        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Inspecting image port %i\n", nPortIndex);

        if (pPort->sPortDef.eDir == OMX_DirInput)
        {

            if (!strcmp(pCtx->sRoleArray[nRoleIndex], "image_reader.binary"))
            {
                /* allocate buffers on all ports */
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "No Data Metabolism Test Applicable\n");
                eError = OMX_ErrorNone;
                goto OMX_CONF_TEST_BAIL;
            }

            sprintf(sBufVarFileName, "%s%s%lu%s", pCtx->sRoleArray[nRoleIndex], "_i", nPortIndex, "def.txt");
            sprintf(pPort->sFileName, "%s%s%lu%s", pCtx->sRoleArray[nRoleIndex], "_i", nPortIndex, "def.bin");

            eError = OMX_OSAL_OpenBufferVarFile(&pInFile, sBufVarFileName, "r");
            OMX_CONF_BAIL_ON_ERROR(eError);

            eError = OMX_OSAL_ReadBufferVarianceFile(&sReadPortIndex[0], sParamName, sFieldName, &nFieldValue, &pPort->nNumBuffer, &nMaxBufSize, &pPort->nBufferVarArray, pInFile);
            OMX_CONF_BAIL_ON_ERROR(eError);

            eError = OMX_OSAL_CloseBufferVarFile(pInFile);
            OMX_CONF_BAIL_ON_ERROR(eError);

            sprintf((OMX_STRING) &sPortIndex[0], "%c%lu", 'i', nPortIndex);
            if (strncmp((OMX_STRING) &sPortIndex[0], (OMX_STRING) &sReadPortIndex[0], strlen((OMX_STRING) &sPortIndex[0])) != 0)
            {
                OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Port Index mismatch in input file\n");
            }
            if (nMaxBufSize > pPort->sPortDef.nBufferSize)
            {
                OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Buffer Size Greater Than Allowed in the port\n");
            }

            /* No variance settings for video port*/

            /*This File should be closed somewhere*/
            eError = OMX_OSAL_OpenBufferVarFile(&pPort->pFile, pPort->sFileName, "rb");
            OMX_CONF_BAIL_ON_ERROR(eError);

            pPort->bOpenFile = OMX_TRUE;

            /*Assuming number of buffer sent on each port are equal*/
            nNumOfBufferToProcess = pPort->nNumBuffer;


        }
        else if (pPort->sPortDef.eDir == OMX_DirOutput)
        {
            sprintf(pPort->sFileName, "%s%s%lu%s", pCtx->sRoleArray[nRoleIndex], "_i", nPortIndex, "def_out.bin");

            /*This File should be closed somewhere*/
            eOtherError = OMX_OSAL_OpenBufferVarFile(&pPort->pFile, pPort->sFileName, "wb");
            if (eOtherError != OMX_ErrorNone)
            {
                pPort->pFile = NULL;
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Can't Open output file on this port=%d\n", nPortIndex);
            }
            else
                pPort->bOpenFile = OMX_TRUE;
        }
        OMX_OSAL_Free(sBufVarFileName);
        OMX_OSAL_Free(sParamName);
        OMX_OSAL_Free(sFieldName);


OMX_CONF_TEST_BAIL:

        return(eError);

    }
    OMX_ERRORTYPE datametabolismtest_otherport(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex,
        OMX_U32 nRoleIndex)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_ERRORTYPE eOtherError = OMX_ErrorNone;
        OMX_STRING sBufVarFileName;
        FILE *pInFile;
        OMX_U8 sReadPortIndex[4];
        OMX_U8 sPortIndex[4];
        OMX_STRING sParamName;
        OMX_STRING sFieldName;
        OMX_U32 nFieldValue;
        OMX_U32 nMaxBufSize;
        TEST_PORTTYPE *pPort;

        DataMetabolismTest_FindPortFromIndex(pCtx, &pPort, nPortIndex);

        pPort->nBufferIndex = 0;

        /*Create File Name <role_name>_<port_specifier>min.bin*/
        sBufVarFileName = (OMX_STRING)OMX_OSAL_Malloc(sizeof(OMX_U8) * (128 + 16));
        pPort->sFileName = (OMX_STRING)OMX_OSAL_Malloc(sizeof(OMX_U8) * (128 + 16));
        sParamName      = (OMX_STRING)OMX_OSAL_Malloc(sizeof(OMX_U8) * (128));
        sFieldName      = (OMX_STRING)OMX_OSAL_Malloc(sizeof(OMX_U8) * (128));


        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Inspecting other port %i\n", nPortIndex);

        if (pPort->sPortDef.eDir == OMX_DirInput)
        {

            sprintf(sBufVarFileName, "%s%s%lu%s", pCtx->sRoleArray[nRoleIndex], "_o", nPortIndex, "def.txt");
            sprintf(pPort->sFileName, "%s%s%lu%s", pCtx->sRoleArray[nRoleIndex], "_o", nPortIndex, "def.bin");

            eError = OMX_OSAL_OpenBufferVarFile(&pInFile, sBufVarFileName, "r");
            OMX_CONF_BAIL_ON_ERROR(eError);

            eError = OMX_OSAL_ReadBufferVarianceFile(&sReadPortIndex[0], sParamName, sFieldName, &nFieldValue, &pPort->nNumBuffer, &nMaxBufSize, &pPort->nBufferVarArray, pInFile);
            OMX_CONF_BAIL_ON_ERROR(eError);

            eError = OMX_OSAL_CloseBufferVarFile(pInFile);
            OMX_CONF_BAIL_ON_ERROR(eError);

            sprintf((OMX_STRING) &sPortIndex[0], "%c%lu", 'o', nPortIndex);
            if (strncmp((OMX_STRING) &sPortIndex[0], (OMX_STRING) &sReadPortIndex[0], strlen((OMX_STRING) &sPortIndex[0])) != 0)
            {
                OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Port Index mismatch in input file\n");
            }


            if (nMaxBufSize > pPort->sPortDef.nBufferSize)
            {
                OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Buffer Size Greater Than Allowed in the port\n");
            }

            /* No variance settings for video port*/

            /*This File should be closed somewhere*/
            eError = OMX_OSAL_OpenBufferVarFile(&pPort->pFile, pPort->sFileName, "rb");
            OMX_CONF_BAIL_ON_ERROR(eError);

            pPort->bOpenFile = OMX_TRUE;

            /*Assuming number of buffer sent on each port are equal*/
            nNumOfBufferToProcess = pPort->nNumBuffer;
        }
        else if (pPort->sPortDef.eDir == OMX_DirOutput)
        {
            sprintf(pPort->sFileName, "%s%s%lu%s", pCtx->sRoleArray[nRoleIndex], "_o", nPortIndex, "def_out.bin");

            /*This File should be closed somewhere*/
            eOtherError = OMX_OSAL_OpenBufferVarFile(&pPort->pFile, pPort->sFileName, "wb");
            if (eOtherError != OMX_ErrorNone)
            {
                pPort->pFile = NULL;
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Can't Open output file on this port=%d\n", nPortIndex);
            }
            else
                pPort->bOpenFile = OMX_TRUE;
        }
        OMX_OSAL_Free(sBufVarFileName);
        OMX_OSAL_Free(sParamName);
        OMX_OSAL_Free(sFieldName);


OMX_CONF_TEST_BAIL:

        return(eError);

    }

    /*****************************************************************************/
    OMX_ERRORTYPE datametabolismtest_audioport(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex,
        OMX_U32 nRoleIndex,
        OMX_BOOL bMinMax)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_ERRORTYPE eOtherError = OMX_ErrorNone;
        OMX_AUDIO_PARAM_PCMMODETYPE sFormatPCM;
        OMX_AUDIO_PARAM_AMRTYPE sFormatAMR;
        OMX_AUDIO_PARAM_MP3TYPE sFormatMP3;
        OMX_AUDIO_PARAM_AACPROFILETYPE sFormatAAC;
        OMX_AUDIO_PARAM_WMATYPE sFormatWMA;
        OMX_AUDIO_PARAM_RATYPE sFormatRA;
        OMX_STRING sBufVarFileName;
        FILE *pInFile;
        OMX_U8 sReadPortIndex[4];
        OMX_U8 sPortIndex[4];
        OMX_STRING sParamName;
        OMX_STRING sFieldName;
        OMX_U32 nFieldValue;
        OMX_U32 nMaxBufSize;
        TEST_PORTTYPE *pPort;

        DataMetabolismTest_FindPortFromIndex(pCtx, &pPort, nPortIndex);

        pPort->nBufferIndex = 0;

        /*Create File Name <role_name>_<port_specifier>min.bin*/
        sBufVarFileName = (OMX_STRING)OMX_OSAL_Malloc(sizeof(OMX_U8) * (128 + 16));
        pPort->sFileName = (OMX_STRING)OMX_OSAL_Malloc(sizeof(OMX_U8) * (128 + 16));
        sParamName      = (OMX_STRING)OMX_OSAL_Malloc(sizeof(OMX_U8) * (128));
        sFieldName      = (OMX_STRING)OMX_OSAL_Malloc(sizeof(OMX_U8) * (128));


        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Inspecting audio port %i\n", nPortIndex);

        if (pPort->sPortDef.eDir == OMX_DirInput)
        {

            if (!strcmp(pCtx->sRoleArray[nRoleIndex], "audio_reader.binary") ||
                    !strcmp(pCtx->sRoleArray[nRoleIndex], "audio_writer.bin") ||
                    !strcmp(pCtx->sRoleArray[nRoleIndex], "audio_capturer.pcm"))
            {
                /* allocate buffers on all ports */
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "No Data Metabolism Test Applicable\n");
                eError = OMX_ErrorInvalidComponentName;
                goto OMX_CONF_TEST_BAIL;
            }

            if (bMinMax == OMX_TRUE)
            {
                sprintf(sBufVarFileName, "%s%s%lu%s", pCtx->sRoleArray[nRoleIndex], "_a", nPortIndex, "min.txt");
                sprintf(pPort->sFileName, "%s%s%lu%s", pCtx->sRoleArray[nRoleIndex], "_a", nPortIndex, "min.bin");
            }
            else
            {
                sprintf(sBufVarFileName, "%s%s%lu%s", pCtx->sRoleArray[nRoleIndex], "_a", nPortIndex, "max.txt");
                sprintf(pPort->sFileName, "%s%s%lu%s", pCtx->sRoleArray[nRoleIndex], "_a", nPortIndex, "max.bin");
            }

            eError = OMX_OSAL_OpenBufferVarFile(&pInFile, sBufVarFileName, "r");
            OMX_CONF_BAIL_ON_ERROR(eError);

            eError = OMX_OSAL_ReadBufferVarianceFile(&sReadPortIndex[0], sParamName, sFieldName, &nFieldValue, &pPort->nNumBuffer, &nMaxBufSize, &pPort->nBufferVarArray, pInFile);
            OMX_CONF_BAIL_ON_ERROR(eError);

            eError = OMX_OSAL_CloseBufferVarFile(pInFile);
            OMX_CONF_BAIL_ON_ERROR(eError);

            sprintf((OMX_STRING) &sPortIndex[0], "%c%lu", 'a', nPortIndex);
            if (strncmp((OMX_STRING) &sPortIndex[0], (OMX_STRING) &sReadPortIndex[0], strlen((OMX_STRING) &sPortIndex[0])) != 0)
            {
                OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Port Index mismatch in input file\n");
            }

            if (nMaxBufSize > pPort->sPortDef.nBufferSize)
            {
                OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Buffer Size Greater Than Allowed in the port\n");
            }

            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "nMaxBufSize %d nBufferSize %d \n", nMaxBufSize, pPort->sPortDef.nBufferSize);


            if (!strcmp(pCtx->sRoleArray[nRoleIndex], "audio_decoder.aac") ||
                    !strcmp(pCtx->sRoleArray[nRoleIndex], "audio_encoder.aac"))
            {

                if (strncmp(sParamName, "OMX_AUDIO_PARAM_AACPROFILETYPE", 30))
                {
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Parameter Name Mismatch\n");
                }
                if (strncmp(sFieldName, "nSampleRate", 11))
                {
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Field Name Mismatch\n");
                }

                OMX_CONF_INIT_STRUCT(sFormatAAC, OMX_AUDIO_PARAM_AACPROFILETYPE);
                sFormatAAC.nPortIndex = nPortIndex;
                eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamAudioAac,
                                          (OMX_PTR) & sFormatAAC);
                OMX_CONF_BAIL_ON_ERROR(eError);
                sFormatAAC.nSampleRate = nFieldValue;
                eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamAudioAac,
                                          (OMX_PTR) & sFormatAAC);
                OMX_CONF_BAIL_ON_ERROR(eError);
            }
            if (!strcmp(pCtx->sRoleArray[nRoleIndex], "audio_decoder.amrnb") ||
                    !strcmp(pCtx->sRoleArray[nRoleIndex], "audio_encoder.amrnb") ||
                    !strcmp(pCtx->sRoleArray[nRoleIndex], "audio_decoder.amrwb") ||
                    !strcmp(pCtx->sRoleArray[nRoleIndex], "audio_encoder.amrwb"))
            {

                if (strncmp(sParamName, "OMX_AUDIO_PARAM_AMRTYPE", 23))
                {
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Parameter Name Mismatch\n");
                }
                if (strncmp(sFieldName, "nBitRate", 8))
                {
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Field Name Mismatch\n");
                }

                OMX_CONF_INIT_STRUCT(sFormatAMR, OMX_AUDIO_PARAM_AMRTYPE);
                sFormatAMR.nPortIndex = nPortIndex;
                eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamAudioAmr,
                                          (OMX_PTR) & sFormatAMR);
                OMX_CONF_BAIL_ON_ERROR(eError);
                sFormatAMR.nBitRate = nFieldValue;
                eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamAudioAmr,
                                          (OMX_PTR) & sFormatAMR);
                OMX_CONF_BAIL_ON_ERROR(eError);
            }
            if (!strcmp(pCtx->sRoleArray[nRoleIndex], "audio_decoder.mp3"))
            {

                if (strncmp(sParamName, "OMX_AUDIO_PARAM_MP3TYPE", 23))
                {
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Parameter Name Mismatch\n");
                }
                if (strncmp(sFieldName, "nSampleRate", 11))
                {
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Field Name Mismatch\n");
                }

                OMX_CONF_INIT_STRUCT(sFormatMP3, OMX_AUDIO_PARAM_MP3TYPE);
                sFormatMP3.nPortIndex = nPortIndex;
                eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamAudioMp3,
                                          (OMX_PTR) & sFormatMP3);
                OMX_CONF_BAIL_ON_ERROR(eError);
                sFormatMP3.nSampleRate = nFieldValue;
                eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamAudioMp3,
                                          (OMX_PTR) & sFormatMP3);
                OMX_CONF_BAIL_ON_ERROR(eError);
            }
            if (!strcmp(pCtx->sRoleArray[nRoleIndex], "audio_decoder.wma"))
            {

                if (strncmp(sParamName, "OMX_AUDIO_PARAM_WMATYPE", 23))
                {
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Parameter Name Mismatch\n");
                }
                if (strncmp(sFieldName, "nBitRate", 8))
                {
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Field Name Mismatch\n");
                }

                OMX_CONF_INIT_STRUCT(sFormatWMA, OMX_AUDIO_PARAM_WMATYPE);
                sFormatWMA.nPortIndex = nPortIndex;
                eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamAudioWma,
                                          (OMX_PTR) & sFormatWMA);
                OMX_CONF_BAIL_ON_ERROR(eError);
                sFormatWMA.nBitRate = nFieldValue;
                eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamAudioWma,
                                          (OMX_PTR) & sFormatWMA);
                OMX_CONF_BAIL_ON_ERROR(eError);
            }
            if (!strcmp(pCtx->sRoleArray[nRoleIndex], "audio_decoder.ra"))
            {

                if (strncmp(sParamName, "OMX_AUDIO_PARAM_RATYPE", 22))
                {
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Parameter Name Mismatch\n");
                }
                if (strncmp(sFieldName, "nSampleRate", 11))
                {
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Field Name Mismatch\n");
                }

                OMX_CONF_INIT_STRUCT(sFormatRA, OMX_AUDIO_PARAM_RATYPE);
                sFormatRA.nPortIndex = nPortIndex;
                eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamAudioRa,
                                          (OMX_PTR) & sFormatRA);
                OMX_CONF_BAIL_ON_ERROR(eError);
                sFormatRA.nSamplingRate = nFieldValue;
                eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamAudioRa,
                                          (OMX_PTR) & sFormatRA);
                OMX_CONF_BAIL_ON_ERROR(eError);
            }
            if (!strcmp(pCtx->sRoleArray[nRoleIndex], "audio_renderer.pcm") ||
                    !strcmp(pCtx->sRoleArray[nRoleIndex], "audio_processor.pcm.stereo_widening_loudspeakers") ||
                    !strcmp(pCtx->sRoleArray[nRoleIndex], "audio_processor.pcm.stereo_widening_headphones") ||
                    !strcmp(pCtx->sRoleArray[nRoleIndex], "audio_processor.pcm.reverberation") ||
                    !strcmp(pCtx->sRoleArray[nRoleIndex], "audio_processor.pcm.chorus") ||
                    strstr(pCtx->sRoleArray[nRoleIndex], "audio_processor.pcm.equalizer"))
            {

                if (strncmp(sParamName, "OMX_AUDIO_PARAM_PCMMODETYPE", 27))
                {
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Parameter Name Mismatch\n");
                }
                if (strncmp(sFieldName, "nSampleRate", 11))
                {
                    OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Field Name Mismatch\n");
                }

                OMX_CONF_INIT_STRUCT(sFormatPCM, OMX_AUDIO_PARAM_PCMMODETYPE);
                if (!strstr(pCtx->sRoleArray[nRoleIndex], "audio_processor."))
                    sFormatPCM.nPortIndex = nPortIndex;
                else
                    sFormatPCM.nPortIndex = OMX_ALL; // the standard audio processor port sampling rates must be the same

                eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamAudioPcm,
                                          (OMX_PTR) & sFormatPCM);
                OMX_CONF_BAIL_ON_ERROR(eError);
                sFormatPCM.nSamplingRate = nFieldValue;
                eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamAudioPcm,
                                          (OMX_PTR) & sFormatPCM);
                OMX_CONF_BAIL_ON_ERROR(eError);
            }

            /*This File should be closed somewhere*/
            eError = OMX_OSAL_OpenBufferVarFile(&pPort->pFile, pPort->sFileName, "rb");
            OMX_CONF_BAIL_ON_ERROR(eError);

            pPort->bOpenFile = OMX_TRUE;

            /*Assuming number of buffer sent on each port are equal*/
            nNumOfBufferToProcess = pPort->nNumBuffer;

        }
        else if (pPort->sPortDef.eDir == OMX_DirOutput)
        {

            if (bMinMax == OMX_TRUE)
            {
                sprintf(pPort->sFileName, "%s%s%lu%s", pCtx->sRoleArray[nRoleIndex], "_a", nPortIndex, "min_out.bin");
            }
            else
            {
                sprintf(pPort->sFileName, "%s%s%lu%s", pCtx->sRoleArray[nRoleIndex], "_a", nPortIndex, "max_out.bin");
            }

            /*This File should be closed somewhere*/
            eOtherError = OMX_OSAL_OpenBufferVarFile(&pPort->pFile, pPort->sFileName, "wb");
            if (eOtherError != OMX_ErrorNone)
            {
                pPort->pFile = NULL;
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Can't Open output file on this port=%d\n", nPortIndex);
            }
            else
                pPort->bOpenFile = OMX_TRUE;
        }
        OMX_OSAL_Free(sBufVarFileName);
        OMX_OSAL_Free(sParamName);
        OMX_OSAL_Free(sFieldName);


OMX_CONF_TEST_BAIL:

        return(eError);
    }

    OMX_ERRORTYPE cleanup_ports(
        TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_ERRORTYPE eOtherError = OMX_ErrorNone;
        TEST_PORTTYPE *pPort;
        OMX_U32 i;

        if (0x0 != pCtx->aPorts)
        {
            pPort = pCtx->aPorts;

            for (i = 0; i < pCtx->nNumPorts; i++)
            {
                if (OMX_DirInput == pPort->sPortDef.eDir)
                {

                    eError = OMX_OSAL_CloseBufferVarFile(pPort->pFile);
                    OMX_CONF_BAIL_ON_ERROR(eError);
                    pPort->bOpenFile = OMX_FALSE;
                    /*Free Buffer Variance Data File Name*/
                    OMX_OSAL_Free(pPort->sFileName);
                    /*Free Buffer Variance Array*/
                    OMX_OSAL_Free(pPort->nBufferVarArray);

                }
                else if (OMX_DirOutput == pPort->sPortDef.eDir)
                {
                    if (pPort->bOpenFile == OMX_TRUE)
                    {
                        eOtherError = OMX_OSAL_CloseBufferVarFile(pPort->pFile);
                        if (eOtherError != OMX_ErrorNone)
                        {
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Can't Close output file on this port=%d\n", pPort->sPortDef.nPortIndex);
                        }
                        else
                            pPort->bOpenFile = OMX_FALSE;
                    }

                }

                pPort++;
            }
        }

OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/
    OMX_ERRORTYPE OMX_CONF_DataMetabolismTest(OMX_IN OMX_STRING cComponentName)
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
        OMX_U32 nPortIndex;
        OMX_U32 i, j;
        OMX_U32 nRoles;
        OMX_PARAM_COMPONENTROLETYPE sComponentRole;

        oCallbacks.EventHandler    =  DataMetabolismTest_EventHandler;
        oCallbacks.EmptyBufferDone =  DataMetabolismTest_EmptyBufferDone;
        oCallbacks.FillBufferDone  =  DataMetabolismTest_FillBufferDone;

        pCtx = &ctx;
        memset(pCtx, 0x0, sizeof(TEST_CTXTYPE));

        /* initialize events to track callbacks */
        OMX_OSAL_EventCreate(&pCtx->hStateChangeEvent);
        OMX_OSAL_EventReset(pCtx->hStateChangeEvent);
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
        pCtx->sRoleArray = NULL;

        /*Get number of supported roles by this component*/
        eError = OMX_GetRolesOfComponent(cComponentName, &nRoles, (OMX_U8**) pCtx->sRoleArray);
        OMX_CONF_BAIL_ON_ERROR(eError);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Component Name %s\n", cComponentName);

        /*Allocate memory to role array*/
        pCtx->sRoleArray = (OMX_STRING *)OMX_OSAL_Malloc(nRoles * sizeof(OMX_STRING));
        for (i = 0; i < nRoles; i++)
        {
            pCtx->sRoleArray[i] = (OMX_STRING)OMX_OSAL_Malloc(sizeof(OMX_U8) * 128);
        }

        /*Get Names of supported roles by this component*/
        eError = OMX_GetRolesOfComponent(cComponentName, &nRoles, (OMX_U8**) pCtx->sRoleArray);
        OMX_CONF_BAIL_ON_ERROR(eError);

        if (pCtx->sRoleArray != NULL)
        {
            for (i = 0; i < nRoles; i++)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "THE ROLE %i FOR COMPONENT :  %s \n", (i + 1), pCtx->sRoleArray[i]);
            }
        }
        else
        {
            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined, "Component Return Role Array NULL\n");
        }

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

        /*Set the component into specific role*/
        for (j = 0;j < nRoles;j++)
        {

            OMX_CONF_INIT_STRUCT(sComponentRole, OMX_PARAM_COMPONENTROLETYPE);
            strcpy((OMX_STRING) sComponentRole.cRole , pCtx->sRoleArray[j]);

            eError = OMX_SetParameter(hWrappedComp, OMX_IndexParamStandardComponentRole, &sComponentRole);
            OMX_CONF_BAIL_ON_ERROR(eError);

            /* allocate port structures */
            eError = DataMetabolismTest_AllocatePortStructures(pCtx);
            OMX_CONF_BAIL_ON_ERROR(eError);

            /* process parameters on all audio ports for min settings*/
            for (i = 0x0; i < pCtx->sPortParam[0].nPorts; i++)
            {
                nPortIndex = pCtx->sPortParam[0].nStartPortNumber + i;
                eError = datametabolismtest_audioport(&ctx, nPortIndex, j, OMX_TRUE);
                OMX_CONF_BAIL_ON_ERROR(eError);
            }

            /* process parameters on all video ports */
            for (i = 0x0; i < pCtx->sPortParam[1].nPorts; i++)
            {
                nPortIndex = pCtx->sPortParam[1].nStartPortNumber + i;
                eError = datametabolismtest_videoport(&ctx, nPortIndex, j);
                OMX_CONF_BAIL_ON_ERROR(eError);
            }

            /* process parameters on all image ports */
            for (i = 0x0; i < pCtx->sPortParam[2].nPorts; i++)
            {
                nPortIndex = pCtx->sPortParam[2].nStartPortNumber + i;
                eError = datametabolismtest_imageport(&ctx, nPortIndex, j);
                OMX_CONF_BAIL_ON_ERROR(eError);
            }

            /* process parameters on all other ports */
            for (i = 0x0; i < pCtx->sPortParam[3].nPorts; i++)
            {
                nPortIndex = pCtx->sPortParam[3].nStartPortNumber + i;
                eError = datametabolismtest_otherport(&ctx, nPortIndex, j);
                OMX_CONF_BAIL_ON_ERROR(eError);
            }

            /* transition component from loaded->idle */
            OMX_CONF_SET_STATE(pCtx, OMX_StateIdle, eError);

            /* allocate buffers on all ports */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Allocate buffers on all ports\n");
            eError = DataMetabolismTest_AllocateAllBuffers(pCtx);
            OMX_CONF_WAIT_STATE(pCtx, OMX_StateIdle, eError);

            /* transition component to executing */
            OMX_CONF_SET_STATE_AND_WAIT(pCtx, OMX_StateExecuting, eError);

            /* process some buffers */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Processing buffers\n", nNumOfBufferToProcess);
            eError = DataMetabolismTest_ProcessNBuffers(pCtx, nNumOfBufferToProcess);
            OMX_CONF_BAIL_ON_ERROR(eError);


            /* transition component to idle */
            OMX_CONF_SET_STATE_AND_WAIT(pCtx, OMX_StateIdle, eError);

            /* transition to loaded */
            OMX_CONF_SET_STATE(pCtx, OMX_StateLoaded, eError);

            /* free all buffers */
            eError = DataMetabolismTest_FreeAllBuffers(pCtx);
            OMX_CONF_BAIL_ON_ERROR(eError);
            OMX_CONF_WAIT_STATE(pCtx, OMX_StateLoaded, eError);

            /*Close open file on input ports and Free allocated memories on ports  */
            eError = cleanup_ports(pCtx) ;
            OMX_CONF_BAIL_ON_ERROR(eError);


            /* process parameters on all audio ports for max settings */
            for (i = 0x0; i < pCtx->sPortParam[0].nPorts; i++)
            {
                nPortIndex = pCtx->sPortParam[0].nStartPortNumber + i;
                eError = datametabolismtest_audioport(&ctx, nPortIndex, j, OMX_FALSE);
                OMX_CONF_BAIL_ON_ERROR(eError);
            }

            /* process parameters on all video ports */
            for (i = 0x0; i < pCtx->sPortParam[1].nPorts; i++)
            {
                nPortIndex = pCtx->sPortParam[1].nStartPortNumber + i;
                eError = datametabolismtest_videoport(&ctx, nPortIndex, j);
                OMX_CONF_BAIL_ON_ERROR(eError);
            }

            /* process port definitions and parameters on all image ports */
            for (i = 0x0; i < pCtx->sPortParam[2].nPorts; i++)
            {
                nPortIndex = pCtx->sPortParam[2].nStartPortNumber + i;
                eError = datametabolismtest_imageport(&ctx, nPortIndex, j);
                OMX_CONF_BAIL_ON_ERROR(eError);
            }

            /* process parameters on all other ports */
            for (i = 0x0; i < pCtx->sPortParam[3].nPorts; i++)
            {
                nPortIndex = pCtx->sPortParam[3].nStartPortNumber + i;
                eError = datametabolismtest_otherport(&ctx, nPortIndex, j);
                OMX_CONF_BAIL_ON_ERROR(eError);
            }

            /* transition component from loaded->idle */
            OMX_CONF_SET_STATE(pCtx, OMX_StateIdle, eError);

            /* allocate buffers on all ports */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Allocate buffers on all ports\n");
            eError = DataMetabolismTest_AllocateAllBuffers(pCtx);
            OMX_CONF_WAIT_STATE(pCtx, OMX_StateIdle, eError);

            /* transition component to executing */
            OMX_CONF_SET_STATE_AND_WAIT(pCtx, OMX_StateExecuting, eError);

            /* process some buffers */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Processing buffers\n", nNumOfBufferToProcess);
            eError = DataMetabolismTest_ProcessNBuffers(pCtx, nNumOfBufferToProcess);
            OMX_CONF_BAIL_ON_ERROR(eError);


            /* transition component to idle */
            OMX_CONF_SET_STATE_AND_WAIT(pCtx, OMX_StateIdle, eError);

            /* transition to loaded */
            OMX_CONF_SET_STATE(pCtx, OMX_StateLoaded, eError);

            /* free all buffers */
            eError = DataMetabolismTest_FreeAllBuffers(pCtx);
            OMX_CONF_BAIL_ON_ERROR(eError);
            OMX_CONF_WAIT_STATE(pCtx, OMX_StateLoaded, eError);

            /*Close open file on input ports and Free allocated memories on ports  */
            eError = cleanup_ports(pCtx) ;
            OMX_CONF_BAIL_ON_ERROR(eError);

            eCleanupError = DataMetabolismTest_FreePortStructures(pCtx);
        }

        /*Free Role Array*/
        for (i = 0;i < nRoles;i++)
        {
            OMX_OSAL_Free(pCtx->sRoleArray[i]);
        }
        OMX_OSAL_Free(pCtx->sRoleArray);


OMX_CONF_TEST_BAIL:

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

