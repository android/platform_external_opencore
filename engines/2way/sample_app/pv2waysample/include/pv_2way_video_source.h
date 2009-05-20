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
#ifndef PV_2WAY_VIDEO_SOURCE_H_INCLUDED
#define PV_2WAY_VIDEO_SOURCE_H_INCLUDED
////////////////////////////////////////////////////////////////////////////
// This file includes the class definition for the Video Source MIO
//
// This class includes functions to create and delete the MIO.
//
////////////////////////////////////////////////////////////////////////////

#include "pv_2way_source.h"
#include "pv_2way_codecspecifier.h"

class PV2WayVideoSource : public PV2WaySource
{
    public:
        PV2WayVideoSource(Oscl_Vector<CodecSpecifier*, OsclMemAllocator>* aFormats) :
                PV2WaySource(aFormats)
        {
            iMyType = PV_VIDEO;
        };

        virtual ~PV2WayVideoSource()
        {

        };

    protected:
        virtual int CreateMioNodeFactory(CodecSpecifier* aSelectedCodec);

};



#endif
