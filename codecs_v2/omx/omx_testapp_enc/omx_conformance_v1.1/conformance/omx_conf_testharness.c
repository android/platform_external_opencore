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

/** OMX_CONF_TestHarness.c
 *  Implemenation of the OpenMax IL conformance test harness that accepts a list of 
 *  commands (from a prompt or from file) to set tracing/logfiles, add/remove tests 
 *  from the list applied to components, and ultimately launch testing against components).
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "OMX_OSAL_Interfaces.h"
#include "OMX_CONF_TestHarness.h"
#include "OMX_CONF_StubbedCallbacks.h"
#include "OMX_CONF_TunnelTestComponent.h"

#include <string.h>
#include <stdlib.h>

/***********************************************************************
 * GLOBAL VARIABLES
 ***********************************************************************/
OMX_VERSIONTYPE g_OMX_CONF_Version;

/***********************************************************************
 * INFILE MAP TABLE IMPLEMENTATION
 ***********************************************************************/

#define OMX_CONF_MAXINFILEMAPPINGS 100
OMX_CONF_INFILEMAPPINGTYPE g_OMX_CONF_InFileMap[OMX_CONF_MAXINFILEMAPPINGS];
OMX_U32 g_OMX_CONF_nInFileMappings;

/***********************************************************************
 * INFILE MAP TABLE IMPLEMENTATION
 ***********************************************************************/

#define OMX_CONF_MAXOUTFILEMAPPINGS 100
OMX_CONF_OUTFILEMAPPINGTYPE g_OMX_CONF_OutFileMap[OMX_CONF_MAXOUTFILEMAPPINGS];
OMX_U32 g_OMX_CONF_nOutFileMappings;

/**********************************************************************
 * TEST LOOKUP TABLE
 **********************************************************************/

/* global state storing list of test indices */
OMX_U32 g_OMX_CONF_TestIndexList[OMX_CONF_MAXTESTNUMBER];
OMX_U32 g_OMX_CONF_nTests;
extern OMX_U32 g_OMX_OSAL_TraceFlags;

OMX_ERRORTYPE OMX_CONF_AddTest( OMX_IN OMX_STRING sTestName )
{
    OMX_U32 i,j;

    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\nOMX_CONF_AddTest %s\n\n", sTestName);

    // add all tests?
    if ('*' == sTestName[0])
    {
        g_OMX_CONF_nTests = 0;

        for (i=0;i<g_OMX_CONF_nTestLookupTableEntries;i++)
        {
            /* add index to list */
            g_OMX_CONF_TestIndexList[g_OMX_CONF_nTests++] = i;
        }
        return OMX_ErrorNone;
    }

    /* search for test in lookup table */
    for (i=0;i<g_OMX_CONF_nTestLookupTableEntries;i++)
    {
        /* found test */
        if (!strcmp(g_OMX_CONF_TestLookupTable[i].pName,sTestName))
        {
            /* check test isn't already in list */
            for (j=0;j<g_OMX_CONF_nTests;j++)
            {
                /* duplicate test */
                if (g_OMX_CONF_TestIndexList[j] == i) 
                {
                    OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR,"OMX_CONF_AddTest failed. Duplicate test.\n");
                    return OMX_ErrorBadParameter;
                }
            }
            /* add index to list */
            g_OMX_CONF_TestIndexList[g_OMX_CONF_nTests++] = i;
            return OMX_ErrorNone;
        }
    }

    /* invalid test */
    OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR,"OMX_CONF_AddTest failed. Invalid test name.\n");
    return OMX_ErrorBadParameter;
}

OMX_ERRORTYPE OMX_CONF_RemoveTest( OMX_IN OMX_STRING sTestName )
{
    OMX_U32 i,j;

    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\nOMX_CONF_RemoveTest %s\n\n", sTestName);

    // remove all tests?
    if ('*' == sTestName[0])
    {
        g_OMX_CONF_nTests = 0;
        return OMX_ErrorNone;
    }

    /* search for test in lookup table */
    for (i=0;i<g_OMX_CONF_nTestLookupTableEntries;i++)
    {
        /* found test */
        if (!strcmp(g_OMX_CONF_TestLookupTable[i].pName,sTestName))
        {
            /* search for test in list */
            for (j=0;j<g_OMX_CONF_nTests;j++)
            {
                /* found test */
                if (g_OMX_CONF_TestIndexList[j] == i) 
                {
                    /* remove index from list */
                    g_OMX_CONF_nTests -= 1;
                    for (;j<g_OMX_CONF_nTests;j++){
                        g_OMX_CONF_TestIndexList[j] = g_OMX_CONF_TestIndexList[j+1];
                    }
                    return OMX_ErrorNone;
                }
            }
            /* test not in list */
            OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR,"OMX_CONF_RemoveTest. Test not found.\n");
            return OMX_ErrorBadParameter;
       }
    }

    /* invalid test */
    OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR,"OMX_CONF_RemoveTest. Invalid test name.\n");
    return OMX_ErrorBadParameter;
}

OMX_BOOL OMX_CONF_ComponentExists(OMX_IN OMX_STRING cComponentName)
{
    OMX_ERRORTYPE  eError = OMX_ErrorNone;
    OMX_U32 i = 0;
    OMX_S8 cCompEnumName[256];
    
    /* Initialize OpenMax */
    if (OMX_ErrorNone == (eError = OMX_Init()))
    {
        while (OMX_ErrorNone == eError) 
        {
            /* loop through all enumerated components to determine if the component name
            specificed for the test is enumerated by the OMX core */
            eError = OMX_ComponentNameEnum((OMX_STRING) cCompEnumName, 256, i);
            if (OMX_ErrorNone == eError){
                if (!strcmp((OMX_STRING) cCompEnumName, cComponentName)) {
                    /* found it */    
                    OMX_Deinit();
                    return OMX_TRUE;
                }
            } 
            i++;
        }
    }

    /* didn't find it */
    OMX_Deinit();
    return OMX_FALSE;
}

OMX_ERRORTYPE OMX_CONF_ListComponents()
{
    OMX_ERRORTYPE  eError = OMX_ErrorNone;
    OMX_U32 i = 0;
    OMX_S8 cCompEnumName[256];
 
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\nAvailable Components:\n\n"); 

    /* Initialize OpenMax */
    if (OMX_ErrorNone == (eError = OMX_Init()))
    {
        while (OMX_ErrorNone == eError) 
        {
            /* loop through all enumerated components to determine if the component name
            specificed for the test is enumerated by the OMX core */
            eError = OMX_ComponentNameEnum((OMX_STRING) cCompEnumName, 256, i);
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\t%s\n", cCompEnumName);            
            i++;
        }
    }

    /* didn't find it */
    OMX_Deinit();
    return OMX_ErrorNone;
}

OMX_ERRORTYPE OMX_CONF_TestComponent( OMX_IN OMX_STRING sComponentName, OMX_BOOL *bPassed)
{
    OMX_U32 i;
    OMX_ERRORTYPE eError;
    char szDesc[256]; 
    OMX_U32 nPassedTests, nFailedTests;
    OMX_U32 testId;

    if (!OMX_CONF_ComponentExists(sComponentName)){
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PASSFAIL, "Cannot find component %s, all tests FAILED\n", sComponentName);
        return OMX_ErrorUndefined;
    }

    /* initialize passed and failed test counters */
    nPassedTests = 0;
    nFailedTests = 0;

    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\nOMX_CONF_TestComponent %s\n\n", sComponentName);

    /* Run each test in current list on component */
    for (i=0;i<g_OMX_CONF_nTests;i++)
    {
        /* get test id */
        testId = g_OMX_CONF_TestIndexList[i];

        /* emit test header */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "##\n");
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "## %s \n", g_OMX_CONF_TestLookupTable[testId].pName );
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "##\n");

        /* perform test */
        eError = g_OMX_CONF_TestLookupTable[testId].pFunc(sComponentName);

        /* emit test result */
        if( OMX_ErrorNone != eError ) {
            OMX_CONF_ErrorToString( eError, szDesc );
            OMX_OSAL_Trace(OMX_OSAL_TRACE_PASSFAIL, " %s %s FAILED, %x %s\n",
                g_OMX_CONF_TestLookupTable[testId].pName, sComponentName, eError, szDesc);
            bPassed[i] = OMX_FALSE;
            nFailedTests++;
        } else {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_PASSFAIL, " %s %s PASSED\n",
                g_OMX_CONF_TestLookupTable[testId].pName, sComponentName);
            bPassed[i] = OMX_TRUE;
            nPassedTests++;
        }
    }


    OMX_OSAL_Trace(OMX_OSAL_TRACE_PASSFAIL, "\n");
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PASSFAIL, " \n");
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PASSFAIL, " Summary for %s\n", sComponentName);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PASSFAIL, " \n");

    /* Report passed tests */
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PASSFAIL, " %i tests passed:\n", nPassedTests);
    for (i=0;i<g_OMX_CONF_nTests;i++)
    {
        /* if passed emit test name */
        if (bPassed[i]){
            testId = g_OMX_CONF_TestIndexList[i];
            OMX_OSAL_Trace(OMX_OSAL_TRACE_PASSFAIL, " \t%s\n", g_OMX_CONF_TestLookupTable[testId].pName);
        }
    }

    /* Report failed tests */
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PASSFAIL, " \n");
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PASSFAIL, " %i tests failed: \n", nFailedTests);
    for (i=0;i<g_OMX_CONF_nTests;i++)
    {
        /* if failed emit test name */
        if (!bPassed[i]){
            testId = g_OMX_CONF_TestIndexList[i];
            OMX_OSAL_Trace(OMX_OSAL_TRACE_PASSFAIL, " \t%s\n", g_OMX_CONF_TestLookupTable[testId].pName);
        }
    }
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PASSFAIL, " \n");

    return OMX_ErrorNone;
}

OMX_ERRORTYPE OMX_CONF_PrintSettings()
{
    OMX_U32 i;

    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\nOMX_CONF_PrintSettings\n");
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Trace Flags = 0x%08x\n", g_OMX_OSAL_TraceFlags);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Active Tests:\n");
    for (i=0;i<g_OMX_CONF_nTests;i++){
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\t%s\n", 
            g_OMX_CONF_TestLookupTable[g_OMX_CONF_TestIndexList[i]].pName);
    }
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Input File Mappings:\n");
    for (i=0;i<g_OMX_CONF_nInFileMappings;i++){
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\t%i:%s\n", 
            g_OMX_CONF_InFileMap[i].nPortIndex, g_OMX_CONF_InFileMap[i].sInputFileName);
    }
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Output File Mappings:\n");
    for (i=0;i<g_OMX_CONF_nOutFileMappings;i++){
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\t%i:%s\n", 
            g_OMX_CONF_OutFileMap[i].nPortIndex, g_OMX_CONF_OutFileMap[i].sOutputFileName);
    }
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\n");

    return OMX_ErrorNone;
}

void OMX_CONF_PrintCcUsage()
{
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\tcc <testname> : conformance test component given the \n");
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\t\ttrace flags and tests appropriate for compliance\n"); 
}

void OMX_CONF_PrintStUsage()
{
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\tst <traceflags>: set trace flags to given value.\n");
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\t <traceflags> = bitwise ORing of these flags:\n");
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\t\t0x0001 = Pass/fail.\n");
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\t\t0x0002 = Call sequence.\n");
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\t\t0x0004 = Parameters.\n");
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\t\t0x0008 = Info.\n");
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\t\t0x0010 = Error.\n");
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\t\t0x0020 = Buffer.\n");
}

void OMX_CONF_PrintOlUsage()
{
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\tol <logfilename>: open given log file.\n");
}

void OMX_CONF_PrintAtUsage()
{
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\tat <testname>: add test (""*"" indicates all tests)\n");
}

void OMX_CONF_PrintRtUsage()
{
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\trt <testname>: remove given test (""*"" indicates all tests).\n");
}

void OMX_CONF_PrintTcUsage()
{
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\ttc <testname>: test given component.\n");
}

void OMX_CONF_PrintMiUsage()
{
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\tmi <inputfilename> <portindex> : map input file to port.\n");
}

void OMX_CONF_PrintMoUsage()
{
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\tmo <outputfilename> <portindex> : map output file to port.\n");
}

void OMX_CONF_PrintHelp()
{
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\nOMX_CONF_PrintHelp()\n");

    OMX_CONF_PrintCcUsage();
    OMX_CONF_PrintStUsage();
    OMX_CONF_PrintOlUsage();
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\tcl : close log file.\n");
    OMX_CONF_PrintAtUsage();
    OMX_CONF_PrintRtUsage();
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\tlt : list all available tests\n");
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\tlc : list all available components\n");
    OMX_CONF_PrintTcUsage();
    OMX_CONF_PrintMiUsage();
    OMX_CONF_PrintMoUsage();
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\tps: print settings.\n");
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\th: help.\n");
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\tq: quit.\n\n");
}

OMX_ERRORTYPE OMX_CONF_ListTests()
{
    OMX_U32 i;
   
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\nAvailable Tests:\n\n"); 
    for (i=0;i<g_OMX_CONF_nTestLookupTableEntries;i++)
    {
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\t%s\n", 
            g_OMX_CONF_TestLookupTable[i].pName);
    }
    return OMX_ErrorNone;
}

OMX_ERRORTYPE OMX_CONF_MapInputfile( OMX_IN OMX_STRING sInputFileName, OMX_IN OMX_U32 nPortIndex )
{
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\nOMX_CONF_MapInputFile \"%s\" %d\n", sInputFileName, nPortIndex); 
    if (g_OMX_CONF_nInFileMappings >= OMX_CONF_MAXINFILEMAPPINGS) return OMX_ErrorInsufficientResources;

    strcpy( g_OMX_CONF_InFileMap[g_OMX_CONF_nInFileMappings].sInputFileName, sInputFileName);
    g_OMX_CONF_InFileMap[g_OMX_CONF_nInFileMappings].nPortIndex = nPortIndex;
    g_OMX_CONF_nInFileMappings++;
    return OMX_ErrorNone;
}

OMX_ERRORTYPE OMX_CONF_MapOutputfile( OMX_IN OMX_STRING sOutputFileName, OMX_IN OMX_U32 nPortIndex )
{
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\nOMX_CONF_MapOutputFile \"%s\" %d\n", sOutputFileName, nPortIndex); 
    if (g_OMX_CONF_nOutFileMappings >= OMX_CONF_MAXOUTFILEMAPPINGS) return OMX_ErrorInsufficientResources;

    strcpy( g_OMX_CONF_OutFileMap[g_OMX_CONF_nOutFileMappings].sOutputFileName, sOutputFileName);
    g_OMX_CONF_OutFileMap[g_OMX_CONF_nOutFileMappings].nPortIndex = nPortIndex;
    g_OMX_CONF_nOutFileMappings++;
    return OMX_ErrorNone;
}

OMX_U32 DummyThreadFunction(OMX_PTR pParam)
{
    UNUSED_PARAMETER(pParam);

    return 0;
}

/* detect the compliance profile(s) that the component is eligible for */
OMX_U32 DetectComplianceProfile(OMX_STRING sArgument)
{
    OMX_U32 detectedCompliance;
    OMX_HANDLETYPE hThread, hComp, hTTC;
    OMX_CALLBACKTYPE oDummyCallbacks;
    OMX_TIME_CONFIG_TIMESTAMPTYPE oTimeStamp;
    OMX_TIME_CONFIG_ACTIVEREFCLOCKTYPE oRefClock;

    detectedCompliance = OMX_CONF_TestFlag_Base|OMX_CONF_TestFlag_AutoOutput;

    /* try to create a thread -> implies threading compliance */
    if (OMX_ErrorNone == OMX_OSAL_ThreadCreate( DummyThreadFunction, 0, 0, &hThread)){
        if (OMX_ErrorNone == OMX_OSAL_ThreadDestroy( hThread)){
            detectedCompliance |= OMX_CONF_TestFlag_Threaded;
        }
    }
    oDummyCallbacks.EmptyBufferDone = StubbedEmptyBufferDone;
    oDummyCallbacks.FillBufferDone = StubbedFillBufferDone;
    oDummyCallbacks.EventHandler = StubbedEventHandler;
    if (OMX_ErrorNone != OMX_Init()){
        return 0;
    }
    if (OMX_ErrorNone != OMX_GetHandle(&hComp, sArgument, NULL, &oDummyCallbacks)){
        OMX_Deinit();
        return 0;
    }
    if (OMX_ErrorNone != OMX_CONF_GetTunnelTestComponentHandle(&hTTC, NULL, &oDummyCallbacks)){
        OMX_FreeHandle(hComp);
        OMX_Deinit();
        return 0;
    }

    if (OMX_ErrorNone == OMX_CONF_TTCConnectAllPorts(hTTC, hComp))
    {
        INIT_PARAM(oRefClock);
        INIT_PARAM(oTimeStamp);
        oTimeStamp.nPortIndex = 0;

        detectedCompliance |= OMX_CONF_TestFlag_Interop;

        /* try to set active ref clock -> implies clock component compliance */
        if (OMX_ErrorUnsupportedIndex != OMX_SetConfig(hComp, OMX_IndexConfigTimeActiveRefClock, &oRefClock)){
            detectedCompliance &= ~OMX_CONF_TestFlag_AutoOutput; /* only clock component doesn't have autooutput */
            detectedCompliance |=  OMX_CONF_TestFlag_ClockComp; 
        } 

        /* try to set time position -> implies seeking compliance */
        if (OMX_ErrorUnsupportedIndex != OMX_SetConfig(hComp, OMX_IndexConfigTimePosition, &oTimeStamp)){
            detectedCompliance |=  OMX_CONF_TestFlag_Seeking; 
        }
    }

    OMX_CONF_FreeTunnelTestComponentHandle(hTTC);
    OMX_FreeHandle(hComp);
    OMX_Deinit();

    return detectedCompliance;
}

static const char * OMX_CONF_ConformanceResult(OMX_U32 flags, OMX_U32 detectedCompliance, OMX_U32 actualCompliance)
{
    if ((flags & detectedCompliance) != flags)
        return "NOT RUN";
    if ((flags & actualCompliance) != flags)
        return "FAILED";
    else
        return "PASSED";
}

OMX_ERRORTYPE OMX_CONF_ConformancetestComponent(OMX_STRING sArgument, OMX_BOOL *bPassed)
{
    OMX_U32 i, nTestFlags[OMX_CONF_MAXTESTNUMBER], nTests;
    OMX_U32 detectedCompliance, actualCompliance;

    if (!OMX_CONF_ComponentExists(sArgument)){
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PASSFAIL, "Cannot find component %s, all tests FAILED\n", sArgument);
        return OMX_ErrorUndefined;
    }

    detectedCompliance = DetectComplianceProfile(sArgument);

    /* only add tests in that compliance profile */
    nTests = 0;
    for(i=0;i<g_OMX_CONF_nTestLookupTableEntries;i++)
    {
        OMX_U32 nFlag = g_OMX_CONF_TestLookupTable[i].nFlags;
        if (nFlag == 0)
            nFlag = OMX_CONF_TestFlag_Base;
        if ((nFlag & detectedCompliance)==nFlag)
        {         
            nTestFlags[nTests++] = nFlag;
            OMX_CONF_AddTest(g_OMX_CONF_TestLookupTable[i].pName);
        }
    }
        
    /* run the tests */
    OMX_CONF_SetTraceflags(OMX_OSAL_TRACE_PASSFAIL|OMX_OSAL_TRACE_WARNING|OMX_OSAL_TRACE_INFO);
    OMX_CONF_TestComponent(sArgument, bPassed);
    
    /* determine actual compliance */
    actualCompliance = detectedCompliance;  /* assume best case */
    for(i=0;i<nTests;i++)
    {
        /* if the test failed invalidate appropriate profiles */
        if (OMX_FALSE == bPassed[i]){
            if (0==(OMX_CONF_TestFlag_Interop & nTestFlags[i])){
                /* not an interop test */
                if (OMX_CONF_TestFlag_Threaded & nTestFlags[i]){
                    /* failed a threaded test - fail threaded compliance */
                    actualCompliance &= ~OMX_CONF_TestFlag_Threaded;                             
                } else {
                    /* failed a non-thread test - fail base comformance */
                    actualCompliance &=  ~OMX_CONF_TestFlag_Base;
                }
            } else if ( OMX_CONF_TestFlag_Interop == nTestFlags[i]
                        || (OMX_CONF_TestFlag_Interop|OMX_CONF_TestFlag_AutoOutput) == nTestFlags[i] ) {
                /* failed a interop test without clock/threaded/seeking - fail interop */
                actualCompliance &= ~OMX_CONF_TestFlag_Interop;     
            } else {
                /* failed something above and beyond interop - fail that flag */
                actualCompliance &= ~(~OMX_CONF_TestFlag_Interop & nTestFlags[i]);
            }
        }
    }

    /* report on actual compliance */
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PASSFAIL, " Base Profile:                   %s\n", 
        OMX_CONF_ConformanceResult(OMX_CONF_TestFlag_Base, detectedCompliance, actualCompliance));

    OMX_OSAL_Trace(OMX_OSAL_TRACE_PASSFAIL, " Base Profile with threading:    %s\n", 
        OMX_CONF_ConformanceResult(OMX_CONF_TestFlag_Base|OMX_CONF_TestFlag_Threaded, detectedCompliance, actualCompliance));

    OMX_OSAL_Trace(OMX_OSAL_TRACE_PASSFAIL, " Interop Profile:                %s\n", 
        OMX_CONF_ConformanceResult(OMX_CONF_TestFlag_Interop, detectedCompliance, actualCompliance));

    OMX_OSAL_Trace(OMX_OSAL_TRACE_PASSFAIL, " Interop Profile with threading: %s\n", 
         OMX_CONF_ConformanceResult(OMX_CONF_TestFlag_Interop|OMX_CONF_TestFlag_Threaded, detectedCompliance, actualCompliance));
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PASSFAIL, " Clock Component:                %s\n", 
         OMX_CONF_ConformanceResult(OMX_CONF_TestFlag_Interop|OMX_CONF_TestFlag_ClockComp, detectedCompliance, actualCompliance));
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PASSFAIL, " Seeking Component:              %s\n", 
         OMX_CONF_ConformanceResult(OMX_CONF_TestFlag_Interop|OMX_CONF_TestFlag_Seeking, detectedCompliance, actualCompliance));

    return OMX_ErrorNone;
}

OMX_ERRORTYPE OMX_CONF_ParseCommand( OMX_IN OMX_STRING sCommandAndArgs )
{
    char sLocalCopy[512];
    char sCommand[5], *sArgument, *sArgument2;
    char *pC;
    OMX_BOOL bPassed[OMX_CONF_MAXTESTNUMBER];

    if (('h' == sCommandAndArgs[0]) || ('H' == sCommandAndArgs[0]))
    {
        OMX_CONF_PrintHelp();
        return OMX_ErrorNone;
    } 

    // make a local copy to ensure we're non-destructive
    strcpy(sLocalCopy, sCommandAndArgs);

    // extract command
    for(pC=sLocalCopy;(*pC == ' ')||(*pC == '\t');pC++);     // strip spaces before command
    if (strlen(pC) < 2) return OMX_ErrorNone;                         // ensure at least 2 chars
    if ((pC[0] >= 'A') && (pC[0] <= 'Z')) 
        sCommand[0] = (char) (pC[0]-'A'+'a');
    else sCommand[0] = pC[0]; // lower case (1st char)
    if ((pC[1] >= 'A') && (pC[1] <= 'Z')) 
        sCommand[1] = (char) (pC[1]-'A'+'a');
    else sCommand[1] = pC[1]; // lower case (2nd char)
    sCommand[2] = '\0';
    pC +=2;

    // extract argument
    for(;(*pC == ' ')||(*pC == '\t');pC++);     // strip spaces before argument
    sArgument = pC;
    for(;(*pC != ' ')&&(*pC != '\t')&&(*pC != '\0');pC++);     // null terminate argument
    *pC = '\0';
    pC++;

    if (!strcmp("cc", sCommand))
    {
        if (sArgument[0] == '\0'){
           OMX_CONF_PrintCcUsage();     
        } else{
          OMX_CONF_ConformancetestComponent(sArgument, bPassed);
        }
    }
    if (!strcmp("st", sCommand))
    {
        if (sArgument[0] == '\0'){
           OMX_CONF_PrintStUsage();     
        } else {
           OMX_CONF_SetTraceflags(strtol(sArgument,NULL,0));
        }
    }
    else if (!strcmp("ol", sCommand))
    {
        if (sArgument[0] == '\0'){
           OMX_CONF_PrintOlUsage();     
        } else {
            OMX_OSAL_OpenLogfile(sArgument);
        }
    }
    else if (!strcmp("cl", sCommand)){
        OMX_OSAL_CloseLogfile();
    } 
    else if (!strcmp("at", sCommand))
    {
        if (sArgument[0] == '\0'){
           OMX_CONF_PrintAtUsage();     
        } else {
            OMX_CONF_AddTest(sArgument);
        }
    } 
    else if (!strcmp("rt", sCommand))
    {
        if (sArgument[0] == '\0'){
           OMX_CONF_PrintRtUsage();     
        } else {
            OMX_CONF_RemoveTest(sArgument);
        }
    }
    else if (!strcmp("lt", sCommand))
    {
        OMX_CONF_ListTests();
    } 
    else if (!strcmp("lc", sCommand))
    {
        OMX_CONF_ListComponents();
    }
    else if (!strcmp("tc", sCommand))
    {
        if (sArgument[0] == '\0'){
           OMX_CONF_PrintTcUsage();     
        } else {
            OMX_CONF_TestComponent(sArgument, bPassed);
        }
    } 
    else if (!strcmp("ps", sCommand))
    {
        OMX_CONF_PrintSettings();
    } 
    else if (!strcmp("mi", sCommand))
    {
        // extract second argument
        for(;(*pC == ' ')||(*pC == '\t');pC++);     // strip spaces before argument
        sArgument2 = pC;
        for(;(*pC != ' ')&&(*pC != '\t')&&(*pC != '\0');pC++);     // null terminate argument
        *pC = '\0';

        if ((sArgument[0] == '\0')||(sArgument2[0] == '\0')){
           OMX_CONF_PrintMiUsage();     
        } else {
            OMX_CONF_MapInputfile(sArgument, strtol(sArgument2,NULL,0));
        }
    }
    else if (!strcmp("mo", sCommand))
    {
        // extract second argument
        for(;(*pC == ' ')||(*pC == '\t');pC++);     // strip spaces before argument
        sArgument2 = pC;
        for(;(*pC != ' ')&&(*pC != '\t')&&(*pC != '\0');pC++);     // null terminate argument
        *pC = '\0';

        if ((sArgument[0] == '\0')||(sArgument2[0] == '\0')){
           OMX_CONF_PrintMoUsage();     
        } else {
            OMX_CONF_MapOutputfile(sArgument, strtol(sArgument2,NULL,0));
        }
    }

    return OMX_ErrorNone;
}

/* For each domain: force all the component's ports to be suppliers/non-suppliers */
OMX_ERRORTYPE ForceSuppliersPerDomain(OMX_INDEXTYPE nIndex, OMX_HANDLETYPE hComp, OMX_BOOL bSupplier)
{
    OMX_PORT_PARAM_TYPE oParam;
    OMX_U32 i;
    OMX_U32 iPort;
    OMX_ERRORTYPE eError;
    OMX_PARAM_BUFFERSUPPLIERTYPE oSupplier;
    OMX_PARAM_PORTDEFINITIONTYPE oPortDef;
 
    INIT_PARAM(oSupplier);
    INIT_PARAM(oParam);
    INIT_PARAM(oPortDef);
    oSupplier.eBufferSupplier = bSupplier ? OMX_BufferSupplyInput : OMX_BufferSupplyOutput;

	if (OMX_ErrorNone != (eError = OMX_GetParameter(hComp, nIndex, &oParam))) return eError;
    for (i=0;i<oParam.nPorts;i++)
    {
        iPort = oParam.nStartPortNumber + i;
        oPortDef.nPortIndex = iPort;
        OMX_GetParameter(hComp, OMX_IndexParamPortDefinition, &oPortDef);
        
        /* only set inputs */
        if (oPortDef.eDir == OMX_DirInput){
            oSupplier.nPortIndex = iPort;
            OMX_SetParameter(hComp, OMX_IndexParamCompBufferSupplier, &oSupplier);
        }
    }

	return OMX_ErrorNone;
}

/* Force all the component's ports to be suppliers/non-suppliers */
OMX_ERRORTYPE OMX_CONF_ForceSuppliers(OMX_HANDLETYPE hComp, OMX_BOOL bSupplier)
{
	ForceSuppliersPerDomain(OMX_IndexParamAudioInit, hComp, bSupplier);
	ForceSuppliersPerDomain(OMX_IndexParamVideoInit, hComp, bSupplier);
	ForceSuppliersPerDomain(OMX_IndexParamImageInit, hComp, bSupplier);
	ForceSuppliersPerDomain(OMX_IndexParamOtherInit, hComp, bSupplier);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE AreAllPortsSameDirection(OMX_HANDLETYPE hComp, OMX_BOOL *pbAllSame, OMX_DIRTYPE eDir,
                                                OMX_INDEXTYPE eIndexParamDomainInit)
{
    OMX_PORT_PARAM_TYPE oParam;
    OMX_U32 i;
    OMX_PARAM_PORTDEFINITIONTYPE oPortDef;
    OMX_ERRORTYPE eError;

    *pbAllSame = OMX_TRUE;
    INIT_PARAM(oPortDef);

    /* query the component's ports */
    INIT_PARAM(oParam);
    if (OMX_ErrorNone != (eError = OMX_GetParameter(hComp, eIndexParamDomainInit, &oParam))) return eError;
    oPortDef.nPortIndex = oParam.nStartPortNumber;

    /* for each discovered port */
    for (i=0;i<oParam.nPorts;i++)
    {        
        /* check direction of port */
        if (OMX_ErrorNone != (eError = OMX_GetParameter(hComp, OMX_IndexParamPortDefinition, &oPortDef)) ||
            (oPortDef.eDir != eDir))
        { 
            *pbAllSame = OMX_FALSE;
            return eError;
        }
        oPortDef.nPortIndex++;
    }
    return eError;
}

OMX_ERRORTYPE OMX_CONF_IsSink(OMX_HANDLETYPE hComp, OMX_BOOL *pbIsSink)
{
    OMX_ERRORTYPE eError;

    if (OMX_ErrorNone != (eError = AreAllPortsSameDirection(
        hComp, pbIsSink, OMX_DirInput, OMX_IndexParamAudioInit)) || (OMX_FALSE == *pbIsSink)){
        return eError;
    }
    if (OMX_ErrorNone != (eError = AreAllPortsSameDirection(
        hComp, pbIsSink, OMX_DirInput, OMX_IndexParamVideoInit)) || (OMX_FALSE == *pbIsSink)){
        return eError;
    }
    if (OMX_ErrorNone != (eError = AreAllPortsSameDirection(
        hComp, pbIsSink, OMX_DirInput, OMX_IndexParamImageInit)) || (OMX_FALSE == *pbIsSink)){
        return eError;
    }
    if (OMX_ErrorNone != (eError = AreAllPortsSameDirection(
        hComp, pbIsSink, OMX_DirInput, OMX_IndexParamOtherInit)) || (OMX_FALSE == *pbIsSink)){
        return eError;
    }

    return eError;
}

OMX_ERRORTYPE OMX_CONF_IsSource(OMX_HANDLETYPE hComp, OMX_BOOL *pbIsSource)
{
    OMX_ERRORTYPE eError;

    if (OMX_ErrorNone != (eError = AreAllPortsSameDirection(
        hComp, pbIsSource, OMX_DirOutput, OMX_IndexParamAudioInit)) || (OMX_FALSE == *pbIsSource)){
        return eError;
    }
    if (OMX_ErrorNone != (eError = AreAllPortsSameDirection(
        hComp, pbIsSource, OMX_DirOutput, OMX_IndexParamVideoInit)) || (OMX_FALSE == *pbIsSource)){
        return eError;
    }
    if (OMX_ErrorNone != (eError = AreAllPortsSameDirection(
        hComp, pbIsSource, OMX_DirOutput, OMX_IndexParamImageInit)) || (OMX_FALSE == *pbIsSource)){
        return eError;
    }
    if (OMX_ErrorNone != (eError = AreAllPortsSameDirection(
        hComp, pbIsSource, OMX_DirOutput, OMX_IndexParamOtherInit)) || (OMX_FALSE == *pbIsSource)){
        return eError;
    }

    return eError;
}

int main(int argc, char **argv)
{
    g_OMX_CONF_nTests = 0;
    g_OMX_CONF_nInFileMappings = 0;
    g_OMX_CONF_nOutFileMappings = 0;
    g_OMX_CONF_hTraceMutex = 0;

    OMX_OSAL_MutexCreate(&g_OMX_CONF_hTraceMutex);

    /* Setup common version structure */
    g_OMX_CONF_Version.s.nVersionMajor = 1;
    g_OMX_CONF_Version.s.nVersionMinor = 0;
    g_OMX_CONF_Version.s.nRevision = 0;
    g_OMX_CONF_Version.s.nStep = 0;    
    g_OMX_CONF_Version.nVersion = 0x00000001;

    /* set default error level, so that you can detect errors in script startup! */
    OMX_CONF_SetTraceflags(OMX_OSAL_TRACE_INFO|OMX_OSAL_TRACE_ERROR|OMX_OSAL_TRACE_PASSFAIL|OMX_OSAL_TRACE_WARNING);
    if (2==argc){
        OMX_OSAL_ProcessCommandsFromFile(argv[1]);
    } else if (1==argc){
        OMX_OSAL_ProcessCommandsFromPrompt();
    }

    OMX_OSAL_MutexDestroy(g_OMX_CONF_hTraceMutex);

    return 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */
