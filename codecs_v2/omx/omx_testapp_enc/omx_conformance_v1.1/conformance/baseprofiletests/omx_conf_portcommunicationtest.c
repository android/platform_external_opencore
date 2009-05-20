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

/** OMX_CONF_PortCommunicationTest.c
 *  OpenMax IL conformance test - Port Communication Test
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
#define TEST_NAME_STRING "PortCommunicationTest"
#define TEST_COMPONENT_NAME_SIZE 128
    char szDesc[256];
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

#define LIST_CLEAR_ENTRY(_pL, _pB)\
    _pB = _pL->pBufHdr;		\
    _pL->pBufHdr = NULL;	\
    _pL = _pL->pNextBuf;

#define LIST_SET_ENTRY(_pL, _pB)	\
{				\
    BufferList *_pT = _pL;	\
    while(_pT && _pT->pBufHdr){	\
        _pT = _pT->pNextBuf;	\
    }				\
    if(_pT)			\
        _pT->pBufHdr = _pB;	\
}

#define OMX_CONF_EVENT_HANDLER_ERROR(_nX, _nY, _nZ, _desc)	\
{								\
    _nX = _nY + 1;						\
    _nZ = OMX_CONF_BUFFERS_OF_TRAFFIC;				\
    OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "%s\n", _desc);		\
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
        OMX_U32 nBufFactor;
        OMX_BOOL bClientAllocBuf;
        OMX_HANDLETYPE hWComp;
        OMX_HANDLETYPE hStateSetEvent;
        OMX_HANDLETYPE hPortDisableEvent;
        OMX_HANDLETYPE hPortEnableEvent;
        OMX_HANDLETYPE hBufDoneEvent;
        OMX_HANDLETYPE hEmptyBufDoneEvent;
        BufferList *pInBufferList;
        BufferList *pOutBufferList;
        OMX_U32 nBufDoneCalls;
        OMX_U32 nPorts;
        OMX_U32 nPortsStopped;
        OMX_U32 nPortsRestarted;
        OMX_U32 nInBuf;
        OMX_U32 nOutBuf;
        OMX_U32 nInBufBusy;
        OMX_U32 nOutBufBusy;
        OMX_STATETYPE eState;
        OMX_PORT_PARAM_TYPE sPortParam[NUM_DOMAINS];
    } PortCommTestCtxt;

    typedef enum PortOpType
    {
        AllocBuf,
        NonTunnelTest,
        OpenFile,
        CloseFile,
        EmptyFill,
    } PortOpType;


    /*
     *  E X T E R N A L   F U N C T I O N S
     */
    OMX_ERRORTYPE OMX_CONF_CommandToString(
        OMX_IN OMX_COMMANDTYPE eCommand,
        OMX_OUT OMX_STRING sName);
    OMX_ERRORTYPE OMX_CONF_StateToString(
        OMX_IN OMX_STATETYPE eState,
        OMX_OUT OMX_STRING sName);
    OMX_ERRORTYPE OMX_CONF_ErrorToString(
        OMX_IN OMX_ERRORTYPE eError,
        OMX_OUT OMX_STRING sName);


    /*
     *     F U N C T I O N S
     */

    /*****************************************************************************/
    OMX_ERRORTYPE PortCommTest_EventHandler(OMX_IN OMX_HANDLETYPE hComponent,
                                            OMX_IN OMX_PTR pAppData,
                                            OMX_IN OMX_EVENTTYPE eEvent,
                                            OMX_IN OMX_U32 nData1,
                                            OMX_IN OMX_U32 nData2,
                                            OMX_IN OMX_PTR pEventData)
    {
        PortCommTestCtxt* pContext;

        UNUSED_PARAMETER(hComponent);
        UNUSED_PARAMETER(pEventData);

        if (pAppData == NULL)
            return OMX_ErrorNone;
        pContext = (PortCommTestCtxt*)pAppData;

        if (eEvent == OMX_EventCmdComplete)
        {
            switch ((OMX_COMMANDTYPE)(nData1))
            {
                case OMX_CommandStateSet:
                    OMX_CONF_StateToString((OMX_STATETYPE)(nData2), szDesc);
                    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Component transitioned to %s\n", szDesc);
                    pContext->eState = (OMX_STATETYPE)(nData2);
                    OMX_OSAL_EventSet(pContext->hStateSetEvent);
                    break;
                case OMX_CommandPortDisable:
                    pContext->nPortsStopped++;
                    if (pContext->nPortsStopped == pContext->nPorts)
                    {
                        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "All ports completed stopping\n");
                        OMX_OSAL_EventSet(pContext->hPortDisableEvent);
                    }
                    break;
                case OMX_CommandPortEnable:
                    pContext->nPortsRestarted++;
                    if (pContext->nPortsRestarted == pContext->nPorts)
                    {
                        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "All ports restarted\n");
                        OMX_OSAL_EventSet(pContext->hPortEnableEvent);
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
    OMX_ERRORTYPE PortCommTest_EmptyBufferDone(OMX_IN OMX_HANDLETYPE hComponent,
            OMX_IN OMX_PTR pAppData,
            OMX_IN OMX_BUFFERHEADERTYPE* pBuffer)
    {
        PortCommTestCtxt* pCtxt;

        UNUSED_PARAMETER(hComponent);

        if (pAppData == NULL)
            return OMX_ErrorNone;
        pCtxt = (PortCommTestCtxt*)pAppData;

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
            OMX_OSAL_EventSet(pCtxt->hBufDoneEvent);
        }
        else
        {
            OMX_CONF_EVENT_HANDLER_ERROR(pCtxt->nInBufBusy, pCtxt->nInBuf, pCtxt->nBufDoneCalls,
                                         "Component returned more input buffers than expected\n");
        }

        return OMX_ErrorNone;
    }

    /*****************************************************************************/
    OMX_ERRORTYPE PortCommTest_FillBufferDone(OMX_OUT OMX_HANDLETYPE hComponent,
            OMX_OUT OMX_PTR pAppData,
            OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer)
    {
        PortCommTestCtxt* pCtxt;

        UNUSED_PARAMETER(hComponent);

        if (pAppData == NULL)
            return OMX_ErrorNone;
        pCtxt = (PortCommTestCtxt*)pAppData;

        if (pBuffer->nInputPortIndex != OMX_NOPORT || pBuffer->nOutputPortIndex == OMX_NOPORT)
        {
            OMX_CONF_EVENT_HANDLER_ERROR(pCtxt->nInBufBusy, pCtxt->nInBuf, pCtxt->nBufDoneCalls,
                                         "Component returned invalid output buffer\n");
        }
        else if (pCtxt->nOutBufBusy)
        {
            pCtxt->nOutBufBusy--;
            pCtxt->nBufDoneCalls++;
            LIST_SET_ENTRY(pCtxt->pOutBufferList, pBuffer);
            OMX_OSAL_EventSet(pCtxt->hBufDoneEvent);
        }
        else
        {
            OMX_CONF_EVENT_HANDLER_ERROR(pCtxt->nInBufBusy, pCtxt->nInBuf, pCtxt->nBufDoneCalls,
                                         "Component returned more output buffers than expected\n");
        }

        return OMX_ErrorNone;
    }

    /*****************************************************************************/
    OMX_ERRORTYPE PortCommTest_AllocateBuffers(PortCommTestCtxt *pContext,
            OMX_PARAM_PORTDEFINITIONTYPE *pPortDef)
    {
        OMX_BUFFERHEADERTYPE *pBufferHdr = NULL;
        OMX_U8 *pBuffer = NULL;
        BufferList *pBufferList;
        BufferList *pTemp;
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_U32 i;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Allocate buffers for port %d\n", pPortDef->nPortIndex);
        for (i = 0x0; i < pPortDef->nBufferCountActual; i++)
        {
            pBufferList = (BufferList *)OMX_OSAL_Malloc(sizeof(BufferList));
            if (!pBufferList)
                OMX_CONF_SET_ERROR_BAIL("malloc failed\n", OMX_ErrorInsufficientResources);

            if (pContext->bClientAllocBuf)
            {
                pBuffer = (OMX_U8*)OMX_OSAL_Malloc(pPortDef->nBufferSize);
                if (!pBuffer)
                    OMX_CONF_SET_ERROR_BAIL("malloc failed\n", OMX_ErrorInsufficientResources);
                OMX_CONF_BAIL_IF_ERROR(OMX_UseBuffer(pContext->hWComp, &pBufferHdr, pPortDef->nPortIndex,
                                                     0, pPortDef->nBufferSize, pBuffer));
            }
            else
            {
                OMX_CONF_BAIL_IF_ERROR(OMX_AllocateBuffer(pContext->hWComp, &pBufferHdr, pPortDef->nPortIndex,
                                       0, pPortDef->nBufferSize));
            }
            pBufferList->pNextBuf = NULL;
            pBufferList->pBufHdr = pBufferHdr;
            pBufferList->pOrigBufHdr = pBufferHdr;

            if (pPortDef->eDir == OMX_DirInput)
            {
                /* check if buffer header is as expected */
                if (pBufferHdr->nAllocLen != pPortDef->nBufferSize || !pBufferHdr->pBuffer ||
                        pBufferHdr->nInputPortIndex != pPortDef->nPortIndex)
                    OMX_CONF_SET_ERROR_BAIL("Buffer header incorrect\n", OMX_ErrorUndefined);

                pBufferHdr->nOutputPortIndex = OMX_NOPORT;

                /* put it in internal list */
                if (pContext->pInBufferList == NULL)
                    pContext->pInBufferList = pBufferList;
                else
                {
                    pTemp = pContext->pInBufferList;
                    while (pTemp->pNextBuf)
                        pTemp = pTemp->pNextBuf;
                    pTemp->pNextBuf = pBufferList;
                }
                pContext->nInBuf++;
            }
            else
            {
                /* check if buffer header is as expected */
                if (pBufferHdr->nAllocLen != pPortDef->nBufferSize || !pBufferHdr->pBuffer ||
                        pBufferHdr->nOutputPortIndex != pPortDef->nPortIndex)
                    OMX_CONF_SET_ERROR_BAIL("Buffer header incorrect\n", OMX_ErrorUndefined);

                pBufferHdr->nInputPortIndex = OMX_NOPORT;

                /* put it in internal list */
                if (pContext->pOutBufferList  == NULL)
                    pContext->pOutBufferList = pBufferList;
                else
                {
                    pTemp = pContext->pOutBufferList;
                    while (pTemp->pNextBuf)
                        pTemp = pTemp->pNextBuf;
                    pTemp->pNextBuf = pBufferList;
                }
                pContext->nOutBuf++;
            }
        }

OMX_CONF_TEST_BAIL:
        return eError;
    }

    /*****************************************************************************/
    OMX_ERRORTYPE PortCommTest_DeInitBuffer(PortCommTestCtxt* pContext)
    {
        OMX_U8 *pBuffer;
        BufferList *pBufferListObject;
        BufferList *pTemp;
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Free all buffers\n");
        pTemp = pContext->pInBufferList;
        while (pTemp)
        {
            pBufferListObject = (BufferList *)pTemp;
            pBuffer = (OMX_U8 *)pTemp->pOrigBufHdr->pBuffer;
            if (pContext->bClientAllocBuf)
            {
                OMX_OSAL_Free(pBuffer);
                pTemp->pOrigBufHdr->pBuffer = NULL;
            }
            eError = OMX_FreeBuffer(pContext->hWComp, pTemp->pOrigBufHdr->nInputPortIndex, pTemp->pOrigBufHdr);
            if (eError == OMX_ErrorInvalidState)
                eError = OMX_ErrorNone;
            OMX_CONF_BAIL_IF_ERROR(eError);
            pTemp = pTemp->pNextBuf;
            if (pBufferListObject)
                OMX_OSAL_Free(pBufferListObject);
            pContext->nInBuf--;
        }
        pContext->pInBufferList = NULL;

        pTemp = pContext->pOutBufferList;
        while (pTemp)
        {
            pBufferListObject = (BufferList *)pTemp;
            pBuffer = (OMX_U8 *)pTemp->pOrigBufHdr->pBuffer;
            if (pContext->bClientAllocBuf)
            {
                OMX_OSAL_Free(pBuffer);
                pTemp->pOrigBufHdr->pBuffer = NULL;
            }
            eError = OMX_FreeBuffer(pContext->hWComp, pTemp->pOrigBufHdr->nOutputPortIndex, pTemp->pOrigBufHdr);
            if (eError == OMX_ErrorInvalidState)
                eError = OMX_ErrorNone;
            OMX_CONF_BAIL_IF_ERROR(eError);
            pTemp = pTemp->pNextBuf;
            if (pBufferListObject)
                OMX_OSAL_Free(pBufferListObject);
            pContext->nOutBuf--;
        }
        pContext->pOutBufferList = NULL;
OMX_CONF_TEST_BAIL:
        return eError;
    }

    /*****************************************************************************/
    OMX_ERRORTYPE PortCommTest_ReadOutBuffers(PortCommTestCtxt* pContext)
    {
        BufferList * pList;
        OMX_BUFFERHEADERTYPE *pBufHeader;
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        pList = pContext->pOutBufferList;
        while (pList && pList->pBufHdr)
        {
            pContext->nOutBufBusy++;
            LIST_CLEAR_ENTRY(pList, pBufHeader);
            OMX_CONF_BAIL_IF_ERROR(OMX_FillThisBuffer(pContext->hWComp, pBufHeader));
            if (pBufHeader->nFlags == OMX_BUFFERFLAG_EOS)
                pContext->nBufDoneCalls = OMX_CONF_BUFFERS_OF_TRAFFIC;
        }
OMX_CONF_TEST_BAIL:
        return eError;
    }

    /*****************************************************************************/
    OMX_ERRORTYPE PortCommTest_WriteInBuffers(PortCommTestCtxt* pContext)
    {
        BufferList * pList;
        OMX_BUFFERHEADERTYPE *pBufHeader;
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_U32 nPayload;

        pList = pContext->pInBufferList;
        while (pList && pList->pBufHdr)
        {
            pContext->nInBufBusy++;
            LIST_CLEAR_ENTRY(pList, pBufHeader);
            nPayload = (pContext->nBufFactor) ? pBufHeader->nAllocLen / pContext->nBufFactor : 1;
            pBufHeader->nFilledLen = OMX_OSAL_ReadFromInputFile(pBufHeader->pBuffer, nPayload,
                                     pBufHeader->nInputPortIndex);
            if (pBufHeader->nFilledLen != nPayload)
            {
                pBufHeader->nFlags |= OMX_BUFFERFLAG_EOS;
            }
            OMX_CONF_BAIL_IF_ERROR(OMX_EmptyThisBuffer(pContext->hWComp, pBufHeader));
        }

OMX_CONF_TEST_BAIL:
        return eError;
    }

    /*****************************************************************************/
    OMX_ERRORTYPE PortCommTest_OperateOnPorts(PortCommTestCtxt* pContext, PortOpType eOp)
    {
        OMX_PARAM_PORTDEFINITIONTYPE sPortDef;
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_U32 i, j;
        OMX_BUFFERHEADERTYPE sBufHdr;

        for (j = 0; j < NUM_DOMAINS; j++)
        {
            for (i = pContext->sPortParam[j].nStartPortNumber;
                    i < pContext->sPortParam[j].nStartPortNumber + pContext->sPortParam[j].nPorts; i++)
            {

                OMX_CONF_INIT_STRUCT(sPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
                sPortDef.nPortIndex = i;
                OMX_CONF_BAIL_IF_ERROR(OMX_GetParameter(pContext->hWComp, OMX_IndexParamPortDefinition ,
                                                        (OMX_PTR)&sPortDef));
                switch (eOp)
                {
                    case AllocBuf:
                        if (0x0 == sPortDef.nBufferCountMin || sPortDef.nBufferCountMin > sPortDef.nBufferCountActual)
                        {
                            OMX_CONF_SET_ERROR_BAIL("PortDefinition nBufferCount incorrect\n", OMX_ErrorUndefined);
                        }
                        OMX_CONF_BAIL_IF_ERROR(PortCommTest_AllocateBuffers(pContext, &sPortDef));
                        break;

                    case NonTunnelTest:
                        if (sPortDef.eDir == OMX_DirInput)
                        {
                            eError = OMX_SetupTunnel(NULL, 0, pContext->hWComp, sPortDef.nPortIndex);
                            OMX_CONF_ErrorToString(eError, szDesc);
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Setup tunnel reported error code %s\n", szDesc);
                        }
                        else
                        {
                            eError = OMX_SetupTunnel(pContext->hWComp, sPortDef.nPortIndex, NULL, 0);
                            OMX_CONF_ErrorToString(eError, szDesc);
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Setup tunnel reported error code %s\n", szDesc);
                        }
                        eError = OMX_ErrorNone;
                        break;

                    case OpenFile:
                        /* set buffercount actual */
                        if (0x0 == sPortDef.nBufferCountMin || sPortDef.nBufferCountMin > sPortDef.nBufferCountActual)
                        {
                            OMX_CONF_SET_ERROR_BAIL("PortDefinition nBufferCount incorrect\n", OMX_ErrorUndefined);
                        }
                        sPortDef.nBufferCountActual = sPortDef.nBufferCountMin + 1;
                        OMX_CONF_BAIL_IF_ERROR(OMX_SetParameter(pContext->hWComp, OMX_IndexParamPortDefinition,
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

                    case EmptyFill:
                        OMX_CONF_INIT_STRUCT(sBufHdr, OMX_BUFFERHEADERTYPE);
                        if (sPortDef.eDir == OMX_DirInput)
                        {
                            sBufHdr.nInputPortIndex = i;
                            eError = OMX_EmptyThisBuffer(pContext->hWComp, &sBufHdr);
                        }
                        else
                        {
                            sBufHdr.nOutputPortIndex = i;
                            eError = OMX_FillThisBuffer(pContext->hWComp, &sBufHdr);
                        }
                        if (eError != OMX_ErrorIncorrectStateOperation)
                            OMX_CONF_SET_ERROR_BAIL("Buffer handling while stopped\n", OMX_ErrorUndefined);
                        eError = OMX_ErrorNone;
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
    OMX_ERRORTYPE PortCommTest_TransitionWait(OMX_STATETYPE eToState,
            PortCommTestCtxt* pContext)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_BOOL bTimeout = OMX_FALSE;

        OMX_OSAL_EventReset(pContext->hStateSetEvent);
        eError = OMX_SendCommand(pContext->hWComp, OMX_CommandStateSet, eToState, 0);
        OMX_CONF_BAIL_IF_ERROR(eError);

        if (eToState == OMX_StateIdle && (pContext->eState == OMX_StateLoaded))
        {
            OMX_CONF_BAIL_IF_ERROR(PortCommTest_OperateOnPorts(pContext, AllocBuf));
        }
        else if (eToState == OMX_StateLoaded && pContext->eState == OMX_StateIdle)
        {
            OMX_CONF_BAIL_IF_ERROR(PortCommTest_DeInitBuffer(pContext));
        }

        OMX_OSAL_EventWait(pContext->hStateSetEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);

        if (bTimeout == OMX_TRUE)
            OMX_CONF_SET_ERROR_BAIL("Transition timed out\n", OMX_ErrorUndefined);

        if (pContext->eState != eToState)
            OMX_CONF_SET_ERROR_BAIL("Incorrect transition\n", OMX_ErrorUndefined);

OMX_CONF_TEST_BAIL:
        return eError;
    }

    /*****************************************************************************/
    OMX_ERRORTYPE PortCommTest_TransmitTest(PortCommTestCtxt* pContext,
                                            OMX_BOOL bInit, OMX_BOOL bClose)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_BOOL bTimeout;

        if (bInit)
        {
            OMX_CONF_BAIL_IF_ERROR(PortCommTest_OperateOnPorts(pContext, OpenFile));
            OMX_CONF_BAIL_IF_ERROR(PortCommTest_TransitionWait(OMX_StateIdle, pContext));
            OMX_CONF_BAIL_IF_ERROR(PortCommTest_TransitionWait(OMX_StateExecuting, pContext));
        }
        /* process buffers */
        while (pContext->nBufDoneCalls < OMX_CONF_BUFFERS_OF_TRAFFIC)
        {
            if (pContext->nOutBuf)
                OMX_CONF_BAIL_IF_ERROR(PortCommTest_ReadOutBuffers(pContext));
            if (pContext->nInBuf)
                OMX_CONF_BAIL_IF_ERROR(PortCommTest_WriteInBuffers(pContext));

            /* if no empty input or output buffer, wait for bufferdone call */
            if (pContext->nOutBuf == pContext->nOutBufBusy && pContext->nInBuf == pContext->nInBufBusy)
            {
                OMX_OSAL_EventReset(pContext->hBufDoneEvent);
                OMX_OSAL_EventWait(pContext->hBufDoneEvent, OMX_CONF_TIMEOUT_BUFFER_TRAFFIC, &bTimeout);
                if (OMX_TRUE == bTimeout)
                {
                    OMX_CONF_SET_ERROR_BAIL("Component is not processing any buffers\n", OMX_ErrorUndefined);
                }
            }
        }
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Processed %d buffers\n", pContext->nBufDoneCalls);
        if (bClose)
        {
            OMX_CONF_BAIL_IF_ERROR(PortCommTest_OperateOnPorts(pContext, CloseFile));
            OMX_CONF_BAIL_IF_ERROR(PortCommTest_TransitionWait(OMX_StateIdle, pContext));
            while (pContext->nOutBufBusy || pContext->nInBufBusy)
            {
                OMX_OSAL_EventReset(pContext->hBufDoneEvent);
                OMX_OSAL_EventWait(pContext->hBufDoneEvent, OMX_CONF_TIMEOUT_BUFFER_TRAFFIC, &bTimeout);
                if (OMX_TRUE == bTimeout)
                {
                    OMX_CONF_SET_ERROR_BAIL("All buffers not returned\n", OMX_ErrorUndefined);
                }
            }
            OMX_CONF_BAIL_IF_ERROR(PortCommTest_TransitionWait(OMX_StateLoaded, pContext));
        }
        pContext->nBufDoneCalls = 0;
OMX_CONF_TEST_BAIL:
        return eError;
    }

    /*****************************************************************************/
    OMX_ERRORTYPE PortCommTest_RepackTest(PortCommTestCtxt* pContext)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_BOOL bTimeout;

        OMX_CONF_BAIL_IF_ERROR(PortCommTest_OperateOnPorts(pContext, OpenFile));
        OMX_CONF_BAIL_IF_ERROR(PortCommTest_TransitionWait(OMX_StateIdle, pContext));

        if (pContext->nInBuf == 0)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Source component, Repack Test not applicable\n");
            OMX_CONF_BAIL_IF_ERROR(PortCommTest_OperateOnPorts(pContext, CloseFile));
            OMX_CONF_BAIL_IF_ERROR(PortCommTest_TransitionWait(OMX_StateLoaded, pContext));
            goto OMX_CONF_TEST_BAIL;
        }
        OMX_CONF_BAIL_IF_ERROR(PortCommTest_TransitionWait(OMX_StateExecuting, pContext));

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Sending single byte payloads\n");
        pContext->nBufFactor = 0;
        OMX_OSAL_EventReset(pContext->hEmptyBufDoneEvent);

        if (pContext->nOutBuf)
            OMX_CONF_BAIL_IF_ERROR(PortCommTest_ReadOutBuffers(pContext));
        if (pContext->nInBuf)
            OMX_CONF_BAIL_IF_ERROR(PortCommTest_WriteInBuffers(pContext));

        OMX_OSAL_EventWait(pContext->hEmptyBufDoneEvent, OMX_CONF_TIMEOUT_BUFFER_TRAFFIC, &bTimeout);
        if (pContext->nOutBuf && OMX_TRUE == bTimeout)
        {
            OMX_CONF_SET_ERROR_BAIL("Component is not repacking\n", OMX_ErrorUndefined);
        }
        if (pContext->nInBuf && !pContext->pInBufferList->pBufHdr)
        {
            OMX_CONF_SET_ERROR_BAIL("first buffer has to be returned\n", OMX_ErrorUndefined);
        }
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Sending full payloads\n");
        pContext->nBufFactor = 1;
        OMX_CONF_BAIL_IF_ERROR(PortCommTest_TransmitTest(pContext, OMX_FALSE, OMX_TRUE));

OMX_CONF_TEST_BAIL:
        return eError;
    }

    /*****************************************************************************/
    OMX_ERRORTYPE PortCommTest_StopRestartTest(PortCommTestCtxt* pContext)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_BOOL bTimeout;

        /* process max frames */
        OMX_CONF_BAIL_IF_ERROR(PortCommTest_TransmitTest(pContext, OMX_TRUE, OMX_FALSE));

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Stopping all ports\n");
        OMX_OSAL_EventReset(pContext->hPortDisableEvent);
        OMX_CONF_BAIL_IF_ERROR(OMX_SendCommand(pContext->hWComp, OMX_CommandPortDisable, OMX_ALL, 0x0));

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Wait for all buffers to be returned\n");
        while (pContext->nOutBufBusy || pContext->nInBufBusy)
        {
            OMX_OSAL_EventReset(pContext->hBufDoneEvent);
            OMX_OSAL_EventWait(pContext->hBufDoneEvent, OMX_CONF_TIMEOUT_BUFFER_TRAFFIC, &bTimeout);
            if (OMX_TRUE == bTimeout)
            {
                OMX_CONF_SET_ERROR_BAIL("All buffers not returned\n", OMX_ErrorUndefined);
            }
        }
        /* check if command completed before de-allocation*/
        OMX_OSAL_EventWait(pContext->hPortDisableEvent, OMX_CONF_TIMEOUT_EXPECTING_FAILURE, &bTimeout);
        if (OMX_FALSE == bTimeout)
        {
            OMX_CONF_SET_ERROR_BAIL("Ports stopped unexpectedly\n", OMX_ErrorUndefined);
        }
        OMX_OSAL_EventReset(pContext->hPortDisableEvent);

        /* free all buffers */
        OMX_CONF_BAIL_IF_ERROR(PortCommTest_DeInitBuffer(pContext));

        /* wait for cmd complete */
        OMX_OSAL_EventWait(pContext->hPortDisableEvent, OMX_CONF_TIMEOUT_BUFFER_TRAFFIC, &bTimeout);
        if (OMX_TRUE == bTimeout)
        {
            OMX_CONF_SET_ERROR_BAIL("All ports not stopped\n", OMX_ErrorUndefined);
        }

        /* ensure no more buffer done callbacks are made */
        OMX_OSAL_EventReset(pContext->hBufDoneEvent);
        OMX_OSAL_EventWait(pContext->hBufDoneEvent, OMX_CONF_TIMEOUT_EXPECTING_FAILURE, &bTimeout);
        if (OMX_FALSE == bTimeout)
        {
            OMX_CONF_SET_ERROR_BAIL("callbacks made after cmdcomplete\n", OMX_ErrorUndefined);
        }
        pContext->nBufDoneCalls = 0;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Attempting to process buffers while stopped\n");
        OMX_OSAL_EventReset(pContext->hBufDoneEvent);
        OMX_CONF_BAIL_IF_ERROR(PortCommTest_OperateOnPorts(pContext, EmptyFill));
        OMX_OSAL_EventWait(pContext->hBufDoneEvent, OMX_CONF_TIMEOUT_EXPECTING_FAILURE, &bTimeout);
        if (OMX_FALSE == bTimeout)
        {
            OMX_CONF_SET_ERROR_BAIL("callbacks made while ports disabled\n", OMX_ErrorUndefined);
        }

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Processed %d buffers\n", pContext->nBufDoneCalls);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Restarting all ports\n");
        OMX_OSAL_EventReset(pContext->hPortEnableEvent);

        /* send restart command */
        OMX_CONF_BAIL_IF_ERROR(OMX_SendCommand(pContext->hWComp, OMX_CommandPortEnable, OMX_ALL, 0x0));

        /* check if command completed before allocation*/
        OMX_OSAL_EventWait(pContext->hPortEnableEvent, OMX_CONF_TIMEOUT_EXPECTING_FAILURE, &bTimeout);
        if (OMX_FALSE == bTimeout)
        {
            OMX_CONF_SET_ERROR_BAIL("Ports restarted unexpectedly\n", OMX_ErrorUndefined);
        }
        OMX_OSAL_EventReset(pContext->hPortEnableEvent);

        /* allocate buffers */
        OMX_CONF_BAIL_IF_ERROR(PortCommTest_OperateOnPorts(pContext, AllocBuf));

        /* wait for command complete */
        OMX_OSAL_EventWait(pContext->hPortEnableEvent, OMX_CONF_TIMEOUT_BUFFER_TRAFFIC, &bTimeout);
        if (OMX_TRUE == bTimeout)
        {
            OMX_CONF_SET_ERROR_BAIL("All ports not restarted\n", OMX_ErrorUndefined);
        }
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Process buffers after restarting\n");
        OMX_CONF_BAIL_IF_ERROR(PortCommTest_TransmitTest(pContext, OMX_FALSE, OMX_TRUE));

OMX_CONF_TEST_BAIL:
        return eError;
    }

    /*****************************************************************************/
    OMX_ERRORTYPE PortCommTest_PauseResume(PortCommTestCtxt* pContext)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_BOOL bTimeout;

        OMX_CONF_BAIL_IF_ERROR(PortCommTest_TransmitTest(pContext, OMX_TRUE, OMX_FALSE));
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Pause the component\n");
        OMX_CONF_BAIL_IF_ERROR(PortCommTest_TransitionWait(OMX_StatePause, pContext));
        OMX_OSAL_EventReset(pContext->hBufDoneEvent);
        OMX_OSAL_EventWait(pContext->hBufDoneEvent, OMX_CONF_TIMEOUT_EXPECTING_FAILURE, &bTimeout);
        if (OMX_FALSE == bTimeout)
        {
            OMX_CONF_SET_ERROR_BAIL("bufferdone callbacks made after pause cmdcomplete\n", OMX_ErrorUndefined);
        }
        pContext->nBufDoneCalls = 0;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Attempting to process buffers while paused\n");
        OMX_OSAL_EventReset(pContext->hBufDoneEvent);

        if (pContext->nOutBuf)
            OMX_CONF_BAIL_IF_ERROR(PortCommTest_ReadOutBuffers(pContext));
        if (pContext->nInBuf)
            OMX_CONF_BAIL_IF_ERROR(PortCommTest_WriteInBuffers(pContext));

        OMX_OSAL_EventWait(pContext->hBufDoneEvent, OMX_CONF_TIMEOUT_EXPECTING_FAILURE, &bTimeout);
        if (OMX_FALSE == bTimeout)
        {
            OMX_CONF_SET_ERROR_BAIL("bufferdone callbacks made during paused state\n", OMX_ErrorUndefined);
        }
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Processed %d buffers\n", pContext->nBufDoneCalls);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Return to executing\n");
        OMX_CONF_BAIL_IF_ERROR(PortCommTest_TransitionWait(OMX_StateExecuting, pContext));
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Process buffers after resuming\n");
        OMX_CONF_BAIL_IF_ERROR(PortCommTest_TransmitTest(pContext, OMX_FALSE, OMX_TRUE));

OMX_CONF_TEST_BAIL:
        return eError;
    }

    /*****************************************************************************/
    OMX_ERRORTYPE OMX_CONF_PortCommunicationTest(OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_HANDLETYPE hComp  = 0;
        OMX_CALLBACKTYPE sCallbacks;
        PortCommTestCtxt sContext;
        OMX_HANDLETYPE hWrappedComp = 0;
        OMX_CALLBACKTYPE *pWrappedCallbacks;
        OMX_PTR pWrappedAppData;
        PortCommTestCtxt *pCtxt;
        pCtxt = &sContext;
        memset(pCtxt, 0x0, sizeof(PortCommTestCtxt));

        sCallbacks.EventHandler    =  PortCommTest_EventHandler;
        sCallbacks.EmptyBufferDone =  PortCommTest_EmptyBufferDone;
        sCallbacks.FillBufferDone  =  PortCommTest_FillBufferDone;

        eError = OMX_CONF_CallbackTracerCreate(&sCallbacks, (OMX_PTR)pCtxt, cComponentName,
                                               &pWrappedCallbacks, &pWrappedAppData);

        /* initialize events to track callbacks */
        OMX_OSAL_EventCreate(&pCtxt->hStateSetEvent);
        OMX_OSAL_EventReset(pCtxt->hStateSetEvent);
        OMX_OSAL_EventCreate(&pCtxt->hPortDisableEvent);
        OMX_OSAL_EventReset(pCtxt->hPortDisableEvent);
        OMX_OSAL_EventCreate(&pCtxt->hPortEnableEvent);
        OMX_OSAL_EventReset(pCtxt->hPortEnableEvent);
        OMX_OSAL_EventCreate(&pCtxt->hBufDoneEvent);
        OMX_OSAL_EventReset(pCtxt->hBufDoneEvent);
        OMX_OSAL_EventCreate(&pCtxt->hEmptyBufDoneEvent);
        OMX_OSAL_EventReset(pCtxt->hEmptyBufDoneEvent);

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

        pCtxt->nPorts = pCtxt->sPortParam[0].nPorts + pCtxt->sPortParam[1].nPorts +
                        pCtxt->sPortParam[2].nPorts + pCtxt->sPortParam[3].nPorts;

        /* 3.4.1. Non-Tunneling Setup Test */
        /* TODO: reference core does not take null as port */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Non-Tunneling Setup Test\n");
        OMX_CONF_BAIL_IF_ERROR(PortCommTest_OperateOnPorts(pCtxt, NonTunnelTest));

        /* 3.4.2. Transmit Buffers Between Input and Output Ports and Application */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Transmit Test - Client allocates - Full payload\n");
        pCtxt->nBufFactor = 1;
        pCtxt->bClientAllocBuf = OMX_TRUE;
        OMX_CONF_BAIL_IF_ERROR(PortCommTest_TransmitTest(pCtxt, OMX_TRUE, OMX_TRUE));

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Transmit Test - Client allocates - Half payload\n");
        pCtxt->nBufFactor = 2;
        OMX_CONF_BAIL_IF_ERROR(PortCommTest_TransmitTest(pCtxt, OMX_TRUE, OMX_TRUE));

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Transmit Test - Client allocates - Quarter payload\n");
        pCtxt->nBufFactor = 4;
        OMX_CONF_BAIL_IF_ERROR(PortCommTest_TransmitTest(pCtxt, OMX_TRUE, OMX_TRUE));

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Transmit Test - Component allocates - Full payload\n");
        pCtxt->bClientAllocBuf = OMX_FALSE;
        pCtxt->nBufFactor = 1;
        OMX_CONF_BAIL_IF_ERROR(PortCommTest_TransmitTest(pCtxt, OMX_TRUE, OMX_TRUE));

        /* 3.4.3. Port Repack Test */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Repack Test\n");
        pCtxt->bClientAllocBuf = OMX_TRUE;
        OMX_CONF_BAIL_IF_ERROR(PortCommTest_RepackTest(pCtxt));

        /* 3.4.4. Port Stop and Restart Test */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port Stop and Restart Test\n");
        pCtxt->nBufFactor = 1;
        OMX_CONF_BAIL_IF_ERROR(PortCommTest_StopRestartTest(pCtxt));

        /* 3.4.5. Component Pause and Resume Test*/
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Component Pause and Resume Test\n");
        OMX_CONF_BAIL_IF_ERROR(PortCommTest_PauseResume(pCtxt));

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
            PortCommTest_TransitionWait(OMX_StateInvalid, pCtxt);
            PortCommTest_DeInitBuffer(pCtxt);

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
        OMX_OSAL_EventDestroy(pCtxt->hPortDisableEvent);
        OMX_OSAL_EventDestroy(pCtxt->hPortEnableEvent);
        OMX_OSAL_EventDestroy(pCtxt->hBufDoneEvent);
        OMX_OSAL_EventDestroy(pCtxt->hEmptyBufDoneEvent);

        return eError;
    }

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */
