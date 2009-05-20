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

/** OMX_CONF_Trace.c
 *  Implementation of tracing functions for OpenMax IL conformance testing.
 *  Includes tracing for component and callback functions and their parameters.
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <OMX_Types.h>
#include <OMX_Component.h>
#include <OMX_Core.h>
#include <OMX_Index.h>
#include <OMX_Image.h>
#include <OMX_Audio.h>
#include <OMX_Video.h>
#include <OMX_IVCommon.h>
#include <OMX_Other.h>

#include "OMX_OSAL_Interfaces.h"
#include "OMX_CONF_TestHarness.h"

#include "string.h"

/**********************************************************************
 * TRACING
 **********************************************************************/

OMX_U32 g_OMX_OSAL_TraceFlags = 0;
OMX_HANDLETYPE g_OMX_CONF_hTraceMutex;

OMX_ERRORTYPE OMX_CONF_SetTraceflags(OMX_IN OMX_U32 nTraceFlags)
{
    OMX_OSAL_Trace(OMX_OSAL_TRACE_INFO, "\nOMX_CONF_SetTraceFlags 0x%08x\n\n", nTraceFlags);

    if (!nTraceFlags){
        OMX_OSAL_Trace(OMX_OSAL_TRACE_WARNING,
            "Setting Trace Flags to 0. Test harness will emit no output whatsoever.\n");
    }

    g_OMX_OSAL_TraceFlags = nTraceFlags;
    return OMX_ErrorNone;
}

/**********************************************************************
 * COMMAND TO STRING
 **********************************************************************/

typedef struct OMX_CONF_COMMANDLOOKUPTYPE {
    OMX_COMMANDTYPE eCommand;
    OMX_STRING sName;
} OMX_CONF_COMMANDLOOKUPTYPE;

OMX_CONF_COMMANDLOOKUPTYPE OMX_CONF_CommandLookupTable[] =
{
    {OMX_CommandStateSet,     "OMX_CommandStateSet"},
    {OMX_CommandFlush,        "OMX_CommandFlush"},
    {OMX_CommandPortDisable,     "OMX_CommandPortDisable"},
    {OMX_CommandPortEnable,  "OMX_CommandPortEnable"},
    {OMX_CommandMarkBuffer,   "OMX_CommandMarkBuffer"},
    {OMX_CommandMax,          "OMX_CommandMax"}
};

OMX_ERRORTYPE OMX_CONF_CommandToString( OMX_IN OMX_COMMANDTYPE eCommand, OMX_OUT OMX_STRING sName)
{
    int i;
    for(i =0;i<(sizeof(OMX_CONF_CommandLookupTable)/sizeof(OMX_CONF_COMMANDLOOKUPTYPE));i++){
        if (OMX_CONF_CommandLookupTable[i].eCommand == eCommand){
            strcpy( sName, OMX_CONF_CommandLookupTable[i].sName);
            return OMX_ErrorNone;
        }
    }
    return OMX_ErrorBadParameter;
}

/**********************************************************************
 * EVENT TO STRING
 **********************************************************************/

typedef struct OMX_CONF_EVENTLOOKUPTYPE {
    OMX_EVENTTYPE eEvent;
    OMX_STRING sName;
} OMX_CONF_EVENTLOOKUPTYPE;

OMX_CONF_EVENTLOOKUPTYPE OMX_CONF_EventLookupTable[] =
{
    {OMX_EventCmdComplete,           "OMX_EventCmdComplete"},
    {OMX_EventError,                 "OMX_EventError"},
    {OMX_EventMark,                  "OMX_EventMark"},
    {OMX_EventPortSettingsChanged,   "OMX_EventPortSettingsChanged"},
    {OMX_EventBufferFlag,            "OMX_EventBufferFlag"},
    {OMX_EventMax,                   "OMX_EventMax"}
};

OMX_ERRORTYPE OMX_CONF_EventToString( OMX_IN OMX_EVENTTYPE eEvent, OMX_OUT OMX_STRING sName)
{
    int i;
    for(i =0;i<(sizeof(OMX_CONF_EventLookupTable)/sizeof(OMX_CONF_EVENTLOOKUPTYPE));i++){
        if (OMX_CONF_EventLookupTable[i].eEvent == eEvent){
            strcpy( sName, OMX_CONF_EventLookupTable[i].sName);
            return OMX_ErrorNone;
        }
    }
    return OMX_ErrorBadParameter;
}

/**********************************************************************
 * STATE TO STRING
 **********************************************************************/

typedef struct OMX_CONF_STATELOOKUPTYPE {
    OMX_STATETYPE eState;
    OMX_STRING sName;
} OMX_CONF_STATELOOKUPTYPE;

OMX_CONF_STATELOOKUPTYPE OMX_CONF_StateLookupTable[] =
{
    {OMX_StateInvalid,              "OMX_StateInvalid"},
    {OMX_StateLoaded,               "OMX_StateLoaded"},
    {OMX_StateIdle,                 "OMX_StateIdle"},
    {OMX_StateExecuting,            "OMX_StateExecuting"},
    {OMX_StatePause,                "OMX_StatePause"},
    {OMX_StateWaitForResources,     "OMX_StateWaitForResources"},
    {OMX_StateMax,                   "OMX_StateMax"}
};

OMX_ERRORTYPE OMX_CONF_StateToString( OMX_IN OMX_STATETYPE eState, OMX_OUT OMX_STRING sName)
{
    int i;
    for(i =0;i<(sizeof(OMX_CONF_StateLookupTable)/sizeof(OMX_CONF_STATELOOKUPTYPE));i++){
        if (OMX_CONF_StateLookupTable[i].eState == eState){
            strcpy( sName, OMX_CONF_StateLookupTable[i].sName);
            return OMX_ErrorNone;
        }
    }
    return OMX_ErrorBadParameter;
}


/**********************************************************************
 * ERROR TO STRING
 **********************************************************************/

typedef struct OMX_CONF_ERRORLOOKUPTYPE {
    OMX_ERRORTYPE eError;
    OMX_STRING sName;
} OMX_CONF_ERRORLOOKUPTYPE;

OMX_CONF_ERRORLOOKUPTYPE OMX_CONF_ErrorLookupTable[] =
{
    {OMX_ErrorNone,                                  "OMX_ErrorNone"},
    {OMX_ErrorInsufficientResources,                 "OMX_ErrorInsufficientResources"},
    {OMX_ErrorUndefined,                             "OMX_ErrorUndefined"},
    {OMX_ErrorInvalidComponentName,                  "OMX_ErrorInvalidComponentName"},
    {OMX_ErrorComponentNotFound,                     "OMX_ErrorComponentNotFound"},
    {OMX_ErrorInvalidComponent,                      "OMX_ErrorInvalidComponent"},
    {OMX_ErrorBadParameter,                          "OMX_ErrorBadParameter"},
    {OMX_ErrorNotImplemented,                        "OMX_ErrorNotImplemented"},
    {OMX_ErrorUnderflow,                             "OMX_ErrorUnderflow"},
    {OMX_ErrorOverflow,                              "OMX_ErrorOverflow"},
    {OMX_ErrorHardware,                              "OMX_ErrorHardware"},
    {OMX_ErrorInvalidState,                          "OMX_ErrorInvalidState"},
    {OMX_ErrorStreamCorrupt,                         "OMX_ErrorStreamCorrupt"},
    {OMX_ErrorPortsNotCompatible,                    "OMX_ErrorPortsNotCompatible"},
    {OMX_ErrorResourcesLost,                         "OMX_ErrorResourcesLost"},
    {OMX_ErrorNoMore,                                "OMX_ErrorNoMore"},
    {OMX_ErrorVersionMismatch,                       "OMX_ErrorVersionMismatch"},
    {OMX_ErrorNotReady,                              "OMX_ErrorNotReady"},
    {OMX_ErrorTimeout,                               "OMX_ErrorTimeout"},
    {OMX_ErrorSameState,                             "OMX_ErrorSameState"},
    {OMX_ErrorResourcesPreempted,                    "OMX_ErrorResourcesPreempted"},
    {OMX_ErrorPortUnresponsiveDuringAllocation,      "OMX_ErrorPortUnresponsiveDuringAllocation"},
    {OMX_ErrorPortUnresponsiveDuringDeallocation,    "OMX_ErrorPortUnresponsiveDuringDeallocation"},
    {OMX_ErrorPortUnresponsiveDuringStop,            "OMX_ErrorPortUnresponsiveDuringStop"},
    {OMX_ErrorIncorrectStateTransition,              "OMX_ErrorIncorrectStateTransition"},
    {OMX_ErrorIncorrectStateOperation,               "OMX_ErrorIncorrectStateOperation"},
    {OMX_ErrorUnsupportedSetting,                    "OMX_ErrorUnsupportedSetting"},
    {OMX_ErrorUnsupportedIndex,                      "OMX_ErrorUnsupportedIndex"},
    {OMX_ErrorBadPortIndex,                          "OMX_ErrorBadPortIndex"},
    {OMX_ErrorPortUnpopulated,                       "OMX_ErrorPortUnpopulated"}, 
    {OMX_ErrorComponentSuspended,                    "OMX_ErrorComponentSuspended"},
    {OMX_ErrorDynamicResourcesUnavailable,           "OMX_ErrorDynamicResourcesUnavailable"},
    {OMX_ErrorMbErrorsInFrame,                       "OMX_ErrorMbErrorsInFrame"},
    {OMX_ErrorFormatNotDetected,                     "OMX_ErrorFormatNotDetected"},
    {OMX_ErrorContentPipeOpenFailed,                 "OMX_ErrorContentPipeOpenFailed"},
    {OMX_ErrorContentPipeCreationFailed,             "OMX_ErrorContentPipeCreationFailed"},
    {OMX_ErrorMax,                                   "OMX_ErrorMax"}
};

OMX_ERRORTYPE OMX_CONF_ErrorToString( OMX_IN OMX_ERRORTYPE eError, OMX_OUT OMX_STRING sName)
{
    int i;
    for(i =0;i<(sizeof(OMX_CONF_ErrorLookupTable)/sizeof(OMX_CONF_ERRORLOOKUPTYPE));i++){
        if (OMX_CONF_ErrorLookupTable[i].eError == eError){
            strcpy( sName, OMX_CONF_ErrorLookupTable[i].sName);
            return OMX_ErrorNone;
        }
    }
    strcpy( sName, "(vendor OMX_ERRORTYPE value?)");
    return OMX_ErrorBadParameter;
}

#define OMX_CONF_SHORT_TIMELIMIT 5
#define OMX_CONF_LONG_TIMELIMIT 20

void OMX_OSAL_TraceResultAndTime(OMX_ERRORTYPE eError, OMX_U32 nMsec, OMX_U32 nMsecLimit, OMX_STRING sComp, OMX_STRING sFunc)
{
    char sErrorName[256];
    if (OMX_ErrorNone == OMX_CONF_ErrorToString(eError, sErrorName)){
        OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, ") returned %s, took %i ms\n", sErrorName, nMsec);
    } else {
        OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, ") returned 0x%08x, took %i ms\n", eError, nMsec);
    }

    if (nMsec > nMsecLimit && strncmp("OMX.CONF.", sComp, 9) != 0) {
        OMX_OSAL_Trace(OMX_OSAL_TRACE_WARNING, "Call execution of %s on %s took longer than limit: Limit=%i ms Actual=%i ms\n",
            sFunc, sComp, nMsecLimit, nMsec );
    }
}

/**********************************************************************
 * INDEX TO STRING
 **********************************************************************/

typedef struct OMX_CONF_INDEXLOOKUPTYPE {
    OMX_INDEXTYPE nIndex;
    OMX_STRING sName;
} OMX_CONF_INDEXLOOKUPTYPE;

OMX_CONF_INDEXLOOKUPTYPE OMX_CONF_IndexLookupTable[] =
{
    {OMX_IndexComponentStartUnused,"OMX_IndexComponentStartUnused"},
    {OMX_IndexParamCompBufferSupplier,"OMX_IndexParamCompBufferSupplier"},
    {OMX_IndexReservedStartUnused,"OMX_IndexReservedStartUnused"},
    {OMX_IndexParamPortDefinition,"OMX_IndexParamPortDefinition"},
    {OMX_IndexReservedStartUnused,"OMX_IndexReservedStartUnused"},
    {OMX_IndexParamAudioPortFormat,"OMX_IndexParamAudioPortFormat"},
    {OMX_IndexParamAudioPcm,"OMX_IndexParamAudioPcm"},
    {OMX_IndexParamAudioAac,"OMX_IndexParamAudioAac"},
    {OMX_IndexParamAudioRa,"OMX_IndexParamAudioRa"},
    {OMX_IndexParamAudioMp3,"OMX_IndexParamAudioMp3"},
    {OMX_IndexParamAudioAdpcm,"OMX_IndexParamAudioAdpcm"},
    {OMX_IndexParamAudioG723,"OMX_IndexParamAudioG723"},
    {OMX_IndexParamAudioG729,"OMX_IndexParamAudioG729"},
    {OMX_IndexParamAudioAmr,"OMX_IndexParamAudioAmr"},
    {OMX_IndexParamAudioWma,"OMX_IndexParamAudioWma"},
    {OMX_IndexParamAudioSbc,"OMX_IndexParamAudioSbc"},
    {OMX_IndexParamAudioMidi,"OMX_IndexParamAudioMidi"},
    {OMX_IndexParamAudioGsm_FR,"OMX_IndexParamAudioGsm_FR"},
    {OMX_IndexParamAudioMidiLoadUserSound,"OMX_IndexParamAudioMidiLoadUserSound"},
    {OMX_IndexParamAudioG726,"OMX_IndexParamAudioG726"},
    {OMX_IndexParamAudioGsm_EFR,"OMX_IndexParamAudioGsm_EFR"},
    {OMX_IndexParamAudioGsm_HR,"OMX_IndexParamAudioGsm_HR"},
    {OMX_IndexParamAudioPdc_FR,"OMX_IndexParamAudioPdc_FR"},
    {OMX_IndexParamAudioPdc_EFR,"OMX_IndexParamAudioPdc_EFR"},
    {OMX_IndexParamAudioPdc_HR,"OMX_IndexParamAudioPdc_HR"},
    {OMX_IndexParamAudioTdma_FR,"OMX_IndexParamAudioTdma_FR"},
    {OMX_IndexParamAudioTdma_EFR,"OMX_IndexParamAudioTdma_EFR"},
    {OMX_IndexParamAudioQcelp8,"OMX_IndexParamAudioQcelp8"},
    {OMX_IndexParamAudioQcelp13,"OMX_IndexParamAudioQcelp13"},
    {OMX_IndexParamAudioEvrc,"OMX_IndexParamAudioEvrc"},
    {OMX_IndexParamAudioSmv,"OMX_IndexParamAudioSmv"},
    {OMX_IndexConfigAudioMidiImmediateEvent,"OMX_IndexConfigAudioMidiImmediateEvent"},
    {OMX_IndexConfigAudioMidiControl,"OMX_IndexConfigAudioMidiControl"},
    {OMX_IndexConfigAudioMidiSoundBankProgram,"OMX_IndexConfigAudioMidiSoundBankProgram"},
    {OMX_IndexConfigAudioMidiStatus,"OMX_IndexConfigAudioMidiStatus"},
    {OMX_IndexConfigAudioMidiMetaEvent,"OMX_IndexConfigAudioMidiMetaEvent"},
    {OMX_IndexConfigAudioMidiMetaEventData,"OMX_IndexConfigAudioMidiMetaEventData"},
    {OMX_IndexParamAudioInit,"OMX_IndexParamAudioInit"},
    {OMX_IndexConfigAudioVolume,"OMX_IndexConfigAudioVolume"},
    {OMX_IndexConfigAudioBalance,"OMX_IndexConfigAudioBalance"},
    {OMX_IndexConfigAudioChannelMute,"OMX_IndexConfigAudioChannelMute"},
    {OMX_IndexConfigAudioMute,"OMX_IndexConfigAudioMute"},

    {OMX_IndexConfigAudioLoudness,"OMX_IndexConfigAudioLoudness"},
    {OMX_IndexConfigAudioEchoCancelation,"OMX_IndexConfigAudioEchoCancelation"},
    {OMX_IndexConfigAudioNoiseReduction,"OMX_IndexConfigAudioNoiseReduction"},
    {OMX_IndexConfigAudioBass,"OMX_IndexConfigAudioBass"},
    {OMX_IndexConfigAudioTreble,"OMX_IndexConfigAudioTreble"},
    {OMX_IndexConfigAudioStereoWidening,"OMX_IndexConfigAudioStereoWidening"},
    {OMX_IndexConfigAudioChorus,"OMX_IndexConfigAudioChorus"},
    {OMX_IndexConfigAudioEqualizer,"OMX_IndexConfigAudioEqualizer"},
    {OMX_IndexConfigAudioReverberation,"OMX_IndexConfigAudioReverberation"},

    {OMX_IndexImageStartUnused,"OMX_IndexImageStartUnused"},
    {OMX_IndexParamImagePortFormat,"OMX_IndexParamImagePortFormat"},
    {OMX_IndexParamImageInit,"OMX_IndexParamImageInit"},
    {OMX_IndexParamFlashControl,"OMX_IndexParamFlashControl"},
    {OMX_IndexConfigFocusControl,"OMX_IndexConfigFocusControl"},
    {OMX_IndexParamQFactor,"OMX_IndexParamQFactor"},
    {OMX_IndexParamQuantizationTable,"OMX_IndexParamQuantizationTable"},

    {OMX_IndexParamHuffmanTable,"OMX_IndexParamHuffmanTable"},

    {OMX_IndexVideoStartUnused,"OMX_IndexVideoStartUnused"},
    {OMX_IndexParamVideoPortFormat,"OMX_IndexParamVideoPortFormat"},
    {OMX_IndexParamVideoInit,"OMX_IndexParamVideoInit"},
    {OMX_IndexParamVideoQuantization,"OMX_IndexParamVideoQuantization"},
    {OMX_IndexParamVideoFastUpdate,"OMX_IndexParamVideoFastUpdate"},
    {OMX_IndexParamVideoBitrate,"OMX_IndexParamVideoBitrate"},
    {OMX_IndexParamVideoMotionVector,"OMX_IndexParamVideoMotionVector"},
    {OMX_IndexParamVideoIntraRefresh,"OMX_IndexParamVideoIntraRefresh"},
    {OMX_IndexParamVideoErrorCorrection,"OMX_IndexParamVideoErrorCorrection"},
    {OMX_IndexParamVideoVBSMC,"OMX_IndexParamVideoVBSMC"},

    {OMX_IndexParamVideoMpeg2,"OMX_IndexParamVideoMpeg2"},
    {OMX_IndexParamVideoMpeg4,"OMX_IndexParamVideoMpeg4"},
    {OMX_IndexParamVideoWmv,"OMX_IndexParamVideoWmv"},
    {OMX_IndexParamVideoRv,"OMX_IndexParamVideoRv"},
    {OMX_IndexParamVideoAvc,"OMX_IndexParamVideoAvc"},
    {OMX_IndexParamVideoH263,"OMX_IndexParamVideoH263"},
  
    {OMX_IndexCommonStartUnused,"OMX_IndexCommonStartUnused"},
    {OMX_IndexParamCommonDeblocking,"OMX_IndexParamCommonDeblocking"},
    {OMX_IndexParamCommonSensorMode,"OMX_IndexParamCommonSensorMode"},
    {OMX_IndexConfigCommonColorFormatConversion,"OMX_IndexConfigCommonColorFormatConversion"},
    {OMX_IndexConfigCommonScale,"OMX_IndexConfigCommonScale"},
    {OMX_IndexConfigCommonImageFilter,"OMX_IndexConfigCommonImageFilter"},
    {OMX_IndexConfigCommonColorEnhancement,"OMX_IndexConfigCommonColorEnhancement"},
    {OMX_IndexConfigCommonColorKey,"OMX_IndexConfigCommonColorKey"},
    {OMX_IndexConfigCommonColorBlend,"OMX_IndexConfigCommonColorBlend"},
    {OMX_IndexConfigCommonFrameStabilisation,"OMX_IndexConfigCommonFrameStabilisation"},
    {OMX_IndexConfigCommonRotate,"OMX_IndexConfigCommonRotate"},
    {OMX_IndexConfigCommonMirror,"OMX_IndexConfigCommonMirror"},
    {OMX_IndexConfigCommonOutputPosition,"OMX_IndexConfigCommonOutputPosition"},
    {OMX_IndexConfigCommonInputCrop,"OMX_IndexConfigCommonInputCrop"},
    {OMX_IndexConfigCommonOutputCrop,"OMX_IndexConfigCommonOutputCrop"},
    {OMX_IndexConfigCommonDigitalZoom,"OMX_IndexConfigCommonDigitalZoom"},
    {OMX_IndexConfigCommonOpticalZoom,"OMX_IndexConfigCommonOpticalZoom"},
    {OMX_IndexConfigCommonWhiteBalance,"OMX_IndexConfigCommonWhiteBalance"},
    {OMX_IndexConfigCommonExposure,"OMX_IndexConfigCommonExposure"},

    {OMX_IndexConfigCommonContrast,"OMX_IndexConfigCommonContrast"},
    {OMX_IndexConfigCommonBrightness,"OMX_IndexConfigCommonBrightness"},
    {OMX_IndexConfigCommonBacklight,"OMX_IndexConfigCommonBacklight"},
    {OMX_IndexConfigCommonGamma,"OMX_IndexConfigCommonGamma"},
    {OMX_IndexConfigCommonSaturation,"OMX_IndexConfigCommonSaturation"},
    {OMX_IndexConfigCommonLightness,"OMX_IndexConfigCommonLightness"},
    {OMX_IndexConfigCommonExclusionRect,"OMX_IndexConfigCommonExclusionRect"},
    {OMX_IndexConfigCommonDithering,"OMX_IndexConfigCommonDithering"},

    {OMX_IndexConfigCommonPlaneBlend,"OMX_IndexConfigCommonPlaneBlend"},
    {OMX_IndexParamCommonInterleave,"OMX_IndexParamCommonInterleave"},

    {OMX_IndexOtherStartUnused,"OMX_IndexOtherStartUnused"},
    {OMX_IndexConfigOtherPower,"OMX_IndexConfigOtherPower"},
    {OMX_IndexConfigOtherStats,"OMX_IndexConfigOtherStats"},
    {OMX_IndexParamOtherPortFormat,"OMX_IndexParamOtherPortFormat"},
    {OMX_IndexParamOtherInit,"OMX_IndexParamOtherInit"},

    {OMX_IndexParamPriorityMgmt,"OMX_IndexParamPriorityMgmt"},

    {OMX_IndexTimeStartUnused,"OMX_IndexTimeStartUnused"},
    {OMX_IndexConfigTimeScale,"OMX_IndexConfigTimeScale"},
    {OMX_IndexConfigTimeClockState,"OMX_IndexConfigTimeClockState"},
    {OMX_IndexConfigTimeActiveRefClock,"OMX_IndexConfigTimeActiveRefClock"},
    {OMX_IndexConfigTimeCurrentMediaTime,"OMX_IndexConfigTimeCurrentMediaTime"},
    {OMX_IndexConfigTimeCurrentWallTime,"OMX_IndexConfigTimeCurrentWallTime"},
    {OMX_IndexConfigTimeCurrentAudioReference,"OMX_IndexConfigTimeCurrentAudioReference"},
    {OMX_IndexConfigTimeCurrentVideoReference,"OMX_IndexConfigTimeCurrentVideoReference"},
    {OMX_IndexConfigTimeMediaTimeRequest,"OMX_IndexConfigTimeMediaTimeRequest"},
    {OMX_IndexConfigTimeClientStartTime,"OMX_IndexConfigTimeClientStartTime"},
    {OMX_IndexConfigTimePosition,"OMX_IndexConfigTimePosition"},
    {OMX_IndexConfigTimeSeekMode,"OMX_IndexConfigTimeSeekMode"},

    {OMX_IndexParamNumAvailableStreams,"OMX_IndexParamNumAvailableStreams"},
    {OMX_IndexParamActiveStream,"OMX_IndexParamActiveStream"},
    {OMX_IndexParamSuspensionPolicy,"OMX_IndexParamSuspensionPolicy"},
    {OMX_IndexParamComponentSuspended,"OMX_IndexParamComponentSuspended"},
    {OMX_IndexConfigCapturing,"OMX_IndexConfigCapturing"},
    {OMX_IndexConfigCaptureMode,"OMX_IndexConfigCaptureMode"},
    {OMX_IndexAutoPauseAfterCapture,"OMX_IndexAutoPauseAfterCapture"},
    {OMX_IndexParamContentURI,"OMX_IndexParamContentURI"},
    {OMX_IndexParamCustomContentPipe,"OMX_IndexParamCustomContentPipe"},
    {OMX_IndexParamDisableResourceConcealment,"OMX_IndexParamDisableResourceConcealment"},
	{OMX_IndexConfigPriorityMgmt,"OMX_IndexConfigPriorityMgmt"},
    {OMX_IndexParamStandardComponentRole,"OMX_IndexParamStandardComponentRole"},
//    {OMX_IndexParamAudioMp3StreamFormat,"OMX_IndexParamAudioMp3StreamFormat"},
    {OMX_IndexConfigAudioChannelVolume,"OMX_IndexConfigAudioChannelVolume"},
    {OMX_IndexConfigFlashControl,"OMX_IndexConfigFlashControl"},
    {OMX_IndexParamVideoProfileLevelQuerySupported,"OMX_IndexParamVideoProfileLevelQuerySupported"},
    {OMX_IndexParamVideoProfileLevelCurrent,"OMX_IndexParamVideoProfileLevelCurrent"},
    {OMX_IndexConfigVideoBitrate,"OMX_IndexConfigVideoBitrate"},
    {OMX_IndexConfigVideoFramerate,"OMX_IndexConfigVideoFramerate"},
    {OMX_IndexConfigVideoIntraVOPRefresh,"OMX_IndexConfigVideoIntraVOPRefresh"},
    {OMX_IndexConfigVideoIntraMBRefresh,"OMX_IndexConfigVideoIntraMBRefresh"},
    {OMX_IndexConfigVideoMBErrorReporting,"OMX_IndexConfigVideoMBErrorReporting"},
    {OMX_IndexParamVideoMacroblocksPerFrame,"OMX_IndexParamVideoMacroblocksPerFrame"},
    {OMX_IndexConfigVideoMacroBlockErrorMap,"OMX_IndexConfigVideoMacroBlockErrorMap"},
    {OMX_IndexParamVideoSliceFMO,"OMX_IndexParamVideoSliceFMO"},
    {OMX_IndexConfigVideoAVCIntraPeriod,"OMX_IndexConfigVideoAVCIntraPeriod"},
    {OMX_IndexConfigVideoNalSize,"OMX_IndexConfigVideoNalSize"},
    {OMX_IndexConfigCommonExposureValue,"OMX_IndexConfigCommonExposureValue"},
    {OMX_IndexConfigCommonOutputSize,"OMX_IndexConfigCommonOutputSize"},
    {OMX_IndexParamCommonExtraQuantData,"OMX_IndexParamCommonExtraQuantData"},
    {OMX_IndexConfigCommonFocusRegion,"OMX_IndexConfigCommonFocusRegion"},
    {OMX_IndexConfigCommonFocusStatus,"OMX_IndexConfigCommonFocusStatus"},

    //{OMX_IndexIndexVendorStartUnused,"OMX_IndexIndexVendorStartUnused"},
    {OMX_IndexMax,"OMX_IndexMax"}
};

OMX_ERRORTYPE OMX_CONF_IndexToString( OMX_IN OMX_INDEXTYPE nIndex, OMX_OUT OMX_STRING sName)
{
    int i;
    for(i =0;i<(sizeof(OMX_CONF_IndexLookupTable)/sizeof(OMX_CONF_INDEXLOOKUPTYPE));i++){
        if (OMX_CONF_IndexLookupTable[i].nIndex == nIndex){
            strcpy( sName, OMX_CONF_IndexLookupTable[i].sName);
            return OMX_ErrorNone;
        }
    }
    return OMX_ErrorBadParameter;
}

OMX_ERRORTYPE OMX_CONF_TraceBuffer(OMX_BUFFERHEADERTYPE *pHeader)
{
    if (!pHeader) {
        return OMX_ErrorNone;
    }

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexLock(g_OMX_CONF_hTraceMutex);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_BUFFER, "\n\t\tnSize = 0x%08x",pHeader->nSize);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_BUFFER, "\n\t\tnVersion = 0x%08x",pHeader->nVersion.nVersion);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_BUFFER, "\n\t\tpBuffer = 0x%08x",pHeader->pBuffer);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_BUFFER, "\n\t\tnAllocLen = 0x%08x",pHeader->nAllocLen);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_BUFFER, "\n\t\tnFilledLen = 0x%08x",pHeader->nFilledLen);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_BUFFER, "\n\t\tnOffset = 0x%08x",pHeader->nOffset);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_BUFFER, "\n\t\tnOutputPortIndex = 0x%08x",pHeader->nOutputPortIndex);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_BUFFER, "\n\t\tnInputPortIndex = 0x%08x",pHeader->nInputPortIndex);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_BUFFER, "\n\t\tpOutputPortPrivate = 0x%08x",pHeader->pOutputPortPrivate);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_BUFFER, "\n\t\tpInputPortPrivate = 0x%08x",pHeader->pInputPortPrivate);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_BUFFER, "\n\t\tpAppPrivate = 0x%08x",pHeader->pAppPrivate);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_BUFFER, "\n\t\tpPlatformPrivate = 0x%08x",pHeader->pPlatformPrivate);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_BUFFER, "\n\t\thMarkTargetComponent = 0x%08x",pHeader->hMarkTargetComponent);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_BUFFER, "\n\t\tpMarkData = 0x%08x",pHeader->pMarkData);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_BUFFER, "\n\t\tnTickCount = 0x%08x",pHeader->nTickCount);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_BUFFER, "\n\t\tnTimeStamp = 0x%x",pHeader->nTimeStamp);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_BUFFER, "\n\t\tnFlags = 0x%08x",pHeader->nFlags);

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexUnlock(g_OMX_CONF_hTraceMutex);

    return OMX_ErrorNone;
}

/**************************************************************
 * COMPONENT TRACER - wraps an existing component structure
 * with functions that trace calls, parameters, and return
 * values.
 **************************************************************/

typedef struct OMX_CONF_COMPTRACERDATATYPE {
    OMX_PTR pOrigComponent;
    char sComponentName[256];
} OMX_CONF_COMPTRACERDATATYPE;

/* Wrapper functions */

OMX_ERRORTYPE Trace_GetComponentVersion(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_OUT OMX_STRING pComponentName,
        OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
        OMX_OUT OMX_VERSIONTYPE* pSpecVersion,
        OMX_OUT OMX_UUIDTYPE* pComponentUUID)
{
    OMX_ERRORTYPE eError;
    OMX_COMPONENTTYPE *pComp;
    OMX_U32 nMsec;
    OMX_STRING sCompName = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->sComponentName;

    pComp = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->pOrigComponent;
    nMsec = OMX_OSAL_GetTime();
    eError = pComp->GetComponentVersion((OMX_HANDLETYPE)pComp, pComponentName, pComponentVersion, pSpecVersion, pComponentUUID);
    nMsec = OMX_OSAL_GetTime() - nMsec;

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexLock(g_OMX_CONF_hTraceMutex);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "%s<- GetComponentVersion(", sCompName);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\thComponent = 0x%08x", hComponent);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpComponentName = %s", pComponentName);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpComponentVersion = %s", pComponentVersion);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpSpecVersion = %s", pSpecVersion);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpComponentUUID = %s", pComponentUUID);
    OMX_OSAL_TraceResultAndTime(eError,nMsec,OMX_CONF_SHORT_TIMELIMIT, sCompName, "GetComponentVersion");

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexUnlock(g_OMX_CONF_hTraceMutex);

    return eError;
}

OMX_ERRORTYPE Trace_SendCommand(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_IN  OMX_COMMANDTYPE Cmd,
        OMX_IN  OMX_U32 nParam1,
        OMX_IN  OMX_PTR pCmdData)
{
    OMX_ERRORTYPE eError;
    OMX_COMPONENTTYPE *pComp;
    char sCmdName[256];
    char sStateName[256];
    OMX_U32 nMsec;
    OMX_MARKTYPE *pMark;
    OMX_STRING sCompName = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->sComponentName;

    pComp = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->pOrigComponent;
    nMsec = OMX_OSAL_GetTime();

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexLock(g_OMX_CONF_hTraceMutex);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "calling %s<- SendCommand(", sCompName);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\thComponent = 0x%08x", hComponent);
    if (OMX_ErrorNone == OMX_CONF_CommandToString(Cmd,sCmdName)){
        OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "\n\tCmd = %s", sCmdName);
    } else {
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tCmd = 0x%08x", Cmd);
    }
    switch(Cmd){
    case OMX_CommandStateSet:
        if (OMX_ErrorNone == OMX_CONF_StateToString((OMX_COMMANDTYPE)nParam1, sStateName)){
            OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "\n\tstate(nParam1) = %s", sStateName);
        }else {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tstate(nParam1) = 0x%08x", nParam1);
        }
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpCmdData = 0x%08x", pCmdData);
        break;
    case OMX_CommandFlush:
    case OMX_CommandPortDisable:
    case OMX_CommandPortEnable:
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnPortIndex(nParam1) = 0x%08x", nParam1);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpCmdData = 0x%08x", pCmdData);
        break;
    case OMX_CommandMarkBuffer:
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnParam1 = 0x%08x", nParam1);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpMarkData(pCmdData) = 0x%08x", pCmdData);
        pMark = (OMX_MARKTYPE *)pCmdData;
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\t\thMarkTargetComponent = 0x%08x", pMark->hMarkTargetComponent);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\t\tpMarkData = 0x%08x", pMark->pMarkData);
        break;
    default:
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnParam1 = 0x%08x", nParam1);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpCmdData = 0x%08x", pCmdData);
    }
    OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "\n)....\n");
    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexUnlock(g_OMX_CONF_hTraceMutex);

    eError = pComp->SendCommand((OMX_HANDLETYPE)pComp, Cmd, nParam1, pCmdData);
    nMsec = OMX_OSAL_GetTime() - nMsec;

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexLock(g_OMX_CONF_hTraceMutex);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "%s<- SendCommand(", sCompName);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\thComponent = 0x%08x", hComponent);
    if (OMX_ErrorNone == OMX_CONF_CommandToString(Cmd,sCmdName)){
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tCmd = %s", sCmdName);
    } else {
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tCmd = 0x%08x", Cmd);
    }

    switch(Cmd){
    case OMX_CommandStateSet:
        if (OMX_ErrorNone == OMX_CONF_StateToString((OMX_COMMANDTYPE)nParam1, sStateName)){
            OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tstate(nParam1) = %s", sStateName);
        }else {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tstate(nParam1) = 0x%08x", nParam1);
        }
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpCmdData = 0x%08x", pCmdData);
        break;
    case OMX_CommandFlush:
    case OMX_CommandPortDisable:
    case OMX_CommandPortEnable:
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnPortIndex(nParam1) = 0x%08x", nParam1);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpCmdData = 0x%08x", pCmdData);
        break;
    case OMX_CommandMarkBuffer:
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnParam1 = 0x%08x", nParam1);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpMarkData(pCmdData) = 0x%08x", pCmdData);
        pMark = (OMX_MARKTYPE *)pCmdData;
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\t\thMarkTargetComponent = 0x%08x", pMark->hMarkTargetComponent);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\t\tpMarkData = 0x%08x", pMark->pMarkData);
        break;
    default:
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnParam1 = 0x%08x", nParam1);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpCmdData = 0x%08x", pCmdData);
    }
    OMX_OSAL_TraceResultAndTime(eError,nMsec,OMX_CONF_SHORT_TIMELIMIT, sCompName, "SendCommand");

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexUnlock(g_OMX_CONF_hTraceMutex);

    return eError;
}


OMX_ERRORTYPE Trace_GetParameter(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_IN  OMX_INDEXTYPE nParamIndex,
        OMX_INOUT OMX_PTR ComponentParameterStructure)
{
    OMX_ERRORTYPE eError;
    OMX_COMPONENTTYPE *pComp;
    char sIndexName[256];
    OMX_U32 nMsec;
    OMX_STRING sCompName = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->sComponentName;

    pComp = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->pOrigComponent;
    nMsec = OMX_OSAL_GetTime();
    eError = pComp->GetParameter((OMX_HANDLETYPE)pComp, nParamIndex, ComponentParameterStructure);
    nMsec = OMX_OSAL_GetTime() - nMsec;

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexLock(g_OMX_CONF_hTraceMutex);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "%s<- GetParameter(", sCompName);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\thComponent = 0x%08x", hComponent);
    if (OMX_ErrorNone == OMX_CONF_IndexToString(nParamIndex, sIndexName)){
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnParamIndex = %s", sIndexName);
    } else {
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnParamIndex = 0x%08x", nParamIndex);
    }
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tComponentParameterStructure = 0x%08x", ComponentParameterStructure);
    OMX_OSAL_TraceResultAndTime(eError,nMsec,OMX_CONF_LONG_TIMELIMIT, sCompName, "GetParameter");

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexUnlock(g_OMX_CONF_hTraceMutex);

    return eError;
}


OMX_ERRORTYPE Trace_SetParameter(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_IN  OMX_INDEXTYPE nIndex,
        OMX_IN  OMX_PTR ComponentParameterStructure)
{
    OMX_ERRORTYPE eError;
    OMX_COMPONENTTYPE *pComp;
    char sIndexName[256];
    OMX_U32 nMsec;
    OMX_STRING sCompName = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->sComponentName;

    pComp = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->pOrigComponent;
    nMsec = OMX_OSAL_GetTime();
    eError = pComp->SetParameter((OMX_HANDLETYPE)pComp, nIndex, ComponentParameterStructure);
    nMsec = OMX_OSAL_GetTime() - nMsec;

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexLock(g_OMX_CONF_hTraceMutex);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "%s<- SetParameter(", sCompName);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\thComponent = 0x%08x", hComponent);
    if (OMX_ErrorNone == OMX_CONF_IndexToString(nIndex, sIndexName)){
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnIndex = %s", sIndexName);
    } else {
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnIndex = 0x%08x", nIndex);
    }
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tComponentParameterStructure = 0x%08x", ComponentParameterStructure);
    OMX_OSAL_TraceResultAndTime(eError,nMsec,OMX_CONF_LONG_TIMELIMIT, sCompName, "SetParameter");

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexUnlock(g_OMX_CONF_hTraceMutex);

    return eError;
}


OMX_ERRORTYPE Trace_GetConfig(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_IN  OMX_INDEXTYPE nIndex,
        OMX_INOUT OMX_PTR pComponentConfigStructure)
{
    OMX_ERRORTYPE eError;
    OMX_COMPONENTTYPE *pComp;
    char sIndexName[256];
    OMX_U32 nMsec;
    OMX_STRING sCompName = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->sComponentName;

    pComp = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->pOrigComponent;
    nMsec = OMX_OSAL_GetTime();
    eError = pComp->GetConfig((OMX_HANDLETYPE)pComp, nIndex, pComponentConfigStructure);
    nMsec = OMX_OSAL_GetTime() - nMsec;

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexLock(g_OMX_CONF_hTraceMutex);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "%s<- GetConfig(", sCompName);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\thComponent = 0x%08x", hComponent);
    if (OMX_ErrorNone == OMX_CONF_IndexToString(nIndex, sIndexName)){
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnIndex = %s", sIndexName);
    } else {
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnIndex = 0x%08x", nIndex);
    }
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpComponentConfigStructure = 0x%08x", pComponentConfigStructure);
    OMX_OSAL_TraceResultAndTime(eError,nMsec,OMX_CONF_SHORT_TIMELIMIT, sCompName, "GetConfig");

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexUnlock(g_OMX_CONF_hTraceMutex);

    return eError;
}


OMX_ERRORTYPE Trace_SetConfig(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_IN  OMX_INDEXTYPE nIndex,
        OMX_IN  OMX_PTR pComponentConfigStructure)
{
    OMX_ERRORTYPE eError;
    OMX_COMPONENTTYPE *pComp;
    char sIndexName[256];
    OMX_U32 nMsec;
    OMX_STRING sCompName = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->sComponentName;

    pComp = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->pOrigComponent;
    nMsec = OMX_OSAL_GetTime();
    eError = pComp->SetConfig((OMX_HANDLETYPE)pComp, nIndex, pComponentConfigStructure);
    nMsec = OMX_OSAL_GetTime() - nMsec;

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexLock(g_OMX_CONF_hTraceMutex);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "%s<- SetConfig(", sCompName);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\thComponent = 0x%08x", hComponent);
    if (OMX_ErrorNone == OMX_CONF_IndexToString(nIndex, sIndexName)){
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnIndex = %s", sIndexName);
    } else {
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnIndex = 0x%08x", nIndex);
    }
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpComponentConfigStructure = 0x%08x", pComponentConfigStructure);
    OMX_OSAL_TraceResultAndTime(eError,nMsec,OMX_CONF_SHORT_TIMELIMIT, sCompName, "SetConfig");

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexUnlock(g_OMX_CONF_hTraceMutex);

    return eError;
}


OMX_ERRORTYPE Trace_GetExtensionIndex(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_IN  OMX_STRING cParameterName,
        OMX_OUT OMX_INDEXTYPE* pIndexType)
{
    OMX_ERRORTYPE eError;
    OMX_COMPONENTTYPE *pComp;
    OMX_U32 nMsec;
    OMX_STRING sCompName = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->sComponentName;

    pComp = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->pOrigComponent;
    nMsec = OMX_OSAL_GetTime();
    eError = pComp->GetExtensionIndex((OMX_HANDLETYPE)pComp, cParameterName, pIndexType);
    nMsec = OMX_OSAL_GetTime() - nMsec;

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexLock(g_OMX_CONF_hTraceMutex);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "%s<- GetExtensionIndex(", sCompName);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\thComponent = 0x%08x", hComponent);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tcParameterName = %s", cParameterName);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpIndexType = 0x%08x", pIndexType);
    OMX_OSAL_TraceResultAndTime(eError,nMsec,OMX_CONF_SHORT_TIMELIMIT, sCompName, "GetExtensionIndex");

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexUnlock(g_OMX_CONF_hTraceMutex);

    return eError;
}


OMX_ERRORTYPE Trace_GetState(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_OUT OMX_STATETYPE* pState)
{
    OMX_ERRORTYPE eError;
    OMX_COMPONENTTYPE *pComp;
    char sStateName[256];
    OMX_U32 nMsec;
    OMX_STRING sCompName = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->sComponentName;

    pComp = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->pOrigComponent;
    nMsec = OMX_OSAL_GetTime();
    eError = pComp->GetState((OMX_HANDLETYPE)pComp, pState);
    nMsec = OMX_OSAL_GetTime() - nMsec;

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexLock(g_OMX_CONF_hTraceMutex);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "%s<- GetState(", sCompName);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\thComponent = 0x%08x", hComponent);
    if (OMX_ErrorNone == OMX_CONF_StateToString(*pState, sStateName)){
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\t*pState = %s", sStateName);
    }else {
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\t*pState = 0x%08x", pState);
    }
    OMX_OSAL_TraceResultAndTime(eError,nMsec,OMX_CONF_SHORT_TIMELIMIT, sCompName, "GetState");

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexUnlock(g_OMX_CONF_hTraceMutex);

    return eError;
}


OMX_ERRORTYPE Trace_ComponentTunnelRequest(
    OMX_IN  OMX_HANDLETYPE hComp,
    OMX_IN  OMX_U32 nPort,
    OMX_IN  OMX_HANDLETYPE hTunneledComp,
    OMX_IN  OMX_U32 nTunneledPort,
    OMX_INOUT  OMX_TUNNELSETUPTYPE* pTunnelSetup)
{
    OMX_ERRORTYPE eError;
    OMX_COMPONENTTYPE *pComp;
    OMX_U32 nMsec;
    OMX_STRING sCompName = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComp)->pApplicationPrivate))->sComponentName;

    pComp = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComp)->pApplicationPrivate))->pOrigComponent;
    nMsec = OMX_OSAL_GetTime();
    eError = pComp->ComponentTunnelRequest((OMX_HANDLETYPE)pComp, nPort, hTunneledComp, nTunneledPort, pTunnelSetup);
    nMsec = OMX_OSAL_GetTime() - nMsec;

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexLock(g_OMX_CONF_hTraceMutex);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "%s<- ComponentTunnelRequest(",
        ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComp)->pApplicationPrivate))->sComponentName);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\thComp = 0x%08x", hComp);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnPort = 0x%08x", nPort);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\thTunneledComp = 0x%08x", hTunneledComp);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnTunneledPort = 0x%08x", nTunneledPort);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpTunnelSetup = 0x%08x", pTunnelSetup);
    OMX_OSAL_TraceResultAndTime(eError,nMsec,OMX_CONF_SHORT_TIMELIMIT, sCompName, "ComponentTunnelRequest");

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexUnlock(g_OMX_CONF_hTraceMutex);

    return eError;
}


OMX_ERRORTYPE Trace_UseBuffer(
        OMX_IN OMX_HANDLETYPE hComponent,
        OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
        OMX_IN OMX_U32 nPortIndex,
        OMX_IN OMX_PTR pAppPrivate,
        OMX_IN OMX_U32 nSizeBytes,
        OMX_IN OMX_U8* pBuffer)
{
    OMX_ERRORTYPE eError;
    OMX_COMPONENTTYPE *pComp;
    OMX_U32 nMsec;
    OMX_STRING sCompName = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->sComponentName;

    pComp = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->pOrigComponent;
    nMsec = OMX_OSAL_GetTime();
    eError = pComp->UseBuffer((OMX_HANDLETYPE)pComp, ppBufferHdr, nPortIndex, pAppPrivate, nSizeBytes, pBuffer);
    nMsec = OMX_OSAL_GetTime() - nMsec;

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexLock(g_OMX_CONF_hTraceMutex);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "%s<- UseBuffer(", sCompName);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\thComponent = 0x%08x", hComponent);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tppBufferHdr = 0x%08x", ppBufferHdr);
    if (OMX_ErrorNone == eError) OMX_CONF_TraceBuffer(*ppBufferHdr);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnPortIndex = 0x%08x", nPortIndex);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpAppPrivate = 0x%08x", pAppPrivate);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnSizeBytes = 0x%08x", nSizeBytes);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpBuffer = 0x%08x", pBuffer);
    OMX_OSAL_TraceResultAndTime(eError,nMsec,OMX_CONF_LONG_TIMELIMIT, sCompName, "UseBuffer");

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexUnlock(g_OMX_CONF_hTraceMutex);

    return eError;
}


OMX_ERRORTYPE Trace_AllocateBuffer(
        OMX_IN OMX_HANDLETYPE hComponent,
        OMX_INOUT OMX_BUFFERHEADERTYPE** pBuffer,
        OMX_IN OMX_U32 nPortIndex,
        OMX_IN OMX_PTR pAppPrivate,
        OMX_IN OMX_U32 nSizeBytes)
{
    OMX_ERRORTYPE eError;
    OMX_COMPONENTTYPE *pComp;
    OMX_U32 nMsec;
    OMX_STRING sCompName = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->sComponentName;

    pComp = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->pOrigComponent;
    nMsec = OMX_OSAL_GetTime();
    eError = pComp->AllocateBuffer((OMX_HANDLETYPE)pComp, pBuffer, nPortIndex, pAppPrivate, nSizeBytes);
    nMsec = OMX_OSAL_GetTime() - nMsec;

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexLock(g_OMX_CONF_hTraceMutex);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "%s<- AllocateBuffer(", sCompName);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\thComponent = 0x%08x", hComponent);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpBuffer = 0x%08x", pBuffer);
    if (OMX_ErrorNone == eError) OMX_CONF_TraceBuffer(*pBuffer);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnPortIndex = 0x%08x", nPortIndex);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpAppPrivate = 0x%08x", pAppPrivate);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnSizeBytes = 0x%08x", nSizeBytes);
    OMX_OSAL_TraceResultAndTime(eError,nMsec,OMX_CONF_SHORT_TIMELIMIT, sCompName, "AllocateBuffer");

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexUnlock(g_OMX_CONF_hTraceMutex);

    return eError;
}


OMX_ERRORTYPE Trace_FreeBuffer(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_IN OMX_U32 nPortIndex,
        OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{
    OMX_ERRORTYPE eError;
    OMX_COMPONENTTYPE *pComp;
    OMX_U32 nMsec;
    OMX_STRING sCompName = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->sComponentName;

    /* do a trace buffer prior to freeing the buffer */
    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexLock(g_OMX_CONF_hTraceMutex);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "%s<- FreeBuffer(", sCompName);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\thComponent = 0x%08x", hComponent);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnPortIndex = 0x%08x", nPortIndex);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpBuffer = 0x%08x", pBuffer);
    OMX_CONF_TraceBuffer(pBuffer);

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexUnlock(g_OMX_CONF_hTraceMutex);

    pComp = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->pOrigComponent;
    nMsec = OMX_OSAL_GetTime();
    eError = pComp->FreeBuffer((OMX_HANDLETYPE)pComp, nPortIndex, pBuffer);
    nMsec = OMX_OSAL_GetTime() - nMsec;

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexLock(g_OMX_CONF_hTraceMutex);
    OMX_OSAL_TraceResultAndTime(eError,nMsec,OMX_CONF_LONG_TIMELIMIT, sCompName, "FreeBuffer");
    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexUnlock(g_OMX_CONF_hTraceMutex);

    return eError;
}


OMX_ERRORTYPE Trace_EmptyThisBuffer(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{
    OMX_ERRORTYPE eError;
    OMX_COMPONENTTYPE *pComp;
    OMX_U32 nMsec;
    OMX_STRING sCompName = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->sComponentName;

    pComp = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->pOrigComponent;
    nMsec = OMX_OSAL_GetTime();
    eError = pComp->EmptyThisBuffer((OMX_HANDLETYPE)pComp, pBuffer);
    nMsec = OMX_OSAL_GetTime() - nMsec;

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexLock(g_OMX_CONF_hTraceMutex);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "%s<- EmptyThisBuffer(", sCompName);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\thComponent = 0x%08x", hComponent);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpBuffer = 0x%08x", pBuffer);
    OMX_CONF_TraceBuffer(pBuffer);
    OMX_OSAL_TraceResultAndTime(eError,nMsec,OMX_CONF_SHORT_TIMELIMIT, sCompName, "EmptyThisBuffer");

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexUnlock(g_OMX_CONF_hTraceMutex);

    return eError;
}


OMX_ERRORTYPE Trace_FillThisBuffer(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_IN  OMX_BUFFERHEADERTYPE* pBuffer)
{
    OMX_ERRORTYPE eError;
    OMX_COMPONENTTYPE *pComp;
    OMX_U32 nMsec;
    OMX_STRING sCompName = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->sComponentName;

    pComp = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->pOrigComponent;
    nMsec = OMX_OSAL_GetTime();
    eError = pComp->FillThisBuffer((OMX_HANDLETYPE)pComp, pBuffer);
    nMsec = OMX_OSAL_GetTime() - nMsec;

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexLock(g_OMX_CONF_hTraceMutex);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "%s<- FillThisBuffer(", sCompName);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\thComponent = 0x%08x", hComponent);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpBuffer = 0x%08x", pBuffer);
    OMX_CONF_TraceBuffer(pBuffer);
    OMX_OSAL_TraceResultAndTime(eError,nMsec,OMX_CONF_SHORT_TIMELIMIT, sCompName, "FillThisBuffer");

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexUnlock(g_OMX_CONF_hTraceMutex);

    return eError;
}

OMX_ERRORTYPE Trace_SetCallbacks(
        OMX_IN  OMX_HANDLETYPE hComponent,
        OMX_IN  OMX_CALLBACKTYPE* pCallbacks,
        OMX_IN  OMX_PTR pAppData)
{
    OMX_ERRORTYPE eError;
    OMX_COMPONENTTYPE *pComp;
    OMX_U32 nMsec;
    OMX_STRING sCompName = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->sComponentName;

    pComp = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->pOrigComponent;
    nMsec = OMX_OSAL_GetTime();
    eError = pComp->SetCallbacks((OMX_HANDLETYPE)pComp, pCallbacks, pAppData);
    nMsec = OMX_OSAL_GetTime() - nMsec;

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexLock(g_OMX_CONF_hTraceMutex);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "%s<- SetCallbacks(", sCompName);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\thComponent = 0x%08x", hComponent);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpCallbacks = 0x%08x", pCallbacks);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpAppData = 0x%08x", pAppData);
    OMX_OSAL_TraceResultAndTime(eError,nMsec,OMX_CONF_SHORT_TIMELIMIT, sCompName, "SetCallbacks");

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexUnlock(g_OMX_CONF_hTraceMutex);

    return eError;
}

OMX_ERRORTYPE Trace_ComponentDeInit(
        OMX_IN  OMX_HANDLETYPE hComponent)
{
    OMX_ERRORTYPE eError;
    OMX_COMPONENTTYPE *pComp;
    OMX_U32 nMsec;
    OMX_STRING sCompName = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->sComponentName;

    pComp = ((OMX_CONF_COMPTRACERDATATYPE *)(((OMX_COMPONENTTYPE *)hComponent)->pApplicationPrivate))->pOrigComponent;
    nMsec = OMX_OSAL_GetTime();
    eError = (pComp->ComponentDeInit)((OMX_HANDLETYPE)pComp);
    nMsec = OMX_OSAL_GetTime() - nMsec;

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexLock(g_OMX_CONF_hTraceMutex);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "%s<- ComponentDeInit(", sCompName);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\thComponent = 0x%08x", hComponent);
    OMX_OSAL_TraceResultAndTime(eError,nMsec,OMX_CONF_LONG_TIMELIMIT, sCompName, "ComponentDeInit");

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexUnlock(g_OMX_CONF_hTraceMutex);

    return eError;
}

/* Component Tracer methods */

OMX_ERRORTYPE OMX_CONF_ComponentTracerCreate( OMX_IN OMX_HANDLETYPE hOrigComp,
                                              OMX_IN OMX_STRING sComponentName,
                                              OMX_OUT OMX_HANDLETYPE *phWrappedComp)
{
    OMX_CONF_COMPTRACERDATATYPE *pTracerData;
    OMX_COMPONENTTYPE *pOrigComp;
    OMX_COMPONENTTYPE *pWrappedComp;

    pOrigComp = (OMX_COMPONENTTYPE *)hOrigComp;

    /* Create wrapper */
    pWrappedComp = (OMX_COMPONENTTYPE *)OMX_OSAL_Malloc(sizeof(OMX_COMPONENTTYPE));

    /* Create tracer data structure. Store original component structure and component name. */
    pTracerData = (OMX_CONF_COMPTRACERDATATYPE *)OMX_OSAL_Malloc(sizeof(OMX_CONF_COMPTRACERDATATYPE));
    pTracerData->pOrigComponent = pOrigComp;
    strcpy(pTracerData->sComponentName, sComponentName);
    pWrappedComp->pApplicationPrivate = (OMX_PTR)pTracerData;

    /* Copy all */
    pWrappedComp->nSize = sizeof(OMX_COMPONENTTYPE);
    pWrappedComp->nVersion = pOrigComp->nVersion;
    pWrappedComp->pComponentPrivate = pOrigComp->pComponentPrivate;

    /* Set function pointers to wrapper functions */
    pWrappedComp->AllocateBuffer           = Trace_AllocateBuffer;
    pWrappedComp->ComponentTunnelRequest   = Trace_ComponentTunnelRequest;
    pWrappedComp->EmptyThisBuffer          = Trace_EmptyThisBuffer;
    pWrappedComp->FillThisBuffer           = Trace_FillThisBuffer;
    pWrappedComp->FreeBuffer               = Trace_FreeBuffer;
    pWrappedComp->GetComponentVersion      = Trace_GetComponentVersion;
    pWrappedComp->GetConfig                = Trace_GetConfig;
    pWrappedComp->GetExtensionIndex        = Trace_GetExtensionIndex;
    pWrappedComp->GetParameter             = Trace_GetParameter;
    pWrappedComp->GetState                 = Trace_GetState;
    pWrappedComp->ComponentDeInit          = Trace_ComponentDeInit;
    pWrappedComp->SendCommand              = Trace_SendCommand;
    pWrappedComp->SetCallbacks             = Trace_SetCallbacks;
    pWrappedComp->SetConfig                = Trace_SetConfig;
    pWrappedComp->SetParameter             = Trace_SetParameter;
    pWrappedComp->UseBuffer                = Trace_UseBuffer;

    *phWrappedComp = (OMX_HANDLETYPE)pWrappedComp;

    return OMX_ErrorNone;
}

OMX_ERRORTYPE OMX_CONF_ComponentTracerDestroy(OMX_IN OMX_HANDLETYPE hWrappedComp)
{
    OMX_COMPONENTTYPE *pWrappedComp;
    pWrappedComp = (OMX_COMPONENTTYPE *)hWrappedComp;
    if (hWrappedComp == NULL)
        return OMX_ErrorNone;

    OMX_OSAL_Free(pWrappedComp->pApplicationPrivate);
    OMX_OSAL_Free(pWrappedComp);
    return OMX_ErrorNone;
}

/**************************************************************
 * CALLBACK TRACER - wraps an existing callback structure
 * with functions that trace calls, parameters, and return
 * values.
 **************************************************************/

typedef struct OMX_CONF_APPDATAWRAPPERTYPE {
    OMX_PTR pOrigAppData;
    char sComponentName[256];
    OMX_CALLBACKTYPE *pOrigCallbacks;
} OMX_CONF_APPDATAWRAPPERTYPE;

/* wrapper functions */

OMX_ERRORTYPE Trace_EventHandler(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_PTR pAppData,
    OMX_IN OMX_EVENTTYPE eEvent,
    OMX_IN OMX_U32 nData1,
    OMX_IN OMX_U32 nData2,
    OMX_IN OMX_PTR pEventData)
{
    OMX_ERRORTYPE eError;
    OMX_CONF_APPDATAWRAPPERTYPE *pAppDataWrapper;
    OMX_U32 nMsec;
    char sEventName[256];
    char sCmdName[256];
    char sStateName[256];
    char sErrorName[256];
    OMX_STRING sCompName;

    pAppDataWrapper = (OMX_CONF_APPDATAWRAPPERTYPE *)pAppData;

    nMsec = OMX_OSAL_GetTime();
    eError = pAppDataWrapper->pOrigCallbacks->EventHandler(hComponent, pAppDataWrapper->pOrigAppData,
        eEvent, nData1, nData2, pEventData);
    nMsec = OMX_OSAL_GetTime() - nMsec;

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexLock(g_OMX_CONF_hTraceMutex);
    sCompName = pAppDataWrapper->sComponentName;
    OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "%s-> EventHandler(", pAppDataWrapper->sComponentName);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\thComponent = 0x%08x", hComponent);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpAppData = 0x%08x", pAppData);
    if (OMX_ErrorNone == OMX_CONF_EventToString(eEvent, sEventName)){
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\teEvent = %s", sEventName);
    }else {
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\teEvent = 0x%08x", eEvent);
    }
    switch(eEvent)
    {
    case OMX_EventCmdComplete:
        if (OMX_ErrorNone == OMX_CONF_CommandToString(nData1,sCmdName)){
            OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tCommand(nData1) = %s", sCmdName);
        } else {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tCommand(nData1) = 0x%08x", nData1);
        }
        switch ((OMX_COMMANDTYPE)nData1)
        {
        case OMX_CommandPortDisable:
        case OMX_CommandPortEnable:
        case OMX_CommandFlush:
            OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnPortIndex(nData2) = 0x%08x", nData2);
            break;
        case OMX_CommandStateSet:
            if (OMX_ErrorNone == OMX_CONF_StateToString((OMX_STATETYPE)nData2, sStateName)){
                OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tstate(nData2) = %s", sStateName);
            } else {
                OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tstate(nData2) = 0x%08x", nData2);
            }
        default:
            OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnData2 = 0x%08x", nData2);
        }
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpEventData = 0x%08x", pEventData);
        break;
    case OMX_EventError:
        if (OMX_ErrorNone == OMX_CONF_ErrorToString((OMX_ERRORTYPE)nData1, sErrorName)){
            OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "\n\terror(nData2) = %s", sErrorName);
        } else {
            OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "\n\terror(nData2) = 0x%08x", nData1);
        }
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnData2 = 0x%08x", nData2);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpEventData = 0x%08x", pEventData);
        break;
    case OMX_EventMark:
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnData1 = 0x%08x", nData1);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnData2 = 0x%08x", nData2);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpMarkData(pEventData) = 0x%08x", pEventData);
        break;
    case OMX_EventPortSettingsChanged:
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tPortIndex(nData1) = 0x%08x", nData1);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnData2 = 0x%08x", nData2);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpEventData = 0x%08x", pEventData);
        break;
    case OMX_EventBufferFlag:
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnPortIndex(nData1) = 0x%08x", nData1);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnFlags(nData2) = 0x%08x", nData2);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpEventData = 0x%08x", pEventData);
    default:
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnData1 = 0x%08x", nData1);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tnData2 = 0x%08x", nData2);
        OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpEventData = 0x%08x", pEventData);
    }
    OMX_OSAL_TraceResultAndTime(eError,nMsec,OMX_CONF_SHORT_TIMELIMIT, sCompName, "EventHandler");

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexUnlock(g_OMX_CONF_hTraceMutex);

    return eError;
}

OMX_ERRORTYPE Trace_EmptyBufferDone(
    OMX_OUT OMX_HANDLETYPE hComponent,
    OMX_OUT OMX_PTR pAppData,
    OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer)
{
    OMX_ERRORTYPE eError;
    OMX_CONF_APPDATAWRAPPERTYPE *pAppDataWrapper;
    OMX_U32 nMsec;
    OMX_STRING sCompName;

    pAppDataWrapper = (OMX_CONF_APPDATAWRAPPERTYPE *)pAppData;
    sCompName = pAppDataWrapper->sComponentName;

    nMsec = OMX_OSAL_GetTime();
    eError = pAppDataWrapper->pOrigCallbacks->EmptyBufferDone(hComponent, pAppDataWrapper->pOrigAppData, pBuffer);
    nMsec = OMX_OSAL_GetTime() - nMsec;

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexLock(g_OMX_CONF_hTraceMutex);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "%s-> EmptyBufferDone(", pAppDataWrapper->sComponentName);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\thComponent = 0x%08x", hComponent);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpAppData = 0x%08x", pAppData);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpBuffer = 0x%08x", pBuffer);
    OMX_CONF_TraceBuffer(pBuffer);
    OMX_OSAL_TraceResultAndTime(eError,nMsec,OMX_CONF_SHORT_TIMELIMIT, sCompName, "EmptyBufferDone");

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexUnlock(g_OMX_CONF_hTraceMutex);

    return eError;
}

OMX_ERRORTYPE Trace_FillBufferDone(
    OMX_OUT OMX_HANDLETYPE hComponent,
    OMX_OUT OMX_PTR pAppData,
    OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer)
{
    OMX_ERRORTYPE eError;
    OMX_CONF_APPDATAWRAPPERTYPE *pAppDataWrapper;
    OMX_U32 nMsec;
    OMX_STRING sCompName;

    pAppDataWrapper = (OMX_CONF_APPDATAWRAPPERTYPE *)pAppData;
    sCompName = pAppDataWrapper->sComponentName;

    nMsec = OMX_OSAL_GetTime();
    eError = pAppDataWrapper->pOrigCallbacks->FillBufferDone( hComponent, pAppDataWrapper->pOrigAppData, pBuffer);
    nMsec = OMX_OSAL_GetTime() - nMsec;

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexLock(g_OMX_CONF_hTraceMutex);

    OMX_OSAL_Trace(OMX_OSAL_TRACE_CALLSEQUENCE, "%s-> FillBufferDone(", pAppDataWrapper->sComponentName);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\thComponent = 0x%08x", hComponent);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpAppData = 0x%08x", pAppData);
    OMX_OSAL_Trace(OMX_OSAL_TRACE_PARAMETERS, "\n\tpBuffer = 0x%08x", pBuffer);
    OMX_CONF_TraceBuffer(pBuffer);
    OMX_OSAL_TraceResultAndTime(eError,nMsec,OMX_CONF_SHORT_TIMELIMIT, sCompName, "FillBufferDone");

    if (g_OMX_CONF_hTraceMutex) OMX_OSAL_MutexUnlock(g_OMX_CONF_hTraceMutex);

    return eError;
}

/* Callback tracer methods */

OMX_ERRORTYPE OMX_CONF_CallbackTracerCreate( OMX_IN OMX_CALLBACKTYPE *pOrigCallbacks,
                                             OMX_IN OMX_PTR pOrigAppData,
                                             OMX_IN OMX_STRING sComponentName,
                                             OMX_OUT OMX_CALLBACKTYPE **ppWrappedCallbacks,
                                             OMX_OUT OMX_PTR *ppWrappedAppData)
{
    /* Create callback wrapper */
    *ppWrappedCallbacks = (OMX_CALLBACKTYPE *)OMX_OSAL_Malloc(sizeof(OMX_CALLBACKTYPE));

    /* Set function pointers to wrapper functions */
    (*ppWrappedCallbacks)->EventHandler    = Trace_EventHandler;
    (*ppWrappedCallbacks)->FillBufferDone        = Trace_FillBufferDone;
    (*ppWrappedCallbacks)->EmptyBufferDone       = Trace_EmptyBufferDone;

    /* Create app data wrapper */
    *ppWrappedAppData = (OMX_PTR)OMX_OSAL_Malloc(sizeof(OMX_CONF_APPDATAWRAPPERTYPE));

    /* Store original values */
    ((OMX_CONF_APPDATAWRAPPERTYPE*)*ppWrappedAppData)->pOrigCallbacks = pOrigCallbacks;
    ((OMX_CONF_APPDATAWRAPPERTYPE*)*ppWrappedAppData)->pOrigAppData = pOrigAppData;
    strcpy(((OMX_CONF_APPDATAWRAPPERTYPE*)*ppWrappedAppData)->sComponentName, sComponentName);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE OMX_CONF_CallbackTracerDestroy(OMX_IN OMX_CALLBACKTYPE *pWrappedCallbacks,
                                             OMX_OUT OMX_PTR *pWrappedAppData)
{
   OMX_OSAL_Free(pWrappedCallbacks);
   OMX_OSAL_Free(pWrappedAppData);
   return OMX_ErrorNone;
}

OMX_ERRORTYPE OMX_OSAL_GetTracePrefix(OMX_IN OMX_U32 nTraceFlags, OMX_OUT OMX_STRING szPrefix)
{
    switch(nTraceFlags)
    {
    case OMX_OSAL_TRACE_PASSFAIL:
        szPrefix[0] = '*';
        szPrefix[1] = '*';
        szPrefix[2] = '*';
        szPrefix[3] = '\0';
        break;
    case OMX_OSAL_TRACE_CALLSEQUENCE:
        szPrefix[0] = '=';
        szPrefix[1] = '\0';
        break;
    case OMX_OSAL_TRACE_INFO:
        szPrefix[0] = '-';
        szPrefix[1] = '\0';
        break;
    case OMX_OSAL_TRACE_ERROR:
        szPrefix[0] = '!';
        szPrefix[1] = '!';
        szPrefix[2] = '!';
        szPrefix[3] = '\0';
        break;
    case OMX_OSAL_TRACE_WARNING:
        szPrefix[0] = '^';
        szPrefix[1] = '^';
        szPrefix[2] = '^';
        szPrefix[3] = '\0';
        break;
    case OMX_OSAL_TRACE_PARAMETERS:
    case OMX_OSAL_TRACE_BUFFER:
    default:
        szPrefix[0] = '\0';
    }

    return OMX_ErrorNone;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */

