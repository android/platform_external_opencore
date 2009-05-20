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
/*********************************************************************************/
/*
    This AACUtils Class contains sime useful methods for operating FILE
*/
/*********************************************************************************/

#include "aacfileio.h"
#include "oscl_base.h"

#ifndef OSCL_SNPRINTF_H_INCLUDED
#include "oscl_snprintf.h"
#endif

int32
AACUtils::getCurrentFilePosition(PVFile *fp)
{
    return (fp->Tell());
}

int32 AACUtils::OpenFile(OSCL_wHeapString<OsclMemAllocator> filename,
                         uint32 mode,
                         PVFile *fp,
                         Oscl_FileServer fileServSession)
{
    OSCL_UNUSED_ARG(mode);
    if (fp != NULL)
    {
        return (fp->Open(filename.get_cstr(),
                         Oscl_File::MODE_READ | Oscl_File::MODE_BINARY, fileServSession));
    }
    return -1;
}

int32 AACUtils::CloseFile(PVFile *fp)
{
    if (fp != NULL)
    {
        return (fp->Close());
    }
    return -1;
}

int32  AACUtils::Flush(PVFile *fp)
{
    if (fp != NULL)
    {
        return (fp->Flush());
    }
    return -1;
}

bool AACUtils::getCurrentFileSize(PVFile *fp, uint32& aCurrentSize)
{
    if (fp != NULL)
    {
        aCurrentSize = 0;
        uint32 aRemBytes = 0;
        if (fp->GetRemainingBytes(aRemBytes))
        {
            uint32 currPos = (uint32)(fp->Tell());
            aCurrentSize = currPos + aRemBytes;
            return true;
        }
    }
    return false;
}

ParserErrorCode AACUtils::SeektoOffset(PVFile *aFP, int32 aOffset, Oscl_File::seek_type aSeekType)
{
    uint32 currFileSize = 0;
    uint32 currPos = 0;
    int32 seekOffset = 0;
    currPos = aFP->Tell();
    AACUtils::getCurrentFileSize(aFP, currFileSize);

    // translate the seek offset to seek from current position
    switch (aSeekType)
    {
        case Oscl_File::SEEKSET:
            seekOffset = aOffset - currPos;
            break;
        case Oscl_File::SEEKEND:
            seekOffset = currFileSize - currPos;
            break;
        case Oscl_File::SEEKCUR:
            seekOffset = aOffset;
        default:
            break;
    }

    if (aOffset <= 0 || currFileSize >= (uint32) aOffset)
    {
        if (aFP->Seek(seekOffset, Oscl_File::SEEKCUR) != 0)
        {
            return AAC_FILE_READ_ERR;
        }
        return AAC_SUCCESS;
    }
    else
    {
        return INSUFFICIENT_DATA;
    }
    return GENERIC_ERROR;
}

