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
#ifndef TEST_PV_PLAYER_ENGINE_CONFIG_EXT_H_INCLUDED
#define TEST_PV_PLAYER_ENGINE_CONFIG_EXT_H_INCLUDED

#ifndef IGNORE_DEFAULT_CFG
//-----------------------------------------------------------------------------
// during package scissoring, this entire section is removed which implies that
// every macro shown below must be defined for the scissoring process with the
// expectation that the final delivered code will have
//    the macro guards removed, and
//    the guarded code conditionally scissored
// if the macro is to remain intact and delivered to the customer, then it does
// not belong inside this section, and should also not be defined for the
// scissoring process.

// this sub-section is common to both opencore_dynamic and opencore_value_add
// configurations.
#define RUN_3GPFILE_TESTCASES 1
#define RUN_AACFILE_TESTCASES 1
#define RUN_AMRFILE_TESTCASES 1
#define RUN_HTTPDOWNLOAD_TESTCASES 1
#define RUN_MP3FILE_TESTCASES 1
#define RUN_MP4FILE_TESTCASES 1
#define RUN_RTSP_CLOAKING_TESTCASES 0
#define RUN_PVNETWORK_DOWNLOAD_TESTCASES 0
#define RUN_STREAMING_TESTCASES 1
#define RUN_WAVFILE_TESTCASES 1
#define RUN_WMAFILE_TESTCASES 1
#define RUN_WMVFILE_TESTCASES 1
#define RUN_CPMOMA1_DLAPASSTHRU_TESTCASES 1
#define OMA1PASSTHRU_IS_LOADABLE_MODULE 1
#define RUN_SHOUTCAST_TESTCASES 1
#ifndef USE_DEFAULT_CFG_VALUEADD
// this sub-section is for opencore_dynamic configuration only and
// is typically used to explicitly disable value-add features.
# define JANUS_IS_LOADABLE_MODULE 0
# define ONLY_3GPP_STREAMING_FORMAT
# define RUN_APP_TRACK_SELECTION_TESTCASES 0
# define RUN_ASFFILE_TESTCASES 0
# define RUN_CPMJANUS_TESTCASES 0
# define RUN_FASTTRACK_TESTCASES 0
# define RUN_MS_HTTP_STREAMING_TESTCASES 0
# define RUN_REALAUDIO_FILE_TESTCASES 0
#else
// this sub-section is for opencore_value_add only and is typically
// used to explicitly enable value-add features.
# define JANUS_IS_LOADABLE_MODULE 1
# undef  ONLY_3GPP_STREAMING_FORMAT
# define RUN_APP_TRACK_SELECTION_TESTCASES 1
# define RUN_ASFFILE_TESTCASES 1
# define RUN_CPMJANUS_TESTCASES 1
# define RUN_FASTTRACK_TESTCASES 1
# define RUN_MS_HTTP_STREAMING_TESTCASES 1
# define RUN_REALAUDIO_FILE_TESTCASES 0 // not supported yet
#endif
//-----------------------------------------------------------------------------
#endif

#endif

