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

/** OMX_CONF_ClockComponentTest.c
 *  OpenMax IL conformance test validating clock component functionality.
 */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include "OMX_OSAL_Interfaces.h"
#include "OMX_CONF_TestHarness.h"
#include "OMX_CONF_StubbedCallbacks.h"
#include "OMX_CONF_TunnelTestComponent.h"

    /* Callback data */
    typedef struct CCTDATATYPE
    {
        OMX_STATETYPE eState;
        OMX_HANDLETYPE hStateChangeEvent;
        OMX_HANDLETYPE hCUT;
    } CCTDATATYPE;

    /* Test proper support for Clock Component configs */
    OMX_ERRORTYPE CCTGTestConfigs(OMX_HANDLETYPE hComp)
    {
        OMX_TIME_CONFIG_SCALETYPE           oScale;
        OMX_TIME_CONFIG_CLOCKSTATETYPE      oClockState;
        OMX_TIME_CONFIG_ACTIVEREFCLOCKTYPE  oActiveClock;
        OMX_TIME_CONFIG_TIMESTAMPTYPE       oTimeStamp;
        OMX_ERRORTYPE eError;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Test Clock Component Configs.\n");

        INIT_CONFIG(oScale);
        INIT_CONFIG(oClockState);
        INIT_CONFIG(oActiveClock);
        INIT_CONFIG(oTimeStamp);
        oTimeStamp.nPortIndex = 0;

        /* OMX_IndexConfigTimeScale */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Confirm support for OMX_IndexConfigTimeScale\n");
        OMX_CONF_FAIL_IF_ERROR(OMX_GetConfig(hComp, OMX_IndexConfigTimeScale, (OMX_PTR)&oScale));
        oScale.xScale = 0x00010000; /*1x*/
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeScale, (OMX_PTR)&oScale));

        /* OMX_IndexConfigTimeClockState */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Confirm support for OMX_IndexConfigTimeClockState\n");
        OMX_CONF_FAIL_IF_ERROR(OMX_GetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        oClockState.eState = OMX_TIME_ClockStateStopped;
        oClockState.nOffset = oClockState.nStartTime = oClockState.nWaitMask = 0;
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));

        /* OMX_IndexConfigTimeActiveRefClock */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Confirm support for OMX_IndexConfigTimeActiveRefClock\n");
        OMX_CONF_FAIL_IF_ERROR(OMX_GetConfig(hComp, OMX_IndexConfigTimeActiveRefClock, (OMX_PTR)&oActiveClock));
        oActiveClock.eClock = OMX_TIME_RefClockNone;
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeActiveRefClock, (OMX_PTR)&oActiveClock));

        /* OMX_IndexConfigTimeCurrentMediaTime */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Confirm support for OMX_IndexConfigTimeCurrentMediaTime\n");
        OMX_CONF_FAIL_IF_ERROR(OMX_GetConfig(hComp, OMX_IndexConfigTimeCurrentMediaTime, (OMX_PTR)&oTimeStamp));
        oTimeStamp.nTimestamp = 0;
        if (OMX_ErrorNone == OMX_SetConfig(hComp, OMX_IndexConfigTimeCurrentMediaTime, (OMX_PTR)&oTimeStamp))
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,
                           "--ERROR:  OMX_SetConfig on read-only OMX_IndexConfigTimeCurrentMediaTime should fail.\n");
            return OMX_ErrorBadParameter;
        }

        /* OMX_IndexConfigTimeCurrentWallTime */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Confirm support for OMX_IndexConfigTimeCurrentWallTime\n");
        OMX_CONF_FAIL_IF_ERROR(OMX_GetConfig(hComp, OMX_IndexConfigTimeCurrentWallTime, (OMX_PTR)&oTimeStamp));
        oTimeStamp.nTimestamp = 0;
        if (OMX_ErrorNone == OMX_SetConfig(hComp, OMX_IndexConfigTimeCurrentWallTime, (OMX_PTR)&oTimeStamp))
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,
                           "--ERROR:  OMX_SetConfig on read-only OMX_IndexConfigTimeCurrentWallTime should fail.\n");
            return OMX_ErrorBadParameter;
        }

        /* OMX_IndexConfigTimeClientStartTime */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Confirm support for OMX_IndexConfigTimeActiveRefClock\n");
        OMX_CONF_FAIL_IF_ERROR(OMX_GetConfig(hComp, OMX_IndexConfigTimeActiveRefClock, (OMX_PTR)&oTimeStamp));
        oTimeStamp.nTimestamp = 0;
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeClientStartTime, (OMX_PTR)&oTimeStamp));

        /* OMX_IndexConfigTimeCurrentAudioReference */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Confirm support for OMX_IndexConfigTimeCurrentAudioReference\n");
        if (OMX_ErrorNone == OMX_GetConfig(hComp, OMX_IndexConfigTimeCurrentAudioReference, (OMX_PTR)&oTimeStamp))
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,
                           "--ERROR:  OMX_GetConfig on write-only OMX_IndexConfigTimeCurrentAudioReference should fail.\n");
            return OMX_ErrorBadParameter;
        }
        oTimeStamp.nTimestamp = 0;
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeCurrentAudioReference, (OMX_PTR)&oTimeStamp));

        /* OMX_IndexConfigTimeCurrentVideoReference */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Confirm support for OMX_IndexConfigTimeCurrentVideoReference\n");
        if (OMX_ErrorNone == OMX_GetConfig(hComp, OMX_IndexConfigTimeCurrentVideoReference, (OMX_PTR)&oTimeStamp))
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO,
                           "--ERROR:  OMX_GetConfig on write-only OMX_IndexConfigTimeCurrentVideoReference should fail.\n");
            return OMX_ErrorBadParameter;
        }
        oTimeStamp.nTimestamp = 0;
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeCurrentVideoReference, (OMX_PTR)&oTimeStamp));

        return OMX_ErrorNone;

OMX_CONF_TEST_FAIL:
        if (eError != OMX_ErrorNone)
            return eError;
        else
            return OMX_ErrorUndefined;
    }

    OMX_TIME_MEDIATIMETYPE g_Update;	    /* the last media time update received */
    OMX_TIME_MEDIATIMETYPE g_StateChange;	/* the last media time update received that was a state change */
    OMX_HANDLETYPE g_WaitEvent;			/* the event we use to wait on a media time update */
    OMX_HANDLETYPE g_WaitMutex;			/* the mutex for access to the media time update */

    /* Callback called when the Tunnel Test Component recieves a OnEmptythisBuffer call */
    OMX_ERRORTYPE CCTOnEmptyThisBuffer(OMX_BUFFERHEADERTYPE *pHdr)
    {
        OMX_OSAL_MutexLock(g_WaitMutex);
        g_Update = *((OMX_TIME_MEDIATIMETYPE *)pHdr->pBuffer);
        if (g_Update.eUpdateType == OMX_TIME_UpdateClockStateChanged)
        {
            g_StateChange = g_Update;
        }

        OMX_OSAL_EventSet(g_WaitEvent);
        OMX_OSAL_MutexUnlock(g_WaitMutex);

        return OMX_ErrorNone;
    }
    /* Wait for the Component Under Test to send a media time update, timing out after the given duration */
    OMX_BOOL CCTWaitForMediaTimeUpdate(OMX_U32 timeoutMsec)
    {
        OMX_BOOL bTimedOut = OMX_FALSE;

        /* wait */
        OMX_OSAL_EventWait(g_WaitEvent, timeoutMsec, &bTimedOut);
        OMX_OSAL_MutexLock(g_WaitMutex);
        OMX_OSAL_EventReset(g_WaitEvent);
        OMX_OSAL_MutexUnlock(g_WaitMutex);

        return bTimedOut ? OMX_FALSE : OMX_TRUE;
    }


    void CCTClearMediaTimeUpdate()
    {
        OMX_OSAL_MutexLock(g_WaitMutex);
        g_Update.eUpdateType = OMX_TIME_UpdateMax;
        OMX_OSAL_EventReset(g_WaitEvent);
        OMX_OSAL_MutexUnlock(g_WaitMutex);
    }

    void CCTClearStateChangeUpdate()
    {
        CCTWaitForMediaTimeUpdate(OMX_CONF_TIMEOUT_EXPECTING_SUCCESS); /* flush any old event */

        OMX_OSAL_MutexLock(g_WaitMutex);
        g_Update.eUpdateType = OMX_TIME_UpdateMax;
        g_StateChange.eUpdateType = OMX_TIME_UpdateMax;
        OMX_OSAL_EventReset(g_WaitEvent);
        OMX_OSAL_MutexUnlock(g_WaitMutex);
    }


    /* Wait for a clock state change notification (via a media time update) and confirm it is the one we expect */
    OMX_BOOL CCTConfirmClockStateChange(OMX_HANDLETYPE hComp, OMX_TIME_CLOCKSTATE eState)
    {
        OMX_TIME_CONFIG_CLOCKSTATETYPE oClockState;
        if (OMX_TRUE == CCTWaitForMediaTimeUpdate(OMX_CONF_TIMEOUT_EXPECTING_SUCCESS))
        {
            if ((g_StateChange.eUpdateType != OMX_TIME_UpdateClockStateChanged) ||
                    (eState != g_StateChange.eState))
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--ERROR:  Bad parameters in clock state change update, type %d, state %d\n",
                               g_StateChange.eUpdateType, g_StateChange.eState);
                return OMX_FALSE;
            }
        }
        else
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--ERROR:  Missing clock state change update\n");
            return OMX_FALSE;
        }

        if (OMX_ErrorNone != OMX_GetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState))
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--ERROR:  OMX_GetConfig failed.\n");
            return OMX_FALSE;
        }
        if (eState != oClockState.eState)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--ERROR:  OMX_GetConfig returned wrong state.\n");
            return OMX_FALSE;
        }
        return OMX_TRUE;
    }

#define CCT_STARTTIME   ((OMX_TICKS)100*OMX_TICKS_PER_SECOND)
#define CCT_PASTTIME    ((OMX_TICKS)99*OMX_TICKS_PER_SECOND)
#define CCT_FUTURETIME  ((OMX_TICKS)101*OMX_TICKS_PER_SECOND)
#define CCT_WAITTIME    2000

    /* Test Clock Component's rate control functionality */
    OMX_ERRORTYPE CCTRateControl(OMX_HANDLETYPE hComp)
    {
        OMX_TIME_CONFIG_CLOCKSTATETYPE oClockState;
        OMX_TIME_CONFIG_SCALETYPE oScale;
        OMX_TIME_CONFIG_MEDIATIMEREQUESTTYPE oRequest;
        OMX_ERRORTYPE eError;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Test Rate Control.\n");

        /* setup config structures */
        oClockState.nSize = sizeof(oClockState);
        oClockState.nVersion = g_OMX_CONF_Version;
        oClockState.nWaitMask = 0;
        oClockState.nOffset = 0;
        oScale.nSize = sizeof(oScale);
        oScale.nVersion = g_OMX_CONF_Version;
        oRequest.nSize = sizeof(oRequest);
        oRequest.nVersion = g_OMX_CONF_Version;

        /* Scale change */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Confirm scale change.\n");
        oScale.xScale = 0x00020000; /*2x*/
        CCTClearMediaTimeUpdate();
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeScale, (OMX_PTR)&oScale));
        if (OMX_TRUE == CCTWaitForMediaTimeUpdate(OMX_CONF_TIMEOUT_EXPECTING_SUCCESS))
        {
            if ((g_Update.eUpdateType != OMX_TIME_UpdateScaleChanged) ||
                    (0x00020000 != g_Update.xScale))
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--ERROR:  Bad parameters in scale change update.\n");
                return OMX_ErrorUndefined;
            }
        }
        else
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--ERROR:  Missing scale change update.\n");
            return OMX_ErrorTimeout;
        }
        if (OMX_ErrorNone != OMX_GetConfig(hComp, OMX_IndexConfigTimeScale, (OMX_PTR)&oScale))
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--ERROR:  OMX_GetConfig failed.\n");
            return OMX_ErrorUndefined;
        }
        if (0x00020000 != oScale.xScale)
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--ERROR:  OMX_GetConfig returned wrong scale.\n");
            return OMX_ErrorUndefined;
        }

        /* use same start time througout */
        oClockState.nStartTime = CCT_STARTTIME;

        /* Future request when playing forward */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Confirm future request when playing forward is fulfilled.\n");
        oScale.xScale = 0x00010000; /*1x*/
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeScale, (OMX_PTR)&oScale));
        oClockState.eState = OMX_TIME_ClockStateRunning;
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        oRequest.nMediaTimestamp = CCT_FUTURETIME;
        oRequest.nOffset = 0;
        oRequest.nPortIndex = 0;
        CCTClearMediaTimeUpdate();
        CCTWaitForMediaTimeUpdate(OMX_CONF_TIMEOUT_EXPECTING_SUCCESS); /* flush any old event */
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeMediaTimeRequest, (OMX_PTR)&oRequest));
        if (OMX_TRUE == CCTWaitForMediaTimeUpdate(CCT_WAITTIME))
        {
            if ((g_Update.eUpdateType != OMX_TIME_UpdateRequestFulfillment) ||
                    (CCT_FUTURETIME != oRequest.nMediaTimestamp))
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--ERROR:  Bad parameters in request fulfillment.\n");
                return OMX_ErrorUndefined;
            }
        }
        else
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--ERROR:  Missing request fulfillment\n");
            return OMX_ErrorUndefined;
        }

        /* Request when stopped */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Confirm request when stopped not fulfilled.\n");
        oClockState.eState = OMX_TIME_ClockStateStopped;
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        CCTWaitForMediaTimeUpdate(OMX_CONF_TIMEOUT_EXPECTING_SUCCESS); /* flush any old event */
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeMediaTimeRequest, (OMX_PTR)&oRequest));
        if (OMX_TRUE == CCTWaitForMediaTimeUpdate(CCT_WAITTIME))
        {
            if ((g_Update.eUpdateType == OMX_TIME_UpdateRequestFulfillment) &&
                    (CCT_FUTURETIME == oRequest.nMediaTimestamp))
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--ERROR:  Fulfilled request when stopped.\n");
                return OMX_ErrorUndefined;
            }
        }

        /* Request when waiting */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Confirm request when waiting not fulfilled.\n");
        oClockState.eState = OMX_TIME_ClockStateWaitingForStartTime;
        oClockState.nWaitMask = 1;
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        CCTWaitForMediaTimeUpdate(OMX_CONF_TIMEOUT_EXPECTING_SUCCESS); /* flush any old event */
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeMediaTimeRequest, (OMX_PTR)&oRequest));
        if (OMX_TRUE == CCTWaitForMediaTimeUpdate(CCT_WAITTIME))
        {
            if ((g_Update.eUpdateType == OMX_TIME_UpdateRequestFulfillment) &&
                    (CCT_FUTURETIME == oRequest.nMediaTimestamp))
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--ERROR:  Fulfilled request when waiting.\n");
                return OMX_ErrorUndefined;
            }
        }

        /* Request when paused */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Confirm request when paused not fulfilled.\n");
        oClockState.eState = OMX_TIME_ClockStateStopped;
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        oClockState.eState = OMX_TIME_ClockStateRunning;
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        oScale.xScale = 0; /*paused*/
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeScale, (OMX_PTR)&oScale));
        CCTWaitForMediaTimeUpdate(OMX_CONF_TIMEOUT_EXPECTING_SUCCESS); /* flush any old event */
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeMediaTimeRequest, (OMX_PTR)&oRequest));
        if (OMX_TRUE == CCTWaitForMediaTimeUpdate(CCT_WAITTIME))
        {
            if ((g_Update.eUpdateType == OMX_TIME_UpdateRequestFulfillment) &&
                    (CCT_FUTURETIME == oRequest.nMediaTimestamp))
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--ERROR:  Fulfilled request when paused.\n");
                return OMX_ErrorUndefined;
            }
        }

        /* Future request when playing backward */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Confirm future request when playing backward is fulfilled.\n");
        oClockState.eState = OMX_TIME_ClockStateStopped;
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        oScale.xScale = 0xffff0000; /*-1x*/
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeScale, (OMX_PTR)&oScale));
        oClockState.eState = OMX_TIME_ClockStateRunning;
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        oRequest.nMediaTimestamp = CCT_FUTURETIME;
        oRequest.nOffset = 0;
        oRequest.nPortIndex = 0;
        CCTWaitForMediaTimeUpdate(OMX_CONF_TIMEOUT_EXPECTING_SUCCESS); /* flush any old event */
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeMediaTimeRequest, (OMX_PTR)&oRequest));
        if (OMX_TRUE == CCTWaitForMediaTimeUpdate(CCT_WAITTIME))
        {
            if ((g_Update.eUpdateType != OMX_TIME_UpdateRequestFulfillment) ||
                    (CCT_FUTURETIME != oRequest.nMediaTimestamp))
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--ERROR:  Bad parameters in request fulfillment.\n");
                return OMX_ErrorUndefined;
            }
        }
        else
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--ERROR:  Missing request fulfillment\n");
            return OMX_ErrorUndefined;
        }

        /* Past request when playing backward */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Confirm past request when playing backward is fulfilled.\n");
        oClockState.eState = OMX_TIME_ClockStateStopped;
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        oScale.xScale = 0xffff0000; /*-1x*/
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeScale, (OMX_PTR)&oScale));
        oClockState.eState = OMX_TIME_ClockStateRunning;
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        oRequest.nMediaTimestamp = CCT_PASTTIME;
        oRequest.nOffset = 0;
        oRequest.nPortIndex = 0;
        CCTWaitForMediaTimeUpdate(OMX_CONF_TIMEOUT_EXPECTING_SUCCESS); /* flush any old event */
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeMediaTimeRequest, (OMX_PTR)&oRequest));
        if (OMX_TRUE == CCTWaitForMediaTimeUpdate(CCT_WAITTIME))
        {
            if ((g_Update.eUpdateType != OMX_TIME_UpdateRequestFulfillment) ||
                    (CCT_PASTTIME != oRequest.nMediaTimestamp))
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--ERROR:  Bad parameters in request fulfillment.\n");
                return OMX_ErrorUndefined;
            }
        }
        else
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--ERROR:  Missing request fulfillment\n");
            return OMX_ErrorUndefined;
        }

        /* Past request when playing forward */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Confirm past request when playing forward is fulfilled.\n");
        oClockState.eState = OMX_TIME_ClockStateStopped;
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        oScale.xScale = 0x00010000; /*1x*/
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeScale, (OMX_PTR)&oScale));
        oClockState.eState = OMX_TIME_ClockStateRunning;
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        oRequest.nMediaTimestamp = CCT_PASTTIME;
        oRequest.nOffset = 0;
        oRequest.nPortIndex = 0;
        CCTWaitForMediaTimeUpdate(OMX_CONF_TIMEOUT_EXPECTING_SUCCESS); /* flush any old event */
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeMediaTimeRequest, (OMX_PTR)&oRequest));
        if (OMX_TRUE == CCTWaitForMediaTimeUpdate(CCT_WAITTIME))
        {
            if ((g_Update.eUpdateType != OMX_TIME_UpdateRequestFulfillment) ||
                    (CCT_PASTTIME != oRequest.nMediaTimestamp))
            {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--ERROR:  Bad parameters in request fulfillment.\n");
                return OMX_ErrorUndefined;
            }
        }
        else
        {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "--ERROR:  Missing request fulfillment\n");
            return OMX_ErrorUndefined;
        }

        return OMX_ErrorNone;

OMX_CONF_TEST_FAIL:
        if (eError != OMX_ErrorNone)
            return eError;
        else
            return OMX_ErrorUndefined;
    }

    /* Test Clock Component's clock state functionality */
    OMX_ERRORTYPE CCTClockStateTransitions(OMX_HANDLETYPE hComp)
    {
        OMX_TIME_CONFIG_CLOCKSTATETYPE oClockState;
        OMX_TIME_CONFIG_TIMESTAMPTYPE nTimestamp;
        OMX_ERRORTYPE eError = OMX_ErrorNone;

        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Test Clock State Transitions.\n");

        oClockState.nSize = sizeof(oClockState);
        oClockState.nVersion = g_OMX_CONF_Version;
        oClockState.nWaitMask = 0;
        oClockState.nOffset = 0;

        /* Check that we're stopped */
        OMX_CONF_FAIL_IF_ERROR(OMX_GetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Confirm initial state.\n");
        OMX_CONF_FAIL_IF_NEQ(OMX_TIME_ClockStateStopped, oClockState.eState);

        /* Stopped->Running */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Confirm stopped->running transition.\n");
        oClockState.eState = OMX_TIME_ClockStateRunning;
        CCTClearStateChangeUpdate();
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        OMX_CONF_FAIL_IF(!CCTConfirmClockStateChange(hComp, OMX_TIME_ClockStateRunning));

        /* Running->Waiting MUST FAIL */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Confirm running->waiting transition fails.\n");
        oClockState.eState = OMX_TIME_ClockStateWaitingForStartTime;
        eError = OMX_SetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR) & oClockState);
        OMX_CONF_FAIL_IF_ERROR(OMX_GetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        OMX_CONF_FAIL_IF_NEQ(oClockState.eState, OMX_TIME_ClockStateRunning);

        /* Running->Stopped */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Confirm running->stopped transition.\n");
        oClockState.eState = OMX_TIME_ClockStateStopped;
        CCTClearStateChangeUpdate();
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        OMX_CONF_FAIL_IF(!CCTConfirmClockStateChange(hComp, OMX_TIME_ClockStateStopped));

        /* Stopped->Waiting...Running (before start time is sent) */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Confirm stopped->waiting...running transitions.\n");
        oClockState.eState = OMX_TIME_ClockStateWaitingForStartTime;
        oClockState.nWaitMask = 0x1;
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        OMX_CONF_FAIL_IF_ERROR(OMX_GetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        OMX_CONF_FAIL_IF_NEQ(OMX_TIME_ClockStateWaitingForStartTime, oClockState.eState);

        /* Stopped->Waiting...Running (after start time is sent) */
        nTimestamp.nPortIndex = 0;
        nTimestamp.nTimestamp = OMX_TICKS_PER_SECOND * 5;
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeClientStartTime, (OMX_PTR)&nTimestamp));
        OMX_CONF_FAIL_IF_ERROR(OMX_GetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        OMX_CONF_FAIL_IF_NEQ(OMX_TIME_ClockStateRunning, oClockState.eState);

        /* Waiting->Running */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Confirm waiting->running transition.\n");
        oClockState.eState = OMX_TIME_ClockStateStopped;
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        oClockState.eState = OMX_TIME_ClockStateWaitingForStartTime;
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        oClockState.eState = OMX_TIME_ClockStateRunning;
        CCTClearStateChangeUpdate();
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        OMX_CONF_FAIL_IF(!CCTConfirmClockStateChange(hComp, OMX_TIME_ClockStateRunning));

        /* Waiting->Stopped */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "-Confirm waiting->stopped transition.\n");
        oClockState.eState = OMX_TIME_ClockStateStopped;
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        oClockState.eState = OMX_TIME_ClockStateWaitingForStartTime;
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        oClockState.eState = OMX_TIME_ClockStateStopped;

        CCTClearStateChangeUpdate();
        OMX_CONF_FAIL_IF_ERROR(OMX_SetConfig(hComp, OMX_IndexConfigTimeClockState, (OMX_PTR)&oClockState));
        OMX_CONF_FAIL_IF(!CCTConfirmClockStateChange(hComp, OMX_TIME_ClockStateStopped));

        return OMX_ErrorNone;

OMX_CONF_TEST_FAIL:
        if (eError != OMX_ErrorNone)
            return eError;
        else
            return OMX_ErrorUndefined;
    }

    /* Clock Component Test's implementation of OMX_CALLBACKTYPE.EventHandler */
    OMX_ERRORTYPE CCTEventHandler(
        OMX_IN OMX_HANDLETYPE hComponent,
        OMX_IN OMX_PTR pAppData,
        OMX_IN OMX_EVENTTYPE eEvent,
        OMX_IN OMX_U32 nData1,
        OMX_IN OMX_U32 nData2,
        OMX_IN OMX_PTR pEventData)
    {
        CCTDATATYPE* pContext = pAppData;

        UNUSED_PARAMETER(pEventData);

        if (hComponent != pContext->hCUT)
        {
            return OMX_ErrorNone;
        }

        if ((eEvent == OMX_EventCmdComplete) && ((OMX_COMMANDTYPE)(nData1) == OMX_CommandStateSet))
        {
            pContext->eState = (OMX_STATETYPE)(nData2);
            OMX_OSAL_EventSet(pContext->hStateChangeEvent);
        }
        return OMX_ErrorNone;
    }

    /* Wait for the Component Under Test to change to state and confirm it is the one we expect */
    OMX_ERRORTYPE WaitForState(CCTDATATYPE *pAppData, OMX_STATETYPE eState)
    {
        OMX_BOOL bTimedOut = OMX_FALSE;

        OMX_OSAL_EventWait(pAppData->hStateChangeEvent, OMX_CONF_TIMEOUT_EXPECTING_SUCCESS, &bTimedOut);
        if (bTimedOut || pAppData->eState != eState)
        {
            return OMX_ErrorUndefined;
        }
        return OMX_ErrorNone;
    }

    /* Main entrypoint into the Clock Component Test */
    OMX_ERRORTYPE OMX_CONF_ClockComponentTest(OMX_IN OMX_STRING cComponentName)
    {
        OMX_ERRORTYPE  eTemp, eError = OMX_ErrorNone;
        OMX_HANDLETYPE hComp, hWrappedComp, hTTComp, hWrappedTTComp;
        OMX_CALLBACKTYPE oCallbacks;
        CCTDATATYPE oAppData;
        OMX_CALLBACKTYPE *pWrappedCallbacks;
        OMX_PTR pWrappedAppData;

        /* setup synchronization objects */
        OMX_OSAL_EventCreate(&g_WaitEvent);
        OMX_OSAL_EventReset(g_WaitEvent);
        OMX_OSAL_MutexCreate(&g_WaitMutex);

        /* create state change event */
        OMX_OSAL_EventCreate(&oAppData.hStateChangeEvent);
        OMX_OSAL_EventReset(oAppData.hStateChangeEvent);

        /* init component handles */
        hComp = hWrappedComp = hTTComp = hWrappedTTComp = 0;

        oCallbacks.EventHandler    = CCTEventHandler;
        oCallbacks.EmptyBufferDone = StubbedEmptyBufferDone;
        oCallbacks.FillBufferDone  = StubbedFillBufferDone;
        eError = OMX_CONF_CallbackTracerCreate(&oCallbacks, (OMX_PTR) & oAppData, cComponentName,
                                               &pWrappedCallbacks, &pWrappedAppData);

        /* Initialize OpenMax */
        eError = OMX_Init();

        /* Acquire component under test handle */
        eError = OMX_GetHandle(&hComp, cComponentName, pWrappedAppData, pWrappedCallbacks);
        OMX_CONF_FAIL_IF_ERROR(eError);
        eError = OMX_CONF_ComponentTracerCreate(hComp, cComponentName, &hWrappedComp);
        OMX_CONF_FAIL_IF_ERROR(eError);
        oAppData.hCUT = hComp;

        /* Acquire tunnel test component handle */
        eError = OMX_CONF_GetTunnelTestComponentHandle(&hTTComp, pWrappedAppData, pWrappedCallbacks);
        OMX_CONF_FAIL_IF_ERROR(eError);
        eError = OMX_CONF_ComponentTracerCreate(hTTComp, "OMX.CONF.tunnel.test", &hWrappedTTComp);
        OMX_CONF_FAIL_IF_ERROR(eError);

        /* Set the OnEmptyThisBuffer callback */
        OMX_CONF_SetTTCOnEmptyThisBuffer(hTTComp, CCTOnEmptyThisBuffer);

        /* Connect to each TTC port to each CUT port */
        OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "Connecting all ports.\n");
        OMX_CONF_FAIL_IF_ERROR(OMX_CONF_TTCConnectAllPorts(hWrappedTTComp, hWrappedComp));

        /* transition CUT to idle */
        OMX_CONF_FAIL_IF_ERROR(OMX_SendCommand(hWrappedComp, OMX_CommandStateSet, OMX_StateIdle, 0));

        /* transition TTC to idle */
        OMX_CONF_FAIL_IF_ERROR(OMX_SendCommand(hWrappedTTComp, OMX_CommandStateSet, OMX_StateIdle, 0));

        /* transition CUT to executing */
        WaitForState(&oAppData, OMX_StateIdle);
        OMX_CONF_FAIL_IF_ERROR(OMX_SendCommand(hWrappedComp, OMX_CommandStateSet, OMX_StateExecuting, 0));
        WaitForState(&oAppData, OMX_StateExecuting);

        /* transition TTC to executing  */
        OMX_CONF_FAIL_IF_ERROR(OMX_SendCommand(hWrappedTTComp, OMX_CommandStateSet, OMX_StateExecuting, 0));

        /* query some configs */
        OMX_CONF_FAIL_IF_ERROR(CCTGTestConfigs(hWrappedComp));

        /* test some clock state changes */
        OMX_CONF_FAIL_IF_ERROR(CCTClockStateTransitions(hWrappedComp));

        /* test some clock state changes */
        OMX_CONF_FAIL_IF_ERROR(CCTRateControl(hWrappedComp));

OMX_CONF_TEST_FAIL:

        /* transition CUT to Loaded state */
        if (hWrappedComp)
        {
            OMX_OSAL_EventReset(oAppData.hStateChangeEvent);
            OMX_CONF_REMEMBER_ERROR(OMX_SendCommand(hWrappedComp, OMX_CommandStateSet, OMX_StateIdle, 0));
            WaitForState(&oAppData, OMX_StateIdle);
            OMX_OSAL_EventReset(oAppData.hStateChangeEvent);
            OMX_CONF_REMEMBER_ERROR(OMX_SendCommand(hWrappedComp, OMX_CommandStateSet, OMX_StateLoaded, 0));
            WaitForState(&oAppData, OMX_StateLoaded);
        }

        /* transition TTC to Loaded state */
        if (hWrappedTTComp)
        {
            OMX_CONF_REMEMBER_ERROR(OMX_SendCommand(hWrappedTTComp, OMX_CommandStateSet, OMX_StateIdle, 0));
            OMX_CONF_REMEMBER_ERROR(OMX_SendCommand(hWrappedTTComp, OMX_CommandStateSet, OMX_StateLoaded, 0));
        }

        /* destroy state change event */
        if (hComp)
        {
            OMX_CONF_REMEMBER_ERROR(OMX_FreeHandle(hComp));
        }

        if (hTTComp)
        {
            OMX_CONF_REMEMBER_ERROR(OMX_CONF_FreeTunnelTestComponentHandle(hTTComp));
        }

        if (hWrappedComp)
        {
            OMX_CONF_REMEMBER_ERROR(OMX_CONF_ComponentTracerDestroy(hWrappedComp));
        }

        if (hWrappedTTComp)
        {
            OMX_CONF_REMEMBER_ERROR(OMX_CONF_ComponentTracerDestroy(hWrappedTTComp));
        }

        OMX_CONF_REMEMBER_ERROR(OMX_CONF_CallbackTracerDestroy(pWrappedCallbacks, pWrappedAppData));

        OMX_CONF_REMEMBER_ERROR(OMX_Deinit());

        /* clean up synchronization objects */
        OMX_OSAL_EventDestroy(oAppData.hStateChangeEvent);

        OMX_OSAL_EventDestroy(g_WaitEvent);
        OMX_OSAL_MutexDestroy(g_WaitMutex);

        return eError;
    }

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */
