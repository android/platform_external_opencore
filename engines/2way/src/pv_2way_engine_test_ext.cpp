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
#ifndef PV_2WAY_ENGINE_H_INCLUDED
#include "pv_2way_engine.h"
#endif

//
//
//
// NOTE: This file is a work-in-progress
// It gives a flavor of what is to come with changes
// with the 2way test extension interface


#include "pvlogger.h"



///////////////////////////////////////////////////////////////////
// CPV324m2Way::SetPreferredCodecs
// This function takes the selection of codecs from the MIOs/app
// and uses these to select the appropriate codecs for the stack node
// it also sets these as the expected codecs in the channels.
//
///////////////////////////////////////////////////////////////////
bool CPV324m2Way::AcceptableFormatsMatch(
    Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>& iInAudFormatCapability,
    Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>& iOutAudFormatCapability,
    Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>& iInVidFormatCapability,
    Oscl_Vector<FormatCapabilityInfo, OsclMemAllocator>& iOutVidFormatCapability)
{
    bool result = false;
    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "CPV324m2Way::AcceptableFormatsMatch"));

    // go through inAudioChannelParams, see that codecs match
    // iOutgoingChannelParams

    //iIncomingAudioCodecs, etc

    /*    FormatCapabilityInfo formatCap;
        if (formatCap.format === otherFormatCap.format)
        {
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "CPV324m2Way::AcceptableFormatsMatch: FOUND A MISMATCH"));
            return false;
        }
        */
    return result;
}

bool CPV324m2Way::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    if (QueryInterface(uuid, iface))
        return true;
    return false;
}

void CPV324m2Way::addRef()
{
    iReferenceCount++;
}

void CPV324m2Way::removeRef()
{
    if (--iReferenceCount <= 0)
    {
        OSCL_DELETE(this);
    }
}

