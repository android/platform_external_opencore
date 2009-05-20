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
#include "omxenctestbase.h"


OmxEncTestBase::OmxEncTestBase(const char AOName[]) :
        OsclActiveObject(OsclActiveObject::EPriorityNominal, AOName)
        , iCallbacks(NULL)
        , iHeaderSent(OMX_FALSE)
{
    OMX_S32 jj;

    iCallbacks = new CallbackContainer(this);

    iState = StateUnLoaded;
    iPendingCommands = 0;
    ipAppPriv = NULL;

    //Set the first frame to zero.
    iFrameNum = 0;
    iFramesWritten = 0;

    //Assumed maximum number of frames in an input bitstream
    iLastInputFrame = 50000;

    iStopProcessingInput = OMX_FALSE;

    iInputWasRefused = OMX_FALSE;

    iStopOutput = OMX_FALSE;
    iStopInput = OMX_FALSE;
    //This value is 1 for no fragmentation & any number "n" for n partial frames.
    iNumSimFrags = 1;
    iFragmentInProgress = OMX_FALSE;
    iDivideBuffer = OMX_FALSE;
    //Set the value of Current fragment number to 0
    iCurFrag = 0;

    iInputReady = OMX_TRUE;

    /* Initialize all the buffers to NULL */
#ifdef USEBUFFER_FLAG
    ipUseInBuff = NULL;
    ipUseOutBuff = NULL;
#endif
    ipInBuffer = NULL;
    ipOutBuffer = NULL;
    ipInputAvail = NULL;
    ipOutReleased = NULL;

    iCount = 0;
    iCount1 = 0;
    iCount2 = 0;
    iCount3 = 0;

    for (jj = 0; jj < 50; jj++)
    {
        iSimFragSize[jj] = 0;
    }

#if PROXY_INTERFACE
    ipThreadSafeHandlerEventHandler = NULL;
    ipThreadSafeHandlerEmptyBufferDone = NULL;
    ipThreadSafeHandlerFillBufferDone = NULL;
#endif

    //Get the logger object here in the base class
    iLogger = PVLogger::GetLoggerObject("OmxEncComponentTestApplication");

}


void OmxEncTestBase::InitScheduler()
{
    OsclScheduler::Init("OMX.PV.enc");

}


void OmxEncTestBase::StartTestApp()
{
    if (!IsAdded())
    {
        AddToScheduler();
    }

    RunIfNotReady();

    OsclExecScheduler* sched = OsclExecScheduler::Current();
    if (sched)
    {
        sched->StartScheduler();
    }
}


#if PROXY_INTERFACE
OsclReturnCode OmxEncTestBase::ProcessCallbackEventHandler(OsclAny* P)
{
    // re-cast the pointer
    EventHandlerSpecificData* ED = (EventHandlerSpecificData*) P;

    OMX_HANDLETYPE aComponent = ED->hComponent;
    OMX_PTR aAppData = ED->pAppData;
    OMX_EVENTTYPE aEvent = ED->eEvent;
    OMX_U32 aData1 = ED->nData1;
    OMX_U32 aData2 = ED->nData2;
    OMX_PTR aEventData = ED->pEventData;

    EventHandler(aComponent, aAppData, aEvent, aData1, aData2, aEventData);

    // release the allocated memory when no longer needed
    //ED->hComponent = NULL;
    //ED->nData1 = 0;
    //ED->nData2 = 0;
    //ED->pAppData = NULL;
    //ED->pEventData = NULL;

    // release the allocated memory when no longer needed
    ipThreadSafeHandlerEventHandler->iMemoryPool->deallocate(ED);
    ED = NULL;

    return OsclSuccess;
}
#endif


OMX_ERRORTYPE OmxEncTestBase::EventHandler(OMX_OUT OMX_HANDLETYPE aComponent,
        OMX_OUT OMX_PTR aAppData,
        OMX_OUT OMX_EVENTTYPE aEvent,
        OMX_OUT OMX_U32 aData1,
        OMX_OUT OMX_U32 aData2,
        OMX_OUT OMX_PTR aEventData)
{
    OSCL_UNUSED_ARG(aComponent);
    OSCL_UNUSED_ARG(aAppData);
    OSCL_UNUSED_ARG(aEventData);

    if (OMX_EventCmdComplete == aEvent)
    {
        if (OMX_CommandStateSet == aData1)
        {
            switch ((OMX_S32) aData2)
            {
                    //Falling through next case
                case OMX_StateInvalid:
                case OMX_StatePause:
                case OMX_StateWaitForResources:
                    break;

                case OMX_StateLoaded:
                    if (StateCleanUp == iState)
                    {
                        iState = StateStop;
                        if (0 == --iPendingCommands)
                        {
                            RunIfNotReady();
                        }
                    }
                    break;

                case OMX_StateIdle:
                    if (StateStopping == iState || StateExecuting == iState ||
                            StateDynamicReconfig == iState || StateDisablePort == iState
                            || StateDecodeHeader == iState)
                    {
                        iState = StateCleanUp;
                        if (0 == --iPendingCommands)
                        {
                            RunIfNotReady();
                        }
                    }
                    else
                    {
                        iState = StateIdle;
                        if (0 == --iPendingCommands)
                        {
                            RunIfNotReady();
                        }
                    }
                    break;

                case OMX_StateExecuting:    //Change the state on receiving callback
                    if (StateIdle == iState)    //Chk whether some error condition has occured previously or not
                    {
                        iState = StateExecuting;
                        if (0 == --iPendingCommands)
                        {
                            RunIfNotReady();
                        }
                    }
                    break;

                default:
                    break;
            }
        }
        else if (OMX_CommandPortDisable == aData1)
        {
            iState = StateDynamicReconfig;
            if (0 == --iPendingCommands)
            {
                RunIfNotReady();
            }
        }
        else if (OMX_CommandPortEnable == aData1)
        {
            //Change the state from Reconfig to Executing on receiving this callback
            iState = StateExecuting;
            if (0 == --iPendingCommands)
            {
                RunIfNotReady();
            }
        }
    }
    else if (OMX_EventPortSettingsChanged == aEvent)
    {
        if (StateDecodeHeader == iState || StateExecuting == iState)
        {
            iState = StateDisablePort;
        }
        if (0 == --iPendingCommands)
        {
            RunIfNotReady();
        }
    }
    else if (OMX_EventBufferFlag == aEvent)
    {   //callback for EOS  //Change the state on receiving EOS callback
        printf("EOS Event recieved\n");
        iState = StateStopping;
        if (0 == --iPendingCommands)
        {
            RunIfNotReady();
        }
    }
    else if (OMX_EventError == aEvent)
    {
        if (OMX_ErrorSameState == aData1)
        {
            printf("Component reported SameState Error, try to proceed\n");
            if (StateCleanUp == iState)
            {
                iState = StateStop;
                if (0 == --iPendingCommands)
                {
                    RunIfNotReady();
                }
            }
        }
        else if (OMX_ErrorStreamCorrupt == aData1)
        {
            /* Don't do anything right now for the stream corrupt error,
             * just count the number of such callbacks and let the component to proceed */
            //printf("Component/Encoder reported Stream Corrupt Error\n");
        }
        else
        {
            // do nothing, just try to proceed normally
        }
    }

    return OMX_ErrorNone;
}


#if PROXY_INTERFACE
OsclReturnCode OmxEncTestBase::ProcessCallbackEmptyBufferDone(OsclAny* P)
{
    // re-cast the pointer
    EmptyBufferDoneSpecificData* ED = (EmptyBufferDoneSpecificData*) P;

    OMX_HANDLETYPE aComponent = ED->hComponent;
    OMX_PTR aAppData = ED->pAppData;
    OMX_BUFFERHEADERTYPE* aBuffer = ED->pBuffer;

    EmptyBufferDone(aComponent, aAppData, aBuffer);

    // release the allocated memory when no longer needed
    ipThreadSafeHandlerEmptyBufferDone->iMemoryPool->deallocate(ED);
    ED = NULL;

    //ED->hComponent = NULL;
    //ED->pAppData = NULL;
    //ED->pBuffer = NULL;

    return OsclSuccess;
}
#endif


OMX_ERRORTYPE OmxEncTestBase::EmptyBufferDone(OMX_OUT OMX_HANDLETYPE aComponent,
        OMX_OUT OMX_PTR aAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer)
{
    OSCL_UNUSED_ARG(aAppData);

    //Check the validity of buffer
    if (NULL == aBuffer || OMX_DirInput != aBuffer->nInputPortIndex)
    {
        RunIfNotReady();
        return OMX_ErrorBadParameter;
    }

    //ACTUAL PROCESSING
    OMX_S32 ii = 0;

    while ((OMX_U32)(ipInBuffer[ii]) != (OMX_U32) aBuffer && ii < iInBufferCount)
    {
        ii++;
    }

    if (iInBufferCount != ii)
    {
        ipInputAvail[ii] = OMX_TRUE;
    }
    else
    {
        printf("InBuffer corruption \n");
    }

    iInputReady = OMX_TRUE;

    iCount3++;
    if (0 == (iCount3 % 2))
    {
        iStopInput = OMX_TRUE;
    }

    if (0 >= iPendingCommands)
    {
        RunIfNotReady();
    }

    return OMX_ErrorNone;
}


#if PROXY_INTERFACE
OsclReturnCode OmxEncTestBase::ProcessCallbackFillBufferDone(OsclAny* P)
{
    // re-cast the pointer
    FillBufferDoneSpecificData* ED = (FillBufferDoneSpecificData*) P;

    OMX_HANDLETYPE aComponent = ED->hComponent;
    OMX_PTR aAppData = ED->pAppData;
    OMX_BUFFERHEADERTYPE* aBuffer = ED->pBuffer;

    FillBufferDone(aComponent, aAppData, aBuffer);

    // release the allocated memory when no longer needed
    ipThreadSafeHandlerFillBufferDone->iMemoryPool->deallocate(ED);
    ED = NULL;

    //ED->hComponent = NULL;
    //ED->pAppData = NULL;
    //ED->pBuffer = NULL;

    return OsclSuccess;
}
#endif


OMX_ERRORTYPE OmxEncTestBase::FillBufferDone(OMX_OUT OMX_HANDLETYPE aComponent,
        OMX_OUT OMX_PTR aAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer)
{
    OSCL_UNUSED_ARG(aAppData);

    //Check the validity of buffer
    if (NULL == aBuffer || OMX_DirOutput != aBuffer->nOutputPortIndex)
    {
        RunIfNotReady();
        return OMX_ErrorBadParameter;
    }

    OMX_U8* pOutputBuffer;
    OMX_S32 ii = 0;

    pOutputBuffer = (OMX_U8*)(aBuffer->pBuffer);

    //Output buffer has been freed by component & can now be passed again in FillThisBuffer call
    if (NULL != aBuffer)
    {
        if (0 != aBuffer->nFilledLen)
        {
            if (WriteOutput(pOutputBuffer, aBuffer->nFilledLen))
            {
                /* Clear the output buffer after writing to file,
                 * so that there is no chance of getting it written again while flushing port */
                aBuffer->nFilledLen = 0;
                iFramesWritten++;
            }
            else
            {
                printf("Failed to write output to file\n");
            }
        }
    }

    while (((OMX_U32) ipOutBuffer[ii] != (OMX_U32) aBuffer) &&
            (ii < iOutBufferCount))
    {
        ii++;
    }

    if (iOutBufferCount != ii)
    {
        ipOutReleased[ii] = OMX_TRUE;
    }
    else
    {
        printf("outbuffer corruption \n");
    }


    iCount++;
    if (0 == (iCount % 5))
    {
        iStopOutput = OMX_TRUE;
    }

    if (0 >= iPendingCommands)
    {
        RunIfNotReady();
    }

    return OMX_ErrorNone;
}

OMX_BOOL OmxEncTestBase::VerifyAllBuffersReturned()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxEncTestBase::VerifyAllBuffersReturned() - IN"));

    OMX_S32 ii;
    OMX_BOOL AllBuffersReturned = OMX_TRUE;
    //check here to verify whether all the ip/op buffers are returned back by the component or not

    for (ii = 0; ii < iInBufferCount; ii++)
    {
        if (OMX_FALSE == ipInputAvail[ii])
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxEncTestBase::VerifyAllBuffersReturned() - Not all the input buffers returned by component yet, rescheduling"));
            AllBuffersReturned = OMX_FALSE;
            break;
        }
    }


    for (ii = 0; ii < iOutBufferCount; ii++)
    {
        if (OMX_FALSE == ipOutReleased[ii])
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxEncTestBase::VerifyAllBuffersReturned() - Not all the input buffers returned by component yet, rescheduling"));
            AllBuffersReturned = OMX_FALSE;
            break;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxEncTestBase::VerifyAllBuffersReturned() - OUT"));

    return AllBuffersReturned;
}

//#if PROXY_INTERFACE
//struct EventHandlerSpecificData* CallbackContainer::pEventHandlerArray[EVENT_HANDLER_QUEUE_DEPTH];
//struct EmptyBufferDoneSpecificData* CallbackContainer::pEmptyBufferDoneArray[EMPTY_BUFFER_DONE_QUEUE_DEPTH];
//struct FillBufferDoneSpecificData* CallbackContainer::pFillBufferDoneArray[FILL_BUFFER_DONE_QUEUE_DEPTH];
//#endif

CallbackParentInt* CallbackContainer::iParent = NULL;

CallbackContainer::CallbackContainer(CallbackParentInt *parentinterface)
{
    iParent = parentinterface;

    iCallbackType.EventHandler = CallbackEventHandler;
    iCallbackType.EmptyBufferDone = CallbackEmptyBufferDone;
    iCallbackType.FillBufferDone = CallbackFillBufferDone;

//#if PROXY_INTERFACE
//    oscl_memset(pEventHandlerArray, 0, sizeof(pEventHandlerArray));
//    oscl_memset(pEmptyBufferDoneArray, 0, sizeof(pEmptyBufferDoneArray));
//    oscl_memset(pFillBufferDoneArray, 0, sizeof(pFillBufferDoneArray));
//#endif
};


// callback for Event Handler - in multithreaded case, event is queued to be processed later
//  in AO case, event is processed immediately by calling EventHandler
OMX_ERRORTYPE CallbackContainer::CallbackEventHandler(OMX_OUT OMX_HANDLETYPE aComponent,
        OMX_OUT OMX_PTR aAppData,
        OMX_OUT OMX_EVENTTYPE aEvent,
        OMX_OUT OMX_U32 aData1,
        OMX_OUT OMX_U32 aData2,
        OMX_OUT OMX_PTR aEventData)
{
#if PROXY_INTERFACE
    //OMX_S32 ii = 0;

    //while (NULL != pEventHandlerArray[ii]->hComponent && ii < EVENT_HANDLER_QUEUE_DEPTH)
    //{
    //    ii++;
    //}

    //EventHandlerSpecificData* ED = pEventHandlerArray[ii];


    // allocate the memory for the callback event specific data
    EventHandlerSpecificData* ED = (EventHandlerSpecificData*) iParent->ipThreadSafeHandlerEventHandler->iMemoryPool->allocate(sizeof(EventHandlerSpecificData));

    // pack the relevant data into the structure
    ED->hComponent = aComponent;
    ED->pAppData = aAppData;
    ED->eEvent = aEvent;
    ED->nData1 = aData1;
    ED->nData2 = aData2;
    ED->pEventData = aEventData;

    // convert the pointer into OsclAny ptr
    OsclAny* P = (OsclAny*) ED;

    // CALL the generic callback AO API:
    iParent->ipThreadSafeHandlerEventHandler->ReceiveEvent(P);

    return OMX_ErrorNone;
#else

    OMX_ERRORTYPE Status = OMX_ErrorNone;
    Status = iParent->EventHandler(aComponent, aAppData, aEvent, aData1, aData2, aEventData);
    return Status;

#endif
}


// callback for EmptyBufferDone - in multithreaded case, event is queued to be processed later
//  in AO case, event is processed immediately by calling EmptyBufferDone
OMX_ERRORTYPE CallbackContainer::CallbackEmptyBufferDone(OMX_OUT OMX_HANDLETYPE aComponent,
        OMX_OUT OMX_PTR aAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer)
{
#if PROXY_INTERFACE
    // OMX_S32 ii = 0;

    //while (NULL != pEmptyBufferDoneArray[ii]->hComponent && ii < EMPTY_BUFFER_DONE_QUEUE_DEPTH)
    //{
    //    ii++;
    //}

    //EmptyBufferDoneSpecificData* ED = pEmptyBufferDoneArray[ii];

    // allocate the memory for the callback event specific data
    EmptyBufferDoneSpecificData* ED = (EmptyBufferDoneSpecificData*) iParent->ipThreadSafeHandlerEmptyBufferDone->iMemoryPool->allocate(sizeof(EmptyBufferDoneSpecificData));

    // pack the relevant data into the structure
    ED->hComponent = aComponent;
    ED->pAppData = aAppData;
    ED->pBuffer = aBuffer;

    // convert the pointer into OsclAny ptr
    OsclAny* P = (OsclAny *) ED;

    // CALL the generic callback AO API:
    iParent->ipThreadSafeHandlerEmptyBufferDone->ReceiveEvent(P);
    return OMX_ErrorNone;

#else

    OMX_ERRORTYPE Status = OMX_ErrorNone;
    Status = iParent->EmptyBufferDone(aComponent, aAppData, aBuffer);
    return Status;

#endif
}


// callback for FillBufferDone - in multithreaded case, event is queued to be processed later
//  in AO case, event is processed immediately by calling FillBufferDone
OMX_ERRORTYPE CallbackContainer::CallbackFillBufferDone(OMX_OUT OMX_HANDLETYPE aComponent,
        OMX_OUT OMX_PTR aAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer)
{
#if PROXY_INTERFACE
    //OMX_S32 ii = 0;

    //while (NULL != pFillBufferDoneArray[ii]->hComponent && ii < FILL_BUFFER_DONE_QUEUE_DEPTH)
    //{
    //    ii++;
    //}

    //FillBufferDoneSpecificData* ED = pFillBufferDoneArray[ii];

    // allocate the memory for the callback event specific data
    FillBufferDoneSpecificData* ED = (FillBufferDoneSpecificData*) iParent->ipThreadSafeHandlerFillBufferDone->iMemoryPool->allocate(sizeof(FillBufferDoneSpecificData));

    // pack the relevant data into the structure
    ED->hComponent = aComponent;
    ED->pAppData = aAppData;

    ED->pBuffer = aBuffer;
    // convert the pointer into OsclAny ptr
    OsclAny* P = (OsclAny*) ED;

    // CALL the generic callback AO API:
    iParent->ipThreadSafeHandlerFillBufferDone->ReceiveEvent(P);

    return OMX_ErrorNone;

#else

    OMX_ERRORTYPE Status = OMX_ErrorNone;
    Status = iParent->FillBufferDone(aComponent, aAppData, aBuffer);
    return Status;

#endif
}
