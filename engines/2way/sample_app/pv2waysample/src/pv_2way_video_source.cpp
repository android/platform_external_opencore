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
#include "pv_2way_video_source.h"
#ifdef PERF_TEST
#include "pv_2way_dummy_input_mio_node_factory.h"
#endif

#include "pv_2way_media_input_mio_node_factory.h"

int PV2WayVideoSource::CreateMioNodeFactory(CodecSpecifier* aSelectedCodec)
{
    switch (aSelectedCodec->GetType())
    {
        case EPVCharInput:
        {
#if defined(USE_DIRECTX)
            iMioNodeFactory = OSCL_NEW(PV2WayVideoInputMIONodeFactory,
                                       (iPreviewHandle));
#endif
            break;
        }
        case EPVFileInput:
        {
            // PERF_TEST
#ifdef PERF_TEST
            FileCodecSpecifier* temp = OSCL_REINTERPRET_CAST(FileCodecSpecifier*, aSelectedCodec);
            iMioNodeFactory = OSCL_NEW(PV2WayDummyInputMIONodeFactory,
                                       (temp->GetSpecifierType()));
#endif
            break;
        }
        case EPVMIOFileInput:
        {
            MIOFileCodecSpecifier* temp = OSCL_REINTERPRET_CAST(MIOFileCodecSpecifier*, aSelectedCodec);
            iMioNodeFactory = OSCL_NEW(PV2WayMediaInputMIONodeFactory,
                                       (temp->GetSpecifierType()));
            break;
        }

        default:
            break;
    }


    if (!iMioNodeFactory)
        return PVMFFailure;
    return PVMFSuccess;
}


