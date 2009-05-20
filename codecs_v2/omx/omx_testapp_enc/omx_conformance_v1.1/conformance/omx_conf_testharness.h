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

/** OMX_CONF_TestHarness.h
 *  Header for the OpenMAX IL conformance test harness that accepts a list of commands
 *  (from a prompt or from file) to set tracing/logfiles, add/remove tests from the list
 *  applied to components, and ultimately launch testing against components).
 */

#ifndef OMX_CONF_TestHarness_h
#define OMX_CONF_TestHarness_h

#include <OMX_Types.h>
#include <OMX_Core.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

    /***********************************************************************
     * GLOBAL VARIABLES/CONSTANTS
     ***********************************************************************/
    extern OMX_VERSIONTYPE g_OMX_CONF_Version;
#define OMX_CONF_TIMEOUT_EXPECTING_SUCCESS 500  /* duration of event timeout in msec when we expect event to be set */
#define OMX_CONF_TIMEOUT_EXPECTING_FAILURE 2000 /* duration of event timeout in msec when we don't expect event to be set */

    /***********************************************************************
     * UTILITY FUNCTIONS
     ***********************************************************************/

    /* Constants for when we are waiting on buffer traffic (to implement "wait a while" in OMX_StateExecuting). */
#define OMX_CONF_TIMEOUT_BUFFER_TRAFFIC    5000 /* duration of event timeout in msec when we are waiting on buffer traffic */
#define OMX_CONF_BUFFERS_OF_TRAFFIC 100 /* number of buffers exchanged by TTC's Fill/EmptyThisBuffer when we are waiting on buffer traffic */

    /* Force all the component's ports to be suppliers/non-suppliers */
    OMX_ERRORTYPE OMX_CONF_ForceSuppliers(OMX_HANDLETYPE hComp, OMX_BOOL bSupplier);

    /* Examines the direction of all the component's ports to determine if it is a source */
    OMX_ERRORTYPE OMX_CONF_IsSource(OMX_HANDLETYPE hComp, OMX_BOOL *pbIsSource);

    /* Examines the direction of all the component's ports to determine if it is a sink */
    OMX_ERRORTYPE OMX_CONF_IsSink(OMX_HANDLETYPE hComp, OMX_BOOL *pbIsSink);

    /**********************************************************************
     * COMMON MACROS
     **********************************************************************/
#define INIT_PARAM(_X_)  (((_X_).nSize = sizeof (_X_)), (_X_).nVersion = g_OMX_CONF_Version)
#define INIT_CONFIG(_X_)  (((_X_).nSize = sizeof (_X_)), (_X_).nVersion = g_OMX_CONF_Version)

    /**********************************************************************
     * UNUSED PARAMETERS
     **********************************************************************/
#define UNUSED_PARAMETER(p) (void)(p)

    /**********************************************************************
     * TRACING
     **********************************************************************/


#define OMX_CONF_FAIL() OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "%s:%d FAILED\n", __FILE__, __LINE__);  \
    eError = OMX_ErrorUndefined;                                                                    \
    goto OMX_CONF_TEST_FAIL;

#define OMX_CONF_FAIL_IF_NEQ( __X, __Y ) if( (long)(__X) != (long)(__Y) ) {         \
         OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "!!! %s:%d %s (%ld)!= %s (%ld)\n",     \
                        __FILE__, __LINE__, #__X, (long)(__X), #__Y, (long)(__Y));  \
         eError = OMX_ErrorUndefined;                                               \
         goto OMX_CONF_TEST_FAIL;                                                   \
    }

#define OMX_CONF_FAIL_IF( __X) if(__X) {                        \
         OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "!!! %s:%d %s\n",  \
                        __FILE__, __LINE__, #__X);              \
         eError = OMX_ErrorUndefined;                           \
         goto OMX_CONF_TEST_FAIL;                               \
    }

#define OMX_CONF_FAIL_IF_ERROR( __X ) eError = __X;                     \
    if( OMX_ErrorNone != eError ) {                                     \
        char szDesc[256];                                               \
        OMX_CONF_ErrorToString( eError, szDesc );                       \
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "!!! %s:%d Error %x %s\n",  \
                       __FILE__, __LINE__, eError, szDesc);             \
        goto OMX_CONF_TEST_FAIL;                                        \
    }

#define OMX_CONF_REMEMBER_ERROR(__X) if( OMX_ErrorNone != (eTemp = __X) && (OMX_ErrorNone == eError)) {  \
       eError = eTemp;                                                                                   \
    }

    /* Component Tracer

       A component tracer is a thin wrapper around a component structure (OMX_COMPONENTTYPE)
       that adds trace message displaying the call sequences, parameters, and return values
       to component method calls.

       The IL client should create a component tracer around a OMX_COMPONENTTYPE structure
       after it has been returned by OMX_GetHandle. The IL client should use the returned
       component wrapper instead of the original component interface.
    */
    OMX_ERRORTYPE OMX_CONF_ComponentTracerCreate(OMX_IN OMX_HANDLETYPE hOrigComp,
            OMX_IN OMX_STRING sComponentName,
            OMX_OUT OMX_HANDLETYPE *phWrappedComp);
    OMX_ERRORTYPE OMX_CONF_ComponentTracerDestroy(OMX_IN OMX_HANDLETYPE hWrappedComp);

    /* Component Tracer

       A callback tracer is a thin wrapper around a callback structure (OMX_CALLBACKTYPE)
       that adds trace message displaying the call sequences, parameters, and return values
       to callback calls. The tracer also stores the name of the component that will use the
       given callbacks and appdata.

       The IL client should create a callback tracer around a OMX_CALLBACK structure and the
       accompanying appdata prior to OMX_GetHandle. The IL client should pass the returned
       callbacks and appdata wrappers to OMX_GetHandle instead of the original callbacks and
       appdata.
    */
    OMX_ERRORTYPE OMX_CONF_CallbackTracerCreate(OMX_IN OMX_CALLBACKTYPE *pOrigCallbacks,
            OMX_IN OMX_PTR pOrigAppData,
            OMX_IN OMX_STRING sComponentName,
            OMX_OUT OMX_CALLBACKTYPE **ppWrappedCallbacks,
            OMX_OUT OMX_PTR *ppWrappedAppData);
    OMX_ERRORTYPE OMX_CONF_CallbackTracerDestroy(OMX_IN OMX_CALLBACKTYPE *pWrappedCallbacks,
            OMX_OUT OMX_PTR *pWrappedAppData);

    /* Trace utility functions */
    OMX_ERRORTYPE OMX_CONF_ErrorToString(OMX_IN OMX_ERRORTYPE eError, OMX_OUT OMX_STRING sName);
    OMX_ERRORTYPE OMX_CONF_StateToString(OMX_IN OMX_STATETYPE eState, OMX_OUT OMX_STRING sName);
    OMX_ERRORTYPE OMX_CONF_CommandToString(OMX_IN OMX_COMMANDTYPE eCommand, OMX_OUT OMX_STRING sName);
    OMX_ERRORTYPE OMX_CONF_IndexToString(OMX_IN OMX_INDEXTYPE nIndex, OMX_OUT OMX_STRING sName);
    OMX_ERRORTYPE OMX_OSAL_GetTracePrefix(OMX_IN OMX_U32 nTraceFlags, OMX_OUT OMX_STRING szPrefix);

    /**********************************************************************
     * TEST FUNCTION PROTOTYPES
     **********************************************************************/

    /** The signature of all test functions should match that of OMX_CONF_TESTFUNCTION
     *   so test names may be mapped to test functions (i.e. via the g_OMX_CONF_TestLookupTable). */
    typedef OMX_ERRORTYPE(*OMX_CONF_TESTFUNCTION)(OMX_IN OMX_STRING cComponentName);

#define OMX_CONF_TestFlag_Base          0x80000000
#define OMX_CONF_TestFlag_Interop       0x01
#define OMX_CONF_TestFlag_Threaded      0x02
#define OMX_CONF_TestFlag_ClockComp     0x04
#define OMX_CONF_TestFlag_Seeking       0x08
#define OMX_CONF_TestFlag_AutoOutput    0x10
#define OMX_CONF_TestFlag_StdComponent  0x20

    /**< Maps a test name to a test entrypoint */
    typedef struct OMX_CONF_TESTLOOKUPTYPE
    {
        char *pName;
        OMX_CONF_TESTFUNCTION pFunc;
        OMX_U32 nFlags;
    } OMX_CONF_TESTLOOKUPTYPE;

#define OMX_CONF_MAXTESTNUMBER 200
    extern OMX_CONF_TESTLOOKUPTYPE g_OMX_CONF_TestLookupTable[];
    extern OMX_U32 g_OMX_CONF_nTestLookupTableEntries;

    /**********************************************************************
     * TEST HARNESS INTERFACE
     *
     * This interface may be called programmatically or indirectly via
     * a conformance test script as described below.
     *********************************************************************/

    /** Set the trace types that are enabled. nTraceFlags are a bitmask of
     *  OMX_OSAL_Trace flags indicating which type of messages will be emit.*/
    OMX_ERRORTYPE OMX_CONF_SetTraceflags(OMX_IN OMX_U32 nTraceFlags);

    /** Add the given test those that will be executed upon components on a
     *  OMX_CONF_TestComponent. sTestname must be one of the strings listed in
     *  g_OMX_CONF_TestLookupTable or "*" which indicates all tests. */
    OMX_ERRORTYPE OMX_CONF_AddTest(OMX_IN OMX_STRING sTestName);

    /** Remove the given test to those that will be executed upon components on a
     *  OMX_CONF_TestComponent. sTestname must be one of the strings listed in
     *  g_OMX_CONF_TestLookupTable or "*" which indicates all tests. */
    OMX_ERRORTYPE OMX_CONF_RemoveTest(OMX_IN OMX_STRING sTestName);

    /** List all available tests */
    OMX_ERRORTYPE OMX_CONF_ListTests();

    /** List all available components */
    OMX_ERRORTYPE OMX_CONF_ListComponents();

    /** Test the component using the current trace flags, input/output mapping, and test settings */
    OMX_ERRORTYPE OMX_CONF_TestComponent(OMX_IN OMX_STRING sComponentName, OMX_BOOL *bPassed);

    /** Test the component using the official trace flags, input/output mapping, and test settings
        for conformance testing. Report on compliance. */
    OMX_ERRORTYPE OMX_CONF_ConformanceTestComponent(OMX_IN OMX_STRING sComponentName, OMX_BOOL *bPassed);

    /** Print the current list of all tests that will be applied to a component and the trace flags.*/
    OMX_ERRORTYPE OMX_CONF_PrintSettings();

    /** Print a help message explaining all commands */
    void OMX_CONF_PrintHelp();

    /* Parse input string into a command an arguments and execute that command with those arguments.*/
    OMX_ERRORTYPE OMX_CONF_ParseCommand(OMX_IN OMX_STRING sCommandAndArgs);

    /** Map an input file to a port. The test will feed the input data from the file to the port
     *  (if the test requires this). */
    OMX_ERRORTYPE OMX_CONF_MapInputfile(OMX_IN OMX_STRING sInputFileName, OMX_IN OMX_U32 nPortIndex);

    /***********************************************************************
     * INFILE MAP TABLE DEFINITION
     ***********************************************************************/

    typedef struct OMX_CONF_INFILEMAPPINGTYPE
    {
        char sInputFileName[512];
        OMX_U32 nPortIndex;
    } OMX_CONF_INFILEMAPPINGTYPE;

#define OMX_CONF_MAXINFILEMAPPINGS 100
    extern OMX_CONF_INFILEMAPPINGTYPE g_OMX_CONF_InFileMap[OMX_CONF_MAXINFILEMAPPINGS];
    extern OMX_U32 g_OMX_CONF_nInFileMappings;

    /***********************************************************************
     * OUTFILE MAP TABLE DEFINITION
     ***********************************************************************/

    typedef struct OMX_CONF_OUTFILEMAPPINGTYPE
    {
        char sOutputFileName[512];
        OMX_U32 nPortIndex;
    } OMX_CONF_OUTFILEMAPPINGTYPE;

#define OMX_CONF_MAXOUTFILEMAPPINGS 100
    extern OMX_CONF_OUTFILEMAPPINGTYPE g_OMX_CONF_OutFileMap[OMX_CONF_MAXOUTFILEMAPPINGS];
    extern OMX_U32 g_OMX_CONF_nOutFileMappings;

    /**********************************************************************
     * CONFORMANCE TEST SCRIPT
     *
     * A conformance test script consists of a series of commands
     * that equate to the execution of the corresponding conformance test
     * functions:
     *
     * cc <testname> : conformance test component (sets appropriate flags, adds all tests, tests component)
     * st <traceflags>: OMX_CONF_SetTraceflags(<traceflags>);
     * ol <logfilename>: OMX_OSAL_OpenLogfile(<logfilename>);
     * cl : OMX_OSAL_CloseLogfile();
     * at <testname>: OMX_CONF_AddTest(<testname>);
     * rt <testname>: OMX_CONF_RemoveTest(<testname>);
     * mi <inputfilename> <portindex> : OMX_CONF_MapInputfile(<inputfilename>,<portindex>);
     * tc <testname>: OMX_CONF_TestComponent(<testname>);
     * ps: OMX_CONF_PrintSettings();
     * h: OMX_CONF_PrintHelp();
     * lt: list all available tests.
     * lc: list all available components.
     *
     * The ';' starts a comment all characters after the ';' are ignored.
     * Lines consisting entirely of whitespace are ignored.
     *
     * EXAMPLE:
     *

    <Start-of-file>
    ; setup tracing, open logfile
    st 0x7 ; (OMX_OSAL_TRACE_PASSFAIL|OMX_OSAL_TRACE_CALLSEQUENCE|OMX_OSAL_TRACE_PARAMTERS)
    ol log1.txt

    ; add tests
    at StateTransitionTest
    at InvalidStateTest
    at MultiThreadTest

    ; test components
    tc OMX.CompanyXYZ.mp3.decode
    tc OMX.CompanyABC.audio.render

    ; close logfile, alter tracing settings, open new logfile
    cl
    st 0x3 ; (OMX_OSAL_TRACE_PASSFAIL|OMX_OSAL_TRACE_CALLSEQUENCE)
    ol log2.txt

    ; remove tests
    rt MultiThreadTest

    ; test components
    tc OMX.CompanyABC.mp3.decode

    ; close logfile
    cl
    <End-of-File>

     *
     **********************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
/* File EOF */
