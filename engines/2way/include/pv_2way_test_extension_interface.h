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
#ifndef PV_2WAY_TEST_EXTENSION_H_INCLUDED
#define PV_2WAY_TEST_EXTENSION_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_STRING_H_INCLUDED
#include "oscl_string.h"
#endif
#ifndef OSCL_REFCOUNTER_MEMFRAG_H_INCLUDED
#include "oscl_refcounter_memfrag.h"
#endif
#ifndef PV_UUID_H_INCLUDED
#include "pv_uuid.h"
#endif
#ifndef PV_INTERFACE_H_INCLUDED
#include "pv_interface.h"
#endif

// Forward declaration
template <class T> class OsclSharedPtr;

#define PV2WayTestEncExtensionUUID PVUuid(0x19b53654, 0x2dd4,0x4469,0xa9,0xdb,0x86,0x28,0xa7,0x69,0x92,0xe3)

////////////////////////////////////////////////////////////////////////////
class PV2WayTestExtensionInterface : public PVInterface
{
    public:
        /** Increment reference counter for this interface. */
        virtual void addRef() = 0;

        /** Decrement reference counter for this interface. */
        virtual void removeRef() = 0;

        /**
         * Query for a pointer to an instance of the interface specified by the UUID.
         *
         * @param uuid UUID of the interface to be queried.
         * @param iface Output parameter where a pointer to an instance of the requested
         * interface is stored if the interface is supported.
         * @return True if successful, else false.
         */
        virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface) = 0;

        virtual bool AcceptableFormatsMatch(
            Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>& iInAudFormatCapability,
            Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>& iOutAudFormatCapability,
            Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>& iInVidFormatCapability,
            Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>& iOutVidFormatCapability) = 0;


};
#endif





