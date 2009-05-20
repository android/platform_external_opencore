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

/** OMX_CONF_Tests.c
 *  Implemenation of the OpenMax IL conformance tests lookup table. 
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "OMX_OSAL_Interfaces.h"
#include "OMX_CONF_TestHarness.h"

/** Base Profile Tests */
OMX_ERRORTYPE OMX_CONF_StateTransitionTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_ComponentNameTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_ParameterTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_BufferTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_BufferFlagTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_FlushTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_BaseMultiThreadedTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_PortCommunicationTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_ResourceExhaustionTest(OMX_IN OMX_STRING cComponentName);

/** Interop Profile Tests */
OMX_ERRORTYPE OMX_CONF_ClockComponentTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_ParameterTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_MultiThreadedTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_PortBufferSupplierTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_PortDisableEnableTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_InvalidInputOutputTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_ValidInputOutputTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_IncompleteStopTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_ResourcePreemptionTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_WaitForResourcesTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_MinPayloadSizeTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_SeekingComponentTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_TunnelledUnitTest(OMX_IN OMX_STRING cComponentName);

/* OSAL Test Prototypes */
OMX_ERRORTYPE OMX_OSAL_TestAll(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_OSAL_MemoryTest1(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_OSAL_MultiThreadTest1(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_OSAL_TimerTest1(OMX_IN OMX_STRING cComponentName);

/* Standard Component Class tests */
OMX_ERRORTYPE OMX_CONF_StdAudioDecoderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdMp3DecoderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdAacDecoderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdRealAudioDecoderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdWmaDecoderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdVideoDecoderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdH263DecoderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdAvcDecoderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdMpeg4DecoderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdRvDecoderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdWmvDecoderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdReaderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdBinaryAudioReaderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdBinaryVideoReaderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdBinaryImageReaderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdWriterTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdBinaryAudioWriterTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdBinaryVideoWriterTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdBinaryImageWriterTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdAudioCapturerTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdPcmCapturerTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdVideoEncoderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdH263EncoderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdAvcEncoderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdMpeg4EncoderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdImageDecoderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdJpegDecoderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdImageEncoderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdJpegEncoderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdIVRendererTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdYuvOverlayTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdRgbOverlayTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdAudioProcessorTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdAudioMixerTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdPcmMixerTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdAudioRendererTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdPcmRendererTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdAudioEncoderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdMp3EncoderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdAacEncoderTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdClockTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdComponentRoleTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdCameraTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdYuvCameraTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdContainerDemuxerTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_Std3GpContainerDemuxerTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdAsfContainerDemuxerTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdRealContainerDemuxerTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_Std3GpContainerMuxerTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_StdVideoSchedulerTest(OMX_IN OMX_STRING cComponentName);
OMX_ERRORTYPE OMX_CONF_DataMetabolismTest(OMX_IN OMX_STRING cComponentName);


OMX_CONF_TESTLOOKUPTYPE g_OMX_CONF_TestLookupTable [] = 
{
    /* Base Profile Tests */
    {"StateTransitionTest",       OMX_CONF_StateTransitionTest,       OMX_CONF_TestFlag_Base},
    {"ComponentNameTest",         OMX_CONF_ComponentNameTest,         OMX_CONF_TestFlag_Base},
    {"ParameterTest",             OMX_CONF_ParameterTest,             OMX_CONF_TestFlag_Base},
    {"BufferTest",                OMX_CONF_BufferTest,                OMX_CONF_TestFlag_Base},
    {"BufferFlagTest",            OMX_CONF_BufferFlagTest,            OMX_CONF_TestFlag_Base|OMX_CONF_TestFlag_AutoOutput},
    {"BaseMultiThreadedTest",     OMX_CONF_BaseMultiThreadedTest,     OMX_CONF_TestFlag_Base|OMX_CONF_TestFlag_AutoOutput|OMX_CONF_TestFlag_Threaded},
    {"FlushTest",                 OMX_CONF_FlushTest,                 OMX_CONF_TestFlag_Base|OMX_CONF_TestFlag_AutoOutput}, 
    {"PortCommunicationTest",     OMX_CONF_PortCommunicationTest,     OMX_CONF_TestFlag_Base|OMX_CONF_TestFlag_AutoOutput}, 
    {"ResourceExhaustionTest",    OMX_CONF_ResourceExhaustionTest,    OMX_CONF_TestFlag_Base}, 

//We don't want to run all these tests, just pick whatever is applicable
    
#ifdef OSAL_TESTS
    /* special tests for OSAL layer (useful when bringing up new platform); component is ignored */
    ,{"_OSAL_All", OMX_OSAL_TestAll},
    {"_OSAL_MemoryTest1", OMX_OSAL_MemoryTest1},
    {"_OSAL_MultiThreadTest1", OMX_OSAL_MultiThreadTest1},
    {"_OSAL_TimereTest1", OMX_OSAL_TimerTest1}
#endif
};

OMX_U32 g_OMX_CONF_nTestLookupTableEntries = sizeof(g_OMX_CONF_TestLookupTable)/sizeof(OMX_CONF_TESTLOOKUPTYPE);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* File EOF */
