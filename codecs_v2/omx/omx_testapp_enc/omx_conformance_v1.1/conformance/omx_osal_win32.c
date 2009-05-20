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

/** OMX_OSAL_Win32.c
 *  Win32 implemenation of the OpenMax IL OS abstraction layer including memory, threading,
 *  mutex, event, time query and tracing functionality.
 */

#include "OMX_OSAL_Interfaces.h"
#include "OMX_CONF_TestHarness.h"

#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern OMX_U32 g_OMX_OSAL_TraceFlags;

/**********************************************************************
 * MEMORY               
 **********************************************************************/
OMX_PTR OMX_OSAL_Malloc( OMX_IN OMX_U32 size )
{
    OMX_PTR pData;
    pData = (OMX_PTR)malloc(size);
    return pData;
}

void OMX_OSAL_Free( OMX_IN OMX_PTR pData )
{
    free(pData);
}

/* JVW-NVIDIA: Added for http://cvs.khronos.org/bugzilla/show_bug.cgi?id=217 */

OMX_PTR OMX_OSAL_Memset( OMX_IN OMX_PTR pDest, OMX_IN OMX_U32 cChar, OMX_IN OMX_U32 nCount)
{    
    return memset(pDest, cChar, nCount);
}

/* JVW-NVIDIA: Added for http://cvs.khronos.org/bugzilla/show_bug.cgi?id=217 */

OMX_PTR OMX_OSAL_Memcpy( OMX_IN OMX_PTR pDest, OMX_IN OMX_PTR pSrc, OMX_IN OMX_U32 nCount)
{
    return memcpy(pDest, pSrc, nCount);
}

/**********************************************************************
 * THREADS               
 **********************************************************************/

typedef struct OMX_CONF_THREADDATATYPE {
    OMX_HANDLETYPE hActualThread;
    OMX_U32 (*pFunc)(OMX_PTR pParam);
    OMX_PTR pParam;
} OMX_CONF_THREADDATATYPE;

DWORD WINAPI ThreadProc( LPVOID lpParameter )
{ 
   OMX_CONF_THREADDATATYPE *pThreadData;
   pThreadData = (OMX_CONF_THREADDATATYPE *)lpParameter;
   return pThreadData->pFunc(pThreadData->pParam);
}

/** Create a thread given the thread function, a data parameter to pass 
 *  to the thread, and the thread priority (higher values=higher priority).
 *  The thread is created and executed immediately calling the thread 
 *  function with the supplied data parameter. */
OMX_ERRORTYPE OMX_OSAL_ThreadCreate( OMX_IN OMX_U32 (*pFunc)(OMX_PTR pParam), 
                                     OMX_IN OMX_PTR pParam, 
                                     OMX_IN OMX_U32 nPriority, 
                                     OMX_OUT OMX_HANDLETYPE *phThread )
{
    OMX_CONF_THREADDATATYPE *pThreadData;

    pThreadData = (OMX_CONF_THREADDATATYPE*)OMX_OSAL_Malloc(sizeof(OMX_CONF_THREADDATATYPE));
    pThreadData->pFunc = pFunc;
    pThreadData->pParam = pParam;
    
    pThreadData->hActualThread = CreateThread(NULL, 0, ThreadProc, pThreadData, 0, 0);
    if (pThreadData->hActualThread == NULL) {
        *phThread = NULL;
        OMX_OSAL_Free(pThreadData);
        return OMX_ErrorInsufficientResources;
    }

    if (nPriority) {
        SetThreadPriority(*phThread, nPriority);
    }

    *phThread = (OMX_HANDLETYPE)pThreadData;

    return OMX_ErrorNone;       
}

OMX_ERRORTYPE OMX_OSAL_ThreadDestroy( OMX_IN OMX_HANDLETYPE hThread )
{
   OMX_CONF_THREADDATATYPE *pThreadData;
   pThreadData = (OMX_CONF_THREADDATATYPE *)hThread;

   if (WaitForSingleObject(pThreadData->hActualThread, INFINITE) != WAIT_OBJECT_0) {
        return OMX_ErrorBadParameter;
    }

    if (!CloseHandle(pThreadData->hActualThread)) {
        return OMX_ErrorBadParameter;
    }
        
    OMX_OSAL_Free(hThread);

    return OMX_ErrorNone;       
}

/**********************************************************************
 * MUTEX               
 **********************************************************************/

OMX_ERRORTYPE OMX_OSAL_MutexCreate(OMX_OUT OMX_HANDLETYPE *phMutex)
{
    *phMutex = CreateMutex(NULL, FALSE, NULL);
    if (*phMutex == NULL) {
        return OMX_ErrorInsufficientResources;
    }
    return OMX_ErrorNone;       
}

OMX_ERRORTYPE OMX_OSAL_MutexDestroy(OMX_IN OMX_HANDLETYPE hMutex)
{
    if (!CloseHandle(hMutex)) {
        return OMX_ErrorBadParameter;
    }    
    return OMX_ErrorNone;       
}

OMX_ERRORTYPE OMX_OSAL_MutexLock(OMX_IN OMX_HANDLETYPE hMutex)
{
   if (WaitForSingleObject(hMutex, INFINITE) != WAIT_OBJECT_0) {
        return OMX_ErrorBadParameter;
    }
   return OMX_ErrorNone;       
}

OMX_ERRORTYPE OMX_OSAL_MutexUnlock(OMX_IN OMX_HANDLETYPE hMutex)
{
    if (!ReleaseMutex(hMutex)) {
          return OMX_ErrorBadParameter;  
    }
    return OMX_ErrorNone;       
}

/**********************************************************************
 * EVENTS               
 **********************************************************************/

#define INFINITE_WAIT 0xffffffff

OMX_ERRORTYPE OMX_OSAL_EventCreate(OMX_OUT OMX_HANDLETYPE *phEvent)
{
    /* Create a win32 event that
        - has default security
        - must be manualy reset
        - starts as nonsignaled
        - is unnamed
    */
    *phEvent = CreateEvent(NULL, TRUE, 0, NULL);
    if (*phEvent == NULL) {
        return OMX_ErrorInsufficientResources;
    }
    return OMX_ErrorNone;       
}

OMX_ERRORTYPE OMX_OSAL_EventDestroy(OMX_IN OMX_HANDLETYPE hEvent)
{
    if (!CloseHandle(hEvent)) {
        return OMX_ErrorBadParameter;
    }    
    return OMX_ErrorNone;       
}

OMX_ERRORTYPE OMX_OSAL_EventReset(OMX_IN OMX_HANDLETYPE hEvent)
{
    if (!ResetEvent(hEvent)) {
        return OMX_ErrorBadParameter;
    }
    return OMX_ErrorNone;       
}

OMX_ERRORTYPE OMX_OSAL_EventSet(OMX_IN OMX_HANDLETYPE hEvent)
{
    if (!SetEvent(hEvent)) {
        return OMX_ErrorBadParameter;
    }    
    return OMX_ErrorNone;       
}

OMX_ERRORTYPE OMX_OSAL_EventWait(OMX_IN OMX_HANDLETYPE hEvent, OMX_IN OMX_U32 mSec, OMX_OUT OMX_BOOL *pbTimedOut)
{
    *pbTimedOut = 0;
    switch (WaitForSingleObject(hEvent, mSec)) 
    {
    case WAIT_TIMEOUT:
        *pbTimedOut = 1; 
    case WAIT_OBJECT_0:
        return OMX_ErrorNone;       
    default:
        return OMX_ErrorBadParameter;
    }
}

/**********************************************************************
 * TIME
 **********************************************************************/

/** Returns a time value in milliseconds based on a clock starting at
 *  some arbitrary base. Given a call to GetTime that returns a value
 *  of n a subsequent call to GetTime made m milliseconds later should 
 *  return a value of (approximately) (n+m). This method is used, for
 *  instance, to compute the duration of call. */
OMX_U32 OMX_OSAL_GetTime()
{
    //return (OMX_U32) timeGetTime();
	return 0;
}

/**************************************************************
 * LOG FILES
 **************************************************************/

FILE *g_pLogFile=NULL;

/** Open logfile. All enabled tracing will be sent to the file (in addition
 *  to the display). */
OMX_ERRORTYPE OMX_OSAL_OpenLogfile(OMX_IN OMX_STRING sFilename)
{
    OMX_OSAL_CloseLogfile();
    g_pLogFile = fopen(sFilename, "w");
    return g_pLogFile? OMX_ErrorNone : OMX_ErrorBadParameter;
}

/** Close logfile.*/
OMX_ERRORTYPE OMX_OSAL_CloseLogfile()
{
    if (g_pLogFile){
        fclose(g_pLogFile);
        g_pLogFile = NULL;
    }
    return OMX_ErrorNone;
}

/***********************************************************************
 * TRACE
 ***********************************************************************/

static OMX_BOOL bSkipPrefix = OMX_FALSE;

/** Output a trace message */
OMX_ERRORTYPE OMX_OSAL_Trace(OMX_IN OMX_U32 nTraceFlags, OMX_IN char *format, ...)
{
    char szPrefix[5];
    va_list args;

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexLock(g_OMX_CONF_hTraceMutex);

    if (g_OMX_OSAL_TraceFlags & nTraceFlags)
    {
        if (format[0]!='#' && format[0]!='\n' && !bSkipPrefix)
        {
            /* emit trace prefix */
            OMX_OSAL_GetTracePrefix(nTraceFlags,szPrefix);
            printf(szPrefix);
            if (g_pLogFile){
                fprintf(g_pLogFile,szPrefix);
            }
        }

        /* emit trace */
        va_start(args, format);
        vprintf(format, args);
        if (g_pLogFile){
            vfprintf(g_pLogFile,format, args);
        }
        va_end(args);

        /* skip prefix on next output if this is no the end of line */
        if (format[0] != 0)
            bSkipPrefix = (format[strlen(format)-1]  != '\n');
    }    

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexUnlock(g_OMX_CONF_hTraceMutex);

    return OMX_ErrorNone;       
}

/**********************************************************************
 * INPUT FILE MAPPING
 **********************************************************************/

FILE *g_OMX_CONF_pInFile[OMX_CONF_MAXINFILEMAPPINGS];

OMX_ERRORTYPE OMX_OSAL_InPortIndexToArrayIndex( OMX_IN OMX_U32 nPortIndex, OMX_OUT OMX_U32 *pArrayIndex )
{
    OMX_U32 i;
    for (i=0;i<g_OMX_CONF_nInFileMappings;i++){
        if (g_OMX_CONF_InFileMap[i].nPortIndex == nPortIndex){
            *pArrayIndex = i;
            return OMX_ErrorNone;
        }
    }
    return OMX_ErrorBadParameter;
}

OMX_ERRORTYPE OMX_OSAL_OpenInputFile( OMX_IN OMX_U32 nPortIndex )
{
    OMX_U32 nArrayIndex;
    
    if ( OMX_ErrorNone != OMX_OSAL_InPortIndexToArrayIndex(nPortIndex, &nArrayIndex)){
        return OMX_ErrorBadParameter;
    }
    
    g_OMX_CONF_pInFile[nArrayIndex] = fopen(g_OMX_CONF_InFileMap[nArrayIndex].sInputFileName, "rb");

    if (!g_OMX_CONF_pInFile[nArrayIndex]) return OMX_ErrorUndefined;

    return OMX_ErrorNone;
}

OMX_U32 OMX_OSAL_ReadFromInputFile( OMX_OUT OMX_PTR pData, OMX_IN OMX_U32 nBytes, OMX_IN OMX_U32 nPortIndex )
{
    OMX_U32 nArrayIndex;
    if ( OMX_ErrorNone != OMX_OSAL_InPortIndexToArrayIndex(nPortIndex, &nArrayIndex)){
        return 0;
    }

    if (!g_OMX_CONF_pInFile[nArrayIndex]) return 0;

    return (OMX_U32)fread(pData, 1, nBytes, g_OMX_CONF_pInFile[nArrayIndex]);
}

OMX_ERRORTYPE OMX_OSAL_CloseInputFile( OMX_IN OMX_U32 nPortIndex )
{
    OMX_U32 nArrayIndex;
    if ( OMX_ErrorNone != OMX_OSAL_InPortIndexToArrayIndex(nPortIndex, &nArrayIndex)){
        return OMX_ErrorBadParameter;
    }
    
    if (!g_OMX_CONF_pInFile[nArrayIndex]) return OMX_ErrorBadParameter;

    if (fclose(g_OMX_CONF_pInFile[nArrayIndex])) return OMX_ErrorUndefined;
    g_OMX_CONF_pInFile[nArrayIndex] = 0;

    return OMX_ErrorNone;
}

/**********************************************************************
 * OUTPUT FILE MAPPING
 **********************************************************************/

FILE *g_OMX_CONF_pOutFile[OMX_CONF_MAXOUTFILEMAPPINGS];

OMX_ERRORTYPE OMX_OSAL_OutPortIndexToArrayIndex( OMX_IN OMX_U32 nPortIndex, OMX_OUT OMX_U32 *pArrayIndex )
{
    OMX_U32 i;
    for (i=0;i<g_OMX_CONF_nOutFileMappings;i++){
        if (g_OMX_CONF_OutFileMap[i].nPortIndex == nPortIndex){
            *pArrayIndex = i;
            return OMX_ErrorNone;
        }
    }
    return OMX_ErrorBadParameter;
}

OMX_ERRORTYPE OMX_OSAL_OpenOutputFile( OMX_IN OMX_U32 nPortIndex )
{
    OMX_U32 nArrayIndex;
    
    if ( OMX_ErrorNone != OMX_OSAL_OutPortIndexToArrayIndex(nPortIndex, &nArrayIndex)){
        return OMX_ErrorBadParameter;
    }
    
    g_OMX_CONF_pOutFile[nArrayIndex] = fopen(g_OMX_CONF_OutFileMap[nArrayIndex].sOutputFileName, "wb");

    if (!g_OMX_CONF_pOutFile[nArrayIndex]) return OMX_ErrorUndefined;

    return OMX_ErrorNone;
}

OMX_U32 OMX_OSAL_WriteToOutputFile( OMX_OUT OMX_PTR pData, OMX_IN OMX_U32 nBytes, OMX_IN OMX_U32 nPortIndex )
{
    OMX_U32 nArrayIndex;
    if ( OMX_ErrorNone != OMX_OSAL_OutPortIndexToArrayIndex(nPortIndex, &nArrayIndex)){
        return 0;
    }

    if (!g_OMX_CONF_pOutFile[nArrayIndex]) return 0;

    return (OMX_U32)fwrite(pData, 1, nBytes, g_OMX_CONF_pOutFile[nArrayIndex]);
}

OMX_ERRORTYPE OMX_OSAL_CloseOutputFile( OMX_IN OMX_U32 nPortIndex )
{
    OMX_U32 nArrayIndex;
    if ( OMX_ErrorNone != OMX_OSAL_OutPortIndexToArrayIndex(nPortIndex, &nArrayIndex)){
        return OMX_ErrorBadParameter;
    }

    if (!g_OMX_CONF_pOutFile[nArrayIndex]) return OMX_ErrorBadParameter;

    if (fclose(g_OMX_CONF_pOutFile[nArrayIndex])) return OMX_ErrorUndefined;
    g_OMX_CONF_pOutFile[nArrayIndex] = 0;

    return OMX_ErrorNone;
}

/***********************************************************************
 * Conformance Script Parser
 ***********************************************************************/

OMX_ERRORTYPE OMX_OSAL_ProcessCommandsFromFile(OMX_STRING sFileName)
{
    FILE *fptr;
    char sLine[512];

    fptr = fopen(sFileName, "r");
    if (!fptr) {
        OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "Failed to open script file %s\n", sFileName);
        exit(1);
    }
    while (!feof(fptr)){
        if (fgets(sLine, 512, fptr))
        {
            size_t len = strlen(sLine);
            while (sLine[len-1] == '\r' || sLine[len-1] == '\n')
                sLine[--len] = 0;  /* delete CR/LF from end of line */
            OMX_CONF_ParseCommand(sLine);  
        }
    }
    fclose(fptr);

    return OMX_ErrorNone;
}


OMX_ERRORTYPE OMX_OSAL_ProcessCommandsFromPrompt()
{
    char sLine[512];
    do{
        printf("> ");fflush(stdout);
        if (gets(sLine))
        {
            OMX_CONF_ParseCommand(sLine);  
        }
    } while (1 != strlen(sLine) || ('q' != tolower(sLine[0])));

    return OMX_ErrorNone;
}

#define START_LEN 	256

OMX_U32 getline(OMX_STRING *buf, OMX_U32 *pLen, FILE *file)
{
	OMX_STRING p = 0;
	OMX_U32 len, new_len;
	OMX_U32 n = 0;
	OMX_U8 c;
	OMX_U32 dirty_buffer = 0;

	len = START_LEN;
    /*Allocate memory is not allocated*/
    if(*buf==NULL)
        *buf = (OMX_STRING)malloc(START_LEN + 1);
	if (!*buf) {
        OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "Failed to Allocate Memory\n");
        exit(1);
	}

	p = *buf;

    for (;;) {
        c = (OMX_U8) getc(file);
        if (feof(file)) {
            if (dirty_buffer)
	            break;
            *pLen= (OMX_U32) -1;
            return ((OMX_U32) -1);
        }
        if (c == '\n')
        break;

        if (n++ >= len) {
            new_len = len << 1;
            *buf = (OMX_STRING)realloc(buf, new_len + 1);
            if (*buf == NULL) {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "Failed to Re-allocate Memory\n");
                exit(1);
            }
            len = new_len;
            p = *buf;
            p += len >> 1;
        }
        *p = c;
        p++;
        dirty_buffer = 1;
    }

    *p = '\0';
    *pLen=n;
    return n;
}

/**This function open buffer file */
OMX_ERRORTYPE OMX_OSAL_OpenBufferVarFile( OMX_INOUT FILE** pInFile,OMX_IN OMX_STRING sInFileName,OMX_STRING sMode)
{
    *pInFile = fopen(sInFileName, sMode);

    if (!*pInFile) return OMX_ErrorUndefined;

    return OMX_ErrorNone;
}

/**This function read data from file */
OMX_U32 OMX_OSAL_ReadFromBufferVarInputFile( OMX_OUT OMX_PTR pData, OMX_IN OMX_U32 nBytes, OMX_INOUT FILE* pInFile )
{
    if (!pInFile) return 0;

    return (OMX_U32)fread(pData, 1, nBytes, pInFile);
}

OMX_U32 OMX_OSAL_WriteToBufferVarOutputFile( OMX_OUT OMX_PTR pData, OMX_IN OMX_U32 nBytes, OMX_IN FILE* pInFile)
{
    if (!pInFile) return 0;

    return (OMX_U32)fwrite(pData, 1, nBytes, pInFile);
}

/**This function close a file */
OMX_ERRORTYPE OMX_OSAL_CloseBufferVarFile( OMX_INOUT FILE* pInFile )
{
    if (fclose(pInFile)) return OMX_ErrorUndefined;
    
    return OMX_ErrorNone;
}

/**This function parsed buffer variance file */
OMX_ERRORTYPE OMX_OSAL_ReadBufferVarianceFile (OMX_OUT OMX_U8 *sPortIndex,
                                               OMX_OUT OMX_STRING sParamName,
                                               OMX_OUT OMX_STRING sFieldName,
                                               OMX_OUT OMX_U32 *nFieldValue,
                                               OMX_OUT OMX_U32 *nArraySize,
                                               OMX_OUT OMX_U32* nMaxBufSize,
                                               OMX_INOUT OMX_U32 **nBufferVarArray,
                                               OMX_IN FILE* pInFile)
{
    OMX_U32 nRead;
    OMX_U32 nListStartPosition;
    OMX_STRING pLine = NULL;
    OMX_U32 nLen = 0,i=0;

    if (!pInFile) return 1;

    /*Read Port Index*/
    nRead = getline(&pLine, &nLen, pInFile);
    strcpy((OMX_STRING)sPortIndex,pLine);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,"sPortIndex=%s\n", sPortIndex);

    /*Read Parameter Name Index*/
    nRead = getline(&pLine, &nLen, pInFile);
    strcpy(sParamName,pLine);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,"Param Type=%s, %s\n", sParamName,pLine);

    /*Read Field Name Index*/
    nRead = getline(&pLine, &nLen, pInFile);
    strcpy(sFieldName,pLine);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,"Field Type=%s, %s\n", sFieldName,pLine);

    nRead = getline(&pLine, &nLen, pInFile);
    if(nRead!=0)
    *nFieldValue=(OMX_U32)atoi(pLine);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,"Field Value=%d, %s\n", *nFieldValue,pLine);
    nListStartPosition = ftell(pInFile);

    /*Count number of buffer to be sent on the input port*/
    *nArraySize=0;
    while ((nRead = getline(&pLine, &nLen, pInFile)) != -1) {
      (*nArraySize)++;
    }

    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,"nArraySize=%d\n", *nArraySize);

    /*Allocate buffer size and It's freed in cleanup ports function*/
    *nBufferVarArray=(OMX_U32 *)malloc((*nArraySize)*sizeof(OMX_U32));
    *nMaxBufSize=0;

    fseek(pInFile,nListStartPosition,SEEK_SET);
    while ((nRead = getline(&pLine, &nLen, pInFile)) != -1) {
        (*nBufferVarArray)[i]=atoi(pLine);
        /*Calculation maximum size of the buffer*/
        *nMaxBufSize=((*nMaxBufSize>(*nBufferVarArray)[i])?*nMaxBufSize:(*nBufferVarArray)[i]);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,"i=%d nBufferSize=%d, %s\n",i, (*nBufferVarArray)[i],pLine);
        i++;
    }

    if (pLine)
        free(pLine);

    return 0;

}

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */
