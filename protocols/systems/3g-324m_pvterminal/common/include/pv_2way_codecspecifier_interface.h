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
#ifndef PV_2WAY_CODECSPECIFIER_INTERFACE_H_INCLUDED
#define PV_2WAY_CODECSPECIFIER_INTERFACE_H_INCLUDED

typedef enum
{
    EPVCharInput,
    EPVFileInput,
    EPVMIOFileInput
} PV2WayCodecSpecifierType;

class CodecSpecifier
{
    public:
        virtual ~CodecSpecifier()
        {
            iType = EPVCharInput;
        };
        virtual bool IsFormat(PVMFFormatType& format) = 0;
        PV2WayCodecSpecifierType GetType()
        {
            return iType;
        };
        virtual PVMFFormatType& GetFormat() = 0;

        bool CompareFormat(PVMFFormatType& aFormat, const char* aFormChar)
        {
            PVMFFormatType form = aFormChar;
            if (form == aFormat)
                return true;
            return false;
        }
    protected:
        CodecSpecifier() {};
        PV2WayCodecSpecifierType iType;
};


#endif
