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

#ifndef PVMF_PROTOCOLENGINE_NODE_SHOUTCAST_H_INCLUDED
#define PVMF_PROTOCOLENGINE_NODE_SHOUTCAST_H_INCLUDED

#ifndef PVMF_PROTOCOLENGINE_NODE_DOWNLOAD_COMMON_H_INCLUDED
#include "pvmf_protocol_engine_node_progressive_streaming.h"
#endif


// forward declaration
typedef struct __PvmiKvp PvmiKvp;

////////////////////////////////////////////////////////////////////////////////////
//////	ProgressiveStreamingContainer
////////////////////////////////////////////////////////////////////////////////////
class ShoutcastContainer : public ProgressiveStreamingContainer
{
    public:
        bool createProtocolObjects();
        PVMFStatus doStop();

        // constructor
        ShoutcastContainer(PVMFProtocolEngineNode *aNode = NULL);
};

class ShoutcastControl : public progressiveStreamingControl
{
    public:

        // constructor
        ShoutcastControl();

    private:
        void setProtocolInfo();

    private:
        bool iSetProtocolInfo;
        Oscl_Vector<PvmiKvp*, OsclMemAllocator> iInfoKvpVec;
};

#endif

