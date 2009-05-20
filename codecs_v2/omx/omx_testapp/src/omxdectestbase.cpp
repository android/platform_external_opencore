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
#include "omxdectestbase.h"


OmxDecTestBase::OmxDecTestBase(const char AOName[]) :
        OsclTimerObject(OsclActiveObject::EPriorityNominal, AOName)
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
    ipUseInBuff = NULL;
    ipUseOutBuff = NULL;
    ipInBuffer = NULL;
    ipOutBuffer = NULL;
    ipInputAvail = NULL;
    ipOutReleased = NULL;

    ipAVCBSO = NULL;
    ipMp3Bitstream = NULL;
    ipBitstreamBuffer = NULL;

    iCount = 0;
    iCount1 = 0;
    iCount2 = 0;
    iCount3 = 0;

    iStreamCorruptCount = 0;

    for (jj = 0; jj < 50; jj++)
    {
        iSimFragSize[jj] = 0;
    }

    //Get the logger object here in the base class
    iLogger = PVLogger::GetLoggerObject("OmxComponentTestApplication");

#if PROXY_INTERFACE
    ipThreadSafeHandlerEventHandler = NULL;
    ipThreadSafeHandlerEmptyBufferDone = NULL;
    ipThreadSafeHandlerFillBufferDone = NULL;

#endif
}

void OmxDecTestBase::InitScheduler()
{
    OsclScheduler::Init("OMX.PV.dec"); //DV: removed, &iAllocator);

}


void OmxDecTestBase::StartTestApp()
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
OsclReturnCode OmxDecTestBase::ProcessCallbackEventHandler(OsclAny* P)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "OmxDecTestBase::ProcessCallbackEventHandler() - Processing the EventHandler Callback IN"));

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
    ED->hComponent = NULL;
    ED->nData1 = 0;
    ED->nData2 = 0;
    ED->pAppData = NULL;
    ED->pEventData = NULL;

    ipThreadSafeHandlerEventHandler->iMemoryPool->deallocate(ED);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "OmxDecTestBase::ProcessCallbackEventHandler() - OUT"));

    return OsclSuccess;
}
#endif


OMX_ERRORTYPE OmxDecTestBase::EventHandler(OMX_OUT OMX_HANDLETYPE aComponent,
        OMX_OUT OMX_PTR aAppData,
        OMX_OUT OMX_EVENTTYPE aEvent,
        OMX_OUT OMX_U32 aData1,
        OMX_OUT OMX_U32 aData2,
        OMX_OUT OMX_PTR aEventData)
{
    OSCL_UNUSED_ARG(aComponent);
    OSCL_UNUSED_ARG(aAppData);
    OSCL_UNUSED_ARG(aEventData);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestBase::EventHandler() - IN"));

    if (OMX_EventCmdComplete == aEvent)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestBase::EventHandler() - Command Complete callback arrived"));

        if (OMX_CommandStateSet == aData1)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestBase::EventHandler() - State Changed callback has come under Command Complete"));

            switch ((OMX_S32) aData2)
            {
                    //Falling through next case
                case OMX_StateInvalid:
                case OMX_StateWaitForResources:
                    break;

                case OMX_StateLoaded:
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxDecTestBase::EventHandler() - Callback, State Changed to OMX_StateLoaded"));

                    if (StateCleanUp == iState)
                    {
                        iState = StateStop;
                        if (0 == --iPendingCommands)
                        {
                            RunIfNotReady();
                        }
                    }
                }
                break;

                case OMX_StateIdle:
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxDecTestBase::EventHandler() - Callback, State Changed to OMX_StateIdle"));

                    if (StateStopping == iState ||
                            StateDynamicReconfig == iState ||
                            StateDisablePort == iState ||
                            StateDecodeHeader == iState)
                    {
                        iState = StateCleanUp;
                        if (0 == --iPendingCommands)
                        {
                            RunIfNotReady();
                        }
                    }
                    //this will be the case in EOS missing test case, go to a stopping state now
                    else if (StateExecuting == iState)
                    {
                        iState = StateStopping;
                        if (0 == --iPendingCommands)
                        {
                            RunIfNotReady();
                        }
                    }
                    else if (StateStop == iState || StateError == iState)
                    {
                        //Do not change the state because error has occured previously
                        RunIfNotReady();
                    }
                    else
                    {
                        iState = StateIdle;
                        if (0 == --iPendingCommands)
                        {
                            RunIfNotReady();
                        }
                    }
                }
                break;

                case OMX_StateExecuting:    //Change the state on receiving callback
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxDecTestBase::EventHandler() - Callback, State Changed to OMX_StateExecuting"));

                    if (StateIdle == iState)    //Chk whether some error condition has occured previously or not
                    {
                        iState = StateDecodeHeader;
                        if (0 == --iPendingCommands)
                        {
                            RunIfNotReady();
                        }
                    }
                    else if (StatePause == iState)
                    {
                        iState = StateExecuting;
                        if (0 == --iPendingCommands)
                        {
                            RunIfNotReady();
                        }
                    }
                }
                break;

                case OMX_StatePause:    //Change the state on receiving callback
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxDecTestBase::EventHandler() - Callback, State Changed to OMX_StatePause"));

                    if (StateExecuting == iState)
                    {
                        iState = StatePause;
                        if (0 == --iPendingCommands)
                        {
                            RunIfNotReady();
                        }
                    }
                }
                break;

                default:
                    break;
            }
        }
        else if (OMX_CommandPortDisable == aData1)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestBase::EventHandler() - Port Disable callback has come under Command Complete"));

            //Do the transition only if no error has occured previously
            if (StateStop != iState && StateError != iState)
            {
                iState = StateDynamicReconfig;

                if (0 == --iPendingCommands)
                {
                    RunIfNotReady();
                }
            }
        }
        else if (OMX_CommandPortEnable == aData1)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestBase::EventHandler() - Port Enable callback has come under Command Complete"));

            //Change the state from Reconfig to Executing on receiving this callback if there is no error
            if (StateStop != iState && StateError != iState)
            {
                iState = StateExecuting;

                if (0 == --iPendingCommands)
                {
                    RunIfNotReady();
                }
            }
        }
        else if (OMX_CommandFlush == aData1)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestBase::EventHandler() - Flush Port callback has come under Command Complete"));

            if (0 == --iPendingCommands)
            {
                //Move to a intermediate state for port/buffer verification
                iState = StateIntermediate;
                RunIfNotReady();
            }
        }
    }
    else if (OMX_EventPortSettingsChanged == aEvent)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestBase::EventHandler() - Port Settings Changed callback arrived"));

        if (StateDecodeHeader == iState || StateExecuting == iState)
        {
            iState = StateDisablePort;
            RunIfNotReady();

        }
    }
    else if (OMX_EventBufferFlag == aEvent)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestBase::EventHandler() - End Of Stream callback arrived"));

        //callback for EOS  //Change the state on receiving EOS callback
        iState = StateStopping;
        if (0 == --iPendingCommands)
        {
            RunIfNotReady();
        }
    }
    else if (OMX_EventError == aEvent)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestBase::EventHandler() - Error returned in the callback"));

        if (OMX_ErrorSameState == aData1)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestBase::EventHandler() - Same State Error, trying to proceed"));
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
             * just count the number of such callbacks and let the decoder to proceed */
            iStreamCorruptCount++;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestBase::EventHandler() - Stream Corrupt Error, total as of now is %d", iStreamCorruptCount));
        }
        else
        {
            // do nothing, just try to proceed normally
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestBase::EventHandler() - OUT"));

    return OMX_ErrorNone;
}


#if PROXY_INTERFACE
OsclReturnCode OmxDecTestBase::ProcessCallbackEmptyBufferDone(OsclAny* P)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "OmxDecTestBase::ProcessCallbackEmptyBufferDone() - Processing the EmptyBufferDone Callback IN"));

    // re-cast the pointer
    EmptyBufferDoneSpecificData* ED = (EmptyBufferDoneSpecificData*) P;

    OMX_HANDLETYPE aComponent = ED->hComponent;
    OMX_PTR aAppData = ED->pAppData;
    OMX_BUFFERHEADERTYPE* aBuffer = ED->pBuffer;

    EmptyBufferDone(aComponent, aAppData, aBuffer);

    // release the allocated memory when no longer needed

    ED->hComponent = NULL;
    ED->pAppData = NULL;
    ED->pBuffer = NULL;

    ipThreadSafeHandlerEmptyBufferDone->iMemoryPool->deallocate(ED);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "OmxDecTestBase::ProcessCallbackEmptyBufferDone() - OUT"));

    return OsclSuccess;
}
#endif


OMX_ERRORTYPE OmxDecTestBase::EmptyBufferDone(OMX_OUT OMX_HANDLETYPE aComponent,
        OMX_OUT OMX_PTR aAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer)
{
    OSCL_UNUSED_ARG(aComponent);
    OSCL_UNUSED_ARG(aAppData);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestBase::EmptyBufferDone() - IN"));

    //Check the validity of buffer
    if (NULL == aBuffer || OMX_DirInput != aBuffer->nInputPortIndex)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "OmxDecTestBase::EmptyBufferDone() - ERROR, Invalid buffer found in callback, OUT"));

        iState = StateError;
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
        //If the buffer is already returned and then a callback comes for same buffer
        //report an error and stop
        if (OMX_TRUE == ipInputAvail[ii])
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "OmxDecTestBase::EmptyBufferDone() - ERROR, Same bufer with index %d returned twice", ii));

            iState = StateError;
            RunIfNotReady();
        }

        ipInputAvail[ii] = OMX_TRUE;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "OmxDecTestBase::EmptyBufferDone() - Input buffer with index %d returned in callback", ii));
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "OmxDecTestBase::EmptyBufferDone() - ERROR, Invalid buffer found in callback, cannot mark it free"));

        iState = StateError;
        RunIfNotReady();
    }

    iInputReady = OMX_TRUE;


    //To simulate input busy condition

    iCount3++;
    if (0 == (iCount3 % 2))
    {
        iStopInput = OMX_TRUE;
    }


    if (0 >= iPendingCommands)
    {
        RunIfNotReady();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestBase::EmptyBufferDone() - OUT"));
    return OMX_ErrorNone;
}


#if PROXY_INTERFACE
OsclReturnCode OmxDecTestBase::ProcessCallbackFillBufferDone(OsclAny* P)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "OmxDecTestBase::ProcessCallbackFillBufferDone() - Processing the FillBufferDone Callback IN"));

    // re-cast the pointer
    FillBufferDoneSpecificData* ED = (FillBufferDoneSpecificData*) P;

    OMX_HANDLETYPE aComponent = ED->hComponent;
    OMX_PTR aAppData = ED->pAppData;
    OMX_BUFFERHEADERTYPE* aBuffer = ED->pBuffer;

    FillBufferDone(aComponent, aAppData, aBuffer);

    ED->hComponent = NULL;
    ED->pAppData = NULL;
    ED->pBuffer = NULL;

    ipThreadSafeHandlerFillBufferDone->iMemoryPool->deallocate(ED);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "OmxDecTestBase::ProcessCallbackFillBufferDone() - OUT"));

    return OsclSuccess;
}
#endif


OMX_ERRORTYPE OmxDecTestBase::FillBufferDone(OMX_OUT OMX_HANDLETYPE aComponent,
        OMX_OUT OMX_PTR aAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer)
{
    OSCL_UNUSED_ARG(aComponent);
    OSCL_UNUSED_ARG(aAppData);

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestBase::FillBufferDone() - IN"));

    //Check the validity of buffer
    if (NULL == aBuffer || OMX_DirOutput != aBuffer->nOutputPortIndex)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "OmxDecTestBase::FillBufferDone() - ERROR, Invalid buffer found in callback, OUT"));

        iState = StateError;
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

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxDecTestBase::FillBufferDone() - Frame %i processed Timestamp %d", iFramesWritten, aBuffer->nTimeStamp));

                iFramesWritten++;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "OmxDecTestBase::FillBufferDone() - ERROR, Failed to write output to file"));
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
        //If the buffer is already returned and then a callback comes for same buffer
        //report an error and stop
        if (OMX_TRUE == ipOutReleased[ii])
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "OmxDecTestBase::FillBufferDone() - ERROR, Same bufer with index %d returned twice", ii));

            iState = StateError;
            RunIfNotReady();
        }

        ipOutReleased[ii] = OMX_TRUE;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "OmxDecTestBase::FillBufferDone() - Output buffer with index %d marked free", ii));
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "OmxDecTestBase::FillBufferDone() - ERROR, Invalid buffer found in callback, cannot mark it free"));
        iState = StateError;
        RunIfNotReady();
    }

    //To simulate output busy condition
    iCount++;
    if (0 == (iCount % 5))
    {
        iStopOutput = OMX_TRUE;
    }

    if (0 >= iPendingCommands)
    {
        RunIfNotReady();
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestBase::FillBufferDone() - OUT"));
    return OMX_ErrorNone;
}

OMX_BOOL OmxDecTestBase::VerifyAllBuffersReturned()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestBase::VerifyAllBuffersReturned() - IN"));

    OMX_S32 ii;
    OMX_BOOL AllBuffersReturned = OMX_TRUE;
    //check here to verify whether all the ip/op buffers are returned back by the component or not

    for (ii = 0; ii < iInBufferCount; ii++)
    {
        if (OMX_FALSE == ipInputAvail[ii])
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxDecTestBase::VerifyAllBuffersReturned() - Not all the input buffers returned by component yet, rescheduling"));
            AllBuffersReturned = OMX_FALSE;
            break;
        }
    }


    for (ii = 0; ii < iOutBufferCount; ii++)
    {
        if (OMX_FALSE == ipOutReleased[ii])
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxDecTestBase::VerifyAllBuffersReturned() - Not all the input buffers returned by component yet, rescheduling"));
            AllBuffersReturned = OMX_FALSE;
            break;
        }
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestBase::VerifyAllBuffersReturned() - OUT"));

    return AllBuffersReturned;
}


#if PROXY_INTERFACE
struct EventHandlerSpecificData* CallbackContainer::pEventHandlerArray[EVENT_HANDLER_QUEUE_DEPTH];
struct EmptyBufferDoneSpecificData* CallbackContainer::pEmptyBufferDoneArray[EMPTY_BUFFER_DONE_QUEUE_DEPTH];
struct FillBufferDoneSpecificData* CallbackContainer::pFillBufferDoneArray[FILL_BUFFER_DONE_QUEUE_DEPTH];
#endif

CallbackParentInt* CallbackContainer::iParent = NULL;

CallbackContainer::CallbackContainer(CallbackParentInt *parentinterface)
{
    iParent = parentinterface;

    iCallbackType.EventHandler = CallbackEventHandler;
    iCallbackType.EmptyBufferDone = CallbackEmptyBufferDone;
    iCallbackType.FillBufferDone = CallbackFillBufferDone;

#if PROXY_INTERFACE
    oscl_memset(pEventHandlerArray, 0, sizeof(pEventHandlerArray));
    oscl_memset(pEmptyBufferDoneArray, 0, sizeof(pEmptyBufferDoneArray));
    oscl_memset(pFillBufferDoneArray, 0, sizeof(pFillBufferDoneArray));
#endif
};

OMX_ERRORTYPE CallbackContainer::CallbackEventHandler(OMX_OUT OMX_HANDLETYPE aComponent,
        OMX_OUT OMX_PTR aAppData,
        OMX_OUT OMX_EVENTTYPE aEvent,
        OMX_OUT OMX_U32 aData1,
        OMX_OUT OMX_U32 aData2,
        OMX_OUT OMX_PTR aEventData)
{
#if PROXY_INTERFACE
    OMX_S32 ii = 0;

    OmxDecTestBase *client_ptr = (OmxDecTestBase *) aAppData;

    EventHandlerSpecificData* ED = (EventHandlerSpecificData*) client_ptr->ipThreadSafeHandlerEventHandler->iMemoryPool->allocate(sizeof(EventHandlerSpecificData));


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
#else

    iParent->EventHandler(aComponent, aAppData, aEvent, aData1, aData2, aEventData);

#endif

    return OMX_ErrorNone;
}


OMX_ERRORTYPE CallbackContainer::CallbackEmptyBufferDone(OMX_OUT OMX_HANDLETYPE aComponent,
        OMX_OUT OMX_PTR aAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer)
{
#if PROXY_INTERFACE
    OMX_S32 ii = 0;

    OmxDecTestBase *client_ptr = (OmxDecTestBase *) aAppData;
    EmptyBufferDoneSpecificData* ED = (EmptyBufferDoneSpecificData*) client_ptr->ipThreadSafeHandlerEmptyBufferDone->iMemoryPool->allocate(sizeof(EmptyBufferDoneSpecificData));

    // pack the relevant data into the structure
    ED->hComponent = aComponent;
    ED->pAppData = aAppData;
    ED->pBuffer = aBuffer;

    // convert the pointer into OsclAny ptr
    OsclAny* P = (OsclAny *) ED;

    // CALL the generic callback AO API:
    iParent->ipThreadSafeHandlerEmptyBufferDone->ReceiveEvent(P);
#else

    iParent->EmptyBufferDone(aComponent, aAppData, aBuffer);

#endif

    return OMX_ErrorNone;
}


OMX_ERRORTYPE CallbackContainer::CallbackFillBufferDone(OMX_OUT OMX_HANDLETYPE aComponent,
        OMX_OUT OMX_PTR aAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* aBuffer)
{
#if PROXY_INTERFACE
    OMX_S32 ii = 0;

    OmxDecTestBase *client_ptr = (OmxDecTestBase *) aAppData;
    FillBufferDoneSpecificData* ED = (FillBufferDoneSpecificData*) client_ptr->ipThreadSafeHandlerFillBufferDone->iMemoryPool->allocate(sizeof(FillBufferDoneSpecificData));


    // pack the relevant data into the structure
    ED->hComponent = aComponent;
    ED->pAppData = aAppData;

    ED->pBuffer = aBuffer;
    // convert the pointer into OsclAny ptr
    OsclAny* P = (OsclAny*) ED;

    // CALL the generic callback AO API:
    iParent->ipThreadSafeHandlerFillBufferDone->ReceiveEvent(P);

#else

    iParent->FillBufferDone(aComponent, aAppData, aBuffer);

#endif

    return OMX_ErrorNone;
}
