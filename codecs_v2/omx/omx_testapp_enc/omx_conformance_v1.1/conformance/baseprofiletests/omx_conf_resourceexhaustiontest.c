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

/** OMX_CONF_ResourceExhaustionTest.c
 *  OpenMax IL conformance test - Resource Exhaustion Test
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
#define TEST_NAME_STRING "ResourceExhaustionTest"
#define TEST_COMPONENT_NAME_SIZE 128
    static char szDesc[256];
#define NUM_DOMAINS 0x4
#define OMX_NOPORT 0xfffffffe
#define MAX_INSTANCE 300
#define MAX_ITERATIONS 100

    /*
     *     D E F I N I T I O N S
     */
    typedef struct _BufferList BufferList;

    struct _BufferList
    {
        OMX_BUFFERHEADERTYPE *pBufHdr;
        BufferList *pNextBuf;
    };

    typedef struct
    {
        OMX_U32 nInst;
        OMX_ERRORTYPE eLastError;
        BufferList *pBufferList[MAX_INSTANCE];
        OMX_HANDLETYPE hComp[MAX_INSTANCE];
        OMX_HANDLETYPE hWComp[MAX_INSTANCE];
        OMX_HANDLETYPE hStateSetEvent;
        OMX_PORT_PARAM_TYPE sPortParam[NUM_DOMAINS];
    } ResourceExhaustionTestContext;


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

#define OMX_CONF_BAIL_IF_ERROR(_eE)		\
    if((eError = _eE) != OMX_ErrorNone){	\
        goto OMX_CONF_TEST_BAIL;		\
    }

#define OMX_CONF_SET_ERROR_BAIL(_sDesc, _eError)\
{						\
    eError = _eError;				\
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, _sDesc);\
    goto OMX_CONF_TEST_BAIL;			\
}


    /*
     *     F U N C T I O N S
     */

    /*****************************************************************************/
    OMX_ERRORTYPE ResourceExhaustionTest_EventHandler(OMX_IN OMX_HANDLETYPE hComponent,
            OMX_IN OMX_PTR pAppData,
            OMX_IN OMX_EVENTTYPE eEvent,
            OMX_IN OMX_U32 nData1,
            OMX_IN OMX_U32 nData2,
            OMX_IN OMX_PTR pEventData)
    {
        ResourceExhaustionTestContext *pCtxt;

        UNUSED_PARAMETER(hComponent);
        UNUSED_PARAMETER(pEventData);

        if (pAppData == NULL)
            return OMX_ErrorNone;
        pCtxt = (ResourceExhaustionTestContext *)pAppData;

        if (eEvent == OMX_EventCmdComplete)
        {
            switch ((OMX_COMMANDTYPE)(nData1))
            {
                case OMX_CommandStateSet:
                    OMX_CONF_StateToString((OMX_STATETYPE)(nData2), szDesc);
                    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Instance %d transitioned to %s\n",
                                   pCtxt->nInst, szDesc);
                    OMX_OSAL_EventSet(pCtxt->hStateSetEvent);
                    break;
                default:
                    break;
            }
        }
        else if (eEvent == OMX_EventError)
        {
            pCtxt->eLastError = (OMX_ERRORTYPE)(nData1);
            OMX_CONF_ErrorToString(pCtxt->eLastError, szDesc);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Instance %d sent error %0#x %s\n",
                           pCtxt->nInst, nData1, szDesc);
            OMX_OSAL_EventSet(pCtxt->hStateSetEvent);
        }
        return OMX_ErrorNone;
    }

    /*****************************************************************************/
    OMX_ERRORTYPE ResourceExhaustionTest_DeInitBuffer(ResourceExhaustionTestContext *pCtxt)
    {
        BufferList *pBufferListObject;
        BufferList *pTemp;
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        pTemp = pCtxt->pBufferList[pCtxt->nInst];
        while (pTemp)
        {
            pBufferListObject = (BufferList *)pTemp;
            if (pTemp->pBufHdr->nInputPortIndex != OMX_NOPORT)
                eError = OMX_FreeBuffer(pCtxt->hWComp[pCtxt->nInst],
                                        pTemp->pBufHdr->nInputPortIndex, pTemp->pBufHdr);
            else
                eError = OMX_FreeBuffer(pCtxt->hWComp[pCtxt->nInst],
                                        pTemp->pBufHdr->nOutputPortIndex, pTemp->pBufHdr);

            if (eError == OMX_ErrorInvalidState || eError == OMX_ErrorIncorrectStateOperation)
                eError = OMX_ErrorNone;
            OMX_CONF_BAIL_IF_ERROR(eError);

            pTemp = pTemp->pNextBuf;
            if (pBufferListObject)
                OMX_OSAL_Free(pBufferListObject);
        }
        pCtxt->pBufferList[pCtxt->nInst] = NULL;

OMX_CONF_TEST_BAIL:
        return eError;
    }

    /*****************************************************************************/
    OMX_ERRORTYPE ResourceExhaustionTest_LOAD(ResourceExhaustionTestContext *pCtxt,
            OMX_STRING cComponentName,
            OMX_PTR pWAppData,
            OMX_PTR pWCallbacks)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_STATETYPE eState;
        eError = OMX_GetHandle(&pCtxt->hComp[pCtxt->nInst], cComponentName, pWAppData, pWCallbacks);
        OMX_CONF_BAIL_IF_ERROR(eError);
        eError = OMX_CONF_ComponentTracerCreate(pCtxt->hComp[pCtxt->nInst], cComponentName,
                                                &pCtxt->hWComp[pCtxt->nInst]);
        OMX_CONF_BAIL_IF_ERROR(eError);
        eError = OMX_GetState(pCtxt->hWComp[pCtxt->nInst], &eState);
        OMX_CONF_BAIL_IF_ERROR(eError);
        if (eState != OMX_StateLoaded)
            OMX_CONF_SET_ERROR_BAIL("Not in loaded state\n", OMX_ErrorUndefined);
OMX_CONF_TEST_BAIL:
        return eError;
    }

    /*****************************************************************************/
    OMX_ERRORTYPE ResourceExhaustionTest_UNLOAD(ResourceExhaustionTestContext *pCtxt)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        if (pCtxt->hComp[pCtxt->nInst])
        {
            eError = OMX_FreeHandle(pCtxt->hComp[pCtxt->nInst]);
            OMX_CONF_BAIL_IF_ERROR(eError);
        }
        if (pCtxt->hWComp[pCtxt->nInst])
        {
            eError = OMX_CONF_ComponentTracerDestroy(pCtxt->hWComp[pCtxt->nInst]);
            OMX_CONF_BAIL_IF_ERROR(eError);
        }
        pCtxt->hComp[pCtxt->nInst] = NULL;
        pCtxt->hWComp[pCtxt->nInst] = NULL;

OMX_CONF_TEST_BAIL:
        return eError;
    }

    /*****************************************************************************/
    OMX_ERRORTYPE ResourceExhaustionTest_AllocateAllBuffers(ResourceExhaustionTestContext *pCtxt)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_PARAM_PORTDEFINITIONTYPE sPortDef;
        OMX_U32 i, j, k;
        OMX_BUFFERHEADERTYPE *pBufferHdr = NULL;
        BufferList *pBufferListObj;
        BufferList *pTemp;

        /* for all ports */
        for (j = 0; j < NUM_DOMAINS; j++)
        {
            for (i = pCtxt->sPortParam[j].nStartPortNumber;
                    i < pCtxt->sPortParam[j].nStartPortNumber +
                    pCtxt->sPortParam[j].nPorts; i++)
            {

                OMX_CONF_INIT_STRUCT(sPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
                sPortDef.nPortIndex = i;
                OMX_CONF_BAIL_IF_ERROR(OMX_GetParameter(pCtxt->hWComp[pCtxt->nInst],
                                                        OMX_IndexParamPortDefinition,
                                                        (OMX_PTR)&sPortDef));
                if (sPortDef.nBufferCountActual == 0x0 ||
                        sPortDef.nBufferCountActual < sPortDef.nBufferCountMin)
                    OMX_CONF_SET_ERROR_BAIL("Incorrect nBufferCountActual\n", OMX_ErrorUndefined);

                for (k = 0x0; k < sPortDef.nBufferCountActual; k++)
                {
                    pBufferListObj = (BufferList *)OMX_OSAL_Malloc(sizeof(BufferList));
                    if (!pBufferListObj)
                    {
                        OMX_CONF_SET_ERROR_BAIL("Malloc internal list failed\n", OMX_ErrorInsufficientResources);
                    }
                    OMX_CONF_BAIL_IF_ERROR(OMX_AllocateBuffer(pCtxt->hWComp[pCtxt->nInst], &pBufferHdr,
                                           sPortDef.nPortIndex, 0, sPortDef.nBufferSize));
                    pBufferListObj->pNextBuf = NULL;
                    pBufferListObj->pBufHdr = pBufferHdr;

                    if (sPortDef.eDir == OMX_DirInput)
                        pBufferListObj->pBufHdr->nOutputPortIndex = OMX_NOPORT;
                    else
                        pBufferListObj->pBufHdr->nInputPortIndex = OMX_NOPORT;

                    if (pCtxt->pBufferList[pCtxt->nInst] == NULL)
                        pCtxt->pBufferList[pCtxt->nInst] = pBufferListObj;
                    else
                    {
                        pTemp = pCtxt->pBufferList[pCtxt->nInst];
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

    /*****************************************************************************/
    OMX_ERRORTYPE ResourceExhaustionTest_TransitionWait(ResourceExhaustionTestContext* pCtxt,
            OMX_STATETYPE eToState)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_BOOL bTimeout = OMX_FALSE;
        OMX_STATETYPE eState;

        OMX_OSAL_EventReset(pCtxt->hStateSetEvent);
        pCtxt->eLastError = OMX_ErrorNone;
        OMX_CONF_BAIL_IF_ERROR(OMX_GetState(pCtxt->hWComp[pCtxt->nInst], &eState));

        eError = OMX_SendCommand(pCtxt->hWComp[pCtxt->nInst], OMX_CommandStateSet, eToState, 0);
        OMX_CONF_BAIL_IF_ERROR(eError);

        if (eToState == OMX_StateIdle && (eState == OMX_StateLoaded))
        {
            OMX_CONF_BAIL_IF_ERROR(ResourceExhaustionTest_AllocateAllBuffers(pCtxt));
        }
        else if (eToState == OMX_StateLoaded && eState == OMX_StateIdle)
        {
            OMX_CONF_BAIL_IF_ERROR(ResourceExhaustionTest_DeInitBuffer(pCtxt));
        }

        OMX_OSAL_EventWait(pCtxt->hStateSetEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
        OMX_CONF_BAIL_IF_ERROR(pCtxt->eLastError);

        if (bTimeout == OMX_TRUE)
            OMX_CONF_SET_ERROR_BAIL("Timeout on state transition\n", OMX_ErrorUndefined);

        OMX_CONF_BAIL_IF_ERROR(OMX_GetState(pCtxt->hWComp[pCtxt->nInst], &eState));
        if (eState != eToState)
            OMX_CONF_SET_ERROR_BAIL("Incorrect transition\n", OMX_ErrorUndefined);

OMX_CONF_TEST_BAIL:
        if (OMX_ErrorNone != eError && eToState == OMX_StateIdle && (eState == OMX_StateLoaded))
        {

            /* Add some delay before cleaning up */
            OMX_OSAL_EventWait(pCtxt->hStateSetEvent, OMX_CONF_TIMEOUT_EXPECTING_FAILURE, &bTimeout);

            /* Disable all ports to free buffers*/
            OMX_SendCommand(pCtxt->hWComp[pCtxt->nInst], OMX_CommandPortDisable, OMX_ALL, 0x0);

            /* Cleanup last instance */
            ResourceExhaustionTest_DeInitBuffer(pCtxt);
        }
        return eError;
    }

    /*****************************************************************************/
    OMX_ERRORTYPE OMX_CONF_ResourceExhaustionTest(OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_CALLBACKTYPE oCallbacks;
        ResourceExhaustionTestContext oAppData;
        OMX_CALLBACKTYPE *pWrapCallbacks;
        OMX_PTR pWrapAppData;
        ResourceExhaustionTestContext *pCtxt;
        OMX_U32 i;

        pCtxt = &oAppData;
        memset(pCtxt, 0x0, sizeof(ResourceExhaustionTestContext));

        oCallbacks.EventHandler    =  ResourceExhaustionTest_EventHandler;
        oCallbacks.EmptyBufferDone =  StubbedEmptyBufferDone;
        oCallbacks.FillBufferDone  =  StubbedFillBufferDone;

        eError = OMX_CONF_CallbackTracerCreate(&oCallbacks, (OMX_PTR)pCtxt, cComponentName,
                                               &pWrapCallbacks, &pWrapAppData);

        OMX_OSAL_EventCreate(&pCtxt->hStateSetEvent);
        OMX_OSAL_EventReset(pCtxt->hStateSetEvent);

        /* Initialize OpenMax */
        eError = OMX_Init();
        OMX_CONF_BAIL_IF_ERROR(eError);

        eError = ResourceExhaustionTest_LOAD(pCtxt, cComponentName, pWrapAppData, pWrapCallbacks);
        OMX_CONF_BAIL_IF_ERROR(eError);

        /* detect all audio ports on the component */
        OMX_CONF_INIT_STRUCT(pCtxt->sPortParam[0], OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(pCtxt->hWComp[pCtxt->nInst], OMX_IndexParamAudioInit,
                                  (OMX_PTR) & pCtxt->sPortParam[0]);
        if (OMX_ErrorUnsupportedIndex == eError)
            eError = OMX_ErrorNone;
        OMX_CONF_BAIL_IF_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "detected %i audio ports starting at %i \n",
                       pCtxt->sPortParam[0].nPorts, pCtxt->sPortParam[0].nStartPortNumber);

        /* detect all video ports on the component */
        OMX_CONF_INIT_STRUCT(pCtxt->sPortParam[1], OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(pCtxt->hWComp[pCtxt->nInst], OMX_IndexParamVideoInit,
                                  (OMX_PTR) & pCtxt->sPortParam[1]);
        if (OMX_ErrorUnsupportedIndex == eError)
            eError = OMX_ErrorNone;
        OMX_CONF_BAIL_IF_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "detected %i video ports starting at %i \n",
                       pCtxt->sPortParam[1].nPorts, pCtxt->sPortParam[1].nStartPortNumber);

        /* detect all image ports on the component */
        OMX_CONF_INIT_STRUCT(pCtxt->sPortParam[2], OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(pCtxt->hWComp[pCtxt->nInst], OMX_IndexParamImageInit,
                                  (OMX_PTR) & pCtxt->sPortParam[2]);
        if (OMX_ErrorUnsupportedIndex == eError)
            eError = OMX_ErrorNone;
        OMX_CONF_BAIL_IF_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "detected %i image ports starting at %i \n",
                       pCtxt->sPortParam[2].nPorts, pCtxt->sPortParam[2].nStartPortNumber);

        /* detect all other ports on the component */
        OMX_CONF_INIT_STRUCT(pCtxt->sPortParam[3], OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(pCtxt->hWComp[pCtxt->nInst], OMX_IndexParamOtherInit,
                                  (OMX_PTR) & pCtxt->sPortParam[3]);
        if (OMX_ErrorUnsupportedIndex == eError)
            eError = OMX_ErrorNone;
        OMX_CONF_BAIL_IF_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "detected %i other ports starting at %i \n",
                       pCtxt->sPortParam[3].nPorts, pCtxt->sPortParam[3].nStartPortNumber);

        OMX_CONF_BAIL_IF_ERROR(ResourceExhaustionTest_TransitionWait(pCtxt, OMX_StateIdle));

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Repeatedly load and idle component\n");
        /* JVW-NVIDIA: set bound to MAX_INSTANCE-1 http://cvs.khronos.org/bugzilla/show_bug.cgi?id=219 */
        while (pCtxt->nInst++ < MAX_INSTANCE - 1)
        {
            eError = ResourceExhaustionTest_LOAD(pCtxt, cComponentName, pWrapAppData, pWrapCallbacks);
            if (eError != OMX_ErrorNone)
                break;

            eError = ResourceExhaustionTest_TransitionWait(pCtxt, OMX_StateIdle);
            if (eError != OMX_ErrorNone)
                break;
        }
        if (eError == OMX_ErrorInsufficientResources)
            eError = OMX_ErrorNone;

        if (pCtxt->nInst == MAX_INSTANCE)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Resources not exhausted - increase max instance\n");
            eError = OMX_ErrorUndefined;
        }
        OMX_CONF_BAIL_IF_ERROR(eError);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Resources exhausted by instance %d\n", pCtxt->nInst);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Alternatively de-allocate/allocate last 2 instances\n");

        for (i = 0x0; i < MAX_ITERATIONS; i++)
        {
            /* Unload last successfully idled instance */
            pCtxt->nInst--;
            OMX_CONF_BAIL_IF_ERROR(ResourceExhaustionTest_TransitionWait(pCtxt, OMX_StateLoaded));
            OMX_CONF_BAIL_IF_ERROR(ResourceExhaustionTest_UNLOAD(pCtxt));

            /* Load last failing instance */
            pCtxt->nInst++;
            OMX_CONF_BAIL_IF_ERROR(ResourceExhaustionTest_UNLOAD(pCtxt));
            OMX_CONF_BAIL_IF_ERROR(ResourceExhaustionTest_LOAD(pCtxt, cComponentName, pWrapAppData, pWrapCallbacks));
            OMX_CONF_BAIL_IF_ERROR(ResourceExhaustionTest_TransitionWait(pCtxt, OMX_StateIdle));

            /* Unload last successfully idled instance */
            OMX_CONF_BAIL_IF_ERROR(ResourceExhaustionTest_TransitionWait(pCtxt, OMX_StateLoaded));
            OMX_CONF_BAIL_IF_ERROR(ResourceExhaustionTest_UNLOAD(pCtxt));

            /* Load last failing instance */
            pCtxt->nInst--;
            OMX_CONF_BAIL_IF_ERROR(ResourceExhaustionTest_LOAD(pCtxt, cComponentName, pWrapAppData, pWrapCallbacks));
            OMX_CONF_BAIL_IF_ERROR(ResourceExhaustionTest_TransitionWait(pCtxt, OMX_StateIdle));

            pCtxt->nInst++;
        }

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "De-allocate all instances\n");
        while (pCtxt->nInst--)
        {
            OMX_CONF_BAIL_IF_ERROR(ResourceExhaustionTest_TransitionWait(pCtxt, OMX_StateLoaded));
            OMX_CONF_BAIL_IF_ERROR(ResourceExhaustionTest_UNLOAD(pCtxt));
        }

OMX_CONF_TEST_BAIL:
        /* cleanup: return function errors rather than closing errors if appropriate */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Cleanup\n");
        if (OMX_ErrorNone == eError)
        {
            eError = OMX_CONF_CallbackTracerDestroy(pWrapCallbacks, pWrapAppData);
            eError = OMX_Deinit();
        }
        else
        {
            do
            {
                if (pCtxt->hWComp[pCtxt->nInst])
                {
                    ResourceExhaustionTest_TransitionWait(pCtxt, OMX_StateInvalid);
                    ResourceExhaustionTest_DeInitBuffer(pCtxt);
                    ResourceExhaustionTest_UNLOAD(pCtxt);
                }
            }
            while (pCtxt->nInst--);

            OMX_CONF_CallbackTracerDestroy(pWrapCallbacks, pWrapAppData);
            OMX_Deinit();
        }

        OMX_OSAL_EventDestroy(pCtxt->hStateSetEvent);

        return eError;
    }

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */
