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

#define TEST_NUM_BUFFERS_TO_PROCESS 10

/*
 * Active Object class's Run () function
 * Control all the states of AO & sends openmax API's to the component
 */
static OMX_BOOL DisableRun = OMX_FALSE;

void OmxDecTestPauseResume::Run()
{
    switch (iState)
    {
        case StateUnLoaded:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestPauseResume::Run() - StateUnLoaded IN"));
            OMX_ERRORTYPE Err;
            OMX_U32 PortIndex, ii;

            if (!iCallbacks->initCallbacks())
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestPauseResume::Run() - ERROR initCallbacks failed, OUT"));
                StopOnError();
                break;
            }

            ipAppPriv = (AppPrivateType*) oscl_malloc(sizeof(AppPrivateType));
            CHECK_MEM(ipAppPriv, "Component_Handle");

            //Allocate bitstream buffer for AVC component
            if (0 == oscl_strcmp(iFormat, "H264"))
            {
                ipAVCBSO = OSCL_NEW(AVCBitstreamObject, (ipInputFile));
                CHECK_MEM(ipAVCBSO, "Bitstream_Buffer");
            }

            //Allocate bitstream buffer for MPEG4/H263 component
            if (0 == oscl_strcmp(iFormat, "M4V") || 0 == oscl_strcmp(iFormat, "H263"))
            {
                ipBitstreamBuffer = (OMX_U8*) oscl_malloc(BIT_BUFF_SIZE);
                CHECK_MEM(ipBitstreamBuffer, "Bitstream_Buffer")
                ipBitstreamBufferPointer = ipBitstreamBuffer;
            }

            //Allocate bitstream buffer for MP3 component
            if (0 == oscl_strcmp(iFormat, "MP3"))
            {
                ipMp3Bitstream = OSCL_NEW(Mp3BitstreamObject, (ipInputFile));
                CHECK_MEM(ipMp3Bitstream, "Bitstream_Buffer");
            }

            //This should be the first call to the component to load it.
            Err = OMX_Init();
            CHECK_ERROR(Err, "OMX_Init");
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestPauseResume::Run() - OMX_Init done"));

            if (NULL != iName)
            {
                Err = OMX_GetHandle(&ipAppPriv->Handle, iName, (OMX_PTR) this , iCallbacks->getCallbackStruct());
                CHECK_ERROR(Err, "GetHandle");
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestPauseResume::Run() - Got Handle for the component %s", iName));
            }
            else if (NULL != iRole)
            {
                //Determine the component first & then get the handle
                OMX_U32 NumComps = 0;
                OMX_STRING* pCompOfRole;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestPauseResume::Run() - Finding out the role for the component %s", iRole));
                // call once to find out the number of components that can fit the role
                Err = OMX_GetComponentsOfRole(iRole, &NumComps, NULL);

                if (OMX_ErrorNone != Err || NumComps < 1)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestPauseResume::Run() - ERROR, No component can handle the specified role %s", iRole));
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
                                    (0, "OmxDecTestPauseResume::Run() - Error occured in this state, StateUnLoaded OUT"));
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
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestPauseResume::Run() - Got Handle for the component %s", pCompOfRole[ii]));
                        break;
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestPauseResume::Run() - ERROR, Cannot get component %s handle, try another if possible", pCompOfRole[ii]));
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

            if (0 == oscl_strcmp(iFormat, "AAC") || 0 == oscl_strcmp(iFormat, "AMR")
                    || 0 == oscl_strcmp(iFormat, "MP3") || 0 == oscl_strcmp(iFormat, "WMA"))
            {
                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamAudioInit, &iPortInit);
            }
            else
            {
                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamVideoInit, &iPortInit);
            }

            CHECK_ERROR(Err, "GetParameter_Audio/Video_Init");

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestPauseResume::Run() - GetParameter called for OMX_IndexParamAudioInit/OMX_IndexParamVideoInit"));
            for (ii = 0; ii < iPortInit.nPorts; ii++)
            {
                PortIndex = iPortInit.nStartPortNumber + ii;

                //This will initialize the size and version of the iParamPort structure
                INIT_GETPARAMETER_STRUCT(OMX_PARAM_PORTDEFINITIONTYPE, iParamPort);
                iParamPort.nPortIndex = PortIndex;

                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamPortDefinition, &iParamPort);
                CHECK_ERROR(Err, "GetParameter_IndexParamPortDefinition");
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestPauseResume::Run() - GetParameter called for OMX_IndexParamPortDefinition on port %d", PortIndex));

                if (0 == iParamPort.nBufferCountMin)
                {
                    /* a buffer count of 0 is not allowed */
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestPauseResume::Run() - Error, GetParameter for OMX_IndexParamPortDefinition returned 0 min buffer count"));
                    StopOnError();
                    break;
                }

                if (iParamPort.nBufferCountMin > iParamPort.nBufferCountActual)
                {
                    /* Min buff count can't be more than actual buff count */
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "OmxDecTestPauseResume::Run() - ERROR, GetParameter for OMX_IndexParamPortDefinition returned actual buffer count %d less than min buffer count %d", iParamPort.nBufferCountActual, iParamPort.nBufferCountMin));
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
                                    (0, "OmxDecTestPauseResume::Run() - GetParameter returned Num of input buffers %d with Size %d", iInBufferCount, iInBufferSize));
                }
                else if (OMX_DirOutput == iParamPort.eDir)
                {
                    iOutputPortIndex = PortIndex;

                    iOutBufferSize = iParamPort.nBufferSize;
                    iOutBufferCount = iParamPort.nBufferCountActual;

                    iParamPort.nBufferCountActual = iOutBufferCount;
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxDecTestPauseResume::Run() - GetParameter returned Num of output buffers %d with Size %d", iOutBufferCount, iOutBufferSize));
                }

                //Take the buffer parameters of what component has specified
                iParamPort.nPortIndex = PortIndex;
                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamPortDefinition, &iParamPort);
                CHECK_ERROR(Err, "SetParameter_OMX_IndexParamPortDefinition");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxDecTestPauseResume::Run() - SetParameter called for OMX_IndexParamPortDefinition on port %d", PortIndex));
            }

            if (StateError == iState)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "OmxDecTestPauseResume::Run() - Error, Exiting the test case, OUT"));
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
                                (0, "OmxDecTestPauseResume::Run() - Error, ThreadSafe Callback Handler initialization failed, OUT"));

                iState = StateUnLoaded;
                OsclExecScheduler* sched = OsclExecScheduler::Current();
                sched->StopScheduler();
            }
#endif
            //Decide about the getinput call for the correct compoent

            if (0 == oscl_strcmp(iFormat, "H264"))
            {
                pGetInputFrame = &OmxComponentDecTest::GetInputFrameAvc;
            }
            else if (0 == oscl_strcmp(iFormat, "M4V"))
            {
                pGetInputFrame = &OmxComponentDecTest::GetInputFrameMpeg4;
            }
            else if (0 == oscl_strcmp(iFormat, "H263"))
            {
                pGetInputFrame = &OmxComponentDecTest::GetInputFrameH263;
            }
            else if (0 == oscl_strcmp(iFormat, "AAC"))
            {
                pGetInputFrame = &OmxComponentDecTest::GetInputFrameAac;

                INIT_GETPARAMETER_STRUCT(OMX_AUDIO_PARAM_PCMMODETYPE, iPcmMode);
                iPcmMode.nPortIndex = iOutputPortIndex;

                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamAudioPcm, &iPcmMode);
                CHECK_ERROR(Err, "GetParameter_AAC_IndexParamAudioPcm");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxDecTestPauseResume::Run() - GetParameter called for OMX_IndexParamAudioPcm for AAC on port %d", iOutputPortIndex));

                iPcmMode.nPortIndex = iOutputPortIndex;
                /* Pass the number of channel information for AAC component
                 * from input arguments to the component */
                iPcmMode.nChannels = iNumberOfChannels;

                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamAudioPcm, &iPcmMode);
                CHECK_ERROR(Err, "SetParameter_AAC_IndexParamAudioPcm");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxDecTestPauseResume::Run() - SetParameter called for OMX_IndexParamAudioPcm for AAC on port %d", iOutputPortIndex));
            }
            else if (0 == oscl_strcmp(iFormat, "WMV"))
            {
                pGetInputFrame = &OmxComponentDecTest::GetInputFrameWmv;
            }
            else if (0 == oscl_strcmp(iFormat, "AMR"))
            {
                /* Determine the file format type for AMR file */

                OMX_S32 AmrFileByte = 0;

                fread(&AmrFileByte, 1, 4, ipInputFile); // read in 1st 4 bytes

                if (2 == AmrFileByte)
                {
                    iAmrFileType = OMX_AUDIO_AMRFrameFormatFSF;
                }
                else if (4 == AmrFileByte)
                {
                    iAmrFileType = OMX_AUDIO_AMRFrameFormatIF2;
                }
                else if (6 == AmrFileByte)
                {
                    iAmrFileType = OMX_AUDIO_AMRFrameFormatRTPPayload;
                }
                else if (0 == AmrFileByte)
                {
                    iAmrFileType = OMX_AUDIO_AMRFrameFormatConformance;
                }
                else if (7 == AmrFileByte)
                {
                    iAmrFileType = OMX_AUDIO_AMRFrameFormatRTPPayload;
                    iAmrFileMode = OMX_AUDIO_AMRBandModeWB0;

                }
                else if (8 == AmrFileByte)
                {
                    iAmrFileType = OMX_AUDIO_AMRFrameFormatFSF;
                    iAmrFileMode = OMX_AUDIO_AMRBandModeWB0;
                }
                else
                {
                    /* Invalid input format type */
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "OmxDecTestPauseResume::Run() - Error, Invalid AMR input format %d, OUT", AmrFileByte));
                    StopOnError();
                }


                pGetInputFrame = &OmxComponentDecTest::GetInputFrameAmr;

                /* Pass the input format type information to the AMR component*/

                INIT_GETPARAMETER_STRUCT(OMX_AUDIO_PARAM_AMRTYPE, iAmrParam);
                iAmrParam.nPortIndex = iInputPortIndex;

                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamAudioAmr, &iAmrParam);
                CHECK_ERROR(Err, "GetParameter_AMR_IndexParamAudioAmr");


                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxDecTestPauseResume::Run() - GetParameter called for OMX_IndexParamAudioAmr for AMR on port %d", iInputPortIndex));

                iAmrParam.nPortIndex = iInputPortIndex;
                iAmrParam.eAMRFrameFormat = iAmrFileType;
                iAmrParam.eAMRBandMode = iAmrFileMode;

                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamAudioAmr, &iAmrParam);
                CHECK_ERROR(Err, "SetParameter_AMR_IndexParamAudioAmr");


                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxDecTestPauseResume::Run() - SetParameter called for OMX_IndexParamAudioAmr for AMR on port %d", iInputPortIndex));
            }
            else if (0 == oscl_strcmp(iFormat, "MP3"))
            {
                pGetInputFrame = &OmxComponentDecTest::GetInputFrameMp3;

                INIT_GETPARAMETER_STRUCT(OMX_AUDIO_PARAM_PCMMODETYPE, iPcmMode);
                iPcmMode.nPortIndex = iOutputPortIndex;

                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamAudioPcm, &iPcmMode);
                CHECK_ERROR(Err, "GetParameter_MP3_IndexParamAudioPcm");


                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxDecTestPauseResume::Run() - GetParameter called for OMX_IndexParamAudioPcm for MP3 on port %d", iOutputPortIndex));


                iPcmMode.nPortIndex = iOutputPortIndex;
                /* Pass the number of channel information from input arguments to the component */
                iPcmMode.nChannels = iNumberOfChannels;

                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamAudioPcm, &iPcmMode);
                CHECK_ERROR(Err, "SetParameter_MP3_IndexParamAudioPcm");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxDecTestPauseResume::Run() - SetParameter called for OMX_IndexParamAudioPcm for MP3 on port %d", iOutputPortIndex));
            }
            else if (0 == oscl_strcmp(iFormat, "WMA"))
            {
                pGetInputFrame = &OmxComponentDecTest::GetInputFrameWma;
            }


            if (StateError != iState)
            {
                iState = StateLoaded;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestPauseResume::Run() - StateUnLoaded OUT, moving to next state"));
            }

            RunIfNotReady();
        }
        break;

        case StateLoaded:
        {
            OMX_ERRORTYPE Err;
            OMX_S32 ii;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestPauseResume::Run() - StateLoaded IN"));
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
                            (0, "OmxDecTestPauseResume::Run() - Sent State Transition Command from Loaded->Idle"));
            iPendingCommands = 1;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxDecTestPauseResume::Run() - Allocating %d input and %d output buffers", iInBufferCount, iOutBufferCount));
            //These calls are required because the control of in & out buffer should be with the testapp.
            for (ii = 0; ii < iInBufferCount; ii++)
            {
                Err = OMX_AllocateBuffer(ipAppPriv->Handle, &ipInBuffer[ii], iInputPortIndex, NULL, iInBufferSize);
                CHECK_ERROR(Err, "AllocateBuffer_Input");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxDecTestPauseResume::Run() - Called AllocateBuffer for buffer index %d on port %d", ii, iInputPortIndex));
                ipInputAvail[ii] = OMX_TRUE;
                ipInBuffer[ii]->nInputPortIndex = iInputPortIndex;
            }

            if (StateError == iState)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "OmxDecTestPauseResume::Run() - AllocateBuffer Error, StateLoaded OUT"));
                RunIfNotReady();
                break;
            }

            for (ii = 0; ii < iOutBufferCount; ii++)
            {
                Err = OMX_AllocateBuffer(ipAppPriv->Handle, &ipOutBuffer[ii], iOutputPortIndex, NULL, iOutBufferSize);
                CHECK_ERROR(Err, "AllocateBuffer_Output");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxDecTestPauseResume::Run() - Called AllocateBuffer for buffer index %d on port %d", ii, iOutputPortIndex));
                ipOutReleased[ii] = OMX_TRUE;

                ipOutBuffer[ii]->nOutputPortIndex = iOutputPortIndex;
                ipOutBuffer[ii]->nInputPortIndex = 0;
            }
            if (StateError == iState)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "OmxDecTestPauseResume::Run() - AllocateBuffer Error, StateLoaded OUT"));
                RunIfNotReady();
                break;
            }
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestPauseResume::Run() - StateLoaded OUT, Moving to next state"));
        }
        break;

        case StateIdle:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestPauseResume::Run() - StateIdle IN"));
            OMX_ERRORTYPE Err = OMX_ErrorNone;

            Err = OMX_FillThisBuffer(ipAppPriv->Handle, ipOutBuffer[0]);
            CHECK_ERROR(Err, "FillThisBuffer");

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxDecTestPauseResume::Run() - FillThisBuffer command called for initiating dynamic port reconfiguration"));

            ipOutReleased[0] = OMX_FALSE;

            Err = OMX_SendCommand(ipAppPriv->Handle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
            CHECK_ERROR(Err, "SendCommand Idle->Executing");

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxDecTestPauseResume::Run() - Sent State Transition Command from Idle->Executing"));
            iPendingCommands = 1;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestPauseResume::Run() - StateIdle OUT"));
        }
        break;

        case StateDecodeHeader:
        {
            static OMX_BOOL FlagTemp = OMX_FALSE;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "OmxDecTestPauseResume::Run() - StateDecodeHeader IN, Sending configuration input buffers to the component to start dynamic port reconfiguration"));

            if (!FlagTemp)
            {
                (*this.*pGetInputFrame)();
                //For AAC component , send one more frame apart from the config frame, so that we can receive the callback
                if (0 == oscl_strcmp(iFormat, "AAC") || 0 == oscl_strcmp(iFormat, "AMR"))
                {
                    (*this.*pGetInputFrame)();
                }
                FlagTemp = OMX_TRUE;
                iFrameCount++;

                //Proceed to executing state and if Port settings changed callback comes,
                //then do the dynamic port reconfiguration
                iState = StateExecuting;

                RunIfNotReady();
            }
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestPauseResume::Run() - StateDecodeHeader OUT"));
        }
        break;

        case StateDisablePort:
        {
            static OMX_BOOL FlagTemp = OMX_FALSE;
            OMX_ERRORTYPE Err = OMX_ErrorNone;
            OMX_S32 ii;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestPauseResume::Run() - StateDisablePort IN"));

            if (!DisableRun)
            {
                if (!FlagTemp)
                {
                    Err = OMX_SendCommand(ipAppPriv->Handle, OMX_CommandPortDisable, iOutputPortIndex, NULL);
                    CHECK_ERROR(Err, "SendCommand_PortDisable");

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxDecTestPauseResume::Run() - Sent Command for OMX_CommandPortDisable on port %d as a part of dynamic port reconfiguration", iOutputPortIndex));

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
                                        (0, "OmxDecTestPauseResume::Run() - Not all the output buffers returned by component yet, wait for it"));
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
                                            (0, "OmxDecTestPauseResume::Run() - Called FreeBuffer for buffer index %d on port %d", ii, iOutputPortIndex));
                        }
                    }

                    if (StateError == iState)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "OmxDecTestPauseResume::Run() - Error occured in this state, StateDisablePort OUT"));
                        RunIfNotReady();
                        break;
                    }
                    DisableRun = OMX_TRUE;
                }
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestPauseResume::Run() - StateDisablePort OUT"));
        }
        break;

        case StateDynamicReconfig:
        {
            OMX_ERRORTYPE Err = OMX_ErrorNone;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestPauseResume::Run() - StateDynamicReconfig IN"));
            INIT_GETPARAMETER_STRUCT(OMX_PARAM_PORTDEFINITIONTYPE, iParamPort);
            iParamPort.nPortIndex = iOutputPortIndex;

            Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamPortDefinition, &iParamPort);
            CHECK_ERROR(Err, "GetParameter_DynamicReconfig");

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestPauseResume::Run() - GetParameter called for OMX_IndexParamPortDefinition on port %d", iParamPort.nPortIndex));

            Err = OMX_SendCommand(ipAppPriv->Handle, OMX_CommandPortEnable, iOutputPortIndex, NULL);
            CHECK_ERROR(Err, "SendCommand_PortEnable");

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxDecTestPauseResume::Run() - Sent Command for OMX_CommandPortEnable on port %d as a part of dynamic port reconfiguration", iOutputPortIndex));

            iPendingCommands = 1;

            if (0 == oscl_strcmp(iFormat, "H264") || 0 == oscl_strcmp(iFormat, "H263")
                    || 0 == oscl_strcmp(iFormat, "M4V"))
            {
                iOutBufferSize = ((iParamPort.format.video.nFrameWidth + 15) & ~15) * ((iParamPort.format.video.nFrameHeight + 15) & ~15) * 3 / 2;
            }
            else if (0 == oscl_strcmp(iFormat, "WMV"))
            {
                iOutBufferSize = ((iParamPort.format.video.nFrameWidth + 3) & ~3) * ((iParamPort.format.video.nFrameHeight + 3) & ~3) * 3 / 2;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxDecTestPauseResume::Run() - Allocating buffer again after port reconfigutauion has been complete"));

            for (OMX_S32 ii = 0; ii < iOutBufferCount; ii++)
            {
                Err = OMX_AllocateBuffer(ipAppPriv->Handle, &ipOutBuffer[ii], iOutputPortIndex, NULL, iOutBufferSize);
                CHECK_ERROR(Err, "AllocateBuffer_Output_DynamicReconfig");
                ipOutReleased[ii] = OMX_TRUE;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxDecTestPauseResume::Run() - AllocateBuffer called for buffer index %d on port %d", ii, iOutputPortIndex));
            }

            if (StateError == iState)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "OmxDecTestPauseResume::Run() - Error occured in this state, StateDynamicReconfig OUT"));
                RunIfNotReady();
                break;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestPauseResume::Run() - StateDynamicReconfig OUT"));
        }
        break;

        case StateExecuting:
        {
            static OMX_BOOL EosFlag = OMX_FALSE;
            static OMX_ERRORTYPE Status;
            OMX_S32 Index;
            OMX_BOOL MoreOutput;
            OMX_ERRORTYPE Err = OMX_ErrorNone;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestPauseResume::Run() - StateExecuting IN"));

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
                    //Reset this till u receive the callback for output buffer free
                    ipOutReleased[Index] = OMX_FALSE;

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxDecTestPauseResume::Run() - FillThisBuffer command called for output buffer index %d", Index));
                }
                else
                {
                    MoreOutput = OMX_FALSE;
                }
            }


            if (!iStopProcessingInput || (OMX_ErrorInsufficientResources == Status))
            {
                //After Processing N number of buffers, send the pause command
                if ((iFrameCount > TEST_NUM_BUFFERS_TO_PROCESS) && (OMX_FALSE == iPauseCommandSent))
                {
                    OMX_ERRORTYPE Err = OMX_ErrorNone;
                    Err = OMX_SendCommand(ipAppPriv->Handle, OMX_CommandStateSet, OMX_StatePause, NULL);
                    CHECK_ERROR(Err, "SendCommand Executing->Pause");

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxDecTestPauseResume::Run() - Sent state transition command from Executing -> Pause"));

                    iPendingCommands = 1;
                    iPauseCommandSent = OMX_TRUE;
                }
                else
                {
                    // find available input buffer
                    Index = 0;
                    while (OMX_FALSE == ipInputAvail[Index] && Index < iInBufferCount)
                    {
                        Index++;
                    }

                    if (Index != iInBufferCount)
                    {
                        Status = (*this.*pGetInputFrame)();
                        iFrameCount++;
                    }
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
                                    (0, "OmxDecTestPauseResume::Run() - Input buffer sent to the component with OMX_BUFFERFLAG_EOS flag set"));
                }
            }
            else
            {
                //nothing to do here
            }

            RunIfNotReady();

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestPauseResume::Run() - StateExecuting OUT"));
        }

        break;

        case StatePause:
        {
            OMX_BOOL MoreOutput;
            OMX_S32 Index;
            OMX_ERRORTYPE Err = OMX_ErrorNone;
            OMX_ERRORTYPE Status = OMX_ErrorNone;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestPauseResume::Run() - StatePause IN"));

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxDecTestPauseResume::Run() - In the paused state, sending ip and op buffers to the component"));

            //Once paused state is reached send some more buffers in the component's queue.
            //Queuing output buffers
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
                    Err = OMX_FillThisBuffer(ipAppPriv->Handle, ipOutBuffer[Index]);
                    ipOutReleased[Index] = OMX_FALSE;
                    CHECK_ERROR(Err, "FillThisBuffer");

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxDecTestPauseResume::Run() - FillThisBuffer command called for output buffer index %d", Index));
                }
                else
                {
                    MoreOutput = OMX_FALSE;
                }
            }

            //Queuing input buffers
            MoreOutput = OMX_TRUE;
            while (MoreOutput && (OMX_FALSE == iStopProcessingInput))
            {
                Index = 0;
                while (OMX_FALSE == ipInputAvail[Index] && Index < iInBufferCount)
                {
                    Index++;
                }

                if (Index != iInBufferCount)
                {
                    Status = (*this.*pGetInputFrame)();
                    iFrameCount++;
                }
                else
                {
                    MoreOutput = OMX_FALSE;
                }
            }

            //Now resume processing by changing state to Executing again
            Err = OMX_SendCommand(ipAppPriv->Handle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
            CHECK_ERROR(Err, "SendCommand Pause->Executing");

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxDecTestPauseResume::Run() - State Transition command sent from Pause->Executing"));

            iPendingCommands = 1;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestPauseResume::Run() - StatePause OUT"));
        }
        break;


        case StateStopping:
        {
            static OMX_BOOL FlagTemp = OMX_FALSE;
            OMX_ERRORTYPE Err = OMX_ErrorNone;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestPauseResume::Run() - StateStopping IN"));

            //stop execution by state transition to Idle state.
            if (!FlagTemp)
            {
                Err = OMX_SendCommand(ipAppPriv->Handle, OMX_CommandStateSet, OMX_StateIdle, NULL);
                CHECK_ERROR(Err, "SendCommand Executing->Idle");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxDecTestPauseResume::Run() - Sent State Transition Command from Executing->Idle"));

                iPendingCommands = 1;
                FlagTemp = OMX_TRUE;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestPauseResume::Run() - StateStopping OUT"));
        }
        break;


        case StateCleanUp:
        {
            OMX_S32 ii;
            OMX_ERRORTYPE Err = OMX_ErrorNone;
            static OMX_BOOL FlagTemp = OMX_FALSE;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestPauseResume::Run() - StateCleanUp IN"));


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
                                (0, "OmxDecTestPauseResume::Run() - Sent State Transition Command from Idle->Loaded"));

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
                                            (0, "OmxDecTestPauseResume::Run() - Called FreeBuffer for buffer index %d on port %d", ii, iInputPortIndex));
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
                                            (0, "OmxDecTestPauseResume::Run() - Called FreeBuffer for buffer index %d on port %d", ii, iOutputPortIndex));
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

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestPauseResume::Run() - StateCleanUp OUT"));

        }
        break;


        /********* FREE THE HANDLE & CLOSE FILES FOR THE COMPONENT ********/
        case StateStop:
        {
            OMX_U8 TestName[] = "PAUSE_RESUME_TEST";
            OMX_ERRORTYPE Err = OMX_ErrorNone;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestPauseResume::Run() - StateStop IN"));

            if (ipAppPriv)
            {
                if (ipAppPriv->Handle)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxDecTestPauseResume::Run() - Free the Component Handle"));

                    Err = OMX_FreeHandle(ipAppPriv->Handle);
                    if (OMX_ErrorNone != Err)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestPauseResume::Run() - FreeHandle Error"));
                        iTestStatus = OMX_FALSE;
                    }
                }
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxDecTestPauseResume::Run() - De-initialize the omx component"));

            Err = OMX_Deinit();
            if (OMX_ErrorNone != Err)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestPauseResume::Run() - OMX_Deinit Error"));
                iTestStatus = OMX_FALSE;
            }

            if (0 == oscl_strcmp(iFormat, "H264"))
            {
                if (ipAVCBSO)
                {
                    OSCL_DELETE(ipAVCBSO);
                    ipAVCBSO = NULL;
                }
            }

            if (0 == oscl_strcmp(iFormat, "M4V") || 0 == oscl_strcmp(iFormat, "H263"))
            {
                if (ipBitstreamBuffer)
                {
                    oscl_free(ipBitstreamBufferPointer);
                    ipBitstreamBuffer = NULL;
                    ipBitstreamBufferPointer = NULL;
                }
            }

            if (0 == oscl_strcmp(iFormat, "MP3"))
            {
                if (ipMp3Bitstream)
                {
                    OSCL_DELETE(ipMp3Bitstream);
                    ipMp3Bitstream = NULL;
                }
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

            VerifyOutput(TestName);

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestPauseResume::Run() - StateStop OUT"));

            iState = StateUnLoaded;
            OsclExecScheduler* sched = OsclExecScheduler::Current();
            sched->StopScheduler();
        }
        break;

        case StateError:
        {
            //Do all the cleanup's and exit from here
            OMX_S32 ii;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestPauseResume::Run() - StateError IN"));

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
                                        (0, "OmxDecTestPauseResume::Run() - Called FreeBuffer for buffer index %d on port %d", ii, iInputPortIndex));
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
                                        (0, "OmxDecTestPauseResume::Run() - Called FreeBuffer for buffer index %d on port %d", ii, iOutputPortIndex));
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

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestPauseResume::Run() - StateError OUT"));

        }
        break;

        default:
        {
            break;
        }
    }
    return ;
}
