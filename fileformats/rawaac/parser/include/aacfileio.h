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
#ifndef AAC_UTILS_H_INCLUDED
#define AAC_UTILS_H_INCLUDED

#include "pvfile.h"

enum ParserErrorCode
{
    AAC_ERR_NO_MEMORY   = -6,
    AAC_FILE_READ_ERR   = -5,
    GENERIC_ERROR       = -4,
    INSUFFICIENT_DATA   = -3,
    FILE_OPEN_ERROR     = -2,
    MEMORY_ERROR        = -1,
    AAC_SUCCESS         = 0
};

class AACUtils
{

    public:

        static int32  getCurrentFilePosition(PVFile *fp);
        static int32  OpenFile(OSCL_wHeapString<OsclMemAllocator> filename,
                               uint32 mode,
                               PVFile *fp,
                               Oscl_FileServer fileServSession);
        static int32  CloseFile(PVFile *fp);
        static int32  Flush(PVFile *fp);
        static bool   getCurrentFileSize(PVFile *fp, uint32& aCurrentSize);
        static ParserErrorCode SeektoOffset(PVFile *fp, int32 offset, Oscl_File::seek_type aSeekType);
};

#endif // AAC_UTILS_H_INCLUDED
