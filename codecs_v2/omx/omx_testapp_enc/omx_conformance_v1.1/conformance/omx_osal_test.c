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

/** OMX_OSAL_Test.c
 *  Tests to verify a working implementation of OSAL functions.
 */

#include "OMX_OSAL_Interfaces.h"
#include "OMX_CONF_TestHarness.h"

#define OMX_OSAL_TESTERR( eError ) if( OMX_ErrorNone != eError ) {                            \
        OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "%s:%d Error %x\n", __FILE__, __LINE__, eError); \
        goto OMX_OSAL_TEST_BAIL;                                                              \
    }


/* ******************************************************************
 *  Memory test 1:
 * ******************************************************************
 *  Just basic memory tests 
 */
OMX_ERRORTYPE OMX_OSAL_MemoryTest1(OMX_IN OMX_STRING cComponentName)
{
    OMX_ERRORTYPE eError = OMX_ErrorNone;    
    OMX_U32 *pValues = 0;
    OMX_U8 * pBuffer = (OMX_U8 *)OMX_OSAL_Malloc( sizeof(OMX_U32) * 4);

    UNUSED_PARAMETER(cComponentName);

    if(!pBuffer) {
        eError = OMX_ErrorInsufficientResources;
    }

    /* Write into memory */
    pValues = (OMX_U32 *)pBuffer;
    pValues[0] = 0; pValues[1] = OMX_ALL; pValues[2] = 0; pValues[3] = OMX_ALL; 

    /* Read from memory */
    if( pValues[2] != 0 ) {
        OMX_OSAL_Free( pBuffer ); 
        return OMX_ErrorUndefined;
    }

    /* Free memory */
    OMX_OSAL_Free( pBuffer ); 

    return eError;
}


/* ******************************************************************
 *  Multi-thread test 1:
 * ******************************************************************
 *  Creates threads to increment a common variable. 
 *  At the end, nFilledIn should equal MULTITHREAD_TEST1_ARRAYSIZE 
 *  (can test w/ and w/o thread protection)
 */
#define MULTITHREAD_TEST1_LOOPSIZE   1024*10
#define MULTITHREAD_TEST1_NUMTHREADS 32
#define MULTITHREAD_TEST1_ARRAYSIZE  MULTITHREAD_TEST1_LOOPSIZE*MULTITHREAD_TEST1_NUMTHREADS

typedef struct SMultiThreadTest1Param 
{   /* Each thread has a handle to the mutex lock and it's event handle */
    OMX_U32 *       pFilledIn;
    OMX_HANDLETYPE  oMutex;
    OMX_HANDLETYPE  oEvent;
} MultiThreadTest1Param;

OMX_U32 MultiThreadTest1Func(OMX_PTR pParam)
{
    OMX_U32 nIndex = 0;
    MultiThreadTest1Param *pData = (MultiThreadTest1Param *)pParam;

    /* Increment number filled in */
    for(nIndex = 0; nIndex < MULTITHREAD_TEST1_LOOPSIZE; nIndex++) {
        OMX_OSAL_MutexLock( pData->oMutex );
        *(pData->pFilledIn) += 1;
        *(pData->pFilledIn) -= 1;
        *(pData->pFilledIn) *= 2;
        *(pData->pFilledIn) /= 2;
        *(pData->pFilledIn) += 1;
        *(pData->pFilledIn) -= 1;
        *(pData->pFilledIn) *= 2;
        *(pData->pFilledIn) /= 2;

        *(pData->pFilledIn) += 1;
        OMX_OSAL_MutexUnlock( pData->oMutex );
    }

    /* Set event to notify thread complete */
    OMX_OSAL_EventSet( pData->oEvent );
    return 0;
}

OMX_ERRORTYPE OMX_OSAL_MultiThreadTest1(OMX_IN OMX_STRING cComponentName)
{    
    OMX_HANDLETYPE          oMutex;
    OMX_HANDLETYPE          oEvent[MULTITHREAD_TEST1_NUMTHREADS]; 
    OMX_HANDLETYPE          oThread[MULTITHREAD_TEST1_NUMTHREADS];
    MultiThreadTest1Param   oThreadParams[MULTITHREAD_TEST1_NUMTHREADS];
    OMX_U32                 nIndex;
    OMX_U32                 nFilledIn; 
    OMX_ERRORTYPE           eError;    
    OMX_BOOL                bTimeout;

    UNUSED_PARAMETER(cComponentName);

    /* Initialize variables */
    eError              = OMX_ErrorNone;
    oMutex              = 0; 
    nFilledIn           = 0;     
    bTimeout            = OMX_FALSE;
    for(nIndex = 0; nIndex < MULTITHREAD_TEST1_NUMTHREADS; nIndex++) {
        oEvent[nIndex]  = 0; 
        oThread[nIndex] = 0; 
    }
    
    /* Create mutex and event instances */
    eError = OMX_OSAL_MutexCreate( &oMutex ); 
    OMX_OSAL_TESTERR( eError );
    for(nIndex = 0; nIndex < MULTITHREAD_TEST1_NUMTHREADS; nIndex++) {
        eError = OMX_OSAL_EventCreate( &oEvent[nIndex] ); 
        OMX_OSAL_TESTERR( eError );

        oThreadParams[nIndex].pFilledIn = &nFilledIn;
        oThreadParams[nIndex].oMutex    = oMutex;
        oThreadParams[nIndex].oEvent    = oEvent[nIndex];        
    }

    /* Launch threads */
    for(nIndex = 0; nIndex < MULTITHREAD_TEST1_NUMTHREADS; nIndex++) {
        eError = OMX_OSAL_ThreadCreate( MultiThreadTest1Func, 
            (OMX_PTR) &oThreadParams[nIndex],
            0, &oThread[nIndex] ); 
        OMX_OSAL_TESTERR( eError );
    }

    /* Wait for the threads to complete */
    for(nIndex = 0; nIndex < MULTITHREAD_TEST1_NUMTHREADS; nIndex++) {
        eError = OMX_OSAL_EventWait( oEvent[nIndex], INFINITE_WAIT, &bTimeout);
    }

    /* Check if values are correct */
    if( nFilledIn == MULTITHREAD_TEST1_ARRAYSIZE ) {
        /* Threading passes */
        nFilledIn = 1; 
    }
    else {
        /* Error */
        nFilledIn = 0; 

        /* TO DO: Set correct error */
        eError = OMX_ErrorUndefined;
    }

OMX_OSAL_TEST_BAIL:
    /* Free memory and return error value */    
    for(nIndex = 0; nIndex < MULTITHREAD_TEST1_NUMTHREADS; nIndex++) {
        if( oThread[nIndex] ) OMX_OSAL_ThreadDestroy( oThread[nIndex] ); 
        if( oEvent[nIndex] )  OMX_OSAL_EventDestroy( oEvent[nIndex] );         
    }
    if(oMutex) OMX_OSAL_MutexDestroy( oMutex );

    return eError; 
}

/* ******************************************************************
 *  Timer test 1:
 * ******************************************************************
 *  Basic timer test: 
 *  Fill in an array without using threads first. Should be fast.
 *  Next, fill in the array using threads. See how much extra time
 *  used in overhead/waiting.
 */
typedef struct STimerTest1Param 
{ 
    OMX_U8 *        pBuffer;
    OMX_U32         uStartIndex;
    OMX_U32         uEndIndex;
    OMX_HANDLETYPE  oEvent;
} TimerTest1Param;

#define TIMER_TEST1_NUMTHREADS 10
#define TIMER_TEST1_LOOPSIZE   1024*40
#define TIMER_TEST1_ARRAYSIZE  TIMER_TEST1_LOOPSIZE*TIMER_TEST1_NUMTHREADS

OMX_U32 TimerTest1Func(OMX_PTR pParam)
{
    OMX_U32 nIndex = 0;
    TimerTest1Param *pData = (TimerTest1Param *)pParam;

    /* Increment number filled in */
    for(nIndex = pData->uStartIndex; nIndex <= pData->uEndIndex; nIndex++) {
        pData->pBuffer[nIndex] = 2;
    }

    /* Set event to notify thread complete */
    OMX_OSAL_EventSet( pData->oEvent );
    return 0;
}

OMX_ERRORTYPE OMX_OSAL_TimerTest1(OMX_IN OMX_STRING cComponentName)
{
    OMX_ERRORTYPE   eError = OMX_ErrorNone;
    OMX_U32 nIndex  = 0;
    OMX_U32 uBegin  = 0;
    OMX_U32 uEnd    = 0;
    OMX_U8 *pBuffer = 0;

    OMX_HANDLETYPE     oEvent[TIMER_TEST1_NUMTHREADS]; 
    OMX_HANDLETYPE     oThread[TIMER_TEST1_NUMTHREADS];
    TimerTest1Param    oThreadParams[TIMER_TEST1_NUMTHREADS]; 
    OMX_BOOL           bTimeout = OMX_FALSE;

    UNUSED_PARAMETER(cComponentName);
    
    pBuffer = OMX_OSAL_Malloc( TIMER_TEST1_ARRAYSIZE * sizeof(OMX_U32) );
    if(!pBuffer) {
        return OMX_ErrorInsufficientResources;
    }

    /* Time how long it takes to fill in an array */
    uBegin = OMX_OSAL_GetTime();
    for( nIndex = 0; nIndex < TIMER_TEST1_ARRAYSIZE; nIndex++) {
        pBuffer[nIndex] = 1;
    }
    uEnd = OMX_OSAL_GetTime(); 
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Total time without threads: %d\n", uEnd - uBegin ); 

    for(nIndex = 0; nIndex < TIMER_TEST1_NUMTHREADS; nIndex++) {
        oEvent[nIndex]  = 0; 
        oThread[nIndex] = 0; 
    }
    for(nIndex = 0; nIndex < TIMER_TEST1_NUMTHREADS; nIndex++) {
        eError = OMX_OSAL_EventCreate( &oEvent[nIndex] ); 
        OMX_OSAL_TESTERR( eError );

        oThreadParams[nIndex].pBuffer       = pBuffer;
        oThreadParams[nIndex].oEvent        = oEvent[nIndex];  
        oThreadParams[nIndex].uStartIndex   = nIndex * TIMER_TEST1_LOOPSIZE;
        oThreadParams[nIndex].uEndIndex     = nIndex * TIMER_TEST1_LOOPSIZE 
                                                + TIMER_TEST1_LOOPSIZE -1;
    }

    uBegin = OMX_OSAL_GetTime();
    /* Launch threads */
    for(nIndex = 0; nIndex < TIMER_TEST1_NUMTHREADS; nIndex++) {
        eError = OMX_OSAL_ThreadCreate( TimerTest1Func, 
            (OMX_PTR) &oThreadParams[nIndex],
            0, &oThread[nIndex] ); 
        OMX_OSAL_TESTERR( eError );
    }

    /* Wait for the threads to complete */
    for(nIndex = 0; nIndex < TIMER_TEST1_NUMTHREADS; nIndex++) {
        eError = OMX_OSAL_EventWait( oEvent[nIndex], INFINITE_WAIT, &bTimeout);
    }
    uEnd = OMX_OSAL_GetTime(); 
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Total time with threads: %d\n", uEnd - uBegin ); 

    for( nIndex = 0; nIndex < TIMER_TEST1_ARRAYSIZE; nIndex++) {
        if( pBuffer[nIndex] != 2 ) {
            eError = OMX_ErrorUndefined;
            break;
        }
    }

OMX_OSAL_TEST_BAIL:
    /* Free memory and return error value */    
    for(nIndex = 0; nIndex < TIMER_TEST1_NUMTHREADS; nIndex++) {
        if( oThread[nIndex] ) OMX_OSAL_ThreadDestroy( oThread[nIndex] ); 
        if( oEvent[nIndex] )  OMX_OSAL_EventDestroy( oEvent[nIndex] );         
    }
    if(pBuffer) OMX_OSAL_Free(pBuffer);
    return eError;
}


#define OMX_CONF_DOTEST(_T_) eError = _T_(cComponentName);          \
    if (eError != OMX_ErrorNone) {                                  \
        nErrors++;                                                  \
        OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "%s:%d Error %x\n",    \
                       __FILE__, __LINE__, eError);                 \
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PASSFAIL, #_T_ " FAILED\n");  \
    }                                                               \
    else                                                            \
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PASSFAIL, #_T_ " PASSED\n");

OMX_ERRORTYPE OMX_OSAL_TestAll(OMX_IN OMX_STRING cComponentName)
{
    int nErrors = 0;
    OMX_ERRORTYPE eError;

    UNUSED_PARAMETER(cComponentName);

    OMX_CONF_DOTEST(OMX_OSAL_MemoryTest1);
    OMX_CONF_DOTEST(OMX_OSAL_MultiThreadTest1);
    OMX_CONF_DOTEST(OMX_OSAL_TimerTest1);
    if (nErrors > 0)
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PASSFAIL, "OMX_OSAL_TestAll FAILED\n");
    return (nErrors > 0) ? OMX_ErrorUndefined : OMX_ErrorNone;
}


/* File EOF */
