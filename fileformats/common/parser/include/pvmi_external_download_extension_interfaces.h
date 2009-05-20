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
#ifndef PVMI_EXTERNAL_DOWNLOAD_EXTENSION_INTERFACES_H_INCLUDED
#define PVMI_EXTERNAL_DOWNLOAD_EXTENSION_INTERFACES_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif
#ifndef PV_INTERFACE_H
#include "pv_interface.h"
#endif

/*
 * This is an observer to recv updates on filesize and download completion status
 */
class PVMIExternalDownloadSizeObserver
{
    public:
        virtual void DownloadUpdate(uint32 aLatestFileSizeInBytes, bool aDownloadComplete) = 0;
};

#define PVMIExternalDownloadSizeUpdateInterfaceUUID PVUuid(0x417da9d4, 0x3200, 0x450b, 0x95, 0x57, 0x95, 0x17, 0x54, 0x57, 0x66, 0xc5)

class PVMIExternalDownloadSizeUpdateInterface : public PVInterface
{
    public:
        virtual void SetObserver(PVMIExternalDownloadSizeObserver* aObs) = 0;
        virtual void RemoveObserver(PVMIExternalDownloadSizeObserver* aObs) = 0;
        virtual uint32 GetDownloadedFileSize() = 0;
        virtual bool IsDownloadComplete() = 0;
};

#endif //PVMI_EXTERNAL_DOWNLOAD_EXTENSION_INTERFACES_H_INCLUDED


