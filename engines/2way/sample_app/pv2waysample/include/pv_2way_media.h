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
#ifndef PV_2WAY_MEDIA_H_INCLUDED
#define PV_2WAY_MEDIA_H_INCLUDED
////////////////////////////////////////////////////////////////////////////
// This file includes the class definition determining MediaFormat
//
// This class may be expanded to handle more complex combinations of formats.
//
////////////////////////////////////////////////////////////////////////////

#include "pvmf_format_type.h"

void ConvertToLowerCase(char *aString);

class PV2WayMedia
{
    public:
        PV2WayMedia()
        {}

        ~PV2WayMedia();

        static PVMFFormatType GetMediaFormat(const oscl_wchar* aFileName);
//        static PVMFFormatType GetMediaFormat(const char* aFileName);
    private:

};

#endif
