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

#include "pv_2way_unittest_source_and_sinks.h"

#ifndef PVMF_MEDIA_INPUT_NODE_FACTORY_H_INCLUDED
#include "pvmf_media_input_node_factory.h"
#endif

#include "pvmf_format_type.h"


PV2WayUnitTestSourceAndSinks::PV2WayUnitTestSourceAndSinks(PV2WaySourceAndSinksObserver* aObserver,
        PV2Way324InitInfo& aSdkInitInfo) :
        PV2WaySourceAndSinksBase(aObserver, aSdkInitInfo)
{
}

PV2WayUnitTestSourceAndSinks::~PV2WayUnitTestSourceAndSinks()
{
}

int PV2WayUnitTestSourceAndSinks::CreateCodecs()
{
    return PVMFSuccess;
}

