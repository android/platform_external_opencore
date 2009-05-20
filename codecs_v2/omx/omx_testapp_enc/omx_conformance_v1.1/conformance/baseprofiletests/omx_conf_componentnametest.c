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

/** OMX_CONF_ComponentNameTest.c
 *  OpenMax IL conformance test - Component Name Test
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "OMX_OSAL_Interfaces.h"
#include "OMX_CONF_TestHarness.h"

#include <string.h>
#include <stdlib.h>

#define TEST_NAME_STRING "ComponentNameTest"
#define TEST_COMPONENT_NAME_SIZE 128

    static char szDesc[256];

    OMX_ERRORTYPE ComponentNameTest_EventHandler(
        OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_EVENTTYPE eEvent,
        OMX_IN OMX_U32 nData1,
        OMX_IN OMX_U32 nData2,
        OMX_IN OMX_PTR pEventData)
    {
        UNUSED_PARAMETER(hComponent);
        UNUSED_PARAMETER(pAppData);
        UNUSED_PARAMETER(eEvent);
        UNUSED_PARAMETER(nData1);
        UNUSED_PARAMETER(nData2);
        UNUSED_PARAMETER(pEventData);

        return OMX_ErrorNotImplemented;
    }

    OMX_ERRORTYPE ComponentNameTest_EmptyBufferDone(
        OMX_OUT OMX_HANDLETYPE hComponent,
        OMX_OUT OMX_PTR pAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer)
    {
        UNUSED_PARAMETER(hComponent);
        UNUSED_PARAMETER(pAppData);
        UNUSED_PARAMETER(pBuffer);

        return OMX_ErrorNotImplemented;
    }

    OMX_ERRORTYPE ComponentNameTest_FillBufferDone(
        OMX_OUT OMX_HANDLETYPE hComponent,
        OMX_OUT OMX_PTR pAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer)
    {
        UNUSED_PARAMETER(hComponent);
        UNUSED_PARAMETER(pAppData);
        UNUSED_PARAMETER(pBuffer);

        return OMX_ErrorNotImplemented;
    }


    OMX_ERRORTYPE OMX_CONF_ComponentNameTest(OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE  eError = OMX_ErrorNone;
        OMX_HANDLETYPE hComp  = 0;
        OMX_U32 i = 0;
        OMX_BOOL bNameValid = OMX_FALSE;
        OMX_BOOL bFound = OMX_FALSE;
        OMX_S8 cCompEnumName[TEST_COMPONENT_NAME_SIZE];
        OMX_CALLBACKTYPE sCallbacks;

        sCallbacks.EventHandler    = ComponentNameTest_EventHandler;
        sCallbacks.EmptyBufferDone = ComponentNameTest_EmptyBufferDone;
        sCallbacks.FillBufferDone  = ComponentNameTest_FillBufferDone;

        /* Initialize OpenMax */
        eError = OMX_Init();
        if (eError != OMX_ErrorNone)
        {
            goto OMX_CONF_TEST_BAIL;
        }

        while (OMX_ErrorNone == eError)
        {
            /* loop through all enumerated components to determine if the component name
               specificed for the test is enumerated by the OMX core */
            eError = OMX_ComponentNameEnum((OMX_STRING) cCompEnumName, TEST_COMPONENT_NAME_SIZE, i);
            if (OMX_ErrorNone == eError)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "OMX_ComponenNameEnum enumerated %s at index %i\n",
                               cCompEnumName, i);

                if (!strcmp((OMX_STRING) cCompEnumName, cComponentName))
                {
                    /* component name enumerated by OMX_CORE */
                    bFound = OMX_TRUE;

                    eError = OMX_GetHandle(&hComp, (OMX_STRING) cCompEnumName, 0x0, &sCallbacks);
                    if (eError == OMX_ErrorNone)
                    {
                        /* validate the first 4 characters of the name match
                           the OMX standard */
                        if (!strncmp("OMX.", cComponentName, 4))
                        {
                            bNameValid = OMX_TRUE;
                        }

                        eError = OMX_FreeHandle(hComp);

                    }
                    else
                    {
                        OMX_CONF_ErrorToString(eError, szDesc);
                        OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "%s error=0x%X (%s) from OMX_GetHandle\n",
                                       cCompEnumName, eError, szDesc);

                    }

                }

            }
            else if (OMX_ErrorNoMore != eError)
            {
                /* OMX_CORE reported unexpected error other than no more components */
                OMX_CONF_ErrorToString(eError, szDesc);
                OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "unexepected error=0x%X (%s) from OMX_ComponenNameEnum\n",
                               eError, szDesc);
            }
            i++;

        }

        if (OMX_ErrorNoMore == eError)
        {
            /* not an error, so clear the error code */
            eError = OMX_ErrorNone;
        }


OMX_CONF_TEST_BAIL:

        if (OMX_ErrorNone == eError)
        {
            eError = OMX_Deinit();

        }
        else
        {
            OMX_Deinit();
        }

        return eError;
    }

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */

