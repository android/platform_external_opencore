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

/** OMX_CONF_StdCompCommon.h
 *  Common header file for testing of the Standard Components classes defined by
 *  OpenMAX_IL_v11_Section8f_StdComponents.doc.
 */

#ifndef OMX_CONF_StdCompCommon_h
#define OMX_CONF_StdCompCommon_h

#include <OMX_Types.h>
#include <OMX_Core.h>

#include <OMX_OSAL_Interfaces.h>
#include <OMX_CONF_TestHarness.h>
#include <OMX_CONF_StubbedCallbacks.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define MAX_COMPONENT_ROLES 16
#define MAX_COMPONENT_NAMELENGTH 128*sizeof(OMX_U8)

    /**********************************************************************
     * COMMON MACROS
     **********************************************************************/

    /* Utility macro.
       Initialize the given structure with OpenMAX version and size.
       Hardcoded to 1.0 value. May need to be changed to 1.1 in future.
    */
#define OMX_CONF_INIT_STRUCT(_s_, _name_)\
    memset(&(_s_), 0x0, sizeof(_name_));\
    (_s_).nSize = sizeof(_name_);\
    (_s_).nVersion.s.nVersionMajor = 0x1;\
    (_s_).nVersion.s.nVersionMinor = 0x0;\
    (_s_).nVersion.s.nRevision = 0x0;\
    (_s_).nVersion.s.nStep = 0x0                               /* Macro End */

    /* Utility macro.
       Bail out if there is an error.
    */
#define OMX_CONF_BAIL_ON_ERROR(_e_) \
    if (OMX_ErrorNone != (_e_))\
    {\
        goto OMX_CONF_TEST_BAIL;\
    }                                                          /* Macro End */

    /* Utility macro.
       Sets the error return value to the given value and then bails out.
       An appropriate error message based on the error value is output.
    */
#define OMX_CONF_SET_ERROR_BAIL(_e_, _c_, _s_)\
    _e_ = _c_;\
    OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, _s_);\
    goto OMX_CONF_TEST_BAIL                                    /* Macro End */

    /*  Utility macro.
        Bail out with an error OMX_ErrorUndfined if the expression _a_
        does not evalute to be as expected. Outputs the user supplies message
        passed on by _r_.
    */
#define OMX_CONF_ASSERT(_e_, _a_, _r_) \
    if (!(_a_))\
    {\
        OMX_OSAL_Trace(OMX_OSAL_TRACE_ERROR, "%s\n", (_r_));\
        (_e_) = OMX_ErrorUndefined;\
        goto OMX_CONF_TEST_BAIL;\
    }                                                          /* Macro End */

    /* Verify that a component supports the GetParamater and SetParameter
       for a given parameter. Can be used to verify the mandatory support of
       parameters (e.g. OMX_IndexParamPortDefinition, etc). Bails out with
       an error if the mandatory support is not available.
    */
#define OMX_CONF_PARAM_REQUIREDGETSET(_p_, _i_, _s_, _e_)\
    _e_ = OMX_GetParameter((_p_)->hWrappedComp, _i_, _s_);\
    OMX_CONF_BAIL_ON_ERROR(_e_);\
    _e_ = OMX_SetParameter((_p_)->hWrappedComp, _i_, _s_);\
    OMX_CONF_BAIL_ON_ERROR(_e_)                                /* Macro End */


    /* Test that a component properly handles a GetParameter/SetParameter
       call with an unsupported parameter value.
    */
#define OMX_CONF_PARAM_OPTIONALGETSET(_p_, _i_, _s_, _e_)\
    _e_ = OMX_GetParameter((_p_)->hWrappedComp, _i_, _s_);\
    if (OMX_ErrorUnsupportedIndex == _e_) _e_ = OMX_ErrorNone;\
    OMX_CONF_BAIL_ON_ERROR(_e_);\
    _e_ = OMX_SetParameter((_p_)->hWrappedComp, _i_, _s_);\
    if (OMX_ErrorUnsupportedIndex == _e_) _e_ = OMX_ErrorNone;\
    OMX_CONF_BAIL_ON_ERROR(_e_)                                /* Macro End */


    /* Verify that a component supports the GetConfig and SetConfig
       for a given parameter. Can be used to verify the mandatory support of
       parameters (e.g. OMX_IndexParamPortDefinition, etc).
    */
#define OMX_CONF_CONFIG_REQUIREDGETSET(_p_, _i_, _s_, _e_)\
    _e_ = OMX_GetConfig((_p_)->hWrappedComp, _i_, _s_);\
    OMX_CONF_BAIL_ON_ERROR(_e_);\
    _e_ = OMX_SetConfig((_p_)->hWrappedComp, _i_, _s_);\
    OMX_CONF_BAIL_ON_ERROR(_e_)                                /* Macro End */


    /* Test that a component properly handles a GetParameter/SetParameter
       call with an unsupported parameter value.
     */
#define OMX_CONF_CONFIG_OPTIONALGETSET(_p_, _i_, _s_, _e_)\
    _e_ = OMX_GetConfig((_p_)->hWrappedComp, _i_, _s_);\
    if (OMX_ErrorUnsupportedIndex == _e_) _e_ = OMX_ErrorNone;\
    OMX_CONF_BAIL_ON_ERROR(_e_);\
    _e_ = OMX_SetConfig((_p_)->hWrappedComp, _i_, _s_);\
    if (OMX_ErrorUnsupportedIndex == _e_) _e_ = OMX_ErrorNone;\
    OMX_CONF_BAIL_ON_ERROR(_e_)                                /* Macro End */


    /* Implements a mechanism to verify that a component is able to accept
       a new value for a parameter and is able to set it correctly.
    */
#define OMX_CONF_PARAM_READ_WRITE_VERIFY(_p_, _i_, _s_, _m_, _v_, _e_)\
    _e_ = OMX_GetParameter((_p_)->hWrappedComp, _i_, _s_);\
    OMX_CONF_BAIL_ON_ERROR(_e_);\
    _m_ = _v_;\
    _e_ = OMX_SetParameter((_p_)->hWrappedComp, _i_, _s_);\
    OMX_CONF_BAIL_ON_ERROR(_e_);\
    _e_ = OMX_GetParameter((_p_)->hWrappedComp, _i_, _s_);\
    OMX_CONF_BAIL_ON_ERROR(_e_);\
    if (_m_ != _v_) _e_ = OMX_ErrorBadParameter;\
    OMX_CONF_BAIL_ON_ERROR(_e_)                                /* Macro End */


    /* Implements a mechanism to verify that a component is able to accept
       a new value for a parameter and is able to set it correctly.
    */
#define OMX_CONF_PARAM_WRITE_VERIFY(_p_, _i_, _s_, _m_, _v_, _e_)\
    _m_ = _v_;\
    _e_ = OMX_SetParameter((_p_)->hWrappedComp, _i_, _s_);\
    OMX_CONF_BAIL_ON_ERROR(_e_);\
    _e_ = OMX_GetParameter((_p_)->hWrappedComp, _i_, _s_);\
    OMX_CONF_BAIL_ON_ERROR(_e_);\
    if (_m_ != _v_) _e_ = OMX_ErrorBadParameter;\
    OMX_CONF_BAIL_ON_ERROR(_e_)                                /* Macro End */


    /* Implements a mechanism to verify that a component is able to accept
       a new value for a config and is able to set it correctly.
       Bogusvalue is used to make sure the component really does change the value
    */
#define OMX_CONF_CONFIG_READ_WRITE_VERIFY(_p_, _i_, _s_, _m_, _v_, _bogusvalue_, _e_)\
    _e_ = OMX_GetConfig((_p_)->hWrappedComp, _i_, _s_);\
    OMX_CONF_BAIL_ON_ERROR(_e_);\
    _m_ = _v_;\
    _e_ = OMX_SetConfig((_p_)->hWrappedComp, _i_, _s_);\
    OMX_CONF_BAIL_ON_ERROR(_e_);\
    _m_ = _bogusvalue_;\
    _e_ = OMX_GetConfig((_p_)->hWrappedComp, _i_, _s_);\
    OMX_CONF_BAIL_ON_ERROR(_e_);\
    if (_m_ != _v_) _e_ = OMX_ErrorBadParameter;\
    OMX_CONF_BAIL_ON_ERROR(_e_)                                /* Macro End */


    /* Implements a mechanism to verify that a component is able to accept
       a new value for a parameter and is able to set it correctly.
    */
#define OMX_CONF_CONFIG_WRITE_VERIFY(_p_, _i_, _s_, _m_, _v_, _e_)\
    _m_ = _v_;\
    _e_ = OMX_SetConfig((_p_)->hWrappedComp, _i_, _s_);\
    OMX_CONF_BAIL_ON_ERROR(_e_);\
    _e_ = OMX_GetConfig((_p_)->hWrappedComp, _i_, _s_);\
    OMX_CONF_BAIL_ON_ERROR(_e_);\
    if (_m_ != _v_) _e_ = OMX_ErrorBadParameter;\
    OMX_CONF_BAIL_ON_ERROR(_e_)                                /* Macro End */


    /**********************************************************************/

    typedef struct _TEST_CTXTYPE
    {
        OMX_HANDLETYPE hComp;
        OMX_HANDLETYPE hWrappedComp;
        OMX_STATETYPE eState;
        OMX_U32 nNumPorts;
        OMX_PORT_PARAM_TYPE sPortParamAudio;
        OMX_PORT_PARAM_TYPE sPortParamVideo;
        OMX_PORT_PARAM_TYPE sPortParamImage;
        OMX_PORT_PARAM_TYPE sPortParamOther;
        OMX_HANDLETYPE hStateChangeEvent;
    } TEST_CTXTYPE;


    typedef OMX_ERRORTYPE(*STDCOMPTEST_COMPONENT)(TEST_CTXTYPE *pCtx);

    /************************ Function declarations **************************/

    /* Utility function: Allocate memory for the roles string array. */
    OMX_ERRORTYPE StdComponentTest_PopulateRolesArray(OMX_STRING, OMX_U32 *, OMX_STRING*);

    /* Utility function: Release memory used by the roles string array. */
    void StdComponentTest_FreeRolesArray(OMX_U32, OMX_STRING*);

    /* Test if component supports a role. */
    OMX_ERRORTYPE StdComponentTest_IsRoleSupported(OMX_STRING cComponentName, OMX_STRING cRole);

    /* Set a role for a component */
    OMX_ERRORTYPE StdComponentTest_SetRole(TEST_CTXTYPE *pCtx, OMX_STRING cRole);

    /* Verify support for OMX_IndexParamCompBufferSupplier. */
    OMX_ERRORTYPE StdComponentTest_BufferSupplier(TEST_CTXTYPE *, OMX_U32);

    /* Determine how many ports of each domain are present. */
    OMX_ERRORTYPE StdComponentTest_InspectPorts(TEST_CTXTYPE *);

    /* Check if the given port is an input port. */
    OMX_BOOL StdComponentTest_IsInputPort(TEST_CTXTYPE *, OMX_U32);

    /* Check if the given port is an output port. */
    OMX_BOOL StdComponentTest_IsOutputPort(TEST_CTXTYPE *, OMX_U32);

    /* Verify support for standard component parameters. */
    OMX_ERRORTYPE StdComponentTest_StdPortParameters(TEST_CTXTYPE *, OMX_U32);

    /* Common setup and test function standard component tests. */
    OMX_ERRORTYPE StdComponentTest_StdComp(OMX_IN OMX_STRING,
                                           TEST_CTXTYPE *,
                                           STDCOMPTEST_COMPONENT);

    /* Verify support for standard component parameters for an MP3 port. */
    OMX_ERRORTYPE StdCompCommonAudio_Mp3PortParameters(TEST_CTXTYPE *, OMX_U32);

    /* Verify support for standard component parameters for an PCM port. */
    OMX_ERRORTYPE StdCompCommonAudio_PcmPortParameters(TEST_CTXTYPE *, OMX_U32);

    /* Verify support for standard component parameters for an AAC port. */
    OMX_ERRORTYPE StdCompCommonAudio_AacPortParameters(TEST_CTXTYPE *, OMX_U32);

    /* Verify support for standard component parameters for an Real Audio port. */
    OMX_ERRORTYPE StdCompCommonAudio_RealAudioPortParameters(TEST_CTXTYPE *, OMX_U32);

    /* Verify support for standard component parameters for an WMA port. */
    OMX_ERRORTYPE StdCompCommonAudio_WmaPortParameters(TEST_CTXTYPE *, OMX_U32);

    /* Verify support for standard component parameters for an H263 port. */
    OMX_ERRORTYPE StdCompCommonVideo_H263PortParameters(TEST_CTXTYPE *, OMX_U32);

    /* Verify support for standard component parameters for an Real Video port. */
    OMX_ERRORTYPE StdCompCommonVideo_RvPortParameters(TEST_CTXTYPE *, OMX_U32);

    /* Verify support for standard component parameters for an WMV port. */
    OMX_ERRORTYPE StdCompCommonVideo_WmvPortParameters(TEST_CTXTYPE *, OMX_U32);

    /* Verify support for standard component parameters for an MPEG4 port. */
    OMX_ERRORTYPE StdCompCommonVideo_Mpeg4PortParameters(TEST_CTXTYPE *, OMX_U32);

    /* Verify support for standard component parameters for an AVC(H264) port. */
    OMX_ERRORTYPE StdCompCommonVideo_AvcPortParameters(TEST_CTXTYPE *, OMX_U32);

    /* Verify support for standard component parameters for an Raw Video port. */
    OMX_ERRORTYPE StdCompCommonVideo_RawPortParameters(TEST_CTXTYPE *, OMX_U32);

    /* Verify support for standard component parameters for an raw QCIF+ port. */
    OMX_ERRORTYPE StdCompCommonVideo_QcifPlusPortParameters(TEST_CTXTYPE *, OMX_U32, OMX_COLOR_FORMATTYPE);

    /* Verify support for standard component parameters for an OTHER domain port. */
    OMX_ERRORTYPE StdCompCommonOther_OtherPortParameters(TEST_CTXTYPE *, OMX_U32);

    /* Verify support for standard component parameters for a Container Demuxer port. */
    OMX_ERRORTYPE StdCompCommonOther_ContainerDemuxerPortParameters(TEST_CTXTYPE *, OMX_U32);

    /* Verify support for standard component parameters for a Container Muxer port. */
    OMX_ERRORTYPE StdCompCommonOther_ContainerMuxerPortParameters(TEST_CTXTYPE *, OMX_U32);

    /* Verify support for standard component parameters for a Camera port. */
    OMX_ERRORTYPE StdCompCommonOther_YuvCameraPortParameters(TEST_CTXTYPE *, OMX_U32);

    /* Verify support for standard component parameters for an JPEG port. */
    OMX_ERRORTYPE StdCompCommonImage_JpegPortParameters(TEST_CTXTYPE *, OMX_U32);

    /* Verify support for standard component parameters for an Raw Image port. */
    OMX_ERRORTYPE StdCompCommonImage_RawPortParameters(TEST_CTXTYPE *, OMX_U32);

    /* Verify support for standard component parameters for an image/video renderer port. */
    OMX_ERRORTYPE StdCompCommonImage_IVRendererPortParameters(TEST_CTXTYPE *, OMX_U32);

    /* Verify support for standard component parameters for an raw QCIF+ port. */
    OMX_ERRORTYPE StdCompCommonImage_QcifPlusPortParameters(TEST_CTXTYPE *, OMX_U32, OMX_COLOR_FORMATTYPE);

    /**********************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
/* File EOF */
