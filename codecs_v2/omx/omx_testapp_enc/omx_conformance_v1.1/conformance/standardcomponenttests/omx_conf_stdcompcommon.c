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

/** OMX_CONF_StdCompCommon.c
 *  OpenMax IL conformance test - Standard Component Test
 *  Contains common code that can be reused by various standard component tests.
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "OMX_CONF_StdCompCommon.h"

#include <string.h>

    /**************************** G L O B A L S **********************************/

    /*****************************************************************************/
    /* Utility function: This function queries the number of the roles of a
       component and allocates memory for those many strings, 128 bytes each.
       The second call to OMX_GetRolesOfComponent populates the roles strings.
    */

    OMX_ERRORTYPE StdComponentTest_PopulateRolesArray(OMX_STRING cComponentName, OMX_U32 *nNumRoles, OMX_STRING *sRolesArray)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_U32 i = 0;

        eError = OMX_Init();
        OMX_CONF_BAIL_ON_ERROR(eError);

        eError = OMX_GetRolesOfComponent(cComponentName, nNumRoles, NULL);
        OMX_CONF_BAIL_ON_ERROR(eError);

        for (i = 0; i < *nNumRoles; i++)
        {
            sRolesArray[i] = (OMX_STRING)(OMX_OSAL_Malloc(MAX_COMPONENT_NAMELENGTH));
            if (sRolesArray[i] == NULL)
                eError = OMX_ErrorInsufficientResources;
            OMX_CONF_BAIL_ON_ERROR(eError);
        }

        eError = OMX_GetRolesOfComponent(cComponentName, nNumRoles, (OMX_U8**) sRolesArray);
        OMX_CONF_BAIL_ON_ERROR(eError);

        eError = OMX_Deinit();
        OMX_CONF_BAIL_ON_ERROR(eError);

OMX_CONF_TEST_BAIL:
        return eError;
    }

    /*****************************************************************************/
    /* Utility function: Release memory used by the roles string array. */

    void StdComponentTest_FreeRolesArray(OMX_U32 nRolesFound, OMX_STRING *sRolesArray)
    {
        OMX_U32 i = 0;

        for (i = 0; i < nRolesFound; i++)
        {
            if (sRolesArray[i] != NULL)
            {
                OMX_OSAL_Free(sRolesArray[i]);
                sRolesArray[i] = NULL;
            }
        }
    }

    /*****************************************************************************/
    /* Test if component supports a role. */

    OMX_ERRORTYPE StdComponentTest_IsRoleSupported(OMX_STRING cComponentName, OMX_STRING cRole)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_U32 nNumRoles = 0;
        OMX_STRING sRoles[MAX_COMPONENT_ROLES];
        OMX_BOOL bRoleSupported = OMX_FALSE;
        OMX_U32 i;

        eError = StdComponentTest_PopulateRolesArray(cComponentName, &nNumRoles, sRoles);
        OMX_CONF_BAIL_ON_ERROR(eError);

        for (i = 0; i < nNumRoles; i++)
        {
            if (strstr(sRoles[i], cRole) != NULL)
            {
                bRoleSupported = OMX_TRUE;
                break;
            }
        }
        if (bRoleSupported == OMX_TRUE)
            eError = OMX_ErrorNone;
        else
            eError = OMX_ErrorComponentNotFound;

OMX_CONF_TEST_BAIL:

        StdComponentTest_FreeRolesArray(nNumRoles, sRoles);
        return eError;
    }

    /*****************************************************************************/
    /* Set a role for a component */

    OMX_ERRORTYPE StdComponentTest_SetRole(TEST_CTXTYPE *pCtx, OMX_STRING cRole)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_PARAM_COMPONENTROLETYPE sRole;

        OMX_CONF_INIT_STRUCT(sRole, OMX_PARAM_COMPONENTROLETYPE);
        strcpy((OMX_STRING) sRole.cRole, cRole);

        eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamStandardComponentRole, (OMX_PTR) & sRole);
        OMX_CONF_BAIL_ON_ERROR(eError);

OMX_CONF_TEST_BAIL:
        return (eError);

    }

    /*****************************************************************************/
    /* Verify that the index OMX_IndexParamCompBufferSupplier is supported and
       the preferences could be set to any valid value. */

    OMX_ERRORTYPE StdComponentTest_BufferSupplier(TEST_CTXTYPE *pCtx, OMX_U32 nPortIndex)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_PARAM_BUFFERSUPPLIERTYPE sBufferSupplier;

        OMX_CONF_INIT_STRUCT(sBufferSupplier, OMX_PARAM_BUFFERSUPPLIERTYPE);
        sBufferSupplier.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamCompBufferSupplier,
                                  (OMX_PTR) & sBufferSupplier);
        OMX_CONF_BAIL_ON_ERROR(eError);

        sBufferSupplier.eBufferSupplier = OMX_BufferSupplyInput;
        eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamCompBufferSupplier,
                                  (OMX_PTR) & sBufferSupplier);
        OMX_CONF_BAIL_ON_ERROR(eError);

        sBufferSupplier.eBufferSupplier = OMX_BufferSupplyOutput;
        eError = OMX_SetParameter(pCtx->hWrappedComp, OMX_IndexParamCompBufferSupplier,
                                  (OMX_PTR) & sBufferSupplier);
        OMX_CONF_BAIL_ON_ERROR(eError);

OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/
    /* Inspect componet ports.
       This also verifies that the component supports OMX_IndexParam*****Init.
    */

    OMX_ERRORTYPE StdComponentTest_InspectPorts(TEST_CTXTYPE *pCtx)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        /* Check support for OMX_IndexParam*****Init. */
        OMX_CONF_INIT_STRUCT(pCtx->sPortParamAudio, OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamAudioInit,
                                  (OMX_PTR) & pCtx->sPortParamAudio);
        OMX_CONF_BAIL_ON_ERROR(eError);

        OMX_CONF_INIT_STRUCT(pCtx->sPortParamVideo, OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamVideoInit,
                                  (OMX_PTR) & pCtx->sPortParamVideo);
        OMX_CONF_BAIL_ON_ERROR(eError);

        OMX_CONF_INIT_STRUCT(pCtx->sPortParamImage, OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamImageInit,
                                  (OMX_PTR) & pCtx->sPortParamImage);
        OMX_CONF_BAIL_ON_ERROR(eError);

        OMX_CONF_INIT_STRUCT(pCtx->sPortParamOther, OMX_PORT_PARAM_TYPE);
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamOtherInit,
                                  (OMX_PTR) & pCtx->sPortParamOther);
        OMX_CONF_BAIL_ON_ERROR(eError);


OMX_CONF_TEST_BAIL:

        return(eError);
    }

    /*****************************************************************************/

    OMX_BOOL StdComponentTest_IsInputPort(TEST_CTXTYPE *pCtx, OMX_U32 nPortIndex)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;

        OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
        sPortDefinition.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition,
                                  (OMX_PTR) & sPortDefinition);
        OMX_CONF_BAIL_ON_ERROR(eError);

        if (sPortDefinition.eDir == OMX_DirInput)
            return OMX_TRUE;

OMX_CONF_TEST_BAIL:
        return OMX_FALSE;
    }

    /*****************************************************************************/

    OMX_BOOL StdComponentTest_IsOutputPort(TEST_CTXTYPE *pCtx, OMX_U32 nPortIndex)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;

        OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
        sPortDefinition.nPortIndex = nPortIndex;
        eError = OMX_GetParameter(pCtx->hWrappedComp, OMX_IndexParamPortDefinition,
                                  (OMX_PTR) & sPortDefinition);
        OMX_CONF_BAIL_ON_ERROR(eError);

        if (sPortDefinition.eDir == OMX_DirOutput)
            return OMX_TRUE;

OMX_CONF_TEST_BAIL:
        return OMX_FALSE;
    }

    /*****************************************************************************/

    OMX_ERRORTYPE StdComponentTest_StdPortParameters(
        TEST_CTXTYPE *pCtx,
        OMX_U32 nPortIndex)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_PARAM_PORTDEFINITIONTYPE sPortDefinition;
        OMX_PARAM_BUFFERSUPPLIERTYPE sBufferSupplier;

        /* Check support for OMX_IndexParamPortDefinition. */
        sPortDefinition.nPortIndex = nPortIndex;
        OMX_CONF_INIT_STRUCT(sPortDefinition, OMX_PARAM_PORTDEFINITIONTYPE);
        OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamPortDefinition,
                                      (OMX_PTR)&sPortDefinition, eError);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* Check support for OMX_IndexParamCompBufferSupplier. */
        OMX_CONF_INIT_STRUCT(sBufferSupplier, OMX_PARAM_BUFFERSUPPLIERTYPE);
        sBufferSupplier.nPortIndex = nPortIndex;

        sBufferSupplier.eBufferSupplier = OMX_BufferSupplyInput;
        OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamCompBufferSupplier,
                                      (OMX_PTR)&sBufferSupplier, eError);
        OMX_CONF_BAIL_ON_ERROR(eError);

        sBufferSupplier.eBufferSupplier = OMX_BufferSupplyOutput;
        OMX_CONF_PARAM_REQUIREDGETSET(pCtx, OMX_IndexParamCompBufferSupplier,
                                      (OMX_PTR)&sBufferSupplier, eError);
        OMX_CONF_BAIL_ON_ERROR(eError);


OMX_CONF_TEST_BAIL:
        return(eError);
    }

    /*****************************************************************************/
    /* This is the common code reuseable by all standard component tests.
       This routine creates the component and sets up the required entities for
       the test. Then it runs the actual component test passed on by pCompTest.
    */
    OMX_ERRORTYPE StdComponentTest_StdComp(
        OMX_IN OMX_STRING cComponentName,
        TEST_CTXTYPE *pCtx,
        STDCOMPTEST_COMPONENT pCompTest)
    {
        OMX_ERRORTYPE eError = OMX_ErrorNone;
        OMX_ERRORTYPE eCleanupError = OMX_ErrorNone;
        OMX_HANDLETYPE hComp  = 0x0;
        OMX_HANDLETYPE hWrappedComp = 0x0;
        OMX_CALLBACKTYPE *pWrappedCallbacks;
        OMX_PTR pWrappedAppData;
        OMX_CALLBACKTYPE sCallbacks;

        /* create callback tracer */
        sCallbacks.EventHandler    = StubbedEventHandler;
        sCallbacks.EmptyBufferDone = StubbedEmptyBufferDone;
        sCallbacks.FillBufferDone  = StubbedFillBufferDone;

        eError = OMX_CONF_CallbackTracerCreate(&sCallbacks, (OMX_PTR)pCtx, cComponentName,
                                               &pWrappedCallbacks, &pWrappedAppData);
        OMX_CONF_BAIL_ON_ERROR(eError);

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

        /* Inspect what ports the cmponent has.
           This also checks for the support for the following OMX_IndexParam*****Init.
        */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Inspecting component ports\n");
        eError = StdComponentTest_InspectPorts(pCtx);
        OMX_CONF_BAIL_ON_ERROR(eError);

        /* All set up done, run the actual component test now.
        */
        eError = pCompTest(pCtx);
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

        if (OMX_ErrorNone == eError)
        {
            /* if there were no failures during the test, report any errors found
               during cleanup */
            eError = eCleanupError;
        }

        return (eError);
    }

    /*****************************************************************************/


#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */
