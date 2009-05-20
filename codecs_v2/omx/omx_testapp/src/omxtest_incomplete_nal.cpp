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
#include "omxdectest.h"
#include "oscl_mem.h"

#define FRAGMENT_DROP_RATE 20
#define NUMBER_PARTIAL_FRAGMENTS 4

// For AVC component, Miss some of the NAL's at random locations
OMX_ERRORTYPE OmxDecTestIncompleteNALTest::GetInputFrameAvc()
{
    OMX_S32 Index;
    OMX_S32 Size;
    OMX_ERRORTYPE Status;
    OMX_S32 TempSize;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestIncompleteNALTest::GetInputFrameAvc() - IN"));

    if (OMX_TRUE == iInputReady)
    {
        if (OMX_TRUE == iInputWasRefused)
        {
            Index = iInIndex; //use previously found and saved Index, do not read the file (this was done earlier)
            iInputWasRefused = OMX_FALSE; // reset the flag
        }
        else
        {
            Index = 0;
            while (OMX_FALSE == ipInputAvail[Index] && Index < iInBufferCount)
            {
                Index++;
            }

            if (iInBufferCount == Index)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestIncompleteNALTest::GetInputFrameAvc() - Input buffer not available, OUT"));
                return OMX_ErrorNone;
            }

            if (iHeaderSent)
            {
                int inserted_size = 0;

                if (!iFragmentInProgress && !iDivideBuffer)
                {
                    if (AVCOMX_FAIL == (ipAVCBSO->GetNextFullNAL(&ipNalBuffer, &iNalSize)))
                    {
                        iStopProcessingInput = OMX_TRUE;
                        iInIndex = Index;
                        iLastInputFrame = iFrameNum;
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestIncompleteNALTest::GetInputFrameAvc() - Error GetNextFullNAL failed, OUT"));
                        return OMX_ErrorNone;
                    }

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxDecTestIncompleteNALTest::GetInputFrameAvc() - Next NAL identified of size %d", iNalSize));

                    Size = iNalSize;
                    iCurFrag = 0;

                    /* Count the number of input NAL's except SPS and PPS
                     * Wiil be used later to verify the conformance */
                    if (AVC_NALTYPE_SPS != (ipNalBuffer[0] & 0x1F) &&
                            AVC_NALTYPE_PPS != (ipNalBuffer[0] & 0x1F))
                    {
                        iFrameNum++;
                    }

                    for (OMX_S32 kk = 0; kk < iNumSimFrags - 1; kk++)
                    {
                        iSimFragSize[kk] = Size / iNumSimFrags;
                    }

                    iSimFragSize[iNumSimFrags - 1] = Size - (iNumSimFrags - 1) * (Size / iNumSimFrags); // last fragment
                }

                if (OMX_FALSE == iDivideBuffer)
                {
                    Size = iSimFragSize[iCurFrag];

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxDecTestIncompleteNALTest::GetInputFrameAvc() - Fragment number %d of the current NAL is of size %d", iCurFrag, iNalSize));

                    iCurFrag++;

                    if (iCurFrag == iNumSimFrags)
                    {
                        iFragmentInProgress = OMX_FALSE;
                    }
                    else
                    {
                        iFragmentInProgress = OMX_TRUE;
                    }

#ifdef INSERT_NAL_START_CODE
                    if (iCurFrag == 1) // frag was already incremented
                    {
                        oscl_memcpy(ipInBuffer[Index]->pBuffer, NAL_START_CODE, NAL_START_CODE_SIZE);
                        inserted_size = NAL_START_CODE_SIZE;
                    }
                    else
                    {
                        inserted_size = 0;
                    }
#endif
                    if ((Size + inserted_size) > iInBufferSize)
                    {
                        // break up frame (fragment) into multiple buffers
                        iDivideBuffer = OMX_TRUE;

                        TempSize = iInBufferSize - inserted_size;
                        iRemainingFrameSize = Size - TempSize;
                        Size = TempSize;
                    }
                }
                else
                {
                    if ((Size = iRemainingFrameSize) < iInBufferSize)
                    {
                        // this is the last piece of a broken up frame
                        iRemainingFrameSize = 0;
                        iDivideBuffer = OMX_FALSE;
                    }
                    else
                    {
                        iRemainingFrameSize = Size - iInBufferSize;
                        Size = iInBufferSize;
                    }
                }

                oscl_memcpy(ipInBuffer[Index]->pBuffer + inserted_size, ipNalBuffer, Size);
                ipInBuffer[Index]->nFilledLen = Size + inserted_size;
                ipInBuffer[Index]->nOffset = 0;  // for now all data starts at the beginning of the buffer

                ipNalBuffer += Size;

                //Check for next piece of NAL & drop it if it's the one
                if ((0 == iFrameNum % FRAGMENT_DROP_RATE) && (iFrameNum >= FRAGMENT_DROP_RATE)
                        && (OMX_FALSE == iDivideBuffer))
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxDecTestIncompleteNALTest::GetInputFrameAvc() - Dropping the next fragment number %d of NAL this time", iCurFrag));

                    //Drop the last piece of NAL containing EndOfFrame Flag
                    if (0 == iFrameNum % (FRAGMENT_DROP_RATE*2))
                    {
                        if ((NUMBER_PARTIAL_FRAGMENTS - 1) == iCurFrag)
                        {
                            Size = iSimFragSize[iCurFrag];
                            ipNalBuffer += Size;
                            iCurFrag++;
                            iIncompleteFrames++;
                        }
                    }
                    else
                    {
                        //Drop the middle piece of NAL
                        if ((NUMBER_PARTIAL_FRAGMENTS / 2) == iCurFrag)
                        {
                            Size = iSimFragSize[iCurFrag];
                            ipNalBuffer += Size;
                            iCurFrag++;
                            iIncompleteFrames++;
                        }
                    }
                }
            }
            else
            {
                ipAVCBSO->GetNextFullNAL(&ipNalBuffer, &iNalSize);

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxDecTestIncompleteNALTest::GetInputFrameAvc() - Next NAL identified of size %d", iNalSize));

#ifdef INSERT_NAL_START_CODE

                oscl_memcpy(ipInBuffer[Index]->pBuffer, NAL_START_CODE, NAL_START_CODE_SIZE);
                oscl_memcpy(ipInBuffer[Index]->pBuffer + NAL_START_CODE_SIZE, ipNalBuffer, iNalSize);
                ipInBuffer[Index]->nFilledLen = iNalSize + NAL_START_CODE_SIZE;

#else
                oscl_memcpy(ipInBuffer[Index]->pBuffer, ipNalBuffer, iNalSize);
                ipInBuffer[Index]->nFilledLen = iNalSize;
#endif
                ipInBuffer[Index]->nOffset = 0;  // for now all data starts at the beginning of the buffer

                iHeaderSent = OMX_TRUE;

                /* Count the number of input NAL's except SPS and PPS
                 * Will be used later to verify the conformance */
                if (AVC_NALTYPE_SPS != (ipNalBuffer[0] & 0x1F) &&
                        AVC_NALTYPE_PPS != (ipNalBuffer[0] & 0x1F))
                {
                    iFrameNum++;
                }
            }

            if (iFragmentInProgress || iDivideBuffer)
            {
                ipInBuffer[Index]->nFlags  = 0;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxDecTestIncompleteNALTest::GetInputFrameAvc() - Input buffer of index %d without any marker", Index));
            }
            else
            {
                ipInBuffer[Index]->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxDecTestIncompleteNALTest::GetInputFrameAvc() - Input buffer of index %d with OMX_BUFFERFLAG_ENDOFFRAME flag marked", Index));
            }

            //Check again, we may have dropped last piece of NAL
            if (iCurFrag == iNumSimFrags)
            {
                iFragmentInProgress = OMX_FALSE;
            }
        }

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "OmxDecTestIncompleteNALTest::GetInputFrameAvc() - Calling EmptyThisBuffer with Alloc Len %d, Filled Len %d Offset %d TimeStamp %d",
                         ipInBuffer[Index]->nAllocLen, ipInBuffer[Index]->nFilledLen, ipInBuffer[Index]->nOffset, ipInBuffer[Index]->nTimeStamp));

        Status = OMX_EmptyThisBuffer(ipAppPriv->Handle, ipInBuffer[Index]);
        if (OMX_ErrorNone == Status)
        {
            ipInputAvail[Index] = OMX_FALSE; // mark unavailable

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxDecTestIncompleteNALTest::GetInputFrameAvc() - Sent the input buffer sucessfully, OUT"));

            return OMX_ErrorNone;
        }
        else if (OMX_ErrorInsufficientResources == Status)
        {
            iInIndex = Index; // save the current input buffer, wait for call back
            iInputReady = OMX_FALSE;
            iInputWasRefused = OMX_TRUE;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestIncompleteNALTest::GetInputFrameAvc() - EmptyThisBuffer failed with Error OMX_ErrorInsufficientResources, OUT"));
            return OMX_ErrorInsufficientResources;
        }
        else
        {
            iStopProcessingInput = OMX_TRUE;
            iLastInputFrame = iFrameNum;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestIncompleteNALTest::GetInputFrameAvc() - ERROR, EmptyThisBuffer failed, OUT"));

            return OMX_ErrorNone;
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestIncompleteNALTest::GetInputFrameAvc() - ERROR, Input is not ready to be sent, OUT"));
        return OMX_ErrorInsufficientResources;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestIncompleteNALTest::GetInputFrameAvc() - OUT"));

    return OMX_ErrorNone;
}


/*
 * Active Object class's Run () function
 * Control all the states of AO & sends openmax API's to the component
 */
static OMX_BOOL DisableRun = OMX_FALSE;

void OmxDecTestIncompleteNALTest::Run()
{

    switch (iState)
    {
        case StateUnLoaded:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestIncompleteNALTest::Run() - StateUnLoaded IN"));

            OMX_ERRORTYPE Err;
            OMX_U32 PortIndex, ii;

            //Run this test case only for AVC component, exit if there is some other
            if (0 != oscl_strcmp(iFormat, "H264"))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestIncompleteNALTest::Run() - ERROR, This test is only meant for AVC component"));
                iState = StateUnLoaded;
                OsclExecScheduler* sched = OsclExecScheduler::Current();
                sched->StopScheduler();
                break;
            }

            //Change this varaible to turn on partial fragmentation
            iNumSimFrags = NUMBER_PARTIAL_FRAGMENTS;

            if (!iCallbacks->initCallbacks())
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestIncompleteNALTest::Run() - ERROR initCallbacks failed, OUT"));
                StopOnError();
                break;
            }

            ipAppPriv = (AppPrivateType*) oscl_malloc(sizeof(AppPrivateType));
            CHECK_MEM(ipAppPriv, "Component_Handle");

            ipAVCBSO = OSCL_NEW(AVCBitstreamObject, (ipInputFile));
            CHECK_MEM(ipAVCBSO, "Bitstream_Buffer");

            //This should be the first call to the component to load it.
            Err = OMX_Init();
            CHECK_ERROR(Err, "OMX_Init");
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestIncompleteNALTest::Run() - OMX_Init done"));


            if (NULL != iName)
            {
                Err = OMX_GetHandle(&ipAppPriv->Handle, iName, (OMX_PTR) this , iCallbacks->getCallbackStruct());
                CHECK_ERROR(Err, "GetHandle");
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestIncompleteNALTest::Run() - Got Handle for the component %s", iName));
            }
            else if (NULL != iRole)
            {
                //Determine the component first & then get the handle
                OMX_U32 NumComps = 0;
                OMX_STRING* pCompOfRole;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestIncompleteNALTest::Run() - Finding out the role for the component %s", iRole));

                // call once to find out the number of components that can fit the role
                Err = OMX_GetComponentsOfRole(iRole, &NumComps, NULL);

                if (OMX_ErrorNone != Err || NumComps < 1)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestIncompleteNALTest::Run() - ERROR, No component can handle the specified role %s", iRole));
                    StopOnError();
                    ipAppPriv->Handle = NULL;
                    break;
                }

                pCompOfRole = (OMX_STRING*) oscl_malloc(NumComps * sizeof(OMX_STRING));
                CHECK_MEM(pCompOfRole, "ComponentRoleArray");

                for (ii = 0; ii < NumComps; ii++)
                {
                    pCompOfRole[ii] = (OMX_STRING) oscl_malloc(PV_OMX_MAX_COMPONENT_NAME_LENGTH * sizeof(OMX_U8));
                    CHECK_MEM(pCompOfRole[ii], "ComponentRoleArray");
                }

                if (StateError == iState)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "OmxDecTestIncompleteNALTest::Run() - Error occured in this state, StateUnLoaded OUT"));
                    RunIfNotReady();
                    break;
                }

                // call 2nd time to get the component names
                Err = OMX_GetComponentsOfRole(iRole, &NumComps, (OMX_U8**) pCompOfRole);
                CHECK_ERROR(Err, "GetComponentsOfRole");

                for (ii = 0; ii < NumComps; ii++)
                {
                    // try to create component
                    Err = OMX_GetHandle(&ipAppPriv->Handle, (OMX_STRING) pCompOfRole[ii], (OMX_PTR) this, iCallbacks->getCallbackStruct());
                    // if successful, no need to continue
                    if ((OMX_ErrorNone == Err) && (NULL != ipAppPriv->Handle))
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestIncompleteNALTest::Run() - Got Handle for the component %s", pCompOfRole[ii]));
                        break;
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestIncompleteNALTest::Run() - ERROR, Cannot get component %s handle, try another if possible", pCompOfRole[ii]));
                    }

                }
                // whether successful or not, need to free CompOfRoles
                for (ii = 0; ii < NumComps; ii++)
                {
                    oscl_free(pCompOfRole[ii]);
                    pCompOfRole[ii] = NULL;
                }
                oscl_free(pCompOfRole);
                pCompOfRole = NULL;

                // check if there was a problem
                CHECK_ERROR(Err, "GetHandle");
                CHECK_MEM(ipAppPriv->Handle, "ComponentHandle");

            }
            //This will initialize the size and version of the iPortInit structure
            INIT_GETPARAMETER_STRUCT(OMX_PORT_PARAM_TYPE, iPortInit);

            Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamVideoInit, &iPortInit);
            CHECK_ERROR(Err, "GetParameter_VideoInit");

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestIncompleteNALTest::Run() - GetParameter called for OMX_IndexParamVideoInit"));

            for (ii = 0; ii < iPortInit.nPorts; ii++)
            {
                PortIndex = iPortInit.nStartPortNumber + ii;

                //This will initialize the size and version of the iParamPort structure
                INIT_GETPARAMETER_STRUCT(OMX_PARAM_PORTDEFINITIONTYPE, iParamPort);
                iParamPort.nPortIndex = PortIndex;

                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamPortDefinition, &iParamPort);
                CHECK_ERROR(Err, "GetParameter_IndexParamPortDefinition");
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestIncompleteNALTest::Run() - GetParameter called for OMX_IndexParamPortDefinition on port %d", PortIndex));

                if (0 == iParamPort.nBufferCountMin)
                {
                    /* a buffer count of 0 is not allowed */
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestIncompleteNALTest::Run() - Error, GetParameter for OMX_IndexParamPortDefinition returned 0 min buffer count"));
                    StopOnError();
                    break;
                }

                if (iParamPort.nBufferCountMin > iParamPort.nBufferCountActual)
                {
                    /* Min buff count can't be more than actual buff count */
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "OmxDecTestIncompleteNALTest::Run() - ERROR, GetParameter for OMX_IndexParamPortDefinition returned actual buffer count %d less than min buffer count %d", iParamPort.nBufferCountActual, iParamPort.nBufferCountMin));
                    StopOnError();
                    break;
                }

                if (OMX_DirInput == iParamPort.eDir)
                {
                    iInputPortIndex = PortIndex;

                    iInBufferSize = iParamPort.nBufferSize;
                    iInBufferCount = iParamPort.nBufferCountActual;
                    iParamPort.nBufferCountActual = iInBufferCount;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxDecTestIncompleteNALTest::Run() - GetParameter returned Num of input buffers %d with Size %d", iInBufferCount, iInBufferSize));

                }
                else if (OMX_DirOutput == iParamPort.eDir)
                {
                    iOutputPortIndex = PortIndex;

                    iOutBufferSize = iParamPort.nBufferSize;
                    iOutBufferCount = iParamPort.nBufferCountActual;

                    iParamPort.nBufferCountActual = iOutBufferCount;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxDecTestIncompleteNALTest::Run() - GetParameter returned Num of output buffers %d with Size %d", iOutBufferCount, iOutBufferSize));
                }

                //Take the buffer parameters of what component has specified
                iParamPort.nPortIndex = PortIndex;
                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamPortDefinition, &iParamPort);
                CHECK_ERROR(Err, "SetParameter_OMX_IndexParamPortDefinition");
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxDecTestIncompleteNALTest::Run() - SetParameter called for OMX_IndexParamPortDefinition on port %d", PortIndex));
            }

            if (StateError == iState)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "OmxDecTestIncompleteNALTest::Run() - Error, Exiting the test case, OUT"));
                RunIfNotReady();
                break;
            }

#if PROXY_INTERFACE
            ipThreadSafeHandlerEventHandler = OSCL_NEW(EventHandlerThreadSafeCallbackAO, (this, EVENT_HANDLER_QUEUE_DEPTH, "EventHandlerAO"));
            ipThreadSafeHandlerEmptyBufferDone = OSCL_NEW(EmptyBufferDoneThreadSafeCallbackAO, (this, iInBufferCount, "EmptyBufferDoneAO"));
            ipThreadSafeHandlerFillBufferDone = OSCL_NEW(FillBufferDoneThreadSafeCallbackAO, (this, iOutBufferCount, "FillBufferDoneAO"));

            if ((NULL == ipThreadSafeHandlerEventHandler) ||
                    (NULL == ipThreadSafeHandlerEmptyBufferDone) ||
                    (NULL == ipThreadSafeHandlerFillBufferDone))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "OmxDecTestIncompleteNALTest::Run() - Error, ThreadSafe Callback Handler initialization failed, OUT"));

                iState = StateUnLoaded;
                OsclExecScheduler* sched = OsclExecScheduler::Current();
                sched->StopScheduler();
            }
#endif

            if (StateError != iState)
            {
                iState = StateLoaded;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestIncompleteNALTest::Run() - StateUnLoaded OUT, moving to next state"));
            }

            RunIfNotReady();
        }
        break;

        case StateLoaded:
        {
            OMX_ERRORTYPE Err;
            OMX_S32 ii;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestIncompleteNALTest::Run() - StateLoaded IN"));

            // allocate memory for ipInBuffer
            ipInBuffer = (OMX_BUFFERHEADERTYPE**) oscl_malloc(sizeof(OMX_BUFFERHEADERTYPE*) * iInBufferCount);
            CHECK_MEM(ipInBuffer, "InputBufferHeader");

            ipInputAvail = (OMX_BOOL*) oscl_malloc(sizeof(OMX_BOOL) * iInBufferCount);
            CHECK_MEM(ipInputAvail, "InputBufferFlag");

            /* Initialize all the buffers to NULL */
            for (ii = 0; ii < iInBufferCount; ii++)
            {
                ipInBuffer[ii] = NULL;
            }

            //allocate memory for output buffer
            ipOutBuffer = (OMX_BUFFERHEADERTYPE**) oscl_malloc(sizeof(OMX_BUFFERHEADERTYPE*) * iOutBufferCount);
            CHECK_MEM(ipOutBuffer, "OutputBuffer");

            ipOutReleased = (OMX_BOOL*) oscl_malloc(sizeof(OMX_BOOL) * iOutBufferCount);
            CHECK_MEM(ipOutReleased, "OutputBufferFlag");

            /* Initialize all the buffers to NULL */
            for (ii = 0; ii < iOutBufferCount; ii++)
            {
                ipOutBuffer[ii] = NULL;
            }

            Err = OMX_SendCommand(ipAppPriv->Handle, OMX_CommandStateSet, OMX_StateIdle, NULL);
            CHECK_ERROR(Err, "SendCommand Loaded->Idle");

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxDecTestIncompleteNALTest::Run() - Sent State Transition Command from Loaded->Idle"));

            iPendingCommands = 1;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxDecTestIncompleteNALTest::Run() - Allocating %d input and %d output buffers", iInBufferCount, iOutBufferCount));

            //These calls are required because the control of in & out buffer should be with the testapp.
            for (ii = 0; ii < iInBufferCount; ii++)
            {
                Err = OMX_AllocateBuffer(ipAppPriv->Handle, &ipInBuffer[ii], iInputPortIndex, NULL, iInBufferSize);
                CHECK_ERROR(Err, "AllocateBuffer_Input");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxDecTestIncompleteNALTest::Run() - Called AllocateBuffer for buffer index %d on port %d", ii, iInputPortIndex));

                ipInputAvail[ii] = OMX_TRUE;
                ipInBuffer[ii]->nInputPortIndex = iInputPortIndex;
            }

            if (StateError == iState)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "OmxDecTestIncompleteNALTest::Run() - AllocateBuffer Error, StateLoaded OUT"));
                RunIfNotReady();
                break;
            }

            for (ii = 0; ii < iOutBufferCount; ii++)
            {
                Err = OMX_AllocateBuffer(ipAppPriv->Handle, &ipOutBuffer[ii], iOutputPortIndex, NULL, iOutBufferSize);
                CHECK_ERROR(Err, "AllocateBuffer_Output");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxDecTestIncompleteNALTest::Run() - Called AllocateBuffer for buffer index %d on port %d", ii, iOutputPortIndex));

                ipOutReleased[ii] = OMX_TRUE;

                ipOutBuffer[ii]->nOutputPortIndex = iOutputPortIndex;
                ipOutBuffer[ii]->nInputPortIndex = 0;
            }
            if (StateError == iState)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "OmxDecTestIncompleteNALTest::Run() - AllocateBuffer Error, StateLoaded OUT"));
                RunIfNotReady();
                break;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestIncompleteNALTest::Run() - StateLoaded OUT, Moving to next state"));

        }
        break;

        case StateIdle:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestIncompleteNALTest::Run() - StateIdle IN"));

            OMX_ERRORTYPE Err = OMX_ErrorNone;

            /*Send an output buffer before dynamic reconfig */
            Err = OMX_FillThisBuffer(ipAppPriv->Handle, ipOutBuffer[0]);
            CHECK_ERROR(Err, "FillThisBuffer");

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxDecTestIncompleteNALTest::Run() - FillThisBuffer command called for initiating dynamic port reconfiguration"));

            ipOutReleased[0] = OMX_FALSE;

            Err = OMX_SendCommand(ipAppPriv->Handle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
            CHECK_ERROR(Err, "SendCommand Idle->Executing");

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxDecTestIncompleteNALTest::Run() - Sent State Transition Command from Idle->Executing"));

            iPendingCommands = 1;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestIncompleteNALTest::Run() - StateIdle OUT"));
        }
        break;

        case StateDecodeHeader:
        {
            static OMX_BOOL FlagTemp = OMX_FALSE;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "OmxDecTestIncompleteNALTest::Run() - StateDecodeHeader IN, Sending configuration input buffers to the component to start dynamic port reconfiguration"));

            if (!FlagTemp)
            {
                GetInputFrameAvc();
                FlagTemp = OMX_TRUE;
                //Proceed to executing state and if Port settings changed callback comes,
                //then do the dynamic port reconfiguration
                iState = StateExecuting;

                RunIfNotReady();
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestIncompleteNALTest::Run() - StateDecodeHeader OUT"));
        }
        break;

        case StateDisablePort:
        {
            static OMX_BOOL FlagTemp = OMX_FALSE;
            OMX_ERRORTYPE Err = OMX_ErrorNone;
            OMX_S32 ii;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestIncompleteNALTest::Run() - StateDisablePort IN"));

            if (!DisableRun)
            {
                if (!FlagTemp)
                {
                    Err = OMX_SendCommand(ipAppPriv->Handle, OMX_CommandPortDisable, iOutputPortIndex, NULL);
                    CHECK_ERROR(Err, "SendCommand_PortDisable");

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxDecTestIncompleteNALTest::Run() - Sent Command for OMX_CommandPortDisable on port %d as a part of dynamic port reconfiguration", iOutputPortIndex));

                    iPendingCommands = 1;
                    FlagTemp = OMX_TRUE;
                    RunIfNotReady();
                }
                else
                {
                    //Wait for all the buffers to be returned on output port before freeing them
                    //This wait is required because of the queueing delay in all the Callbacks
                    for (ii = 0; ii < iOutBufferCount; ii++)
                    {
                        if (OMX_FALSE == ipOutReleased[ii])
                        {
                            break;
                        }
                    }

                    if (ii != iOutBufferCount)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                        (0, "OmxDecTestIncompleteNALTest::Run() - Not all the output buffers returned by component yet, wait for it"));
                        RunIfNotReady();
                        break;
                    }

                    for (ii = 0; ii < iOutBufferCount; ii++)
                    {
                        if (ipOutBuffer[ii])
                        {
                            Err = OMX_FreeBuffer(ipAppPriv->Handle, iOutputPortIndex, ipOutBuffer[ii]);
                            CHECK_ERROR(Err, "FreeBuffer_Output_DynamicReconfig");
                            ipOutBuffer[ii] = NULL;

                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                            (0, "OmxDecTestIncompleteNALTest::Run() - Called FreeBuffer for buffer index %d on port %d", ii, iOutputPortIndex));
                        }
                    }

                    if (StateError == iState)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "OmxDecTestIncompleteNALTest::Run() - Error occured in this state, StateDisablePort OUT"));
                        RunIfNotReady();
                        break;
                    }
                    DisableRun = OMX_TRUE;
                }
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestIncompleteNALTest::Run() - StateDisablePort OUT"));
        }
        break;

        case StateDynamicReconfig:
        {
            OMX_ERRORTYPE Err = OMX_ErrorNone;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestIncompleteNALTest::Run() - StateDynamicReconfig IN"));

            INIT_GETPARAMETER_STRUCT(OMX_PARAM_PORTDEFINITIONTYPE, iParamPort);
            iParamPort.nPortIndex = iOutputPortIndex;
            Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamPortDefinition, &iParamPort);
            CHECK_ERROR(Err, "GetParameter_DynamicReconfig");

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestIncompleteNALTest::Run() - GetParameter called for OMX_IndexParamPortDefinition on port %d", iParamPort.nPortIndex));
            Err = OMX_SendCommand(ipAppPriv->Handle, OMX_CommandPortEnable, iOutputPortIndex, NULL);

            CHECK_ERROR(Err, "SendCommand_PortEnable");

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxDecTestIncompleteNALTest::Run() - Sent Command for OMX_CommandPortEnable on port %d as a part of dynamic port reconfiguration", iOutputPortIndex));

            iPendingCommands = 1;

            iOutBufferSize = ((iParamPort.format.video.nFrameWidth + 15) & ~15) * ((iParamPort.format.video.nFrameHeight + 15) & ~15) * 3 / 2;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxDecTestIncompleteNALTest::Run() - Allocating buffer again after port reconfigutauion has been complete"));

            for (OMX_S32 ii = 0; ii < iOutBufferCount; ii++)
            {
                Err = OMX_AllocateBuffer(ipAppPriv->Handle, &ipOutBuffer[ii], iOutputPortIndex, NULL, iOutBufferSize);
                CHECK_ERROR(Err, "AllocateBuffer_Output_DynamicReconfig");
                ipOutReleased[ii] = OMX_TRUE;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxDecTestIncompleteNALTest::Run() - AllocateBuffer called for buffer index %d on port %d", ii, iOutputPortIndex));
            }

            if (StateError == iState)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "OmxDecTestIncompleteNALTest::Run() - Error occured in this state, StateDynamicReconfig OUT"));
                RunIfNotReady();
                break;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestIncompleteNALTest::Run() - StateDynamicReconfig OUT"));
        }
        break;

        case StateExecuting:
        {
            static OMX_BOOL EosFlag = OMX_FALSE;
            static OMX_ERRORTYPE Status;
            OMX_S32 Index;
            OMX_BOOL MoreOutput;
            OMX_ERRORTYPE Err = OMX_ErrorNone;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestIncompleteNALTest::Run() - StateExecuting IN"));

            MoreOutput = OMX_TRUE;
            while (MoreOutput)
            {
                Index = 0;
                while (OMX_FALSE == ipOutReleased[Index] && Index < iOutBufferCount)
                {
                    Index++;
                }

                if (Index != iOutBufferCount)
                {
                    //This call is being made only once per frame
                    Err = OMX_FillThisBuffer(ipAppPriv->Handle, ipOutBuffer[Index]);
                    CHECK_ERROR(Err, "FillThisBuffer");
                    //Make this flag OMX_TRUE till u receive the callback for output buffer free
                    ipOutReleased[Index] = OMX_FALSE;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxDecTestIncompleteNALTest::Run() - FillThisBuffer command called for output buffer index %d", Index));
                }
                else
                {
                    MoreOutput = OMX_FALSE;
                }
            }


            if (!iStopProcessingInput || (OMX_ErrorInsufficientResources == Status))
            {
                // find available input buffer
                Index = 0;
                while (OMX_FALSE == ipInputAvail[Index] && Index < iInBufferCount)
                {
                    Index++;
                }

                if (Index != iInBufferCount)
                {
                    Status = GetInputFrameAvc();
                }
            }
            else if (OMX_FALSE == EosFlag)
            {
                //Only send one successful dummy buffer with flag set to signal EOS
                Index = 0;
                while (OMX_FALSE == ipInputAvail[Index] && Index < iInBufferCount)
                {
                    Index++;
                }

                if (Index != iInBufferCount)
                {
                    ipInBuffer[Index]->nFlags |= OMX_BUFFERFLAG_EOS;
                    ipInBuffer[Index]->nFilledLen = 0;
                    Err = OMX_EmptyThisBuffer(ipAppPriv->Handle, ipInBuffer[Index]);

                    CHECK_ERROR(Err, "EmptyThisBuffer_EOS");

                    ipInputAvail[Index] = OMX_FALSE; // mark unavailable
                    EosFlag = OMX_TRUE;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxDecTestIncompleteNALTest::Run() - Input buffer sent to the component with OMX_BUFFERFLAG_EOS flag set"));
                }
            }
            else
            {
                //nothing to do here
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestIncompleteNALTest::Run() - StateExecuting OUT"));
            RunIfNotReady();
        }
        break;

        case StateStopping:
        {
            static OMX_BOOL FlagTemp = OMX_FALSE;
            OMX_ERRORTYPE Err = OMX_ErrorNone;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestIncompleteNALTest::Run() - StateStopping IN"));

            //stop execution by state transition to Idle state.
            if (!FlagTemp)
            {
                Err = OMX_SendCommand(ipAppPriv->Handle, OMX_CommandStateSet, OMX_StateIdle, NULL);
                CHECK_ERROR(Err, "SendCommand Executing->Idle");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxDecTestIncompleteNALTest::Run() - Sent State Transition Command from Executing->Idle"));

                iPendingCommands = 1;
                FlagTemp = OMX_TRUE;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestIncompleteNALTest::Run() - StateStopping OUT"));
        }
        break;

        case StateCleanUp:
        {
            OMX_S32 ii;
            OMX_ERRORTYPE Err = OMX_ErrorNone;
            static OMX_BOOL FlagTemp = OMX_FALSE;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestIncompleteNALTest::Run() - StateCleanUp IN"));


            if (!FlagTemp)
            {
                //Added a check here to verify whether all the ip/op buffers are returned back by the component or not
                //in case of Executing->Idle state transition

                if (OMX_FALSE == VerifyAllBuffersReturned())
                {
                    // not all buffers have been returned yet, reschedule
                    RunIfNotReady();
                    break;
                }

                //Destroy the component by state transition to Loaded state
                Err = OMX_SendCommand(ipAppPriv->Handle, OMX_CommandStateSet, OMX_StateLoaded, NULL);
                CHECK_ERROR(Err, "SendCommand Idle->Loaded");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxDecTestIncompleteNALTest::Run() - Sent State Transition Command from Idle->Loaded"));

                iPendingCommands = 1;

                if (ipInBuffer)
                {
                    for (ii = 0; ii < iInBufferCount; ii++)
                    {
                        if (ipInBuffer[ii])
                        {
                            Err = OMX_FreeBuffer(ipAppPriv->Handle, iInputPortIndex, ipInBuffer[ii]);
                            CHECK_ERROR(Err, "FreeBuffer_Input");
                            ipInBuffer[ii] = NULL;

                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                            (0, "OmxDecTestIncompleteNALTest::Run() - Called FreeBuffer for buffer index %d on port %d", ii, iInputPortIndex));
                        }
                    }

                    oscl_free(ipInBuffer);
                    ipInBuffer = NULL;
                }

                if (ipInputAvail)
                {
                    oscl_free(ipInputAvail);
                    ipInputAvail = NULL;
                }


                if (ipOutBuffer)
                {
                    for (ii = 0; ii < iOutBufferCount; ii++)
                    {
                        if (ipOutBuffer[ii])
                        {
                            Err = OMX_FreeBuffer(ipAppPriv->Handle, iOutputPortIndex, ipOutBuffer[ii]);
                            CHECK_ERROR(Err, "FreeBuffer_Output");
                            ipOutBuffer[ii] = NULL;

                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                            (0, "OmxDecTestIncompleteNALTest::Run() - Called FreeBuffer for buffer index %d on port %d", ii, iOutputPortIndex));
                        }
                    }
                    oscl_free(ipOutBuffer);
                    ipOutBuffer = NULL;

                }

                if (ipOutReleased)
                {
                    oscl_free(ipOutReleased);
                    ipOutReleased = NULL;
                }

                FlagTemp = OMX_TRUE;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestIncompleteNALTest::Run() - StateCleanUp OUT"));

        }
        break;


        /********* FREE THE HANDLE & CLOSE FILES FOR THE COMPONENT ********/
        case StateStop:
        {
            OMX_U8 TestName[] = "INCOMPLETE_NAL_TEST";
            OMX_ERRORTYPE Err = OMX_ErrorNone;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestIncompleteNALTest::Run() - StateStop IN"));

            if (ipAppPriv)
            {
                if (ipAppPriv->Handle)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxDecTestIncompleteNALTest::Run() - Free the Component Handle"));


                    Err = OMX_FreeHandle(ipAppPriv->Handle);
                    if (OMX_ErrorNone != Err)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestIncompleteNALTest::Run() - FreeHandle Error"));
                        iTestStatus = OMX_FALSE;
                    }
                }
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxDecTestIncompleteNALTest::Run() - De-initialize the omx component"));

            Err = OMX_Deinit();
            if (OMX_ErrorNone != Err)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestIncompleteNALTest::Run() - OMX_Deinit Error"));
                iTestStatus = OMX_FALSE;
            }

            if (ipAVCBSO)
            {
                OSCL_DELETE(ipAVCBSO);
                ipAVCBSO = NULL;
            }

            if (ipAppPriv)
            {
                oscl_free(ipAppPriv);
                ipAppPriv = NULL;
            }

#if PROXY_INTERFACE
            OSCL_DELETE(ipThreadSafeHandlerEventHandler);
            ipThreadSafeHandlerEventHandler = NULL;

            OSCL_DELETE(ipThreadSafeHandlerEmptyBufferDone);
            ipThreadSafeHandlerEmptyBufferDone = NULL;

            OSCL_DELETE(ipThreadSafeHandlerFillBufferDone);
            ipThreadSafeHandlerFillBufferDone = NULL;
#endif

            if (OMX_FALSE == iTestStatus)
            {
#ifdef PRINT_RESULT
                fprintf(iConsOutFile, "%s: Fail \n", TestName);
#endif

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                                (0, "OmxDecTestIncompleteNALTest::Run() - %s : Fail", TestName));
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                                (0, "OmxDecTestIncompleteNALTest::Run() - Total frames %d, Incomplete frames %d, Written Frame %d", iFrameNum, iIncompleteFrames, iFramesWritten));

#ifdef PRINT_RESULT
                fprintf(iConsOutFile, "%s: Success \n", TestName);
#endif

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                                (0, "OmxDecTestIncompleteNALTest::Run() - %s : Success", TestName));
            }

            iState = StateUnLoaded;
            OsclExecScheduler* sched = OsclExecScheduler::Current();
            sched->StopScheduler();
        }
        break;

        case StateError:
        {
            //Do all the cleanup's and exit from here
            OMX_S32 ii;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestIncompleteNALTest::Run() - StateError IN"));

            iTestStatus = OMX_FALSE;

            if (ipInBuffer)
            {
                for (ii = 0; ii < iInBufferCount; ii++)
                {
                    if (ipInBuffer[ii])
                    {
                        OMX_FreeBuffer(ipAppPriv->Handle, iInputPortIndex, ipInBuffer[ii]);
                        ipInBuffer[ii] = NULL;

                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                        (0, "OmxDecTestIncompleteNALTest::Run() - Called FreeBuffer for buffer index %d on port %d", ii, iInputPortIndex));
                    }
                }
                oscl_free(ipInBuffer);
                ipInBuffer = NULL;
            }

            if (ipInputAvail)
            {
                oscl_free(ipInputAvail);
                ipInputAvail = NULL;
            }

            if (ipOutBuffer)
            {
                for (ii = 0; ii < iOutBufferCount; ii++)
                {
                    if (ipOutBuffer[ii])
                    {
                        OMX_FreeBuffer(ipAppPriv->Handle, iOutputPortIndex, ipOutBuffer[ii]);
                        ipOutBuffer[ii] = NULL;

                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                        (0, "OmxDecTestIncompleteNALTest::Run() - Called FreeBuffer for buffer index %d on port %d", ii, iOutputPortIndex));
                    }
                }
                oscl_free(ipOutBuffer);
                ipOutBuffer = NULL;
            }

            if (ipOutReleased)
            {
                oscl_free(ipOutReleased);
                ipOutReleased = NULL;
            }

            iState = StateStop;
            RunIfNotReady();

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestIncompleteNALTest::Run() - StateError OUT"));

        }
        break;

        default:
        {
            break;
        }
    }
    return ;
}


