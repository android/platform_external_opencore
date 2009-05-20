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
#ifndef OSCL_EXCEPTION_H_INCLUDED
#include "oscl_exception.h"
#endif

#ifndef PVMF_SHOUTCAST_STREAM_PARSER_H_INCLUDED
#include "pvmf_shoutcast_stream_parser.h"
#endif

// logging
#define LOGDEBUG(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_VERBOSE, m);
#define LOGERROR(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, m);
#define LOGTRACE(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, m);

#define LOG_FOR_DEBUGGING       0     // set to 1 to enable and 0 to disable debug logging at PVLOGMSG_ERR

// to handle 32 bit 4GB wrap around
#define WRAP_THRESHOLD 0x80000000

// returns true in inRange, if
// thisOffset is in the range of bytes,
// first byte being firstRangeOffset and last byte being lastRangeOffset
#define IS_OFFSET_IN_RANGE(firstRangeOffset, lastRangeOffset, thisOffset, inRange)      \
    {                                                                                   \
        inRange = true;                                                                 \
        if ((firstRangeOffset >= WRAP_THRESHOLD) && (lastRangeOffset < WRAP_THRESHOLD)) \
        {                                                                               \
            if ((thisOffset >= WRAP_THRESHOLD) && (thisOffset < firstRangeOffset))      \
            {                                                                           \
                inRange = false;                                                        \
            }                                                                           \
            else if ((thisOffset < WRAP_THRESHOLD) && (thisOffset > lastRangeOffset))   \
            {                                                                           \
                inRange = false;                                                        \
            }                                                                           \
        }                                                                               \
        else if ((thisOffset < firstRangeOffset) || (thisOffset > lastRangeOffset))     \
        {                                                                               \
            inRange = false;                                                            \
        }                                                                               \
    }

/////////////////////////////////////////////////////////////////////////////////////////////

PVMFMediaChunkMap::PVMFMediaChunkMap()
{
    iLogger = PVLogger::GetLoggerObject("PVMFShoutcastStreamParser");

    LOGTRACE((0, "PVMFMediaChunkMap::PVMFMediaChunkMap"));

    ClearMapInfo();
}

// Set up the map pointers, chunk size and max number of chunks
void PVMFMediaChunkMap::SetMapInfo(uint32 aMaxChunks, uint32 aChunkSize, uint32* aMapPtr)
{
    LOGTRACE((0, "PVMFMediaChunkMap::SetMapInfo - max chunks %d, chunk size %d mem ptr %d", aMaxChunks, aChunkSize, aMapPtr));

    iMaxChunks = aMaxChunks;
    iChunkSize = aChunkSize;

    iNextEntry = iFirstEntry = iCurrentEntry = aMapPtr;
    iLastEntry = iFirstEntry + (iMaxChunks - 1);

    *iFirstEntry = 0;
    *iLastEntry = 0;

    iWrapped = false;
}

void PVMFMediaChunkMap::ClearMapInfo()
{
    LOGTRACE((0, "PVMFMediaChunkMap::ClearMapInfo"));

    iMaxChunks = 0;
    iChunkSize = 0;

    iFirstEntry = NULL;
    iLastEntry = NULL;
    iNextEntry = NULL;
    iCurrentEntry = NULL;

    iWrapped = false;
}

void PVMFMediaChunkMap::ResetMapInfo()
{
    LOGTRACE((0, "PVMFMediaChunkMap::ResetMapInfo"));

    // iFirstEntry and iLastEntry stay the same
    iNextEntry = iCurrentEntry = iFirstEntry;

    // start from 0 offset again
    *iFirstEntry = 0;
    *iLastEntry = 0;

    iWrapped = false;
}

// Returns the total number of bytes in the map
// starting from and including the specified offset
// If the offset is not in the map, 0 is returned
uint32 PVMFMediaChunkMap::GetBytesMapped(uint32 aStreamOffset)
{
    uint32 bytesMapped = 0;

    // the map is a circular list
    // next to find the start of the map
    // the end of map is pointed to by iCurrentEntry
    uint32* startEntry = iWrapped ? iNextEntry : iFirstEntry;

    uint32 firstStreamOffsetInMap = *startEntry;
    uint32 lastStreamOffsetInMap = (*iCurrentEntry) + iChunkSize - 1;

    LOGDEBUG((0, "PVMFMediaChunkMap::GetBytesMapped - stream offset %d, first map offset %d, last map offset %d",
              aStreamOffset, firstStreamOffsetInMap, lastStreamOffsetInMap));

    bool inMap = false;
    IS_OFFSET_IN_RANGE(firstStreamOffsetInMap, lastStreamOffsetInMap, aStreamOffset, inMap);
    if (inMap)
    {
        // stream offset may be in the map
        // find the chunk that the offset is in
        uint32* tmpEntry = startEntry;

        bool found = false;
        while (!found)
        {
            uint32 firstStreamOffsetInChunk = *tmpEntry;
            uint32 lastStreamOffsetInChunk = firstStreamOffsetInChunk + iChunkSize - 1;
            bool inChunk = false;
            IS_OFFSET_IN_RANGE(firstStreamOffsetInChunk, lastStreamOffsetInChunk, aStreamOffset, inChunk);
            if (inChunk)
            {
                found = true;
                // offset is in this chunk
                // find out how many bytes are mapped from offset on
                // number of chunks * chunk size
                uint32 entries = 0;
                if (iWrapped)
                {
                    if (iCurrentEntry == iLastEntry)
                    {
                        // map is about to wrap around
                        entries = iLastEntry - tmpEntry;
                    }
                    else if ((iLastEntry >= tmpEntry) && (tmpEntry > iCurrentEntry))
                    {
                        // map has fully wrapped around
                        // tmpEntry has not wrapped
                        entries = (iLastEntry - tmpEntry) + (iCurrentEntry - iFirstEntry) + 1;
                    }
                    else
                    {
                        // map has fully wrapped around
                        // tmpEntry has wrapped
                        entries = iCurrentEntry - tmpEntry;
                    }
                }
                else
                {
                    entries = iCurrentEntry - tmpEntry;
                }

                bytesMapped = (entries * iChunkSize) + GetBytesLeftInRange(firstStreamOffsetInChunk, lastStreamOffsetInChunk, aStreamOffset);
                // done
                break;
            }

            // not found, look at the next entry
            if (tmpEntry == iCurrentEntry)
            {
                // at the end of map
                // not in the media chunks
                // offset is in the metadata tag?
                LOGERROR((0, "PVMFMediaChunkMap::GetBytesMapped stream offset %d not found in map", aStreamOffset));
                break;
            }
            else if (tmpEntry != iLastEntry)
            {
                tmpEntry++;
            }
            else
            {
                tmpEntry = iFirstEntry;
            }
        }
    }

    LOGTRACE((0, "PVMFMediaChunkMap::GetBytesMapped - stream offset %d bytes mapped %d", aStreamOffset, bytesMapped));

    return bytesMapped;
}

// Set the starting stram offset for the next media chunk in map

void PVMFMediaChunkMap::SetNextChunkOffset(uint32 aStreamOffset)
{
    LOGTRACE((0, "PVMFMediaChunkMap::SetNextChunkOffset - stream offset %d", aStreamOffset));

    // when there is only one entry in the map
    // first, current and next are all the same
    // update the next pointer first
    if ((!iWrapped) && (iCurrentEntry == iFirstEntry) && (iNextEntry == iFirstEntry))
    {
        iNextEntry++;
    }

    *iNextEntry = aStreamOffset;

    // update the pointers
    iCurrentEntry = iNextEntry;
    if (iCurrentEntry == iLastEntry)
    {
        iNextEntry = iFirstEntry;
        iWrapped = true;
    }
    else
    {
        iNextEntry++;
    }
}

// Return the start offset of the chunk that the specified stream offset is on
// also return the pointer to the map entry

bool PVMFMediaChunkMap::GetChunkOffset(uint32 aStreamOffset, uint32& aThisChunkStreamOffset, uint32*& aThisEntry)
{
    uint32* startEntry = iWrapped ? iNextEntry : iFirstEntry;

    uint32 firstStreamOffsetInMap = *startEntry;
    uint32 lastStreamOffsetInMap = (*iCurrentEntry) + iChunkSize - 1;

    bool inMap = false;
    IS_OFFSET_IN_RANGE(firstStreamOffsetInMap, lastStreamOffsetInMap, aStreamOffset, inMap);
    if (inMap)
    {
        // stream offset may be in the map
        // find the chunk that the offset is in
        uint32* tmpEntry = startEntry;

        while (1)
        {
            uint32 firstStreamOffsetInChunk = *tmpEntry;
            uint32 lastStreamOffsetInChunk = firstStreamOffsetInChunk + iChunkSize - 1;
            bool inChunk = false;

            IS_OFFSET_IN_RANGE(firstStreamOffsetInChunk, lastStreamOffsetInChunk, aStreamOffset, inChunk);
            if (inChunk)
            {
                aThisChunkStreamOffset = *tmpEntry;

                aThisEntry = tmpEntry;

                LOGTRACE((0, "PVMFMediaChunkMap::GetChunkOffset - stream offset %d, chunk offset %d, entry ptr %x",
                          aStreamOffset, aThisChunkStreamOffset, aThisEntry));
                return true;
            }

            if (tmpEntry == iCurrentEntry)
            {
                // at the end of map
                // not in the media chunks
                // offset is in the metadata tag?
                break;
            }
            else if (tmpEntry != iLastEntry)
            {
                tmpEntry++;
            }
            else
            {
                tmpEntry = iFirstEntry;
            }
        } // end while
    }

    // not in map
    LOGTRACE((0, "PVMFMediaChunkMap::GetChunkOffset - stream offset %d not found in map", aStreamOffset));

    return false;
}

// Locate the media chunk containing the specified offset and return the start offset of the next chunk

bool PVMFMediaChunkMap::GetNextChunkOffset(uint32 aStreamOffset, uint32& aNextChunkStreamOffset)
{
    uint32* thisEntry = NULL;
    uint32 thisChunkOffset = 0;

    // find the offset of the chunk that the specified offset is in
    if (GetChunkOffset(aStreamOffset, thisChunkOffset, thisEntry))
    {
        if (thisEntry != iCurrentEntry)
        {
            aNextChunkStreamOffset = *(++thisEntry);

            LOGTRACE((0, "PVMFMediaChunkMap::GetNextChunkOffset - stream offset %d, next chunk offset %d", aStreamOffset,  aNextChunkStreamOffset));

            return true;
        }
    }

    // not in map
    LOGTRACE((0, "PVMFMediaChunkMap::GetNextChunkOffset - stream offset %d not found in map", aStreamOffset));

    return false;
}

// Locate the media chunk containing the specified offset and return the start offset of the previous chunk

bool PVMFMediaChunkMap::GetPrevChunkOffset(uint32 aStreamOffset, uint32& aPrevChunkStreamOffset)
{
    uint32* thisEntry = NULL;
    uint32 thisChunkOffset = 0;

    // find the offset of the chunk that the specified offset is in
    if (GetChunkOffset(aStreamOffset, thisChunkOffset, thisEntry))
    {
        if (iWrapped)
        {
            if (thisEntry != iNextEntry)
            {
                if (thisEntry == iFirstEntry)
                {
                    // wrap around
                    aPrevChunkStreamOffset = *iLastEntry;

                    LOGTRACE((0, "PVMFMediaChunkMap::GetPrevChunkOffset - stream offset %d, prev chunk offset %d", aStreamOffset,  aPrevChunkStreamOffset));

                    return true;
                }
                else
                {
                    aPrevChunkStreamOffset = *(--thisEntry);

                    LOGTRACE((0, "PVMFMediaChunkMap::GetPrevChunkOffset - stream offset %d, prev chunk offset %d", aStreamOffset,  aPrevChunkStreamOffset));

                    return true;
                }
            }
        }
        else if (thisEntry != iFirstEntry)
        {
            aPrevChunkStreamOffset = *(--thisEntry);

            LOGTRACE((0, "PVMFMediaChunkMap::GetPrevChunkOffset - stream offset %d, prev chunk offset %d", aStreamOffset,  aPrevChunkStreamOffset));

            return true;
        }
    }

    // not in map
    LOGTRACE((0, "PVMFMediaChunkMap::GetPrevChunkOffset - stream offset %d not found in map", aStreamOffset));

    return false;
}


// should make this into a macro
uint32 PVMFMediaChunkMap::GetBytesLeftInRange(uint32 aFirstOffset, uint32 aLastOffset, uint32 aOffset)
{
    OSCL_UNUSED_ARG(aFirstOffset);

    return aLastOffset - aOffset + 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// From PVMFDataStreamFactory aka PVMFCPMPluginAccessInterfaceFactory

OSCL_EXPORT_REF PVMFShoutcastStreamParserFactory::PVMFShoutcastStreamParserFactory(PVMFDataStreamFactory* aDataStreamFactory, uint32 aMetadataInterval)
{
    iLogger = PVLogger::GetLoggerObject("PVMFShoutcastStreamParser");

    if ((NULL == aDataStreamFactory) || (0 == aMetadataInterval))
    {
        LOGERROR((0, "PVMFShoutcastStreamParserFactory::PVMFShoutcastStreamParserFactory - failed aDataStreamFactory 0x%x aMetadataInterval %d", aDataStreamFactory, aMetadataInterval));
        OSCL_LEAVE(OsclErrArgument);
    }

    iDataStreamFactory = aDataStreamFactory;
    iMetadataInterval = aMetadataInterval;

    PVUuid uuid = PVMIDataStreamSyncInterfaceUuid;
    PVInterface* iFace = iDataStreamFactory->CreatePVMFCPMPluginAccessInterface(uuid);
    if (NULL == iFace)
    {
        LOGERROR((0, "PVMFShoutcastStreamParserFactory::PVMFShoutcastStreamParserFactory - failed"));
        OSCL_LEAVE(OsclErrNoMemory);
    }

    // make a connection to the data stream
    PVMIDataStreamSyncInterface* readStream = OSCL_STATIC_CAST(PVMIDataStreamSyncInterface*, iFace);

    uint32 bufCap = readStream->QueryBufferingCapacity();
    if (0 == bufCap)
    {
        // not using MBDS!!
        LOGERROR((0, "PVMFShoutcastStreamParserFactory::PVMFShoutcastStreamParserFactory - failed, not MBDS"));
        OSCL_LEAVE(OsclErrArgument);
    }
    else
    {
        // allocate memory a media chunk map
        uint32 maxChunks = (bufCap / iMetadataInterval) + 1;

        iMediaMapMemPtr = (uint32*)oscl_malloc(maxChunks * sizeof(uint32));
        if (NULL == iMediaMapMemPtr)
        {
            LOGERROR((0, "PVMFShoutcastStreamParserFactory::PVMFShoutcastStreamParserFactory - failed to allocate %d bytes", maxChunks * sizeof(uint32)));
            OSCL_LEAVE(OsclErrNoMemory);
        }
        else
        {
            iMediaChunkMap.SetMapInfo(maxChunks, iMetadataInterval, iMediaMapMemPtr);
        }
    }

    // close the connection to the data stream
    uuid = PVMIDataStreamSyncInterfaceUuid;
    iDataStreamFactory->DestroyPVMFCPMPluginAccessInterface(uuid, readStream);

    // allocate the metadata buffer
    // 4080 + null termination
    iMetadataInfo.iBuffer = (uint8*)oscl_malloc(PV_SCSP_MAX_METADATA_TAG_SIZE_WITH_LENGTH_BYTE * sizeof(uint8));
    if (NULL == iMetadataInfo.iBuffer)
    {
        LOGERROR((0, "PVMFShoutcastStreamParser::PVMFShoutcastStreamParser failed to allocate %d bytes", PV_SCSP_MAX_METADATA_TAG_SIZE_WITH_LENGTH_BYTE));
        if (NULL != iMediaMapMemPtr)
        {
            iMediaChunkMap.ClearMapInfo();
            oscl_free(iMediaMapMemPtr);
            iMediaMapMemPtr = NULL;
        }
        OSCL_LEAVE(OsclErrNoMemory);
    }

    oscl_memset(iMetadataInfo.iBuffer, '\0', PV_SCSP_MAX_METADATA_TAG_SIZE_WITH_LENGTH_BYTE);

    iMetadataInfo.iSize = 0;

    // init the struct
    for (int32 i = 0; i < PV_SCSP_NUMBER_OF_READ_CONNECTIONS; i++)
    {
        iReadFilePosVec.iReadFilePos[i].iReadPositionStructValid = false;
        iReadFilePosVec.iReadFilePos[i].iMetadataObserver = NULL;
        iReadFilePosVec.iReadFilePos[i].iMetadataObserverBufPtr = NULL;
        iReadFilePosVec.iReadFilePos[i].iSCSP = NULL;
    }
}


OSCL_EXPORT_REF PVMFShoutcastStreamParserFactory::~PVMFShoutcastStreamParserFactory()
{
    if (NULL != iMediaMapMemPtr)
    {
        iMediaChunkMap.ClearMapInfo();

        oscl_free(iMediaMapMemPtr);

        iMediaMapMemPtr = NULL;
    }

    if (NULL != iMetadataInfo.iBuffer)
    {
        oscl_free(iMetadataInfo.iBuffer);

        iMetadataInfo.iBuffer = NULL;
    }
}


OSCL_EXPORT_REF PVMFStatus PVMFShoutcastStreamParserFactory::QueryAccessInterfaceUUIDs(Oscl_Vector<PVUuid, OsclMemAllocator>& aUuids)
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::QueryAccessInterfaceUUIDs"));

    if (NULL == iDataStreamFactory)
    {
        LOGERROR((0, "PVMFShoutcastStreamParser::QueryAccessInterfaceUUIDs - iDataStreamFactory is NULL"));
        return PVDS_FAILURE;
    }

    return iDataStreamFactory->QueryAccessInterfaceUUIDs(aUuids);
}


// From PVMFDataStreamFactory
OSCL_EXPORT_REF PVInterface* PVMFShoutcastStreamParserFactory::CreatePVMFCPMPluginAccessInterface(PVUuid& aUuid)
{
    LOGTRACE((0, "PVMFShoutcastStreamParserFactory::CreatePVMFCPMPluginAccessInterface"));

    if (NULL == iDataStreamFactory)
    {
        LOGERROR((0, "PVMFShoutcastStreamParserFactory::CreatePVMFCPMPluginAccessInterface - iDataStreamFactory is NULL"));
        return NULL;
    }

    if (PVMIDataStreamSyncInterfaceUuid != aUuid)
    {
        LOGERROR((0, "PVMFShoutcastStreamParserFactory::CreatePVMFCPMPluginAccessInterface - failed aUuid not supported"));
        return NULL;
    }

    ReadStreamStruct* readStreamStruct = (ReadStreamStruct*)oscl_malloc(sizeof(ReadStreamStruct));
    if (NULL == readStreamStruct)
    {
        LOGERROR((0, "PVMFShoutcastStreamParserFactory::CreatePVMFCPMPluginAccessInterface - failed"));
        return NULL;
    }
    PVInterface* pvIFace = iDataStreamFactory->CreatePVMFCPMPluginAccessInterface(aUuid);
    if (NULL == pvIFace)
    {
        LOGERROR((0, "PVMFShoutcastStreamParserFactory::CreatePVMFCPMPluginAccessInterface - failed"));
        return NULL;
    }

    PVMIDataStreamSyncInterface* dataStream = OSCL_STATIC_CAST(PVMIDataStreamSyncInterface*, pvIFace);


    PVMFShoutcastStreamParser* readStream = NULL;
    readStream = OSCL_NEW(PVMFShoutcastStreamParser, (dataStream, iMetadataInterval, iMediaChunkMap, iMetadataInfo, iReadFilePosVec));
    if (NULL == readStream)
    {
        OSCL_LEAVE(OsclErrNoMemory);
    }

    readStreamStruct->iDataStream = dataStream;
    readStreamStruct->iSCSP = readStream;

    // save this connection in list
    iReadStreamVec.push_back(readStreamStruct);

    return OSCL_STATIC_CAST(PVInterface*, readStream);
}


// From PVMFDataStreamFactory
OSCL_EXPORT_REF void PVMFShoutcastStreamParserFactory::DestroyPVMFCPMPluginAccessInterface(PVUuid& aUuid, PVInterface* aPtr)
{
    LOGTRACE((0, "PVMFShoutcastStreamParserFactory::DestroyPVMFCPMPluginAccessInterface"));

    // destroy the incoming object only if it uses the right UUID and has a vaild pointer
    if ((aUuid == PVMIDataStreamSyncInterfaceUuid) && (NULL != aPtr))
    {
        PVMFShoutcastStreamParser* readStream = OSCL_STATIC_CAST(PVMFShoutcastStreamParser*, aPtr);

        // find the corresponding data stream
        PVMIDataStreamSyncInterface* dataStream = NULL;

        // remove from list
        Oscl_Vector<ReadStreamStruct*, OsclMemAllocator>::iterator it;
        it = iReadStreamVec.begin();
        while (it != iReadStreamVec.end())
        {
            ReadStreamStruct *rss = *it;
            if (aPtr == rss->iSCSP)
            {
                dataStream = rss->iDataStream;
                oscl_free(rss);
                iReadStreamVec.erase(it);
                break;
            }
            else
            {
                it++;
            }
        }

        if (NULL != dataStream)
        {
            // destroy the data stream object
            iDataStreamFactory->DestroyPVMFCPMPluginAccessInterface(aUuid, dataStream);
        }

        // destroy the SCSP object
        OSCL_DELETE(readStream);
    }
}

OSCL_EXPORT_REF PVMFStatus PVMFShoutcastStreamParserFactory::ResetShoutcastStream()
{
    LOGTRACE((0, "PVMFShoutcastStreamParserFactory::ResetShoutcastStream"));

    // reset all read pointers to 0
    for (int32 i = 0; i < PV_SCSP_NUMBER_OF_READ_CONNECTIONS; i++)
    {
        if (iReadFilePosVec.iReadFilePos[i].iReadPositionStructValid)
        {
            iReadFilePosVec.iReadFilePos[i].iStreamPosition = 0;
            iReadFilePosVec.iReadFilePos[i].iParserFilePosition = 0;

            PvmiDataStreamStatus status = iReadFilePosVec.iReadFilePos[i].iSCSP->ResetReadPosition(i);
            if (PVDS_SUCCESS != status)
            {
                LOGERROR((0, "PVMFShoutcastStreamParserFactory::ResetShoutcastStream - ResetReadPosition failed status %d", status));
            }
        }
    }

    // clear media map
    if (NULL != iMediaMapMemPtr)
    {
        iMediaChunkMap.ResetMapInfo();
    }

    return PVDS_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////

OSCL_EXPORT_REF PVMFShoutcastStreamParser::PVMFShoutcastStreamParser(PVMIDataStreamSyncInterface* aDataStream, uint32 aMetadataInterval,
        PVMFMediaChunkMap& aMediaMap, MetadataInfo& aMetaInfo,
        ReadFilePositionVec& aReadFilePosVec)
{
    iReadStream = aDataStream;
    iMetadataInterval = aMetadataInterval;
    iMediaChunkMap = &aMediaMap;
    iMetadataInfo = &aMetaInfo;
    iReadFilePosVec = &aReadFilePosVec;


    iLogger = PVLogger::GetLoggerObject("PVMFShoutcastStreamParser");

    LOGTRACE((0, "PVMFShoutcastStreamParser::PVMFShoutcastStreamParser data stream %x, metadata interval %d, media map %x, meta info %x, read file pos %x",
              aDataStream, aMetadataInterval, &aMediaMap, &aMetaInfo, &aReadFilePosVec));

    if ((0 == iMetadataInterval) || (NULL == &aMediaMap) ||
            (NULL == &aMetaInfo) || (NULL == &aReadFilePosVec))
    {
        LOGERROR((0, "PVMFShoutcastStreamParser::PVMFShoutcastStreamParser - failed"));
        return;
    }

    iLargestMetadataTag = 0;
    iLargestRead = 0;
    iBytesToNextMetadataTag = iMetadataInterval;

    LOGDEBUG((0, "PVMFShoutcastStreamParser::PVMFShoutcastStreamParser iBytesToNextMetadataTag %d", iBytesToNextMetadataTag));

}

OSCL_EXPORT_REF PVMFShoutcastStreamParser::~PVMFShoutcastStreamParser()
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::~PVMFShoutcastStreamParser"));

    iMediaChunkMap = NULL;
    iMetadataInfo = NULL;
    iReadStream = NULL;
    iReadFilePosVec = NULL;
    iLogger = NULL;
}

// From PVInterface
bool PVMFShoutcastStreamParser::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::queryInterface"));

    iface = NULL;
    if (uuid == PVMIDataStreamSyncInterfaceUuid)
    {
        PVMIDataStreamSyncInterface* myInterface = OSCL_STATIC_CAST(PVMIDataStreamSyncInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
        return true;
    }

    return false;
}


// From PVMIDataStreamSyncInterface
OSCL_EXPORT_REF PvmiDataStreamStatus PVMFShoutcastStreamParser::OpenSession(PvmiDataStreamSession& aSessionID,
        PvmiDataStreamMode aMode,
        bool nonblocking)
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::OpenSession"));

    PvmiDataStreamSession sessionID;
    PvmiDataStreamStatus status = iReadStream->OpenSession(sessionID, aMode, nonblocking);
    if (PVDS_SUCCESS == status)
    {
        bool found = false;
        for (int32 i = 0; i < PV_SCSP_NUMBER_OF_READ_CONNECTIONS; i++)
        {
            if (!iReadFilePosVec->iReadFilePos[i].iReadPositionStructValid)
            {
                // this is available
                iReadFilePosVec->iReadFilePos[i].iReadPositionStructValid = true;
                iReadFilePosVec->iReadFilePos[i].iStreamSessionID = sessionID;

                // both pointers should be 0
                iReadFilePosVec->iReadFilePos[i].iParserFilePosition = 0;
                iReadFilePosVec->iReadFilePos[i].iStreamPosition = 0;

                iReadFilePosVec->iReadFilePos[i].iSCSP = this;

                found = true;
                aSessionID = i;
                break;
            }
        }
        if (!found)
        {
            // should never happen
            status = PVDS_INVALID_REQUEST;
            // close the session
            iReadStream->CloseSession(sessionID);

            LOGERROR((0, "PVMFShoutcastStreamParser::OpenSession - failed, too many sessions"));
        }
    }

    LOGTRACE((0, "PVMFShoutcastStreamParser::OpenSession - status %d session id %d", status, aSessionID));

    return status;
}

OSCL_EXPORT_REF PvmiDataStreamStatus PVMFShoutcastStreamParser::CloseSession(PvmiDataStreamSession aSessionID)
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::CloseSession - session id %d", aSessionID));

    if (!iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid)
    {
        LOGERROR((0, "PVMFShoutcastStreamParser::CloseSession - failed, session id %d, read stream %x, session validation %d",
                  aSessionID, iReadStream, iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid));

        return PVDS_INVALID_REQUEST;
    }

    PvmiDataStreamStatus status = iReadStream->CloseSession(iReadFilePosVec->iReadFilePos[aSessionID].iStreamSessionID);

    iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid = false;

    LOGTRACE((0, "PVMFShoutcastStreamParser::CloseSession - status %d session id %d", status, aSessionID));

    return status;
}

OSCL_EXPORT_REF PvmiDataStreamRandomAccessType PVMFShoutcastStreamParser::QueryRandomAccessCapability()
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::QueryRandomAccessCapability - returning PVDS_FULL_RANDOM_ACCESS"));
    return PVDS_FULL_RANDOM_ACCESS;
}

OSCL_EXPORT_REF PvmiDataStreamStatus PVMFShoutcastStreamParser::QueryReadCapacity(PvmiDataStreamSession aSessionID,
        uint32& aCapacity)
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::QueryReadCapacity"));

    if (!iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid)
    {
        LOGERROR((0, "PVMFShoutcastStreamParser::QueryReadCapacity - failed, session id %d, session validation %d",
                  aSessionID, iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid));

        return PVDS_INVALID_REQUEST;
    }

    uint32 streamCapacity = 0;
    PvmiDataStreamStatus status = iReadStream->QueryReadCapacity(iReadFilePosVec->iReadFilePos[aSessionID].iStreamSessionID, streamCapacity);

    if (PVDS_SUCCESS != status)
    {
        LOGERROR((0, "PVMFShoutcastStreamParser::QueryReadCapacity - failed, session id %d status %d", aSessionID, status));
        return status;

    }

    // We may only run into a problem if we are underflowing, i.e. cache is running low
    //
    // - use iLargestRead, iLargestMetadataTag and iBytesToNextMetadataTag to calculate the read capacity
    //  - iLargestRead keeps track of the largest block read
    //  - iLargestMetadataTag keeps track of largest metadata tag encountered (not including the length byte)
    //  - iBytesToNextMetadataTag keeps track of how much media data is there in the stream before the next metadata tag shows up
    //  - if there is enough in cache for the largest block and the largest metadata tag, don't know to worry
    //  - otherwise, check if there is a metadata tag in cache
    //  - if there is, do not include the metadata tag in the capacity

    // add the length byte
    uint32 tagSize = iLargestMetadataTag + 1;

    if (streamCapacity > + (iLargestRead + tagSize))
    {
        // enough data in cache
        aCapacity = streamCapacity;
    }
    else if (iBytesToNextMetadataTag > streamCapacity)
    {
        // there is no metadata tag in cache
        aCapacity = streamCapacity;
    }
    else
    {
        // there is a tag in cache, assume the largest tag (not the max 4081)
        // don't include tag in capacity
        aCapacity = (streamCapacity > tagSize) ? (streamCapacity - tagSize) : 0;

        LOGDEBUG((0, "PVMFShoutcastStreamParser::QueryReadCapacity - session id %d, capacity %d, streamCapacity %d, iLargestMetadataTag %d, iLargestRead %d, iBytesToNextMetadataTag %d",
                  aSessionID, aCapacity, streamCapacity, iLargestMetadataTag, iLargestRead, iBytesToNextMetadataTag));
    }

    LOGTRACE((0, "PVMFShoutcastStreamParser::QueryReadCapacity - session id %d, capacity %d", aSessionID, aCapacity));

    return PVDS_SUCCESS;
}

OSCL_EXPORT_REF PvmiDataStreamCommandId PVMFShoutcastStreamParser::RequestReadCapacityNotification(PvmiDataStreamSession aSessionID,
        PvmiDataStreamObserver& aObserver,
        uint32 aCapacity,
        OsclAny* aContextData)
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::RequestReadCapacityNotification - session id %d, capacity %d, context data %x",
              aSessionID, aCapacity, aContextData));

    if (!iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid)
    {
        LOGERROR((0, "PVMFShoutcastStreamParser::RequestReadCapacityNotification - failed, session id %d, session validation %d",
                  aSessionID, iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid));
        OSCL_LEAVE(OsclErrNoResources);
    }

    // need to find out metadata is expected shortly
    // if so, need to add max metadata tag size to the requested capacity

    uint32 chunkStart = 0;
    uint32* unused = NULL;
    bool found = iMediaChunkMap->GetChunkOffset(iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition, chunkStart, unused);
    if (!found)
    {
        // current stream position not in map
        // fail the call
        LOGERROR((0, "PVMFShoutcastStreamParser::RequestReadCapacityNotification - failed, current stream position not mapped"));

        OSCL_LEAVE(OsclErrArgument);
    }

    uint32 bytesInChunk = iMediaChunkMap->GetBytesLeftInRange(chunkStart, chunkStart + iMetadataInterval - 1, iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition);

    uint32 streamCapacity = aCapacity;
    if (aCapacity > bytesInChunk)
    {
        uint32 overflow = aCapacity - bytesInChunk;
        uint32 numMetadataTags = overflow / iMetadataInterval;
        uint32 mod = overflow % iMetadataInterval;
        if (0 != mod)
        {
            numMetadataTags++;
        }

        streamCapacity += PV_SCSP_MAX_METADATA_TAG_SIZE_WITH_LENGTH_BYTE * numMetadataTags;
    }

    LOGTRACE((0, "PVMFShoutcastStreamParser::RequestReadCapacityNotification - session id %d, stream capacity %d", aSessionID, streamCapacity));

    return iReadStream->RequestReadCapacityNotification(iReadFilePosVec->iReadFilePos[aSessionID].iStreamSessionID, aObserver, streamCapacity, aContextData);
}

OSCL_EXPORT_REF PvmiDataStreamStatus PVMFShoutcastStreamParser::QueryWriteCapacity(PvmiDataStreamSession aSessionID,
        uint32& capacity)
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::QueryWriteCapacity"));

    if (!iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid)
    {
        LOGERROR((0, "PVMFShoutcastStreamParser::QueryWriteCapacity - failed, session id %d, session validation %d",
                  aSessionID, iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid));

        return PVDS_INVALID_REQUEST;
    }

    return iReadStream->QueryWriteCapacity(iReadFilePosVec->iReadFilePos[aSessionID].iStreamSessionID, capacity);
}

OSCL_EXPORT_REF PvmiDataStreamCommandId PVMFShoutcastStreamParser::RequestWriteCapacityNotification(PvmiDataStreamSession aSessionID,
        PvmiDataStreamObserver& aObserver,
        uint32 aCapacity,
        OsclAny* aContextData)
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::RequestWriteCapacityNotification - session id %d, capacity %d, context data %x",
              aSessionID, aCapacity, aContextData));

    if (!iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid)
    {
        LOGERROR((0, "PVMFShoutcastStreamParser::RequestWriteCapacityNotification - failed, session id %d, session validation %d",
                  aSessionID, iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid));

        return PVDS_INVALID_REQUEST;
    }

    return iReadStream->RequestWriteCapacityNotification(iReadFilePosVec->iReadFilePos[aSessionID].iStreamSessionID, aObserver, aCapacity, aContextData);
}

OSCL_EXPORT_REF PvmiDataStreamCommandId PVMFShoutcastStreamParser::CancelNotification(PvmiDataStreamSession aSessionID,
        PvmiDataStreamObserver& observer,
        PvmiDataStreamCommandId aID,
        OsclAny* aContextData)
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::CancelNotification - session id %d, cmd id %d, context data %x",
              aSessionID, aID, aContextData));

    if (!iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid)
    {
        LOGERROR((0, "PVMFShoutcastStreamParser::CancelNotification - failed, session id %d, session validation %d",
                  aSessionID, iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid));

        return PVDS_INVALID_REQUEST;
    }

    return iReadStream->CancelNotification(iReadFilePosVec->iReadFilePos[aSessionID].iStreamSessionID, observer, aID, aContextData);
}

OSCL_EXPORT_REF PvmiDataStreamStatus PVMFShoutcastStreamParser::CancelNotificationSync(PvmiDataStreamSession aSessionID)
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::CancelNotificationSync - session id %d", aSessionID));

    if (!iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid)
    {
        LOGERROR((0, "PVMFShoutcastStreamParser::CancelNotificationSync - failed, session id %d, session validation %d",
                  aSessionID, iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid));

        return PVDS_INVALID_REQUEST;
    }
    return iReadStream->CancelNotificationSync(iReadFilePosVec->iReadFilePos[aSessionID].iStreamSessionID);
}

OSCL_EXPORT_REF PvmiDataStreamStatus PVMFShoutcastStreamParser::Read(PvmiDataStreamSession aSessionID, uint8* aBuffer,
        uint32 aSize, uint32& aNumElements)
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::Read - session id %d, buf %x, size %d, num elems %d",
              aSessionID, aBuffer, aSize, aNumElements));

    if (!iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid)
    {
        LOGERROR((0, "PVMFShoutcastStreamParser::Read - failed, session id %d, session validation %d",
                  aSessionID, iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid));

        return PVDS_INVALID_REQUEST;
    }

    PvmiDataStreamStatus status = PVDS_SUCCESS;

    // find out the current byte range in the stream
    uint32 firstStreamOffset = 0;
    uint32 lastStreamOffset = 0;
    iReadStream->GetCurrentByteRange(firstStreamOffset, lastStreamOffset);

    uint32 bytesRequested = aSize * aNumElements;

    // update stats
    iLargestRead = (iLargestRead >= bytesRequested) ? iLargestRead : bytesRequested;

    uint8* bufPtr = aBuffer;

    uint32 totalBytesRead = 0;

    // check media chunk map to see how many bytes are mapped from the current stream position on
    uint32 bytesMapped = iMediaChunkMap->GetBytesMapped(iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition);

    bool firstRound = true;

    while (0 != bytesRequested)
    {
        if (0 != bytesMapped)
        {
            // these bytes should be mapped
            // find out how many are there in this chunk
            // if we are at the end of one chunk, move the stream pointer the next chunk
            uint32 bytesInChunk = 0;
            status = GetBytesInChunk(aSessionID, lastStreamOffset, bytesInChunk);
            if (PVDS_SUCCESS != status)
            {
                // data not in stream
                // fail the read
                LOGERROR((0, "PVMFShoutcastStreamParser::Read - failed, data not in stream"));
                return status;
            }

            // read the bytes into caller's buffer
            uint32 bytesRead = (bytesInChunk < bytesRequested) ? bytesInChunk : bytesRequested;

            status = ReadBytes(aSessionID, firstStreamOffset, lastStreamOffset, bufPtr, bytesRead, true);

            // return bytes read so far
            totalBytesRead += bytesRead;
            aNumElements = totalBytesRead / aSize;

            if (PVDS_SUCCESS != status)
            {
                LOGERROR((0, "PVMFShoutcastStreamParser::Read - failed #1, ReadBytes status %d", status));
                return status;
            }

            bool pointToTag = (bytesRequested >= bytesInChunk);

            // if ok, update counters and pointers
            bytesRequested -= bytesRead;
            bytesMapped -= bytesRead;
            bufPtr += bytesRead;

            // the next byte may be the beginning of a metadata tag that has been read already
            // this can happen if the parser seeks backwards (usually with the header)
            // we need to move the file position pass the metadata tag and to the beginning of the next media chunk
            if (pointToTag && bytesMapped != 0)
            {
                // reset the counter
                iBytesToNextMetadataTag = iMetadataInterval;

                uint8 length = 0;
                bytesRead = 1;

                status = ReadBytes(aSessionID, firstStreamOffset, lastStreamOffset, &length, bytesRead, false);
                if (PVDS_SUCCESS != status)
                {
                    // data not in stream
                    // fail the seek
                    LOGERROR((0, "PVMFShoutcastStreamParser::Read - failed #2, Readbytes status %d", status));
                    break;
                }

                // if length is 0, there is no more metadata, media data begins at the next byte
                // otherwise, skip over metadata tag, upate stream position, but not file position
                if (0 != length)
                {
                    int seekToOffset = length * 16;

                    status = SkipRelativeBytes(aSessionID, firstStreamOffset, lastStreamOffset, seekToOffset, false);
                    if (PVDS_SUCCESS != status)
                    {
                        LOGERROR((0, "PVMFShoutcastStreamParser::Read - failed, SkipRelativeBytes status %d", status));
                        break;
                    }
                }
            }
        } // end if bytes are mapped
        else
        {
            if (firstRound)
            {
                firstRound = false;

                // these bytes are not mapped
                // the next byte in the stream is the metadata tag length byte
                // max metadata tag size is 4080 + length byte
                uint8 length = 0;
                uint32 bytesRead = 1;

                status = ReadBytes(aSessionID, firstStreamOffset, lastStreamOffset, &length, bytesRead, false);

                if (PVDS_SUCCESS != status)
                {
                    LOGERROR((0, "PVMFShoutcastStreamParser::Read - failed #3, ReadBytes status %d", status));
                    return status;
                }

                // if length is 0, there is no more metadata, media data begins at the next byte
                // otherwise, save in metadata and notify observer
                if (0 != length)
                {
                    status = HandleMetadata(aSessionID, firstStreamOffset, lastStreamOffset, length);

                    if (PVDS_SUCCESS != status)
                    {
                        // data is not available yet
                        // rewind the read pointer back to the length back
                        PvmiDataStreamStatus status2 = SkipRelativeBytes(aSessionID, firstStreamOffset, lastStreamOffset, -1, false);
                        if (PVDS_SUCCESS != status2)
                        {
                            LOGERROR((0, "PVMFShoutcastStreamParser::Read - failed, HandleMetadata SkipRelativeBytes status %d", status2));
                        }

                        LOGERROR((0, "PVMFShoutcastStreamParser::Read - failed, HandleMetadata status %d", status));
                        return status;
                    }
                }

                // update media chunk map
                iMediaChunkMap->SetNextChunkOffset(iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition);

                // reset the counter
                iBytesToNextMetadataTag = iMetadataInterval;

            } // end if first round

            // the next bytes should be the beginning of a media chunk
            // read in the entire media chunk plus the next metadata tag length byte
            // if there is enough room in the caller's buffer
            uint8 metaLength = 0;
            uint32 bytesRead = (iMetadataInterval < bytesRequested) ? iMetadataInterval + 1 : bytesRequested;

            status = ReadBytes(aSessionID, firstStreamOffset, lastStreamOffset, bufPtr, bytesRead, true);

            bool readMetaLength = false;
            if (bytesRead > iMetadataInterval)
            {
                // account for the extra byte read
                bytesRead--;
                metaLength = *(bufPtr + bytesRead);
                iReadFilePosVec->iReadFilePos[aSessionID].iParserFilePosition--;
                readMetaLength = true;
            }

            // update bytes read so far
            totalBytesRead += bytesRead;
            aNumElements = totalBytesRead / aSize;

            // update counters and pointers
            bytesRequested -= bytesRead;
            bufPtr += bytesRead;

            if (PVDS_SUCCESS != status)
            {
                LOGERROR((0, "PVMFShoutcastStreamParser::Read - failed #4, ReadBytes status %d", status));
                return status;
            }

            // if length is 0, there is no more metadata, media data begins at the next byte
            // otherwise, read in metadata in buffer, update counters and pointers
            if (readMetaLength)
            {
                if (0 != metaLength)
                {
                    status = HandleMetadata(aSessionID, firstStreamOffset, lastStreamOffset, metaLength);

                    if (PVDS_SUCCESS != status)
                    {
                        // data is not available yet
                        // rewind the read pointer back to the length back
                        PvmiDataStreamStatus status2 = SkipRelativeBytes(aSessionID, firstStreamOffset, lastStreamOffset, -1, false);
                        if (PVDS_SUCCESS != status2)
                        {
                            LOGERROR((0, "PVMFShoutcastStreamParser::Read - failed, HandleMetadata SkipRelativeBytes status %d", status2));
                        }

                        LOGERROR((0, "PVMFShoutcastStreamParser::Read - failed, HandleMetadata status %d", status));
                        return status;
                    }
                }

                // update media chunk map
                iMediaChunkMap->SetNextChunkOffset(iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition);

                // reset the counter
                iBytesToNextMetadataTag = iMetadataInterval;
            }

        } // end else bytes not mapped

    } // end while

    LOGTRACE((0, "PVMFShoutcastStreamParser::Read - session id %d, status %d, num elems %d", aSessionID, status, aNumElements));

    return status;
}

OSCL_EXPORT_REF PvmiDataStreamStatus PVMFShoutcastStreamParser::Write(PvmiDataStreamSession aSessionID, uint8* aBuffer,
        uint32 aSize, uint32& aNumElements)
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::Write - session id %d, buf %x, size %d, num elems %d",
              aSessionID, aBuffer, aSize, aNumElements));

    if (!iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid)
    {
        LOGERROR((0, "PVMFShoutcastStreamParser::Write - failed, session id %d, read stream %x, session validation %d",
                  aSessionID, iReadStream, iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid));

        return PVDS_INVALID_REQUEST;
    }

    // should not be supported
    // but pass the call go though
    return iReadStream->Write(iReadFilePosVec->iReadFilePos[aSessionID].iStreamSessionID, aBuffer, aSize, aNumElements);
}

OSCL_EXPORT_REF PvmiDataStreamStatus PVMFShoutcastStreamParser::Write(PvmiDataStreamSession aSessionID, OsclRefCounterMemFrag* aFrag,
        uint32& aNumElements)
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::Write - session id %d, frag %x, num elems %d",
              aSessionID, aFrag, aNumElements));

    if (!iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid)
    {
        LOGERROR((0, "PVMFShoutcastStreamParser::Write - failed, session id %d, read stream %x, session validation %d",
                  aSessionID, iReadStream, iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid));

        return PVDS_INVALID_REQUEST;
    }

    // should not be supported
    // but pass the call go though
    return iReadStream->Write(iReadFilePosVec->iReadFilePos[aSessionID].iStreamSessionID, aFrag, aNumElements);
}

OSCL_EXPORT_REF PvmiDataStreamStatus PVMFShoutcastStreamParser::Seek(PvmiDataStreamSession aSessionID,
        int32 aOffset, PvmiDataStreamSeekType aOrigin)
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::Seek - session id %d, offset %d, origin %d iStreamPosition %d iParserFilePosition %d",
              aSessionID, aOffset, aOrigin, iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition,
              iReadFilePosVec->iReadFilePos[aSessionID].iParserFilePosition));

    if (!iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid)
    {
        LOGERROR((0, "PVMFShoutcastStreamParser::Seek - failed, session id %d, read stream %x, session validation %d",
                  aSessionID, iReadStream, iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid));

        return PVDS_INVALID_REQUEST;
    }

    // only support PVDS_SEEK_CUR
    if (PVDS_SEEK_CUR != aOrigin)
    {
        LOGERROR((0, "PVMFShoutcastStreamParser::Seek - failed, does not support origin %d", aOrigin));

        return PVDS_UNSUPPORTED_MODE;
    }

    PvmiDataStreamStatus status = PVDS_SUCCESS;

    // find out the current byte range in the stream
    uint32 firstStreamOffset = 0;
    uint32 lastStreamOffset = 0;
    iReadStream->GetCurrentByteRange(firstStreamOffset, lastStreamOffset);

    int32 seekToOffset = 0;
    uint32 streamPosition = iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition;
    uint32 filePosition = iReadFilePosVec->iReadFilePos[aSessionID].iParserFilePosition;

    // in case multiple seek failed, need to put the pointer back
    uint32 savedOffset = iReadStream->GetCurrentPointerPosition(iReadFilePosVec->iReadFilePos[aSessionID].iStreamSessionID);


    uint32 bytesMapped = iMediaChunkMap->GetBytesMapped(iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition);

    // do nothing if offset is 0
    if (aOffset < 0)
    {
        // seek backward, bytes should have been mapped
        // find the start of this media chunk
        uint32* unused = NULL;
        uint32 chunkStart = 0;

        uint32 bytesToRewind = oscl_abs(aOffset);

        bool chunkFound = iMediaChunkMap->GetChunkOffset(iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition, chunkStart, unused);
        if (!chunkFound)
        {
            status = SkipRelativeBytes(aSessionID, firstStreamOffset, lastStreamOffset, -1, true);
            if (PVDS_SUCCESS != status)
            {
                LOGERROR((0, "PVMFShoutcastStreamParser::Seek - failed, SkipRelativeBytes status %d", status));
                return status;
            }
            bytesToRewind--;

            chunkFound = iMediaChunkMap->GetChunkOffset(iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition, chunkStart, unused);
            if (!chunkFound)
            {
                LOGERROR((0, "PVMFShoutcastStreamParser::Seek - failed, stream position not mapped"));
                return PVDS_FAILURE;
            }
        }

        uint32 bytesLeftInChunk = iMediaChunkMap->GetBytesLeftInRange(chunkStart, chunkStart + iMetadataInterval - 1, iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition);
        uint32 bytesInChunk = iMetadataInterval - bytesLeftInChunk;

        while (0 != bytesToRewind)
        {
            bytesInChunk = (bytesInChunk < bytesToRewind) ? bytesInChunk : bytesToRewind;

            // rewind to the start of this chunk
            // update stream position and file position
            seekToOffset = 0 - bytesInChunk;

            status = SkipRelativeBytes(aSessionID, firstStreamOffset, lastStreamOffset, seekToOffset, true);
            if (PVDS_SUCCESS != status)
            {
                LOGERROR((0, "PVMFShoutcastStreamParser::Seek - failed, SkipRelativeBytes status %d", status));
                break;
            }

            bytesToRewind -= bytesInChunk;

            if (0 != bytesToRewind)
            {
                // keep rewinding
                chunkFound = iMediaChunkMap->GetPrevChunkOffset(iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition, chunkStart);
                if (!chunkFound)
                {
                    LOGERROR((0, "PVMFShoutcastStreamParser::Seek - failed, stream position not mapped"));
                    status = PVDS_FAILURE;
                    break;
                }
                // need to rewind the stream position to the last byte of the previous media chunk
                // skip over the metadata tag
                // parser file position only rewind by one byte
                seekToOffset = 0 - (iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition - (chunkStart + iMetadataInterval - 1));
                status = SkipRelativeBytes(aSessionID, firstStreamOffset, lastStreamOffset, seekToOffset, false);
                if (PVDS_SUCCESS != status)
                {
                    LOGERROR((0, "PVMFShoutcastStreamParser::Seek - failed, SkipRelativeBytes status %d", status));
                    status = PVDS_FAILURE;
                    break;
                }

                // update parser file position
                iReadFilePosVec->iReadFilePos[aSessionID].iParserFilePosition--;

                bytesToRewind--;
                bytesInChunk = iMetadataInterval - 1;
            }
        } // end while
    }
    else if (aOffset > 0)
    {
        // seek forward, bytes may not be mapped
        bytesMapped = iMediaChunkMap->GetBytesMapped(iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition);

        uint32 bytesToSkip = aOffset;

        while (0 != bytesToSkip)
        {
            if (0 != bytesMapped)
            {
                // these bytes are mapped
                uint32 bytesInChunk = 0;

                status = GetBytesInChunk(aSessionID, lastStreamOffset, bytesInChunk);
                if (PVDS_SUCCESS != status)
                {
                    // data not in stream
                    // fail the seek
                    LOGERROR((0, "PVMFShoutcastStreamParser::Seek - failed, stream position not mapped"));
                    status = PVDS_FAILURE;
                    break;
                }

                // skip over the media bytes
                // update stream and file positions
                seekToOffset = (bytesInChunk < bytesToSkip) ? bytesInChunk : bytesToSkip;

                status = SkipRelativeBytes(aSessionID, firstStreamOffset, lastStreamOffset, seekToOffset, true);
                if (PVDS_SUCCESS != status)
                {
                    // put things back
                    break;
                }

                bool pointToTag = (bytesToSkip >= bytesInChunk);

                bytesToSkip -= seekToOffset;
                bytesMapped -= seekToOffset;

                // if the file position is pointing to the beginning to the metadata tag,
                // this will happen if the reader reads forward and rewinds,
                // we need to move the file position pass the metadata tag and to the beginning of the next media chunk
                if (pointToTag && bytesMapped != 0)
                {
                    // reset the counter
                    iBytesToNextMetadataTag = iMetadataInterval;

                    uint8 length = 0;
                    uint32 bytesRead = 1;

                    status = ReadBytes(aSessionID, firstStreamOffset, lastStreamOffset, &length, bytesRead, false);
                    if (PVDS_SUCCESS != status)
                    {
                        // data not in stream
                        // fail the seek
                        LOGERROR((0, "PVMFShoutcastStreamParser::Seek - failed, Readbytes status %d", status));
                        break;
                    }

                    // if length is 0, there is no more metadata, media data begins at the next byte
                    // otherwise, skip over metadata tag, upate stream position
                    if (0 != length)
                    {
                        seekToOffset = length * 16;

                        status = SkipRelativeBytes(aSessionID, firstStreamOffset, lastStreamOffset, seekToOffset, false);
                        if (PVDS_SUCCESS != status)
                        {
                            LOGERROR((0, "PVMFShoutcastStreamParser::Seek - failed, SkipRelativeBytes status %d", status));
                            break;
                        }
                    }
                }
            }
            else
            {
                // these bytes are not mapped
                // the next byte in the stream is the metadata tag length byte
                uint8 length = 0;
                uint32 bytesRead = 1;

                status = ReadBytes(aSessionID, firstStreamOffset, lastStreamOffset, &length, bytesRead, false);
                if (PVDS_SUCCESS != status)
                {
                    // data not in stream
                    // fail the seek
                    LOGERROR((0, "PVMFShoutcastStreamParser::Seek - failed, Readbytes status %d", status));
                    break;
                }

                // if length is 0, there is no more metadata, media data begins at the next byte
                // otherwise, skip over metadata tag, update stream position, but not the file position
                if (0 != length)
                {
                    seekToOffset = length * 16;

                    status = SkipRelativeBytes(aSessionID, firstStreamOffset, lastStreamOffset, seekToOffset, false);
                    if (PVDS_SUCCESS != status)
                    {
                        LOGERROR((0, "PVMFShoutcastStreamParser::Seek - failed, SkipRelativeBytes status %d", status));
                        break;
                    }
                }

                // update media chunk map
                iMediaChunkMap->SetNextChunkOffset(iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition);

                // reset the counter
                iBytesToNextMetadataTag = iMetadataInterval;

                // the next bytes should be the beginning of a media chunk
                // skip pass entire media chunk to the next metadata tag length byte
                // update stream and file position
                seekToOffset = (iMetadataInterval < bytesToSkip) ? iMetadataInterval : bytesToSkip;

                status = SkipRelativeBytes(aSessionID, firstStreamOffset, lastStreamOffset, seekToOffset, true);
                if (PVDS_SUCCESS != status)
                {
                    LOGERROR((0, "PVMFShoutcastStreamParser::Seek - failed, SkipRelativeBytes status %d", status));
                    break;
                }

                bytesToSkip -= seekToOffset;
            } // end else, bytes not mapped
        } // end while
    } // end else if forward seek

    if (PVDS_SUCCESS != status)
    {
        // put everything back to where they were
        iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition = streamPosition;
        iReadFilePosVec->iReadFilePos[aSessionID].iParserFilePosition = filePosition;
        iReadStream->Seek(iReadFilePosVec->iReadFilePos[aSessionID].iStreamSessionID, savedOffset, PVDS_SEEK_SET);
    }

    LOGTRACE((0, "PVMFShoutcastStreamParser::Seek - session id %d, offset %d, status %d", aSessionID, aOffset, status));

    return status;
}


OSCL_EXPORT_REF uint32 PVMFShoutcastStreamParser::GetCurrentPointerPosition(PvmiDataStreamSession aSessionID)
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::GetCurrentPointerPosition - session id %d", aSessionID));

    if (!iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid)
    {
        LOGERROR((0, "PVMFShoutcastStreamParser::GetCurrentPointerPosition - failed, session id %d, read stream %x, session validation %d",
                  aSessionID, iReadStream, iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid));

        return PVDS_INVALID_REQUEST;
    }

    LOGTRACE((0, "PVMFShoutcastStreamParser::GetCurrentPointerPosition - session id %d current pos %d",
              aSessionID, iReadFilePosVec->iReadFilePos[aSessionID].iParserFilePosition));

    return iReadFilePosVec->iReadFilePos[aSessionID].iParserFilePosition;
}

OSCL_EXPORT_REF PvmiDataStreamStatus PVMFShoutcastStreamParser::Flush(PvmiDataStreamSession aSessionID)
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::Flush - session id %d", aSessionID));

    if (!iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid)
    {
        LOGERROR((0, "PVMFShoutcastStreamParser::Flush - failed, session id %d, read stream %x, session validation %d",
                  aSessionID, iReadStream, iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid));
        return PVDS_INVALID_REQUEST;
    }
    return iReadStream->Flush(iReadFilePosVec->iReadFilePos[aSessionID].iStreamSessionID);
}

OSCL_EXPORT_REF uint32 PVMFShoutcastStreamParser::QueryBufferingCapacity()
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::QueryBufferingCapacity"));

    return iReadStream->QueryBufferingCapacity();
}

OSCL_EXPORT_REF PvmiDataStreamStatus PVMFShoutcastStreamParser::MakePersistent(int32 aOffset, uint32 aSize)
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::MakePersistent - offset %d, size %d", aOffset, aSize));

    // convert the parser offset and size into shoutcast stream offset and size
    // should be 0, 0 for shoutcast
    if ((0 != aOffset) || (0 != aSize))
    {
        LOGERROR((0, "PVMFShoutcastStreamParser::MakePersistent - failed, aOffset %d, aSize %d, should both be 0", aOffset, aSize));
        return PVDS_INVALID_REQUEST;
    }

    return iReadStream->MakePersistent(aOffset, aSize);
}

OSCL_EXPORT_REF void PVMFShoutcastStreamParser::GetCurrentByteRange(uint32& aCurrentFirstByteOffset, uint32& aCurrentLastByteOffset)
{
    OSCL_UNUSED_ARG(aCurrentFirstByteOffset);
    OSCL_UNUSED_ARG(aCurrentLastByteOffset);
    // this function was added to the MBDS to provide repositioning capability during progressive streaming
    // this function should not be called for shoutcast
    // however, this may be useful later on
    LOGTRACE((0, "PVMFShoutcastStreamParser::GetCurrentByteRange should not be called!!"));
}

OSCL_EXPORT_REF PvmiDataStreamStatus PVMFShoutcastStreamParser::RequestMetadataUpdates(PvmiDataStreamSession aSessionID,
        PVMFMetadataUpdatesObserver& aObserver,
        uint32 aBufSize,
        uint8 *aBufPtr)
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::RequestMetadataUpdates - session id %d, observer %x, bufsize %d, bufptr %x",
              aSessionID, &aObserver, aBufSize, aBufPtr));

    if (!iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid ||
            ((NULL != &aObserver) && ((0 == aBufSize) || (NULL == aBufPtr))))
    {
        LOGERROR((0, "PVMFShoutcastStreamParser::RequestMetadataUpdates - failed, read stream %x, session id %d, session validation %d, observer %x, bufsize %d, bufptr %x",
                  iReadStream, aSessionID, iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid, &aObserver, aBufSize, aBufPtr));

        return PVDS_INVALID_REQUEST;
    }

    iReadFilePosVec->iReadFilePos[aSessionID].iMetadataObserver = &aObserver;
    iReadFilePosVec->iReadFilePos[aSessionID].iMetadataObserverBufSize = aBufSize;
    iReadFilePosVec->iReadFilePos[aSessionID].iMetadataObserverBufPtr = aBufPtr;

    return PVDS_SUCCESS;
}

OSCL_EXPORT_REF PvmiDataStreamStatus PVMFShoutcastStreamParser::ResetReadPosition(PvmiDataStreamSession aSessionID)
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::ResetReadPosition"));

    if ((!iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid) || (NULL == iReadStream))
    {
        LOGERROR((0, "PVMFShoutcastStreamParser::ResetReadPosition - failed, session id %d, read stream %x, session validation %d",
                  aSessionID, iReadStream, iReadFilePosVec->iReadFilePos[aSessionID].iReadPositionStructValid));
        return PVDS_INVALID_REQUEST;
    }

    // reset read pointer to 0
    // used when a new server connection is established, e.g. in play stop play scenario
    return iReadStream->Seek(iReadFilePosVec->iReadFilePos[aSessionID].iStreamSessionID, 0, PVDS_SEEK_SET);
}

//
// private functions
//
void PVMFShoutcastStreamParser::ManageMetadataUpdateNotifications()
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::ManageMetadataUpdateNotifications"));

    // go through all the valid read session and see which ones need to be notified
    for (int i = 0; i < PV_SCSP_NUMBER_OF_READ_CONNECTIONS; i++)
    {
        if ((iReadFilePosVec->iReadFilePos[i].iReadPositionStructValid) && (NULL != iReadFilePosVec->iReadFilePos[i].iMetadataObserver))
        {
            // copy metadata over to observer's buffer
            uint32 bytesToCopy = (iReadFilePosVec->iReadFilePos[i].iMetadataObserverBufSize < iMetadataInfo->iSize) ? iReadFilePosVec->iReadFilePos[i].iMetadataObserverBufSize : iMetadataInfo->iSize;

            oscl_memcpy(iReadFilePosVec->iReadFilePos[i].iMetadataObserverBufPtr, iMetadataInfo->iBuffer, bytesToCopy);

            // notify update
            PVMFMetadataUpdatesObserver* observer = iReadFilePosVec->iReadFilePos[i].iMetadataObserver;

            LOGDEBUG((0, "PVMFShoutcastStreamParser::ManageMetadataUpdateNotifications - callback observer %x", observer));

            observer->MetadataUpdated(bytesToCopy);
        }
    }
}


PvmiDataStreamStatus PVMFShoutcastStreamParser::GetBytesInChunk(PvmiDataStreamSession aSessionID, uint32 aLastStreamOffset, uint32& aBytesInChunk)
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::GetBytesInChunk - session id %d, last offset %d", aSessionID, aLastStreamOffset));

    uint32 chunkStart = 0;
    uint32* unused = NULL;
    bool found = iMediaChunkMap->GetChunkOffset(iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition, chunkStart, unused);
    if (!found)
    {
        // stream position not in map
        // fail the call
        LOGERROR((0, "PVMFShoutcastStreamParser::GetBytesInChunk - failed, stream position %d not mapped", iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition));
        return PVDS_FAILURE;
    }

    uint32 bytesInChunk = iMediaChunkMap->GetBytesLeftInRange(chunkStart, chunkStart + iMetadataInterval - 1, iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition);

    if (0 == bytesInChunk)
    {
        // move to the next chunk
        // get the next chunk offset
        uint32 nextChunkOffset = 0;
        found = iMediaChunkMap->GetNextChunkOffset(iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition, nextChunkOffset);
        if ((!found) || (nextChunkOffset > aLastStreamOffset))
        {
            // data not in stream
            // fail the call
            LOGERROR((0, "PVMFShoutcastStreamParser::GetBytesInChunk - failed, next chunk not mapped"));
            return PVDS_FAILURE;
        }

        // skip over the metadata tag
        uint32 seekOffset = nextChunkOffset - iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition;

        PvmiDataStreamStatus status = iReadStream->Seek(iReadFilePosVec->iReadFilePos[aSessionID].iStreamSessionID, seekOffset, PVDS_SEEK_CUR);
        if (PVDS_SUCCESS != status)
        {
            LOGERROR((0, "PVMFShoutcastStreamParser::GetBytesInChunk - failed, skipping metadata status %d", status));
            return status;
        }

        // update stream pointer
        iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition += seekOffset;

        LOGDEBUG((0, "PVMFShoutcastStreamParser::GetBytesInChunk skip over metadata tag %d iStreamPosition %d",
                  seekOffset, iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition));

        // next byte should be the media data in the next chunk
        // chunk size should be the metadata interval
        bytesInChunk = iMetadataInterval;

        // reset the counter
        iBytesToNextMetadataTag = iMetadataInterval;
    }

    aBytesInChunk = bytesInChunk;


    LOGTRACE((0, "PVMFShoutcastStreamParser::GetBytesInChunk - session id %d, bytes %d", aSessionID, aBytesInChunk));

    return PVDS_SUCCESS;
}

PvmiDataStreamStatus PVMFShoutcastStreamParser::ReadBytes(PvmiDataStreamSession aSessionID, uint32 aFirstStreamOffset, uint32 aLastStreamOffset,
        uint8* aBufPtr, uint32& aBytesToRead, bool bUpdateParsingPosition)
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::ReadBytes - session id %d, first offset %d, last offset %d, bufptr %x, bytes %d, update %d",
              aSessionID, aFirstStreamOffset, aLastStreamOffset, aBufPtr, aBytesToRead, bUpdateParsingPosition));

    // make sure all the bytes are in the stream
    bool inStream = false;
    IS_OFFSET_IN_RANGE(aFirstStreamOffset, aLastStreamOffset, iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition, inStream);
    bool inStream2 = false;
    IS_OFFSET_IN_RANGE(aFirstStreamOffset, aLastStreamOffset, iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition + aBytesToRead - 1, inStream2);

    if (!inStream || !inStream2)
    {
        // data not in stream
        // fail the read
        aBytesToRead = 0;

        LOGERROR((0, "PVMFShoutcastStreamParser::ReadBytes - failed, bytes not in stream, session id %d, first offset 0x%x, last offset 0x%x, stream pos 0x%x, bytes %d, update %d",
                  aSessionID, aFirstStreamOffset, aLastStreamOffset, iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition, aBytesToRead, bUpdateParsingPosition));

        return PVDS_FAILURE;
    }

    PvmiDataStreamStatus status = iReadStream->Read(iReadFilePosVec->iReadFilePos[aSessionID].iStreamSessionID, aBufPtr, 1, aBytesToRead);

    // update both pointers
    iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition += aBytesToRead;

    if (bUpdateParsingPosition)
    {
        iReadFilePosVec->iReadFilePos[aSessionID].iParserFilePosition += aBytesToRead;

        // update counter
        iBytesToNextMetadataTag -= aBytesToRead;
    }

    LOGDEBUG((0, "PVMFShoutcastStreamParser::ReadBytes session id %d, iStreamPosition %d, iParserFilePosition %d",
              aSessionID, iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition,
              iReadFilePosVec->iReadFilePos[aSessionID].iParserFilePosition));


    LOGTRACE((0, "PVMFShoutcastStreamParser::ReadBytes - session id %d, status %d, bytes read %d", aSessionID, status, aBytesToRead));

    return status;
}


PvmiDataStreamStatus PVMFShoutcastStreamParser::SkipRelativeBytes(PvmiDataStreamSession aSessionID, uint32 aFirstStreamOffset, uint32 aLastStreamOffset,
        int32 aSeekToOffset, bool bUpdateParsingPosition)
{
    LOGTRACE((0, "PVMFShoutcastStreamParser::SkipRelativeBytes - session id %d, first offset %d, last offset %d, offset %d, update %d",
              aSessionID, aFirstStreamOffset, aLastStreamOffset, aSeekToOffset, bUpdateParsingPosition));

    // make sure all the bytes are in the stream
    // seeking forward, it is ok for data to be not in stream yet
    if (0 == aSeekToOffset)
    {
        return PVDS_SUCCESS;
    }
    else if (aSeekToOffset < 0)
    {
        // seeking backward
        bool inStream = false;
        IS_OFFSET_IN_RANGE(aFirstStreamOffset, aLastStreamOffset, iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition + aSeekToOffset, inStream);

        if (!inStream)
        {
            // data not in stream
            // fail the skipiReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition
            LOGERROR((0, "PVMFShoutcastStreamParser::SkipRelativeBytes - backward failed, offset 0x%x not in stream, aFirstStreamOffset 0x%x aLastStreamOffset 0x%x",
                      iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition, aFirstStreamOffset, aLastStreamOffset));
            return PVDS_FAILURE;
        }
    }

    PvmiDataStreamStatus status = iReadStream->Seek(iReadFilePosVec->iReadFilePos[aSessionID].iStreamSessionID, aSeekToOffset, PVDS_SEEK_CUR);
    if (PVDS_SUCCESS == status)
    {
        // update both pointers
        iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition += aSeekToOffset;

        if (bUpdateParsingPosition)
        {
            iReadFilePosVec->iReadFilePos[aSessionID].iParserFilePosition += aSeekToOffset;

            // update counter
            if (aSeekToOffset < 0)
            {
                // seeking backward, may seek across multiple chunks
                uint32 absOffset = oscl_abs(aSeekToOffset);

                if (absOffset > (iMetadataInterval - iBytesToNextMetadataTag))
                {
                    // not in current chunk
                    // but we rewinding to the last byte of the chunk
                    // so need to minus one
                    iBytesToNextMetadataTag = ((absOffset - (iMetadataInterval - iBytesToNextMetadataTag)) % iMetadataInterval) - 1;
                }
                else
                {
                    // still inside current chunk
                    iBytesToNextMetadataTag += absOffset;
                }
            }
            else
            {
                // seeking forward, may seek across multiple chunks
                if ((uint32)aSeekToOffset > iBytesToNextMetadataTag)
                {
                    // cross over to another chunk
                    iBytesToNextMetadataTag = iMetadataInterval - ((aSeekToOffset - iBytesToNextMetadataTag) % iMetadataInterval);
                }
                else
                {
                    // still inside current chunk
                    iBytesToNextMetadataTag -= aSeekToOffset;
                }
            }
        }
    }

    LOGDEBUG((0, "PVMFShoutcastStreamParser::SkipRelativeBytes session id %d, iStreamPosition %d, iParserFilePosition %d",
              aSessionID, iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition,
              iReadFilePosVec->iReadFilePos[aSessionID].iParserFilePosition));

    LOGTRACE((0, "PVMFShoutcastStreamParser::SkipRelativeBytes session id %d, status %d", aSessionID, status));

    return status;
}


PvmiDataStreamStatus PVMFShoutcastStreamParser::HandleMetadata(PvmiDataStreamSession aSessionID, uint32 aFirstStreamOffset, uint32 aLastStreamOffset,
        uint32 aMetadataLengthBase16)
{

    LOGTRACE((0, "PVMFShoutcastStreamParser::HandleMetadata - session id %d, first offset %d, last offset %d, length base 16 %d",
              aSessionID, aFirstStreamOffset, aLastStreamOffset, aMetadataLengthBase16));

    // multiply the length byte by 16
    uint32 bytesRead = aMetadataLengthBase16 * 16;

    // update stats
    iLargestMetadataTag = (iLargestMetadataTag >= bytesRead) ? iLargestMetadataTag : bytesRead;

    // clear out the buffer
    oscl_memset(iMetadataInfo->iBuffer, '\0', PV_SCSP_MAX_METADATA_TAG_SIZE_WITH_LENGTH_BYTE);

    PvmiDataStreamStatus status = ReadBytes(aSessionID, aFirstStreamOffset, aLastStreamOffset, iMetadataInfo->iBuffer, bytesRead, false);

    if (PVDS_SUCCESS != status)
    {
        LOGERROR((0, "PVMFShoutcastStreamParser::HandleMetadata - failed, ReadBytes status %d", status));
        return status;
    }

#if (LOG_FOR_DEBUGGING)
    // for debugging only
    LOGERROR((0, "PVMFShoutcastStreamParser::HandleMetadata iStreamPosition 0x%x bytesRead %d: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
              iReadFilePosVec->iReadFilePos[aSessionID].iStreamPosition, bytesRead,
              *iMetadataInfo->iBuffer, *(iMetadataInfo->iBuffer + 1), *(iMetadataInfo->iBuffer + 2), *(iMetadataInfo->iBuffer + 3),
              *(iMetadataInfo->iBuffer + 4), *(iMetadataInfo->iBuffer + 5), *(iMetadataInfo->iBuffer + 6), *(iMetadataInfo->iBuffer + 7),
              *(iMetadataInfo->iBuffer + 8), *(iMetadataInfo->iBuffer + 9), *(iMetadataInfo->iBuffer + 10), *(iMetadataInfo->iBuffer + 11),
              *(iMetadataInfo->iBuffer + 12), *(iMetadataInfo->iBuffer + 13), *(iMetadataInfo->iBuffer + 14), *(iMetadataInfo->iBuffer + 15)
             ));
#endif

    // if successful
    iMetadataInfo->iSize = bytesRead;

    // manage metadata update callback
    ManageMetadataUpdateNotifications();


    LOGTRACE((0, "PVMFShoutcastStreamParser::HandleMetadata session id %d, status %d", aSessionID, status));
    return status;
}

