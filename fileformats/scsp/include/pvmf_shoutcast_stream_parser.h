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
#ifndef PVMF_SHOUTCAST_STREAM_PARSER_H_INCLUDED
#define PVMF_SHOUTCAST_STREAM_PARSER_H_INCLUDED

#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif
#ifndef OSCL_QUEUE_H_INCLUDED
#include "oscl_queue.h"
#endif
#ifndef OSCL_ERROR_CODES_H_INCLUDED
#include "oscl_error_codes.h"
#endif
#ifndef OSCL_TYPES_H_INCLUDED
#include "oscl_types.h"
#endif
#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif
#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif
#ifndef PVMF_EVENT_HANDLING_H_INCLUDED
#include "pvmf_event_handling.h"
#endif
#ifndef PVMI_DATA_STREAM_INTERFACE_H_INCLUDED
#include "pvmi_data_stream_interface.h"
#endif
#ifndef PVMF_CPMPLUGIN_ACCESS_INTERFACE_FACTORY_H_INCLUDED
#include "pvmf_cpmplugin_access_interface_factory.h"
#endif
#ifndef PVMI_DATASTREAMUSER_INTERFACE_H_INCLUDED
#include "pvmi_datastreamuser_interface.h"
#endif
#ifndef PVMF_MEDIA_DATA_H_INCLUDED
#include "pvmf_media_data.h"
#endif

#define PV_SCSP_NUMBER_OF_READ_CONNECTIONS  16

// includes the length byte
#define PV_SCSP_MAX_METADATA_TAG_SIZE                   4080
#define PV_SCSP_MAX_METADATA_TAG_SIZE_WITH_LENGTH_BYTE  PV_SCSP_MAX_METADATA_TAG_SIZE + 1

class PVMFMediaChunkMap
{
    public:

        PVMFMediaChunkMap();

        ~PVMFMediaChunkMap() {};

        void SetMapInfo(uint32 aMaxChunks, uint32 aChunkSize, uint32* aMapPtr);

        void ClearMapInfo();

        void ResetMapInfo();

        uint32 GetBytesMapped(uint32 aStreamOffset);

        void SetNextChunkOffset(uint32 aStreamOffset);

        bool GetChunkOffset(uint32 aStreamOffset, uint32& aThisChunkStreamOffset, uint32*& aThisEntry);

        bool GetNextChunkOffset(uint32 aStreamOffset, uint32& aNextChunkStreamOffset);

        bool GetPrevChunkOffset(uint32 aStreamOffset, uint32& aPrevChunkStreamOffset);

        uint32 GetBytesLeftInRange(uint32 aFirstOffset, uint32 aLastOffset, uint32 aOffset);

    private:

        // pointer to the beginning (first uin32) of allocated memory
        // this does not change once it is set in SetMapInfo
        uint32* iFirstEntry;
        // pointer to the end (last uint32) of the allocated memory
        // this does not change once it is set in SetMapInfo
        uint32* iLastEntry;
        // pointer to where the next media chunk stream offset should be stored
        // this is the start of the map
        uint32* iNextEntry;
        // pointer to where the current media chunk stream offset is stored
        // this is the end of the map
        uint32* iCurrentEntry;
        // max number of media chunks in the map
        // this does not change once it is set in SetMapInfo
        uint32 iMaxChunks;
        // media chunk size
        // this does not change once it is set in SetMapInfo
        uint32 iChunkSize;
        // flag to indicate if iCurrentEntry has gone past the end of the map and back to the beginning
        bool iWrapped;

        PVLogger* iLogger;
};


typedef struct _MetadataInfo
{
    uint8* iBuffer;

    uint32 iSize;
} MetadataInfo;


class PVMFMetadataUpdatesObserver
{
    public:
        virtual void MetadataUpdated(uint32 aMetadataSize) = 0;
        virtual ~PVMFMetadataUpdatesObserver() {}
};

class PVMFShoutcastStreamParser;

typedef struct _ReadFilePositionStruct
{
    bool iReadPositionStructValid;

    uint32 iParserFilePosition;

    uint32 iStreamPosition;

    uint32 iStreamSessionID;

    PVMFMetadataUpdatesObserver *iMetadataObserver;

    uint32 iMetadataObserverBufSize;

    uint8* iMetadataObserverBufPtr;

    PVMFShoutcastStreamParser* iSCSP;

} ReadFilePositionStruct;


typedef struct _ReadFilePositionVec
{
    ReadFilePositionStruct iReadFilePos[PV_SCSP_NUMBER_OF_READ_CONNECTIONS];
} ReadFilePositionVec;


class PVMFShoutcastStreamParserFactory : public PVMFCPMPluginAccessInterfaceFactory
{
    public:
        OSCL_IMPORT_REF PVMFShoutcastStreamParserFactory(PVMFDataStreamFactory* aDataStreamFactory, uint32 aMetadataInterval);

        OSCL_IMPORT_REF ~PVMFShoutcastStreamParserFactory();

        // From PVInterface
        void addRef() {};

        // From PVInterface
        void removeRef() {};

        // From PVInterface
        bool queryInterface(const PVUuid& uuid, PVInterface*& iface)
        {
            OSCL_UNUSED_ARG(uuid);
            OSCL_UNUSED_ARG(iface);
            return false;
        };

        // From PVMFDataStreamFactory aka PVMFCPMPluginAccessInterfaceFactory
        OSCL_IMPORT_REF PVMFStatus QueryAccessInterfaceUUIDs(Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids);

        // From PVMFDataStreamFactory
        OSCL_IMPORT_REF PVInterface* CreatePVMFCPMPluginAccessInterface(PVUuid& aUuid);

        // From PVMFDataStreamFactory
        OSCL_IMPORT_REF void DestroyPVMFCPMPluginAccessInterface(PVUuid& aUuid, PVInterface* aPtr);

        // Reset all pointers to 0 and clear media map info
        // This is used in Start after Stop
        OSCL_IMPORT_REF PVMFStatus ResetShoutcastStream();

    private:

        struct ReadStreamStruct
        {
            PVMFShoutcastStreamParser* iSCSP;
            PVMIDataStreamSyncInterface* iDataStream;
        };

        PVMFDataStreamFactory* iDataStreamFactory;

        uint32 iMetadataInterval;

        Oscl_Vector<ReadStreamStruct*, OsclMemAllocator> iReadStreamVec;

        uint32* iMediaMapMemPtr;

        PVMFMediaChunkMap iMediaChunkMap;

        MetadataInfo iMetadataInfo;

        ReadFilePositionVec iReadFilePosVec;

        PVLogger* iLogger;
};


class PVMFShoutcastStreamParser : public PVMIDataStreamSyncInterface
{
    public:
        OSCL_IMPORT_REF PVMFShoutcastStreamParser(PVMIDataStreamSyncInterface* aDataStream, uint32 aMetadataInterval,
                PVMFMediaChunkMap& aMediaMap, MetadataInfo& aMetaInfo,
                ReadFilePositionVec& aReadFilePosVec);

        OSCL_IMPORT_REF ~PVMFShoutcastStreamParser();

        // From PVInterface
        void addRef() {};

        // From PVInterface
        void removeRef() {};

        // From PVInterface
        bool queryInterface(const PVUuid& uuid, PVInterface*& iface);

        // From PVMIDataStreamSyncInterface
        OSCL_IMPORT_REF PvmiDataStreamStatus OpenSession(PvmiDataStreamSession& aSessionID,
                PvmiDataStreamMode aMode,
                bool nonblocking = false);

        OSCL_IMPORT_REF PvmiDataStreamStatus CloseSession(PvmiDataStreamSession aSessionID);

        OSCL_IMPORT_REF PvmiDataStreamRandomAccessType QueryRandomAccessCapability();

        OSCL_IMPORT_REF PvmiDataStreamStatus QueryReadCapacity(PvmiDataStreamSession aSessionID,
                uint32& capacity);

        OSCL_IMPORT_REF PvmiDataStreamCommandId RequestReadCapacityNotification(PvmiDataStreamSession aSessionID,
                PvmiDataStreamObserver& observer,
                uint32 capacity,
                OsclAny* aContextData = NULL);

        OSCL_IMPORT_REF PvmiDataStreamStatus QueryWriteCapacity(PvmiDataStreamSession aSessionID,
                uint32& capacity);

        OSCL_IMPORT_REF PvmiDataStreamCommandId RequestWriteCapacityNotification(PvmiDataStreamSession aSessionID,
                PvmiDataStreamObserver& observer,
                uint32 capacity,
                OsclAny* aContextData = NULL);

        OSCL_IMPORT_REF PvmiDataStreamCommandId CancelNotification(PvmiDataStreamSession aSessionID,
                PvmiDataStreamObserver& observer,
                PvmiDataStreamCommandId aID,
                OsclAny* aContextData = NULL);

        OSCL_IMPORT_REF PvmiDataStreamStatus CancelNotificationSync(PvmiDataStreamSession aSessionID);

        OSCL_IMPORT_REF PvmiDataStreamStatus Read(PvmiDataStreamSession aSessionID, uint8* buffer,
                uint32 size, uint32& numelements);

        OSCL_IMPORT_REF PvmiDataStreamStatus Write(PvmiDataStreamSession aSessionID, uint8* buffer,
                uint32 size, uint32& numelements);

        OSCL_IMPORT_REF PvmiDataStreamStatus Write(PvmiDataStreamSession aSessionID, OsclRefCounterMemFrag* frag,
                uint32& aNumElements);

        OSCL_IMPORT_REF PvmiDataStreamStatus Seek(PvmiDataStreamSession aSessionID,
                int32 offset, PvmiDataStreamSeekType origin);

        OSCL_IMPORT_REF uint32 GetCurrentPointerPosition(PvmiDataStreamSession aSessionID);

        OSCL_IMPORT_REF PvmiDataStreamStatus Flush(PvmiDataStreamSession aSessionID);

        OSCL_IMPORT_REF uint32 QueryBufferingCapacity();

        OSCL_IMPORT_REF PvmiDataStreamStatus MakePersistent(int32 aOffset, uint32 aSize);

        OSCL_IMPORT_REF void GetCurrentByteRange(uint32& aCurrentFirstByteOffset, uint32& aCurrentLastByteOffset);

        OSCL_IMPORT_REF PvmiDataStreamStatus RequestMetadataUpdates(PvmiDataStreamSession aSessionID,
                PVMFMetadataUpdatesObserver& aObserver,
                uint32 aBufSize,
                uint8 *aBufPtr);

        OSCL_IMPORT_REF PvmiDataStreamStatus ResetReadPosition(PvmiDataStreamSession aSessionID);

    private:

        void ManageMetadataUpdateNotifications();

        PvmiDataStreamStatus HandleMetadata(PvmiDataStreamSession aSessionID, uint32 aFirstStreamOffset, uint32 aLastStreamOffset,
                                            uint32 aMetadataLengthBase16);

        PvmiDataStreamStatus GetBytesInChunk(PvmiDataStreamSession aSessionID, uint32 aLastStreamOffset, uint32& aBytesInChunk);

        PvmiDataStreamStatus ReadBytes(PvmiDataStreamSession aSessionID, uint32 aFirstStreamOffset, uint32 aLastStreamOffset,
                                       uint8* aBufPtr, uint32& aBytesToRead, bool bUpdateParsingPosition);

        PvmiDataStreamStatus SkipRelativeBytes(PvmiDataStreamSession aSessionID, uint32 aFirstStreamOffset, uint32 aLastStreamOffset,
                                               int32 aSeekToOffset, bool bUpdateParsingPosition);

    private:

        uint32 iMetadataInterval;

        PVMIDataStreamSyncInterface* iReadStream;

        PVMFMediaChunkMap *iMediaChunkMap;

        MetadataInfo* iMetadataInfo;

        ReadFilePositionVec *iReadFilePosVec;

        PVLogger* iLogger;

        // For QueryReadCapacity
        uint32 iLargestMetadataTag;
        // For QueryReadCapacity
        uint32 iLargestRead;
        // For QueryReadCapacity
        uint32 iBytesToNextMetadataTag;

};

#endif // PVMF_SHOUTCAST_STREAM_PARSER_H_INCLUDED


