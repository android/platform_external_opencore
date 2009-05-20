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

#include "pv_2way_win_video_mio_node_factory.h"
#include "pv_2way_interface.h"
#include "pvmf_fileoutput_factory.h"
#include "pv_2way_media.h"
#include "pvmi_media_io_fileoutput.h"
#include "pv_media_output_node_factory.h"
#include "pvmi_media_io_video.h"



// creating outside this class and setting it instead of creating it here.
void PV2WayWinVideoMIONodeFactory::CreateIOControl()
{
    DeleteIOControl();
    if (!iIOControl)
    {
        if (iPlayOnMediaObserver)
        {
            iIOControl = OSCL_NEW(RefVideoIO, (iPlayOnMediaObserver));
        }
    }
}


void PV2WayWinVideoMIONodeFactory::DeleteIOControl()
{
    if (iIOControl)
    {
        OSCL_DELETE(iIOControl);
        iIOControl = NULL;
    }
}

void PV2WayWinVideoMIONodeFactory::Delete(PVMFNodeInterface** aMioNode)
{
    PVMediaOutputNodeFactory::DeleteMediaOutputNode(*aMioNode);
    *aMioNode = NULL;
    DeleteIOControl();
}

PVMFNodeInterface* PV2WayWinVideoMIONodeFactory::Create()
{
    PVMFNodeInterface* mioNode = NULL;

    CreateIOControl();
    if (!iIOControl)
    {
        return mioNode;
    }
    mioNode = PVMediaOutputNodeFactory::CreateMediaOutputNode(iIOControl);
    return mioNode;
}



#endif
