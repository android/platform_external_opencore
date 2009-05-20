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

/** OMX_CONF_BaseMultiThreadedTest.c
 *  OpenMax IL conformance test - Base Multi Threaded Test
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
#define TEST_NAME_STRING "BaseMultiThreadedTest"
#define TEST_COMPONENT_NAME_SIZE 128
    static char szDesc[256];
#define NUM_DOMAINS 0x4
#define OMX_NOPORT 0xfffffffe

    /*
     *     M A C R O S
     */
#define OMX_CONF_INIT_STRUCT(_s_, _name_)	\
    memset(&(_s_), 0x0, sizeof(_name_));	\
    (_s_).nSize = sizeof(_name_);		\
    (_s_).nVersion.s.nVersionMajor = 0x1;	\
    (_s_).nVersion.s.nVersionMinor = 0x0;	\
    (_s_).nVersion.s.nRevision = 0x0;		\
    (_s_).nVersion.s.nStep = 0x0

#define OMX_CONF_BAIL_IF_ERROR(_eError)		\
    if((eError = _eError) != OMX_ErrorNone){	\
        goto OMX_CONF_TEST_BAIL;		\
    }

#define OMX_CONF_SET_ERROR_BAIL( _desc, _eError)\
{						\
    eError = _eError;				\
    OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, _desc);	\
    goto OMX_CONF_TEST_BAIL;			\
}

#define OMX_CONF_EVENT_HANDLER_ERROR(_nX, _nY, _nZ, _desc)	\
{								\
    _nX = _nY + 1;						\
    _nZ = OMX_CONF_BUFFERS_OF_TRAFFIC;				\
    OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "%s\n", _desc);		\
}

#define LIST_CLEAR_ENTRY(_pL, _pB)	\
    _pB = _pL->pBufHdr;			\
    _pL->pBufHdr = NULL;		\
    _pL = _pL->pNextBuf;

#define LIST_SET_ENTRY(_pL, _pB)\
{				\
    BufferList *_pT = _pL;	\
    while(_pT && _pT->pBufHdr){	\
        _pT = _pT->pNextBuf;	\
    }				\
    if(_pT && !_pT->pBufHdr)	\
        _pT->pBufHdr = _pB;	\
}

#define WAIT_FOR_STATE_CHANGE(_hH, _eC, _eE, _pC, _bT)			\
{									\
    OMX_STATETYPE _eS;							\
    OMX_U32 _nC = 0x0;							\
    OMX_CONF_BAIL_IF_ERROR(OMX_GetState(_pC->hWComp, &_eS));		\
    while(_eS != _eC){							\
	OMX_OSAL_EventReset(_hH);					\
	OMX_OSAL_EventWait(_hH, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &_bT);\
	OMX_CONF_BAIL_IF_ERROR(OMX_GetState(_pC->hWComp, &_eS));	\
	if(_nC++ > 5 &&_eS != _eC){					\
	    OMX_CONF_StateToString(_eC, szDesc);			\
	    OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "Transition to %s timed out", szDesc);\
	    OMX_CONF_SET_ERROR_BAIL("\n", OMX_ErrorUndefined);		\
	}								\
    }									\
}


    /*
     *     D E F I N I T I O N S
     */
    typedef struct _BufferList BufferList;

    struct _BufferList
    {
        OMX_BUFFERHEADERTYPE *pBufHdr;
        OMX_BUFFERHEADERTYPE *pOrigBufHdr;
        BufferList *pNextBuf;
    };

    typedef struct
    {
        OMX_U32 nInBuf;
        OMX_U32 nOutBuf;
        OMX_U32 nInBufBusy;
        OMX_U32 nOutBufBusy;
        OMX_U32 nBufDoneCalls;
        OMX_BOOL bStop;
        OMX_STATETYPE eState;
        OMX_ERRORTYPE eThreadError;
        OMX_HANDLETYPE hWComp;
        OMX_HANDLETYPE hStateSetEvent;
        OMX_HANDLETYPE hFillBufDoneEvent;
        OMX_HANDLETYPE hEmptyBufDoneEvent;
        OMX_HANDLETYPE hBufDoneCallsEvent;
        OMX_HANDLETYPE hThreadEvent;
        OMX_HANDLETYPE hInLock;
        OMX_HANDLETYPE hOutLock;
        BufferList *pInBufferList;
        BufferList *pOutBufferList;
        OMX_PORT_PARAM_TYPE sPortParam[NUM_DOMAINS];
    } BaseMultiThreadedTestCtxt;

    typedef enum PortOpType
    {
        AllocBuf,
        OpenFile,
        CloseFile,
    } PortOpType;


    /*
     *  E X T E R N A L   F U N C T I O N S
     */
    OMX_ERRORTYPE OMX_CONF_ErrorToString(
        OMX_IN OMX_ERRORTYPE eError,
        OMX_OUT OMX_STRING sName);

    OMX_ERRORTYPE OMX_CONF_StateToString(
        OMX_IN OMX_STATETYPE eState,
        OMX_OUT OMX_STRING sName);


    /*
     *     F U N C T I O N S
     */

    /*****************************************************************************/
    OMX_ERRORTYPE BaseMultiThreadedTest_EventHandler(OMX_IN OMX_HANDLETYPE hComponent,
            OMX_IN OMX_PTR pAppData,
            OMX_IN OMX_EVENTTYPE eEvent,
            OMX_IN OMX_U32 nData1,
            OMX_IN OMX_U32 nData2,
            OMX_IN OMX_PTR pEventData)
    {
        BaseMultiThreadedTestCtxt* pCtxt;

        UNUSED_PARAMETER(hComponent);
        UNUSED_PARAMETER(pEventData);

        if (pAppData == NULL)
            return OMX_ErrorNone;
        pCtxt = (BaseMultiThreadedTestCtxt*)pAppData;

        if (eEvent == OMX_EventCmdComplete)
        {
            switch ((OMX_COMMANDTYPE)(nData1))
            {
                case OMX_CommandStateSet:
                    OMX_CONF_StateToString((OMX_STATETYPE)(nData2), szDesc);
                    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Component transitioned to %s\n", szDesc);
                    pCtxt->eState = (OMX_STATETYPE)(nData2);
                    OMX_OSAL_EventSet(pCtxt->hStateSetEvent);
                    break;
                default:
                    break;
            }
        }
        return OMX_ErrorNone;
    }

    /*****************************************************************************/
    OMX_ERRORTYPE BaseMultiThreadedTest_EmptyBufferDone(OMX_IN OMX_HANDLETYPE hComponent,
            OMX_IN OMX_PTR pAppData,
            OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
    {
        BaseMultiThreadedTestCtxt* pCtxt;

        UNUSED_PARAMETER(hComponent);

        if (pAppData == NULL)
            return OMX_ErrorNone;
        pCtxt = (BaseMultiThreadedTestCtxt*)pAppData;

        OMX_OSAL_MutexLock(pCtxt->hInLock);
        if (pBuffer->nInputPortIndex == OMX_NOPORT || pBuffer->nOutputPortIndex != OMX_NOPORT)
        {
            OMX_CONF_EVENT_HANDLER_ERROR(pCtxt->nInBufBusy, pCtxt->nInBuf, pCtxt->nBufDoneCalls,
                                         "Component returned invalid input buffer\n");
        }
        else if (pCtxt->nInBufBusy)
        {
            pCtxt->nInBufBusy--;
            pCtxt->nBufDoneCalls++;
            LIST_SET_ENTRY(pCtxt->pInBufferList, pBuffer);
            OMX_OSAL_EventSet(pCtxt->hEmptyBufDoneEvent);
        }
        else
        {
            OMX_CONF_EVENT_HANDLER_ERROR(pCtxt->nInBufBusy, pCtxt->nInBuf, pCtxt->nBufDoneCalls,
                                         "Component returned more input buffers than expected\n");
        }
        OMX_OSAL_MutexUnlock(pCtxt->hInLock);

        return OMX_ErrorNone;
    }

    /*****************************************************************************/
    OMX_ERRORTYPE BaseMultiThreadedTest_FillBufferDone(OMX_OUT OMX_HANDLETYPE hComponent,
            OMX_OUT OMX_PTR pAppData,
            OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer)
    {
        BaseMultiThreadedTestCtxt* pCtxt;

        UNUSED_PARAMETER(hComponent);

        if (pAppData == NULL)
            return OMX_ErrorNone;
        pCtxt = (BaseMultiThreadedTestCtxt*)pAppData;

        OMX_OSAL_MutexLock(pCtxt->hOutLock);
        if (pBuffer->nInputPortIndex != OMX_NOPORT || pBuffer->nOutputPortIndex == OMX_NOPORT)
        {
            OMX_CONF_EVENT_HANDLER_ERROR(pCtxt->nOutBufBusy, pCtxt->nOutBuf, pCtxt->nBufDoneCalls,
                                         "Component returned invalid output buffer\n");
        }
        else if (pCtxt->nOutBufBusy)
        {
            pCtxt->nOutBufBusy--;
            pCtxt->nBufDoneCalls++;
            LIST_SET_ENTRY(pCtxt->pOutBufferList, pBuffer);
            OMX_OSAL_EventSet(pCtxt->hFillBufDoneEvent);
        }
        else
        {
            OMX_CONF_EVENT_HANDLER_ERROR(pCtxt->nOutBufBusy, pCtxt->nOutBuf, pCtxt->nBufDoneCalls,
                                         "Component returned more output buffers than expected\n");
        }
        OMX_OSAL_MutexUnlock(pCtxt->hOutLock);

        return OMX_ErrorNone;
    }

    /*****************************************************************************/
    OMX_ERRORTYPE BaseMultiThreadedTest_AllocateBuffers(BaseMultiThreadedTestCtxt *pCtxt,
            OMX_PARAM_PORTDEFINITIONTYPE *pPortDef)
    {
        OMX_BUFFERHEADERTYPE *pBufferHdr = NULL;
        BufferList *pBufferList;
        BufferList *pTemp;
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_U32 i;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Allocate buffers for port %d\n",
                       pPortDef->nPortIndex);
        for (i = 0x0; i < pPortDef->nBufferCountActual; i++)
        {
            pBufferList = (BufferList *)OMX_OSAL_Malloc(sizeof(BufferList));
            if (!pBufferList)
                OMX_CONF_SET_ERROR_BAIL("Malloc failed\n", OMX_ErrorInsufficientResources);

            OMX_CONF_BAIL_IF_ERROR(OMX_AllocateBuffer(pCtxt->hWComp, &pBufferHdr,
                                   pPortDef->nPortIndex,
                                   0, pPortDef->nBufferSize));

            pBufferList->pNextBuf = NULL;
            pBufferList->pBufHdr = pBufferHdr;
            pBufferList->pOrigBufHdr = pBufferHdr;

            if (pPortDef->eDir == OMX_DirInput)
            {
                if (pBufferHdr->nAllocLen != pPortDef->nBufferSize || !pBufferHdr->pBuffer ||
                        pBufferHdr->nInputPortIndex != pPortDef->nPortIndex)
                    OMX_CONF_SET_ERROR_BAIL("Buffer header incorrect\n", OMX_ErrorUndefined);

                pBufferHdr->nOutputPortIndex = OMX_NOPORT;
                if (pCtxt->pInBufferList == NULL)
                    pCtxt->pInBufferList = pBufferList;
                else
                {
                    pTemp = pCtxt->pInBufferList;
                    while (pTemp->pNextBuf)
                        pTemp = pTemp->pNextBuf;
                    pTemp->pNextBuf = pBufferList;
                }
                pCtxt->nInBuf++;
            }
            else
            {
                if (pBufferHdr->nAllocLen != pPortDef->nBufferSize || !pBufferHdr->pBuffer ||
                        pBufferHdr->nOutputPortIndex != pPortDef->nPortIndex)
                    OMX_CONF_SET_ERROR_BAIL("Buffer header incorrect\n", OMX_ErrorUndefined);

                pBufferHdr->nInputPortIndex = OMX_NOPORT;
                if (pCtxt->pOutBufferList  == NULL)
                    pCtxt->pOutBufferList = pBufferList;
                else
                {
                    pTemp = pCtxt->pOutBufferList;
                    while (pTemp->pNextBuf)
                        pTemp = pTemp->pNextBuf;
                    pTemp->pNextBuf = pBufferList;
                }
                pCtxt->nOutBuf++;
            }
        }

OMX_CONF_TEST_BAIL:
        return eError;
    }

    /*****************************************************************************/
    OMX_ERRORTYPE BaseMultiThreadedTest_DeInitBuffer(BaseMultiThreadedTestCtxt* pCtxt)
    {
        BufferList *pBufferListObject;
        BufferList *pTemp;
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Free all buffers\n");
        pTemp = pCtxt->pInBufferList;
        while (pTemp)
        {
            pBufferListObject = (BufferList *)pTemp;

            eError = OMX_FreeBuffer(pCtxt->hWComp, pTemp->pOrigBufHdr->nInputPortIndex,
                                    pTemp->pOrigBufHdr);
            if (eError == OMX_ErrorInvalidState)
                eError = OMX_ErrorNone;
            OMX_CONF_BAIL_IF_ERROR(eError);
            pTemp = pTemp->pNextBuf;
            if (pBufferListObject)
                OMX_OSAL_Free(pBufferListObject);
            pCtxt->nInBuf--;
        }
        pCtxt->pInBufferList = NULL;

        pTemp = pCtxt->pOutBufferList;
        while (pTemp)
        {
            pBufferListObject = (BufferList *)pTemp;

            eError = OMX_FreeBuffer(pCtxt->hWComp, pTemp->pOrigBufHdr->nOutputPortIndex,
                                    pTemp->pOrigBufHdr);
            if (eError == OMX_ErrorInvalidState)
                eError = OMX_ErrorNone;
            OMX_CONF_BAIL_IF_ERROR(eError);
            pTemp = pTemp->pNextBuf;
            if (pBufferListObject)
                OMX_OSAL_Free(pBufferListObject);
            pCtxt->nOutBuf--;
        }
        pCtxt->pOutBufferList = NULL;
OMX_CONF_TEST_BAIL:
        return eError;
    }

    /*****************************************************************************/
    OMX_U32 BaseMultiThreadedTest_StartThread(OMX_PTR pParam)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        BaseMultiThreadedTestCtxt * pCtxt;
        OMX_HANDLETYPE hTimeout;
        OMX_BOOL bTimeout;
        pCtxt = (BaseMultiThreadedTestCtxt *)pParam;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Created new thread for initialization\n");
        OMX_OSAL_EventCreate(&hTimeout);
        WAIT_FOR_STATE_CHANGE(hTimeout, OMX_StateLoaded, eError, pCtxt, bTimeout);

        eError = OMX_SendCommand(pCtxt->hWComp, OMX_CommandStateSet, OMX_StateIdle, 0);
        OMX_CONF_BAIL_IF_ERROR(eError);
        OMX_OSAL_EventSet(pCtxt->hThreadEvent);
        WAIT_FOR_STATE_CHANGE(hTimeout, OMX_StateIdle, eError, pCtxt, bTimeout);

        eError = OMX_SendCommand(pCtxt->hWComp, OMX_CommandStateSet, OMX_StateExecuting, 0);
        OMX_CONF_BAIL_IF_ERROR(eError);
// Following line commented out to fix http://cvs.khronos.org/bugzilla/show_bug.cgi?id=226
//    WAIT_FOR_STATE_CHANGE(hTimeout, OMX_StateExecuting, eError, pCtxt, bTimeout);

OMX_CONF_TEST_BAIL:
        OMX_OSAL_EventDestroy(hTimeout);
        if (eError != OMX_ErrorNone)
        {
            pCtxt->eThreadError = eError;
            OMX_CONF_ErrorToString(eError, szDesc);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "Start thread failed with %s\n", szDesc);
        }
        return 0;
    }


    /*****************************************************************************/
    OMX_U32 BaseMultiThreadedTest_StopThread(OMX_PTR pParam)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        BaseMultiThreadedTestCtxt * pCtxt;
        OMX_HANDLETYPE hTimeout;
        OMX_BOOL bTimeout;
        pCtxt = (BaseMultiThreadedTestCtxt *)pParam;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Created new thread for stopping\n");
        OMX_OSAL_EventCreate(&hTimeout);
        WAIT_FOR_STATE_CHANGE(hTimeout, OMX_StateExecuting, eError, pCtxt, bTimeout);

        eError = OMX_SendCommand(pCtxt->hWComp, OMX_CommandStateSet, OMX_StateIdle, 0);
        OMX_CONF_BAIL_IF_ERROR(eError);

        pCtxt->bStop = OMX_TRUE;
        OMX_OSAL_EventSet(pCtxt->hThreadEvent);
// Following line commented out to fix http://cvs.khronos.org/bugzilla/show_bug.cgi?id=226
//   WAIT_FOR_STATE_CHANGE(hTimeout, OMX_StateIdle, eError, pCtxt, bTimeout);

OMX_CONF_TEST_BAIL:
        OMX_OSAL_EventDestroy(hTimeout);
        if (eError != OMX_ErrorNone)
        {
            pCtxt->eThreadError = eError;
            OMX_CONF_ErrorToString(eError, szDesc);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "Stop thread failed with %s\n", szDesc);
        }
        return 0;
    }

    /*****************************************************************************/
    OMX_U32 BaseMultiThreadedTest_ReadOutBuffers(OMX_PTR pParam)
    {
        BufferList * pList;
        OMX_BOOL bTimeout = OMX_FALSE;
        OMX_BUFFERHEADERTYPE *pBufHeader = NULL;
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        BaseMultiThreadedTestCtxt * pCtxt;
        pCtxt = (BaseMultiThreadedTestCtxt *)pParam;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Created new thread for output processing\n");
        do
        {
            OMX_OSAL_EventReset(pCtxt->hFillBufDoneEvent);
            pList = pCtxt->pOutBufferList;
            while (pList && pList->pBufHdr && pCtxt->nBufDoneCalls < OMX_CONF_BUFFERS_OF_TRAFFIC)
            {
                OMX_OSAL_MutexLock(pCtxt->hOutLock);
                pCtxt->nOutBufBusy++;
                LIST_CLEAR_ENTRY(pList, pBufHeader);
                OMX_OSAL_MutexUnlock(pCtxt->hOutLock);
                OMX_CONF_BAIL_IF_ERROR(OMX_FillThisBuffer(pCtxt->hWComp, pBufHeader));
            }
            if (bTimeout == OMX_TRUE)
                OMX_CONF_SET_ERROR_BAIL("No Out buffers being processed\n", OMX_ErrorTimeout);
            OMX_OSAL_EventWait(pCtxt->hFillBufDoneEvent,
                               OMX_CONF_TIMEOUT_BUFFER_TRAFFIC, &bTimeout);
        }
        while (pCtxt->nBufDoneCalls < OMX_CONF_BUFFERS_OF_TRAFFIC);

OMX_CONF_TEST_BAIL:
        if (eError != OMX_ErrorNone)
        {
            if (pCtxt->bStop && eError == OMX_ErrorIncorrectStateOperation)
            {
                /* Expected behavior; fix the list */
                eError = OMX_ErrorNone;
                OMX_OSAL_MutexLock(pCtxt->hOutLock);
                pCtxt->nOutBufBusy--;
                LIST_SET_ENTRY(pCtxt->pOutBufferList, pBufHeader);
                OMX_OSAL_MutexUnlock(pCtxt->hOutLock);
            }
            OMX_CONF_ErrorToString(eError, szDesc);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "Output thread failed with %s\n", szDesc);
            pCtxt->nBufDoneCalls = OMX_CONF_BUFFERS_OF_TRAFFIC;
            pCtxt->eThreadError = eError;
        }
        return 0;
    }

    /*****************************************************************************/
    OMX_U32 BaseMultiThreadedTest_WriteInBuffers(OMX_PTR pParam)
    {
        BufferList * pList;
        OMX_BOOL bTimeout = OMX_FALSE;
        OMX_BUFFERHEADERTYPE *pBufHeader = NULL;
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        BaseMultiThreadedTestCtxt * pCtxt;
        pCtxt = (BaseMultiThreadedTestCtxt *)pParam;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Created new thread for input processing\n");
        do
        {
            OMX_OSAL_EventReset(pCtxt->hEmptyBufDoneEvent);
            pList = pCtxt->pInBufferList;
            while (pList && pList->pBufHdr && pCtxt->nBufDoneCalls < OMX_CONF_BUFFERS_OF_TRAFFIC)
            {
                OMX_OSAL_MutexLock(pCtxt->hInLock);
                pCtxt->nInBufBusy++;
                LIST_CLEAR_ENTRY(pList, pBufHeader);
                OMX_OSAL_MutexUnlock(pCtxt->hInLock);
                pBufHeader->nFilledLen = OMX_OSAL_ReadFromInputFile(pBufHeader->pBuffer,
                                         pBufHeader->nAllocLen, pBufHeader->nInputPortIndex);
                if (pBufHeader->nFilledLen != pBufHeader->nAllocLen)
                {
                    pBufHeader->nFlags |= OMX_BUFFERFLAG_EOS;
                    pCtxt->nBufDoneCalls = OMX_CONF_BUFFERS_OF_TRAFFIC;
                }
                OMX_CONF_BAIL_IF_ERROR(OMX_EmptyThisBuffer(pCtxt->hWComp, pBufHeader));
            }
            if (bTimeout == OMX_TRUE)
                OMX_CONF_SET_ERROR_BAIL("No In buffers being processed\n", OMX_ErrorTimeout);
            OMX_OSAL_EventWait(pCtxt->hEmptyBufDoneEvent,
                               OMX_CONF_TIMEOUT_BUFFER_TRAFFIC, &bTimeout);
        }
        while (pCtxt->nBufDoneCalls < OMX_CONF_BUFFERS_OF_TRAFFIC);

OMX_CONF_TEST_BAIL:
        if (eError != OMX_ErrorNone)
        {
            if (pCtxt->bStop && eError == OMX_ErrorIncorrectStateOperation)
            {
                /* Expected behavior; fix the list */
                eError = OMX_ErrorNone;

                OMX_OSAL_MutexLock(pCtxt->hInLock);
                pCtxt->nInBufBusy--;
                LIST_SET_ENTRY(pCtxt->pInBufferList, pBufHeader);
                OMX_OSAL_MutexUnlock(pCtxt->hInLock);
            }
            OMX_CONF_ErrorToString(eError, szDesc);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "Input thread failed with %s\n", szDesc);
            pCtxt->nBufDoneCalls = OMX_CONF_BUFFERS_OF_TRAFFIC;
            pCtxt->eThreadError = eError;
        }
        return 0;
    }

    /*****************************************************************************/
    OMX_ERRORTYPE BaseMultiThreadedTest_OperateOnPorts(BaseMultiThreadedTestCtxt* pCtxt, PortOpType eOp)
    {
        OMX_PARAM_PORTDEFINITIONTYPE sPortDef;
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_U32 i, j;

        for (j = 0; j < NUM_DOMAINS; j++)
        {
            for (i = pCtxt->sPortParam[j].nStartPortNumber;
                    i < pCtxt->sPortParam[j].nStartPortNumber + pCtxt->sPortParam[j].nPorts; i++)
            {

                OMX_CONF_INIT_STRUCT(sPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
                sPortDef.nPortIndex = i;
                OMX_CONF_BAIL_IF_ERROR(OMX_GetParameter(pCtxt->hWComp, OMX_IndexParamPortDefinition ,
                                                        (OMX_PTR)&sPortDef));
                switch (eOp)
                {
                    case AllocBuf:
                        if (0x0 == sPortDef.nBufferCountMin || sPortDef.nBufferCountMin > sPortDef.nBufferCountActual)
                        {
                            OMX_CONF_SET_ERROR_BAIL("PortDefinition nBufferCount incorrect\n", OMX_ErrorUndefined);
                        }
                        OMX_CONF_BAIL_IF_ERROR(BaseMultiThreadedTest_AllocateBuffers(pCtxt, &sPortDef));
                        break;

                    case OpenFile:
                        /* set buffercount actual */
                        if (0x0 == sPortDef.nBufferCountMin || sPortDef.nBufferCountMin > sPortDef.nBufferCountActual)
                        {
                            OMX_CONF_SET_ERROR_BAIL("PortDefinition nBufferCount incorrect\n", OMX_ErrorUndefined);
                        }
                        sPortDef.nBufferCountActual = sPortDef.nBufferCountMin + 1;
                        OMX_CONF_BAIL_IF_ERROR(OMX_SetParameter(pCtxt->hWComp, OMX_IndexParamPortDefinition,
                                                                (OMX_PTR)&sPortDef));
                        /* open files for input ports */
                        if (sPortDef.eDir == OMX_DirInput)
                        {
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Opened input file for port %d\n", i);
                            OMX_CONF_BAIL_IF_ERROR(OMX_OSAL_OpenInputFile(i));
                        }
                        break;

                    case CloseFile:
                        if (sPortDef.eDir == OMX_DirInput)
                        {
                            OMX_CONF_BAIL_IF_ERROR(OMX_OSAL_CloseInputFile(i));
                        }
                        break;

                    default:
                        eError = OMX_ErrorBadParameter;
                }
            }
        }
OMX_CONF_TEST_BAIL:
        return eError;
    }

    /*****************************************************************************/
    OMX_ERRORTYPE BaseMultiThreadedTest_TransitionWait(OMX_STATETYPE eToState,
            BaseMultiThreadedTestCtxt* pCtxt)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_BOOL bTimeout = OMX_FALSE;

        OMX_OSAL_EventReset(pCtxt->hStateSetEvent);
        eError = OMX_SendCommand(pCtxt->hWComp, OMX_CommandStateSet, eToState, 0);
        OMX_CONF_BAIL_IF_ERROR(eError);

        if (eToState == OMX_StateIdle && (pCtxt->eState == OMX_StateLoaded))
        {
            OMX_CONF_BAIL_IF_ERROR(BaseMultiThreadedTest_OperateOnPorts(pCtxt, AllocBuf));
        }
        else if (eToState == OMX_StateLoaded && pCtxt->eState == OMX_StateIdle)
        {
            OMX_CONF_BAIL_IF_ERROR(BaseMultiThreadedTest_DeInitBuffer(pCtxt));
        }

        OMX_OSAL_EventWait(pCtxt->hStateSetEvent,
                           OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);

        if (bTimeout == OMX_TRUE)
            OMX_CONF_SET_ERROR_BAIL("Transition timed out\n", OMX_ErrorUndefined);

        if (pCtxt->eState != eToState)
            OMX_CONF_SET_ERROR_BAIL("Incorrect transition\n", OMX_ErrorUndefined);

OMX_CONF_TEST_BAIL:
        return eError;
    }

    /*****************************************************************************/
    OMX_ERRORTYPE BaseMultiThreadedTest_TransmitTest(BaseMultiThreadedTestCtxt* pCtxt,
            OMX_BOOL bInit, OMX_BOOL bClose)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_HANDLETYPE hInThread, hOutThread;
        OMX_BOOL bTimeout;

        hInThread = hOutThread = 0;

        if (bInit)
        {
            OMX_CONF_BAIL_IF_ERROR(BaseMultiThreadedTest_OperateOnPorts(pCtxt, OpenFile));
            OMX_CONF_BAIL_IF_ERROR(BaseMultiThreadedTest_TransitionWait(OMX_StateIdle, pCtxt));
            OMX_CONF_BAIL_IF_ERROR(BaseMultiThreadedTest_TransitionWait(OMX_StateExecuting, pCtxt));
        }

        if (pCtxt->nOutBuf)
            OMX_CONF_BAIL_IF_ERROR(OMX_OSAL_ThreadCreate(BaseMultiThreadedTest_ReadOutBuffers, (OMX_PTR)pCtxt, 0, &hOutThread));
        if (pCtxt->nInBuf)
            OMX_CONF_BAIL_IF_ERROR(OMX_OSAL_ThreadCreate(BaseMultiThreadedTest_WriteInBuffers, (OMX_PTR)pCtxt, 0, &hInThread));

        if (bClose)
        {
            /* process buffers */
            while (pCtxt->nBufDoneCalls < OMX_CONF_BUFFERS_OF_TRAFFIC)
            {
                OMX_OSAL_EventReset(pCtxt->hBufDoneCallsEvent);
                OMX_OSAL_EventWait(pCtxt->hBufDoneCallsEvent, OMX_CONF_TIMEOUT_BUFFER_TRAFFIC, &bTimeout);
            }

            if (pCtxt->nOutBuf)
                OMX_CONF_BAIL_IF_ERROR(OMX_OSAL_ThreadDestroy(hOutThread));
            if (pCtxt->nInBuf)
                OMX_CONF_BAIL_IF_ERROR(OMX_OSAL_ThreadDestroy(hInThread));

            if (pCtxt->eThreadError)
            {
                OMX_CONF_SET_ERROR_BAIL("Exiting due to prior errors\n", OMX_ErrorUndefined);
            }

            if (!pCtxt->bStop)
                OMX_CONF_BAIL_IF_ERROR(BaseMultiThreadedTest_TransitionWait(OMX_StateIdle, pCtxt));

            /* wait for all buffers to be returned */
            while (pCtxt->nInBufBusy)
            {
                OMX_OSAL_EventReset(pCtxt->hEmptyBufDoneEvent);
                OMX_OSAL_EventWait(pCtxt->hEmptyBufDoneEvent,
                                   OMX_CONF_TIMEOUT_BUFFER_TRAFFIC, &bTimeout);
                if (OMX_TRUE == bTimeout)
                {
                    OMX_CONF_SET_ERROR_BAIL("All In buffers not returned\n", OMX_ErrorUndefined);
                }
            }
            while (pCtxt->nOutBufBusy)
            {
                OMX_OSAL_EventReset(pCtxt->hFillBufDoneEvent);
                OMX_OSAL_EventWait(pCtxt->hFillBufDoneEvent,
                                   OMX_CONF_TIMEOUT_BUFFER_TRAFFIC, &bTimeout);
                if (OMX_TRUE == bTimeout)
                {
                    OMX_CONF_SET_ERROR_BAIL("All Out buffers not returned\n", OMX_ErrorUndefined);
                }
            }

            OMX_CONF_BAIL_IF_ERROR(BaseMultiThreadedTest_TransitionWait(OMX_StateLoaded, pCtxt));
            OMX_CONF_BAIL_IF_ERROR(BaseMultiThreadedTest_OperateOnPorts(pCtxt, CloseFile));
        }
        pCtxt->nBufDoneCalls = 0;
OMX_CONF_TEST_BAIL:
        return eError;
    }


    /*****************************************************************************/
    OMX_ERRORTYPE OMX_CONF_BaseMultiThreadedTest(OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_HANDLETYPE hComp  = 0;
        OMX_CALLBACKTYPE sCallbacks;
        BaseMultiThreadedTestCtxt sCtxt;
        OMX_HANDLETYPE hWrappedComp = 0;
        OMX_CALLBACKTYPE *pWrappedCallbacks;
        OMX_PTR pWrappedAppData;
        OMX_HANDLETYPE hStartThread, hStopThread;
        OMX_BOOL bTimeout;
        BaseMultiThreadedTestCtxt *pCtxt;
        pCtxt = &sCtxt;
        memset(pCtxt, 0x0, sizeof(BaseMultiThreadedTestCtxt));

        sCallbacks.EventHandler    =  BaseMultiThreadedTest_EventHandler;
        sCallbacks.EmptyBufferDone =  BaseMultiThreadedTest_EmptyBufferDone;
        sCallbacks.FillBufferDone  =  BaseMultiThreadedTest_FillBufferDone;

        eError = OMX_CONF_CallbackTracerCreate(&sCallbacks, (OMX_PTR)pCtxt, cComponentName,
                                               &pWrappedCallbacks, &pWrappedAppData);

        /* initialize events to track callbacks */
        OMX_OSAL_EventCreate(&pCtxt->hStateSetEvent);
        OMX_OSAL_EventReset(pCtxt->hStateSetEvent);
        OMX_OSAL_EventCreate(&pCtxt->hFillBufDoneEvent);
        OMX_OSAL_EventReset(pCtxt->hFillBufDoneEvent);
        OMX_OSAL_EventCreate(&pCtxt->hEmptyBufDoneEvent);
        OMX_OSAL_EventReset(pCtxt->hEmptyBufDoneEvent);
        OMX_OSAL_EventCreate(&pCtxt->hThreadEvent);
        OMX_OSAL_EventReset(pCtxt->hThreadEvent);
        OMX_OSAL_MutexCreate(&pCtxt->hInLock);
        OMX_OSAL_MutexCreate(&pCtxt->hOutLock);
        OMX_OSAL_EventCreate(&pCtxt->hBufDoneCallsEvent);
        OMX_OSAL_EventReset(pCtxt->hBufDoneCallsEvent);

        /* Initialize OpenMax */
        eError = OMX_Init();
        OMX_CONF_BAIL_IF_ERROR(eError);

        OMX_CONF_BAIL_IF_ERROR(OMX_GetHandle(&hComp, cComponentName, pWrappedAppData,
                                             pWrappedCallbacks));
        OMX_CONF_BAIL_IF_ERROR(OMX_CONF_ComponentTracerCreate(hComp, cComponentName,
                               &hWrappedComp));
        pCtxt->hWComp = hWrappedComp;

        /* Verify start in Loaded state */
        OMX_CONF_BAIL_IF_ERROR(OMX_GetState(pCtxt->hWComp, &pCtxt->eState));
        if (pCtxt->eState != OMX_StateLoaded)
            OMX_CONF_SET_ERROR_BAIL("Component not in loaded state at init\n", OMX_ErrorUndefined);

        /* detect all audio ports on the component */
        OMX_CONF_INIT_STRUCT(pCtxt->sPortParam[0], OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(pCtxt->hWComp, OMX_IndexParamAudioInit,
                                  (OMX_PTR) & pCtxt->sPortParam[0]);
        if (OMX_ErrorUnsupportedIndex == eError)
            eError = OMX_ErrorNone;
        OMX_CONF_BAIL_IF_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "detected %i audio ports starting at %i \n",
                       pCtxt->sPortParam[0].nPorts, pCtxt->sPortParam[0].nStartPortNumber);

        /* detect all video ports on the component */
        OMX_CONF_INIT_STRUCT(pCtxt->sPortParam[1], OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(pCtxt->hWComp, OMX_IndexParamVideoInit,
                                  (OMX_PTR) & pCtxt->sPortParam[1]);
        if (OMX_ErrorUnsupportedIndex == eError)
            eError = OMX_ErrorNone;
        OMX_CONF_BAIL_IF_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "detected %i video ports starting at %i \n",
                       pCtxt->sPortParam[1].nPorts, pCtxt->sPortParam[1].nStartPortNumber);

        /* detect all image ports on the component */
        OMX_CONF_INIT_STRUCT(pCtxt->sPortParam[2], OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(pCtxt->hWComp, OMX_IndexParamImageInit,
                                  (OMX_PTR) & pCtxt->sPortParam[2]);
        if (OMX_ErrorUnsupportedIndex == eError)
            eError = OMX_ErrorNone;
        OMX_CONF_BAIL_IF_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "detected %i image ports starting at %i \n",
                       pCtxt->sPortParam[2].nPorts, pCtxt->sPortParam[2].nStartPortNumber);

        /* detect all other ports on the component */
        OMX_CONF_INIT_STRUCT(pCtxt->sPortParam[3], OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(pCtxt->hWComp, OMX_IndexParamOtherInit,
                                  (OMX_PTR) & pCtxt->sPortParam[3]);
        if (OMX_ErrorUnsupportedIndex == eError)
            eError = OMX_ErrorNone;
        OMX_CONF_BAIL_IF_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "detected %i other ports starting at %i \n",
                       pCtxt->sPortParam[3].nPorts, pCtxt->sPortParam[3].nStartPortNumber);

        /* 3.7.1. Buffer processing in a separate thread */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Buffer processing in separate thread\n");
        OMX_CONF_BAIL_IF_ERROR(BaseMultiThreadedTest_TransmitTest(pCtxt, OMX_TRUE, OMX_TRUE));

        /* 3.7.2. Initialization from 2 threads */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Initialization from 2 threads\n");

        /* open input file for each input port*/
        OMX_CONF_BAIL_IF_ERROR(BaseMultiThreadedTest_OperateOnPorts(pCtxt, OpenFile));

        /* create a separate initialization thread */
        OMX_CONF_BAIL_IF_ERROR(OMX_OSAL_ThreadCreate(BaseMultiThreadedTest_StartThread,
                               (OMX_PTR)pCtxt, 0, &hStartThread));

        /* wait for initialization thread to send idle command */
        OMX_OSAL_EventWait(pCtxt->hThreadEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
        if (bTimeout == OMX_TRUE)
            OMX_CONF_SET_ERROR_BAIL("Wait for cmd idle timed out\n", OMX_ErrorUndefined);
        OMX_CONF_BAIL_IF_ERROR(pCtxt->eThreadError);

        /* allocate buffers */
        OMX_CONF_BAIL_IF_ERROR(BaseMultiThreadedTest_OperateOnPorts(pCtxt, AllocBuf));
        OMX_CONF_BAIL_IF_ERROR(pCtxt->eThreadError);

        /* wait for initialization thread to put component in execute state */
        WAIT_FOR_STATE_CHANGE(pCtxt->hThreadEvent, OMX_StateExecuting, eError, pCtxt, bTimeout);
        OMX_CONF_BAIL_IF_ERROR(pCtxt->eThreadError);

        /* do normal buffer processing */
        OMX_CONF_BAIL_IF_ERROR(BaseMultiThreadedTest_TransmitTest(pCtxt, OMX_FALSE, OMX_TRUE));

        OMX_CONF_BAIL_IF_ERROR(OMX_OSAL_ThreadDestroy(hStartThread));
        OMX_CONF_BAIL_IF_ERROR(pCtxt->eThreadError);

        /* 3.7.3. Start and Stop in differemt threads */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Start and stop in differemt threads\n");
        OMX_OSAL_EventReset(pCtxt->hThreadEvent);

        /* do normal buffer processing */
        OMX_CONF_BAIL_IF_ERROR(BaseMultiThreadedTest_TransmitTest(pCtxt, OMX_TRUE, OMX_FALSE));

        /* proces atleast half the buffers */
        while (pCtxt->nBufDoneCalls < (OMX_CONF_BUFFERS_OF_TRAFFIC >> 1))
        {
            OMX_OSAL_EventReset(pCtxt->hBufDoneCallsEvent);
            OMX_OSAL_EventWait(pCtxt->hBufDoneCallsEvent, OMX_CONF_TIMEOUT_BUFFER_TRAFFIC, &bTimeout);
        }

        /* create a separate thread to send idle command */
        OMX_CONF_BAIL_IF_ERROR(OMX_OSAL_ThreadCreate(BaseMultiThreadedTest_StopThread,
                               (OMX_PTR)pCtxt, 0, &hStopThread));

        /* wait for stop thread to send idle command */
        OMX_OSAL_EventWait(pCtxt->hThreadEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
        if (bTimeout == OMX_TRUE)
            OMX_CONF_SET_ERROR_BAIL("Wait for cmd idle timed out\n", OMX_ErrorUndefined);
        OMX_CONF_BAIL_IF_ERROR(pCtxt->eThreadError);

        /* wrap up buffer processing */
        WAIT_FOR_STATE_CHANGE(pCtxt->hThreadEvent, OMX_StateIdle, eError, pCtxt, bTimeout);
        OMX_CONF_BAIL_IF_ERROR(BaseMultiThreadedTest_TransmitTest(pCtxt, OMX_FALSE, OMX_TRUE));

        OMX_CONF_BAIL_IF_ERROR(OMX_OSAL_ThreadDestroy(hStopThread));
        OMX_CONF_BAIL_IF_ERROR(pCtxt->eThreadError);

OMX_CONF_TEST_BAIL:
        /* cleanup: return function errors rather than closing errors if appropriate */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Cleanup\n");

        if (OMX_ErrorNone == eError)
        {
            if (hWrappedComp)
            {
                eError = OMX_CONF_ComponentTracerDestroy(hWrappedComp);
            }
            if (hComp)
            {
                eError = OMX_FreeHandle(hComp);
            }
            eError = OMX_CONF_CallbackTracerDestroy(pWrappedCallbacks, pWrappedAppData);

            eError = OMX_Deinit();
        }
        else
        {
            BaseMultiThreadedTest_TransitionWait(OMX_StateInvalid, pCtxt);
            BaseMultiThreadedTest_DeInitBuffer(pCtxt);

            if (hWrappedComp)
            {
                OMX_CONF_ComponentTracerDestroy(hWrappedComp);
            }
            if (hComp)
            {
                OMX_FreeHandle(hComp);
            }
            OMX_CONF_CallbackTracerDestroy(pWrappedCallbacks, pWrappedAppData);
            OMX_Deinit();
        }

        OMX_OSAL_EventDestroy(pCtxt->hStateSetEvent);
        OMX_OSAL_EventDestroy(pCtxt->hFillBufDoneEvent);
        OMX_OSAL_EventDestroy(pCtxt->hEmptyBufDoneEvent);
        OMX_OSAL_EventDestroy(pCtxt->hThreadEvent);
        OMX_OSAL_MutexDestroy(pCtxt->hInLock);
        OMX_OSAL_MutexDestroy(pCtxt->hOutLock);
        OMX_OSAL_EventDestroy(pCtxt->hBufDoneCallsEvent);

        return eError;
    }

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */
