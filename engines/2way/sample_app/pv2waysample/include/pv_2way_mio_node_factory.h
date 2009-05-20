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
#ifndef PV_2WAY_MIO_NODE_FACTORY_H_INCLUDED
#define PV_2WAY_MIO_NODE_FACTORY_H_INCLUDED
////////////////////////////////////////////////////////////////////////////
// This file includes the class definition for the base class for an MIO Node Factory
//
// This class includes pure virtual functions to create and delete the MIO Node Factory
//
// This class also defines common MIO behavior: Creating an MIO Node, Deleting an MIO Node
//
// With this design the only code that needs to be in the derived classes is how
// to create and delete the specific MIO Node.
//
////////////////////////////////////////////////////////////////////////////



class PV2WayMIONodeFactory
{
    public:
        PV2WayMIONodeFactory() {};
        virtual ~PV2WayMIONodeFactory() {};
        virtual PVMFNodeInterface* Create() = 0;
        virtual void Delete(PVMFNodeInterface** mioNode) = 0;
};



#endif
