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
#ifndef PV_2WAY_CODECSPECIFIER_H_INCLUDED
#define PV_2WAY_CODECSPECIFIER_H_INCLUDED
////////////////////////////////////////////////////////////////////////////
// This file includes the class definition for the base class for an MIO
//
// This class includes pure virtual functions to create and delete the MIO
// that need to be overriden in derived classes (such as the AudioSink, AudioSource,
// VideoSink, VideoSource classes).
//
// This class also defines common MIO behavior: Adding/Removing an MIO, handling
// events with respect to the MIO, etc.
//
// With this design the only code that needs to be in the derived classes is how
// to create and delete the specific MIO.
//
////////////////////////////////////////////////////////////////////////////
#include "pv_2way_codecspecifier_interface.h"

#include "pvmf_fileinput_settings.h"
#include "pvmi_mio_fileinput_factory.h"
//


class CharCodecSpecifier : public CodecSpecifier
{
    public:
        CharCodecSpecifier(PVMFFormatType& aFormat):
                iFormat(aFormat)
        {
            iType = EPVCharInput;
        };
        bool IsFormat(PVMFFormatType& format)
        {
            if (format == iFormat)
                return true;
            return false;
        };
        PVMFFormatType& GetSpecifierType()
        {
            return iFormat;
        }
        virtual PVMFFormatType& GetFormat()
        {
            return iFormat;
        };
    private:
        PVMFFormatType iFormat;

};


class MIOFileCodecSpecifier : public CodecSpecifier
{
    public:
        MIOFileCodecSpecifier(PvmiMIOFileInputSettings& aFormat):
                iFormat(aFormat)
        {
            iType = EPVMIOFileInput;
        };
        bool IsFormat(PVMFFormatType& format)
        {
            return CompareFormat(format, iFormat.iMediaFormat.getMIMEStrPtr());
        };
        PvmiMIOFileInputSettings GetSpecifierType()
        {
            return iFormat;
        }
        virtual PVMFFormatType& GetFormat()
        {
            return iFormat.iMediaFormat;
        };
    private:
        PvmiMIOFileInputSettings iFormat;
};


class FileCodecSpecifier : public CodecSpecifier
{
        // this class is for PERF_TEST
    public:
        FileCodecSpecifier(PVMFFileInputSettings& aFormat):
                iFormat(aFormat)
        {
            iType = EPVFileInput;
        };
        bool IsFormat(PVMFFormatType& format)
        {
            return CompareFormat(format, iFormat.iMediaFormat.getMIMEStrPtr());
        };
        PVMFFileInputSettings GetSpecifierType()
        {
            return iFormat;
        }
        virtual PVMFFormatType& GetFormat()
        {
            return iFormat.iMediaFormat;
        };
    private:
        PVMFFileInputSettings iFormat;
};

#endif
