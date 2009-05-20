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
#ifndef PV_2WAY_SINK_H_INCLUDED
#define PV_2WAY_SINK_H_INCLUDED
////////////////////////////////////////////////////////////////////////////
// This file includes the class definition for the Audio Sink MIO
//
// This class includes functions to create and delete the MIO.
//
////////////////////////////////////////////////////////////////////////////


#include "pv_2way_mio.h"
#include "pvmi_media_io_fileoutput_registry_factory.h"

class PV2WaySink : public PV2WayMIO
{
    public:
        PV2WaySink(Oscl_Vector<CodecSpecifier*, OsclMemAllocator>* aFormats) :
                PV2WayMIO(aFormats)
        {
            iMyDir = INCOMING;
        };

        virtual ~PV2WaySink()
        {
        };
        int AddCodec(PvmiMIOFileInputSettings& aFileSettings);


        int AddCodec(PVMFFileInputSettings& aFileSettings);

    protected:

};



#endif
