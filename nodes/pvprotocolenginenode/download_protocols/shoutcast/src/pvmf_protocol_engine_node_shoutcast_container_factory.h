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
#ifndef PVMF_PROTOCOLENGINE_NODE_SHOUTCAST_FACTORY_H_INCLUDED
#define PVMF_PROTOCOLENGINE_NODE_SHOUTCAST_FACTORY_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif

// Forward declaration
class ProtocolContainer;
class PVMFProtocolEngineNode;

// UUID for the factory
#define KPVMFProtocolEngineNodeShoutcastUuid PVUuid(0x38b600d2,0xb676,0x4e17,0x8d,0xf9,0x26,0x20,0x95,0x6a,0x35,0xff);

class PVMFProtocolEngineNodeShoutcastContainerFactory
{
    public:
        OSCL_IMPORT_REF static ProtocolContainer* Create(PVMFProtocolEngineNode *aNode);
        OSCL_IMPORT_REF static bool Delete(ProtocolContainer* aContainer);
};

#endif // PVMF_PROTOCOLENGINE_NODE_SHOUTCAST_FACTORY_H_INCLUDED


