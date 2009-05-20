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
#ifndef PV_2WAY_VIDEO_INPUT_MIO_NODE_FACTORY_H_INCLUDED
#define PV_2WAY_VIDEO_INPUT_MIO_NODE_FACTORY_H_INCLUDED

#include "pvmf_node_interface.h"
#include "pv_2way_mio_node_factory.h"


class PV2WayVideoInputMIONodeFactory : public PV2WayMIONodeFactory
{
    public:
        PV2WayVideoInputMIONodeFactory() {};
        virtual ~PV2WayVideoInputMIONodeFactory() {};
        virtual PVMFNodeInterface* Create();
        virtual void Delete(PVMFNodeInterface** mioNode);
    private:

};



#endif
