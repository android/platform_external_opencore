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

/** OMX_CONF_StateTransitionTest.c
 *  OpenMax IL conformance test - State Transition Test
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
    OMX_ERRORTYPE OMX_CONF_ErrorToString(
        OMX_IN OMX_ERRORTYPE eError,
        OMX_OUT OMX_STRING sName);


    /*
     *  D E C L A R A T I O N S
     */
#define TEST_NAME_STRING "StateTransitionTest"
#define TEST_COMPONENT_NAME_SIZE 128
    static char szDesc[256];
#define NUM_DOMAINS 0x4
#define OMX_NOPORT 0xFFFFFFFE

    /*
     *     D E F I N I T I O N S
     */
    typedef struct _BufferList BufferList;

    struct _BufferList
    {
        OMX_BUFFERHEADERTYPE *pBufHdr;
        OMX_BOOL bInUse;
        BufferList *pNextBuf;
    };

    typedef struct
    {
        OMX_HANDLETYPE hWComp;
        OMX_STATETYPE eState;
        OMX_HANDLETYPE hStateSetEvent;
        OMX_HANDLETYPE hErrorEvent;
        BufferList *pBufferList;
        OMX_ERRORTYPE eLastError;
        OMX_PORT_PARAM_TYPE sPortParam[NUM_DOMAINS];
    } StateTransitionTestContext;


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

#define OMX_CONF_CHECK_STATETYPE(_eS_, _eC_)		\
    if(_eS_ != _eC_){					\
	OMX_CONF_ErrorToString(_eC_, szDesc);		\
        OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "invalid state transition: expected %s\n", szDesc);\
	OMX_CONF_ErrorToString(_eS_, szDesc);		\
        OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "actual %s\n", szDesc);\
        eError = OMX_ErrorUndefined;			\
    }

#define OMX_CONF_CHECK_ERRORTYPE(_eE_, _eC_)		\
    if(_eE_ != _eC_){					\
	OMX_CONF_ErrorToString(_eC_, szDesc);		\
        OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "incorrect error code: expected %s\n", szDesc);\
	OMX_CONF_ErrorToString(_eE_, szDesc);		\
        OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "actual %s\n", szDesc);\
        eError = OMX_ErrorUndefined;			\
    }

#define OMX_CONF_BAIL_IF_ERROR(_eError)		\
    if(OMX_ErrorNone != (eError = _eError)){	\
        goto OMX_CONF_TEST_BAIL;		\
    }

#define OMX_CONF_SET_ERROR_BAIL(_eCode, _desc)	\
{						\
    eError = _eCode;				\
    OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, _desc);\
    goto OMX_CONF_TEST_BAIL;			\
}


    /*
     *     F U N C T I O N S
     */
    OMX_ERRORTYPE StateTransitionTest_EventHandler(OMX_IN OMX_HANDLETYPE hComponent,
            OMX_IN OMX_PTR pAppData,
            OMX_IN OMX_EVENTTYPE eEvent,
            OMX_IN OMX_U32 nData1,
            OMX_IN OMX_U32 nData2,
            OMX_IN OMX_PTR pEventData)
    {
        StateTransitionTestContext *pContext;

        UNUSED_PARAMETER(hComponent);
        UNUSED_PARAMETER(pEventData);

        if (pAppData == NULL)
            return OMX_ErrorNone;
        pContext = (StateTransitionTestContext *)pAppData;

        if (eEvent == OMX_EventCmdComplete)
        {
            switch ((OMX_COMMANDTYPE)(nData1))
            {
                case OMX_CommandStateSet:
                    pContext->eState = (OMX_STATETYPE)(nData2);
                    OMX_OSAL_EventSet(pContext->hStateSetEvent);
                    break;
                default:
                    break;
            }
        }
        else if (eEvent == OMX_EventError)
        {
            pContext->eLastError = (OMX_ERRORTYPE)(nData1);
            OMX_OSAL_EventSet(pContext->hErrorEvent);
        }
        return OMX_ErrorNone;
    }

#define OMX_CONF_UNLOAD(_pC, _hC, _hWC, _eE){					\
    _eE = OMX_CONF_DeInitBuffer(_pC);						\
    OMX_CONF_BAIL_IF_ERROR(_eE);						\
    if(_hC){									\
	_eE = OMX_FreeHandle(_hC);						\
	OMX_CONF_BAIL_IF_ERROR(_eE);						\
    }										\
    if(_hWC){									\
	_eE = OMX_CONF_ComponentTracerDestroy(_hWC);				\
	OMX_CONF_BAIL_IF_ERROR(_eE);						\
    }										\
    _hWC = 0;									\
    _hC = 0;									\
}

#define OMX_CONF_LOAD(_eS, _eE, _pC, _hC, _hWC, _cC, _pWD, _pWC)\
{								\
    _eE = OMX_GetHandle(&_hC, _cC, _pWD, _pWC);			\
    OMX_CONF_BAIL_IF_ERROR(_eE);				\
    _eE = OMX_CONF_ComponentTracerCreate(_hC, _cC, &_hWC);	\
    OMX_CONF_BAIL_IF_ERROR(_eE);				\
    _pC->hWComp = _hWC;						\
    /* Verify start in Loaded state */				\
    _eE = OMX_GetState(_hWC, &_eS);				\
    OMX_CONF_BAIL_IF_ERROR(_eE);				\
    if(_eS != OMX_StateLoaded)					\
        OMX_CONF_SET_ERROR_BAIL(OMX_ErrorUndefined, "not in loaded state\n");\
}

    static OMX_ERRORTYPE OMX_CONF_DeInitBuffer(StateTransitionTestContext *pContext)
    {
        OMX_U8 *pBuffer;
        BufferList *pBufferListObject;
        BufferList *pTemp;
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        pTemp = pContext->pBufferList;
        while (pTemp)
        {
            pBufferListObject = (BufferList *)pTemp;
            pBuffer = (OMX_U8 *)pTemp->pBufHdr->pBuffer;
            if (pBuffer)
                OMX_OSAL_Free(pBuffer);
            pTemp->pBufHdr->pBuffer = NULL;
            if (pTemp->pBufHdr->nInputPortIndex != OMX_NOPORT)
                eError = OMX_FreeBuffer(pContext->hWComp, pTemp->pBufHdr->nInputPortIndex, pTemp->pBufHdr);
            else
                eError = OMX_FreeBuffer(pContext->hWComp, pTemp->pBufHdr->nOutputPortIndex, pTemp->pBufHdr);
            if (eError == OMX_ErrorInvalidState || eError == OMX_ErrorIncorrectStateOperation)
                eError = OMX_ErrorNone;
            OMX_CONF_BAIL_IF_ERROR(eError);

            pTemp = pTemp->pNextBuf;
            if (pBufferListObject)
                OMX_OSAL_Free(pBufferListObject);
        }
        pContext->pBufferList = NULL;

OMX_CONF_TEST_BAIL:
        return eError;
    }

    OMX_ERRORTYPE OMX_CONF_AllocateAllBuffers(StateTransitionTestContext *pContext)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_PARAM_PORTDEFINITIONTYPE sPortDef;
        OMX_U32 i, j, k;
        OMX_BUFFERHEADERTYPE *pBufferHdr = NULL;
        OMX_U8 *pBuffer = NULL;
        BufferList *pBufferListObj;
        BufferList *pTemp;

        /* for all ports */
        for (j = 0; j < NUM_DOMAINS; j++)
        {
            for (i = pContext->sPortParam[j].nStartPortNumber;
                    i < pContext->sPortParam[j].nStartPortNumber +
                    pContext->sPortParam[j].nPorts; i++)
            {

                OMX_CONF_INIT_STRUCT(sPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
                sPortDef.nPortIndex = i;
                OMX_CONF_BAIL_IF_ERROR(OMX_GetParameter(pContext->hWComp,
                                                        OMX_IndexParamPortDefinition,
                                                        (OMX_PTR)&sPortDef));
                if (sPortDef.nBufferCountActual == 0x0)
                    OMX_CONF_SET_ERROR_BAIL(OMX_ErrorUndefined, "incorrect buffer count\n");

                for (k = 0x0; k < sPortDef.nBufferCountActual; k++)
                {
                    pBufferListObj = (BufferList *)OMX_OSAL_Malloc(sizeof(BufferList));
                    if (!pBufferListObj)
                    {
                        OMX_CONF_SET_ERROR_BAIL(OMX_ErrorInsufficientResources, "malloc failed\n");
                    }
                    pBuffer = (OMX_U8*)OMX_OSAL_Malloc(sPortDef.nBufferSize);
                    if (!pBuffer)
                    {
                        OMX_CONF_SET_ERROR_BAIL(OMX_ErrorInsufficientResources, "malloc failed\n");
                    }
                    OMX_CONF_BAIL_IF_ERROR(OMX_UseBuffer(pContext->hWComp, &pBufferHdr, sPortDef.nPortIndex,
                                                         0, sPortDef.nBufferSize, pBuffer));
                    pBufferListObj->pNextBuf = NULL;
                    pBufferListObj->pBufHdr = pBufferHdr;
                    pBufferListObj->bInUse = OMX_FALSE;

                    if (sPortDef.eDir == OMX_DirInput)
                        pBufferListObj->pBufHdr->nOutputPortIndex = OMX_NOPORT;
                    else
                        pBufferListObj->pBufHdr->nInputPortIndex = OMX_NOPORT;

                    if (pContext->pBufferList == NULL)
                        pContext->pBufferList = pBufferListObj;
                    else
                    {
                        pTemp = pContext->pBufferList;
                        while (pTemp->pNextBuf)
                        {
                            pTemp = pTemp->pNextBuf;
                        }
                        pTemp->pNextBuf = pBufferListObj;
                    }
                }
            }
        }
OMX_CONF_TEST_BAIL:
        return eError;
    }

    /*  This function sends the stateset command and it checks whether
        the component processed the command correctly

        @param hComp
    	handle of the component being accessed
        @param eToState
    	transition to this state
        @param eCheckState
    	component should be in this state after processing command
        @param eCheckError
    	component should return this error code (if there is an error)
        @param pContext
    	data maintained by application (recording command completions etc.)
    */
    OMX_ERRORTYPE OMX_CONF_TransitionWaitCheck(StateTransitionTestContext* pContext,
            OMX_STATETYPE eToState,
            OMX_STATETYPE eCheckState,
            OMX_ERRORTYPE eCheckError)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_BOOL bTimeout = OMX_FALSE;
        OMX_STATETYPE eState;

        pContext->eLastError = OMX_ErrorNone;
        OMX_OSAL_EventReset(pContext->hStateSetEvent);
        OMX_OSAL_EventReset(pContext->hErrorEvent);
        OMX_CONF_BAIL_IF_ERROR(OMX_GetState(pContext->hWComp, &eState));

        eError = OMX_SendCommand(pContext->hWComp, OMX_CommandStateSet, eToState, 0);
        if (eState == OMX_StateInvalid)
        {
            if (eError != OMX_ErrorInvalidState)
            {
                OMX_CONF_SET_ERROR_BAIL(OMX_ErrorUndefined, "should return OMX_ErrorInvalidState in invalid state\n");
            }
            else
            {
                eError = OMX_ErrorNone;
                goto OMX_CONF_TEST_BAIL;
            }
        }
        else
        {
            OMX_CONF_BAIL_IF_ERROR(eError);
        }
        if (eToState == OMX_StateIdle && (eState == OMX_StateLoaded))
        {
            OMX_CONF_BAIL_IF_ERROR(OMX_CONF_AllocateAllBuffers(pContext));
        }
        else if (eToState == OMX_StateLoaded && eState == OMX_StateIdle)
        {
            OMX_CONF_BAIL_IF_ERROR(OMX_CONF_DeInitBuffer(pContext));
        }
        if (eCheckError == OMX_ErrorNone)
            OMX_OSAL_EventWait(pContext->hStateSetEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
        else
            OMX_OSAL_EventWait(pContext->hErrorEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);

        if (bTimeout == OMX_TRUE)
            OMX_CONF_SET_ERROR_BAIL(OMX_ErrorUndefined, "transition timed out\n");

        OMX_CONF_BAIL_IF_ERROR(OMX_GetState(pContext->hWComp, &eState));
        OMX_CONF_CHECK_STATETYPE(eState, eCheckState);
        OMX_CONF_CHECK_ERRORTYPE(pContext->eLastError, eCheckError);

OMX_CONF_TEST_BAIL:
        return eError;
    }

    OMX_ERRORTYPE OMX_CONF_StateTransitionTest(OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_HANDLETYPE hComp  = 0;
        OMX_CALLBACKTYPE oCallbacks;
        StateTransitionTestContext oAppData;
        OMX_HANDLETYPE hWrappedComp = 0;
        OMX_CALLBACKTYPE *pWrappedCallbacks;
        OMX_PTR pWrappedAppData;
        StateTransitionTestContext *pContext;
        OMX_STATETYPE eState;
        pContext = &oAppData;
        memset(pContext, 0x0, sizeof(StateTransitionTestContext));

        oCallbacks.EventHandler    =  StateTransitionTest_EventHandler;
        oCallbacks.EmptyBufferDone =  StubbedEmptyBufferDone;
        oCallbacks.FillBufferDone  =  StubbedFillBufferDone;

        eError = OMX_CONF_CallbackTracerCreate(&oCallbacks, (OMX_PTR)pContext, cComponentName,
                                               &pWrappedCallbacks, &pWrappedAppData);

        OMX_OSAL_EventCreate(&pContext->hStateSetEvent);
        OMX_OSAL_EventReset(pContext->hStateSetEvent);
        OMX_OSAL_EventCreate(&pContext->hErrorEvent);
        OMX_OSAL_EventReset(pContext->hErrorEvent);

        /* Initialize OpenMax */
        eError = OMX_Init();
        OMX_CONF_BAIL_IF_ERROR(eError);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "unloaded -> loaded\n");
        OMX_CONF_LOAD(eState, eError, pContext, hComp, hWrappedComp,
                      cComponentName, pWrappedAppData, pWrappedCallbacks);

        /* detect all audio ports on the component */
        OMX_CONF_INIT_STRUCT(pContext->sPortParam[0], OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(pContext->hWComp, OMX_IndexParamAudioInit, (OMX_PTR) & pContext->sPortParam[0]);
        if (OMX_ErrorUnsupportedIndex == eError)
            eError = OMX_ErrorNone;
        OMX_CONF_BAIL_IF_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "detected %i audio ports starting at %i \n",
                       pContext->sPortParam[0].nPorts, pContext->sPortParam[0].nStartPortNumber);

        /* detect all video ports on the component */
        OMX_CONF_INIT_STRUCT(pContext->sPortParam[1], OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(pContext->hWComp, OMX_IndexParamVideoInit, (OMX_PTR) & pContext->sPortParam[1]);
        if (OMX_ErrorUnsupportedIndex == eError)
            eError = OMX_ErrorNone;
        OMX_CONF_BAIL_IF_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "detected %i video ports starting at %i \n",
                       pContext->sPortParam[1].nPorts, pContext->sPortParam[1].nStartPortNumber);

        /* detect all image ports on the component */
        OMX_CONF_INIT_STRUCT(pContext->sPortParam[2], OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(pContext->hWComp, OMX_IndexParamImageInit, (OMX_PTR) & pContext->sPortParam[2]);
        if (OMX_ErrorUnsupportedIndex == eError)
            eError = OMX_ErrorNone;
        OMX_CONF_BAIL_IF_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "detected %i image ports starting at %i \n",
                       pContext->sPortParam[2].nPorts, pContext->sPortParam[2].nStartPortNumber);

        /* detect all other ports on the component */
        OMX_CONF_INIT_STRUCT(pContext->sPortParam[3], OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(pContext->hWComp, OMX_IndexParamOtherInit, (OMX_PTR) & pContext->sPortParam[3]);
        if (OMX_ErrorUnsupportedIndex == eError)
            eError = OMX_ErrorNone;
        OMX_CONF_BAIL_IF_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "detected %i other ports starting at %i \n",
                       pContext->sPortParam[3].nPorts, pContext->sPortParam[3].nStartPortNumber);

        /* Tests are numbered based on Section 3.10 in conformance test document */
        /* read as <row,column> */
        /* TODO: minimize number of "setting up" transitions */

        /* <1,2> unload -> loaded */

        /* <2,1> loaded -> unloaded */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "loaded -> unloaded\n");
        OMX_CONF_UNLOAD(pContext, hComp, hWrappedComp, eError);

        /* <2,2> loaded -> loaded */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "loaded -> loaded\n");
        OMX_CONF_LOAD(eState, eError, pContext, hComp, hWrappedComp,
                      cComponentName, pWrappedAppData, pWrappedCallbacks);
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateLoaded,
                                              OMX_StateLoaded, OMX_ErrorSameState);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <2,3> loaded -> waitforresources */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "loaded -> waitforresources\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateWaitForResources,
                                              OMX_StateWaitForResources, OMX_ErrorNone);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <3,2> waitforresources -> loaded */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "waitforresources -> loaded\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateLoaded,
                                              OMX_StateLoaded, OMX_ErrorNone);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <2,4> loaded -> idle */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "loaded -> idle\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateIdle,
                                              OMX_StateIdle, OMX_ErrorNone);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <4,2> idle -> loaded */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "idle -> loaded\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateLoaded,
                                              OMX_StateLoaded, OMX_ErrorNone);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <2,5> loaded -> executing */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "loaded -> executing\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateExecuting,
                                              OMX_StateLoaded, OMX_ErrorIncorrectStateTransition);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <2,6> loaded -> pause */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "loaded -> pause\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StatePause,
                                              OMX_StateLoaded, OMX_ErrorIncorrectStateTransition);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <2,7> loaded -> invalid */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "loaded -> invalid\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateInvalid,
                                              OMX_StateInvalid, OMX_ErrorInvalidState);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* set to waitforresources */
        OMX_CONF_UNLOAD(pContext, hComp, hWrappedComp, eError);
        OMX_CONF_LOAD(eState, eError, pContext, hComp, hWrappedComp,
                      cComponentName, pWrappedAppData, pWrappedCallbacks);
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateWaitForResources,
                                              OMX_StateWaitForResources, OMX_ErrorNone);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <3,2> waitforresources -> loaded; with <2,3> */

        /* <3,3> waitforresources -> waitforresources */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "waitforresources -> waitforresources\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateWaitForResources,
                                              OMX_StateWaitForResources, OMX_ErrorSameState);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <3,5> waitforresources -> executing */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "waitforresources -> executing\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateExecuting,
                                              OMX_StateWaitForResources, OMX_ErrorIncorrectStateTransition);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <3,6> waitforresources -> pause */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "waitforresources -> pause\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StatePause,
                                              OMX_StateWaitForResources, OMX_ErrorIncorrectStateTransition);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <3,4> waitforresources -> idle */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "waitforresources -> idle - not tested\n");
        /* eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateIdle,
        				  OMX_StateIdle, OMX_ErrorNone);
        OMX_CONF_BAIL_IF_ERROR(eError); */

        /* <3,7> waitforresources -> invalid */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "waitforresources -> invalid\n");
        /* idle -> loaded */
        /* eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateLoaded,
        				  OMX_StateLoaded, OMX_ErrorNone);
        				  OMX_CONF_BAIL_IF_ERROR(eError); */
        /* loaded -> waitforresources */
        /*eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateWaitForResources,
        		OMX_StateWaitForResources, OMX_ErrorNone);
        		OMX_CONF_BAIL_IF_ERROR(eError);*/
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateInvalid,
                                              OMX_StateInvalid, OMX_ErrorInvalidState);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <3,1> waitforresources (invalid) -> unloaded */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "waitforresources -> unloaded\n");
        OMX_CONF_UNLOAD(pContext, hComp, hWrappedComp, eError);

        /* set to idle */
        OMX_CONF_LOAD(eState, eError, pContext, hComp, hWrappedComp,
                      cComponentName, pWrappedAppData, pWrappedCallbacks);
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateIdle,
                                              OMX_StateIdle, OMX_ErrorNone);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <4,2> idle -> loaded; with <2,4> */

        /* <4,3> idle -> waitforresources */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "idle -> waitforresources\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateWaitForResources,
                                              OMX_StateIdle, OMX_ErrorIncorrectStateTransition);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <4,4> idle -> idle */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "idle -> idle\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateIdle,
                                              OMX_StateIdle, OMX_ErrorSameState);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <4,5> idle -> executing */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "idle -> executing\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateExecuting,
                                              OMX_StateExecuting, OMX_ErrorNone);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <5,4> executing -> idle */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "executing -> idle\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateIdle,
                                              OMX_StateIdle, OMX_ErrorNone);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <4,6> idle -> pause */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "idle -> pause\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StatePause,
                                              OMX_StatePause, OMX_ErrorNone);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <6,4> pause -> idle */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "pause -> idle\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateIdle,
                                              OMX_StateIdle, OMX_ErrorNone);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <4,7> idle -> invalid */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "idle -> invalid\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateInvalid,
                                              OMX_StateInvalid, OMX_ErrorInvalidState);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <4,1> idle (invalid) -> unloaded */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "idle -> unloaded\n");
        OMX_CONF_UNLOAD(pContext, hComp, hWrappedComp, eError);

        /* set to executing */
        OMX_CONF_LOAD(eState, eError, pContext, hComp, hWrappedComp,
                      cComponentName, pWrappedAppData, pWrappedCallbacks);
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateIdle,
                                              OMX_StateIdle, OMX_ErrorNone);
        OMX_CONF_BAIL_IF_ERROR(eError);
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateExecuting,
                                              OMX_StateExecuting, OMX_ErrorNone);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <5,2> executing -> loaded */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "executing -> loaded\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateLoaded,
                                              OMX_StateExecuting, OMX_ErrorIncorrectStateTransition);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <5,3> executing -> waitforresources */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "executing -> waitforresources\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateWaitForResources,
                                              OMX_StateExecuting, OMX_ErrorIncorrectStateTransition);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <5,5> executing -> executing */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "executing -> executing\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateExecuting,
                                              OMX_StateExecuting, OMX_ErrorSameState);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <5,4> executing -> idle; with <4,5> */

        /* <5,6> executing -> pause */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "executing -> pause\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StatePause,
                                              OMX_StatePause, OMX_ErrorNone);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <6,5> pause -> executing */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "pause -> executing\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateExecuting,
                                              OMX_StateExecuting, OMX_ErrorNone);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <5,7> executing -> invalid */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "executing -> invalid\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateInvalid,
                                              OMX_StateInvalid, OMX_ErrorInvalidState);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <5,1> executing (invalid) -> unloaded */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "executing -> unloaded\n");
        OMX_CONF_UNLOAD(pContext, hComp, hWrappedComp, eError);

        /* set to pause */
        OMX_CONF_LOAD(eState, eError, pContext, hComp, hWrappedComp,
                      cComponentName, pWrappedAppData, pWrappedCallbacks);
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateIdle,
                                              OMX_StateIdle, OMX_ErrorNone);
        OMX_CONF_BAIL_IF_ERROR(eError);
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StatePause,
                                              OMX_StatePause, OMX_ErrorNone);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <6,2> pause -> loaded */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "pause -> loaded\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateLoaded,
                                              OMX_StatePause, OMX_ErrorIncorrectStateTransition);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <6,3> pause -> waitforresources */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "pause -> waitforresources\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateWaitForResources,
                                              OMX_StatePause, OMX_ErrorIncorrectStateTransition);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <6,4> pause -> idle; with <4,6> */

        /* <6,5> pause -> executing; with <5,6> */

        /* <6,6> pause -> pause */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "pause -> pause\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StatePause,
                                              OMX_StatePause, OMX_ErrorSameState);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <6,7> pause -> invalid */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "pause -> invalid\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateInvalid,
                                              OMX_StateInvalid, OMX_ErrorInvalidState);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <6,1> pause (invalid) -> unloaded */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "pause -> unloaded\n");
        OMX_CONF_UNLOAD(pContext, hComp, hWrappedComp, eError);

        /* set to invalid */
        OMX_CONF_LOAD(eState, eError, pContext, hComp, hWrappedComp,
                      cComponentName, pWrappedAppData, pWrappedCallbacks);
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateInvalid,
                                              OMX_StateInvalid, OMX_ErrorInvalidState);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <7,2> invalid -> loaded */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "invalid -> loaded\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateLoaded,
                                              OMX_StateInvalid, OMX_ErrorInvalidState);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <7,3> invalid -> waitforresources */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "invalid -> waitforresources\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateWaitForResources,
                                              OMX_StateInvalid, OMX_ErrorInvalidState);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <7,4> invalid -> idle */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "invalid -> idle\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateIdle,
                                              OMX_StateInvalid, OMX_ErrorInvalidState);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <7,5> invalid -> executing */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "invalid -> executing\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateExecuting,
                                              OMX_StateInvalid, OMX_ErrorInvalidState);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <7,6> invalid -> pause */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "invalid -> pause\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StatePause,
                                              OMX_StateInvalid, OMX_ErrorInvalidState);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <7,7> invalid -> invalid */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "invalid -> invalid\n");
        eError = OMX_CONF_TransitionWaitCheck(pContext, OMX_StateInvalid,
                                              OMX_StateInvalid, OMX_ErrorInvalidState);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* <7,1> invalid -> unloaded */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "invalid -> unloaded\n");
        OMX_CONF_UNLOAD(pContext, hComp, hWrappedComp, eError);

OMX_CONF_TEST_BAIL:
        /* cleanup: return function errors rather than closing errors if appropriate */

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
            /* set to invalid and cleanup */
            OMX_CONF_TransitionWaitCheck(pContext, OMX_StateInvalid,
                                         OMX_StateInvalid, OMX_ErrorInvalidState);
            OMX_CONF_DeInitBuffer(pContext);

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

        OMX_OSAL_EventDestroy(pContext->hStateSetEvent);
        OMX_OSAL_EventDestroy(pContext->hErrorEvent);

        return eError;
    }

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */
