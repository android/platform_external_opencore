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
#include "omxdectest_basic_test.h"
#include "oscl_mem.h"

#define CHECK_ERROR_ROLE_TEST(e, s) \
    if (OMX_ErrorNone != (e))\
    {\
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestCompRole - %s Error, Stop the test case", s));\
        iTestStatus = OMX_FALSE;\
        iState = StateStop;\
        RunIfNotReady();\
        break;\
    }

#define CHECK_MEM_ROLE_TEST(m, s) \
    if (NULL == (m))\
    {\
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestCompRole - %s Error, Memory Allocation Failed", s));\
        iTestStatus = OMX_FALSE;\
        iState = StateStop;\
        RunIfNotReady();\
        break;\
    }


/*
 * Active Object class's Run () function
 * Control all the states of AO & sends API's to the component
 */

void OmxDecTestCompRole::Run()
{
    switch (iState)
    {
        case StateUnLoaded:
        {
            OMX_ERRORTYPE Err;
            OMX_U32 ii;
            OMX_U32 NumComps = 0;
            OMX_STRING* pCompOfRole = NULL;
            OMX_BOOL IsRoleSupported = OMX_FALSE;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestCompRole::Run() - StateUnLoaded IN"));

#if PROXY_INTERFACE
            ipThreadSafeHandlerEventHandler = OSCL_NEW(EventHandlerThreadSafeCallbackAO, (this, EVENT_HANDLER_QUEUE_DEPTH, "EventHandlerAO"));
            ipThreadSafeHandlerEmptyBufferDone = OSCL_NEW(EmptyBufferDoneThreadSafeCallbackAO, (this, EMPTY_BUFFER_DONE_QUEUE_DEPTH, "EmptyBufferDoneAO"));
            ipThreadSafeHandlerFillBufferDone = OSCL_NEW(FillBufferDoneThreadSafeCallbackAO, (this, FILL_BUFFER_DONE_QUEUE_DEPTH, "FillBufferDoneAO"));

            if ((NULL == ipThreadSafeHandlerEventHandler) ||
                    (NULL == ipThreadSafeHandlerEmptyBufferDone) ||
                    (NULL == ipThreadSafeHandlerFillBufferDone))
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                (0, "OmxDecTestCompRole::Run() - Error, ThreadSafe Callback Handler initialization failed, OUT"));

                iState = StateUnLoaded;
                OsclExecScheduler* sched = OsclExecScheduler::Current();
                sched->StopScheduler();
            }
#endif

            if (!iCallbacks->initCallbacks())
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestCompRole::Run() - ERROR initCallbacks failed, OUT"));
                iTestStatus = OMX_FALSE;
                iState = StateStop;
                RunIfNotReady();
                break;
            }

            ipAppPriv = (AppPrivateType*) oscl_malloc(sizeof(AppPrivateType));
            CHECK_MEM_ROLE_TEST(ipAppPriv, "Component_Handle");

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestCompRole::Run() - OMX_Init pre"));


            //This should be the first call to the component to load it.
            Err = OMX_Init();
            CHECK_ERROR_ROLE_TEST(Err, "OMX_Init");
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestCompRole::Run() - OMX_Init done"));


            if (NULL != iName)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxDecTestCompRole::Run() - Finding out whether the component %s supports the correct role", iName));

                //Given the component name, verify whether it supports correct role or not
                Err = OMX_GetRolesOfComponent(iName, &NumComps, NULL);

                if (OMX_ErrorNone != Err || NumComps < 1)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestCompRole::Run() - ERROR, No role is supported by the given component"));
                    iTestStatus = OMX_FALSE;
                    ipAppPriv->Handle = NULL;
                    iState = StateStop;
                    RunIfNotReady();
                    break;
                }

                pCompOfRole = (OMX_STRING*) oscl_malloc(NumComps * sizeof(OMX_STRING));
                CHECK_MEM_ROLE_TEST(pCompOfRole, "ComponentRoleArray");

                for (ii = 0; ii < NumComps; ii++)
                {
                    pCompOfRole[ii] = (OMX_STRING) oscl_malloc(PV_OMX_MAX_COMPONENT_NAME_LENGTH * sizeof(OMX_U8));
                    CHECK_MEM_ROLE_TEST(pCompOfRole[ii], "ComponentRoleArray");
                }

                if (OMX_FALSE == iTestStatus)
                {
                    break;
                }

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                (0, "OmxDecTestCompRole::Run() - Find out the roles supported now"));

                Err = OMX_GetRolesOfComponent(iName, &NumComps, (OMX_U8**) pCompOfRole);
                CHECK_ERROR_ROLE_TEST(Err, "GetRolesOfComponent");

                for (ii = 0; ii < NumComps; ii++)
                {
                    if (NULL != oscl_strstr(pCompOfRole[ii], iRole))
                    {
                        IsRoleSupported = OMX_TRUE;
                        break;
                    }
                }

                if (OMX_FALSE == IsRoleSupported)
                {
                    iTestStatus = OMX_FALSE;
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "OmxDecTestCompRole::Run() - ERROR, Required Component Role not supported!"));
                    iState = StateStop;
                    RunIfNotReady();
                    break;
                }

                Err = OMX_GetHandle(&ipAppPriv->Handle, iName, (OMX_PTR) this , iCallbacks->getCallbackStruct());

                // check if there was a problem
                CHECK_ERROR_ROLE_TEST(Err, "GetHandle");
                CHECK_MEM_ROLE_TEST(ipAppPriv->Handle, "ComponentHandle");

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestCompRole::Run() - Got Handle for the component %s", iName));

            }
            else if (NULL != iRole)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestCompRole::Run() - Finding out the components that can support the role %s", iRole));

                //Given the role, determine the component first & then get the handle
                // Call once to find out the number of components that can fit the role
                Err = OMX_GetComponentsOfRole(iRole, &NumComps, NULL);

                if (OMX_ErrorNone != Err || NumComps < 1)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestCompRole::Run() - ERROR, No component can handle the specified role %s", iRole));
                    iTestStatus = OMX_FALSE;
                    ipAppPriv->Handle = NULL;
                    iState = StateStop;
                    RunIfNotReady();
                    break;
                }

                pCompOfRole = (OMX_STRING*) oscl_malloc(NumComps * sizeof(OMX_STRING));
                CHECK_MEM_ROLE_TEST(pCompOfRole, "ComponentRoleArray");

                for (ii = 0; ii < NumComps; ii++)
                {
                    pCompOfRole[ii] = (OMX_STRING) oscl_malloc(PV_OMX_MAX_COMPONENT_NAME_LENGTH * sizeof(OMX_U8));
                    CHECK_MEM_ROLE_TEST(pCompOfRole[ii], "ComponentRoleArray");
                }

                if (OMX_FALSE == iTestStatus)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                                    (0, "OmxDecTestCompRole::Run() - Error occured in this state, StateUnLoaded OUT"));
                    break;
                }

                // call 2nd time to get the component names
                Err = OMX_GetComponentsOfRole(iRole, &NumComps, (OMX_U8**) pCompOfRole);
                CHECK_ERROR_ROLE_TEST(Err, "GetComponentsOfRole");

                for (ii = 0; ii < NumComps; ii++)
                {
                    // try to create component
                    Err = OMX_GetHandle(&ipAppPriv->Handle, (OMX_STRING) pCompOfRole[ii], (OMX_PTR) this, iCallbacks->getCallbackStruct());
                    // if successful, no need to continue
                    if ((OMX_ErrorNone == Err) && (NULL != ipAppPriv->Handle))
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "OmxDecTestCompRole::Run() - Got Handle for the component %s", pCompOfRole[ii]));
                        break;
                    }
                    else
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestCompRole::Run() - ERROR, Cannot get component %s handle, try another if possible", pCompOfRole[ii]));
                    }
                }

                CHECK_ERROR_ROLE_TEST(Err, "GetHandle");
                CHECK_MEM_ROLE_TEST(ipAppPriv->Handle, "ComponentHandle");
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

            iState = StateStop;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestCompRole::Run() - StateUnLoaded OUT, moving to next state"));

            RunIfNotReady();
        }
        break;

        /********* FREE THE HANDLE & CLOSE FILES FOR THE COMPONENT ********/
        case StateStop:
        {
            OMX_U8 TestName[] = "GET_ROLES_TEST";
            OMX_ERRORTYPE Err = OMX_ErrorNone;

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestCompRole::Run() - StateStop IN"));

            if (ipAppPriv)
            {
                if (ipAppPriv->Handle)
                {
                    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                                    (0, "OmxDecTestCompRole::Run() - Free the Component Handle"));

                    Err = OMX_FreeHandle(ipAppPriv->Handle);
                    if (OMX_ErrorNone != Err)
                    {
                        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestCompRole::Run() - FreeHandle Error"));
                        iTestStatus = OMX_FALSE;
                    }
                }
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "OmxDecTestCompRole::Run() - De-initialize the omx component"));

            Err = OMX_Deinit();
            if (OMX_ErrorNone != Err)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "OmxDecTestCompRole::Run() - OMX_Deinit Error"));
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
#ifdef PRINT_RESULT
                fprintf(iConsOutFile, "%s: Fail \n", TestName);
#endif

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                                (0, "OmxComponentDecTest::VerifyOutput() - %s : Fail", TestName));
            }
            else
            {
#ifdef PRINT_RESULT
                fprintf(iConsOutFile, "%s: Success \n", TestName);
#endif

                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO,
                                (0, "OmxComponentDecTest::VerifyOutput() - %s : Success", TestName));
            }

            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "OmxDecTestCompRole::Run() - StateStop OUT"));

            iState = StateUnLoaded;
            OsclExecScheduler* sched = OsclExecScheduler::Current();
            sched->StopScheduler();
        }
        break;

        default:
        {
            break;
        }
    }
    return ;
}
