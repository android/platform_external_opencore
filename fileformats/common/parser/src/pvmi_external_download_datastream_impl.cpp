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
#ifndef PVMI_EXTERNAL_DOWNLOAD_EXTENSION_INTERFACES_H_INCLUDED
#include "pvmi_external_download_extension_interfaces.h"
#endif

#define LOGDEBUG(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iLogger,PVLOGMSG_DEBUG,m);
#define LOGDATAPATH(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG,iDataPathLogger,PVLOGMSG_DEBUG,m);


////////////////////////////////////////////////////////////////////////////////////////
OSCL_EXPORT_REF PVMIDataStreamSyncInterface* PVMIExternalDownloadDataStreamImpl::CreateExternalDownloadDataStreamImpl(OSCL_wString& aFileName,
        Oscl_Vector<PVInterface*, OsclMemAllocator>& aIntfVec)
{
    PVMIExternalDownloadDataStreamImpl* newsyncif = OSCL_NEW(PVMIExternalDownloadDataStreamImpl, (aFileName, aIntfVec));
    return OSCL_STATIC_CAST(PVMIDataStreamSyncInterface*, newsyncif);
}


OSCL_EXPORT_REF PVMIDataStreamSyncInterface* PVMIExternalDownloadDataStreamImpl::CreateExternalDownloadDataStreamImpl(OsclFileHandle* aFileHandle,
        Oscl_Vector<PVInterface*, OsclMemAllocator>& aIntfVec)
{
    PVMIExternalDownloadDataStreamImpl* newsyncif = OSCL_NEW(PVMIExternalDownloadDataStreamImpl, (aFileHandle, aIntfVec));
    return OSCL_STATIC_CAST(PVMIDataStreamSyncInterface*, newsyncif);
}


OSCL_EXPORT_REF void PVMIExternalDownloadDataStreamImpl::DestroyDataStreamSyncInterfaceRefImpl(PVMIDataStreamSyncInterface* aInterface)
{
    PVMIExternalDownloadDataStreamImpl* syncif = OSCL_STATIC_CAST(PVMIExternalDownloadDataStreamImpl*, aInterface);
    OSCL_DELETE(syncif);
}


PVMIExternalDownloadDataStreamImpl::PVMIExternalDownloadDataStreamImpl(OSCL_wString& aFileName,
        Oscl_Vector<PVInterface*, OsclMemAllocator>& aIntfVec)
{
    iSessionID = 0;
    iFileName = aFileName;
    iFileHandle = NULL;
    iFileObject = NULL;
    iFs.Connect();

    iLogger = PVLogger::GetLoggerObject("PVMIExternalDownloadDataStreamImpl");
    iDataPathLogger = PVLogger::GetLoggerObject("datapath.sourcenode.PVMIExternalDownloadDataStreamImpl");

    iFileNumBytes = 0;
    iDownloadComplete = false;
    iDownloadSizeUpdateInterface = NULL;
    iReadCapacityCallbackCmdId = 0;
    iReadCapacityCBVec.clear();

    Oscl_Vector<PVInterface*, OsclMemAllocator>::iterator it;
    for (it = aIntfVec.begin(); it != aIntfVec.end(); it++)
    {
        if ((*it) == NULL) OSCL_LEAVE(OsclErrArgument);
        PVInterface* tmp = NULL;
        if ((*it)->queryInterface(PVMIExternalDownloadSizeUpdateInterfaceUUID, tmp))
        {
            iDownloadSizeUpdateInterface =
                OSCL_STATIC_CAST(PVMIExternalDownloadSizeUpdateInterface*, tmp);
        }
    }
    LOGDEBUG((0, "PVMIExternalDownloadDataStreamImpl::PVMIExternalDownloadDataStreamImpl"));
}

PVMIExternalDownloadDataStreamImpl::PVMIExternalDownloadDataStreamImpl(OsclFileHandle* aFileHandle,
        Oscl_Vector<PVInterface*, OsclMemAllocator>& aIntfVec)
{
    iSessionID = 0;
    iFileHandle = aFileHandle;
    iFileObject = NULL;
    iFs.Connect();

    iLogger = PVLogger::GetLoggerObject("PVMIExternalDownloadDataStreamImpl");
    iDataPathLogger = PVLogger::GetLoggerObject("datapath.sourcenode.PVMIExternalDownloadDataStreamImpl");

    iFileNumBytes = 0;
    iDownloadComplete = false;
    iDownloadSizeUpdateInterface = NULL;
    iReadCapacityCallbackCmdId = 0;
    iReadCapacityCBVec.clear();

    Oscl_Vector<PVInterface*, OsclMemAllocator>::iterator it;
    for (it = aIntfVec.begin(); it != aIntfVec.end(); it++)
    {
        if ((*it) == NULL) OSCL_LEAVE(OsclErrArgument);
        PVInterface* tmp = NULL;
        if ((*it)->queryInterface(PVMIExternalDownloadSizeUpdateInterfaceUUID, tmp))
        {
            iDownloadSizeUpdateInterface =
                OSCL_STATIC_CAST(PVMIExternalDownloadSizeUpdateInterface*, tmp);
        }
    }
    LOGDEBUG((0, "PVMIExternalDownloadDataStreamImpl::PVMIExternalDownloadDataStreamImpl"));
}

PVMIExternalDownloadDataStreamImpl::~PVMIExternalDownloadDataStreamImpl()
{
    LOGDEBUG((0, "PVMIExternalDownloadDataStreamImpl::~PVMIExternalDownloadDataStreamImpl"));
    if (iDownloadSizeUpdateInterface)
    {
        PVMIExternalDownloadSizeObserver* dnldObs =
            OSCL_STATIC_CAST(PVMIExternalDownloadSizeObserver*, this);
        iDownloadSizeUpdateInterface->RemoveObserver(dnldObs);
        iDownloadSizeUpdateInterface->removeRef();
    }
    if (iFileObject)
        OSCL_DELETE(iFileObject);
    iFileObject = NULL;
    iFs.Close();
    iLogger = NULL;
    iDataPathLogger = NULL;
}


OSCL_EXPORT_REF bool
PVMIExternalDownloadDataStreamImpl::queryInterface(const PVUuid& uuid,
        PVInterface*& iface)
{
    iface = NULL;
    LOGDEBUG((0, "PVMIExternalDownloadDataStreamImpl::queryInterface"));
    if (uuid == PVMIDataStreamSyncInterfaceUuid)
    {
        PVMIDataStreamSyncInterface* myInterface
        = OSCL_STATIC_CAST(PVMIDataStreamSyncInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        return true;
    }
    return false;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMIExternalDownloadDataStreamImpl::OpenSession(PvmiDataStreamSession& aSessionID,
        PvmiDataStreamMode aMode,
        bool nonblocking)
{
    OSCL_UNUSED_ARG(nonblocking);

    LOGDEBUG((0, "PVMIExternalDownloadDataStreamImpl::OpenSession"));
    if (!iFileObject)
    {
        if (iDownloadSizeUpdateInterface == NULL)
        {
            iFileObject = OSCL_NEW(Oscl_File, (OSCL_FILE_BUFFER_MAX_SIZE, iFileHandle));
        }
        else
        {
            //do not use a read cache in case the file is being
            //written simulataneously, since we do not have an
            //api to keep the read cache in sync
            iFileObject = OSCL_NEW(Oscl_File, (0, iFileHandle));
        }
    }

    int32 result;
    if (iFileHandle)
    {
        result = 0;
    }
    else
    {
        if (aMode == PVDS_READ_ONLY)
        {
            result = iFileObject->Open(iFileName.get_cstr(), Oscl_File::MODE_READ, iFs);
            LOGDEBUG((0, "PVMIExternalDownloadDataStreamImpl::OpenSession - File Open returning %d", result));
        }
        else
        {
            return PVDS_UNSUPPORTED_MODE;
        }
    }

    if (result == 0)
    {
        //compute filesize only if download sim is null
        //if download sim is present, it provides periodic updates of filesizes
        //when we hook up to the download backend it will do the same
        if (iDownloadSizeUpdateInterface == NULL)
        {
            iFileNumBytes = 0;
            int32 res = iFileObject->Seek(0, Oscl_File::SEEKEND);
            if (res == 0)
            {
                iFileNumBytes = (TOsclFileOffsetInt32)iFileObject->Tell();
                iFileObject->Seek(0, Oscl_File::SEEKSET);
            }
        }
        else
        {
            PVMIExternalDownloadSizeObserver* dnldObs =
                OSCL_STATIC_CAST(PVMIExternalDownloadSizeObserver*, this);
            iDownloadSizeUpdateInterface->SetObserver(dnldObs);
            iFileNumBytes = iDownloadSizeUpdateInterface->GetDownloadedFileSize();
            iDownloadComplete = iDownloadSizeUpdateInterface->IsDownloadComplete();
        }
        aSessionID = iSessionID;
        return PVDS_SUCCESS;
    }
    return PVDS_FAILURE;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMIExternalDownloadDataStreamImpl::CloseSession(PvmiDataStreamSession sessionID)
{
    OSCL_UNUSED_ARG(sessionID);
    if (iDownloadSizeUpdateInterface != NULL)
    {
        PVMIExternalDownloadSizeObserver* dnldObs =
            OSCL_STATIC_CAST(PVMIExternalDownloadSizeObserver*, this);
        iDownloadSizeUpdateInterface->RemoveObserver(dnldObs);
    }
    if (!iFileObject)
    {
        LOGDEBUG((0, "PVMIExternalDownloadDataStreamImpl::CloseSession returning %d", -1));
        return PVDS_FAILURE;
    }
    int32 result = 0;
    if (!iFileHandle)
        result = iFileObject->Close();
    LOGDEBUG((0, "PVMIExternalDownloadDataStreamImpl::CloseSession returning %d", result));
    OSCL_DELETE(iFileObject);
    iFileObject = NULL;
    if (result == 0)
        return PVDS_SUCCESS;

    return PVDS_FAILURE;
}

OSCL_EXPORT_REF PvmiDataStreamRandomAccessType
PVMIExternalDownloadDataStreamImpl::QueryRandomAccessCapability()
{
    return PVDS_FULL_RANDOM_ACCESS;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMIExternalDownloadDataStreamImpl::QueryReadCapacity(PvmiDataStreamSession sessionID,
        uint32& capacity)
{
    OSCL_UNUSED_ARG(sessionID);

    // Get the current file position
    uint32 currFilePosition = GetCurrentPointerPosition(iSessionID);

    // for projects on Symbian using RFileBuf cache enabled
    // we need to reload the filecache in symbian, since oscl fileio
    // does not have a sync / reload API we cheat by calling flush
    // It so happens that the flush impl on symbian does a "synch"
    iFileObject->Flush();

    // since the behaviour of fflush is undefined for read-only files
    // file pos may not be preserved. So seek back
    iFileObject->Seek((int32)(currFilePosition), Oscl_File::SEEKSET);

    capacity = ((uint32)iFileNumBytes - currFilePosition);

    if (iDownloadComplete == true)
    {
        return PVDS_END_OF_STREAM;
    }
    return PVDS_SUCCESS;
}

OSCL_EXPORT_REF PvmiDataStreamCommandId
PVMIExternalDownloadDataStreamImpl::RequestReadCapacityNotification(PvmiDataStreamSession sessionID,
        PvmiDataStreamObserver& observer,
        uint32 capacity,
        OsclAny* aContextData)
{
    OSCL_UNUSED_ARG(sessionID);
    PvmiDataStreamCommandId cmdId;
    if (iDownloadComplete == true)
    {
        OSCL_LEAVE(OsclErrInvalidState);
    }
    //get current filesize
    //the capacity passed here means that the entity using the read datastream
    //wants "capacity" number of bytes FROM its CURRENT location
    //Read datastream's current read location cannot exceed downloaded file size
    uint32 currreadpos = GetCurrentPointerPosition(0);
    uint32 finalreadpositionforthisrequest = currreadpos + capacity;
    if ((uint32)iFileNumBytes >= finalreadpositionforthisrequest)
    {
        //this request should never have been sent
        //there is enough data in the datastream to be read
        OSCL_LEAVE(OsclErrArgument);
    }
    //accept the request - request will be completed when download updates happen
    cmdId = iReadCapacityCallbackCmdId++;
    PVMIExternalDownloadDataStreamReadCallBackParams params;
    params.iId = cmdId;
    params.iRequestedFileSize = finalreadpositionforthisrequest;
    params.iObserver = &observer;
    params.iContext = aContextData;
    iReadCapacityCBVec.push_back(params);
    return cmdId;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMIExternalDownloadDataStreamImpl::QueryWriteCapacity(PvmiDataStreamSession sessionID,
        uint32& capacity)
{
    OSCL_UNUSED_ARG(sessionID);
    OSCL_UNUSED_ARG(capacity);

    return PVDS_NOT_SUPPORTED;
}

OSCL_EXPORT_REF PvmiDataStreamCommandId
PVMIExternalDownloadDataStreamImpl::RequestWriteCapacityNotification(PvmiDataStreamSession sessionID,
        PvmiDataStreamObserver& observer,
        uint32 capacity,
        OsclAny* aContextData)
{
    OSCL_UNUSED_ARG(sessionID);
    OSCL_UNUSED_ARG(observer);
    OSCL_UNUSED_ARG(capacity);
    OSCL_UNUSED_ARG(aContextData);

    OSCL_LEAVE(OsclErrNotSupported);
    return 0;
}

OSCL_EXPORT_REF PvmiDataStreamCommandId
PVMIExternalDownloadDataStreamImpl::CancelNotification(PvmiDataStreamSession sessionID,
        PvmiDataStreamObserver& observer,
        PvmiDataStreamCommandId aID,
        OsclAny* aContextData)
{
    OSCL_UNUSED_ARG(sessionID);
    OSCL_UNUSED_ARG(observer);
    OSCL_UNUSED_ARG(aID);
    OSCL_UNUSED_ARG(aContextData);

    OSCL_LEAVE(OsclErrNotSupported);
    return 0;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMIExternalDownloadDataStreamImpl::CancelNotificationSync(PvmiDataStreamSession sessionID)
{
    //clear out all pending notifications
    iReadCapacityCBVec.clear();
    return PVDS_SUCCESS;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMIExternalDownloadDataStreamImpl::Read(PvmiDataStreamSession sessionID,
        uint8* buffer,
        uint32 size,
        uint32& numelements)
{
    OSCL_UNUSED_ARG(sessionID);

    if (!iFileObject)
        return PVDS_FAILURE;
    uint32 result = iFileObject->Read(buffer, size, numelements);
    LOGDEBUG((0, "PVMIExternalDownloadDataStreamImpl::ReadAndUnlockContent returning %d", result));
    numelements = result;
    return PVDS_SUCCESS;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMIExternalDownloadDataStreamImpl::Write(PvmiDataStreamSession sessionID,
        uint8* buffer,
        uint32 size,
        uint32& numelements)
{
    OSCL_UNUSED_ARG(sessionID);
    OSCL_UNUSED_ARG(buffer);
    OSCL_UNUSED_ARG(size);
    OSCL_UNUSED_ARG(numelements);

    return PVDS_NOT_SUPPORTED;
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMIExternalDownloadDataStreamImpl::Seek(PvmiDataStreamSession sessionID,
        int32 offset,
        PvmiDataStreamSeekType origin)
{
    OSCL_UNUSED_ARG(sessionID);

    if (!iFileObject)
        return PVDS_FAILURE;
    Oscl_File::seek_type seekType = Oscl_File::SEEKCUR;
    if (origin == PVDS_SEEK_SET)
    {
        seekType = Oscl_File::SEEKSET;
    }
    if (origin == PVDS_SEEK_CUR)
    {
        seekType = Oscl_File::SEEKCUR;
    }
    if (origin == PVDS_SEEK_END)
    {
        seekType = Oscl_File::SEEKEND;
    }
    int32 result = iFileObject->Seek(offset, seekType);
    LOGDEBUG((0, "PVMIExternalDownloadDataStreamImpl::SeekContent returning %d", result));
    if (result != 0)
    {
        return PVDS_FAILURE;
    }
    return PVDS_SUCCESS;
}

OSCL_EXPORT_REF uint32
PVMIExternalDownloadDataStreamImpl::GetCurrentPointerPosition(PvmiDataStreamSession sessionID)
{
    OSCL_UNUSED_ARG(sessionID);

    if (!iFileObject)
        return 0;
    int32 result = (TOsclFileOffsetInt32)iFileObject->Tell();
    LOGDEBUG((0, "PVMIExternalDownloadDataStreamImpl::GetCurrentContentPosition returning %d", result));
    return (uint32)(result);
}

OSCL_EXPORT_REF PvmiDataStreamStatus
PVMIExternalDownloadDataStreamImpl::Flush(PvmiDataStreamSession sessionID)
{
    OSCL_UNUSED_ARG(sessionID);

    if (!iFileObject)
    {
        LOGDEBUG((0, "PVMIExternalDownloadDataStreamImpl::Flush returning %d", -1));
        return PVDS_FAILURE;
    }
    int32 result;
    result = iFileObject->Flush();
    LOGDEBUG((0, "PVMIExternalDownloadDataStreamImpl::Flush returning %d", result));
    if (result == 0) //Flush will return 0 when successful
        return PVDS_SUCCESS;
    else
        return PVDS_FAILURE;

}

void PVMIExternalDownloadDataStreamImpl::DownloadUpdate(uint32 aLatestFileSizeInBytes, bool aDownloadComplete)
{
    LOGDATAPATH((0, "PVMIExternalDownloadDataStreamImpl::DownloadUpdate - LatestFileSize=%d, DnldComplete=%d",
                 aLatestFileSizeInBytes, aDownloadComplete));

    iFileNumBytes = (int32)aLatestFileSizeInBytes;
    //check if download just completed
    if ((iDownloadComplete == false) && (aDownloadComplete == true))
    {
        if (iReadCapacityCBVec.size() > 0)
        {
            Oscl_Vector<PVMIExternalDownloadDataStreamReadCallBackParams, OsclMemAllocator>::iterator it;
            for (it = iReadCapacityCBVec.begin(); it != iReadCapacityCBVec.end(); it++)
            {
                PVMFAsyncEvent aEvent = PVMFAsyncEvent(PVMFInfoEvent,
                                                       PVMFInfoBufferingComplete,
                                                       it->iContext,
                                                       NULL);
                it->iObserver->DataStreamInformationalEvent(aEvent);
            }
        }
    }
    iDownloadComplete = aDownloadComplete;
    if (iReadCapacityCBVec.size() > 0)
    {
        Oscl_Vector<PVMIExternalDownloadDataStreamReadCallBackParams, OsclMemAllocator>::iterator it;
        it = iReadCapacityCBVec.begin();
        while (it != iReadCapacityCBVec.end())
        {
            PVMFStatus status = PVMFPending;
            if (aLatestFileSizeInBytes > it->iRequestedFileSize)
            {
                status = PVMFSuccess;
            }
            else
            {
                if (iDownloadComplete == true)
                {
                    //go ahead and succeed this request as it is possible that
                    //sometimes caller may not know the complete file size upfront.
                    //in case it turns out the caller needs more data
                    //they would make another request which would not be accepted
                    //since download has completed.
                    status = PVMFSuccess;
                }
            }
            if (status != PVMFPending)
            {
                // Form a command response.
                PVMFCmdResp resp(it->iId,
                                 it->iContext,
                                 status,
                                 NULL,
                                 NULL);
                // Make the Command Complete notification.
                it->iObserver->DataStreamCommandCompleted(resp);
                it = iReadCapacityCBVec.erase(it);
            }
            else
            {
                it++;
            }
        }
    }
}

