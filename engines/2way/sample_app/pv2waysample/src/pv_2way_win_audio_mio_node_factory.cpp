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
#ifdef PV2WAY_WIN32_GUI

#include "pv_2way_win_audio_mio_node_factory.h"
#include "pv_2way_interface.h"
#include "pvmf_fileoutput_factory.h"
#include "pv_2way_media.h"
#include "pvmi_media_io_fileoutput.h"
#include "pv_media_output_node_factory.h"

void PV2WayWinAudioMIONodeFactory::CreateIOControl()
{
    DeleteIOControl();
    iIOControl = OSCL_NEW(RefAudioIO, ());
}

void PV2WayWinAudioMIONodeFactory::DeleteIOControl()
{
    if (iIOControl)
    {
        iIOControl->Destroy();
        OSCL_DELETE(iIOControl);
    }
    iIOControl = NULL;
}

void PV2WayWinAudioMIONodeFactory::Delete(PVMFNodeInterface** aMioNode)
{
    PVMediaOutputNodeFactory::DeleteMediaOutputNode(*aMioNode);
    DeleteIOControl();
}

PVMFNodeInterface* PV2WayWinAudioMIONodeFactory::Create()
{
    PVMFNodeInterface* mioNode = NULL;
    CreateIOControl();

    mioNode = PVMediaOutputNodeFactory::CreateMediaOutputNode(iIOControl);
    return mioNode;
}

#endif


