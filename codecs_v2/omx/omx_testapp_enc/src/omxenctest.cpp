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
#include "omxenctest.h"
#include "oscl_string_utils.h"
#include "oscl_str_ptr_len.h"
#include "oscl_stdstring.h"

#ifndef PVLOGGER_STDERR_APPENDER_H_INCLUDED
#include "pvlogger_stderr_appender.h"
#endif

#ifndef PVLOGGER_FILE_APPENDER_H_INCLUDED
#include "pvlogger_file_appender.h"
#endif


#ifndef PVLOGGER_TIME_AND_ID_LAYOUT_H_INCLUDED
#include "pvlogger_time_and_id_layout.h"
#endif


#define NUMBER_TEST_CASES 9



//Read the data from input file & pass it to the AMR component
OMX_ERRORTYPE OmxComponentEncTest::GetInputFrameAMR()
{
    OMX_S32 Index;
    OMX_S32 ReadCount;
    OMX_S32 Size;
    OMX_ERRORTYPE Status;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentEncTest::GetInputFrameAMR() - IN"));


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
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentEncTest::GetInputFrameAMR() - Input buffer not available, OUT"));
                return OMX_ErrorNone;
            }

            if (!iFragmentInProgress)
            {
                iCurFrag = 0;
                Size = iInBufferSize;
                for (OMX_S32 kk = 0; kk < iNumSimFrags - 1; kk++)
                {
                    iSimFragSize[kk] = Size / iNumSimFrags;
                }
                iSimFragSize[iNumSimFrags-1] = Size - (iNumSimFrags - 1) * (Size / iNumSimFrags); // last fragment
            }

            Size = iSimFragSize[iCurFrag];
            iCurFrag++;
            if (iCurFrag == iNumSimFrags)
            {
                iFragmentInProgress = OMX_FALSE;
            }
            else
            {
                iFragmentInProgress = OMX_TRUE;
            }

            ReadCount = fread(ipInBuffer[Index]->pBuffer, 1, Size, ipInputFile);
            iInputFileSize -= ReadCount;

            if (0 == iInputFileSize)
            {
                iStopProcessingInput = OMX_TRUE;
                iInIndex = Index;
                iLastInputFrame = iFrameNum;
                Size = ReadCount;
                iFragmentInProgress = OMX_FALSE;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentEncTest::GetInputFrameAMR() - Input file has ended, OUT"));
            }

            if (iFragmentInProgress)
            {
                ipInBuffer[Index]->nFlags  = 0;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::GetInputFrameAMR() - Input buffer of index %d without any marker", Index));
            }
            else
            {
                ipInBuffer[Index]->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
                iFrameNum++;
            }

            ipInBuffer[Index]->nFilledLen = Size;
        }

        ipInBuffer[Index]->nOffset = 0;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "OmxComponentEncTest::GetInputFrameAMR() - Calling EmptyThisBuffer with Alloc Len %d, Filled Len %d Offset %d TimeStamp %d",
                         ipInBuffer[Index]->nAllocLen, ipInBuffer[Index]->nFilledLen, ipInBuffer[Index]->nOffset, ipInBuffer[Index]->nTimeStamp));

        Status = OMX_EmptyThisBuffer(ipAppPriv->Handle, ipInBuffer[Index]);

        if (OMX_ErrorNone == Status)
        {
            ipInputAvail[Index] = OMX_FALSE; // mark unavailable

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxComponentEncTest::GetInputFrameAMR() - Sent the input buffer sucessfully, OUT"));

            return OMX_ErrorNone;
        }
        else if (OMX_ErrorInsufficientResources == Status)
        {
            iInIndex = Index; // save the current input buffer, wait for call back
            iInputReady = OMX_FALSE;
            iInputWasRefused = OMX_TRUE;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentEncTest::GetInputFrameAMR() - EmptyThisBuffer failed with Error OMX_ErrorInsufficientResources, OUT"));

            return OMX_ErrorInsufficientResources;
        }
        else
        {
            iStopProcessingInput = OMX_TRUE;
            iLastInputFrame = iFrameNum;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentEncTest::GetInputFrameAMR() - Error, EmptyThisBuffer failed, OUT"));

            return OMX_ErrorNone;
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentEncTest::GetInputFrameAMR() - Error, Input is not ready to be sent, OUT"));
        return OMX_ErrorInsufficientResources;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentEncTest::GetInputFrameAMR() - OUT"));
    return OMX_ErrorNone;
}




OMX_ERRORTYPE OmxComponentEncTest::GetInput()
{
    OMX_S32 Index;
    OMX_S32 ReadCount;
    OMX_S32 Size;
    OMX_ERRORTYPE Status;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentEncTest::GetInput() - IN"));

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
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentEncTest::GetInput() - Input buffer not available, OUT"));
                return OMX_ErrorNone;
            }

            if (!iFragmentInProgress)
            {
                iCurFrag = 0;
                Size = iInBufferSize;
                for (OMX_S32 kk = 0; kk < iNumSimFrags - 1; kk++)
                {
                    iSimFragSize[kk] = Size / iNumSimFrags;
                }
                iSimFragSize[iNumSimFrags-1] = Size - (iNumSimFrags - 1) * (Size / iNumSimFrags); // last fragment
            }

            Size = iSimFragSize[iCurFrag];
            iCurFrag++;
            if (iCurFrag == iNumSimFrags)
            {
                iFragmentInProgress = OMX_FALSE;
            }
            else
            {
                iFragmentInProgress = OMX_TRUE;
            }

            ReadCount = fread(ipInBuffer[Index]->pBuffer, 1, Size, ipInputFile);
            if (!(0 == oscl_strcmp(iFormat, "AMRNB")))
            {
                ipInBuffer[Index]->nTimeStamp = ((iFrameNum * 1000) / iFrameRate);
            }

            if (0 == ReadCount)
            {
                iStopProcessingInput = OMX_TRUE;
                iInIndex = Index;
                iLastInputFrame = iFrameNum;
                iFragmentInProgress = OMX_FALSE;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentEncTest::GetInput() - Input file has ended, OUT"));

                return OMX_ErrorNone;
            }
            else if (ReadCount < Size)
            {
                iStopProcessingInput = OMX_TRUE;
                iInIndex = Index;
                iLastInputFrame = iFrameNum;
                iFragmentInProgress = OMX_FALSE;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentEncTest::GetInput() - Last piece of data in input file"));

                Size = ReadCount;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentEncTest::GetInput() - Input data of %d bytes read from the file", Size));

            if (iFragmentInProgress)
            {

                ipInBuffer[Index]->nFlags  = 0;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::GetInput() - Input buffer of index %d without any marker", Index));

            }
            else
            {
                //Don't mark the EndOfFrame flag
                iFrameNum++;
            }

            ipInBuffer[Index]->nFilledLen = Size;
        }

        ipInBuffer[Index]->nOffset = 0;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "OmxComponentEncTest::GetInput() - Calling EmptyThisBuffer with Alloc Len %d, Filled Len %d Offset %d TimeStamp %d",
                         ipInBuffer[Index]->nAllocLen, ipInBuffer[Index]->nFilledLen, ipInBuffer[Index]->nOffset, ipInBuffer[Index]->nTimeStamp));


        Status = OMX_EmptyThisBuffer(ipAppPriv->Handle, ipInBuffer[Index]);

        if (OMX_ErrorNone == Status)
        {
            ipInputAvail[Index] = OMX_FALSE; // mark unavailable

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxComponentEncTest::GetInput() - Sent the input buffer sucessfully, OUT"));

            return OMX_ErrorNone;
        }
        else if (OMX_ErrorInsufficientResources == Status)
        {
            iInIndex = Index; // save the current input buffer, wait for call back
            iInputReady = OMX_FALSE;
            iInputWasRefused = OMX_TRUE;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentEncTest::GetInput() - EmptyThisBuffer failed with Error OMX_ErrorInsufficientResources, OUT"));
            return OMX_ErrorInsufficientResources;
        }
        else
        {
            iStopProcessingInput = OMX_TRUE;
            iLastInputFrame = iFrameNum;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentEncTest::GetInput() - Error, EmptyThisBuffer failed, OUT"));

            return OMX_ErrorNone;
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentEncTest::GetInput() - Error, Input is not ready to be sent, OUT"));
        return OMX_ErrorInsufficientResources;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentEncTest::GetInput() - OUT"));

    return OMX_ErrorNone;
}

//Read the data from input file & pass it to the  component
OMX_ERRORTYPE OmxComponentEncTest::GetInputFrame()
{
    OMX_S32 Index;
    OMX_S32 ReadCount;
    OMX_S32 Size;
    OMX_ERRORTYPE Status;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentEncTest::GetInputFrame() - IN"));

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
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentEncTest::GetInputFrame() - Input buffer not available, OUT"));
                return OMX_ErrorNone;
            }

            if (!iFragmentInProgress)
            {
                iCurFrag = 0;
                Size = iInBufferSize;
                for (OMX_S32 kk = 0; kk < iNumSimFrags - 1; kk++)
                {
                    iSimFragSize[kk] = Size / iNumSimFrags;
                }
                iSimFragSize[iNumSimFrags-1] = Size - (iNumSimFrags - 1) * (Size / iNumSimFrags); // last fragment
            }

            Size = iSimFragSize[iCurFrag];
            iCurFrag++;
            if (iCurFrag == iNumSimFrags)
            {
                iFragmentInProgress = OMX_FALSE;
            }
            else
            {
                iFragmentInProgress = OMX_TRUE;
            }

            ReadCount = fread(ipInBuffer[Index]->pBuffer, 1, Size, ipInputFile);
            ipInBuffer[Index]->nTimeStamp = ((iFrameNum * 1000) / iFrameRate);

            if (0 == ReadCount)
            {
                iStopProcessingInput = OMX_TRUE;
                iInIndex = Index;
                iLastInputFrame = iFrameNum;
                iFragmentInProgress = OMX_FALSE;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentEncTest::GetInputFrame() - Input file has ended, OUT"));

                return OMX_ErrorNone;
            }
            else if (ReadCount < Size)
            {
                iStopProcessingInput = OMX_TRUE;
                iInIndex = Index;
                iLastInputFrame = iFrameNum;
                iFragmentInProgress = OMX_FALSE;
                Size = ReadCount;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentEncTest::GetInputFrame() - Last piece of data in input file"));
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentEncTest::GetInputFrame() - Input data of %d bytes read from the file", Size));

            if (iFragmentInProgress)
            {
                ipInBuffer[Index]->nFlags  = 0;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::GetInputFrame() - Input buffer of index %d without any marker", Index));

            }
            else
            {
                ipInBuffer[Index]->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;
                iFrameNum++;
            }

            ipInBuffer[Index]->nFilledLen = Size;
        }

        ipInBuffer[Index]->nOffset = 0;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "OmxComponentEncTest::GetInputFrame() - Calling EmptyThisBuffer with Alloc Len %d, Filled Len %d Offset %d TimeStamp %d",
                         ipInBuffer[Index]->nAllocLen, ipInBuffer[Index]->nFilledLen, ipInBuffer[Index]->nOffset, ipInBuffer[Index]->nTimeStamp));

        Status = OMX_EmptyThisBuffer(ipAppPriv->Handle, ipInBuffer[Index]);

        if (OMX_ErrorNone == Status)
        {
            ipInputAvail[Index] = OMX_FALSE; // mark unavailable

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxComponentEncTest::GetInputFrame() - Sent the input buffer sucessfully, OUT"));

            return OMX_ErrorNone;
        }
        else if (OMX_ErrorInsufficientResources == Status)
        {
            iInIndex = Index; // save the current input buffer, wait for call back
            iInputReady = OMX_FALSE;
            iInputWasRefused = OMX_TRUE;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentEncTest::GetInputFrame() - EmptyThisBuffer failed with Error OMX_ErrorInsufficientResources, OUT"));
            return OMX_ErrorInsufficientResources;
        }
        else
        {
            iStopProcessingInput = OMX_TRUE;
            iLastInputFrame = iFrameNum;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentEncTest::GetInputFrame() - Error, EmptyThisBuffer failed, OUT"));
            return OMX_ErrorNone;
        }
    }
    else
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentEncTest::GetInputFrame() - Error, Input is not ready to be sent, OUT"));
        return OMX_ErrorInsufficientResources;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentEncTest::GetInputFrame() - OUT"));

    return OMX_ErrorNone;
}

bool OmxComponentEncTest::WriteOutput(OMX_U8* aOutBuff, OMX_U32 aSize)
{
    if (0 == oscl_strcmp(iFormat, "H264"))
    {
        //Writing fixed bytes 0 0 1 before each NAL, so that it can be decoded by the avc decoder

        OMX_U8 StartCode[] = {0, 0, 1};
        fwrite(StartCode, sizeof(OMX_U8), 3, ipOutputFile);
    }

    OMX_S32 BytesWritten = fwrite(aOutBuff, sizeof(OMX_U8), aSize, ipOutputFile);
    return (BytesWritten == aSize);
}

/*
 * Active Object class's Run () function
 * Control all the states of AO & sends API's to the component
 */
static OMX_BOOL DisableRun = OMX_FALSE;

void OmxComponentEncTest::Run()
{
    switch (iState)
    {
        case StateUnLoaded:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentEncTest::Run() - StateUnLoaded IN"));

            OMX_ERRORTYPE Err;
            OMX_U32 PortIndex, ii;

            if (!iCallbacks->initCallbacks())
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentEncTest::Run() - ERROR initCallbacks failed, OUT"));
                StopOnError();
                break;
            }

            ipAppPriv = (AppPrivateType*) oscl_malloc(sizeof(AppPrivateType));
            CHECK_MEM(ipAppPriv, "Component_Handle");

            //This should be the first call to the component to load it.
            Err = OMX_Init();
            CHECK_ERROR(Err, "OMX_Init");
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentEncTest::Run() - OMX_Init done"));

            //Setting the callbacks
            if (NULL != iRole)
            {
                //Determine the component first & then get the handle
                OMX_U32 NumComps = 0;
                OMX_STRING* pCompOfRole;

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentEncTest::Run() - Finding out the components that can support the role %s", iRole));

                // call once to find out the number of components that can fit the role
                Err = OMX_GetComponentsOfRole(iRole, &NumComps, NULL);

                if (OMX_ErrorNone != Err || NumComps < 1)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentEncTest::Run() - ERROR, No component can handle the specified role %s", iRole));
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
                                    (0, "OmxComponentEncTest::Run() - Error occured in this state, StateUnLoaded OUT"));
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
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentEncTest::Run() - Got Handle for the component %s", pCompOfRole[ii]));
                        break;
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentEncTest::Run() - ERROR, Cannot get component %s handle, try another if possible", pCompOfRole[ii]));
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


            ipInputFile = fopen(iInputFileName, "rb");
            CHECK_MEM(ipInputFile, "Input_FileName");

            ipOutputFile = fopen(iOutputFileName, "wb");
            CHECK_MEM(ipOutputFile, "Output_FileName")

            //This will initialize the size and version of the iPortInit structure
            INIT_GETPARAMETER_STRUCT(OMX_PORT_PARAM_TYPE, iPortInit);

            if (0 == oscl_strcmp(iFormat, "AMRNB"))
            {
                if (ipInputFile)
                {
                    fseek(ipInputFile, 0, SEEK_END);
                    iInputFileSize = ftell(ipInputFile);
                    fseek(ipInputFile, 0, SEEK_SET);
                }
                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamAudioInit, &iPortInit);
            }
            else if (0 == oscl_strcmp(iFormat, "AAC"))
            {
                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamAudioInit, &iPortInit);
            }
            else
            {
                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamVideoInit, &iPortInit);
            }

            CHECK_ERROR(Err, "GetParameter_Audio/Video_Init");

            for (ii = 0; ii < iPortInit.nPorts; ii++)
            {
                PortIndex = iPortInit.nStartPortNumber + ii;

                //This will initialize the size and version of the iParamPort structure
                INIT_GETPARAMETER_STRUCT(OMX_PARAM_PORTDEFINITIONTYPE, iParamPort);
                iParamPort.nPortIndex = PortIndex;

                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamPortDefinition, &iParamPort);
                CHECK_ERROR(Err, "GetParameter_IndexParamPortDefinition");
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxComponentEncTest::Run() - GetParameter called for OMX_IndexParamPortDefinition on port %d", PortIndex));

                if (0 == iParamPort.nBufferCountMin)
                {
                    /* a buffer count of 0 is not allowed */
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentEncTest::Run() - Error, GetParameter for OMX_IndexParamPortDefinition returned 0 min buffer count"));
                    StopOnError();
                    break;
                }

                if (iParamPort.nBufferCountMin > iParamPort.nBufferCountActual)
                {
                    /* Min buff count can't be more than actual buff count */
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "OmxComponentEncTest::Run() - ERROR, GetParameter for OMX_IndexParamPortDefinition returned actual buffer count %d less than min buffer count %d", iParamPort.nBufferCountActual, iParamPort.nBufferCountMin));
                    StopOnError();
                    break;
                }

                if (OMX_DirInput == iParamPort.eDir)
                {
                    iInputPortIndex = PortIndex;

                    iInBufferSize = iParamPort.nBufferSize;
                    iInBufferCount = iParamPort.nBufferCountActual;
                    iParamPort.nBufferCountActual = iInBufferCount;

                    if ((0 == oscl_strcmp(iFormat, "M4V")) | (0 == oscl_strcmp(iFormat, "H264")))
                    {
                        OMX_U32 InputFrameSize;

                        if (OMX_COLOR_Format24bitRGB888 == iColorFormat)
                        {
                            InputFrameSize = (iFrameWidth * iFrameHeight * 3);
                        }
                        else if (OMX_COLOR_Format12bitRGB444 == iColorFormat)
                        {
                            InputFrameSize = (iFrameWidth * iFrameHeight * 2);
                        }
                        else if (OMX_COLOR_FormatYUV420Planar == iColorFormat)
                        {
                            InputFrameSize = (iFrameWidth * iFrameHeight * 3) >> 1;
                        }
                        else if (OMX_COLOR_FormatYUV420SemiPlanar == iColorFormat)
                        {
                            InputFrameSize = (iFrameWidth * iFrameHeight * 3) >> 1;
                        }

                        iInBufferSize = InputFrameSize;

                        //Set the input parameters to the encoder component
                        iParamPort.nPortIndex = PortIndex;
                        iParamPort.nBufferSize = InputFrameSize;

                        iParamPort.format.video.nFrameWidth = iFrameWidth;
                        iParamPort.format.video.nFrameHeight = iFrameHeight;
                        iParamPort.format.video.xFramerate = iFrameRate << 16;
                        iParamPort.format.video.eColorFormat = iColorFormat;

                    }

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxComponentEncTest::Run() - GetParameter returned Num of input buffers %d with Size %d", iInBufferCount, iInBufferSize));
                }
                else if (OMX_DirOutput == iParamPort.eDir)
                {
                    iOutputPortIndex = PortIndex;

                    iOutBufferSize = iParamPort.nBufferSize;
                    iOutBufferCount = iParamPort.nBufferCountActual;

                    iParamPort.nBufferCountActual = iOutBufferCount;

                    if ((0 == oscl_strcmp(iFormat, "M4V")) | (0 == oscl_strcmp(iFormat, "H264")))
                    {
                        iParamPort.nPortIndex = PortIndex;
                        iParamPort.format.video.nFrameWidth = iTgtFrameWidth;
                        iParamPort.format.video.nFrameHeight = iTgtFrameHeight;
                        iParamPort.format.video.xFramerate = iTgtFrameRate << 16;
                        iParamPort.format.video.nBitrate = iTgtBitRate;

                    }

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxComponentEncTest::Run() - GetParameter returned Num of output buffers %d with Size %d", iOutBufferCount, iOutBufferSize));
                }

                //Take the buffer parameters of what component has specified
                iParamPort.nPortIndex = PortIndex;

                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamPortDefinition, &iParamPort);
                CHECK_ERROR(Err, "SetParameter_OMX_IndexParamPortDefinition");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::Run() - SetParameter called for OMX_IndexParamPortDefinition on port %d", PortIndex));
            }

            if (StateError == iState)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "OmxComponentEncTest::Run() - Error, Exiting the test case, OUT"));

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
                iState = StateUnLoaded;
                OsclExecScheduler* sched = OsclExecScheduler::Current();
                sched->StopScheduler();
            }
#endif
            if (0 == oscl_strcmp(iFormat, "AMRNB"))
            {
                INIT_GETPARAMETER_STRUCT(OMX_AUDIO_PARAM_PCMMODETYPE, iPcmMode);
                iPcmMode.nPortIndex = iInputPortIndex;
                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamAudioPcm, &iPcmMode);
                CHECK_ERROR(Err, "GetParameter_ENC_IndexParamAudioPcm");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - GetParameter called for OMX_IndexParamAudioPcm for AMR on port %d", iInputPortIndex));


                //Set the input port parameters
                iPcmMode.nPortIndex = iInputPortIndex;

                iPcmMode.nChannels = iInputNumberOfChannels;
                iPcmMode.nBitPerSample = iInputBitsPerSample;
                iPcmMode.nSamplingRate = iInputSamplingRate;

                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamAudioPcm, &iPcmMode);
                CHECK_ERROR(Err, "SetParameter_ENC_IndexParamAudioPcm");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - SetParameter called for OMX_IndexParamAudioPcm for AMR on port %d", iInputPortIndex));



                /* Pass the output format type information to the  component*/

                INIT_GETPARAMETER_STRUCT(OMX_AUDIO_PARAM_AMRTYPE, iAmrParam);
                iAmrParam.nPortIndex = iOutputPortIndex;

                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamAudioAmr, &iAmrParam);
                CHECK_ERROR(Err, "GetParameter_ENC_IndexParamAudio");
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - GetParameter called for OMX_IndexParamAudioPcm for AMR on port %d", iOutputPortIndex));

                iAmrParam.nPortIndex = iOutputPortIndex;
                iAmrParam.eAMRFrameFormat = iOutputFormat;
                iAmrParam.eAMRBandMode = iOutputBandMode;

                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamAudioAmr, &iAmrParam);
                CHECK_ERROR(Err, "SetParameter_ENC_IndexParamAudio");
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - SetParameter called for OMX_IndexParamAudioAmr for AMR on port %d", iOutputPortIndex));
            }
            else if (0 == oscl_strcmp(iFormat, "M4V"))
            {
                INIT_GETPARAMETER_STRUCT(OMX_VIDEO_PARAM_MPEG4TYPE, iMpeg4Type);
                iMpeg4Type.nPortIndex = iOutputPortIndex;

                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamVideoMpeg4, &iMpeg4Type);
                CHECK_ERROR(Err, "GetParameter_MPEG4ENC_IndexParamVideoMpeg4");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - GetParameter called for OMX_IndexParamVideoMpeg4 for Mpeg4/h263 on port %d", iOutputPortIndex));

                //Set the OMX_VIDEO_PARAM_MPEG4TYPE parameters
                iMpeg4Type.nPortIndex = iOutputPortIndex;
                iMpeg4Type.eProfile = iMpeg4Profile;
                iMpeg4Type.eLevel = iMpeg4Level;
                iMpeg4Type.nPFrames = iNumPFrames;
                iMpeg4Type.nMaxPacketSize = iMaxPacketSize;
                iMpeg4Type.bSVH = iShortHeaderFlag;
                iMpeg4Type.bReversibleVLC = iReversibleVLCFlag;
                iMpeg4Type.nTimeIncRes = iTimeIncRes;

                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamVideoMpeg4, &iMpeg4Type);
                CHECK_ERROR(Err, "SetParameter_MPEG4ENC_IndexParamVideoMpeg4");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - SetParameter called for OMX_IndexParamVideoMpeg4 for Mpeg4/h263 on port %d", iOutputPortIndex));


                //OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE Settings
                INIT_GETPARAMETER_STRUCT(OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE, iErrCorrType);
                iErrCorrType.nPortIndex = iOutputPortIndex;

                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamVideoErrorCorrection, &iErrCorrType);
                CHECK_ERROR(Err, "GetParameter_MPEG4ENC_IndexParamVideoErrorCorrection");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - GetParameter called for OMX_IndexParamVideoErrorCorrection for Mpeg4/h263 on port %d", iOutputPortIndex));

                //Set the parameters now
                iErrCorrType.nPortIndex = iOutputPortIndex;
                iErrCorrType.bEnableDataPartitioning = iDataPartitioningFlag;
                iErrCorrType.bEnableResync = iResyncFlag;
                iErrCorrType.nResynchMarkerSpacing = iResynchMarkerSpacing;

                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamVideoErrorCorrection, &iErrCorrType);
                CHECK_ERROR(Err, "SetParameter_MPEG4ENC_IndexParamVideoErrorCorrection");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - SetParameter called for OMX_IndexParamVideoErrorCorrection for Mpeg4/h263 on port %d", iOutputPortIndex));



                //OMX_VIDEO_PARAM_BITRATETYPE Settings
                INIT_GETPARAMETER_STRUCT(OMX_VIDEO_PARAM_BITRATETYPE, iBitRateType);
                iBitRateType.nPortIndex = iOutputPortIndex;

                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamVideoBitrate, &iBitRateType);
                CHECK_ERROR(Err, "GetParameter_MPEG4ENC_IndexParamVideoBitrate");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - GetParameter called for OMX_IndexParamVideoBitrate for Mpeg4/h263 on port %d", iOutputPortIndex));

                //Set the parameters now
                iBitRateType.nPortIndex = iOutputPortIndex;
                iBitRateType.eControlRate = iRateControlType;

                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamVideoBitrate, &iBitRateType);
                CHECK_ERROR(Err, "SetParameter_MPEG4ENC_IndexParamVideoBitrate");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - SetParameter called for OMX_IndexParamVideoBitrate for Mpeg4/h263 on port %d", iOutputPortIndex));


                //OMX_VIDEO_PARAM_QUANTIZATIONTYPE Settings
                INIT_GETPARAMETER_STRUCT(OMX_VIDEO_PARAM_QUANTIZATIONTYPE, iQuantParam);
                iQuantParam.nPortIndex = iOutputPortIndex;

                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamVideoQuantization, &iQuantParam);
                CHECK_ERROR(Err, "GetParameter_MPEG4ENC_IndexParamVideoQuant");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - GetParameter called for OMX_IndexParamVideoQuantization for Mpeg4/h263 on port %d", iOutputPortIndex));

                //Set the parameters now
                iQuantParam.nPortIndex = iOutputPortIndex;
                iQuantParam.nQpI = iIQuant;
                iQuantParam.nQpP = iPQuant;

                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamVideoQuantization, &iQuantParam);
                CHECK_ERROR(Err, "SetParameter_MPEG4ENC_IndexParamVideoQuant");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - SetParameter called for OMX_IndexParamVideoQuantization for Mpeg4/h263 on port %d", iOutputPortIndex));


                //OMX_VIDEO_PARAM_MOTIONVECTORTYPE Settings
                INIT_GETPARAMETER_STRUCT(OMX_VIDEO_PARAM_MOTIONVECTORTYPE, iMotionVector);
                iMotionVector.nPortIndex = iOutputPortIndex;

                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamVideoMotionVector, &iMotionVector);
                CHECK_ERROR(Err, "GetParameter_MPEG4ENC_IndexParamVideoMotionVector");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - GetParameter called for OMX_IndexParamVideoMotionVector for Mpeg4/h263 on port %d", iOutputPortIndex));

                //Set the parameters now
                iMotionVector.nPortIndex = iOutputPortIndex;
                iMotionVector.sXSearchRange = iSearchRange;
                iMotionVector.sYSearchRange = iSearchRange;
                iMotionVector.bFourMV = iFourMV;

                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamVideoMotionVector, &iMotionVector);
                CHECK_ERROR(Err, "SetParameter_MPEG4ENC_IndexParamVideoMotionVector");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - SetParameter called for OMX_IndexParamVideoMotionVector for Mpeg4/h263 on port %d", iOutputPortIndex));


                //OMX_VIDEO_PARAM_INTRAREFRESHTYPE Settings
                INIT_GETPARAMETER_STRUCT(OMX_VIDEO_PARAM_INTRAREFRESHTYPE, iRefreshParam);
                iRefreshParam.nPortIndex = iOutputPortIndex;

                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamVideoIntraRefresh, &iRefreshParam);
                CHECK_ERROR(Err, "GetParameter_MPEG4ENC_IndexParamVideoIntraRefresh");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - GetParameter called for OMX_IndexParamVideoIntraRefresh for Mpeg4/h263 on port %d", iOutputPortIndex));

                //Set the parameters now
                iRefreshParam.nPortIndex = iOutputPortIndex;
                iRefreshParam.eRefreshMode = iRefreshMode;
                iRefreshParam.nCirMBs = iNumIntraMB;

                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamVideoIntraRefresh, &iRefreshParam);
                CHECK_ERROR(Err, "SetParameter_MPEG4ENC_IndexParamVideoIntraRefresh");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - SetParameter called for OMX_IndexParamVideoIntraRefresh for Mpeg4/h263 on port %d", iOutputPortIndex));


                //OMX_VIDEO_PARAM_H263TYPE type in case of h263 and short header mode
                if ((1 == iCodecMode) || (1 == iShortHeaderFlag))
                {
                    INIT_GETPARAMETER_STRUCT(OMX_VIDEO_PARAM_H263TYPE, iH263Type);
                    iH263Type.nPortIndex = iOutputPortIndex;

                    Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamVideoH263, &iH263Type);
                    CHECK_ERROR(Err, "GetParameter_MPEG4ENC_IndexParamVideoH263");

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxComponentEncTest::RunL() - GetParameter called for OMX_IndexParamVideoH263 for h263 on port %d", iOutputPortIndex));

                    //Set the parameters now
                    iH263Type.nPortIndex = iOutputPortIndex;
                    iH263Type.nGOBHeaderInterval = iGobHeaderInterval;

                    Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamVideoH263, &iH263Type);
                    CHECK_ERROR(Err, "SetParameter_MPEG4ENC_IndexParamVideoH263");

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxComponentEncTest::RunL() - SetParameter called for OMX_IndexParamVideoH263 for h263 on port %d", iOutputPortIndex));

                }


            }
            else if (0 == oscl_strcmp(iFormat, "H264"))
            {
                INIT_GETPARAMETER_STRUCT(OMX_VIDEO_PARAM_AVCTYPE, iAvcType);
                iAvcType.nPortIndex = iOutputPortIndex;

                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamVideoAvc, &iAvcType);
                CHECK_ERROR(Err, "GetParameter_AVCENC_IndexParamVideoAvc");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - GetParameter called for OMX_IndexParamVideoAvc for AVC on port %d", iOutputPortIndex));

                //Set the OMX_VIDEO_PARAM_AVCTYPE parameters
                iAvcType.nPortIndex = iOutputPortIndex;
                iAvcType.eProfile = iAvcProfile;
                iAvcType.eLevel = iAvcLevel;
                iAvcType.nPFrames = iNumPFrames;
                iAvcType.eLoopFilterMode = iLoopFilterType;
                iAvcType.bconstIpred = iIPredictionFlag;

                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamVideoAvc, &iAvcType);
                CHECK_ERROR(Err, "SetParameter_AVCENC_IndexParamVideoAvc");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - SetParameter called for OMX_IndexParamVideoAvc for AVC on port %d", iOutputPortIndex));

                //OMX_VIDEO_PARAM_BITRATETYPE Settings
                INIT_GETPARAMETER_STRUCT(OMX_VIDEO_PARAM_BITRATETYPE, iBitRateType);
                iBitRateType.nPortIndex = iOutputPortIndex;

                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamVideoBitrate, &iBitRateType);
                CHECK_ERROR(Err, "GetParameter_AVCENC_IndexParamVideoBitrate");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - GetParameter called for OMX_IndexParamVideoBitrate for AVC on port %d", iOutputPortIndex));

                //Set the parameters now
                iBitRateType.nPortIndex = iOutputPortIndex;
                iBitRateType.eControlRate = iRateControlType;
                iBitRateType.nTargetBitrate = iTgtBitRate;

                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamVideoBitrate, &iBitRateType);
                CHECK_ERROR(Err, "SetParameter_AVCENC_IndexParamVideoBitrate");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - SetParameter called for OMX_IndexParamVideoBitrate for AVC on port %d", iOutputPortIndex));


                //OMX_VIDEO_PARAM_QUANTIZATIONTYPE Settings
                INIT_GETPARAMETER_STRUCT(OMX_VIDEO_PARAM_QUANTIZATIONTYPE, iQuantParam);
                iQuantParam.nPortIndex = iOutputPortIndex;

                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamVideoQuantization, &iQuantParam);
                CHECK_ERROR(Err, "GetParameter_AVCENC_IndexParamVideoQuant");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - GetParameter called for OMX_IndexParamVideoQuantization for AVC on port %d", iOutputPortIndex));

                //Set the parameters now
                iQuantParam.nPortIndex = iOutputPortIndex;
                iQuantParam.nQpP = iPQuant;

                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamVideoQuantization, &iQuantParam);
                CHECK_ERROR(Err, "SetParameter_AVCENC_IndexParamVideoQuant");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - SetParameter called for OMX_IndexParamVideoQuantization for AVC on port %d", iOutputPortIndex));


                //OMX_VIDEO_PARAM_MOTIONVECTORTYPE Settings
                INIT_GETPARAMETER_STRUCT(OMX_VIDEO_PARAM_MOTIONVECTORTYPE, iMotionVector);
                iMotionVector.nPortIndex = iOutputPortIndex;

                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamVideoMotionVector, &iMotionVector);
                CHECK_ERROR(Err, "GetParameter_AVCENC_IndexParamVideoMotionVector");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - GetParameter called for OMX_IndexParamVideoMotionVector for AVC on port %d", iOutputPortIndex));

                //Set the parameters now
                iMotionVector.nPortIndex = iOutputPortIndex;
                iMotionVector.sXSearchRange = iSearchRange;
                iMotionVector.sYSearchRange = iSearchRange;
                iMotionVector.eAccuracy = iAccuracy;

                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamVideoMotionVector, &iMotionVector);
                CHECK_ERROR(Err, "SetParameter_AVCENC_IndexParamVideoMotionVector");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - SetParameter called for OMX_IndexParamVideoMotionVector for AVC on port %d", iOutputPortIndex));


                //OMX_VIDEO_PARAM_INTRAREFRESHTYPE Settings
                INIT_GETPARAMETER_STRUCT(OMX_VIDEO_PARAM_INTRAREFRESHTYPE, iRefreshParam);
                iRefreshParam.nPortIndex = iOutputPortIndex;

                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamVideoIntraRefresh, &iRefreshParam);
                CHECK_ERROR(Err, "GetParameter_AVCENC_IndexParamVideoIntraRefresh");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - GetParameter called for OMX_IndexParamVideoIntraRefresh for AVC on port %d", iOutputPortIndex));

                //Set the parameters now
                iRefreshParam.nPortIndex = iOutputPortIndex;
                iRefreshParam.eRefreshMode = iRefreshMode;
                iRefreshParam.nCirMBs = iNumIntraMB;

                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamVideoIntraRefresh, &iRefreshParam);
                CHECK_ERROR(Err, "SetParameter_AVCENC_IndexParamVideoIntraRefresh");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - SetParameter called for OMX_IndexParamVideoIntraRefresh for AVC on port %d", iOutputPortIndex));


            }
            else if (0 == oscl_strcmp(iFormat, "AAC"))
            {
                INIT_GETPARAMETER_STRUCT(OMX_AUDIO_PARAM_PCMMODETYPE, iPcmMode);
                iPcmMode.nPortIndex = iInputPortIndex;
                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamAudioPcm, &iPcmMode);
                CHECK_ERROR(Err, "GetParameter_ENC_IndexParamAudioPcm");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - GetParameter called for OMX_IndexParamAudioPcm for AAC on port %d", iInputPortIndex));


                //Set the input port parameters
                iPcmMode.nPortIndex = iInputPortIndex;

                iPcmMode.nChannels = iInputNumberOfChannels;
                iPcmMode.nBitPerSample = iInputBitsPerSample;
                iPcmMode.nSamplingRate = iInputSamplingRate;

                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamAudioPcm, &iPcmMode);
                CHECK_ERROR(Err, "SetParameter_ENC_IndexParamAudioPcm");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - SetParameter called for OMX_IndexParamAudioPcm for AAC on port %d", iInputPortIndex));



                /* Pass the output format type information to the component*/

                INIT_GETPARAMETER_STRUCT(OMX_AUDIO_PARAM_AACPROFILETYPE, iAacParam);
                iAacParam.nPortIndex = iOutputPortIndex;

                Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamAudioAac, &iAacParam);
                CHECK_ERROR(Err, "GetParameter_ENC_IndexParamAudioAac");
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - GetParameter called for IndexParamAudioAac for AAC on port %d", iOutputPortIndex));

                iAacParam.nPortIndex = iOutputPortIndex;

                iAacParam.nChannels = iOutputNumberOfChannels;
                iAacParam.nSampleRate = iInputSamplingRate;
                iAacParam.nBitRate = iTgtBitRate;
                iAacParam.nAudioBandWidth = iAacBandWidth;
                iAacParam.nAACtools = iAacTools;
                iAacParam.eAACProfile = iAacProfile;
                iAacParam.eAACStreamFormat = iAacStreamFormat;
                iAacParam.eChannelMode = iChannelMode;


                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamAudioAac, &iAacParam);
                CHECK_ERROR(Err, "SetParameter_ENC_IndexParamAudioAac");
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - SetParameter called for OMX_IndexParamAudioAac for AAC on port %d", iOutputPortIndex));
            }


            if (StateError != iState)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentEncTest::RunL() - StateUnLoaded OUT, moving to next state"));
                iState = StateLoaded;
            }

            RunIfNotReady();
        }
        break;

        case StateLoaded:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentEncTest::RunL() - StateLoaded IN"));
            OMX_ERRORTYPE Err;
            OMX_S32 ii;

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
                            (0, "OmxComponentEncTest::RunL() - Sent State Transition Command from Loaded->Idle"));

            iPendingCommands = 1;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxComponentEncTest::RunL() - Allocating %d input and %d output buffers", iInBufferCount, iOutBufferCount));



            //These calls are required because the control of in & out buffer should be with the testapp.
            for (ii = 0; ii < iInBufferCount; ii++)
            {

                Err = OMX_AllocateBuffer(ipAppPriv->Handle, &ipInBuffer[ii], iInputPortIndex, NULL, iInBufferSize);

                CHECK_ERROR(Err, "AllocateBuffer_Input");
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - Called AllocateBuffer for buffer index %d on port %d", ii, iInputPortIndex));

                ipInputAvail[ii] = OMX_TRUE;
                ipInBuffer[ii]->nInputPortIndex = iInputPortIndex;
            }

            if (StateError == iState)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "OmxComponentEncTest::RunL() - AllocateBuffer Error, StateLoaded OUT"));

                RunIfNotReady();
                break;
            }


            for (ii = 0; ii < iOutBufferCount; ii++)
            {

                Err = OMX_AllocateBuffer(ipAppPriv->Handle, &ipOutBuffer[ii], iOutputPortIndex, NULL, iOutBufferSize);

                CHECK_ERROR(Err, "AllocateBuffer_Output");
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - Called AllocateBuffer for buffer index %d on port %d", ii, iOutputPortIndex));

                ipOutReleased[ii] = OMX_TRUE;

                ipOutBuffer[ii]->nOutputPortIndex = iOutputPortIndex;
                ipOutBuffer[ii]->nInputPortIndex = 0;
            }

            if (StateError == iState)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "OmxComponentEncTest::RunL() - AllocateBuffer Error, StateLoaded OUT"));
                RunIfNotReady();
                break;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentEncTest::RunL() - StateLoaded OUT, Moving to next state"));
        }
        break;

        case StateIdle:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentEncTest::RunL() - StateIdle IN"));

            OMX_ERRORTYPE Err = OMX_ErrorNone;

            Err = OMX_SendCommand(ipAppPriv->Handle, OMX_CommandStateSet, OMX_StateExecuting, NULL);
            CHECK_ERROR(Err, "SendCommand Idle->Executing");

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxComponentEncTest::RunL() - Sent State Transition Command from Idle->Executing"));

            iPendingCommands = 1;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentEncTest::RunL() - StateIdle OUT"));
        }
        break;

        case StateExecuting:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentEncTest::RunL() - StateExecuting IN"));

            static OMX_BOOL EosFlag = OMX_FALSE;
            static OMX_ERRORTYPE Status;
            OMX_S32 Index;
            OMX_BOOL MoreOutput;
            OMX_ERRORTYPE Err = OMX_ErrorNone;

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
                    ipOutBuffer[Index]->nOffset = 0;
                    Err = OMX_FillThisBuffer(ipAppPriv->Handle, ipOutBuffer[Index]);
                    CHECK_ERROR(Err, "FillThisBuffer");

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxComponentEncTest::RunL() - FillThisBuffer command called for output buffer index %d", Index));

                    //Make this flag OMX_TRUE till u receive the callback for output buffer free
                    ipOutReleased[Index] = OMX_FALSE;
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
                    if (0 == oscl_strcmp(iFormat, "AMRNB"))
                    {
                        Status = GetInputFrameAMR();
                    }
                    else
                    {
                        Status = GetInputFrame();
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
                                    (0, "OmxComponentEncTest::RunL() - Input buffer sent to the component with OMX_BUFFERFLAG_EOS flag set"));

                }
            }
            else
            {
                //nothing to do here
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentEncTest::RunL() - StateExecuting OUT"));

            RunIfNotReady();
        }
        break;

        /********** STOP THE COMPONENT **********/
        case StateStopping:
        {
            static OMX_BOOL FlagTemp = OMX_FALSE;
            OMX_ERRORTYPE Err = OMX_ErrorNone;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentEncTest::RunL() - StateStopping IN"));

            //stop execution by state transition to Idle state.
            if (!FlagTemp)
            {
                Err = OMX_SendCommand(ipAppPriv->Handle, OMX_CommandStateSet, OMX_StateIdle, NULL);
                CHECK_ERROR(Err, "SendCommand Executing->Idle");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - Sent State Transition Command from Executing->Idle"));

                iPendingCommands = 1;
                FlagTemp = OMX_TRUE;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentEncTest::RunL() - StateStopping OUT"));
        }
        break;

        case StateCleanUp:
        {
            OMX_S32 ii;
            OMX_ERRORTYPE Err = OMX_ErrorNone;
            static OMX_BOOL FlagTemp = OMX_FALSE;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentEncTest::RunL() - StateCleanUp IN"));

            if (!FlagTemp)
            {
                //Added a check here to verify whether all the ip/op buffers are returned back by the component or not
                //in case of Executing->Idle state transition

                for (ii = 0; ii < iInBufferCount; ii++)
                {
                    if (OMX_FALSE == ipInputAvail[ii])
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "OmxComponentEncTest::RunL() - Error, All input buffers not returned back"));

                        iState = StateError;
                        break;
                    }
                }


                for (ii = 0; ii < iOutBufferCount; ii++)
                {
                    if (OMX_FALSE == ipOutReleased[ii])
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "OmxComponentEncTest::RunL() - Error, All output buffers not returned back"));

                        iState = StateError;
                        break;
                    }
                }

                if (StateError == iState)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "OmxComponentEncTest::RunL() - Error occured in this state, StateCleanUp OUT"));

                    RunIfNotReady();
                    break;
                }

                //Destroy the component by state transition to Loaded state
                Err = OMX_SendCommand(ipAppPriv->Handle, OMX_CommandStateSet, OMX_StateLoaded, NULL);
                CHECK_ERROR(Err, "SendCommand Idle->Loaded");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxComponentEncTest::RunL() - Sent State Transition Command from Idle->Loaded"));


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
                                            (0, "OmxComponentEncTest::RunL() - Called FreeBuffer for buffer index %d on port %d", ii, iInputPortIndex));
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
                                            (0, "OmxComponentEncTest::RunL() - Called FreeBuffer for buffer index %d on port %d", ii, iOutputPortIndex));
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

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentEncTest::RunL() - StateCleanUp OUT"));

        }
        break;

        /********* FREE THE HANDLE & CLOSE FILES FOR THE COMPONENT ********/
        case StateStop:
        {
            OMX_U8 TestName[] = "NORMAL_SEQ_TEST";
            OMX_ERRORTYPE Err = OMX_ErrorNone;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentEncTest::RunL() - StateStop IN"));

            if (ipAppPriv)
            {
                if (ipAppPriv->Handle)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxComponentEncTest::RunL() - Free the Component Handle"));

                    Err = OMX_FreeHandle(ipAppPriv->Handle);

                    if (OMX_ErrorNone != Err)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentEncTest::RunL() - FreeHandle Error"));
                        iTestStatus = OMX_FALSE;
                    }
                }
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxComponentEncTest::RunL() - De-initialize the omx component"));

            Err = OMX_Deinit();
            if (OMX_ErrorNone != Err)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxComponentEncTest::RunL() - OMX_Deinit Error"));
                iTestStatus = OMX_FALSE;
            }

            if (ipAppPriv)
            {
                oscl_free(ipAppPriv);
                ipAppPriv = NULL;
            }

            if (ipInputFile)
            {
                fclose(ipInputFile);
                ipInputFile = NULL;
            }

            if (ipOutputFile)
            {
                fclose(ipOutputFile);
                ipOutputFile = NULL;
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
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxEncTestEosMissing::Run() - %s: Fail", TestName));
#ifdef PRINT_RESULT
                printf("%s: Fail \n", TestName);
#endif

            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxEncTestEosMissing::Run() - %s: Success", TestName));
#ifdef PRINT_RESULT
                printf("%s: Success \n", TestName);
#endif
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentEncTest::RunL() - StateStop OUT"));

            iState = StateUnLoaded;
            OsclExecScheduler* sched = OsclExecScheduler::Current();
            sched->StopScheduler();
        }
        break;

        case StateError:
        {
            //Do all the cleanup's and exit from here
            OMX_S32 ii;

            iTestStatus = OMX_FALSE;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentEncTest::RunL() - StateError IN"));

            if (ipInBuffer)
            {
                for (ii = 0; ii < iInBufferCount; ii++)
                {
                    if (ipInBuffer[ii])
                    {
                        OMX_FreeBuffer(ipAppPriv->Handle, iInputPortIndex, ipInBuffer[ii]);
                        ipInBuffer[ii] = NULL;

                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                        (0, "OmxComponentEncTest::RunL() - Called FreeBuffer for buffer index %d on port %d", ii, iInputPortIndex));
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
                                        (0, "OmxComponentEncTest::RunL() - Called FreeBuffer for buffer index %d on port %d", ii, iOutputPortIndex));
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

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentEncTest::RunL() - StateError OUT"));
        }

        break;
    }

    return;
}

OMX_BOOL OmxComponentEncTest::Parse(char aConfigFileName[], OMX_STRING role, char Component[])
{
    OMX_BOOL Status = OMX_TRUE;

    //Open the config file
    ipConfigFile = fopen(aConfigFileName, "rb");
    if (!ipConfigFile)
    {
        printf("Config file open error\n");
        return OMX_FALSE;
    }

    strcpy(iRole, role);
    strcpy(iFormat, Component);

    if (ConfigBuffer)
    {
        const char* line_start_ptr;
        const char* line_end_ptr;
        OMX_S32 line_num = 0;

        while (GetNextLine(line_start_ptr, line_end_ptr))
        {
            if (0 == oscl_strcmp(iFormat, "AMRNB"))
            {
                Status = ParseAmrLine(line_start_ptr, line_end_ptr, (AMR_SETTING_TYPE)line_num);
            }
            else if (0 == oscl_strcmp(iFormat, "M4V"))
            {
                Status = ParseM4vLine(line_start_ptr, line_end_ptr, (M4V_SETTING_TYPE)line_num);
            }
            else if (0 == oscl_strcmp(iFormat, "H264"))
            {
                Status = ParseAvcLine(line_start_ptr, line_end_ptr, (AVC_SETTING_TYPE)line_num);
            }
            else if (0 == oscl_strcmp(iFormat, "AAC"))
            {
                Status = ParseAacLine(line_start_ptr, line_end_ptr, (AAC_SETTING_TYPE)line_num);
            }

            if (OMX_FALSE == Status)
            {
                break;
            }

            line_num++;
        }
    }
    else
    {
        Status = OMX_FALSE;
    }

    return Status;
}

bool OmxComponentEncTest::GetNextLine(const char *& line_start, const char *& line_end)
{
    const char* end_ptr;
    OMX_S32 idx = 0;

    fread((OsclAny*)(&ConfigBuffer[idx++]), 1, 1, ipConfigFile);

    while (!(feof(ipConfigFile)) && ConfigBuffer[idx-1] != '\n')
    {
        fread((OsclAny*)(&ConfigBuffer[idx]), 1, 1, ipConfigFile);
        idx++;
        if (idx > (MAX_SIZE - 1))
        {
            break;
        }
    }

    end_ptr = ConfigBuffer + idx;

    line_start = skip_whitespace_and_line_term(ConfigBuffer, end_ptr);
    line_end = skip_to_line_term(line_start, end_ptr);

    return (line_start < end_ptr);
}

void OmxComponentEncTest::Extract(const char * line_start,
                                  const char * line_end, char * first, int32& len)
{
    const char* temp_end = skip_to_whitespace(line_start, line_end);
    len = temp_end - line_start + 1;

    extract_string(line_start, first, len);
}

OMX_BOOL OmxComponentEncTest::ParseAmrLine(const char* line_start,
        const char* line_end, AMR_SETTING_TYPE line_num)
{
    char temp1[100];
    int32 len1;
    uint32 temp = 0;
    oscl_memset(temp1, 0, sizeof(temp1));

    if (line_start && line_end)
    {
        switch (line_num)
        {
            case INPUT_FILE_NAME_AMR:
            {
                Extract(line_start, line_end, iInputFileName, len1);
            }
            break;
            case OUTPUT_FILE_NAME_AMR:
            {
                Extract(line_start, line_end, iOutputFileName, len1);
            }
            break;
            case INPUT_BITS_PER_SAMPLE:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iInputBitsPerSample = temp;
            }
            break;
            case INPUT_SAMPLING_FREQUENCY:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iInputSamplingRate = temp;
            }
            break;
            break;
            case INPUT_NUMBER_OF_CHANNELS:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iInputNumberOfChannels = temp;
            }
            break;
            case OUTPUT_BAND_MODE:
            {
                Extract(line_start, line_end, temp1, len1);
                if (0 == oscl_strcmp("OMX_AUDIO_AMRBandModeNB0", temp1))
                {
                    iOutputBandMode = OMX_AUDIO_AMRBandModeNB0;
                }
                else if (0 == oscl_strcmp("OMX_AUDIO_AMRBandModeNB1", temp1))
                {
                    iOutputBandMode = OMX_AUDIO_AMRBandModeNB1;
                }
                else if (0 == oscl_strcmp("OMX_AUDIO_AMRBandModeNB2", temp1))
                {
                    iOutputBandMode = OMX_AUDIO_AMRBandModeNB2;
                }
                else if (0 == oscl_strcmp("OMX_AUDIO_AMRBandModeNB3", temp1))
                {
                    iOutputBandMode = OMX_AUDIO_AMRBandModeNB3;
                }
                else if (0 == oscl_strcmp("OMX_AUDIO_AMRBandModeNB4", temp1))
                {
                    iOutputBandMode = OMX_AUDIO_AMRBandModeNB4;
                }
                else if (0 == oscl_strcmp("OMX_AUDIO_AMRBandModeNB5", temp1))
                {
                    iOutputBandMode = OMX_AUDIO_AMRBandModeNB5;
                }
                else if (0 == oscl_strcmp("OMX_AUDIO_AMRBandModeNB6", temp1))
                {
                    iOutputBandMode = OMX_AUDIO_AMRBandModeNB6;
                }
                else if (0 == oscl_strcmp("OMX_AUDIO_AMRBandModeNB7", temp1))
                {
                    iOutputBandMode = OMX_AUDIO_AMRBandModeNB7;
                }
                else
                {
                    printf("Invalid band mode specified, taking the default band mode 0 \n");
                }
            }
            break;
            case OUTPUT_FORMAT:
            {
                Extract(line_start, line_end, temp1, len1);

                if (0 == oscl_strcmp("fsf", temp1))
                {
                    iOutputFormat = OMX_AUDIO_AMRFrameFormatFSF;
                }
                else if (0 == oscl_strcmp("if2", temp1))
                {
                    iOutputFormat = OMX_AUDIO_AMRFrameFormatIF2;
                }
                else
                {
                    /* Invalid input format type */
                    printf("Invalid AMR input format specified, taking the default if2 format\n");
                }
            }
            break;

            default:
                break;
        }
    }

    return OMX_TRUE;
}


OMX_BOOL OmxComponentEncTest::ParseM4vLine(const char* line_start,
        const char* line_end,
        M4V_SETTING_TYPE line_num)
{
    char temp1[100];
    int32 len1;
    uint32 temp = 0;
    oscl_memset(temp1, 0, sizeof(temp1));

    if (line_start && line_end)
    {
        switch (line_num)
        {
            case INPUT_FILE_NAME_M4V:
            {
                Extract(line_start, line_end, iInputFileName, len1);

            }
            break;
            case OUTPUT_FILE_NAME_M4V:
            {
                Extract(line_start, line_end, iOutputFileName, len1);

            }
            break;
            case INPUT_WIDTH_M4V:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iFrameWidth = temp;
            }
            break;
            case INPUT_HEIGHT_M4V:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iFrameWidth = temp;
            }
            break;
            case INPUT_FRAMERATE_M4V:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iFrameRate = temp;
            }
            break;
            case INPUT_FORMAT_M4V:
            {
                Extract(line_start, line_end, temp1, len1);
                if (0 == oscl_strcmp(temp1, "RGB24"))
                {
                    iColorFormat = OMX_COLOR_Format24bitRGB888;
                }
                else if (0 == oscl_strcmp(temp1, "RGB12"))
                {
                    iColorFormat = OMX_COLOR_Format12bitRGB444;
                }
                else if (0 == oscl_strcmp(temp1, "YUV420"))
                {
                    iColorFormat = OMX_COLOR_FormatYUV420Planar;
                }
                else if (0 == oscl_strcmp(temp1, "YUV420SEMIPLANAR"))
                {
                    iColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
                }
                else
                {
                    return OMX_FALSE;
                }


            }
            break;
            case ENCODE_WIDTH_M4V:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iTgtFrameWidth = temp;
            }
            break;
            case ENCODE_HEIGHT_M4V:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iTgtFrameHeight = temp;
            }
            break;
            case ENCODE_BITRATE_M4V:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iTgtBitRate = temp;
            }
            break;
            case ENCODE_FRAMERATE_M4V:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iTgtFrameRate = temp;
            }
            break;
            case ENCODE_NUM_PFRAMES_M4V:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iNumPFrames = temp;

            }
            break;
            case ENCODE_CONTENTTYPE:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iCodecMode = temp;
                if (iCodecMode == 1)
                {
                    oscl_strncpy(iRole, "video_encoder.h263", oscl_strlen("video_encoder.h263") + 1);

                }
            }
            break;
            case ENCODE_RATECONTROLTYPE_M4V:
            {
                Extract(line_start, line_end, temp1, len1);

                if (oscl_strcmp(temp1, "CONSTANT_Q") == 0)
                    iRateControlType = OMX_Video_ControlRateDisable;

                else if (oscl_strcmp(temp1, "VBR") == 0)
                    iRateControlType = OMX_Video_ControlRateVariable;

                else if (oscl_strcmp(temp1, "CBR") == 0)
                    iRateControlType = OMX_Video_ControlRateConstant;

                else    //default setting
                    iRateControlType = OMX_Video_ControlRateConstant;
            }
            break;

            case ENCODE_IQUANT_M4V:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', (uint32&)iIQuant);
            }
            break;
            case ENCODE_PQUANT_M4V:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', (uint32&)iPQuant);
            }
            break;
            case ENCODE_SEARCHRANGE_M4V:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', (uint32&)iSearchRange);
            }
            break;
            case ENCODE_MV8x8:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', (uint32&)iFourMV);
            }
            break;
            case ENCODE_INTRAREFRESHTYPE_M4V:
            {
                Extract(line_start, line_end, temp1, len1);
                if (oscl_strcmp(temp1, "Cyclic") == 0)
                    iRefreshMode = OMX_VIDEO_IntraRefreshCyclic;

                else if (oscl_strcmp(temp1, "Adaptive") == 0)
                    iRefreshMode = OMX_VIDEO_IntraRefreshAdaptive;

                else if (oscl_strcmp(temp1, "Both") == 0)
                    iRefreshMode = OMX_VIDEO_IntraRefreshBoth;

                else    //default setting
                    iRefreshMode = OMX_VIDEO_IntraRefreshCyclic;
            }
            break;
            case ENCODE_NUMINTRAMB_M4V:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', (uint32&)iNumIntraMB);
            }
            break;
            case ENCODE_PACKETSIZE:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', (uint32&)iMaxPacketSize);
            }
            break;
            case ENCODE_PROFILE_M4V:
            {
                Extract(line_start, line_end, temp1, len1);

                if (oscl_strcmp(temp1, "Simple") == 0)
                    iMpeg4Profile = OMX_VIDEO_MPEG4ProfileSimple;

                else if (oscl_strcmp(temp1, "SimpleScalable") == 0)
                    iMpeg4Profile = OMX_VIDEO_MPEG4ProfileSimpleScalable;

                else if (oscl_strcmp(temp1, "Core") == 0)
                    iMpeg4Profile = OMX_VIDEO_MPEG4ProfileCore;

                else if (oscl_strcmp(temp1, "CoreScalable") == 0)
                    iMpeg4Profile = OMX_VIDEO_MPEG4ProfileCoreScalable;

                else // default
                    iMpeg4Profile = OMX_VIDEO_MPEG4ProfileCore;


            }
            break;
            case ENCODE_LEVEL_M4V:
            {
                uint32 level;
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', (uint32&)level);

                if (0 == level)
                {
                    iMpeg4Level = OMX_VIDEO_MPEG4Level0;
                }
                else if (1 == level)
                {
                    iMpeg4Level = OMX_VIDEO_MPEG4Level1;
                }
                else if (2 == level)
                {
                    iMpeg4Level = OMX_VIDEO_MPEG4Level2;
                }
                else if (3 == level)
                {
                    iMpeg4Level = OMX_VIDEO_MPEG4Level3;
                }
                else
                {
                    return OMX_FALSE;
                }
            }
            break;

            case ENCODE_RESYNC:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', (uint32&)iResyncFlag);
            }
            break;
            case ENCODE_DATAPARTITION:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', (uint32&)iDataPartitioningFlag);
            }
            break;
            case ENCODE_SHORTHEADER:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', (uint32&)iShortHeaderFlag);
            }
            break;

            case ENCODE_RESYNC_MARKER_SPACING:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', (uint32&)iResynchMarkerSpacing);
            }
            case ENCODE_REVERSIBLE_VLC:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', (uint32&)iReversibleVLCFlag);
            }
            break;
            case ENCODE_TIME_INCREMENT_RESOLUTION:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', (uint32&)iTimeIncRes);
            }
            break;

            case ENCODE_GOBHEADER_INTERVAL:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', (uint32&)iGobHeaderInterval);
            }

            default:
                break;

        }
    }

    return OMX_TRUE;
}

OMX_BOOL OmxComponentEncTest::ParseAvcLine(const char* line_start,
        const char* line_end,
        AVC_SETTING_TYPE line_num)
{
    char temp1[100];
    int32 len1;
    uint32 temp;
    oscl_memset(temp1, 0, sizeof(temp1));

    if (line_start && line_end)
    {
        switch (line_num)
        {
            case INPUT_FILE_NAME:
            {
                Extract(line_start, line_end, iInputFileName, len1);
            }
            break;
            case OUTPUT_FILE_NAME:
            {
                Extract(line_start, line_end, iOutputFileName, len1);
            }
            break;
            case INPUT_WIDTH:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iFrameWidth = temp;
            }
            break;
            case INPUT_HEIGHT:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iFrameHeight = temp;
            }
            break;
            case INPUT_FRAMERATE:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iFrameRate = temp;
            }
            break;
            case INPUT_FORMAT:
            {
                Extract(line_start, line_end, temp1, len1);
                if (0 == oscl_strcmp(temp1, "RGB24"))
                {
                    iColorFormat = OMX_COLOR_Format24bitRGB888;
                }
                else if (0 == oscl_strcmp(temp1, "RGB12"))
                {
                    iColorFormat = OMX_COLOR_Format12bitRGB444;
                }
                else if (0 == oscl_strcmp(temp1, "YUV420"))
                {
                    iColorFormat = OMX_COLOR_FormatYUV420Planar;
                }
                else if (0 == oscl_strcmp(temp1, "YUV420SEMIPLANAR"))
                {
                    iColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
                }
                else
                {
                    return OMX_FALSE;
                }
            }
            break;
            case ENCODE_WIDTH:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iTgtFrameWidth = temp;
            }
            break;
            case ENCODE_HEIGHT:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iTgtFrameHeight = temp;
            }
            break;
            case ENCODE_BITRATE:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iTgtBitRate = temp;
            }
            break;
            case ENCODE_FRAMERATE:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iTgtFrameRate = temp;
            }
            break;
            case ENCODE_NUM_PFRAMES:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iNumPFrames = temp;
            }
            break;
            case ENCODE_RATECONTROLTYPE:
            {
                Extract(line_start, line_end, temp1, len1);

                if (oscl_strcmp(temp1, "CONSTANT_Q") == 0)
                    iRateControlType = OMX_Video_ControlRateDisable;

                else if (oscl_strcmp(temp1, "VBR") == 0)
                    iRateControlType = OMX_Video_ControlRateVariable;

                else if (oscl_strcmp(temp1, "CBR") == 0)
                    iRateControlType = OMX_Video_ControlRateConstant;

                else    //default setting
                    iRateControlType = OMX_Video_ControlRateConstant;
            }
            break;

            case ENCODE_PQUANT:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iPQuant = temp;
            }
            break;
            case ENCODE_SEARCHRANGE:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iSearchRange = temp;
            }
            break;

            case ENCODE_INTRAREFRESHTYPE:
            {
                Extract(line_start, line_end, temp1, len1);
                if (oscl_strcmp(temp1, "Cyclic") == 0)
                    iRefreshMode = OMX_VIDEO_IntraRefreshCyclic;

                else if (oscl_strcmp(temp1, "Adaptive") == 0)
                    iRefreshMode = OMX_VIDEO_IntraRefreshAdaptive;

                else if (oscl_strcmp(temp1, "Both") == 0)
                    iRefreshMode = OMX_VIDEO_IntraRefreshBoth;

                else    //default setting
                    iRefreshMode = OMX_VIDEO_IntraRefreshCyclic;
            }
            break;
            case ENCODE_NUMINTRAMB:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iNumIntraMB = temp;
            }
            break;

            case ENCODE_PROFILE:
            {
                Extract(line_start, line_end, temp1, len1);

                if (0 == oscl_strcmp(temp1, "Baseline"))
                {
                    iAvcProfile = OMX_VIDEO_AVCProfileBaseline;
                }
                else if (0 == oscl_strcmp(temp1, "Main"))
                {
                    iAvcProfile = OMX_VIDEO_AVCProfileMain;
                }
                else if (0 == oscl_strcmp(temp1, "Extended"))
                {
                    iAvcProfile = OMX_VIDEO_AVCProfileExtended;
                }
                else if (0 == oscl_strcmp(temp1, "High"))
                {
                    iAvcProfile = OMX_VIDEO_AVCProfileHigh;
                }
                else if (0 == oscl_strcmp(temp1, "High10"))
                {
                    iAvcProfile = OMX_VIDEO_AVCProfileHigh10;
                }
                else if (0 == oscl_strcmp(temp1, "High422"))
                {
                    iAvcProfile = OMX_VIDEO_AVCProfileHigh422;
                }
                else if (0 == oscl_strcmp(temp1, "High444"))
                {
                    iAvcProfile = OMX_VIDEO_AVCProfileHigh444;
                }
                else // default
                {
                    iAvcProfile = OMX_VIDEO_AVCProfileBaseline;
                }
            }
            break;
            case ENCODE_LEVEL:
            {
                Extract(line_start, line_end, temp1, len1);

                if (0 == oscl_strcmp(temp1, "1"))
                {
                    iAvcLevel = OMX_VIDEO_AVCLevel1;
                }
                else if (0 == oscl_strcmp(temp1, "1b"))
                {
                    iAvcLevel = OMX_VIDEO_AVCLevel1b;
                }
                else if (0 == oscl_strcmp(temp1, "11"))
                {
                    iAvcLevel = OMX_VIDEO_AVCLevel11;
                }
                else if (0 == oscl_strcmp(temp1, "12"))
                {
                    iAvcLevel = OMX_VIDEO_AVCLevel12;
                }
                else if (0 == oscl_strcmp(temp1, "13"))
                {
                    iAvcLevel = OMX_VIDEO_AVCLevel13;
                }
                else if (0 == oscl_strcmp(temp1, "2"))
                {
                    iAvcLevel = OMX_VIDEO_AVCLevel2;
                }
                else if (0 == oscl_strcmp(temp1, "21"))
                {
                    iAvcLevel = OMX_VIDEO_AVCLevel21;
                }

                else if (0 == oscl_strcmp(temp1, "22"))
                {
                    iAvcLevel = OMX_VIDEO_AVCLevel22;
                }
                else if (0 == oscl_strcmp(temp1, "3"))
                {
                    iAvcLevel = OMX_VIDEO_AVCLevel3;
                }
                else if (0 == oscl_strcmp(temp1, "31"))
                {
                    iAvcLevel = OMX_VIDEO_AVCLevel31;
                }
                else if (0 == oscl_strcmp(temp1, "32"))
                {
                    iAvcLevel = OMX_VIDEO_AVCLevel32;
                }
                else if (0 == oscl_strcmp(temp1, "4"))
                {
                    iAvcLevel = OMX_VIDEO_AVCLevel4;
                }
                else if (0 == oscl_strcmp(temp1, "41"))
                {
                    iAvcLevel = OMX_VIDEO_AVCLevel41;
                }
                else if (0 == oscl_strcmp(temp1, "42"))
                {
                    iAvcLevel = OMX_VIDEO_AVCLevel42;
                }

                else if (0 == oscl_strcmp(temp1, "5"))
                {
                    iAvcLevel = OMX_VIDEO_AVCLevel5;
                }
                else if (0 == oscl_strcmp(temp1, "51"))
                {
                    iAvcLevel = OMX_VIDEO_AVCLevel51;
                }

                else // default
                {
                    iAvcLevel = OMX_VIDEO_AVCLevel1b;
                }

            }
            break;

            case ENCODE_LOOP_FILTER_MODE:
            {
                Extract(line_start, line_end, temp1, len1);

                if (0 == oscl_strcmp(temp1, "Enable"))
                {
                    iLoopFilterType = OMX_VIDEO_AVCLoopFilterEnable;
                }
                else if (0 == oscl_strcmp(temp1, "Disable"))
                {
                    iLoopFilterType = OMX_VIDEO_AVCLoopFilterDisable;
                }
                else if (0 == oscl_strcmp(temp1, "DisableSliceBoundary"))
                {
                    iLoopFilterType = OMX_VIDEO_AVCLoopFilterDisableSliceBoundary;
                }
                else    //return error
                {
                    return OMX_FALSE;
                }

            }
            break;
            case ENCODE_CONST_IPRED:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iIPredictionFlag = (temp > 0) ? OMX_TRUE : OMX_FALSE;
            }
            break;
            case ENCODE_ACCURACY:
            {
                Extract(line_start, line_end, temp1, len1);

                if (0 == oscl_strcmp(temp1, "Pixel"))
                {
                    iAccuracy = OMX_Video_MotionVectorPixel;
                }
                else if (0 == oscl_strcmp(temp1, "HalfPel"))
                {
                    iAccuracy = OMX_Video_MotionVectorHalfPel;
                }
                else if (0 == oscl_strcmp(temp1, "QuarterPel"))
                {
                    iAccuracy = OMX_Video_MotionVectorQuarterPel;
                }
                else if (0 == oscl_strcmp(temp1, "EighthPel"))
                {
                    iAccuracy = OMX_Video_MotionVectorEighthPel;
                }
                else    //return error
                {
                    return OMX_FALSE;
                }
            }
            break;

            default:
                break;

        }
    }

    return OMX_TRUE;
}


OMX_BOOL OmxComponentEncTest::ParseAacLine(const char* line_start,
        const char* line_end, AAC_SETTING_TYPE line_num)
{
    char temp1[100];
    int32 len1;
    uint32 temp = 0;

    oscl_memset(temp1, 0, sizeof(temp1));

    if (line_start && line_end)
    {
        switch (line_num)
        {
            case INPUT_FILE_NAME_AAC:
            {
                Extract(line_start, line_end, iInputFileName, len1);
            }
            break;
            case OUTPUT_FILE_NAME_AAC:
            {
                Extract(line_start, line_end, iOutputFileName, len1);
            }
            break;
            case IP_NUMBER_OF_CHANNELS:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iInputNumberOfChannels = temp;
            }
            break;
            case OP_NUMBER_OF_CHANNELS:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iOutputNumberOfChannels = temp;
            }
            break;
            case SAMPLING_FREQ:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iInputSamplingRate = temp;
            }
            break;
            case BIT_RATE:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iTgtBitRate = temp;
            }
            break;
            case AUDIO_BANDWIDTH:
            {
                Extract(line_start, line_end, temp1, len1);
                PV_atoi(temp1, 'd', temp);
                iAacBandWidth = temp;
            }
            break;
            case CHANNEL_MODE:
            {
                Extract(line_start, line_end, temp1, len1);
                if (0 == oscl_strcmp("Mono", temp1))
                {
                    iChannelMode = OMX_AUDIO_ChannelModeMono;
                }
                else if (0 == oscl_strcmp("Dual", temp1))
                {
                    iChannelMode = OMX_AUDIO_ChannelModeDual;
                }
                else if (0 == oscl_strcmp("Stereo", temp1))
                {
                    iChannelMode = OMX_AUDIO_ChannelModeStereo;
                }
                else if (0 == oscl_strcmp("JointStereo", temp1))
                {
                    iChannelMode = OMX_AUDIO_ChannelModeJointStereo;
                }
                else
                {
                    printf("Invalid channel mode specified, taking the default channel mode Stereo \n");
                }
            }
            break;
            case AAC_PROFILE:
            {
                Extract(line_start, line_end, temp1, len1);

                if (0 == oscl_strcmp("Main", temp1))
                {
                    iAacProfile = OMX_AUDIO_AACObjectMain;
                }
                else if (0 == oscl_strcmp("LC", temp1))
                {
                    iAacProfile = OMX_AUDIO_AACObjectLC;
                }
                else if (0 == oscl_strcmp("SSR", temp1))
                {
                    iAacProfile = OMX_AUDIO_AACObjectSSR;
                }
                else if (0 == oscl_strcmp("LTP", temp1))
                {
                    iAacProfile = OMX_AUDIO_AACObjectLTP;
                }
                else if (0 == oscl_strcmp("HE", temp1))
                {
                    iAacProfile = OMX_AUDIO_AACObjectHE;
                }
                else if (0 == oscl_strcmp("HE_PS", temp1))
                {
                    iAacProfile = OMX_AUDIO_AACObjectHE_PS;
                }
                else if (0 == oscl_strcmp("Scalable", temp1))
                {
                    iAacProfile = OMX_AUDIO_AACObjectScalable;
                }
                else if (0 == oscl_strcmp("LD", temp1))
                {
                    iAacProfile = OMX_AUDIO_AACObjectLD;
                }
                else if (0 == oscl_strcmp("ERLC", temp1))
                {
                    iAacProfile = OMX_AUDIO_AACObjectERLC;
                }
                else
                {
                    /* Invalid input format type */
                    printf("Invalid Aac Profile type specified, taking the default LC profile\n");
                }
            }
            break;
            case AAC_STREAM_FORMAT:
            {
                Extract(line_start, line_end, temp1, len1);

                if (0 == oscl_strcmp("MP2ADTS", temp1))
                {
                    iAacStreamFormat = OMX_AUDIO_AACStreamFormatMP2ADTS;
                }
                else if (0 == oscl_strcmp("MP4ADTS", temp1))
                {
                    iAacStreamFormat = OMX_AUDIO_AACStreamFormatMP4ADTS;
                }
                else if (0 == oscl_strcmp("MP4LOAS", temp1))
                {
                    iAacStreamFormat = OMX_AUDIO_AACStreamFormatMP4LOAS;
                }
                else if (0 == oscl_strcmp("MP4LATM", temp1))
                {
                    iAacStreamFormat = OMX_AUDIO_AACStreamFormatMP4LATM;
                }
                else if (0 == oscl_strcmp("ADIF", temp1))
                {
                    iAacStreamFormat = OMX_AUDIO_AACStreamFormatADIF;
                }
                else if (0 == oscl_strcmp("MP4FF", temp1))
                {
                    iAacStreamFormat = OMX_AUDIO_AACStreamFormatMP4FF;
                }
                else if (0 == oscl_strcmp("RAW", temp1))
                {
                    iAacStreamFormat = OMX_AUDIO_AACStreamFormatRAW;
                }
                else
                {
                    /* Invalid input format type */
                    printf("InvalidAAC stream format specified, taking the default MP2ADTS format\n");
                }
            }
            break;
            case AAC_TOOL:
            {
                Extract(line_start, line_end, temp1, len1);

                if (0 == oscl_strcmp("None", temp1))
                {
                    iAacTools = OMX_AUDIO_AACToolNone;
                }
                else if (0 == oscl_strcmp("MS", temp1))
                {
                    iAacTools = OMX_AUDIO_AACToolMS;
                }
                else if (0 == oscl_strcmp("IS", temp1))
                {
                    iAacTools = OMX_AUDIO_AACToolIS;
                }
                else if (0 == oscl_strcmp("TNS", temp1))
                {
                    iAacTools = OMX_AUDIO_AACToolTNS;
                }
                else if (0 == oscl_strcmp("PNS", temp1))
                {
                    iAacTools = OMX_AUDIO_AACToolPNS;
                }
                else if (0 == oscl_strcmp("LTP", temp1))
                {
                    iAacTools = OMX_AUDIO_AACToolLTP;
                }
                else if (0 == oscl_strcmp("All", temp1))
                {
                    iAacTools = OMX_AUDIO_AACToolAll;
                }
                else
                {
                    /* Invalid input format type */
                    printf("Invalid AAC tool specified, taking the default value OMX_AUDIO_AACToolAllt\n");
                }
            }
            break;
            default:
                break;
        }
    }

    return OMX_TRUE;
}



template<class DestructClass>
class LogAppenderDestructDealloc : public OsclDestructDealloc
{
    public:
        virtual void destruct_and_dealloc(OsclAny *ptr)
        {
            delete((DestructClass*)ptr);
        }
};

// Main entry point for the code
int main(int argc, char** argv)
{
    char ConfigFileName[200] = "\0";
    OMX_STRING Role = NULL;
    int ArgIndex = 0, FirstTest, LastTest;
    OmxComponentEncTest* pTestApp;

    char ComponentFormat[10] = "\0";


    OMX_BOOL IsLogFile = OMX_FALSE;

    // OSCL Initializations
    OsclBase::Init();
    OsclErrorTrap::Init();
    OsclMem::Init();
    PVLogger::Init();

    if (argc > 1)
    {
        oscl_strncpy(ConfigFileName, argv[1], oscl_strlen(argv[1]) + 1);


        //default is to run all tests.
        FirstTest = 0;
        LastTest = NUMBER_TEST_CASES;

        ArgIndex = 2;

        while (ArgIndex < argc)
        {
            if ('-' == *(argv[ArgIndex]))
            {
                switch (argv[ArgIndex][1])
                {
                    case 't':
                    {
                        ArgIndex++;
                        FirstTest = atoi(argv[ArgIndex++]);
                        LastTest = atoi(argv[ArgIndex++]);
                    }
                    break;

                    case 'c':
                    {
                        char* Result;
                        ArgIndex++;

                        Role = (OMX_STRING) oscl_malloc(PV_OMX_MAX_COMPONENT_NAME_LENGTH * sizeof(OMX_U8));

                        if (NULL != (Result = (OMX_STRING) oscl_strstr(argv[ArgIndex], "avc")))
                        {
                            oscl_strncpy(Role, "video_encoder.avc", oscl_strlen("video_encoder.avc") + 1);
                            oscl_strncpy(ComponentFormat, "H264", oscl_strlen("H264") + 1);
                        }
                        else if (NULL != (Result = (OMX_STRING) oscl_strstr(argv[ArgIndex], "mpeg4")))
                        {
                            oscl_strncpy(Role, "video_encoder.mpeg4", oscl_strlen("video_encoder.mpeg4") + 1);
                            oscl_strncpy(ComponentFormat, "M4V", oscl_strlen("M4V") + 1);
                        }
                        else if (NULL != (Result = (OMX_STRING) oscl_strstr(argv[ArgIndex], "amr")))
                        {
                            oscl_strncpy(Role, "audio_encoder.amrnb", oscl_strlen("audio_encoder.amrnb") + 1);
                            oscl_strncpy(ComponentFormat, "AMRNB", oscl_strlen("AMRNB") + 1);
                        }
                        else if (NULL != (Result = (OMX_STRING) oscl_strstr(argv[ArgIndex], "aac")))
                        {
                            oscl_strncpy(Role, "audio_encoder.aac", oscl_strlen("audio_encoder.aac") + 1);
                            oscl_strncpy(ComponentFormat, "AAC", oscl_strlen("AAC") + 1);
                        }
                        else
                        {
                            printf("Unsupported component type\n");
                            exit(1);
                        }

                        ArgIndex++;
                    }
                    break;

                    default:
                    {
                        printf("Usage: Config_file {options}\n");
                        printf("Option{} \n");
                        printf("-c      {CodecType}\n");
                        printf("        CodecType could be avc,mpeg4,amr, aac\n");
                        printf("-t x y  {A range of test cases to run}\n ");
                        printf("       {To run one test case use same index for x and y} {Default is ALL}\n");

                        // Clean OSCL
                        PVLogger::Cleanup();
                        OsclErrorTrap::Cleanup();
                        OsclMem::Cleanup();
                        OsclBase::Cleanup();

                        exit(-1);
                    }
                    break;
                }
            }
            else
            {
                printf("Usage: Config_file {options}\n");
                printf("Option{} \n");
                printf("-c      {CodecType}\n");
                printf("        CodecType could be avc, mpeg4, amr, aac\n");
                printf("-t x y  {A range of test cases to run}\n ");
                printf("       {To run one test case use same index for x and y} {Default is ALL}\n");

                // Clean OSCL
                PVLogger::Cleanup();
                OsclErrorTrap::Cleanup();
                OsclMem::Cleanup();
                OsclBase::Cleanup();
                exit(-1);
            }
        }
    }
    else
    {
        printf("Usage: Config_file {options}\n");
        printf("Option{} \n");
        printf("-c      {CodecType}\n");
        printf("        CodecType could be avc, mpeg4, amr, aac\n");
        printf("-t x y  {A range of test cases to run}\n ");
        printf("       {To run one test case use same index for x and y} {Default is ALL}\n");

        // Clean OSCL
        PVLogger::Cleanup();
        OsclErrorTrap::Cleanup();
        OsclMem::Cleanup();
        OsclBase::Cleanup();
        exit(-1);
    }

    OMX_BOOL InitSchedulerFlag = OMX_FALSE;

    int CurrentTestNumber = FirstTest;

//This scope operator will make sure that LogAppenderDestructDealloc is called before the Logger cleanup

    {
        //Enable the following code for logging
        PVLoggerAppender *appender = NULL;
        OsclRefCounter *refCounter = NULL;

        if (IsLogFile)
        {
            OSCL_TCHAR LogFileName[200] = {'l', 'o', 'g', 'f', 'i', 'l', 'e', '.', 't', 'x', 't'};
            appender = (PVLoggerAppender*)TextFileAppender<TimeAndIdLayout, 1024>::CreateAppender((oscl_wchar*)(LogFileName));
            OsclRefCounterSA<LogAppenderDestructDealloc<TextFileAppender<TimeAndIdLayout, 1024> > > *appenderRefCounter =
                new OsclRefCounterSA<LogAppenderDestructDealloc<TextFileAppender<TimeAndIdLayout, 1024> > >(appender);
            refCounter = appenderRefCounter;

        }
        else
        {
            appender = new StdErrAppender<TimeAndIdLayout, 1024>();
            OsclRefCounterSA<LogAppenderDestructDealloc<StdErrAppender<TimeAndIdLayout, 1024> > > *appenderRefCounter =
                new OsclRefCounterSA<LogAppenderDestructDealloc<StdErrAppender<TimeAndIdLayout, 1024> > >(appender);
            refCounter = appenderRefCounter;

        }

        OsclSharedPtr<PVLoggerAppender> appenderPtr(appender, refCounter);

        //Log all the loggers
        PVLogger *rootnode = PVLogger::GetLoggerObject("");
        rootnode->AddAppender(appenderPtr);
        rootnode->SetLogLevel(PVLOGMSG_DEBUG);

        while (CurrentTestNumber <= LastTest)
        {
            // Shutdown PVLogger and scheduler before checking mem stats

            switch (CurrentTestNumber)
            {
                case GET_ROLES_TEST:
                {
                    printf("\nStarting test %4d: GET_ROLES_TEST \n", CurrentTestNumber);

                    pTestApp = OSCL_NEW(OmxEncTestCompRole, ());

                    pTestApp->Parse(ConfigFileName, Role, ComponentFormat);

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pTestApp->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pTestApp->StartTestApp();

                    OSCL_DELETE(pTestApp);
                    pTestApp = NULL;
                    CurrentTestNumber++;
                }
                break;

                case PARAM_NEGOTIATION_TEST:
                {
                    printf("\nStarting test %4d: PARAM_NEGOTIATION_TEST\n", CurrentTestNumber);

                    pTestApp = OSCL_NEW(OmxEncTestBufferNegotiation, ());

                    pTestApp->Parse(ConfigFileName, Role, ComponentFormat);

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pTestApp->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pTestApp->StartTestApp();

                    OSCL_DELETE(pTestApp);
                    pTestApp = NULL;

                    CurrentTestNumber++;
                }
                break;

                case NORMAL_SEQ_TEST:
                {
                    printf("\nStarting test %4d: NORMAL_SEQ_TEST \n", CurrentTestNumber);

                    pTestApp = OSCL_NEW(OmxComponentEncTest, ());

                    pTestApp->Parse(ConfigFileName, Role, ComponentFormat);

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pTestApp->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pTestApp->StartTestApp();

                    OSCL_DELETE(pTestApp);
                    pTestApp = NULL;


                    CurrentTestNumber++;
                }
                break;

                case USE_BUFFER_TEST:
                {
                    printf("\nStarting test %4d: USE_BUFFER_TEST \n", CurrentTestNumber);

                    pTestApp = OSCL_NEW(OmxEncTestUseBuffer, ());

                    pTestApp->Parse(ConfigFileName, Role, ComponentFormat);

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pTestApp->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pTestApp->StartTestApp();

                    OSCL_DELETE(pTestApp);
                    pTestApp = NULL;

                    CurrentTestNumber++;
                }
                break;

                case BUFFER_BUSY_TEST:
                {
                    printf("\nStarting test %4d: BUFFER_BUSY_TEST\n", CurrentTestNumber);

                    pTestApp = OSCL_NEW(OmxEncTestBufferBusy, ());

                    pTestApp->Parse(ConfigFileName, Role, ComponentFormat);

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pTestApp->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pTestApp->StartTestApp();

                    OSCL_DELETE(pTestApp);
                    pTestApp = NULL;

                    CurrentTestNumber++;
                }
                break;


                case PARTIAL_FRAMES_TEST:
                {
                    printf("\nStarting test %4d: PARTIAL_FRAMES_TEST \n", CurrentTestNumber);

                    pTestApp = OSCL_NEW(OmxEncTestPartialFrames, ());

                    pTestApp->Parse(ConfigFileName, Role, ComponentFormat);

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pTestApp->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pTestApp->StartTestApp();

                    OSCL_DELETE(pTestApp);
                    pTestApp = NULL;
                    CurrentTestNumber++;
                }
                break;

                case EXTRA_PARTIAL_FRAMES_TEST:
                {
                    printf("\nStarting test %4d: EXTRA_PARTIAL_FRAMES_TEST \n", CurrentTestNumber);

                    pTestApp = OSCL_NEW(OmxEncTestExtraPartialFrames, ());

                    pTestApp->Parse(ConfigFileName, Role, ComponentFormat);

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pTestApp->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pTestApp->StartTestApp();

                    OSCL_DELETE(pTestApp);
                    pTestApp = NULL;
                    CurrentTestNumber++;
                }
                break;

                case PAUSE_RESUME_TEST:
                {
                    printf("\nStarting test %4d: PAUSE_RESUME_TEST\n", CurrentTestNumber);

                    pTestApp = OSCL_NEW(OmxEncTestPauseResume, ());

                    pTestApp->Parse(ConfigFileName, Role, ComponentFormat);

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pTestApp->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pTestApp->StartTestApp();

                    OSCL_DELETE(pTestApp);
                    pTestApp = NULL;

                    CurrentTestNumber++;
                }
                break;

                case ENDOFSTREAM_MISSING_TEST:
                {
                    printf("\nStarting test %4d: ENDOFSTREAM_MISSING_TEST \n", CurrentTestNumber);

                    pTestApp = OSCL_NEW(OmxEncTestEosMissing, ());

                    pTestApp->Parse(ConfigFileName, Role, ComponentFormat);

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pTestApp->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pTestApp->StartTestApp();

                    OSCL_DELETE(pTestApp);
                    pTestApp = NULL;

                    CurrentTestNumber++;
                }
                break;

                case WITHOUT_MARKER_BIT_TEST:
                {
                    printf("\nStarting test %4d: WITHOUT_MARKER_BIT_TEST \n", CurrentTestNumber);

                    pTestApp = OSCL_NEW(OmxEncTestWithoutMarker, ());

                    pTestApp->Parse(ConfigFileName, Role, ComponentFormat);

                    if (OMX_FALSE == InitSchedulerFlag)
                    {
                        pTestApp->InitScheduler();
                        InitSchedulerFlag = OMX_TRUE;
                    }

                    pTestApp->StartTestApp();

                    OSCL_DELETE(pTestApp);
                    pTestApp = NULL;
                    CurrentTestNumber++;
                }
                break;

                default:
                {
                    // just skip the count
                    CurrentTestNumber++;
                }
                break;
            }
        }


        if (Role)
        {
            oscl_free(Role);
            Role = NULL;
        }
    }


    // Clean OSCL
    PVLogger::Cleanup();
    OsclErrorTrap::Cleanup();
    OsclMem::Cleanup();
    OsclBase::Cleanup();

    return 0;
}

void OmxComponentEncTest::StopOnError()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxComponentEncTest::StopOnError() called"));
    iState = StateError;
    RunIfNotReady();
}
