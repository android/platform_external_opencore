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

/** OMX_CONF_ParameterTest.c
 *  OpenMax IL conformance test - Parameter Port Test
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "OMX_OSAL_Interfaces.h"
#include "OMX_CONF_TestHarness.h"
#include "OMX_CONF_StubbedCallbacks.h"

#include <string.h>

#define TEST_NAME_STRING "ParameterTest"
#define NUM_DOMAINS 0x4
#define TEST_LOOP_BOUND 0x1000
#define TEST_BAD_VERSION 0x02010301

#define TEST_GROUPID 0xF00DBEEF
#define TEST_GROUPPRIORITY 0xDEADC0DE


#define OMX_CONF_INIT_STRUCT(_s_, _name_)\
    memset(&(_s_), 0x0, sizeof(_name_));\
    (_s_).nSize = sizeof(_name_);\
    (_s_).nVersion.s.nVersionMajor = 0x1;\
    (_s_).nVersion.s.nVersionMinor = 0x0;\
    (_s_).nVersion.s.nRevision = 0x0;\
    (_s_).nVersion.s.nStep = 0x0

#define OMX_CONF_BAIL_ON_ERROR(_e_) \
    if (OMX_ErrorNone != (_e_))\
    {\
        goto OMX_CONF_TEST_BAIL;\
    }

#define OMX_CONF_SET_ERROR_BAIL(_e_, _c_, _s_)\
    _e_ = _c_;\
    OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, _s_);\
    goto OMX_CONF_TEST_BAIL

#define OMX_CONF_ASSERT(_e_, _a_, _r_) \
    if (!(_a_))\
    {\
        OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "%s\n", (_r_));\
        (_e_) = OMX_ErrorUndefined;\
        goto OMX_CONF_TEST_BAIL;\
    }

#define OMX_CONF_PARAM_REQUIREDGETSET(_p_, _i_, _s_, _e_)\
    _e_ = OMX_GetParameter((_p_)->hWrappedComp, _i_, _s_);\
    OMX_CONF_BAIL_ON_ERROR(_e_);\
    _e_ = OMX_SetParameter((_p_)->hWrappedComp, _i_, _s_);\
    OMX_CONF_BAIL_ON_ERROR(_e_)

#define OMX_CONF_PARAM_OPTIONALGETSET(_p_, _i_, _s_, _e_)\
    _e_ = OMX_GetParameter((_p_)->hWrappedComp, _i_, _s_);\
    if (OMX_ErrorUnsupportedIndex == _e_) _e_ = OMX_ErrorNone;\
    OMX_CONF_BAIL_ON_ERROR(_e_);\
    _e_ = OMX_SetParameter((_p_)->hWrappedComp, _i_, _s_);\
    if (OMX_ErrorUnsupportedIndex == _e_) _e_ = OMX_ErrorNone;\
    OMX_CONF_BAIL_ON_ERROR(_e_)

#define OMX_CONF_CONFIG_REQUIREDGETSET(_p_, _i_, _s_, _e_)\
    _e_ = OMX_GetConfig((_p_)->hWrappedComp, _i_, _s_);\
    OMX_CONF_BAIL_ON_ERROR(_e_);\
    _e_ = OMX_SetConfig((_p_)->hWrappedComp, _i_, _s_);\
    OMX_CONF_BAIL_ON_ERROR(_e_)

#define OMX_CONF_CONFIG_OPTIONALGETSET(_p_, _i_, _s_, _e_)\
    _e_ = OMX_GetConfig((_p_)->hWrappedComp, _i_, _s_);\
    if (OMX_ErrorUnsupportedIndex == _e_) _e_ = OMX_ErrorNone;\
    OMX_CONF_BAIL_ON_ERROR(_e_);\
    _e_ = OMX_SetConfig((_p_)->hWrappedComp, _i_, _s_);\
    if (OMX_ErrorUnsupportedIndex == _e_) _e_ = OMX_ErrorNone;\
    OMX_CONF_BAIL_ON_ERROR(_e_)



    /************************* T Y P E   D E F I N I T I O N S *******************/

    typedef struct _TEST_CTXTYPE
    {
        OMX_HANDLETYPE hComp;
        OMX_HANDLETYPE hWrappedComp;
        OMX_STATETYPE eState;
        OMX_U32 nNumPorts;
        OMX_PORT_PARAM_TYPE sPortParam[NUM_DOMAINS];
        OMX_HANDLETYPE hStateChangeEvent;
        OMX_HANDLETYPE hPortStopEvent;
        OMX_HANDLETYPE hPortReStartEvent;
        OMX_U32 nReStartPort;
        OMX_U32 nStopPort;

    } TEST_CTXTYPE;


    /**************************** G L O B A L S **********************************/


    /*****************************************************************************/
    OMX_ERRORTYPE paramtest_eventhandler(
        OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_EVENTTYPE eEvent,
        OMX_IN OMX_U32 nData1,
        OMX_IN OMX_U32 nData2,
        OMX_IN OMX_PTR pEventData)
    {
        TEST_CTXTYPE *pCtx;

        UNUSED_PARAMETER(hComponent);
        UNUSED_PARAMETER(pEventData);

        if (NULL == pAppData) return OMX_ErrorNone;

        pCtx = (TEST_CTXTYPE*)pAppData;

        if (eEvent == OMX_EventCmdComplete)
        {
            switch ((OMX_COMMANDTYPE)(nData1))
            {
                case OMX_CommandStateSet:
                    pCtx->eState = (OMX_STATETYPE)(nData2);
                    OMX_OSAL_EventSet(pCtx->hStateChangeEvent);
                    break;
                case OMX_CommandPortDisable:
                    if (pCtx->nStopPort == nData2) OMX_OSAL_EventSet(pCtx->hPortStopEvent);
                    break;
                case OMX_CommandPortEnable:
                    if (pCtx->nReStartPort == nData2) OMX_OSAL_EventSet(pCtx->hPortReStartEvent);
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
    OMX_ERRORTYPE paramtest_FindBogusPortIndex(
        TEST_CTXTYPE *pCtx,
        OMX_U32 *pPortIndex)
    {
        // todo - implement this in a better way
        UNUSED_PARAMETER(pCtx);
        *pPortIndex = 0xDEADBEEF;

        return(OMX_ErrorNone);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE paramtest_bogusparameter(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nIndex,
        OMX_PTR pData)
    {
        OMX_ERRORTYPE eError;

        /* test valid get for this index */
        eError = OMX_GetParameter(pCtx->hWrappedComp, nIndex, pData);
        if (eError != OMX_ErrorUnsupportedIndex)
        {
            OMX_CONF_ASSERT(eError, (OMX_ErrorNone == eError),
                            "Expecting index to be supported by OMX_GetParameter\n");

            /* test bogus NULL parameter for this index */
            eError = OMX_GetParameter(pCtx->hWrappedComp, nIndex, 0x0);
            OMX_CONF_ASSERT(eError, (OMX_ErrorBadParameter == eError),
                            "Expecting return value OMX_ErrorBadParameter\n");
            eError = OMX_SetParameter(pCtx->hWrappedComp, nIndex, 0x0);
            OMX_CONF_ASSERT(eError, (OMX_ErrorBadParameter == eError),
                            "Expecting return value OMX_ErrorBadParameter\n");

            /* restore error no none */
            eError = OMX_ErrorNone;
        }


OMX_CONF_TEST_BAIL:

        return(eError);

    }


    /*****************************************************************************/
    OMX_ERRORTYPE paramtest_bogusportparameter(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nIndex,
        OMX_PTR pData)
    {
        OMX_ERRORTYPE eError;
        OMX_PARAM_PORTDEFINITIONTYPE *pPortDef;
        OMX_U32 nPortIndex;

        /* test valid get for this index */
        eError = OMX_GetParameter(pCtx->hWrappedComp, nIndex, pData);
        if (eError != OMX_ErrorUnsupportedIndex)
        {
            OMX_CONF_ASSERT(eError, (OMX_ErrorNone == eError),
                            "Expecting index to be supported by OMX_GetParameter\n");

            /* test bogus common parameter fields */
            eError = paramtest_bogusparameter(pCtx, nIndex, pData);
            OMX_CONF_BAIL_ON_ERROR(eError);

            /* test bogus port index */
            pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE*)pData;
            nPortIndex = pPortDef->nPortIndex;
            eError = paramtest_FindBogusPortIndex(pCtx, &pPortDef->nPortIndex);
            OMX_CONF_BAIL_ON_ERROR(eError);

            eError = OMX_GetParameter(pCtx->hWrappedComp, nIndex, pData);
            OMX_CONF_ASSERT(eError, (OMX_ErrorBadPortIndex == eError),
                            "Expecting return value OMX_ErrorBadPortIndex\n");

            eError = OMX_SetParameter(pCtx->hWrappedComp, nIndex, pData);
            OMX_CONF_ASSERT(eError, (OMX_ErrorBadPortIndex == eError),
                            "Expecting return value OMX_ErrorBadPortIndex\n");

            /* restore correct value */
            pPortDef->nPortIndex = nPortIndex;

            /* restore error no none */
            eError = OMX_ErrorNone;
        }


OMX_CONF_TEST_BAIL:

        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE paramtest_bogusportconfig(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nIndex,
        OMX_PTR pData)
    {
        OMX_ERRORTYPE eError;
        OMX_PARAM_PORTDEFINITIONTYPE *pPortDef;
        OMX_U32 nVersion;
        OMX_U32 nPortIndex;

        /* test valid get for this index */
        eError = OMX_GetConfig(pCtx->hWrappedComp, nIndex, pData);
        if (eError != OMX_ErrorUnsupportedIndex)
        {
            OMX_CONF_ASSERT(eError, (OMX_ErrorNone == eError),
                            "Expecting index to be supported by OMX_GetParameter\n");

            /* test bogus NULL parameter for this index */
            eError = OMX_GetConfig(pCtx->hWrappedComp, nIndex, 0x0);
            OMX_CONF_ASSERT(eError, (OMX_ErrorBadParameter == eError),
                            "Expecting return value OMX_ErrorBadParameter\n");
            eError = OMX_SetConfig(pCtx->hWrappedComp, nIndex, 0x0);
            OMX_CONF_ASSERT(eError, (OMX_ErrorBadParameter == eError),
                            "Expecting return value OMX_ErrorBadParameter\n");

            /* test bad structure size for this index */
            pPortDef = (OMX_PARAM_PORTDEFINITIONTYPE*)pData;
            pPortDef->nSize -= 4;

            eError = OMX_GetConfig(pCtx->hWrappedComp, nIndex, pData);
            OMX_CONF_ASSERT(eError, (OMX_ErrorBadParameter == eError),
                            "Expecting return value OMX_ErrorBadParameter\n");

            eError = OMX_SetConfig(pCtx->hWrappedComp, nIndex, pData);
            OMX_CONF_ASSERT(eError, (OMX_ErrorBadParameter == eError),
                            "Expecting return value OMX_ErrorBadParameter\n");

            pPortDef->nSize += 4;

            /* test bad structure version for this index */
            nVersion = pPortDef->nVersion.nVersion;

            eError = OMX_GetConfig(pCtx->hWrappedComp, nIndex, pData);
            OMX_CONF_ASSERT(eError, (OMX_ErrorVersionMismatch == eError),
                            "Expecting return value OMX_ErrorVersionMismatch\n");

            eError = OMX_SetConfig(pCtx->hWrappedComp, nIndex, pData);
            OMX_CONF_ASSERT(eError, (OMX_ErrorVersionMismatch == eError),
                            "Expecting return value OMX_ErrorVersionMismatch\n");

            pPortDef->nVersion.nVersion = nVersion;

            /* test bogus port index */
            nPortIndex = pPortDef->nPortIndex;
            eError = paramtest_FindBogusPortIndex(pCtx, &pPortDef->nPortIndex);
            OMX_CONF_BAIL_ON_ERROR(eError);

            eError = OMX_GetConfig(pCtx->hWrappedComp, nIndex, pData);
            OMX_CONF_ASSERT(eError, (OMX_ErrorBadPortIndex == eError),
                            "Expecting return value OMX_ErrorBadPortIndex\n");

            eError = OMX_SetConfig(pCtx->hWrappedComp, nIndex, pData);
            OMX_CONF_ASSERT(eError, (OMX_ErrorBadPortIndex == eError),
                            "Expecting return value OMX_ErrorBadPortIndex\n");

            /* restore correct value */
            pPortDef->nPortIndex = nPortIndex;

        }


OMX_CONF_TEST_BAIL:

        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE paramtest_buffersupplier(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {
        OMX_ERRORTYPE eError;
        OMX_PARAM_BUFFERSUPPLIERTYPE sBufferSupplier;
        OMX_PARAM_PORTDEFINITIONTYPE sPortDef;

        OMX_CONF_INIT_STRUCT(sBufferSupplier, OMX_PARAM_BUFFERSUPPLIERTYPE);
        sBufferSupplier.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamCompBufferSupplier,
                                  (OMX_PTR) & sBufferSupplier);

        if (OMX_ErrorUnsupportedIndex == eError)
        {
            /* component does not support the buffer supplier index,
               which implies that it is a non-interop component ... as
               such, validate it fails OMX_SetupTunnel, otherwise fail */
            OMX_CONF_INIT_STRUCT(sPortDef, OMX_PARAM_PORTDEFINITIONTYPE);
            eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition,
                                      (OMX_PTR) & sPortDef);
            OMX_CONF_BAIL_ON_ERROR(eError);

            if (OMX_DirInput == sPortDef.eDir)
            {
                eError = OMX_SetupTunnel(0x0, 0x0, pCtx->hWrappedComp, nPortIndex);

            }
            else
            {
                eError = OMX_SetupTunnel(pCtx->hWrappedComp, nPortIndex, 0x0, 0x0);
            }

            OMX_CONF_ASSERT(eError, (OMX_ErrorNotImplemented == eError),
                            "Interop component does not support OMX_IndexParamCompBufferSupplier\n");

        }
        else
        {
            OMX_CONF_BAIL_ON_ERROR(eError);


            sBufferSupplier.eBufferSupplier = OMX_BufferSupplyInput;
            eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamCompBufferSupplier,
                                      (OMX_PTR) & sBufferSupplier);
            OMX_CONF_BAIL_ON_ERROR(eError);

            sBufferSupplier.eBufferSupplier = OMX_BufferSupplyOutput;
            eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamCompBufferSupplier,
                                      (OMX_PTR) & sBufferSupplier);
            OMX_CONF_BAIL_ON_ERROR(eError);

            /* bogus parameter test */
            eError = paramtest_bogusportparameter(pCtx, OMX_IndexParamCompBufferSupplier,
                                                  (OMX_PTR) & sBufferSupplier);
            OMX_CONF_BAIL_ON_ERROR(eError);
        }


OMX_CONF_TEST_BAIL:

        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE paramtest_commandport(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex,
        OMX_BOOL bReStart)
    {
        /* test function is used to command a port to stop, or restart */
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_BOOL bTimeout = OMX_FALSE;

        if (OMX_TRUE == bReStart)
        {
            /* reset markers to track if the event was completed */
            OMX_OSAL_EventReset(pCtx->hPortReStartEvent);
            pCtx->nReStartPort = nPortIndex;
            eError = OMX_SendCommand(pCtx->hWrappedComp, OMX_CommandPortEnable, nPortIndex, 0x0);
            OMX_CONF_BAIL_ON_ERROR(eError);

            /* wait until command is complete, timeout if it takes too long */
            OMX_OSAL_EventWait(pCtx->hPortReStartEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
            eError = ((OMX_TRUE == bTimeout) ? OMX_ErrorUndefined : OMX_ErrorNone);
            OMX_CONF_BAIL_ON_ERROR(eError);

        }
        else
        {
            /* reset markers to track if the event was completed */
            OMX_OSAL_EventReset(pCtx->hPortStopEvent);
            pCtx->nStopPort = nPortIndex;
            eError = OMX_SendCommand(pCtx->hWrappedComp, OMX_CommandPortDisable, nPortIndex, 0x0);
            OMX_CONF_BAIL_ON_ERROR(eError);

            /* wait until command is complete, timeout if it takes too long */
            OMX_OSAL_EventWait(pCtx->hPortStopEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
            eError = ((OMX_TRUE == bTimeout) ? OMX_ErrorUndefined : OMX_ErrorNone);
            OMX_CONF_BAIL_ON_ERROR(eError);
        }


OMX_CONF_TEST_BAIL:
        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE paramtest_portenable(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {
        /* test function is used to command a port to stop, and validate if the
           port definition correctly reports the port as not enabled */
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;

        eError = paramtest_commandport(pCtx, nPortIndex, OMX_FALSE);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* query port definition */
        OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
        sPortDefinition.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition,
                                  (OMX_PTR) & sPortDefinition);
        OMX_CONF_BAIL_ON_ERROR(eError);

        OMX_CONF_ASSERT(eError, (OMX_FALSE == sPortDefinition.bEnabled),
                        "Incorrect value in PortDefinition bEnabled field\n");
        OMX_CONF_ASSERT(eError, (OMX_FALSE == sPortDefinition.bPopulated),
                        "Incorrect value in PortDefinition bPopulated field\n");

        eError = paramtest_commandport(pCtx, nPortIndex, OMX_TRUE);
        OMX_CONF_BAIL_ON_ERROR(eError);

        OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
        sPortDefinition.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition,
                                  (OMX_PTR) & sPortDefinition);
        OMX_CONF_BAIL_ON_ERROR(eError);

        OMX_CONF_ASSERT(eError, (OMX_TRUE == sPortDefinition.bEnabled),
                        "Incorrect value in PortDefinition bEnabled field\n");
        OMX_CONF_ASSERT(eError, (OMX_FALSE == sPortDefinition.bPopulated),
                        "Incorrect value in PortDefinition bPopulated field\n");

        /* test bogus port definition structure as part of port enable test,
           to single source checking of a port related structure */
        eError = paramtest_bogusportparameter(pCtx, OMX_IndexParamPortDefinition,
                                              (OMX_PTR) & sPortDefinition);
        OMX_CONF_BAIL_ON_ERROR(eError);


OMX_CONF_TEST_BAIL:
        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE paramtest_audioport(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_AUDIO_PARAM_PORTFORMATTYPE sPortFormat;
        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;
        OMX_AUDIO_PARAM_PCMMODETYPE sFormatPCM;
        OMX_AUDIO_PARAM_ADPCMTYPE sFormatADPCM;
        OMX_AUDIO_PARAM_AMRTYPE sFormatAMR;
        OMX_AUDIO_PARAM_GSMFRTYPE sFormatGSMFR;
        OMX_AUDIO_PARAM_GSMEFRTYPE sFormatGSMEFR;
        OMX_AUDIO_PARAM_GSMHRTYPE sFormatGSMHR;
        OMX_AUDIO_PARAM_MP3TYPE sFormatMP3;
        OMX_AUDIO_PARAM_AACPROFILETYPE sFormatAAC;
        OMX_AUDIO_PARAM_WMATYPE sFormatWMA;
        OMX_AUDIO_PARAM_MIDITYPE sFormatMIDI;
        OMX_AUDIO_PARAM_SBCTYPE sFormatSBC;
        OMX_AUDIO_PARAM_G729TYPE sFormatG729;
        OMX_AUDIO_PARAM_G726TYPE sFormatG726;
        OMX_AUDIO_PARAM_G723TYPE sFormatG723;
        OMX_AUDIO_PARAM_RATYPE sFormatRA;
        OMX_AUDIO_PARAM_PDCFRTYPE sFormatPDCFR;
        OMX_AUDIO_PARAM_PDCEFRTYPE sFormatPDCEFR;
        OMX_AUDIO_PARAM_PDCHRTYPE sFormatPDCHR;
        OMX_AUDIO_PARAM_SMVTYPE sFormatSMV;
        OMX_AUDIO_PARAM_EVRCTYPE sFormatEVRC;
        OMX_AUDIO_PARAM_QCELP13TYPE sFormatQCELP13;
        OMX_AUDIO_PARAM_QCELP8TYPE sFormatQCELP8;
        OMX_AUDIO_PARAM_TDMAFRTYPE sFormatTDMAFR;
        OMX_AUDIO_PARAM_TDMAEFRTYPE sFormatTDMAEFR;
        OMX_AUDIO_PARAM_VORBISTYPE sFormatVORBIS;

        OMX_U32 i = 0x0;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Inspecting audio port %i\n",
                       nPortIndex);

        OMX_CONF_INIT_STRUCT(sPortFormat, OMX_AUDIO_PARAM_PORTFORMATTYPE);
        sPortFormat.nPortIndex = nPortIndex;
        while ((OMX_ErrorNoMore != eError) && (i < TEST_LOOP_BOUND))
        {
            sPortFormat.nIndex = i;
            eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamAudioPortFormat,
                                      (OMX_PTR) & sPortFormat);
            if (OMX_ErrorNoMore != eError)
            {
                /* don't count the error indicating no more data to enumerate
                   as an actual failure */
                OMX_CONF_BAIL_ON_ERROR(eError);

                /* no failure so test this port for it's default parameters */
                eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamAudioPortFormat, (OMX_PTR) & sPortFormat);
                OMX_CONF_BAIL_ON_ERROR(eError);

                /* query port definition type for this format */
                OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
                sPortDefinition.nPortIndex = nPortIndex;
                eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
                OMX_CONF_BAIL_ON_ERROR(eError);

                /* port must default to enabled */
                OMX_CONF_ASSERT(eError, (OMX_TRUE == sPortDefinition.bEnabled),
                                "Port must default to enabled\n");

                /* port must report the domain consistently domain */
                OMX_CONF_ASSERT(eError, (OMX_PortDomainAudio == sPortDefinition.eDomain),
                                "Port must report format consistently as Audio\n");

                /* validate eFormat matchs the format selected when setting the port format */
                OMX_CONF_ASSERT(eError, (sPortFormat.eEncoding == sPortDefinition.format.audio.eEncoding),
                                "Port format mismatch\n");

                /* apply port definition defaults back into the component */
                eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition,
                                          (OMX_PTR) & sPortDefinition);
                OMX_CONF_BAIL_ON_ERROR(eError);

                /* if port format is compresssed data, validate format specific
                   parameter structure */
                if (OMX_AUDIO_CodingUnused != sPortFormat.eEncoding)
                {
                    switch (sPortFormat.eEncoding)
                    {
                        case OMX_AUDIO_CodingAutoDetect:
                            /* auto detection cannot be tested for parameter
                               configuration */
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingAutoDetect\n",
                                           nPortIndex);
                            break;
                        case OMX_AUDIO_CodingPCM:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingPCM\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatPCM, OMX_AUDIO_PARAM_PCMMODETYPE);
                            sFormatPCM.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioPcm, (OMX_PTR)&sFormatPCM, eError);
                            break;
                        case OMX_AUDIO_CodingADPCM:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingADPCM\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatADPCM, OMX_AUDIO_PARAM_ADPCMTYPE);
                            sFormatADPCM.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioAdpcm, (OMX_PTR)&sFormatADPCM, eError);
                            break;
                        case OMX_AUDIO_CodingAMR:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingAMR\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatAMR, OMX_AUDIO_PARAM_AMRTYPE);
                            sFormatAMR.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioAmr, (OMX_PTR)&sFormatAMR, eError);
                            break;
                        case OMX_AUDIO_CodingGSMFR:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingGSMFR\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatGSMFR, OMX_AUDIO_PARAM_GSMFRTYPE);
                            sFormatGSMFR.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioGsm_FR, (OMX_PTR)&sFormatGSMFR, eError);
                            break;
                        case OMX_AUDIO_CodingGSMEFR:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingGSMEFR\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatGSMEFR, OMX_AUDIO_PARAM_GSMEFRTYPE);
                            sFormatGSMEFR.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioGsm_EFR, (OMX_PTR)&sFormatGSMEFR, eError);
                            break;
                        case OMX_AUDIO_CodingGSMHR:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingGSHR\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatGSMFR, OMX_AUDIO_PARAM_GSMHRTYPE);
                            sFormatGSMHR.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioGsm_HR, (OMX_PTR)&sFormatGSMHR, eError);
                            break;
                        case OMX_AUDIO_CodingPDCFR:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingPDCFR\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatPDCFR, OMX_AUDIO_PARAM_PDCFRTYPE);
                            sFormatPDCFR.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioPdc_FR, (OMX_PTR)&sFormatPDCFR, eError);
                            break;
                        case OMX_AUDIO_CodingPDCEFR:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingPDCEFR\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatPDCEFR, OMX_AUDIO_PARAM_PDCEFRTYPE);
                            sFormatPDCEFR.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioPdc_EFR, (OMX_PTR)&sFormatPDCEFR, eError);
                            break;
                        case OMX_AUDIO_CodingPDCHR:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingPDCHR\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatPDCHR, OMX_AUDIO_PARAM_PDCHRTYPE);
                            sFormatPDCHR.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioPdc_HR, (OMX_PTR)&sFormatPDCHR, eError);
                            break;
                        case OMX_AUDIO_CodingTDMAFR:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingTDMAFR\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatTDMAFR, OMX_AUDIO_PARAM_TDMAFRTYPE);
                            sFormatTDMAFR.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioTdma_FR, (OMX_PTR)&sFormatTDMAFR, eError);
                            break;
                        case OMX_AUDIO_CodingTDMAEFR:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingTDMAEFR\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatTDMAEFR, OMX_AUDIO_PARAM_TDMAEFRTYPE);
                            sFormatTDMAEFR.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioTdma_EFR, (OMX_PTR)&sFormatTDMAEFR, eError);
                            break;
                        case OMX_AUDIO_CodingQCELP8:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingQCELP8\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatQCELP8, OMX_AUDIO_PARAM_QCELP8TYPE);
                            sFormatQCELP8.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioQcelp8, (OMX_PTR)&sFormatQCELP8, eError);
                            break;
                        case OMX_AUDIO_CodingQCELP13:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingQCELP13\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatQCELP13, OMX_AUDIO_PARAM_QCELP13TYPE);
                            sFormatQCELP13.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioQcelp13, (OMX_PTR)&sFormatQCELP13, eError);
                            break;
                        case OMX_AUDIO_CodingEVRC:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingEVRC\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatEVRC, OMX_AUDIO_PARAM_EVRCTYPE);
                            sFormatEVRC.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioEvrc, (OMX_PTR)&sFormatEVRC, eError);
                            break;
                        case OMX_AUDIO_CodingSMV:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingSMV\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatSMV, OMX_AUDIO_PARAM_SMVTYPE);
                            sFormatSMV.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioSmv, (OMX_PTR)&sFormatSMV, eError);
                            break;
                        case OMX_AUDIO_CodingG711:
                            // todo - how does this correlate to PCM?
                            break;
                        case OMX_AUDIO_CodingG723:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingG723\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatG723, OMX_AUDIO_PARAM_G723TYPE);
                            sFormatG723.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioG723, (OMX_PTR)&sFormatG723, eError);
                            break;
                        case OMX_AUDIO_CodingG726:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingG726\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatG726, OMX_AUDIO_PARAM_G726TYPE);
                            sFormatG726.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioG726, (OMX_PTR)&sFormatG726, eError);
                            break;
                        case OMX_AUDIO_CodingG729:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingG729\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatG729, OMX_AUDIO_PARAM_G729TYPE);
                            sFormatG729.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioG729, (OMX_PTR)&sFormatG729, eError);
                            break;
                        case OMX_AUDIO_CodingAAC:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingAAC\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatAAC, OMX_AUDIO_PARAM_AACPROFILETYPE);
                            sFormatAAC.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioAac, (OMX_PTR)&sFormatAAC, eError);
                            break;
                        case OMX_AUDIO_CodingMP3:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingMP3\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatMP3, OMX_AUDIO_PARAM_MP3TYPE);
                            sFormatMP3.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioMp3, (OMX_PTR)&sFormatMP3, eError);
                            break;
                        case OMX_AUDIO_CodingSBC:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingSBC\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatSBC, OMX_AUDIO_PARAM_SBCTYPE);
                            sFormatSBC.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioSbc, (OMX_PTR)&sFormatSBC, eError);
                            break;
                        case OMX_AUDIO_CodingVORBIS:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingVORBIS\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatVORBIS, OMX_AUDIO_PARAM_VORBISTYPE);
                            sFormatVORBIS.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioVorbis, (OMX_PTR)&sFormatVORBIS, eError);
                            break;
                        case OMX_AUDIO_CodingWMA:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingWMA\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatWMA, OMX_AUDIO_PARAM_WMATYPE);
                            sFormatWMA.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioWma, (OMX_PTR)&sFormatWMA, eError);
                            break;
                        case OMX_AUDIO_CodingRA:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingRA\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatRA, OMX_AUDIO_PARAM_RATYPE);
                            sFormatRA.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioRa, (OMX_PTR)&sFormatRA, eError);
                            break;
                        case OMX_AUDIO_CodingMIDI:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_AUDIO_CodingMIDI\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatMIDI, OMX_AUDIO_PARAM_MIDITYPE);
                            sFormatMIDI.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamAudioMidi, (OMX_PTR)&sFormatMIDI, eError);
                            break;
                        default:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "Port %i has invalid eEncoding=0x%X\n",
                                           nPortIndex, sPortFormat.eEncoding);
                            eError = OMX_ErrorUndefined;
                            goto OMX_CONF_TEST_BAIL;

                    }

                }

            }

            i++;
        }

        /* mask the error indicating no more into a no error */
        if (OMX_ErrorNoMore == eError) eError = OMX_ErrorNone;

        if (i == 0x0)
        {
            /* the port did not enumerate a single format */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "Port %i does not support any color or compression formats\n",
                           nPortIndex);
            eError = OMX_ErrorUndefined;
            goto OMX_CONF_TEST_BAIL;
        }

OMX_CONF_TEST_BAIL:
        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE paramtest_videoport(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_VIDEO_PARAM_PORTFORMATTYPE sPortFormat;
        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;
        OMX_VIDEO_PARAM_AVCTYPE sFormatAVC;
        OMX_VIDEO_PARAM_MPEG4TYPE sFormatMpeg4;
        OMX_VIDEO_PARAM_MPEG2TYPE sFormatMpeg2;
        OMX_VIDEO_PARAM_WMVTYPE sFormatWMV;
        OMX_VIDEO_PARAM_RVTYPE sFormatRV;
        OMX_VIDEO_PARAM_H263TYPE sFormatH263;
        OMX_U32 i = 0x0;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Inspecting video port %i\n",
                       nPortIndex);

        OMX_CONF_INIT_STRUCT(sPortFormat, OMX_VIDEO_PARAM_PORTFORMATTYPE);
        sPortFormat.nPortIndex = nPortIndex;
        while ((OMX_ErrorNoMore != eError) && (i < TEST_LOOP_BOUND))
        {
            sPortFormat.nIndex = i;
            eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamVideoPortFormat, (OMX_PTR) & sPortFormat);
            if (OMX_ErrorNoMore != eError)
            {
                /* don't count the error indicating no more data to enumerate
                   as an actual failure */
                OMX_CONF_BAIL_ON_ERROR(eError);

                /* no failure so test this port for it's default parameters */
                eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamVideoPortFormat, (OMX_PTR) & sPortFormat);
                OMX_CONF_BAIL_ON_ERROR(eError);

                /* query port definition type for this format */
                OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
                sPortDefinition.nPortIndex = nPortIndex;
                eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
                OMX_CONF_BAIL_ON_ERROR(eError);

                /* port must default to enabled */
                OMX_CONF_ASSERT(eError, (OMX_TRUE == sPortDefinition.bEnabled),
                                "Port must default to enabled\n");

                /* port must report the domain consistently domain */
                OMX_CONF_ASSERT(eError, (OMX_PortDomainVideo == sPortDefinition.eDomain),
                                "Port must report domain consistently as Video\n");

                /* validate eFormat matches the format selected when setting the port format */
                OMX_CONF_ASSERT(eError, (sPortFormat.eCompressionFormat == sPortDefinition.format.video.eCompressionFormat),
                                "Port format mismatch\n");

                /* validate eFormat matches the format selected when setting the port format */
                OMX_CONF_ASSERT(eError, (sPortFormat.eColorFormat == sPortDefinition.format.video.eColorFormat),
                                "Port format mismatch\n");

                /* apply port definition defaults back into the component */
                eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
                OMX_CONF_BAIL_ON_ERROR(eError);

                /* if port format is compresssed data, validate format specific
                   parameter structure */
                if (OMX_VIDEO_CodingUnused != sPortFormat.eCompressionFormat)
                {
                    switch (sPortFormat.eCompressionFormat)
                    {
                        case OMX_VIDEO_CodingAutoDetect:
                            /* auto detection cannot be tested for parameter
                               configuration */
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_VIDEO_CodingAutoDetect\n",
                                           nPortIndex);
                            break;
                        case OMX_VIDEO_CodingMPEG2:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_VIDEO_CodingMPEG2\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatMpeg2, OMX_VIDEO_PARAM_MPEG2TYPE);
                            sFormatMpeg2.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamVideoMpeg2, (OMX_PTR)&sFormatMpeg2, eError);
                            break;
                        case OMX_VIDEO_CodingH263:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_VIDEO_CodingH263\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatH263, OMX_VIDEO_PARAM_H263TYPE);
                            sFormatH263.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamVideoH263, (OMX_PTR)&sFormatH263, eError);
                            break;
                        case OMX_VIDEO_CodingMPEG4:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_VIDEO_CodingMPEG4\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatMpeg4, OMX_VIDEO_PARAM_MPEG4TYPE);
                            sFormatMpeg4.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamVideoMpeg4, (OMX_PTR)&sFormatMpeg4, eError);
                            break;
                        case OMX_VIDEO_CodingWMV:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_VIDEO_CodingWMV\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatWMV, OMX_VIDEO_PARAM_WMVTYPE);
                            sFormatWMV.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamVideoWmv, (OMX_PTR)&sFormatWMV, eError);
                            break;
                        case OMX_VIDEO_CodingRV:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_VIDEO_CodingRV\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatRV, OMX_VIDEO_PARAM_RVTYPE);
                            sFormatRV.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamVideoRv, (OMX_PTR)&sFormatRV, eError);
                            break;
                        case OMX_VIDEO_CodingAVC:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_VIDEO_CodingAVC\n",
                                           nPortIndex);
                            OMX_CONF_INIT_STRUCT(sFormatAVC, OMX_VIDEO_PARAM_AVCTYPE);
                            sFormatAVC.nPortIndex = nPortIndex;
                            OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamVideoAvc, (OMX_PTR)&sFormatAVC, eError);
                            break;
                        case OMX_VIDEO_CodingMJPEG:
                            /* no parameter structure exists for motion Jpeg */
                            break;
                        default:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "Port %i has invalid eCompressionFormat=0x%X\n",
                                           nPortIndex, sPortFormat.eCompressionFormat);
                            eError = OMX_ErrorUndefined;
                            goto OMX_CONF_TEST_BAIL;

                    }

                }
                /* if port has supported color format, verify color format*/
                if (OMX_COLOR_FormatUnused != sPortFormat.eColorFormat)
                {
                    if ((sPortFormat.eColorFormat >= OMX_COLOR_FormatMonochrome) ||
                            (sPortFormat.eColorFormat <= OMX_COLOR_Format24BitABGR6666))
                    {
                        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting valid color format \n",
                                       nPortIndex);
                    }
                    else
                    {
                        OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "Port %i has invalid eColorFormat=0x%X\n",
                                       nPortIndex, sPortFormat.eColorFormat,
                                       sPortDefinition.format.video.eColorFormat);
                        eError = OMX_ErrorUndefined;
                        goto OMX_CONF_TEST_BAIL;
                    }
                }

            }

            i++;
        }

        /* mask the error indicating no more into a no error */
        if (OMX_ErrorNoMore == eError) eError = OMX_ErrorNone;

        if (i == 0x0)
        {
            /* the port did not enumerate a single format */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "Port %i does not support any color or compression formats\n",
                           nPortIndex);
            eError = OMX_ErrorUndefined;
            goto OMX_CONF_TEST_BAIL;
        }

OMX_CONF_TEST_BAIL:
        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE paramtest_imageport(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_IMAGE_PARAM_PORTFORMATTYPE sPortFormat;
        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;
        OMX_U32 i = 0x0;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Inspecting image port %i\n",
                       nPortIndex);

        OMX_CONF_INIT_STRUCT(sPortFormat, OMX_IMAGE_PARAM_PORTFORMATTYPE);
        sPortFormat.nPortIndex = nPortIndex;
        while ((OMX_ErrorNoMore != eError) && (i < TEST_LOOP_BOUND))
        {
            sPortFormat.nIndex = i;
            eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamImagePortFormat, (OMX_PTR) & sPortFormat);
            if (OMX_ErrorNoMore != eError)
            {
                /* don't count the error indicating no more data to enumerate
                   as an actual failure */
                OMX_CONF_BAIL_ON_ERROR(eError);

                /* no failure so test this port for it's default parameters */
                eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamImagePortFormat, (OMX_PTR) & sPortFormat);
                OMX_CONF_BAIL_ON_ERROR(eError);

                /* query port definition type for this format */
                OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
                sPortDefinition.nPortIndex = nPortIndex;
                eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
                OMX_CONF_BAIL_ON_ERROR(eError);

                /* port must default to enabled */
                OMX_CONF_ASSERT(eError, (OMX_TRUE == sPortDefinition.bEnabled),
                                "Port must default to enabled\n");

                /* port must report the domain consistently domain */
                OMX_CONF_ASSERT(eError, (OMX_PortDomainImage == sPortDefinition.eDomain),
                                "Port must report domain consistently as Image\n");

                /* validate eFormat matches the format selected when setting the port format */
                OMX_CONF_ASSERT(eError, (sPortFormat.eCompressionFormat == sPortDefinition.format.image.eCompressionFormat),
                                "Port format mismatch\n");

                /* validate eFormat matches the format selected when setting the port format */
                OMX_CONF_ASSERT(eError, (sPortFormat.eColorFormat == sPortDefinition.format.image.eColorFormat),
                                "Port format mismatch\n");

                /* apply port definition defaults back into the component */
                eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
                OMX_CONF_BAIL_ON_ERROR(eError);

                /* if port format is compresssed data, validate format specific
                   parameter structure */
                if (OMX_IMAGE_CodingUnused != sPortFormat.eCompressionFormat)
                {
                    switch (sPortFormat.eCompressionFormat)
                    {
                        case OMX_IMAGE_CodingAutoDetect:
                            /* auto detection cannot be tested for parameter
                               configuration */
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_IMAGE_CodingAutoDetect\n",
                                           nPortIndex);
                            break;
                        case OMX_IMAGE_CodingJPEG:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_IMAGE_CodingJPEG\n",
                                           nPortIndex);
                            break;
                        case OMX_IMAGE_CodingJPEG2K:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_IMAGE_CodingJPEG2K\n",
                                           nPortIndex);
                            break;
                        case OMX_IMAGE_CodingEXIF:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_IMAGE_CodingEXIF\n",
                                           nPortIndex);
                            break;
                        case OMX_IMAGE_CodingTIFF:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_IMAGE_CodingTIFF\n",
                                           nPortIndex);
                            break;
                        case OMX_IMAGE_CodingGIF:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_IMAGE_CodingGIF\n",
                                           nPortIndex);
                            break;
                        case OMX_IMAGE_CodingPNG:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_IMAGE_CodingPNG\n",
                                           nPortIndex);
                            break;
                        case OMX_IMAGE_CodingLZW:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_IMAGE_CodingLZW\n",
                                           nPortIndex);
                            break;
                        case OMX_IMAGE_CodingBMP:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting format OMX_IMAGE_CodingBMP\n",
                                           nPortIndex);
                            break;
                        default:
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "Port %i has invalid eCompressionFormat=0x%X\n",
                                           nPortIndex, sPortFormat.eCompressionFormat);
                            eError = OMX_ErrorUndefined;
                            goto OMX_CONF_TEST_BAIL;

                    }

                }

                /* if port has supported color format, verify color format*/
                if (OMX_COLOR_FormatUnused != sPortFormat.eColorFormat)
                {
                    if ((sPortFormat.eColorFormat >= OMX_COLOR_FormatMonochrome) ||
                            (sPortFormat.eColorFormat <= OMX_COLOR_Format24BitABGR6666))
                    {
                        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i inspecting valid color format \n",
                                       nPortIndex);
                    }
                    else
                    {
                        OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "Port %i has invalid eColorFormat=0x%X\n",
                                       nPortIndex, sPortFormat.eColorFormat,
                                       sPortDefinition.format.video.eColorFormat);
                        eError = OMX_ErrorUndefined;
                        goto OMX_CONF_TEST_BAIL;
                    }
                }

            }

            i++;
        }

        /* mask the error indicating no more into a no error */
        if (OMX_ErrorNoMore == eError) eError = OMX_ErrorNone;

        if (i == 0x0)
        {
            /* the port did not enumerate a single format */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "Port %i does not support any color or compression formats\n",
                           nPortIndex);
            eError = OMX_ErrorUndefined;
            goto OMX_CONF_TEST_BAIL;
        }

OMX_CONF_TEST_BAIL:
        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE paramtest_otherport(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_OTHER_PARAM_PORTFORMATTYPE sPortFormat;
        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;
        OMX_U32 i = 0x0;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Inspecting other port %i\n",
                       nPortIndex);

        OMX_CONF_INIT_STRUCT(sPortFormat, OMX_OTHER_PARAM_PORTFORMATTYPE);
        sPortFormat.nPortIndex = nPortIndex;
        while ((OMX_ErrorNoMore != eError) && (i < TEST_LOOP_BOUND))
        {
            sPortFormat.nIndex = i;
            eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamOtherPortFormat, (OMX_PTR) & sPortFormat);
            if (OMX_ErrorNoMore != eError)
            {
                /* don't count the error indicating no more data to enumerate
                   as an actual failure */
                OMX_CONF_BAIL_ON_ERROR(eError);

                /* no failure so test this port for it's default parameters */
                eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamOtherPortFormat, (OMX_PTR) & sPortFormat);
                OMX_CONF_BAIL_ON_ERROR(eError);

                /* query port definition type for this format */
                OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
                sPortDefinition.nPortIndex = nPortIndex;
                eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
                OMX_CONF_BAIL_ON_ERROR(eError);

                /* port must default to enabled */
                OMX_CONF_ASSERT(eError, (OMX_TRUE == sPortDefinition.bEnabled),
                                "Port must default to enabled\n");

                /* port must report the domain consistently domain */
                OMX_CONF_ASSERT(eError, (OMX_PortDomainOther == sPortDefinition.eDomain),
                                "Port must report format as Other\n");

                /* validate eFormat matchs the format selected when setting the port format */
                OMX_CONF_ASSERT(eError, (sPortFormat.eFormat == sPortDefinition.format.other.eFormat),
                                "Port format mismatch\n");

                /* apply port definition defaults back into the component */
                eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition, (OMX_PTR) & sPortDefinition);
                OMX_CONF_BAIL_ON_ERROR(eError);

                /* validate format specific parameter structure */
                switch (sPortFormat.eFormat)
                {
                    case OMX_OTHER_FormatTime:
                        // todo - add config calls required for this port format
                        break;
                    case OMX_OTHER_FormatPower:
                        break;
                    case OMX_OTHER_FormatStats:
                        break;
                    default:
                        // todo - why is vendor specific formats in this enum ... should they not be done
                        //        in the same way as image, video and audio vendor specific formats?
                        if (OMX_OTHER_FormatVendorReserved > sPortFormat.eFormat)
                        {
                            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined,
                                                    "Invalid other format\n");

                        }
                        else
                        {
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Port %i has vendor defined eFormat=0x%X\n",
                                           nPortIndex, sPortFormat.eFormat);
                        }
                        break;

                }

            }

            i++;
        }

        /* mask the error indicating no more into a no error */
        if (OMX_ErrorNoMore == eError) eError = OMX_ErrorNone;

        if (i == 0x0)
        {
            /* the port did not enumerate a single format */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "Port %i does not support any color or compression formats\n",
                           nPortIndex);
            eError = OMX_ErrorUndefined;
            goto OMX_CONF_TEST_BAIL;
        }

OMX_CONF_TEST_BAIL:
        return(eError);
    }


    /*****************************************************************************/
    OMX_ERRORTYPE OMX_CONF_ParameterTest(
        OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_ERRORTYPE eCleanupError = OMX_ErrorNone;
        OMX_HANDLETYPE hComp  = 0x0;
        OMX_HANDLETYPE hWrappedComp = 0x0;
        OMX_CALLBACKTYPE *pWrappedCallbacks;
        OMX_PTR pWrappedAppData;
        OMX_CALLBACKTYPE sCallbacks;
        TEST_CTXTYPE ctx;
        TEST_CTXTYPE *pCtx;
        OMX_BOOL bTimeout;
        OMX_PRIORITYMGMTTYPE sPriorityMgmt;
        OMX_U32 nPortIndex;
        OMX_U32 i, j;
        OMX_STRING cDomainName[NUM_DOMAINS] = {"audio", "video", "image", "other"};

        pCtx = &ctx;
        memset(pCtx, 0x0, sizeof(TEST_CTXTYPE));

        /* create callback tracer */
        sCallbacks.EventHandler    = paramtest_eventhandler;
        sCallbacks.EmptyBufferDone = StubbedEmptyBufferDone;
        sCallbacks.FillBufferDone  = StubbedFillBufferDone;

        eError = OMX_CONF_CallbackTracerCreate(&sCallbacks, (OMX_PTR)pCtx, cComponentName,
                                               &pWrappedCallbacks, &pWrappedAppData);
        OMX_CONF_BAIL_ON_ERROR(eError);


        /* initialize events to track callbacks */
        OMX_OSAL_EventCreate(&ctx.hStateChangeEvent);
        OMX_OSAL_EventReset(ctx.hStateChangeEvent);
        OMX_OSAL_EventCreate(&ctx.hPortStopEvent);
        OMX_OSAL_EventReset(ctx.hPortStopEvent);
        OMX_OSAL_EventCreate(&ctx.hPortReStartEvent);
        OMX_OSAL_EventReset(ctx.hPortReStartEvent);

        /* Initialize OpenMax */
        eError = OMX_Init();
        if (eError != OMX_ErrorNone)
        {
            goto OMX_CONF_TEST_BAIL;
        }

        /* Acquire handle */
        eError = OMX_GetHandle(&hComp, cComponentName, pWrappedAppData, pWrappedCallbacks);
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = OMX_CONF_ComponentTracerCreate(hComp, cComponentName, &hWrappedComp);
        OMX_CONF_BAIL_ON_ERROR(eError);
        pCtx->hWrappedComp = hWrappedComp;

        /* Verify start in Loaded state */
        eError = OMX_GetState(pCtx->hWrappedComp, &pCtx->eState);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_CONF_ASSERT(eError, (OMX_StateLoaded == pCtx->eState),
                        "Component state is not OMX_StateLoaded\n");

        /* validate priority management structure */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Inspecting OMX_IndexParamPriorityMgmt\n");
        OMX_CONF_INIT_STRUCT(sPriorityMgmt, OMX_PRIORITYMGMTTYPE);
        eError = paramtest_bogusparameter(pCtx, OMX_IndexParamPriorityMgmt, (OMX_PTR) & sPriorityMgmt);
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPriorityMgmt, (OMX_PTR) & sPriorityMgmt);
        OMX_CONF_BAIL_ON_ERROR(eError);
        sPriorityMgmt.nGroupPriority = TEST_GROUPPRIORITY;
        sPriorityMgmt.nGroupID = TEST_GROUPID;
        eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamPriorityMgmt, (OMX_PTR) & sPriorityMgmt);
        OMX_CONF_BAIL_ON_ERROR(eError);
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPriorityMgmt, (OMX_PTR) & sPriorityMgmt);
        OMX_CONF_BAIL_ON_ERROR(eError);
        if ((TEST_GROUPPRIORITY != sPriorityMgmt.nGroupPriority) ||
                (TEST_GROUPID != sPriorityMgmt.nGroupID))
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "Unexpected priority management values!\n");
            eError = OMX_ErrorUndefined;
            goto OMX_CONF_TEST_BAIL;
        }

        /* inspect component's ports */
        OMX_CONF_INIT_STRUCT(pCtx->sPortParam[0], OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(hWrappedComp, OMX_IndexParamAudioInit, (OMX_PTR) & pCtx->sPortParam[0]);
        if (OMX_ErrorUnsupportedIndex == eError)  eError = OMX_ErrorNone;
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "detected %i audio ports starting at %i \n",
                       pCtx->sPortParam[0].nPorts, pCtx->sPortParam[0].nStartPortNumber);

        /* detect all video ports on the component */
        OMX_CONF_INIT_STRUCT(pCtx->sPortParam[1], OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(hWrappedComp, OMX_IndexParamVideoInit, (OMX_PTR) & pCtx->sPortParam[1]);
        if (OMX_ErrorUnsupportedIndex == eError)  eError = OMX_ErrorNone;
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "detected %i video ports starting at %i \n",
                       pCtx->sPortParam[1].nPorts, pCtx->sPortParam[1].nStartPortNumber);

        /* detect all image ports on the component */
        OMX_CONF_INIT_STRUCT(pCtx->sPortParam[2], OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(hWrappedComp, OMX_IndexParamImageInit, (OMX_PTR) & pCtx->sPortParam[2]);
        if (OMX_ErrorUnsupportedIndex == eError)  eError = OMX_ErrorNone;
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "detected %i image ports starting at %i \n",
                       pCtx->sPortParam[2].nPorts, pCtx->sPortParam[2].nStartPortNumber);

        /* detect all other ports on the component */
        OMX_CONF_INIT_STRUCT(pCtx->sPortParam[3], OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(hWrappedComp, OMX_IndexParamOtherInit, (OMX_PTR) & pCtx->sPortParam[3]);
        if (OMX_ErrorUnsupportedIndex == eError)  eError = OMX_ErrorNone;
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "detected %i other ports starting at %i \n",
                       pCtx->sPortParam[3].nPorts, pCtx->sPortParam[3].nStartPortNumber);

        /* record total number of ports and allocate port structures */
        pCtx->nNumPorts = pCtx->sPortParam[0].nPorts +
                          pCtx->sPortParam[1].nPorts +
                          pCtx->sPortParam[2].nPorts +
                          pCtx->sPortParam[3].nPorts;

        OMX_CONF_ASSERT(eError, (0x0 != pCtx->nNumPorts),
                        "Component has reported no ports\n");

        /* loop through all domains checking that no 2 port index's overlap */
        for (i = 0x0; i < NUM_DOMAINS; i++)
        {
            if (0x0 != pCtx->sPortParam[i].nPorts)
            {
                /* flag that a port was found */
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "%s domain reported %i ports starting at %i\n",
                               cDomainName[i], pCtx->sPortParam[i].nPorts,
                               pCtx->sPortParam[i].nStartPortNumber);

                for (j = 0x0; j < NUM_DOMAINS; j++)
                {
                    /* don't check for an overlapping port domains against
                       the same domain */
                    if ((i != j) && (0x0 != pCtx->sPortParam[j].nPorts))
                    {
                        if ((pCtx->sPortParam[i].nStartPortNumber >=
                                pCtx->sPortParam[j].nStartPortNumber) &&
                                (pCtx->sPortParam[i].nStartPortNumber <
                                 pCtx->sPortParam[j].nStartPortNumber + pCtx->sPortParam[j].nPorts))
                        {
                            OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "%s port numbers overlap with %s port numbers\n",
                                           cDomainName[i], cDomainName[j]);
                            OMX_CONF_SET_ERROR_BAIL(eError, OMX_ErrorUndefined,
                                                    "Component does not have unique port numbers\n");
                            goto OMX_CONF_TEST_BAIL;

                        }

                    }

                }

            }

        }

        /* process port definitions and parameters on all audio ports */
        for (i = 0x0; i < pCtx->sPortParam[0].nPorts; i++)
        {
            nPortIndex = pCtx->sPortParam[0].nStartPortNumber + i;
            eError = paramtest_buffersupplier(&ctx, nPortIndex);
            OMX_CONF_BAIL_ON_ERROR(eError);
            eError = paramtest_audioport(&ctx, nPortIndex);
            OMX_CONF_BAIL_ON_ERROR(eError);
            eError = paramtest_portenable(&ctx, nPortIndex);
            OMX_CONF_BAIL_ON_ERROR(eError);
        }

        /* process port definitions and parameters on all video ports */
        for (i = 0x0; i < pCtx->sPortParam[1].nPorts; i++)
        {
            nPortIndex = pCtx->sPortParam[1].nStartPortNumber + i;
            eError = paramtest_buffersupplier(&ctx, nPortIndex);
            OMX_CONF_BAIL_ON_ERROR(eError);
            eError = paramtest_videoport(&ctx, nPortIndex);
            OMX_CONF_BAIL_ON_ERROR(eError);
            eError = paramtest_portenable(&ctx, nPortIndex);
            OMX_CONF_BAIL_ON_ERROR(eError);
        }

        /* process port definitions and parameters on all image ports */
        for (i = 0x0; i < pCtx->sPortParam[2].nPorts; i++)
        {
            nPortIndex = pCtx->sPortParam[2].nStartPortNumber + i;
            eError = paramtest_buffersupplier(&ctx, nPortIndex);
            OMX_CONF_BAIL_ON_ERROR(eError);
            eError = paramtest_imageport(&ctx, nPortIndex);
            OMX_CONF_BAIL_ON_ERROR(eError);

            eError = paramtest_portenable(&ctx, nPortIndex);
            OMX_CONF_BAIL_ON_ERROR(eError);
        }

        /* process port definitions and parameters on all other ports */
        for (i = 0x0; i < pCtx->sPortParam[3].nPorts; i++)
        {
            nPortIndex = pCtx->sPortParam[3].nStartPortNumber + i;
            eError = paramtest_buffersupplier(&ctx, nPortIndex);
            OMX_CONF_BAIL_ON_ERROR(eError);
            eError = paramtest_otherport(&ctx, nPortIndex);
            OMX_CONF_BAIL_ON_ERROR(eError);
            eError = paramtest_portenable(&ctx, nPortIndex);
            OMX_CONF_BAIL_ON_ERROR(eError);
        }

        /* command all port to stop, so the test can easily transition the
           component to idle */
        for (i = 0x0; i < pCtx->sPortParam[0].nPorts; i++)
        {
            nPortIndex = pCtx->sPortParam[0].nStartPortNumber + i;
            paramtest_commandport(&ctx, nPortIndex, OMX_FALSE);
            OMX_CONF_BAIL_ON_ERROR(eError);
        }

        /* process port definitions and parameters on all video ports */
        for (i = 0x0; i < pCtx->sPortParam[1].nPorts; i++)
        {
            nPortIndex = pCtx->sPortParam[1].nStartPortNumber + i;
            paramtest_commandport(&ctx, nPortIndex, OMX_FALSE);
            OMX_CONF_BAIL_ON_ERROR(eError);
        }

        /* process port definitions and parameters on all image ports */
        for (i = 0x0; i < pCtx->sPortParam[2].nPorts; i++)
        {
            nPortIndex = pCtx->sPortParam[2].nStartPortNumber + i;
            paramtest_commandport(&ctx, nPortIndex, OMX_FALSE);
            OMX_CONF_BAIL_ON_ERROR(eError);
        }

        /* process port definitions and parameters on all other ports */
        for (i = 0x0; i < pCtx->sPortParam[3].nPorts; i++)
        {
            nPortIndex = pCtx->sPortParam[3].nStartPortNumber + i;
            paramtest_commandport(&ctx, nPortIndex, OMX_FALSE);
            OMX_CONF_BAIL_ON_ERROR(eError);
        }

        /* transition the component to idle */
        OMX_OSAL_EventReset(ctx.hStateChangeEvent);
        eError = OMX_SendCommand(pCtx->hWrappedComp, OMX_CommandStateSet, OMX_StateIdle, 0);
        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_EventWait(ctx.hStateChangeEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
        eError = ((OMX_TRUE == bTimeout) ? OMX_ErrorUndefined : OMX_ErrorNone);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* test OMX_SetParameter calls which should not be accepted with
           the component in the IDLE state */
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPriorityMgmt, (OMX_PTR) & sPriorityMgmt);
        OMX_CONF_BAIL_ON_ERROR(eError);
        sPriorityMgmt.nGroupPriority = TEST_GROUPPRIORITY;
        sPriorityMgmt.nGroupID = TEST_GROUPID;
        eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamPriorityMgmt, (OMX_PTR) & sPriorityMgmt);
        OMX_CONF_ASSERT(eError, (OMX_ErrorIncorrectStateOperation == eError),
                        "Bad return from OMX_SetParameter while OMX_StateIdle\n");
        /* expected error code, so mark the failure as success */
        eError = OMX_ErrorNone;

        /* transition the component to loaded */
        OMX_OSAL_EventReset(ctx.hStateChangeEvent);

        eError = OMX_SendCommand(pCtx->hWrappedComp, OMX_CommandStateSet, OMX_StateLoaded, 0);

        OMX_CONF_BAIL_ON_ERROR(eError);
        OMX_OSAL_EventWait(ctx.hStateChangeEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimeout);
        eError = ((OMX_TRUE == bTimeout) ? OMX_ErrorUndefined : OMX_ErrorNone);
        OMX_CONF_BAIL_ON_ERROR(eError);



OMX_CONF_TEST_BAIL:

        if (hWrappedComp)
        {
            OMX_CONF_ComponentTracerDestroy(hWrappedComp);
        }

        if (hComp)
        {
            if (OMX_ErrorNone == eCleanupError)
            {
                eCleanupError = OMX_FreeHandle(hComp);

            }
            else
            {
                OMX_FreeHandle(hComp);
            }
        }

        if (OMX_ErrorNone != eCleanupError)
        {
            OMX_Deinit();

        }
        else
        {
            eCleanupError = OMX_Deinit();
        }

        OMX_OSAL_EventDestroy(ctx.hStateChangeEvent);
        OMX_OSAL_EventDestroy(ctx.hPortStopEvent);
        OMX_OSAL_EventDestroy(ctx.hPortReStartEvent);

        if (OMX_ErrorNone == eError)
        {
            /* if there were no failures during the test, report any errors found
               during cleanup */
            eError = eCleanupError;
        }

        return (eError);
    }

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */
