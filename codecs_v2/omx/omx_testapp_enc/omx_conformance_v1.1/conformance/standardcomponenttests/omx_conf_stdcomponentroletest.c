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

/** OMX_CONF_StdComponentRoleTest.c
 *  OpenMax IL conformance test - Standard Component Roles Query and Verification Test
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "OMX_CONF_StdCompCommon.h"
#include <string.h>

#define TEST_NAME_STRING "StdComponentRoleTest"

    extern OMX_ERRORTYPE OMX_CONF_StdAudioCapturerTest(OMX_IN OMX_STRING);
    extern OMX_ERRORTYPE OMX_CONF_StdAudioDecoderTest(OMX_IN OMX_STRING);
    extern OMX_ERRORTYPE OMX_CONF_StdAudioProcessorTest(OMX_IN OMX_STRING);
    extern OMX_ERRORTYPE OMX_CONF_StdBinaryAudioReaderTest(OMX_IN OMX_STRING);
    extern OMX_ERRORTYPE OMX_CONF_StdBinaryVideoReaderTest(OMX_IN OMX_STRING);
    extern OMX_ERRORTYPE OMX_CONF_StdBinaryImageReaderTest(OMX_IN OMX_STRING);
    extern OMX_ERRORTYPE OMX_CONF_StdBinaryAudioWriterTest(OMX_IN OMX_STRING);
    extern OMX_ERRORTYPE OMX_CONF_StdBinaryVideoWriterTest(OMX_IN OMX_STRING);
    extern OMX_ERRORTYPE OMX_CONF_StdBinaryImageWriterTest(OMX_IN OMX_STRING);
    extern OMX_ERRORTYPE OMX_CONF_StdImageDecoderTest(OMX_IN OMX_STRING);
    extern OMX_ERRORTYPE OMX_CONF_StdImageEncoderTest(OMX_IN OMX_STRING);
    extern OMX_ERRORTYPE OMX_CONF_StdIVRendererTest(OMX_IN OMX_STRING);
    extern OMX_ERRORTYPE OMX_CONF_StdVideoDecoderTest(OMX_IN OMX_STRING);
    extern OMX_ERRORTYPE OMX_CONF_StdVideoEncoderTest(OMX_IN OMX_STRING);


    /**************************** G L O B A L S **********************************/

    /*****************************************************************************/
    /*  Get all roles of the given standard component and test each of those roles
        for the 'standardness'.
    */
    OMX_ERRORTYPE OMX_CONF_StdComponentRoleTest(
        OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_U32 nNumRoles;
        OMX_STRING sRoles[MAX_COMPONENT_ROLES] = {NULL};
        OMX_U32 i;

        /* The following utility function calls OMX_GetRolesOfComponent,
           allocates memory, and populates strings.
        */
        eError = StdComponentTest_PopulateRolesArray(cComponentName, &nNumRoles, sRoles);
        OMX_CONF_BAIL_ON_ERROR(eError);

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Roles supported by %s : \n", cComponentName);
        for (i = 0; i < nNumRoles; i++)
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\tIndex %d : %s\n", i, sRoles[i]);

        /*
        For each role, find out the class of the standard component.
        Then invoke the standard component test for that class.
        */
        for (i = 0; i < nNumRoles; i++)
        {

            if (strstr(sRoles[i], "audio_capturer") != NULL)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Testing Role %s of %s\n",
                               sRoles[i], cComponentName);
                eError = OMX_CONF_StdAudioCapturerTest(cComponentName);
            }
            else if (strstr(sRoles[i], "audio_decoder") != NULL)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Testing Role %s of %s\n",
                               sRoles[i], cComponentName);
                eError = OMX_CONF_StdAudioDecoderTest(cComponentName);
            }
            else if (strstr(sRoles[i], "audio_processor") != NULL)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Testing Role %s of %s\n",
                               sRoles[i], cComponentName);
                eError = OMX_CONF_StdAudioProcessorTest(cComponentName);
            }
            else if (strstr(sRoles[i], "audio_reader") != NULL)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Testing Role %s of %s\n",
                               sRoles[i], cComponentName);
                eError = OMX_CONF_StdBinaryAudioReaderTest(cComponentName);
            }
            else if (strstr(sRoles[i], "video_reader") != NULL)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Testing Role %s of %s\n",
                               sRoles[i], cComponentName);
                eError = OMX_CONF_StdBinaryVideoReaderTest(cComponentName);
            }
            else if (strstr(sRoles[i], "image_reader") != NULL)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Testing Role %s of %s\n",
                               sRoles[i], cComponentName);
                eError = OMX_CONF_StdBinaryImageReaderTest(cComponentName);
            }
            else if (strstr(sRoles[i], "audio_writer") != NULL)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Testing Role %s of %s\n",
                               sRoles[i], cComponentName);
                eError = OMX_CONF_StdBinaryAudioWriterTest(cComponentName);
            }
            else if (strstr(sRoles[i], "video_writer") != NULL)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Testing Role %s of %s\n",
                               sRoles[i], cComponentName);
                eError = OMX_CONF_StdBinaryVideoWriterTest(cComponentName);
            }
            else if (strstr(sRoles[i], "image_writer") != NULL)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Testing Role %s of %s\n",
                               sRoles[i], cComponentName);
                eError = OMX_CONF_StdBinaryImageWriterTest(cComponentName);
            }
            else if (strstr(sRoles[i], "image_decoder") != NULL)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Testing Role %s of %s\n",
                               sRoles[i], cComponentName);
                eError = OMX_CONF_StdImageDecoderTest(cComponentName);
            }
            else if (strstr(sRoles[i], "image_encoder") != NULL)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Testing Role %s of %s\n",
                               sRoles[i], cComponentName);
                eError = OMX_CONF_StdImageEncoderTest(cComponentName);
            }
            else if (strstr(sRoles[i], "iv_renderer") != NULL)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Testing Role %s of %s\n",
                               sRoles[i], cComponentName);
                eError = OMX_CONF_StdIVRendererTest(cComponentName);
            }
            else if (strstr(sRoles[i], "video_decoder") != NULL)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Testing Role %s of %s\n",
                               sRoles[i], cComponentName);
                eError = OMX_CONF_StdVideoDecoderTest(cComponentName);
            }
            else if (strstr(sRoles[i], "video_encoder") != NULL)
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Testing Role %s of %s\n",
                               sRoles[i], cComponentName);
                eError = OMX_CONF_StdVideoEncoderTest(cComponentName);
            }

            /* Add here more classes when tests are available. */

            else
            {
                eError = OMX_ErrorUndefined;   // How about OMX_ErrorNotStandardComponent.
            }
            OMX_CONF_BAIL_ON_ERROR(eError);

        }

OMX_CONF_TEST_BAIL:
        StdComponentTest_FreeRolesArray(nNumRoles, sRoles);
        return (eError);
    }

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */
