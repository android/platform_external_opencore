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

#define MAX_TEST_LOOP_COUNT 100

/* Negotiate and Verifify the parameters on both the ports*/
OMX_BOOL OmxEncTestBufferNegotiation::NegotiateParameters()
{
    OMX_ERRORTYPE Err;
    OMX_S32 NumPorts, ii;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - IN"));

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Testing the component for number of ports supported for audio/video format"));

    //First get the port indices and do the port verification
    //For AUDIO Component
    if (OMX_TRUE == iIsAudioFormat)
    {
        //In case of audio component, the videoinit call should not fail but return 0 ports
        OMX_PORT_PARAM_TYPE VideoPortParameters;
        //This will initialize the size and version of the VideoPortParameters structure
        INIT_GETPARAMETER_STRUCT(OMX_PORT_PARAM_TYPE, VideoPortParameters);

        Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamVideoInit, &VideoPortParameters);
        NumPorts = VideoPortParameters.nPorts;

        if (OMX_ErrorNone != Err || NumPorts > 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Video Initialization returned error, Num of ports %d, OUT", NumPorts));
            return OMX_FALSE;
        }

        INIT_GETPARAMETER_STRUCT(OMX_PORT_PARAM_TYPE, iPortInit);

        Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamAudioInit, &iPortInit);
        NumPorts = iPortInit.nPorts;    //must be at least 2 of them (for in & out)
        if (Err != OMX_ErrorNone || NumPorts < 2)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Audio Initialization returned error, Num of ports %d, OUT", NumPorts));
            return OMX_FALSE;
        }
    }
    //For VIDEO Component
    else
    {
        //In case of video component, the audioinit call should not fail but return 0 ports
        OMX_PORT_PARAM_TYPE AudioPortParameters;

        //This will initialize the size and version of the AudioPortParameters structure
        INIT_GETPARAMETER_STRUCT(OMX_PORT_PARAM_TYPE, AudioPortParameters);

        Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamAudioInit, &AudioPortParameters);
        NumPorts = AudioPortParameters.nPorts;

        if (OMX_ErrorNone != Err || NumPorts > 0)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Audio Initialization returned error, Num of ports %d, OUT", NumPorts));
            return OMX_FALSE;
        }

        INIT_GETPARAMETER_STRUCT(OMX_PORT_PARAM_TYPE, iPortInit);
        Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamVideoInit, &iPortInit);

        NumPorts = iPortInit.nPorts;    //must be at least 2 of them (for in & out)
        if (Err != OMX_ErrorNone || NumPorts < 2)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Video Initialization returned error, Num of ports %d, OUT", NumPorts));
            return OMX_FALSE;
        }

    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Finding the input and ouput port index"));

    //Loop through ports starting from the starting index to find index of the first input port
    for (ii = iPortInit.nStartPortNumber; ii < iPortInit.nStartPortNumber + NumPorts; ii++)
    {
        // get port parameters, and determine if it is input or output
        // if there are more than 2 ports, the first one we encounter that has input direction is picked

        INIT_GETPARAMETER_STRUCT(OMX_PARAM_PORTDEFINITIONTYPE, iParamPort);
        iParamPort.nPortIndex = ii;

        Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamPortDefinition, &iParamPort);
        if (Err != OMX_ErrorNone)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Port %d returned ERROR for OMX_IndexParamPortDefinition GetParameter, OUT", ii));
            return OMX_FALSE;
        }

        if (OMX_DirInput == iParamPort.eDir)
        {
            iInputPortIndex = ii;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Input port found at index %d", iInputPortIndex));

            break;
        }
    }

    if (ii == iPortInit.nStartPortNumber + NumPorts)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - No Input Port found for this component, OUT"));
        return OMX_FALSE;
    }


    //Loop through ports starting from the starting index to find index of the first output port
    for (ii = iPortInit.nStartPortNumber; ii < iPortInit.nStartPortNumber + NumPorts; ii++)
    {
        // get port parameters, and determine if it is input or output
        // if there are more than 2 ports, the first one we encounter that has output direction is picked

        INIT_GETPARAMETER_STRUCT(OMX_PARAM_PORTDEFINITIONTYPE, iParamPort);

        iParamPort.nPortIndex = ii;
        Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamPortDefinition, &iParamPort);

        if (Err != OMX_ErrorNone)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Port %d returned ERROR for OMX_IndexParamPortDefinition GetParameter, OUT", ii));
            return OMX_FALSE;
        }

        if (OMX_DirOutput == iParamPort.eDir)
        {
            iOutputPortIndex = ii;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Output port found at index %d", iOutputPortIndex));

            break;
        }
    }

    if (ii == iPortInit.nStartPortNumber + NumPorts)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - No Output Port found for this component, OUT"));
        return OMX_FALSE;
    }


    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Now verifying the buffer parameters on the Input Port"));

    //Now get INPUT parameters
    INIT_GETPARAMETER_STRUCT(OMX_PARAM_PORTDEFINITIONTYPE, iParamPort);
    iParamPort.nPortIndex = iInputPortIndex;

    Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamPortDefinition, &iParamPort);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Port %d returned ERROR for OMX_IndexParamPortDefinition GetParameter, OUT", iInputPortIndex));
        return OMX_FALSE;
    }

    if (0 == iParamPort.nBufferCountMin)
    {
        /* a buffer count of 0 is not allowed */
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Error, Port %d returned 0 min buffer count, OUT", iInputPortIndex));
        return OMX_FALSE;
    }

    if (iParamPort.nBufferCountMin > iParamPort.nBufferCountActual)
    {
        /* Min buff count can't be more than actual buff count */
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Port %d  returned actual buffer count %d less than min buffer count %d, OUT",
                         iInputPortIndex, iParamPort.nBufferCountActual, iParamPort.nBufferCountMin));

        return OMX_FALSE;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Trying to Set new buffer parameters on the input port"));

    //Make the changes in buffer count & sizes
    // Change size only for audio components, as for video, size is dependent upon WxH
    // and cannot be changed independently
    if (OMX_TRUE == iIsAudioFormat)
    {
        iInBufferSize = iParamPort.nBufferSize + 1000;
    }
    else
    {
        iInBufferSize = iParamPort.nBufferSize;
    }

    iInBufferCount = iParamPort.nBufferCountActual + 2;

    iParamPort.nBufferCountActual = iInBufferCount;
    iParamPort.nBufferSize = iInBufferSize;

    iParamPort.nPortIndex = iInputPortIndex;

    // set the number of actual input buffers and their sizes
    Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamPortDefinition, &iParamPort);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Port %d returned ERROR for OMX_IndexParamPortDefinition SetParameter, OUT", iInputPortIndex));
        return OMX_FALSE;
    }

    //Verify whether the parameters have been set prpoerly or not
    INIT_GETPARAMETER_STRUCT(OMX_PARAM_PORTDEFINITIONTYPE, iParamPort);
    Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamPortDefinition, &iParamPort);

    if ((OMX_ErrorNone != Err) || (iParamPort.nBufferCountActual != iInBufferCount) ||
            (iParamPort.nBufferSize != iInBufferSize))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Buffer parameter verificication failed with Get/Set Parameter combination on port %d, OUT", iInputPortIndex));
        return OMX_FALSE;
    }


    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Now verifying the buffer parameters on the Output Port"));

    //NOW GET OUTPUT PARAMETERS
    INIT_GETPARAMETER_STRUCT(OMX_PARAM_PORTDEFINITIONTYPE, iParamPort);
    iParamPort.nPortIndex = iOutputPortIndex;

    Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamPortDefinition, &iParamPort);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Port %d returned ERROR for OMX_IndexParamPortDefinition GetParameter, OUT", iOutputPortIndex));
        return OMX_FALSE;
    }

    if (0 == iParamPort.nBufferCountMin)
    {
        /* a buffer count of 0 is not allowed */
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Error, Port %d returned 0 min buffer count, OUT", iOutputPortIndex));
        return OMX_FALSE;
    }

    if (iParamPort.nBufferCountMin > iParamPort.nBufferCountActual)
    {
        /* Min buff count can't be more than actual buff count */
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Port %d  returned actual buffer count %d less than min buffer count %d, OUT",
                         iOutputPortIndex, iParamPort.nBufferCountActual, iParamPort.nBufferCountMin));
        return OMX_FALSE;
    }


    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Trying to Set new buffer parameters on the output port"));

    //Make the changes in buffer count & sizes
    iOutBufferSize = iParamPort.nBufferSize + 1000;
    iOutBufferCount = iParamPort.nBufferCountActual + 2;

    iParamPort.nBufferCountActual = iOutBufferCount;
    iParamPort.nBufferSize = iOutBufferSize;

    iParamPort.nPortIndex = iOutputPortIndex;

    // set the number of actual output buffers & their sizes
    Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamPortDefinition, &iParamPort);
    if (OMX_ErrorNone != Err)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Port %d returned ERROR for OMX_IndexParamPortDefinition SetParameter, OUT", iOutputPortIndex));
        return OMX_FALSE;
    }

    //Verify whether the parameters have been set prpoerly or not
    INIT_GETPARAMETER_STRUCT(OMX_PARAM_PORTDEFINITIONTYPE, iParamPort);
    iParamPort.nPortIndex = iOutputPortIndex;

    Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamPortDefinition, &iParamPort);
    if ((OMX_ErrorNone != Err) || (iParamPort.nBufferCountActual != iOutBufferCount)
            || (iParamPort.nBufferSize != iOutBufferSize))
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - Buffer parameter verificication failed with Get/Set Parameter combination on port %d, OUT", iOutputPortIndex));
        return OMX_FALSE;
    }

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxEncTestBufferNegotiation::NegotiateParameters() - OUT"));
    return OMX_TRUE;
}


OMX_BOOL OmxEncTestBufferNegotiation::ParamTestVideoPort()
{

    OMX_U32 ii = 0;
    OMX_U32 Index  = 0;
    OMX_U32 PortIndex;
    OMX_ERRORTYPE Err = OMX_ErrorNone;

    OMX_VIDEO_PARAM_PORTFORMATTYPE iParamPortFormat;
    OMX_VIDEO_PARAM_AVCTYPE FormatAvc;
    OMX_VIDEO_PARAM_MPEG4TYPE FormatMpeg4;
    OMX_VIDEO_PARAM_WMVTYPE FormatWmv;
    OMX_VIDEO_PARAM_H263TYPE FormatH263;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - IN"));

    /* process port definitions and parameters on all video ports */
    for (ii = 0; ii < iPortInit.nPorts; ii++)
    {
        PortIndex = iPortInit.nStartPortNumber + ii;
        Index  = 0;
        Err = OMX_ErrorNone;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - Looping through all the formats supported by OMX_IndexParamVideoPortFormat on port %d", PortIndex));

        INIT_GETPARAMETER_STRUCT(OMX_VIDEO_PARAM_PORTFORMATTYPE, iParamPortFormat);
        iParamPortFormat.nPortIndex = PortIndex;

        while ((OMX_ErrorNoMore != Err) && (Index < MAX_TEST_LOOP_COUNT))
        {
            iParamPortFormat.nIndex = Index;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - Calling GetParameter on Port %d for VideoPortFormat", PortIndex));

            Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamVideoPortFormat, &iParamPortFormat);

            if (OMX_ErrorNoMore != Err)
            {
                if (OMX_ErrorNone != Err)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - GetParameter at port %i returned ERROR for  OMX_IndexParamVideoPortFormat", PortIndex));
                    return OMX_FALSE;
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - Trying to Set the same parameters on Port %d for VideoPortFormat", PortIndex));

                /* no failure so test this port also for setting the same parameters */
                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamVideoPortFormat, &iParamPortFormat);
                if (OMX_ErrorNone != Err)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - SetParameter at port %i returned ERROR for OMX_IndexParamVideoPortFormat", PortIndex));
                    return OMX_FALSE;
                }

                //Now set the default preferred values in both the structures
                if (0 == Index)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - Calling GetParameter to find out the default format supported on Port %d for ParamPortDefinition", PortIndex));

                    INIT_GETPARAMETER_STRUCT(OMX_PARAM_PORTDEFINITIONTYPE, iParamPort);
                    iParamPort.nPortIndex = PortIndex;
                    Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamPortDefinition, &iParamPort);
                    if (OMX_ErrorNone != Err)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - GetParameter at port %i returned ERROR for OMX_IndexParamPortDefinition", PortIndex));
                        return OMX_FALSE;
                    }


                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - Verifying the default parameters reported against the preferred ones"));

                    //Check the parameter and match them against the preferred ones
                    if ((OMX_TRUE != iParamPort.bEnabled) ||
                            (OMX_PortDomainVideo != iParamPort.eDomain) ||
                            (iParamPortFormat.eCompressionFormat != iParamPort.format.video.eCompressionFormat) ||
                            (iParamPortFormat.eColorFormat != iParamPort.format.video.eColorFormat))
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - Port %i reported incorrect parameters OMX_IndexParamPortDefinition", PortIndex));
                        return OMX_FALSE;
                    }


                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - Trying to Set the same parameters on Port %d for ParamPortDefinition", PortIndex));

                    /* apply port definition defaults back into the component */
                    Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamPortDefinition, &iParamPort);
                    if (OMX_ErrorNone != Err)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - SetParameter at port %i returned ERROR for OMX_IndexParamPortDefinition", PortIndex));
                        return OMX_FALSE;
                    }
                }

                /* if port format is compresssed data, validate format specific parameter structure */

                if (OMX_VIDEO_CodingUnused != iParamPortFormat.eCompressionFormat)
                {
                    switch (iParamPortFormat.eCompressionFormat)
                    {
                        case OMX_VIDEO_CodingMPEG4:
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                            (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - MPEG4 compression format found on port %d, Verifying the Get/Set Parameter combination", PortIndex));

                            INIT_GETPARAMETER_STRUCT(OMX_VIDEO_PARAM_MPEG4TYPE, FormatMpeg4);
                            FormatMpeg4.nPortIndex = PortIndex;

                            //Check the Get and Set Parameter api's with the compressed format
                            Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamVideoMpeg4, &FormatMpeg4);
                            if (OMX_ErrorNone != Err)
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - Port %i returned error for format OMX_VIDEO_CodingMPEG4", PortIndex));
                                return OMX_FALSE;
                            }

                            Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamVideoMpeg4, &FormatMpeg4);
                            if (OMX_ErrorNone != Err)
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - Port %i returned error while setting format OMX_VIDEO_CodingMPEG4", PortIndex));
                                return OMX_FALSE;
                            }
                        }
                        break;

                        case OMX_VIDEO_CodingH263:
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                            (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - H263 compression format found on port %d, Verifying the Get/Set Parameter combination", PortIndex));

                            INIT_GETPARAMETER_STRUCT(OMX_VIDEO_PARAM_H263TYPE, FormatH263);
                            FormatH263.nPortIndex = PortIndex;

                            //Check the Get and Set Parameter api's with the compressed format
                            Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamVideoH263, &FormatH263);
                            if (OMX_ErrorNone != Err)
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - Port %i returned error for format OMX_VIDEO_CodingH263", PortIndex));
                                return OMX_FALSE;
                            }

                            Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamVideoH263, &FormatH263);
                            if (OMX_ErrorNone != Err)
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - Port %i returned error while setting format OMX_VIDEO_CodingH263", PortIndex));
                                return OMX_FALSE;
                            }
                        }
                        break;

                        case OMX_VIDEO_CodingWMV:
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                            (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - WMV compression format found on port %d, Verifying the Get/Set Parameter combination", PortIndex));

                            INIT_GETPARAMETER_STRUCT(OMX_VIDEO_PARAM_WMVTYPE, FormatWmv);
                            FormatWmv.nPortIndex = PortIndex;

                            //Check the Get and Set Parameter api's with the compressed format
                            Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamVideoWmv, &FormatWmv);
                            if (OMX_ErrorNone != Err)
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - Port %i returned error for format OMX_VIDEO_CodingWMV", PortIndex));
                                return OMX_FALSE;
                            }

                            Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamVideoWmv, &FormatWmv);
                            if (OMX_ErrorNone != Err)
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - Port %i returned error while setting format OMX_VIDEO_CodingWMV", PortIndex));
                                return OMX_FALSE;
                            }
                        }
                        break;

                        case OMX_VIDEO_CodingAVC:
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                            (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - AVC compression format found on port %d, Verifying the Get/Set Parameter combination", PortIndex));

                            INIT_GETPARAMETER_STRUCT(OMX_VIDEO_PARAM_AVCTYPE, FormatAvc);
                            FormatAvc.nPortIndex = PortIndex;

                            //Check the Get and Set Parameter api's with the compressed format
                            Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamVideoAvc, &FormatAvc);
                            if (OMX_ErrorNone != Err)
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - Port %i returned error for format OMX_VIDEO_CodingAVC, OUT", PortIndex));
                                return OMX_FALSE;
                            }

                            Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamVideoAvc, &FormatAvc);
                            if (OMX_ErrorNone != Err)
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - Port %i returned error while setting format OMX_VIDEO_CodingAVC", PortIndex));
                                return OMX_FALSE;
                            }
                        }
                        break;

                        default:
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                            (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - Port %i has invalid eCompressionFormat=0x%X, OUT", PortIndex, iParamPortFormat.eCompressionFormat));
                            return OMX_FALSE;
                        }
                    }
                }

                /* if port has supported color format, verify color format */
                if (OMX_COLOR_FormatUnused != iParamPortFormat.eColorFormat)
                {
                    if ((iParamPortFormat.eColorFormat >= OMX_COLOR_FormatMonochrome) ||
                            (iParamPortFormat.eColorFormat <= OMX_COLOR_Format24BitABGR6666))
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                        (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - Port %i inspecting valid color format", PortIndex));
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - Port %i has invalid eColorFormat=0x%X",
                                         PortIndex, iParamPortFormat.eColorFormat));

                        return OMX_FALSE;
                    }
                }

            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - Port %d reported the formats, no more supported now", PortIndex));

            Index++;

        } //Closing braces for while ((OMX_ErrorNoMore != Err) && (Index < MAX_TEST_LOOP_COUNT))

        if (0 == Index)
        {
            /* the port did not enumerate a single format */
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - Port %i does not support any color or compression formats", PortIndex));

            return OMX_FALSE;
        }

    } //Closing braces for the loop for (ii = 0; ii < iPortInit.nPorts; ii++)

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxEncTestBufferNegotiation::ParamTestVideoPort() - OUT"));

    return OMX_TRUE;
}



OMX_BOOL OmxEncTestBufferNegotiation::ParamTestAudioPort()
{
    OMX_U32 ii = 0;
    OMX_U32 Index  = 0;
    OMX_U32 PortIndex;
    OMX_ERRORTYPE Err = OMX_ErrorNone;

    OMX_AUDIO_PARAM_PORTFORMATTYPE iParamPortFormat;
    OMX_AUDIO_PARAM_PCMMODETYPE FormatPCM;
    OMX_AUDIO_PARAM_AMRTYPE FormatAMR;
    OMX_AUDIO_PARAM_MP3TYPE FormatMP3;
    OMX_AUDIO_PARAM_AACPROFILETYPE FormatAAC;
    OMX_AUDIO_PARAM_WMATYPE FormatWMA;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - IN"));

    /* process port definitions and parameters on all audio ports */
    for (ii = 0; ii < iPortInit.nPorts; ii++)
    {
        PortIndex = iPortInit.nStartPortNumber + ii;
        Index  = 0;
        Err = OMX_ErrorNone;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - Looping through all the formats supported by OMX_IndexParamAudioPortFormat on port %d", PortIndex));

        INIT_GETPARAMETER_STRUCT(OMX_AUDIO_PARAM_PORTFORMATTYPE, iParamPortFormat);
        iParamPortFormat.nPortIndex = PortIndex;

        while ((OMX_ErrorNoMore != Err) && (Index < MAX_TEST_LOOP_COUNT))
        {
            iParamPortFormat.nIndex = Index;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - Calling GetParameter on Port %d for AudioPortFormat", PortIndex));

            Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamAudioPortFormat, &iParamPortFormat);

            if (OMX_ErrorNoMore != Err)
            {
                if (OMX_ErrorNone != Err)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - GetParameter at port %i returned ERROR for OMX_IndexParamAudioPortFormat", PortIndex));
                    return OMX_FALSE;
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - Trying to Set the same parameters on Port %d for AudioPortFormat", PortIndex));

                /* no failure so test this port also for setting the same parameters */
                Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamAudioPortFormat, &iParamPortFormat);
                if (OMX_ErrorNone != Err)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - SetParameter at port %i returned ERROR for OMX_IndexParamAudioPortFormat", PortIndex));
                    return OMX_FALSE;
                }

                //Now set the default preferred values in both the structures
                if (0 == Index)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - Calling GetParameter to find out the default format supported on Port %d for ParamPortDefinition", PortIndex));

                    INIT_GETPARAMETER_STRUCT(OMX_PARAM_PORTDEFINITIONTYPE, iParamPort);
                    iParamPort.nPortIndex = PortIndex;
                    Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamPortDefinition, &iParamPort);
                    if (OMX_ErrorNone != Err)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - GetParameter at port %i returned ERROR for OMX_IndexParamPortDefinition", PortIndex));
                        return OMX_FALSE;
                    }

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - Verifying the default parameters reported against the preferred ones"));

                    //Check the parameter and match them against the preferred ones
                    if ((OMX_TRUE != iParamPort.bEnabled) ||
                            (OMX_PortDomainAudio != iParamPort.eDomain) ||
                            (iParamPortFormat.eEncoding != iParamPort.format.audio.eEncoding))
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - Port %i reported incorrect parameters OMX_IndexParamPortDefinition", PortIndex));
                        return OMX_FALSE;
                    }

                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - Trying to Set the same parameters on Port %d for ParamPortDefinition", PortIndex));

                    /* apply port definition defaults back into the component */
                    Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamPortDefinition, &iParamPort);
                    if (OMX_ErrorNone != Err)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                        (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - SetParameter at port %i returned ERROR for OMX_IndexParamPortDefinition", PortIndex));
                        return OMX_FALSE;
                    }
                }

                /* if port format is compresssed data, validate format specific parameter structure */

                if (OMX_AUDIO_CodingUnused != iParamPortFormat.eEncoding)
                {
                    switch (iParamPortFormat.eEncoding)
                    {
                        case OMX_AUDIO_CodingAutoDetect:
                        {
                            /* auto detection cannot be tested for parameter configuration */
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                            (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - Port %i inspecting format OMX_AUDIO_CodingAutoDetect", PortIndex));
                        }
                        break;

                        case OMX_AUDIO_CodingPCM:
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                            (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - PCM Encoding format found on port %d, Verifying the Get/Set Parameter combination", PortIndex));

                            INIT_GETPARAMETER_STRUCT(OMX_AUDIO_PARAM_PCMMODETYPE, FormatPCM);
                            FormatPCM.nPortIndex = PortIndex;

                            //Check the Get and Set Parameter api's with the compressed format
                            Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamAudioPcm, &FormatPCM);
                            if (OMX_ErrorNone != Err)
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - Port %i returned error for format OMX_AUDIO_CodingPCM", PortIndex));
                                return OMX_FALSE;
                            }

                            Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamAudioPcm, &FormatPCM);
                            if (OMX_ErrorNone != Err)
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - Port %i returned error while setting format OMX_AUDIO_CodingPCM", PortIndex));
                                return OMX_FALSE;
                            }
                        }
                        break;

                        case OMX_AUDIO_CodingAMR:
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                            (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - AMR Encoding format found on port %d, Verifying the Get/Set Parameter combination", PortIndex));

                            INIT_GETPARAMETER_STRUCT(OMX_AUDIO_PARAM_AMRTYPE, FormatAMR);
                            FormatAMR.nPortIndex = PortIndex;

                            //Check the Get and Set Parameter api's with the compressed format
                            Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamAudioAmr, &FormatAMR);
                            if (OMX_ErrorNone != Err)
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - Port %i returned error for format OMX_AUDIO_CodingAMR", PortIndex));
                                return OMX_FALSE;
                            }

                            Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamAudioAmr, &FormatAMR);
                            if (OMX_ErrorNone != Err)
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - Port %i returned error while setting format OMX_AUDIO_CodingAMR", PortIndex));
                                return OMX_FALSE;
                            }
                        }
                        break;

                        case OMX_AUDIO_CodingAAC:
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                            (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - AAC Encoding format found on port %d, Verifying the Get/Set Parameter combination", PortIndex));

                            INIT_GETPARAMETER_STRUCT(OMX_AUDIO_PARAM_AACPROFILETYPE, FormatAAC);
                            FormatAAC.nPortIndex = PortIndex;

                            //Check the Get and Set Parameter api's with the compressed format
                            Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamAudioAac, &FormatAAC);
                            if (OMX_ErrorNone != Err)
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - Port %i returned error for format OMX_AUDIO_CodingAAC", PortIndex));
                                return OMX_FALSE;
                            }

                            Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamAudioAac, &FormatAAC);
                            if (OMX_ErrorNone != Err)
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - Port %i returned error while setting format OMX_AUDIO_CodingAAC", PortIndex));
                                return OMX_FALSE;
                            }
                        }
                        break;

                        case OMX_AUDIO_CodingMP3:
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                            (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - MP3 Encoding format found on port %d, Verifying the Get/Set Parameter combination", PortIndex));

                            INIT_GETPARAMETER_STRUCT(OMX_AUDIO_PARAM_MP3TYPE, FormatMP3);
                            FormatMP3.nPortIndex = PortIndex;

                            //Check the Get and Set Parameter api's with the compressed format
                            Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamAudioMp3, &FormatMP3);
                            if (OMX_ErrorNone != Err)
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - Port %i returned error for format OMX_AUDIO_CodingMP3", PortIndex));
                                return OMX_FALSE;
                            }

                            Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamAudioMp3, &FormatMP3);
                            if (OMX_ErrorNone != Err)
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - Port %i returned error while setting format OMX_AUDIO_CodingMP3", PortIndex));
                                return OMX_FALSE;
                            }
                        }
                        break;

                        case OMX_AUDIO_CodingWMA:
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                            (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - WMA Encoding format found on port %d, Verifying the Get/Set Parameter combination", PortIndex));

                            INIT_GETPARAMETER_STRUCT(OMX_AUDIO_PARAM_WMATYPE, FormatWMA);
                            FormatWMA.nPortIndex = PortIndex;

                            //Check the Get and Set Parameter api's with the compressed format
                            Err = OMX_GetParameter(ipAppPriv->Handle, OMX_IndexParamAudioWma, &FormatWMA);
                            if (OMX_ErrorNone != Err)
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - Port %i returned error for format OMX_AUDIO_CodingWMA", PortIndex));
                                return OMX_FALSE;
                            }

                            Err = OMX_SetParameter(ipAppPriv->Handle, OMX_IndexParamAudioWma, &FormatWMA);
                            if (OMX_ErrorNone != Err)
                            {
                                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                                (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - Port %i returned error while setting format OMX_AUDIO_CodingWMA", PortIndex));
                                return OMX_FALSE;
                            }
                        }
                        break;

                        default:
                        {
                            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                            (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - Port %i has invalid eEncoding=0x%X", PortIndex, iParamPortFormat.eEncoding));
                            return OMX_FALSE;
                        }
                    }
                }
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - Port %d reported the formats, no more supported now", PortIndex));

            Index++;

        } //Closing braces for while ((OMX_ErrorNoMore != Err) && (Index < MAX_TEST_LOOP_COUNT))

        if (0 == Index)
        {
            /* the port did not enumerate a single format */
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - Port %i does not support any compression encoding", PortIndex));

            return OMX_FALSE;
        }

    } //Closing braces for the loop for (ii = 0; ii < iPortInit.nPorts; ii++)

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxEncTestBufferNegotiation::ParamTestAudioPort() - OUT"));

    return OMX_TRUE;
}


/*
 * Active Object class's Run () function
 * Control all the states of AO & sends API's to the component
 */

void OmxEncTestBufferNegotiation::Run()
{
    switch (iState)
    {
        case StateUnLoaded:
        {
            OMX_ERRORTYPE Err;
            OMX_BOOL Status;
            OMX_U32 ii;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxEncTestBufferNegotiation::Run() - StateUnLoaded IN"));

            if (!iCallbacks->initCallbacks())
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxEncTestBufferNegotiation::Run() - ERROR initCallbacks failed, OUT"));
                StopOnError();
                break;
            }

            ipAppPriv = (AppPrivateType*) oscl_malloc(sizeof(AppPrivateType));
            CHECK_MEM(ipAppPriv, "Component_Handle");

            //This should be the first call to the component to load it.
            Err = OMX_Init();
            CHECK_ERROR(Err, "OMX_Init");
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxEncTestBufferNegotiation::Run() - OMX_Init done"));

            if (NULL != iRole)
            {
                OMX_U32 NumComps = 0;
                OMX_STRING* pCompOfRole = NULL;
                OMX_BOOL IsRoleSupported = OMX_FALSE;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxEncTestBufferNegotiation::Run() - Finding out the role for the component %s", iRole));

                //Given the role, determine the component first & then get the handle
                // Call once to find out the number of components that can fit the role
                Err = OMX_GetComponentsOfRole(iRole, &NumComps, NULL);

                if (OMX_ErrorNone != Err || NumComps < 1)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxEncTestBufferNegotiation::Run() - ERROR, No component can handle the specified role %s", iRole));
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
                                    (0, "OmxEncTestBufferNegotiation::Run() - Error occured in this state, StateUnLoaded OUT"));
                    RunIfNotReady();
                    break;
                }

                // call 2nd time to get the component names
                Err = OMX_GetComponentsOfRole(iRole, &NumComps, (OMX_U8**) pCompOfRole);
                CHECK_ERROR(Err, "GetComponentsOfRole");

                for (ii = 0; ii < NumComps; ii++)
                {
                    // try to create component
                    Err = OMX_GetHandle(&ipAppPriv->Handle, (OMX_STRING) pCompOfRole[ii], (OMX_PTR) this , iCallbacks->getCallbackStruct());
                    // if successful, no need to continue
                    if ((OMX_ErrorNone == Err) && (NULL != ipAppPriv->Handle))
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxEncTestBufferNegotiation::Run() - Got Handle for the component %s", pCompOfRole[ii]));
                        break;
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxEncTestBufferNegotiation::Run() - ERROR, Cannot get component %s handle, try another if possible", pCompOfRole[ii]));
                    }

                }

                //Free the role of component arrays
                if (pCompOfRole)
                {
                    for (ii = 0; ii < NumComps; ii++)
                    {
                        oscl_free(pCompOfRole[ii]);
                        pCompOfRole[ii] = NULL;
                    }
                    oscl_free(pCompOfRole);
                    pCompOfRole = NULL;
                }

                // check if there was a problem
                CHECK_ERROR(Err, "GetHandle");
                CHECK_MEM(ipAppPriv->Handle, "ComponentHandle");
            }

            if (0 == oscl_strcmp(iFormat, "AMRNB") || 0 == oscl_strcmp(iFormat, "AAC"))
            {
                iIsAudioFormat = OMX_TRUE;
            }

            Status = NegotiateParameters();

            if (StateError == iState || OMX_FALSE == Status)
            {
                iState = StateError;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "OmxEncTestBufferNegotiation::Run() - Error occured in this state, Exiting the test case, OUT"));
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
                                (0, "OmxEncTestBufferNegotiation::Run() - Error, ThreadSafe Callback Handler initialization failed, OUT"));

                iState = StateUnLoaded;
                OsclExecScheduler* sched = OsclExecScheduler::Current();
                sched->StopScheduler();
            }
#endif

            if (OMX_TRUE == iIsAudioFormat)
            {
                Status = ParamTestAudioPort();
            }
            else
            {
                Status = ParamTestVideoPort();
            }

            if (StateError == iState || OMX_FALSE == Status)
            {
                iState = StateError;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "OmxEncTestBufferNegotiation::Run() - Error occured in this state, Exiting the test case, OUT"));
            }
            else
            {
                iState = StateLoaded;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxEncTestBufferNegotiation::Run() - StateUnLoaded OUT, moving to next state"));
            }

            RunIfNotReady();
        }
        break;

        case StateLoaded:
        {
            OMX_ERRORTYPE Err;
            OMX_S32 ii;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxEncTestBufferNegotiation::Run() - StateLoaded IN"));

            // allocate memory for ipInBuffer
            ipInBuffer = (OMX_BUFFERHEADERTYPE**) oscl_malloc(sizeof(OMX_BUFFERHEADERTYPE*) * iInBufferCount);
            CHECK_MEM(ipInBuffer, "InputBufferHeader");

            /* Initialize all the buffers to NULL */
            for (ii = 0; ii < iInBufferCount; ii++)
            {
                ipInBuffer[ii] = NULL;
            }

            //allocate memory for ipOutBuffer
            ipOutBuffer = (OMX_BUFFERHEADERTYPE**) oscl_malloc(sizeof(OMX_BUFFERHEADERTYPE*) * iOutBufferCount);
            CHECK_MEM(ipOutBuffer, "OutputBuffer");

            /* Initialize all the buffers to NULL */
            for (ii = 0; ii < iOutBufferCount; ii++)
            {
                ipOutBuffer[ii] = NULL;
            }

            Err = OMX_SendCommand(ipAppPriv->Handle, OMX_CommandStateSet, OMX_StateIdle, NULL);
            CHECK_ERROR(Err, "SendCommand Loaded->Idle");
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxEncTestBufferNegotiation::Run() - Sent State Transition Command from Loaded->Idle"));
            iPendingCommands = 1;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxEncTestBufferNegotiation::Run() - Allocating %d input and %d output buffers", iInBufferCount, iOutBufferCount));

            //Allocate the buffers
            for (ii = 0; ii < iInBufferCount; ii++)
            {
                Err = OMX_AllocateBuffer(ipAppPriv->Handle, &ipInBuffer[ii], iInputPortIndex, NULL, iInBufferSize);
                CHECK_ERROR(Err, "AllocateBuffer_Input");
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxEncTestBufferNegotiation::Run() - Called AllocateBuffer for buffer index %d on port %d", ii, iInputPortIndex));
            }
            if (StateError == iState)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "OmxEncTestBufferNegotiation::Run() - AllocateBuffer Error, StateLoaded OUT"));
                RunIfNotReady();
                break;
            }

            for (ii = 0; ii < iOutBufferCount; ii++)
            {
                Err = OMX_AllocateBuffer(ipAppPriv->Handle, &ipOutBuffer[ii], iOutputPortIndex, NULL, iOutBufferSize);
                CHECK_ERROR(Err, "AllocateBuffer_Output");
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxEncTestBufferNegotiation::Run() - Called AllocateBuffer for buffer index %d on port %d", ii, iOutputPortIndex));
            }

            if (StateError == iState)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "OmxEncTestBufferNegotiation::Run() - AllocateBuffer Error, StateLoaded OUT"));
                RunIfNotReady();
                break;
            }
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxEncTestBufferNegotiation::Run() - StateLoaded OUT, Moving to next state"));
        }
        break;

        case StateIdle:
        {
            iState = StateCleanUp;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "OmxEncTestBufferNegotiation::Run() - StateIdle IN, Nothing to be done here, move to StateCleanUp"));
            RunIfNotReady();
        }
        break;

        case StateCleanUp:
        {
            OMX_S32 ii;
            OMX_ERRORTYPE Err = OMX_ErrorNone;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxEncTestBufferNegotiation::Run() - StateCleanUp IN"));
            //Destroy the component by state transition to Loaded state
            Err = OMX_SendCommand(ipAppPriv->Handle, OMX_CommandStateSet, OMX_StateLoaded, NULL);
            CHECK_ERROR(Err, "SendCommand Idle->Loaded");


            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxEncTestBufferNegotiation::Run() - Sent State Transition Command from Idle->Loaded"));

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
                                        (0, "OmxEncTestBufferNegotiation::Run() - Called FreeBuffer for buffer index %d on port %d", ii, iInputPortIndex));
                    }
                }

                oscl_free(ipInBuffer);
                ipInBuffer = NULL;
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
                                        (0, "OmxEncTestBufferNegotiation::Run() - Called FreeBuffer for buffer index %d on port %d", ii, iOutputPortIndex));
                    }
                }
                oscl_free(ipOutBuffer);
                ipOutBuffer = NULL;
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxEncTestBufferNegotiation::Run() - StateCleanUp OUT"));

        }
        break;

        /********* FREE THE HANDLE & CLOSE FILES FOR THE COMPONENT ********/
        case StateStop:
        {
            OMX_U8 TestName[] = "PARAM_NEGOTIATION_TEST";
            OMX_ERRORTYPE Err = OMX_ErrorNone;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxEncTestBufferNegotiation::Run() - StateStop IN"));
            if (ipAppPriv)
            {
                if (ipAppPriv->Handle)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxEncTestBufferNegotiation::Run() - Free the Component Handle"));

                    Err = OMX_FreeHandle(ipAppPriv->Handle);

                    if (OMX_ErrorNone != Err)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxEncTestBufferNegotiation::Run() - FreeHandle Error"));
                        iTestStatus = OMX_FALSE;
                    }
                }
            }
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxEncTestBufferNegotiation::Run() - De-initialize the omx component"));

            Err = OMX_Deinit();
            if (OMX_ErrorNone != Err)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxEncTestBufferNegotiation::Run() - OMX_Deinit Error"));
                iTestStatus = OMX_FALSE;
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
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                                (0, "OmxEncTestBufferNegotiation::Run() - %s : Fail", TestName));

#ifdef PRINT_RESULT
                printf("%s: Fail \n", TestName);
#endif

            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                                (0, "OmxEncTestBufferNegotiation::Run() - %s : Success", TestName));

#ifdef PRINT_RESULT
                printf("%s: Success \n", TestName);
#endif
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxEncTestBufferNegotiation::Run() - StateStop OUT"));
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
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxEncTestBufferNegotiation::Run() - StateError IN"));

            if (ipInBuffer)
            {
                for (ii = 0; ii < iInBufferCount; ii++)
                {
                    if (ipInBuffer[ii])
                    {
                        OMX_FreeBuffer(ipAppPriv->Handle, iInputPortIndex, ipInBuffer[ii]);
                        ipInBuffer[ii] = NULL;
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                        (0, "OmxEncTestBufferNegotiation::Run() - Called FreeBuffer for buffer index %d on port %d", ii, iInputPortIndex));
                    }
                }
                oscl_free(ipInBuffer);
                ipInBuffer =  NULL;
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
                                        (0, "OmxEncTestBufferNegotiation::Run() - Called FreeBuffer for buffer index %d on port %d", ii, iOutputPortIndex));
                    }
                }
                oscl_free(ipOutBuffer);
                ipOutBuffer = NULL;
            }

            iState = StateStop;
            RunIfNotReady();
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxEncTestBufferNegotiation::Run() - StateError OUT"));

        }
        break;

        default:
        {
            break;
        }
    }
    return ;
}
