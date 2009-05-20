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
#ifndef PVMI_EXTERNAL_DOWNLOAD_DATASTREAM_FACTORY_H_INCLUDED
#include "pvmi_external_download_datastream_factory.h"
#endif
#ifndef PVMI_EXTERNAL_DOWNLOAD_DATASTREAM_IMPL_H_INCLUDED
#include "pvmi_external_download_datastream_impl.h"
#endif


OSCL_EXPORT_REF PVMIExternalDownloadDataStreamInterfaceFactory::PVMIExternalDownloadDataStreamInterfaceFactory(OSCL_wString& aFileName,
        Oscl_Vector<PVInterface*, OsclMemAllocator>& aIntfVec)
{
    iFileName = aFileName;
    iFileHandle = NULL;
    //go ahead and save the interface pointers, we will pass them as is to underlying DSImpl
    //we do not intentionally pick and choose the ones supported by underlying implementation here
    //in case underlying implementation changes to add support for more interfaces then this bit of code can be unaware of it
    iVec = aIntfVec;
    Oscl_Vector<PVInterface*, OsclMemAllocator>::iterator it;
    for (it = iVec.begin(); it < iVec.end(); it++)
    {
        (*it)->addRef();
    }
}

OSCL_EXPORT_REF PVMIExternalDownloadDataStreamInterfaceFactory::PVMIExternalDownloadDataStreamInterfaceFactory(OsclFileHandle* aFileHandle,
        Oscl_Vector<PVInterface*, OsclMemAllocator>& aIntfVec)
{
    iFileHandle = aFileHandle;
    iFileName = _STRLIT_WCHAR("");
    //go ahead and save the interface pointers, we will pass them as is to underlying DSImpl
    //we do not intentionally pick and choose the ones supported by underlying implementation here
    //in case underlying implementation changes to add support for more interfaces then this bit of code can be unaware of it
    iVec = aIntfVec;
    Oscl_Vector<PVInterface*, OsclMemAllocator>::iterator it;
    for (it = iVec.begin(); it < iVec.end(); it++)
    {
        (*it)->addRef();
    }
}

OSCL_EXPORT_REF PVMIExternalDownloadDataStreamInterfaceFactory::~PVMIExternalDownloadDataStreamInterfaceFactory()
{
    Oscl_Vector<PVInterface*, OsclMemAllocator>::iterator it;
    for (it = iVec.begin(); it < iVec.end(); it++)
    {
        (*it)->removeRef();
    }
    iVec.clear();
}


PVMFStatus PVMIExternalDownloadDataStreamInterfaceFactory::QueryAccessInterfaceUUIDs(Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids)
{
    aUuids.push_back(PVMIDataStreamSyncInterfaceUuid);
    return PVMFSuccess;
}


PVInterface* PVMIExternalDownloadDataStreamInterfaceFactory::CreatePVMFCPMPluginAccessInterface(PVUuid& aUuid)
{
    if (aUuid == PVMIDataStreamSyncInterfaceUuid)
    {
        // Create the ref data stream sync interface object using ref impl's static create method
        PVMIDataStreamSyncInterface* datastreamptr = NULL;
        if (iFileHandle)
        {
            datastreamptr = PVMIExternalDownloadDataStreamImpl::CreateExternalDownloadDataStreamImpl(iFileHandle, iVec);
        }
        else
        {
            datastreamptr = PVMIExternalDownloadDataStreamImpl::CreateExternalDownloadDataStreamImpl(iFileName, iVec);
        }

        return (OSCL_STATIC_CAST(PVInterface*, datastreamptr));
    }
    return NULL;
}


void PVMIExternalDownloadDataStreamInterfaceFactory::DestroyPVMFCPMPluginAccessInterface(PVUuid& aUuid, PVInterface* aPtr)
{
    if (aUuid == PVMIDataStreamSyncInterfaceUuid)
    {
        // Release the data stream sync interface object using ref impl's static destroy method
        PVMIDataStreamSyncInterface* interimPtr = (PVMIDataStreamSyncInterface*)aPtr;
        PVMIExternalDownloadDataStreamImpl::DestroyDataStreamSyncInterfaceRefImpl(interimPtr);
    }
}


void PVMIExternalDownloadDataStreamInterfaceFactory::addRef()
{
    // Nothing needed
}


void PVMIExternalDownloadDataStreamInterfaceFactory::removeRef()
{
    // Nothing needed
}


bool PVMIExternalDownloadDataStreamInterfaceFactory::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    iface = NULL;
    if (uuid == PVMFCPMPluginAccessInterfaceFactoryUuid)
    {
        // Return the current object as data stream factory pointer
        PVMFDataStreamFactory* myInterface = OSCL_STATIC_CAST(PVMFDataStreamFactory*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        return true;
    }
    return false;
}

