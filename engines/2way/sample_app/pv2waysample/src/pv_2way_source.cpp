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
#include "pv_2way_source.h"
#include "testcaseparser.h"

#include "pv_2way_media.h"
#include "pv_mime_string_utils.h"


int PV2WaySource::AddCodec(PvmiMIOFileInputSettings& aFileSettings)
{
    if (aFileSettings.iMediaFormat == PVMF_MIME_FORMAT_UNKNOWN)
    {
        aFileSettings.iMediaFormat = PV2WayMedia::GetMediaFormat(aFileSettings.iFileName.get_cstr());
    }
    AddFormat(aFileSettings);
    if (pv_mime_strcmp(aFileSettings.iMediaFormat.getMIMEStrPtr(), PVMF_MIME_M4V) == 0)
    {
        // add this explicitly so an additional YUV will be added
        PV2WayMIO::AddCodec(PVMFFormatType(PVMF_MIME_YUV420));
    }
#ifdef PV2WAY_WIN32_GUI
    // for sources parse the con file - want to find out the format
    if (!ParseConFile(aFileSettings))
        return -1;
#endif
    return 0;
}

int PV2WaySource::AddCodec(PVMFFileInputSettings& aFileSettings)
{
    AddFormat(aFileSettings);
    if (pv_mime_strcmp(aFileSettings.iMediaFormat.getMIMEStrPtr(), PVMF_MIME_M4V) == 0)
    {
        // add this explicitly so an additional YUV will be added
        PV2WayMIO::AddCodec(PVMFFormatType(PVMF_MIME_YUV420));
    }
    return 0;
}
