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
#include "pvmf_mp3ffparser_node.h"

#define LOGINFO(m) PVLOGGER_LOGMSG(PVLOGMSG_INST_MLDBG,iLogger,PVLOGMSG_INFO,m);

// constructor

PVMFMP3FFParserNode::PVMFMP3FFParserNode(int32 aPriority)
        : PVMFNodeInterfaceImpl(aPriority, "PVMFMP3FFParserNode"),
        iParseStatus(false),
        iSourceURLSet(false),
        iMP3File(NULL),
        iConfigOk(0),
        iMaxFrameSize(PVMP3FF_DEFAULT_MAX_FRAMESIZE),
        iMP3FormatBitrate(0),
        iLogger(NULL),
        iSendDecodeFormatSpecificInfo(true)
{
#if PV_HAS_SHOUTCAST_SUPPORT_ENABLED
    iMetadataBuf = NULL;
    iMetadataBufSize = 0;
    iMetadataSize = 0;
    iSCSPFactory = NULL;
    iSCSP = NULL;
    iClipByteRate = 0;
    iMetadataInterval = 0;
#endif
    iOutPort = NULL;
    iCPMContainer.iCPMLicenseInterface = NULL;
    iCPMContainer.iCPMLicenseInterfacePVI = NULL;
    iCPMContainer.iCPMMetaDataExtensionInterface   = NULL;
    iCPMGetMetaDataKeysCmdId = 0;
    iCPMGetMetaDataValuesCmdId = 0;
    iCPMUsageCompleteCmdId = 0;
    iCPMCloseSessionCmdId = 0;
    iCPMResetCmdId = 0;
    oWaitingOnLicense  = false;
    iFileHandle = NULL;
    iAutoPaused = false;
    iDownloadProgressInterface = NULL;
    iDataStreamFactory         = NULL;
    iDataStreamInterface = NULL;
    iDataStreamReadCapacityObserver = NULL;
    iMP3ParserNodeMetadataValueCount = 0;
    iDownloadComplete = false;
    iFileSizeRecvd = false;
    iFileSize = 0;
    iCheckForMP3HeaderDuringInit = false;
    iDurationCalcAO = NULL;
    iUseCPMPluginRegistry = false;

    int32 err;

    OSCL_TRY(err,
             // Set the node capability data.
             // This node can support an unlimited number of ports.
             iNodeCapability.iCanSupportMultipleInputPorts = false;
             iNodeCapability.iCanSupportMultipleOutputPorts = false;
             iNodeCapability.iHasMaxNumberOfPorts = false;
             iNodeCapability.iMaxNumberOfPorts = 0;//no maximum
             iNodeCapability.iInputFormatCapability.push_back(PVMFFormatType(PVMF_MIME_MP3));
             iNodeCapability.iOutputFormatCapability.push_back(PVMFFormatType(PVMF_MIME_MP3));
             // secondry construction
             Construct();
            );

    if (err != OsclErrNone)
    {
        //if a leave happened, cleanup and re-throw the error
        iInputCommands.clear();
        iNodeCapability.iInputFormatCapability.clear();
        iNodeCapability.iOutputFormatCapability.clear();
        OSCL_CLEANUP_BASE_CLASS(PVMFNodeInterfaceImpl);
        iFileServer.Close();
        OSCL_LEAVE(err);
    }
#if PV_HAS_SHOUTCAST_SUPPORT_ENABLED
    iMetadataVector.reserve(PVMF_MP3FFPARSER_NODE_METADATA_RESERVE);
#endif
}

// Secondary constructor
void PVMFMP3FFParserNode::Construct()
{
    iFileServer.Connect();
    iCPMContainer.Construct(PVMFSubNodeContainerBaseMp3::ECPM, this);
    //create the sub-node command queue.  Use a reserve to avoid
    //dynamic memory failure later.
    //Max depth is the max number of sub-node commands for any one node command.
    //Init command may take up to 9
    iSubNodeCmdVec.reserve(9);
    iLogger = PVLogger::GetLoggerObject("PVFMP3FFParserNode");
}

// Destructor
PVMFMP3FFParserNode::~PVMFMP3FFParserNode()
{
    if (IsAdded())
    {
        RemoveFromScheduler();
    }

    //Reset Logger
    iLogger = NULL;

    // Unbind the download progress clock
    iDownloadProgressClock.Unbind();
    // Release the download progress interface, if any
    if (iDownloadProgressInterface != NULL)
    {
        iDownloadProgressInterface->cancelResumeNotification();
        iDownloadProgressInterface->removeRef();
        iDownloadProgressInterface = NULL;
    }

    //if any CPM commands are pending, there will be a crash when they callback,
    //so panic here instead.
    if (iCPMContainer.CmdPending())
    {
        OSCL_ASSERT(0);
    }

#if PV_HAS_SHOUTCAST_SUPPORT_ENABLED
    while (!iMetadataVector.empty())
    {
        iMetadataVector.erase(iMetadataVector.begin());
    }
#endif

    ReleaseTrack();
    // Clean up the file source
    CleanupFileSource();
    //CPM Cleanup should be done after CleanupFileSource
    iCPMContainer.Cleanup();
    // Disconnect fileserver
    iFileServer.Close();
}

PVMFStatus PVMFMP3FFParserNode::HandleExtensionAPICommands()
{
    PVMFStatus status = PVMFFailure;
    switch (iCurrentCommand.iCmd)
    {
        case PVMF_GENERIC_NODE_SET_DATASOURCE_POSITION:
            status = DoSetDataSourcePosition();
            break;
        case PVMF_GENERIC_NODE_QUERY_DATASOURCE_POSITION:
            status = DoQueryDataSourcePosition();
            break;
        case PVMF_GENERIC_NODE_SET_DATASOURCE_RATE:
            status = DoSetDataSourceRate();
            break;
        case PVMF_GENERIC_NODE_GETNODEMETADATAKEYS:
            status = DoGetNodeMetadataKeys();
            break;
        case PVMF_GENERIC_NODE_GETNODEMETADATAVALUES:
            status = DoGetNodeMetadataValues();
            break;
        default: //unknown command, do an assert here
            //and complete the command with PVMFErrNotSupported
            status = PVMFErrNotSupported;
            OSCL_ASSERT(false);
            break;
    }
    return status;
}

PVMFStatus PVMFMP3FFParserNode::CancelCurrentCommand()
{
    // Cancel DoFlush here and return success.
    if (IsFlushPending())
    {
        CommandComplete(iCurrentCommand, PVMFErrCancelled);
        return PVMFSuccess;
    }

    /* The pending commands DoInit, DoReset, DoGetMetadataKeys and DoGetMetadataValues
     * would be canceled in CPMCommandCompleted.
     * So return pending for now.
     */
    if (iCPMContainer.CancelPendingCommand())
    {
        return PVMFPending;//wait on sub-node cancel to complete.
    }
    else
    {
        CommandComplete(iCurrentCommand, PVMFErrCancelled);
        return PVMFSuccess;
    }
}

void PVMFMP3FFParserNode::CompleteInit(PVMFStatus aStatus)
{
    if (PVMF_GENERIC_NODE_INIT == iCurrentCommand.iCmd)
    {
        if (!iSubNodeCmdVec.empty())
        {
            iSubNodeCmdVec.front().iSubNodeContainer->CommandDone(aStatus, NULL, NULL);
        }
        else
        {
            CommandComplete(iCurrentCommand, aStatus);
        }
    }
    else // Either there is no current command or the current command is not init
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFMP3FFParserNode::CompleteInit() Failure. Either there is no current command or the current command is not init"));
        OSCL_ASSERT(false);
    }
}

// CommandHandler for Reset command
PVMFStatus PVMFMP3FFParserNode::DoReset()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::DoReset() In"));

    // Check if the node can accept the Reset command in the current state
    PVMFStatus status;

    if (iDownloadProgressInterface != NULL)
    {
        iDownloadProgressInterface->cancelResumeNotification();
    }
    if (iDurationCalcAO && iDurationCalcAO->IsBusy())
    {
        iDurationCalcAO->Cancel();
    }

    // Stop and cleanup
    ReleaseTrack();
    CleanupFileSource();
    // Cleanup CPM
    if (iCPMContainer.iCPM)
    {
        Push(iCPMContainer, PVMFSubNodeContainerBaseMp3::ECPMUsageComplete);
        Push(iCPMContainer, PVMFSubNodeContainerBaseMp3::ECPMCloseSession);
        Push(iCPMContainer, PVMFSubNodeContainerBaseMp3::ECPMReset);
        Push(iCPMContainer, PVMFSubNodeContainerBaseMp3::ECPMCleanup);
        //wait on CPM commands to execute
        status = PVMFPending;
    }
    else
    {
        status = PVMFSuccess;
    }
    return status;
}

// CommandHandler for Query UUID
PVMFStatus PVMFMP3FFParserNode::DoQueryUuid()
{
    // This node supports Query UUID from any state
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::DoQueryUuid() In"));

    OSCL_String* mimetype;
    Oscl_Vector<PVUuid, OsclMemAllocator> *uuidvec;
    bool exactmatch;
    iCurrentCommand.PVMFNodeCommandBase::Parse(mimetype, uuidvec, exactmatch);

    // Match the input mimetype against any of the custom interfaces
    // for this node
    if (*mimetype == PVMF_DATA_SOURCE_INIT_INTERFACE_MIMETYPE)
    {
        PVUuid uuid(PVMF_DATA_SOURCE_INIT_INTERFACE_UUID);
        uuidvec->push_back(uuid);
    }
    else if (*mimetype == PVMF_TRACK_SELECTION_INTERFACE_MIMETYPE)
    {
        PVUuid uuid(PVMF_TRACK_SELECTION_INTERFACE_UUID);
        uuidvec->push_back(uuid);
    }
    else if (*mimetype == PVMF_META_DATA_EXTENSION_INTERFACE_MIMETYPE)
    {
        PVUuid uuid(KPVMFMetadataExtensionUuid);
        uuidvec->push_back(uuid);
    }
    else if (*mimetype == PVMF_FF_PROGDOWNLOAD_SUPPORT_INTERFACE_MIMETYPE)
    {
        PVUuid uuid(PVMF_FF_PROGDOWNLOAD_SUPPORT_INTERFACE_UUID);
        uuidvec->push_back(uuid);
    }
    else if (*mimetype == PVMF_DATA_SOURCE_PLAYBACK_CONTROL_INTERFACE_MIMETYPE)
    {
        PVUuid uuid(PvmfDataSourcePlaybackControlUuid);
        uuidvec->push_back(uuid);
    }
    else if (*mimetype == PVMI_DATASTREAMUSER_INTERFACE_MIMETYPE)
    {
        PVUuid uuid(PVMIDatastreamuserInterfaceUuid);
        uuidvec->push_back(uuid);
    }
    return PVMFSuccess;
}

// CommandHandler for Query Interface

PVMFStatus PVMFMP3FFParserNode::DoQueryInterface()
{
    // This node supports Query Interface from any state
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::DoQueryInterface() In"));
    PVUuid* uuid;
    PVInterface** ptr;
    iCurrentCommand.PVMFNodeCommandBase::Parse(uuid, ptr);
    PVMFStatus status;

    if (queryInterface(*uuid, *ptr))
    {
        // PVMFCPMPluginLicenseInterface is not part of this node
        if (*uuid != PVMFCPMPluginLicenseInterfaceUuid)
        {
            (*ptr)->addRef();
        }
        status = PVMFSuccess;
    }
    else
    {
        // Interface not supported
        *ptr = NULL;
        status = PVMFErrNotSupported;
    }
    return status;
}

// CommandHandler for port request

PVMFStatus PVMFMP3FFParserNode::DoRequestPort(PVMFPortInterface*&aPort)
{
    // This node supports port request from any state
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::DoRequestPort In"));
    aPort = NULL;
    // Retrieve port tag.
    int32 tag;
    OSCL_String* mimetype;
    iCurrentCommand.PVMFNodeCommandBase::Parse(tag, mimetype);

    //mimetype is not used on this node
    //validate the tag
    if (tag != PVMF_MP3FFPARSER_NODE_PORT_TYPE_SOURCE)
    {
        // Invalid port tag
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFMP3FFParserNode::DoRequestPort: Error - Invalid port tag"));
        return PVMFFailure;
    }

    //Allocate a new port
    int32 err = 0;
    OSCL_TRY(err,
             iOutPort = OSCL_NEW(PVMFMP3FFParserPort, (tag, this,
                                 0, 0, 0,    // input queue isn't needed.
                                 DEFAULT_DATA_QUEUE_CAPACITY,
                                 DEFAULT_DATA_QUEUE_CAPACITY,
                                 DEFAULT_READY_TO_RECEIVE_THRESHOLD_PERCENT));
            );
    if (err != OsclErrNone || !iOutPort)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFMP3FFParserNode::DoRequestPort: Error - Out of memory"));
        return PVMFErrNoMemory;
    }

    OsclMemPoolResizableAllocator* trackdatamempool = NULL;
    PVMFResizableSimpleMediaMsgAlloc *mediadataimplalloc = NULL;
    PVMFMemPoolFixedChunkAllocator* mediadatamempool = NULL;
    MediaClockConverter* clockconv = NULL;
    err = 0;
    // Try block starts
    OSCL_TRY(err,
             // Instantiate the mem pool which will hold the actual track data
             trackdatamempool = OSCL_NEW(OsclMemPoolResizableAllocator,
                                         (2 * PVMF3FF_DEFAULT_NUM_OF_FRAMES * iMaxFrameSize, 2));

             // Instantiate an allocator for the mediadata implementation,
             // have it use the mem pool defined above as its allocator
             mediadataimplalloc = OSCL_NEW(PVMFResizableSimpleMediaMsgAlloc, (trackdatamempool));

             // Instantiate another memory pool for the media data structures.
             mediadatamempool = OSCL_NEW(PVMFMemPoolFixedChunkAllocator,
                                         ("Mp3FFPar", PVMP3FF_MEDIADATA_CHUNKS_IN_POOL,
                                          PVMP3FF_MEDIADATA_CHUNKSIZE));
             clockconv = OSCL_NEW(MediaClockConverter, (iMP3File->GetTimescale()));
            ); // Try block end

    if (err != 0 || trackdatamempool == NULL || mediadataimplalloc == NULL || mediadatamempool == NULL || clockconv == NULL)
    {
        if (clockconv)
        {
            OSCL_DELETE(clockconv);
        }
        if (mediadatamempool)
        {
            OSCL_DELETE(mediadatamempool);
        }
        if (mediadataimplalloc)
        {
            OSCL_DELETE(mediadataimplalloc);
        }
        if (trackdatamempool)
        {
            trackdatamempool->removeRef();
        }
        if (iOutPort)
        {
            OSCL_DELETE(iOutPort);
            iOutPort = NULL;
        }
        return PVMFErrNoMemory;
    }

    trackdatamempool->enablenullpointerreturn();
    mediadatamempool->enablenullpointerreturn();

    // Instantiate the PVMP3FFNodeTrackPortInfo object that contains the port.
    iTrack.iPort = iOutPort;
    iTrack.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_UNINITIALIZED;
    iTrack.iClockConverter = clockconv;
    iTrack.iMediaDataMemPool = mediadatamempool;
    iTrack.iTrackDataMemoryPool = trackdatamempool;
    iTrack.iMediaDataImplAlloc = mediadataimplalloc;
    iTrack.timestamp_offset = 0;

    // Return the port pointer to the caller.
    aPort = iOutPort;
    return PVMFSuccess;
}

// Called by the command handler AO to do the port release
PVMFStatus PVMFMP3FFParserNode::DoReleasePort()
{
    //This node supports release port from any state
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::DoReleasePort() In"));
    //Find the port in the port vector
    PVMFStatus status;
    PVMFPortInterface* ptr = NULL;
    iCurrentCommand.PVMFNodeCommandBase::Parse(ptr);

    PVMFMP3FFParserPort* port = (PVMFMP3FFParserPort*)ptr;

    if (iDurationCalcAO && iDurationCalcAO->IsBusy())
    {
        iDurationCalcAO->Cancel();
    }
    if (iOutPort == port)
    {
        ReleaseTrack();
        status = PVMFSuccess;
    }
    else
    {
        //port not found.
        status = PVMFFailure;
    }
    return status;
}

// Called by the command handler AO to do the node Init

PVMFStatus PVMFMP3FFParserNode::DoInit()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::DoInit() In"));

    PVMFStatus status;

    if (iUseCPMPluginRegistry)
    {
        //we need to go through the CPM sequence to check access on the file.
        if (oWaitingOnLicense == false)
        {
            Push(iCPMContainer, PVMFSubNodeContainerBaseMp3::ECPMInit);
        }
        else
        {
            Push(iCPMContainer, PVMFSubNodeContainerBaseMp3::ECPMApproveUsage);
            Push(iCPMContainer, PVMFSubNodeContainerBaseMp3::ECPMCheckUsage);
        }

        status = PVMFPending;
    }
    else
    {
        status = CheckForMP3HeaderAvailability();
        if (PVMFSuccess == status)
        {
            LOGINFO((0, "PVMFMP3FFParserNode::Run() CheckForMP3HeaderAvailability() succeeded"));
        }
    }
    return status;
}

// CommandHandler for node Prepare
PVMFStatus PVMFMP3FFParserNode::DoPrepare()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::DoPrepare() In"));

    // If this is an PDL session request callback from ProgressInterface when data with
    // ts TimeStamp is downloaded, DPI shall callback the node once data is downloaded
    if ((iDownloadProgressInterface != NULL) &&
            (iDownloadComplete == false))
    {
        // check for download complete
        // if download is not complete, request to be notified when data is ready
        uint32 bytesReady = 0;
        PvmiDataStreamStatus status = iDataStreamInterface->QueryReadCapacity(iDataStreamSessionID, bytesReady);
        if (status == PVDS_END_OF_STREAM)
        {
            if (!iFileSizeRecvd)
            {
                iFileSize = bytesReady;
                iFileSizeRecvd = true;
            }
            return PVMFSuccess;
        }
        if (bytesReady == 0)
        {

            uint32 ts = 0;
            iDownloadProgressInterface->requestResumeNotification(ts, iDownloadComplete);
            // Data is not available, autopause the track.
            iAutoPaused = true;
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG, (0, "PVMFMP3FFParserNode::DoPrepare() - Auto Pause Triggered, TS = %d", ts));
        }
    }
    else
    {
        // enable the duration scanner for local playback only
        if (!iDurationCalcAO)
        {
            int32 leavecode = 0;
            OSCL_TRY(leavecode, iDurationCalcAO = OSCL_NEW(PVMp3DurationCalculator,
                                                  (OsclActiveObject::EPriorityIdle, iMP3File, this)));
            if (leavecode || !iDurationCalcAO)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFMP3FFParserNode::DoInit() Duration Scan is disabled. DurationCalcAO not created"));
            }
            else
            {
                iDurationCalcAO->ScheduleAO();
            }
        }
    }
    return PVMFSuccess;
}

// CommandHandler for node Start
PVMFStatus PVMFMP3FFParserNode::DoStart()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::DoStart() In"));

    // Process Start according to the Node State
    if (iInterfaceState == EPVMFNodePaused)
    {
        iAutoPaused = false;
        // If track was in Autopause state, change track state to
        // retrieve more data in case
        if (PVMP3FFNodeTrackPortInfo::TRACKSTATE_DOWNLOAD_AUTOPAUSE == iTrack.iState)
        {
            iTrack.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_GETDATA;
        }
    }
    return PVMFSuccess;
}

// CommandHandler for node Stop
PVMFStatus PVMFMP3FFParserNode::DoStop()
{

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::DoStop() In"));

    iStreamID = 0;

    // Clear queued messages in ports
    if (iOutPort)
    {
        iOutPort->ClearMsgQueues();
    }
    // Position the parser to beginning of file
    iMP3File->SeekToTimestamp(0);
    // reset the track
    ResetTrack();

    iFileSizeRecvd = false;
    iDownloadComplete = false;
    if (iDurationCalcAO)
    {
        iDurationCalcAO->Cancel();
    }

    return PVMFSuccess;
}

// CommandHandler for node Flush
PVMFStatus PVMFMP3FFParserNode::DoFlush()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::DoFlush() In"));

    // Notify all ports to suspend their input
    if (iOutPort)
    {
        iOutPort->SuspendInput();
    }

    // reset the track
    ResetTrack();
    if (iDurationCalcAO && iDurationCalcAO->IsBusy())
    {
        iDurationCalcAO->Cancel();
    }

    // Flush is asynchronous. It will remain pending until the flush completes
    return PVMFPending;
}

/**
 * CommandHandler for fetching Metadata Keys
 */
PVMFStatus PVMFMP3FFParserNode::DoGetNodeMetadataKeys()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::DoGetNodeMetadataKeys() In"));

    /* Get Metadata keys from CPM for protected content only */
    if (iCPMContainer.iCPMMetaDataExtensionInterface != NULL)
    {
        GetCPMMetaDataKeys();
        return PVMFPending;
    }
    return (CompleteGetMetadataKeys());
}

PVMFStatus PVMFMP3FFParserNode::CompleteGetMetadataKeys()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::CompleteGetMetadataKeys() In"));

    PVMFMetadataList* keylistptr = NULL;
    uint32 starting_index;
    int32 max_entries;
    char* query_key;

    iCurrentCommand.PVMFNodeCommand::Parse(keylistptr, starting_index, max_entries, query_key);
    // Check parameters
    if (keylistptr == NULL || !iMP3File)
    {
        // The list pointer is invalid
        return PVMFErrArgument;
    }
    // The underlying mp3 ff library will fill in the keys.
    iMP3File->GetMetadataKeys(*keylistptr, starting_index, max_entries, query_key);

    /* Copy the requested keys */
    uint32 num_entries = 0;
    int32 num_added = 0;
    uint32 lcv = 0;
    for (lcv = 0; lcv < iCPMMetadataKeys.size(); lcv++)
    {
        if (query_key == NULL)
        {
            /* No query key so this key is counted */
            ++num_entries;
            if (num_entries > (uint32)starting_index)
            {
                /* Past the starting index so copy the key */
                PVMFStatus status = PushBackCPMMetadataKeys(keylistptr, lcv);
                if (PVMFErrNoMemory == status)
                {
                    return status;
                }
                num_added++;
            }
        }
        else
        {
            /* Check if the key matches the query key */
            if (pv_mime_strcmp(iCPMMetadataKeys[lcv].get_cstr(), query_key) >= 0)
            {
                /* This key is counted */
                ++num_entries;
                if (num_entries > (uint32)starting_index)
                {
                    /* Past the starting index so copy the key */
                    PVMFStatus status = PushBackCPMMetadataKeys(keylistptr, lcv);
                    if (PVMFErrNoMemory == status)
                    {
                        return status;
                    }
                    num_added++;
                }
            }
        }
        // Check if max number of entries have been copied
        if ((max_entries > 0) && (num_added >= max_entries))
        {
            break;
        }
    }
    return PVMFSuccess;
}




/**
 * CommandHandler for fetching Metadata Values
 */
PVMFStatus PVMFMP3FFParserNode::DoGetNodeMetadataValues()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::DoGetNodeMetadataValues() In"));

    PVMFMetadataList* keylistptr_in = NULL;
    PVMFMetadataList* keylistptr = NULL;
    PVMFMetadataList completeKeyList;
    Oscl_Vector<PvmiKvp, OsclMemAllocator>* valuelistptr = NULL;
    uint32 starting_index;
    int32 max_entries;

    // Extract parameters from command structure
    iCurrentCommand.PVMFNodeCommand::Parse(keylistptr_in,
                                           valuelistptr,
                                           starting_index,
                                           max_entries);

    if (iMP3File == NULL || keylistptr_in == NULL || valuelistptr == NULL)
    {
        // The list pointer is invalid, or we cannot access the mp3 ff library.
        return PVMFFailure;
    }

    keylistptr = keylistptr_in;
    if (keylistptr_in->size() == 1)
    {
        if (oscl_strncmp((*keylistptr_in)[0].get_cstr(),
                         PVMF_MP3_PARSER_NODE_ALL_METADATA_KEY,
                         oscl_strlen(PVMF_MP3_PARSER_NODE_ALL_METADATA_KEY)) == 0)
        {
            //check if the user passed in "all" metadata key, in which case get the complete
            //key list from MP3 FF lib first
            int32 max = 0x7FFFFFFF;
            char* query = NULL;
            iMP3File->GetMetadataKeys(completeKeyList, 0, max, query);
            keylistptr = &completeKeyList;
        }
    }

    // The underlying mp3 ff library will fill in the values.
    PVMFStatus status = iMP3File->GetMetadataValues(*keylistptr, *valuelistptr, starting_index, max_entries);

    iMP3ParserNodeMetadataValueCount = (*valuelistptr).size();

    if (iCPMContainer.iCPMMetaDataExtensionInterface != NULL)
    {
        iCPMGetMetaDataValuesCmdId =
            (iCPMContainer.iCPMMetaDataExtensionInterface)->GetNodeMetadataValues(iCPMContainer.iSessionId,
                    (*keylistptr_in),
                    (*valuelistptr),
                    0);
        return PVMFPending;
    }
    return status;
}

// Port Processing routines

// Port Activity Handler
void PVMFMP3FFParserNode::HandlePortActivity(const PVMFPortActivity &aActivity)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::PortActivity: port=0x%x, type=%d",
                     aActivity.iPort, aActivity.iType));

    // A port has reported some activity or state change.
    // Find out whether processing event needs to be queued
    // and/or node event needs to be reported to the observer.
    switch (aActivity.iType)
    {
        case PVMF_PORT_ACTIVITY_CREATED:
            //Report port created info event to the node.
            PVMFNodeInterface::ReportInfoEvent(PVMFInfoPortCreated,
                                               (OsclAny*)aActivity.iPort);
            break;
        case PVMF_PORT_ACTIVITY_DELETED:
            //Report port deleted info event to the node.
            PVMFNodeInterface::ReportInfoEvent(PVMFInfoPortDeleted,
                                               (OsclAny*)aActivity.iPort);
            break;
        case PVMF_PORT_ACTIVITY_CONNECT:
        case PVMF_PORT_ACTIVITY_DISCONNECT:
        case PVMF_PORT_ACTIVITY_INCOMING_MSG:
            //nothing needed.
            break;
        case PVMF_PORT_ACTIVITY_OUTGOING_MSG:
            //An outgoing message was queued on this port.
            if (!aActivity.iPort->IsConnectedPortBusy())
                Reschedule();
            break;
        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_BUSY:
            //No action is needed here, the node checks for
            //outgoing queue busy as needed during data processing.
            break;
        case PVMF_PORT_ACTIVITY_OUTGOING_QUEUE_READY:
            //Outgoing queue was previously busy, but is now ready.
            HandleOutgoingQueueReady(aActivity.iPort);
            break;
        case PVMF_PORT_ACTIVITY_CONNECTED_PORT_BUSY:
            // The connected port is busy
            // No action is needed here, the port processing code
            // checks for connected port busy during data processing.
            break;
        case PVMF_PORT_ACTIVITY_CONNECTED_PORT_READY:
            // The connected port has transitioned from Busy to Ready.
            // It's time to start processing outgoing messages again.
            if (aActivity.iPort->OutgoingMsgQueueSize() > 0)
            {
                Reschedule();
            }
            break;
        default:
            break;
    }
}

/**
 * Outgoing message handler
 */
PVMFStatus PVMFMP3FFParserNode::ProcessOutgoingMsg(PVMFPortInterface* aPort)
{
    // Called by the AO to process one message off the outgoing
    // message queue for the given port.  This routine will
    // try to send the data to the connected port.

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::ProcessOutgoingMsg: aPort=0x%x",
                     aPort));

    PVMFStatus status = aPort->Send();
    if (status == PVMFErrBusy)
    {
        // Port was busy
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "PVMFMP3FFParserNode::ProcessOutgoingMsg: \
                        Connected port goes into busy state"));
    }
    return status;
}

////////////////////////////////////////////////////////////////////////
/**
 * Active object implementation
 */
////////////////////////////////////////////////////////////////////////

/**
 * AO's entry point
 */
void PVMFMP3FFParserNode::Run()
{
    if (iCheckForMP3HeaderDuringInit)
    {
        // Read capacity notification was delivered
        iCheckForMP3HeaderDuringInit = false;
        PVMFStatus cmdStatus = CheckForMP3HeaderAvailability();
        if (PVMFSuccess == cmdStatus)
        {
            LOGINFO((0, "PVMFMP3FFParserNode::Run() CheckForMP3HeaderAvailability() succeeded"));
            // complete init command
            CompleteInit(cmdStatus);
        }
        else if (PVMFErrUnderflow == cmdStatus)
        {
            if (iMP3File)
            {
                bool nextBytes = true;
                uint32 currCapacity = 0;
                uint32 minBytesRequired = iMP3File->GetMinBytesRequired(nextBytes);
                // CheckForMP3HeaderAvailability has failed because of underflow.
                // parser needs more bytes to do parsing and verification of the clip.
                PvmiDataStreamStatus status = iDataStreamInterface->QueryReadCapacity(iDataStreamSessionID,
                                              currCapacity);

                if (PVDS_SUCCESS == status && currCapacity < minBytesRequired + iMP3MetaDataSize)
                {
                    // Request for read capacity notification was issued
                    iRequestReadCapacityNotificationID =
                        iDataStreamInterface->RequestReadCapacityNotification(iDataStreamSessionID,
                                *this,
                                iMP3MetaDataSize + minBytesRequired);
                }
            }
            LOGINFO((0, "PVMFMP3FFParserNode::Run() CheckForMP3HeaderAvailability() failed %d", cmdStatus));
        }
        else if (PVMFFailure == cmdStatus)
        {
            CompleteInit(cmdStatus);
        }
        return;
    }

    // Check InputCommandQueue, If command present
    // process commands

    if (!iInputCommands.empty())
    {
        ProcessCommand();
    }

    //Issue commands to the sub-nodes.
    if (!iCPMContainer.CmdPending() && !iSubNodeCmdVec.empty())
    {
        PVMFStatus status = iSubNodeCmdVec.front().iSubNodeContainer->IssueCommand(iSubNodeCmdVec.front().iCmd);
        if (status != PVMFPending)
        {
            iSubNodeCmdVec.front().iSubNodeContainer->CommandDone(status, NULL, NULL);
        }
    }

    if (!iOutPort)
        return;

    // Send outgoing messages
    if ((iInterfaceState == EPVMFNodeStarted || IsFlushPending()) &&
            iOutPort &&
            iOutPort->OutgoingMsgQueueSize() > 0 &&
            !iOutPort->IsConnectedPortBusy())
    {
        ProcessOutgoingMsg(iOutPort);
        // Reschedule if there is additional data to send.
        if (iOutPort->OutgoingMsgQueueSize() > 0 && !iOutPort->IsConnectedPortBusy())
        {
            Reschedule();
        }
    }

    // Create new data and send to the output queue
    if (iInterfaceState == EPVMFNodeStarted && !IsFlushPending())
    {
        if (HandleTrackState())  // Handle track state returns true if there is more data to be sent
        {
            // Reschedule if there is more data to send out
            Reschedule();
        }
    }

    // If we get here we did not process any ports or commands.
    // Check for completion of a flush command
    if (IsFlushPending() &&
            (!iOutPort || iOutPort->OutgoingMsgQueueSize() == 0))
    {
        //resume port input so the ports can be re-started.
        iOutPort->ResumeInput();
        CommandComplete(iCurrentCommand, PVMFSuccess);
    }
}

void PVMFMP3FFParserNode::PassDatastreamFactory(PVMFDataStreamFactory& aFactory,
        int32 aFactoryTag,
        const PvmfMimeString* aFactoryConfig)
{
    OSCL_UNUSED_ARG(aFactoryTag);
    OSCL_UNUSED_ARG(aFactoryConfig);

    if (iDataStreamFactory == NULL)
    {
        PVUuid uuid = PVMIDataStreamSyncInterfaceUuid;
        PVInterface* iFace = NULL;
        iDataStreamFactory = &aFactory;

#if PV_HAS_SHOUTCAST_SUPPORT_ENABLED
        if (iSourceFormat == PVMF_MIME_DATA_SOURCE_SHOUTCAST_URL)
        {
            iSCSPFactory = OSCL_NEW(PVMFShoutcastStreamParserFactory, (&aFactory, iMetadataInterval));

            iFace = iSCSPFactory->CreatePVMFCPMPluginAccessInterface(uuid);
            if (iFace != NULL)
            {
                iSCSP = OSCL_STATIC_CAST(PVMFShoutcastStreamParser*, iFace);
                if (iMetadataBuf == NULL)
                {
                    iMetadataBuf = (uint8*)oscl_malloc(PV_SCSP_MAX_METADATA_TAG_SIZE * sizeof(uint8));
                    if (iMetadataBuf != NULL)
                    {
                        iMetadataBufSize = PV_SCSP_MAX_METADATA_TAG_SIZE;
                    }
                }
            }
        }
        else
        {
            iFace = iDataStreamFactory->CreatePVMFCPMPluginAccessInterface(uuid);
        }
#else
        iFace = iDataStreamFactory->CreatePVMFCPMPluginAccessInterface(uuid);
#endif

        if (iFace != NULL)
        {
            iDataStreamInterface = OSCL_STATIC_CAST(PVMIDataStreamSyncInterface*, iFace);
            iDataStreamInterface->OpenSession(iDataStreamSessionID, PVDS_READ_ONLY);
        }
    }
    else
    {
        OSCL_ASSERT(false);
    }
}

void
PVMFMP3FFParserNode::PassDatastreamReadCapacityObserver(PVMFDataStreamReadCapacityObserver* aObserver)
{
    iDataStreamReadCapacityObserver = aObserver;
}

int32 PVMFMP3FFParserNode::convertSizeToTime(uint32 aFileSize, uint32& aNPTInMS)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::ConvertSizeToTime() aFileSize=%d, aNPTInMS=%d", aFileSize, aNPTInMS));

    return iMP3File->ConvertSizeToTime(aFileSize, aNPTInMS);
}
bool PVMFMP3FFParserNode::setProtocolInfo(Oscl_Vector<PvmiKvp*, OsclMemAllocator>& aInfoKvpVec)
{
#if PV_HAS_SHOUTCAST_SUPPORT_ENABLED

    if (aInfoKvpVec.empty())
    {
        return false;
    }
    for (uint32 i = 0; i < aInfoKvpVec.size(); i++)
    {
        if (!aInfoKvpVec[i])
        {
            return false;
        }

        if (oscl_strstr(aInfoKvpVec[i]->key, SHOUTCAST_MEDIA_DATA_LENGTH_STRING))
        {
            iMetadataInterval = aInfoKvpVec[i]->value.uint32_value;
        }
        else if (oscl_strstr(aInfoKvpVec[i]->key, SHOUTCAST_CLIP_BITRATE_STRING))
        {
            iClipByteRate = aInfoKvpVec[i]->value.uint32_value;
        }
        else if (oscl_strstr(aInfoKvpVec[i]->key, SHOUTCAST_IS_SHOUTCAST_SESSION_STRING))
        {
        }
    }
#else
    OSCL_UNUSED_ARG(aInfoKvpVec);
#endif

    return true;
}

void PVMFMP3FFParserNode::setFileSize(const uint32 aFileSize)
{
    iFileSize = aFileSize;
    iFileSizeRecvd = true;
}

void PVMFMP3FFParserNode::setDownloadProgressInterface(PVMFDownloadProgressInterface* download_progress)
{
    if (iDownloadProgressInterface)
    {
        iDownloadProgressInterface->removeRef();
    }
    iDownloadProgressInterface = download_progress;
    // get the download clock
    iDownloadProgressClock = iDownloadProgressInterface->getDownloadProgressClock();
}

void PVMFMP3FFParserNode::playResumeNotification(bool aDownloadComplete)

{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::playResumeNotification() In"));

    if (aDownloadComplete)
    {
        // DownloadProgressInterface signalled for resuming the playback
        iDownloadComplete = aDownloadComplete;

        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFMP3FFParserNode::playResumeNotification Unbinding download clock"));
        iDownloadProgressClock.Unbind();
    }

    // Resume playback
    if (iAutoPaused)
    {
        iAutoPaused = false;
        switch (iTrack.iState)
        {
            case PVMP3FFNodeTrackPortInfo::TRACKSTATE_DOWNLOAD_AUTOPAUSE:
                iTrack.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_GETDATA;
                break;
            default:
                break;
        }
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFMP3FFParserNode::playResumeNotification() Sending PVMFInfoDataReady event"));
        // Reschedule AO to resume playback
        Reschedule();
    }
}

////////////////////////////////////////////////////////////////////////
// Private section
////////////////////////////////////////////////////////////////////////

// Handle Node's track state

bool PVMFMP3FFParserNode::HandleTrackState()
{
    // Flag to be active again or not
    bool ret_status = false;

    switch (iTrack.iState)
    {
        case PVMP3FFNodeTrackPortInfo::TRACKSTATE_UNINITIALIZED:
            // Node doesnt need to do any format specific initialization
            // just skip this step and set the state to GetData
            iTrack.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_GETDATA;
            // Continue on to retrieve and send the first frame
        case PVMP3FFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_GETDATA:
            // Check if node needs to send BOS
            if (iTrack.iSendBOS)
            {
                // Send BOS downstream on all available tracks
                uint32 timestamp = iTrack.timestamp_offset;
                if (!SendBeginOfMediaStreamCommand(iTrack.iPort, iStreamID, timestamp))
                {
                    return true;
                }
                iTrack.iSendBOS = false;
            }
            // First, grab some data from the core mp3 ff parser library
            if (!RetrieveTrackData(iTrack))
            {
                // If it returns false, break out of the switch and return false,
                // this means node doesn't need to be run again in the current state.
                if (iTrack.iState == PVMP3FFNodeTrackPortInfo::TRACKSTATE_SEND_ENDOFTRACK)
                {
                    Reschedule();
                }
                break;
            }
            iSendDecodeFormatSpecificInfo = true;
            iTrack.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_SENDDATA;
            // Continue to send data
        case PVMP3FFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_SENDDATA:
            if (SendTrackData(iTrack))
            {
                iTrack.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_GETDATA;
                ret_status = true;
            }
            break;
        case PVMP3FFNodeTrackPortInfo::TRACKSTATE_SEND_ENDOFTRACK:
        {
            uint32 timestamp = iTrack.timestamp_offset;
            // Check if node needs to send BOS
            if (iTrack.iSendBOS)
            {
                // Send BOS downstream on all available tracks
                if (!SendBeginOfMediaStreamCommand(iTrack.iPort, iStreamID, timestamp))
                {
                    return true;
                }
                iTrack.iSendBOS = false;
            }

            timestamp += iTrack.timestamp_offset;
            if (SendEndOfTrackCommand(iTrack.iPort, iStreamID, timestamp, iTrack.iSeqNum++))
            {
                // EOS command sent successfully
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVMFMP3FFParserNode::HandleTrackState() EOS media command sent successfully"));
                iTrack.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_ENDOFTRACK;
                ReportInfoEvent(PVMFInfoEndOfData);
            }
            else
            {
                // EOS command sending failed -- wait on outgoing queue ready notice
                // before trying again.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_NOTICE, (0, "PVMFMP3FFParserNode::HandleTrackState() EOS media command sending failed"));
                return true;
            }
        }
        break;
        case PVMP3FFNodeTrackPortInfo::TRACKSTATE_TRACKDATAPOOLEMPTY:
        case PVMP3FFNodeTrackPortInfo::TRACKSTATE_MEDIADATAPOOLEMPTY:
        case PVMP3FFNodeTrackPortInfo::TRACKSTATE_INITIALIZED:
        case PVMP3FFNodeTrackPortInfo::TRACKSTATE_ENDOFTRACK:
        case PVMP3FFNodeTrackPortInfo::TRACKSTATE_DESTFULL:
        case PVMP3FFNodeTrackPortInfo::TRACKSTATE_SOURCEEMPTY:
        case PVMP3FFNodeTrackPortInfo::TRACKSTATE_ERROR:
        default:
            break;
    }
    return ret_status;
}

// Retrieve Data from Mp3FF

bool PVMFMP3FFParserNode::RetrieveTrackData(PVMP3FFNodeTrackPortInfo& aTrackPortInfo)
{
    // Parsing is successful, we must have estimated the duration by now.
    // Pass the duration to download progress interface
    if (iDownloadProgressInterface && iFileSizeRecvd && iFileSize > 0)
    {
        iMP3File->SetFileSize(iFileSize);
        uint32 durationInMsec = iMP3File->GetDuration();
        if (durationInMsec > 0)
        {
            iDownloadProgressInterface->setClipDuration(durationInMsec);
            int32 leavecode = 0;
            PVMFDurationInfoMessage* eventMsg = NULL;
            OSCL_TRY(leavecode, eventMsg = OSCL_NEW(PVMFDurationInfoMessage, (durationInMsec)));
            ReportInfoEvent(PVMFInfoDurationAvailable, NULL, OSCL_STATIC_CAST(PVInterface*, eventMsg));
            if (eventMsg)
            {
                eventMsg->removeRef();
            }
            iFileSize = 0;
        }
    }

    if (iAutoPaused == true)
    {
        aTrackPortInfo.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_DOWNLOAD_AUTOPAUSE;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFMP3FFParserNode::RetrieveTrackData() Node in Auto pause"));
        return false;
    }

    //Maximum number of mp3 frames to be read at a time
    uint32 numsamples = PVMF3FF_DEFAULT_NUM_OF_FRAMES;
    // Create new media data buffer from pool
    int errcode = 0;
    OsclSharedPtr<PVMFMediaDataImpl> mediaDataImplOut;

    // Try block start
    OSCL_TRY(errcode,
             mediaDataImplOut = aTrackPortInfo.iMediaDataImplAlloc->allocate(numsamples * iMaxFrameSize)
            );
    // Try block end

    if (errcode != 0)
    {
        // There was an error while allocating MediaDataImpl
        if (errcode == OsclErrNoResources)
        {
            aTrackPortInfo.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_TRACKDATAPOOLEMPTY;
            aTrackPortInfo.iTrackDataMemoryPool->notifyfreeblockavailable(*this, numsamples*iMaxFrameSize); // Enable flag to receive event when next deallocate() is called on pool
        }
        else if (errcode == OsclErrNoMemory)
        {
            // Memory allocation for the pool failed
            aTrackPortInfo.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_ERROR;
            ReportErrorEvent(PVMFErrNoMemory, NULL);
        }
        else if (errcode == OsclErrArgument)
        {
            // Invalid parameters passed to mempool
            aTrackPortInfo.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_ERROR;
            ReportErrorEvent(PVMFErrArgument, NULL);
        }
        else
        {
            // General error
            aTrackPortInfo.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_ERROR;
            ReportErrorEvent(PVMFFailure, NULL);
        }
        // All above conditions were Error conditions
        return false;
    }

    if (mediaDataImplOut.GetRep() == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_INFO, (0, "PVMFMP3FFParserNode::RetrieveTrackData() No Resource Found"));
        aTrackPortInfo.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_TRACKDATAPOOLEMPTY;
        aTrackPortInfo.iTrackDataMemoryPool->notifyfreeblockavailable(*this);
        return false;
    }

    PVMFSharedMediaDataPtr mediadataout;
    mediadataout =
        PVMFMediaData::createMediaData(mediaDataImplOut, aTrackPortInfo.iMediaDataMemPool);

    if (mediadataout.GetRep() == NULL)
    {
        aTrackPortInfo.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_MEDIADATAPOOLEMPTY;
        aTrackPortInfo.iMediaDataMemPool->notifyfreechunkavailable(*this);
        return false;
    }

    // Retrieve memory fragment to write to
    OsclRefCounterMemFrag refCtrMemFragOut;
    OsclMemoryFragment memFragOut;
    mediadataout->getMediaFragment(0, refCtrMemFragOut);
    memFragOut.ptr = refCtrMemFragOut.getMemFrag().ptr;

    // Set up the GAU structure
    GAU gau;
    gau.numMediaSamples = numsamples;
    gau.buf.num_fragments = 1;
    gau.buf.buf_states[0] = NULL;
    gau.buf.fragments[0].ptr = refCtrMemFragOut.getMemFrag().ptr;
    gau.buf.fragments[0].len = refCtrMemFragOut.getCapacity();

    // Mp3FF ErrorCode
    MP3ErrorType error = MP3_SUCCESS;
    // Grab data from the mp3 ff parser library
    int32 retval = iMP3File->GetNextBundledAccessUnits(&numsamples, &gau, error);

    // Determine actual size of the retrieved data by summing each sample length in GAU
    uint32 actualdatasize = 0;
    for (uint32 index = 0; index < numsamples; ++index)
    {
        actualdatasize += gau.info[index].len;
    }

    if (retval > 0)
    {
        // Set buffer size
        mediadataout->setMediaFragFilledLen(0, actualdatasize);
        mediaDataImplOut->setCapacity(actualdatasize);
        // Return the unused space from mempool back
        if (refCtrMemFragOut.getCapacity() > actualdatasize)
        {
            // Need to go to the resizable memory pool and free some memory
            aTrackPortInfo.iMediaDataImplAlloc->ResizeMemoryFragment(mediaDataImplOut);
        }

        // Set format specific info for the first frame
        if (iSendDecodeFormatSpecificInfo)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                            (0, "PVMFMP3FFParserNode::RetrieveTrackData() Send channel sample info"));
            iSendDecodeFormatSpecificInfo = false;
            mediadataout->setFormatSpecificInfo(iDecodeFormatSpecificInfo);
        }

        // Set M bit to 1 always - MP3 FF only outputs complete frames
        uint32 markerInfo = 0;
        markerInfo |= PVMF_MEDIA_DATA_MARKER_INFO_M_BIT;

        // Set Key Frame bit
        if (aTrackPortInfo.iFirstFrame)
        {
            markerInfo |= PVMF_MEDIA_DATA_MARKER_INFO_RANDOM_ACCESS_POINT_BIT;
            aTrackPortInfo.iFirstFrame = false;
        }
        mediaDataImplOut->setMarkerInfo(markerInfo);


        // Save the media data in the trackport info
        aTrackPortInfo.iMediaData = mediadataout;

        // Retrieve timestamp and convert to milliseconds
        aTrackPortInfo.iClockConverter->update_clock(gau.info[0].ts);
        uint32 timestamp = aTrackPortInfo.iClockConverter->get_converted_ts(COMMON_PLAYBACK_CLOCK_TIMESCALE);
        timestamp += aTrackPortInfo.timestamp_offset;

        // Set the media data timestamp
        aTrackPortInfo.iMediaData->setTimestamp(timestamp);

        // Set msg sequence number and stream id
        aTrackPortInfo.iMediaData->setSeqNum(aTrackPortInfo.iSeqNum++);
        aTrackPortInfo.iMediaData->setStreamID(iStreamID);
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "PVMFMP3FFParserNode::RetrieveTrackData Seq=%d, TS=%d", aTrackPortInfo.iSeqNum, timestamp));
        if (error == MP3_INSUFFICIENT_DATA && !iDownloadProgressInterface)
        {
            //parser reported underflow during local playback session
            if (!SendTrackData(iTrack))
            {
                // SendTrackData un-successful
                iTrack.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_SENDDATA;
                return false;
            }
        }
    }
    // EOS may occur even if some data was read
    // Also handles the condition if somehow error was not set even
    // if there is no data to read.
    if ((retval <= 0) || (MP3_SUCCESS != error))
    {
        // ffparser was not able to read data
        if (error == MP3_INSUFFICIENT_DATA) // mp3ff return insufficient data
        {
            // Check if DPI is present
            if (iDownloadProgressInterface != NULL)
            {
                if (retval > 0)
                {
                    //parser reported underflow during download playback session
                    if (!SendTrackData(iTrack))
                    {
                        // SendTrackData un-successful
                        iTrack.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_SENDDATA;
                        return true;
                    }
                }

                if (iDownloadComplete)
                {
                    aTrackPortInfo.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_SEND_ENDOFTRACK;
                    return false;
                }
                // ffparser ran out of data, need to request a call back from DPI
                // when data beyond current timestamp is downloaded
                uint32 timeStamp = iMP3File->GetTimestampForCurrentSample();
                iDownloadProgressInterface->requestResumeNotification(timeStamp, iDownloadComplete);
                // Change track state to Autopause and set iAutopause
                aTrackPortInfo.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_DOWNLOAD_AUTOPAUSE;
                iAutoPaused = true;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFMP3FFParserNode::RetrieveTrackData() \
                                Auto pause Triggered"));
                return false;
            }
            else
            {
                // if we recieve Insufficient data for local playback from parser library that means
                // its end of track, so change track state to send end of track.
                aTrackPortInfo.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_SEND_ENDOFTRACK;
                return false;
            }
        }
        else if (error == MP3_END_OF_FILE) // mp3ff return EOF
        {
            aTrackPortInfo.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_SEND_ENDOFTRACK;
            return false;
        }
        else
        {
            PVMFStatus errCode = PVMFErrArgument;
            if (error == MP3_ERROR_UNKNOWN_OBJECT)
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                                (0, "PVMFMP3FFParserNode::RetrieveTrackData() \
                                Clip format not identified by parser %d", error));
                errCode = PVMFErrNotSupported;
            }
            else
            {
                PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_NOTICE,
                                (0, "PVMFMP3FFParserNode::RetrieveTrackData() \
                                Unknown error code reported err code  %d", error));
            }
            aTrackPortInfo.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_SEND_ENDOFTRACK;
            ReportErrorEvent(errCode);
            return false;
        }
    }
    return true;
}

/**
 * Send track data to output port
 */
bool PVMFMP3FFParserNode::SendTrackData(PVMP3FFNodeTrackPortInfo& aTrackPortInfo)
{
    // Send frame to downstream node via port
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                    (0, "PVMFMP3FFParserNode::SendTrackData: SeqNum %d", \
                     aTrackPortInfo.iMediaData->getSeqNum()));

    PVMFSharedMediaMsgPtr mediaMsgOut;
    // Convert media data to media message
    convertToPVMFMediaMsg(mediaMsgOut, aTrackPortInfo.iMediaData);
    // Queue media msg to port's outgoing queue
    PVMFStatus status = aTrackPortInfo.iPort->QueueOutgoingMsg(mediaMsgOut);
    if (status != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_DEBUG,
                        (0, "PVMFMP3FFParserNode::SendTrackData: Outgoing queue busy"));
        return false;
    }

    //keep count of the number of source frames generated on this port
    aTrackPortInfo.iPort->iNumFramesGenerated++;
    // Don't need reference to iMediaData so unbind it
    aTrackPortInfo.iMediaData.Unbind();
    return true;
}

/**
 * Outgoing port queue handler
 */
bool PVMFMP3FFParserNode::HandleOutgoingQueueReady(PVMFPortInterface* aPortInterface)
{
    if (iTrack.iPort == aPortInterface &&
            iTrack.iState == PVMP3FFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_SENDDATA)
    {
        // Found the element and right state
        // re-send the data
        if (!SendTrackData(iTrack))
        {
            // SendTrackData un-successful
            return false;
        }

        // Success in re-sending the data, change state to getdata
        iTrack.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_GETDATA;
        return true;
    }
    // Either the track was not in correct state or the port was not correct
    return false;
}

/**
 * Parse the File
 */
PVMFStatus PVMFMP3FFParserNode::ParseFile()
{
    if (!iSourceURLSet)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFMP3FFParserNode::ParseFile() SourceURL not set"));
        // Can't init the node if the node, File name is not specified yet.
        return PVMFFailure;
    }

    MP3ErrorType mp3Err = iMP3File->ParseMp3File();

    if (mp3Err == MP3_INSUFFICIENT_DATA)
    {
        return PVMFErrUnderflow;
    }
    else if (mp3Err == MP3_END_OF_FILE ||
             mp3Err != MP3_SUCCESS)
    {
        SetState(EPVMFNodeError);
        ReportErrorEvent(PVMFErrResource);
        return PVMFFailure;
    }

    // Find out what the largest frame in the file is. This information is used
    // when allocating the buffers in DoRequestPort().
    iMaxFrameSize = iMP3File->GetMaxBufferSizeDB();
    if (iMaxFrameSize <= 0)
    {
        iMaxFrameSize = PVMP3FF_DEFAULT_MAX_FRAMESIZE;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "PVMFMP3FFParserNode::ParseFile() Mp3FF \
                        MaxFrameSize %d", iMaxFrameSize));
    }

    // get config Details from mp3ff
    MP3ContentFormatType mp3format;
    iConfigOk = iMP3File->GetConfigDetails(mp3format);
    if (!iConfigOk)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_INFO,
                        (0, "PVMFMP3FFParserNode::ParseFile() Mp3FF \
                        Config Not returned", iMaxFrameSize));

    }
    else
    {
        iMP3FormatBitrate = mp3format.Bitrate;
    }
    return PVMFSuccess;
}

/**
 * Reset the trackinfo
 */
void PVMFMP3FFParserNode::ResetTrack()
{
    iTrack.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_UNINITIALIZED;
    iTrack.iMediaData.Unbind();
    iTrack.iSeqNum = 0;
    iTrack.timestamp_offset = 0;
    iTrack.iSendBOS = false;
    iTrack.iFirstFrame = false;
    iAutoPaused = false;

#if PV_HAS_SHOUTCAST_SUPPORT_ENABLED
    // if this is shoutcast session,
    // reset the stream and read pointers
    if ((iSourceFormat == PVMF_MIME_DATA_SOURCE_SHOUTCAST_URL) && (iSCSPFactory != NULL))
    {
        iSCSPFactory->ResetShoutcastStream();
    }
#endif
}

/**
 * Release the trackinfo and do necessary cleanup
 */
void PVMFMP3FFParserNode::ReleaseTrack()
{
    //Cleanup allocated ports
    if (iOutPort)
    {
        OSCL_DELETE(iOutPort);
        iOutPort = NULL;
    }

    iTrack.iMediaData.Unbind();

    iTrack.iPort = NULL;

    if (iTrack.iTrackDataMemoryPool != NULL)
    {
        iTrack.iTrackDataMemoryPool->removeRef();
        iTrack.iTrackDataMemoryPool = NULL;
    }

    if (iTrack.iMediaDataImplAlloc != NULL)
    {
        OSCL_DELETE(iTrack.iMediaDataImplAlloc);
        iTrack.iMediaDataImplAlloc = NULL;
    }

    if (iTrack.iMediaDataMemPool != NULL)
    {
        iTrack.iMediaDataMemPool->CancelFreeChunkAvailableCallback();
        iTrack.iMediaDataMemPool->removeRef();
        iTrack.iMediaDataMemPool = NULL;
    }

    if (iTrack.iClockConverter != NULL)
    {
        OSCL_DELETE(iTrack.iClockConverter);
        iTrack.iClockConverter = NULL;
    }
}

/**
 * Cleanup all file sources
 */
void PVMFMP3FFParserNode::CleanupFileSource()
{
    if (iDurationCalcAO)
    {
        if (iDurationCalcAO->IsBusy())
        {
            iDurationCalcAO->Cancel();
        }

        OSCL_DELETE(iDurationCalcAO);
        iDurationCalcAO = NULL;
    }

    if (iMP3File)
    {
        OSCL_DELETE(iMP3File);
        iMP3File = NULL;
    }

    if (iDataStreamInterface != NULL)
    {
        PVInterface* iFace = OSCL_STATIC_CAST(PVInterface*, iDataStreamInterface);
        PVUuid uuid = PVMIDataStreamSyncInterfaceUuid;

#if PV_HAS_SHOUTCAST_SUPPORT_ENABLED
        if (iSCSPFactory != NULL)
        {
            iSCSPFactory->DestroyPVMFCPMPluginAccessInterface(uuid, iFace);
            iSCSP = NULL;
        }
        else
        {
            iDataStreamFactory->DestroyPVMFCPMPluginAccessInterface(uuid, iFace);
        }
#else
        iDataStreamFactory->DestroyPVMFCPMPluginAccessInterface(uuid, iFace);
#endif
        iDataStreamInterface = NULL;
    }

#if PV_HAS_SHOUTCAST_SUPPORT_ENABLED
    if (iSCSPFactory != NULL)
    {
        OSCL_DELETE(iSCSPFactory);
        iSCSPFactory = NULL;
    }
    if (iMetadataBuf != NULL)
    {
        oscl_free(iMetadataBuf);
        iMetadataBuf = NULL;
        iMetadataBufSize = 0;
        iMetadataSize = 0;
    }
#endif

    if (iDataStreamFactory != NULL)
    {
        iDataStreamFactory->removeRef();
        iDataStreamFactory = NULL;
    }
    iMP3ParserNodeMetadataValueCount = 0;

    iCPMSourceData.iFileHandle = NULL;

    if (iFileHandle)
    {
        OSCL_DELETE(iFileHandle);
        iFileHandle = NULL;
    }
    iSourceURLSet = false;
    oWaitingOnLicense = false;
    iDownloadComplete = false;
    iUseCPMPluginRegistry = false;
}

/**
 * From OsclMemPoolFixedChunkAllocatorObserver
 * Call back is received when free mem-chunk is available
 */
void PVMFMP3FFParserNode::freechunkavailable(OsclAny*)
{
    if (iTrack.iState == PVMP3FFNodeTrackPortInfo::TRACKSTATE_MEDIADATAPOOLEMPTY)
    {
        iTrack.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_GETDATA;
        // Notification is received by node for a free chunk of memory
        // Reschedule the node
        Reschedule();
    }
}

/**
 * From OsclMemPoolResizableAllocatorObserver
 * Call back is received when free mem-block is available
 */
void PVMFMP3FFParserNode::freeblockavailable(OsclAny*)
{
    if (iTrack.iState == PVMP3FFNodeTrackPortInfo::TRACKSTATE_TRACKDATAPOOLEMPTY)
    {
        iTrack.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_GETDATA;
        // Notification is received by node for a free block of memory
        // Reschedule the node
        Reschedule();
    }
}


////////////////////////////////////////////////////////////////////////
/**
 * Extension interface implementation
 */
////////////////////////////////////////////////////////////////////////

void PVMFMP3FFParserNode::addRef()
{
    ++iExtensionRefCount;
}

void PVMFMP3FFParserNode::removeRef()
{
    --iExtensionRefCount;
}

PVMFStatus PVMFMP3FFParserNode::QueryInterfaceSync(PVMFSessionId aSession,
        const PVUuid& aUuid,
        PVInterface*& aInterfacePtr)
{
    OSCL_UNUSED_ARG(aSession);
    aInterfacePtr = NULL;
    if (queryInterface(aUuid, aInterfacePtr))
    {
        // PVMFCPMPluginLicenseInterface is not part of this node
        if (aUuid != PVMFCPMPluginLicenseInterfaceUuid)
        {
            aInterfacePtr->addRef();
        }
        return PVMFSuccess;
    }
    return PVMFErrNotSupported;
}

bool PVMFMP3FFParserNode::queryInterface(const PVUuid& uuid, PVInterface*& iface)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFMP3FFParserNode::queryInterface() In"));

    if (uuid == PVMF_TRACK_SELECTION_INTERFACE_UUID)
    {
        PVMFTrackSelectionExtensionInterface* myInterface = OSCL_STATIC_CAST(PVMFTrackSelectionExtensionInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else if (uuid == PVMF_DATA_SOURCE_INIT_INTERFACE_UUID)
    {
        PVMFDataSourceInitializationExtensionInterface* myInterface = OSCL_STATIC_CAST(PVMFDataSourceInitializationExtensionInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else if (uuid == KPVMFMetadataExtensionUuid)
    {
        PVMFMetadataExtensionInterface* myInterface = OSCL_STATIC_CAST(PVMFMetadataExtensionInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else if (PvmfDataSourcePlaybackControlUuid == uuid)
    {
        PvmfDataSourcePlaybackControlInterface* myInterface = OSCL_STATIC_CAST(PvmfDataSourcePlaybackControlInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else if (uuid == PVMFCPMPluginLicenseInterfaceUuid)
    {
        iface = OSCL_STATIC_CAST(PVInterface*, iCPMContainer.iCPMLicenseInterface);
    }
    else if (PVMF_FF_PROGDOWNLOAD_SUPPORT_INTERFACE_UUID == uuid)
    {
        PVMFFormatProgDownloadSupportInterface* myInterface = OSCL_STATIC_CAST(PVMFFormatProgDownloadSupportInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else if (PVMIDatastreamuserInterfaceUuid == uuid)
    {
        PVMIDatastreamuserInterface* myInterface = OSCL_STATIC_CAST(PVMIDatastreamuserInterface*, this);
        iface = OSCL_STATIC_CAST(PVInterface*, myInterface);
    }
    else
    {
        return false;
    }
    return true;
}


/**
 * From PVMFDataSourceInitializationExtensionInterface
 */
PVMFStatus PVMFMP3FFParserNode::SetSourceInitializationData(OSCL_wString& aSourceURL,
        PVMFFormatType& aSourceFormat,
        OsclAny* aSourceData,
        PVMFFormatTypeDRMInfo aType)
{
    // Initialize the Source data
    if (iSourceURLSet)
    {
        CleanupFileSource();
    }

    if (aType != PVMF_FORMAT_TYPE_CONNECT_UNPROTECTED)
    {
        iUseCPMPluginRegistry = true;
    }
    else
    {
        iUseCPMPluginRegistry = false;
    }

    if (aSourceFormat != PVMF_MIME_MP3FF &&
            aSourceFormat != PVMF_MIME_DATA_SOURCE_SHOUTCAST_URL)
    {
        // Node doesnt support any other format than MP3/Shoutcast stream
        return PVMFFailure;
    }

    iSourceFormat = aSourceFormat;
    iSourceURL = aSourceURL;
    iSourceURLSet = true;
    if (aSourceData)
    {
        PVInterface* pvInterface = OSCL_STATIC_CAST(PVInterface*, aSourceData);
        PVInterface* localDataSrc = NULL;
        PVUuid localDataSrcUuid(PVMF_LOCAL_DATASOURCE_UUID);
        // Check if it is a local file
        if (pvInterface->queryInterface(localDataSrcUuid, localDataSrc))
        {
            PVMFLocalDataSource* opaqueData = OSCL_STATIC_CAST(PVMFLocalDataSource*, localDataSrc);
            if (opaqueData->iFileHandle)
            {
                iFileHandle = OSCL_NEW(OsclFileHandle, (*(opaqueData->iFileHandle)));
                iCPMSourceData.iFileHandle = iFileHandle;
            }
            iCPMSourceData.iPreviewMode = opaqueData->iPreviewMode;
            iCPMSourceData.iIntent = opaqueData->iIntent;
            if (opaqueData->iContentAccessFactory != NULL)
            {
                //Cannot have both plugin usage and a datastream factory
                return PVMFErrArgument;
            }
        }
        else
        {
            PVInterface* sourceDataContext = NULL;
            PVInterface* commonDataContext = NULL;
            PVUuid sourceContextUuid(PVMF_SOURCE_CONTEXT_DATA_UUID);
            PVUuid commonContextUuid(PVMF_SOURCE_CONTEXT_DATA_COMMON_UUID);
            if (pvInterface->queryInterface(sourceContextUuid, sourceDataContext))
            {
                if (sourceDataContext->queryInterface(commonContextUuid, commonDataContext))
                {
                    PVMFSourceContextDataCommon* cContext = OSCL_STATIC_CAST(
                                                                PVMFSourceContextDataCommon*,
                                                                commonDataContext);
                    if (cContext->iFileHandle)
                    {
                        iFileHandle = OSCL_NEW(OsclFileHandle, (*(cContext->iFileHandle)));
                    }
                    if (cContext->iContentAccessFactory != NULL)
                    {
                        //Cannot have both plugin usage and a datastream factory
                        return PVMFErrArgument;
                    }
                    PVMFSourceContextData* sContext = OSCL_STATIC_CAST(
                                                          PVMFSourceContextData*,
                                                          sourceDataContext);
                    iSourceContextData = *sContext;
                    iSourceContextDataValid = true;
                }
            }
        }
    }
    return PVMFSuccess;
}

PVMFStatus PVMFMP3FFParserNode::SetClientPlayBackClock(PVMFMediaClock* aClientClock)
{
    OSCL_UNUSED_ARG(aClientClock);
    return PVMFSuccess;
}

PVMFStatus PVMFMP3FFParserNode::SetEstimatedServerClock(PVMFMediaClock* aClientClock)
{
    OSCL_UNUSED_ARG(aClientClock);
    return PVMFSuccess;
}

/**
 * From PVMFTrackSelectionExtensionInterface
 */
PVMFStatus PVMFMP3FFParserNode::GetMediaPresentationInfo(PVMFMediaPresentationInfo& aInfo)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::GetMediaPresentationInfo() In"));
    if (!iMP3File)
    {
        return PVMFFailure;
    }
    aInfo.setDurationValue(iMP3File->GetDuration());

    int32 iNumTracks = iMP3File->GetNumTracks();
    if (iNumTracks <= 0)
    {
        // Number of tracks is null
        return PVMFFailure;
    }

    int32 id;
    for (id = 0; id < iNumTracks; id++)
    {
        PVMFTrackInfo tmpTrackInfo;
        // set the port tag for this track
        tmpTrackInfo.setPortTag(PVMF_MP3FFPARSER_NODE_PORT_TYPE_SOURCE);
        // track id
        tmpTrackInfo.setTrackID(0);
        // bitrate
        uint32 aBitRate = 0;
        if (iConfigOk)
        {
            aBitRate = iMP3FormatBitrate;
        }
        tmpTrackInfo.setTrackBitRate(aBitRate);
        // config info
        MP3ContentFormatType mp3Config;
        if (!iMP3File->GetConfigDetails(mp3Config))
        {
            // mp3 config not available
            return PVMFFailure;
        }

        if (!CreateFormatSpecificInfo(mp3Config.NumberOfChannels, mp3Config.SamplingRate))
        {
            return PVMFFailure;
        }

        tmpTrackInfo.setTrackConfigInfo(iDecodeFormatSpecificInfo);
        // timescale
        uint64 timescale = (uint64)iMP3File->GetTimescale();
        tmpTrackInfo.setTrackDurationTimeScale(timescale);
        // in movie timescale
        uint32 trackDuration = iMP3File->GetDuration();
        tmpTrackInfo.setTrackDurationValue(trackDuration);
        // mime type
        OSCL_FastString mime_type = _STRLIT_CHAR(PVMF_MIME_MP3);
        tmpTrackInfo.setTrackMimeType(mime_type);
        // add the track
        aInfo.addTrackInfo(tmpTrackInfo);
    }
    return PVMFSuccess;
}

bool PVMFMP3FFParserNode::CreateFormatSpecificInfo(uint32 numChannels, uint32 samplingRate)
{
    // Allocate memory for decode specific info and ref counter
    OsclMemoryFragment frag;
    frag.ptr = NULL;
    frag.len = sizeof(channelSampleInfo);
    uint refCounterSize = oscl_mem_aligned_size(sizeof(OsclRefCounterDA));
    uint8* memBuffer = (uint8*)iDecodeFormatSpecificInfoAlloc.ALLOCATE(refCounterSize + frag.len);
    if (!memBuffer)
    {
        // failure while allocating memory buffer
        return false;
    }

    oscl_memset(memBuffer, 0, refCounterSize + frag.len);
    // Create ref counter
    OsclRefCounter* refCounter = OSCL_PLACEMENT_NEW(memBuffer, OsclRefCounterDA(memBuffer,
                                 (OsclDestructDealloc*) & iDecodeFormatSpecificInfoAlloc));
    memBuffer += refCounterSize;
    // Create channel sample info
    frag.ptr = (OsclAny*)(OSCL_PLACEMENT_NEW(memBuffer, channelSampleInfo));
    ((channelSampleInfo*)frag.ptr)->desiredChannels = numChannels;
    ((channelSampleInfo*)frag.ptr)->samplingRate = samplingRate;

    // Store info in a ref counter memfrag
    iDecodeFormatSpecificInfo = OsclRefCounterMemFrag(frag, refCounter,
                                sizeof(struct channelSampleInfo));
    return true;
}

PVMFStatus PVMFMP3FFParserNode::SelectTracks(PVMFMediaPresentationInfo& aInfo)
{
    OSCL_UNUSED_ARG(aInfo);
    return PVMFSuccess;
}

// From PVMFMetadataExtensionInterface
uint32 PVMFMP3FFParserNode::GetNumMetadataKeys(char* aQueryString)
{
    uint32 num_entries = 0;
    if (iMP3File)
    {
        num_entries = iMP3File->GetNumMetadataKeys(aQueryString);
    }

    if (iCPMContainer.iCPMMetaDataExtensionInterface != NULL)
    {
        num_entries +=
            (iCPMContainer.iCPMMetaDataExtensionInterface)->GetNumMetadataKeys(aQueryString);
    }
    return num_entries;
}

uint32 PVMFMP3FFParserNode::GetNumMetadataValues(PVMFMetadataList& aKeyList)
{
    uint32 numvalentries = 0;
    if (iMP3File)
    {
        numvalentries = iMP3File->GetNumMetadataValues(aKeyList);
    }

    if (iCPMContainer.iCPMMetaDataExtensionInterface != NULL)
    {
        numvalentries +=
            iCPMContainer.iCPMMetaDataExtensionInterface->GetNumMetadataValues(aKeyList);
    }
    return numvalentries;
}

/**
 * From PVMFMetadataExtensionInterface
 * Queue an asynchronous node command for GetNodeMetadataKeys
 */
PVMFCommandId PVMFMP3FFParserNode::GetNodeMetadataKeys(PVMFSessionId aSessionId,
        PVMFMetadataList& aKeyList,
        uint32 starting_index,
        int32 max_entries,
        char* query_key,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::GetNodeMetadataKeys() In"));
    PVMFNodeCommand cmd;
    cmd.PVMFNodeCommand::Construct(aSessionId,
                                   PVMF_GENERIC_NODE_GETNODEMETADATAKEYS,
                                   aKeyList, starting_index,
                                   max_entries, query_key,
                                   aContext);
    return QueueCommandL(cmd);
}

/**
 * From PVMFMetadataExtensionInterface
 * Queue an asynchronous node command for GetNodeMetadataValues
 */
PVMFCommandId PVMFMP3FFParserNode::GetNodeMetadataValues(PVMFSessionId aSessionId,
        PVMFMetadataList& aKeyList,
        Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
        uint32 starting_index,
        int32 max_entries,
        const OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::GetNodeMetadataValues() In"));

    PVMFNodeCommand cmd;
    cmd.PVMFNodeCommand::Construct(aSessionId,
                                   PVMF_GENERIC_NODE_GETNODEMETADATAVALUES,
                                   aKeyList, aValueList,
                                   starting_index, max_entries,
                                   aContext);
    return QueueCommandL(cmd);
}

/**
 * From PVMFMetadataExtensionInterface
 * Queue an asynchronous node command for ReleaseNodeMetadataKeys
 */
PVMFStatus PVMFMP3FFParserNode::ReleaseNodeMetadataKeys(PVMFMetadataList& aMetaDataKeys,
        uint32 , uint32)
{
    OSCL_UNUSED_ARG(aMetaDataKeys);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::ReleaseNodeMetadataKeys() In"));
    return PVMFErrNotSupported;
}

/**
 * From PVMFMetadataExtensionInterface
 * Queue an asynchronous node command for ReleaseNodeMetadataValues
 */
PVMFStatus PVMFMP3FFParserNode::ReleaseNodeMetadataValues(Oscl_Vector<PvmiKvp, OsclMemAllocator>& aValueList,
        uint32 start, uint32 end)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE, (0, "PVMFMP3FFParserNode::ReleaseNodeMetadataValues() In"));
    if (iMP3File == NULL)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFMP3FFParserNode::ReleaseNodeMetadataValues() \
                       MP3 file not parsed yet"));
        return PVMFFailure;
    }

    end = OSCL_MIN(aValueList.size(), iMP3ParserNodeMetadataValueCount);

    if (start > end || aValueList.size() == 0)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFMP3FFParserNode::ReleaseNodeMetadataValues() \
                        Invalid start/end index"));
        return PVMFErrArgument;
    }

    // Go through the specified values and free it
    for (uint32 i = start; i < end; i++)
    {
        iMP3File->ReleaseMetadataValue(aValueList[i]);
    }
    return PVMFSuccess;
}

/**
 * From PvmfDataSourcePlaybackControlInterface
 * Queue an asynchronous node command for SetDataSourcePosition
 */
PVMFCommandId PVMFMP3FFParserNode::SetDataSourcePosition(PVMFSessionId aSessionId,
        PVMFTimestamp aTargetNPT,
        PVMFTimestamp& aActualNPT,
        PVMFTimestamp& aActualMediaDataTS,
        bool aSeekToSyncPoint,
        uint32 aStreamID,
        OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::SetDataSourcePosition()"));
    PVMFNodeCommand cmd;
    cmd.PVMFNodeCommand::Construct(aSessionId,
                                   PVMF_GENERIC_NODE_SET_DATASOURCE_POSITION,
                                   aTargetNPT, &aActualNPT,
                                   &aActualMediaDataTS, aSeekToSyncPoint,
                                   aStreamID, aContext);
    return QueueCommandL(cmd);
}

/**
 * From PvmfDataSourcePlaybackControlInterface
 * Queue an asynchronous node command for QueryDataSourcePosition
 */
PVMFCommandId PVMFMP3FFParserNode::QueryDataSourcePosition(PVMFSessionId aSessionId,
        PVMFTimestamp aTargetNPT,
        PVMFTimestamp& aActualNPT,
        bool aSeekToSyncPoint,
        OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::QueryDataSourcePosition()"));
    PVMFNodeCommand cmd;
    cmd.PVMFNodeCommand::Construct(aSessionId,
                                   PVMF_GENERIC_NODE_QUERY_DATASOURCE_POSITION,
                                   aTargetNPT, &aActualNPT,
                                   aSeekToSyncPoint, aContext);
    return QueueCommandL(cmd);
}

/**
 * From PvmfDataSourcePlaybackControlInterface
 * Queue an asynchronous node command for QueryDataSourcePosition
 */
PVMFCommandId PVMFMP3FFParserNode::QueryDataSourcePosition(PVMFSessionId aSessionId,
        PVMFTimestamp aTargetNPT,
        PVMFTimestamp& aSeekPointBeforeTargetNPT,
        PVMFTimestamp& aSeekPointAfterTargetNPT,
        OsclAny* aContextData,
        bool aSeekToSyncPoint)
{
    OSCL_UNUSED_ARG(aSeekPointAfterTargetNPT);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::QueryDataSourcePosition()"));
    PVMFNodeCommand cmd;
    // Construct call is not changed, the aSeekPointBeforeTargetNPT will
    // contain the replaced actualNPT
    cmd.PVMFNodeCommand::Construct(aSessionId,
                                   PVMF_GENERIC_NODE_QUERY_DATASOURCE_POSITION,
                                   aTargetNPT, &aSeekPointBeforeTargetNPT,
                                   aSeekToSyncPoint, aContextData);
    return QueueCommandL(cmd);
}

/**
 * From PvmfDataSourcePlaybackControlInterface
 * Queue an asynchronous node command for SetDataSourceRate
 */
PVMFCommandId PVMFMP3FFParserNode::SetDataSourceRate(PVMFSessionId aSessionId,
        int32 aRate,
        PVMFTimebase* aTimebase,
        OsclAny* aContext)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::SetDataSourcePosition()"));
    PVMFNodeCommand cmd;
    cmd.PVMFNodeCommand::Construct(aSessionId,
                                   PVMF_GENERIC_NODE_SET_DATASOURCE_RATE,
                                   aRate, aTimebase,
                                   aContext);
    return QueueCommandL(cmd);
}

/**
 * Command Handler for SetDataSourceRate
 */
PVMFStatus PVMFMP3FFParserNode::DoSetDataSourceRate()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::DoSetDataSourceRate() In"));

    return PVMFSuccess;
}

/**
 * Command Handler for SetDataSourcePosition
 */
PVMFStatus PVMFMP3FFParserNode::DoSetDataSourcePosition()
{
    uint32 targetNPT = 0;
    uint32* actualNPT = NULL;
    uint32* actualMediaDataTS = NULL;
    bool seektosyncpoint = false;
    uint32 streamID = 0;

    // if progressive streaming, reset download complete flag
    if ((NULL != iDataStreamInterface) && (0 != iDataStreamInterface->QueryBufferingCapacity()))
    {
        iDownloadComplete = false;
    }

    iCurrentCommand.PVMFNodeCommand::Parse(targetNPT, actualNPT,
                                           actualMediaDataTS, seektosyncpoint,
                                           streamID);

    iStreamID = streamID;
    iTrack.iSendBOS = true;
    iTrack.iFirstFrame = true;

    if (iDownloadProgressClock.GetRep())
    {
        // Get the amount downloaded so far
        bool tmpbool = false;
        uint32 dltime = 0;
        iDownloadProgressClock->GetCurrentTime32(dltime, tmpbool, PVMF_MEDIA_CLOCK_MSEC);
        // Check if the requested time is past the downloaded clip
        if (targetNPT >= dltime)
        {
            // For now, fail in this case. In future, we want to reposition to valid location.
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                            (0, "PVMFMP3FFParserNode::DoSetDataSourcePosition() \
                            Positioning past the amount downloaded so return as \
                            argument error"));
            return PVMFErrArgument;
        }
    }
    // get the clock offset
    iTrack.iClockConverter->update_clock(iMP3File->GetTimestampForCurrentSample());
    iTrack.timestamp_offset += iTrack.iClockConverter->get_converted_ts(COMMON_PLAYBACK_CLOCK_TIMESCALE);
    // Set the timestamp
    *actualMediaDataTS = iTrack.timestamp_offset;
    // See if targetNPT is greater or equal to clip duration
    uint32 duration = iMP3File->GetDuration();
    if (duration > 0 && targetNPT >= duration)
    {
        // report End of Stream on the track and reset the track to zero.
        iTrack.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_SEND_ENDOFTRACK;
        iMP3File->SeekToTimestamp(0);
        *actualNPT = duration;
        iTrack.iClockConverter->set_clock_other_timescale(*actualNPT, COMMON_PLAYBACK_CLOCK_TIMESCALE);
        iTrack.timestamp_offset -= *actualNPT;
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                        (0, "PVMFMP3FFParserNode::DoSetDataSourcePosition: targetNPT=%d, actualNPT=%d, actualMediaTS=%d",
                         targetNPT, *actualNPT, *actualMediaDataTS));
        if (iAutoPaused)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFMP3FFParserNode::DoSetDataSourcePosition Track Autopaused"));
            iAutoPaused = false;
            if (iDownloadProgressInterface != NULL)
            {
                iDownloadProgressInterface->cancelResumeNotification();
            }
        }
        return PVMFSuccess;
    }
    // Seek to the next NPT
    // MP3 FF seeks to the beginning if the requested time is past the end of clip
    *actualNPT = iMP3File->SeekToTimestamp(targetNPT);

    if (duration > 0 && *actualNPT == duration)
    {
        // this means there was no data to render after the seek so just send End of Track
        iTrack.iClockConverter->set_clock_other_timescale(*actualNPT, COMMON_PLAYBACK_CLOCK_TIMESCALE);
        iTrack.timestamp_offset -= *actualNPT;
        iTrack.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_SEND_ENDOFTRACK;
        return PVMFSuccess;
    }

    iTrack.iClockConverter->set_clock_other_timescale(*actualNPT, COMMON_PLAYBACK_CLOCK_TIMESCALE);
    iTrack.timestamp_offset -= *actualNPT;

    // Reposition has occured, so reset the track state
    if (iAutoPaused)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFMP3FFParserNode::DoSetDataSourcePosition Track Autopaused"));
        iAutoPaused = false;
        if (iDownloadProgressInterface != NULL)
        {
            iDownloadProgressInterface->cancelResumeNotification();
        }
    }
    iTrack.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_GETDATA;

    PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR,
                    (0, "PVMFMP3FFParserNode::DoSetDataSourcePosition: targetNPT=%d, actualNPT=%d, actualMediaTS=%d",
                     targetNPT, *actualNPT, *actualMediaDataTS));
    return PVMFSuccess;
}

/**
 * Command Handler for QueryDataSourcePosition
 */
PVMFStatus PVMFMP3FFParserNode::DoQueryDataSourcePosition()
{
    uint32 targetNPT = 0;
    uint32* actualNPT = NULL;
    bool seektosyncpoint = false;

    iCurrentCommand.PVMFNodeCommand::Parse(targetNPT, actualNPT, seektosyncpoint);
    if (actualNPT == NULL)
    {
        return PVMFErrArgument;
    }
    // First check if MP3 file is being PDed to make sure the requested
    // position is before amount downloaded
    if (iDownloadProgressClock.GetRep())
    {
        // Get the amount downloaded so far
        bool tmpbool = false;
        uint32 dltime = 0;
        iDownloadProgressClock->GetCurrentTime32(dltime, tmpbool, PVMF_MEDIA_CLOCK_MSEC);
        // Check if the requested time is past clip dl
        if (targetNPT >= dltime)
        {
            return PVMFErrArgument;
        }
    }
    // Determine the actual NPT without actually repositioning
    // MP3 FF goes to the beginning if the requested time is past the end of clip
    *actualNPT = targetNPT;
    uint32 duration = iMP3File->GetDuration();
    if (duration > 0 && targetNPT >= duration)
    {
        // return without any call on parser library.
        return PVMFSuccess;
    }
    iMP3File->SeekPointFromTimestamp(*actualNPT);
    return PVMFSuccess;
}

/**
 * Queues SubNode Commands
 */
void PVMFMP3FFParserNode::Push(PVMFSubNodeContainerBaseMp3& c, PVMFSubNodeContainerBaseMp3::CmdType cmd)
{
    SubNodeCmd snc;
    snc.iSubNodeContainer = &c;
    snc.iCmd = cmd;
    iSubNodeCmdVec.push_back(snc);
}

PVMFCommandId PVMFCPMContainerMp3::GetCPMLicenseInterface()
{
    iCPMLicenseInterfacePVI = NULL;
    return (iCPM->QueryInterface(iSessionId,
                                 PVMFCPMPluginLicenseInterfaceUuid,
                                 iCPMLicenseInterfacePVI));
}


PVMFStatus PVMFCPMContainerMp3::CheckApprovedUsage()
{
    //compare the approved and requested usage bitmaps
    if ((iApprovedUsage.value.uint32_value & iRequestedUsage.value.uint32_value)
            != iRequestedUsage.value.uint32_value)
    {
        return PVMFErrAccessDenied;//media access denied by CPM.
    }
    return PVMFSuccess;
}

PVMFStatus PVMFCPMContainerMp3::CreateUsageKeys()
{
    iCPMContentType = iCPM->GetCPMContentType(iSessionId);
    if ((iCPMContentType != PVMF_CPM_FORMAT_OMA1) &&
            (iCPMContentType != PVMF_CPM_FORMAT_AUTHORIZE_BEFORE_ACCESS))
    {
        return PVMFFailure;//invalid content type.
    }

    //cleanup any old usage keys
    CleanUsageKeys();

    int32 UseKeyLen = oscl_strlen(_STRLIT_CHAR(PVMF_CPM_REQUEST_USE_KEY_STRING));
    int32 AuthKeyLen = oscl_strlen(_STRLIT_CHAR(PVMF_CPM_AUTHORIZATION_DATA_KEY_STRING));
    int32 leavecode = 0;

    OSCL_TRY(leavecode,
             iRequestedUsage.key = OSCL_ARRAY_NEW(char, UseKeyLen + 1);
             iApprovedUsage.key = OSCL_ARRAY_NEW(char, UseKeyLen + 1);
             iAuthorizationDataKvp.key = OSCL_ARRAY_NEW(char, AuthKeyLen + 1);
            );

    if (leavecode || !iRequestedUsage.key || !iApprovedUsage.key || !iAuthorizationDataKvp.key)
    {
        // Leave occured, do neccessary cleanup
        CleanUsageKeys();
        return PVMFErrNoMemory;
    }

    oscl_strncpy(iRequestedUsage.key, PVMF_CPM_REQUEST_USE_KEY_STRING, UseKeyLen);
    iRequestedUsage.key[UseKeyLen] = 0;
    iRequestedUsage.length = 0;
    iRequestedUsage.capacity = 0;
    iRequestedUsage.value.uint32_value =
        (BITMASK_PVMF_CPM_DRM_INTENT_PLAY |
         BITMASK_PVMF_CPM_DRM_INTENT_PAUSE |
         BITMASK_PVMF_CPM_DRM_INTENT_SEEK_FORWARD |
         BITMASK_PVMF_CPM_DRM_INTENT_SEEK_BACK);

    oscl_strncpy(iApprovedUsage.key, PVMF_CPM_REQUEST_USE_KEY_STRING, UseKeyLen);
    iApprovedUsage.key[UseKeyLen] = 0;
    iApprovedUsage.length = 0;
    iApprovedUsage.capacity = 0;
    iApprovedUsage.value.uint32_value = 0;

    oscl_strncpy(iAuthorizationDataKvp.key, _STRLIT_CHAR(PVMF_CPM_AUTHORIZATION_DATA_KEY_STRING),
                 AuthKeyLen);
    iAuthorizationDataKvp.key[AuthKeyLen] = 0;

    if ((iCPMContentType == PVMF_CPM_FORMAT_OMA1) ||
            (iCPMContentType == PVMF_CPM_FORMAT_AUTHORIZE_BEFORE_ACCESS))
    {
        iAuthorizationDataKvp.length = 0;
        iAuthorizationDataKvp.capacity = 0;
        iAuthorizationDataKvp.value.pUint8_value = NULL;
    }
    else
    {
        CleanUsageKeys();
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iContainer->iLogger, PVLOGMSG_STACK_TRACE,
                        (0, "PVMFCPMContainerMp3::CreateUsageKeys Usage key creation failed"));
        return PVMFFailure;
    }
    return PVMFSuccess;
}

void PVMFCPMContainerMp3::CleanUsageKeys()
{

    if (iRequestedUsage.key)
    {
        OSCL_ARRAY_DELETE(iRequestedUsage.key);
        iRequestedUsage.key = NULL;
    }

    if (iApprovedUsage.key)
    {
        OSCL_ARRAY_DELETE(iApprovedUsage.key);
        iApprovedUsage.key = NULL;
    }

    if (iAuthorizationDataKvp.key)
    {
        OSCL_ARRAY_DELETE(iAuthorizationDataKvp.key);
        iAuthorizationDataKvp.key = NULL;
    }
}

void PVMFCPMContainerMp3::Cleanup()
{
    //cleanup usage keys
    CleanUsageKeys();

    //cleanup cpm access
    if (iCPMContentAccessFactory)
    {
        iCPMContentAccessFactory->removeRef();
        iCPMContentAccessFactory = NULL;
    }


    //cleanup CPM object.
    if (iCPM)
    {
        iCPM->ThreadLogoff();
        PVMFCPMFactory::DestroyContentPolicyManager(iCPM);
        iCPM = NULL;
    }
}

PVMFStatus PVMFCPMContainerMp3::IssueCommand(int32 aCmd)
{
    // Issue a command to the sub-node.
    // Return the sub-node completion status: either pending, success, or failure.
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iContainer->iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFCPMContainerMp3::IssueCommand In"));

    OSCL_ASSERT(iCmdState == EIdle && iCancelCmdState == EIdle);
    // Find the current node command since we may need its parameters.
    OSCL_ASSERT(iContainer->IsCommandInProgress(iContainer->iCurrentCommand));

    //save the sub-node command code
    iCmd = aCmd;

    switch (aCmd)
    {
        case ECPMCleanup:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iContainer->iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFCPMContainerMp3::IssueCommand Calling Cleanup"));
            Cleanup();
            return PVMFSuccess;

        case ECPMInit:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iContainer->iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFCPMContainerMp3::IssueCommand Calling Init"));
            //make sure any prior instance is cleaned up
            Cleanup();
            //Create a CPM instance.
            OSCL_ASSERT(iCPM == NULL);
            iCPM = PVMFCPMFactory::CreateContentPolicyManager(*this);
            if (!iCPM)
            {
                return PVMFErrNoMemory;
            }
            //thread logon may leave if there are no plugins
            int32 err;
            OSCL_TRY(err, iCPM->ThreadLogon(););
            if (err != OsclErrNone)
            {
                //end the sequence now.
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iContainer->iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFCPMContainerMp3::IssueCommand No plugins, ending CPM sequence"));

                iCPM->ThreadLogoff();
                PVMFCPMFactory::DestroyContentPolicyManager(iCPM);
                iCPM = NULL;

                //treat it as unprotected content.
                PVMFStatus status = iContainer->CheckForMP3HeaderAvailability();
                return status;
            }
            else
            {
                //continue the sequence
                iContainer->Push(*this, PVMFSubNodeContainerBaseMp3::ECPMOpenSession);
                iContainer->Push(*this, PVMFSubNodeContainerBaseMp3::ECPMRegisterContent);

                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iContainer->iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFCPMContainerMp3::IssueCommand Calling Init"));

                iCmdState = EBusy;
                iCmdId = iCPM->Init();
                return PVMFPending;
            }

        case ECPMOpenSession:
            OSCL_ASSERT(iCPM != NULL);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iContainer->iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFCPMContainerMp3::IssueCommand Calling OpenSession"));
            iCmdState = EBusy;
            iCmdId = iCPM->OpenSession(iSessionId);
            return PVMFPending;

        case ECPMRegisterContent:
            OSCL_ASSERT(iCPM != NULL);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iContainer->iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFCPMContainerMp3::IssueCommand Calling RegisterContent"));
            iCmdState = EBusy;
            iCmdId = iCPM->RegisterContent(iSessionId,
                                           iContainer->iSourceURL,
                                           iContainer->iSourceFormat,
                                           (OsclAny*) & iContainer->iCPMSourceData);
            return PVMFPending;

        case ECPMGetLicenseInterface:
            OSCL_ASSERT(iCPM != NULL);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iContainer->iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFCPMContainerMp3::IssueCommand Calling GetCPMLicenseInterface"));
            iCmdState = EBusy;
            iCmdId = GetCPMLicenseInterface();
            return PVMFPending;

        case ECPMApproveUsage:
        {
            PVMFStatus status = PVMFFailure;
            OSCL_ASSERT(iCPM != NULL);
            GetCPMMetaDataExtensionInterface();
            iCPMContentType = iCPM->GetCPMContentType(iSessionId);
            if ((iCPMContentType == PVMF_CPM_FORMAT_OMA1) ||
                    (iCPMContentType == PVMF_CPM_FORMAT_AUTHORIZE_BEFORE_ACCESS))
            {
                iCmdState = EBusy;
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iContainer->iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFCPMContainerMp3::IssueCommand Calling ApproveUsage"));

                //Create the usage keys
                {
                    status = CreateUsageKeys();
                    if (status != PVMFSuccess)
                    {
                        return status;
                    }
                }
                iCPM->GetContentAccessFactory(iSessionId, iCPMContentAccessFactory);

                if (iContainer->iDataStreamReadCapacityObserver != NULL)
                {
                    iCPMContentAccessFactory->SetStreamReadCapacityObserver(iContainer->iDataStreamReadCapacityObserver);
                }

                iCmdId = iCPM->ApproveUsage(iSessionId,
                                            iRequestedUsage,
                                            iApprovedUsage,
                                            iAuthorizationDataKvp,
                                            iUsageID);
                iContainer->oWaitingOnLicense = true;
                status = PVMFPending;
            }
            else
            {
                /* Unsupported format - use it as unprotected content */
                status = iContainer->CheckForMP3HeaderAvailability();
            }
            return status;
        }

        case ECPMCheckUsage:
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iContainer->iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFCPMContainerMp3::IssueCommand Calling ECPMCheckUsage"));
            iContainer->oWaitingOnLicense = false;
            PVMFStatus status = PVMFSuccess;
            //Check for usage approval, and if approved, parse the file.
            if ((iCPMContentType == PVMF_CPM_FORMAT_OMA1) ||
                    (iCPMContentType == PVMF_CPM_FORMAT_AUTHORIZE_BEFORE_ACCESS))
            {
                status = CheckApprovedUsage();
                if (status != PVMFSuccess)
                {
                    return status;
                }
                if (!iCPMContentAccessFactory)
                {
                    return PVMFFailure;//unexpected, since ApproveUsage succeeded.
                }
                status = iContainer->CheckForMP3HeaderAvailability();
            }
            return status;
        }


        case ECPMUsageComplete:
            if ((iCPMContentType == PVMF_CPM_FORMAT_OMA1) ||
                    (iCPMContentType == PVMF_CPM_FORMAT_AUTHORIZE_BEFORE_ACCESS))
            {
                OSCL_ASSERT(iCPM != NULL);
                PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iContainer->iLogger, PVLOGMSG_STACK_TRACE,
                                (0, "PVMFCPMContainerMp3::IssueCommand Calling UsageComplete"));

                iCmdState = EBusy;
                iContainer->iCPMUsageCompleteCmdId = iCPM->UsageComplete(iSessionId, iUsageID);
                return PVMFPending;
            }
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iContainer->iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFCPMContainerMp3::IssueCommand Calling UsageComplete"));
            return PVMFSuccess;

        case ECPMCloseSession:
            OSCL_ASSERT(iCPM != NULL);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iContainer->iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFCPMContainerMp3::IssueCommand Calling CloseSession"));
            iCmdState = EBusy;
            iContainer->iCPMCloseSessionCmdId = iCPM->CloseSession(iSessionId);
            return PVMFPending;
        case ECPMReset:
            OSCL_ASSERT(iCPM != NULL);
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iContainer->iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFCPMContainerMp3::IssueCommand Calling Reset"));
            iCmdState = EBusy;
            iContainer->iCPMResetCmdId = iCPM->Reset();
            return PVMFPending;
        default:
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iContainer->iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFMP3FFParserNode::IssueCommand Failure. Command not recognized"));
            OSCL_ASSERT(false);
            return PVMFFailure;
    }
}

bool PVMFCPMContainerMp3::CancelPendingCommand()
{
    // Initiate sub-node command cancel, return True if cancel initiated.
    if (iCmdState != EBusy)
    {
        return false;//nothing to cancel
    }
    iCancelCmdState = EBusy;

    return true;//cancel initiated
}

/**
 * From PVMFCPMStatusObserver: callback from the CPM object.
 */
OSCL_EXPORT_REF void PVMFCPMContainerMp3::CPMCommandCompleted(const PVMFCmdResp& aResponse)
{
    //A command to the CPM node is complete
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iContainer->iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFCPMContainerMp3::CPMCommandCompleted "));

    PVMFCommandId aCmdId = aResponse.GetCmdId();

    if (aCmdId == iCmdId && iCmdState == EBusy)
    {
        //this is decision point, if CPM does not care about the content
        //skip rest of the CPM steps
        if (iCmd == ECPMRegisterContent)
        {
            PVMFStatus status = aResponse.GetCmdStatus();
            if (status == PVMFErrNotSupported)
            {
                //if CPM comes back as PVMFErrNotSupported then by pass rest of the CPM
                //sequence. Fake success here so that node doesnt treat this as an error
                status = iContainer->CheckForMP3HeaderAvailability();
                if (status == PVMFPending)
                {
                    return;
                }
            }
            else if (status == PVMFSuccess)
            {
                //proceed with rest of the CPM steps
                iContainer->Push(iContainer->iCPMContainer,
                                 PVMFSubNodeContainerBaseMp3::ECPMGetLicenseInterface);
                iContainer->Push(iContainer->iCPMContainer,
                                 PVMFSubNodeContainerBaseMp3::ECPMApproveUsage);
                iContainer->Push(iContainer->iCPMContainer,
                                 PVMFSubNodeContainerBaseMp3::ECPMCheckUsage);
            }
            CommandDone(status,
                        aResponse.GetEventExtensionInterface(),
                        aResponse.GetEventData());
        }
        else
        {
            if (iCmd == ECPMGetLicenseInterface)
            {
                iCPMLicenseInterface = OSCL_STATIC_CAST(PVMFCPMPluginLicenseInterface*, iCPMLicenseInterfacePVI);
                iCPMLicenseInterfacePVI = NULL;
            }
            CommandDone(aResponse.GetCmdStatus(),
                        aResponse.GetEventExtensionInterface(),
                        aResponse.GetEventData());
        }
        //catch completion of cancel for CPM commands
        //since there's no cancel to the CPM module, the cancel
        //is done whenever the current CPM command is done.
        if (iCancelCmdState != EIdle)
        {
            CancelCommandDone(PVMFSuccess, NULL, NULL);
        }
    }
    else if (aResponse.GetCmdId() == iCancelCmdId
             && iCancelCmdState == EBusy)
    {
        //Process node cancel command response
        CancelCommandDone(aResponse.GetCmdStatus(), aResponse.GetEventExtensionInterface(), aResponse.GetEventData());
    }
    else if (aResponse.GetCmdId() == iContainer->iCPMGetMetaDataKeysCmdId)
    {
        // End of GetNodeMetaDataKeys
        PVMFStatus status =
            iContainer->CompleteGetMetadataKeys();
        iContainer->CommandComplete(iContainer->iCurrentCommand, status);
    }
    else if (aResponse.GetCmdId() == iContainer->iCPMGetMetaDataValuesCmdId)
    {
        // End of GetNodeMetaDataValues
        iContainer->CommandComplete(iContainer->iCurrentCommand, aResponse.GetCmdStatus());
    }
    else if (aResponse.GetCmdId() == iContainer->iCPMUsageCompleteCmdId ||
             aResponse.GetCmdId() == iContainer->iCPMCloseSessionCmdId)
    {
        //In case these commands fail, ignore them
        CommandDone(PVMFSuccess,
                    aResponse.GetEventExtensionInterface(),
                    aResponse.GetEventData());
    }
    else if (aResponse.GetCmdId() == iContainer->iCPMResetCmdId)
    {
        if (aResponse.GetCmdStatus() != PVMFSuccess)
        {
            OSCL_ASSERT(false);
        }
        else
        {
            CommandDone(PVMFSuccess,
                        aResponse.GetEventExtensionInterface(),
                        aResponse.GetEventData());
        }
    }
    else
    {
        OSCL_ASSERT(false);//unexpected response
    }
}

void PVMFSubNodeContainerBaseMp3::CommandDone(PVMFStatus aStatus, PVInterface*aExtMsg,
        OsclAny*aEventData)
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iContainer->iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFCPMContainerMp3::CommandDone "));
    // Sub-node command is completed, process the result.
    OSCL_ASSERT(aStatus != PVMFPending);
    // Pop the sub-node command vector.
    OSCL_ASSERT(!iContainer->iSubNodeCmdVec.empty());
    iContainer->iSubNodeCmdVec.erase(&iContainer->iSubNodeCmdVec.front());

    iCmdState = EIdle;
    PVMFStatus status = aStatus;
    //Check whether the node command is being cancelled.
    if (iCancelCmdState != EIdle)
    {
        if (!iContainer->iSubNodeCmdVec.empty())
        {
            //even if this command succeeded, we want to report
            //the node command status as cancelled since some sub-node
            //commands were not yet issued.
            status = PVMFErrCancelled;
            //go into an error state since the command is partially completed
            iContainer->SetState(EPVMFNodeError);
        }
    }
    //figure out the next step in the sequence
    //A node command is done when either all sub-node commands are
    //done or when one fails.
    if (status == PVMFSuccess && !iContainer->iSubNodeCmdVec.empty())
    {
        //The node needs to issue the next sub-node command.
        iContainer->Reschedule();
    }
    else
    {
        //node command is done.
        OSCL_ASSERT(iContainer->IsCommandInProgress(iContainer->iCurrentCommand));
        iContainer->CommandComplete(iContainer->iCurrentCommand, status, aExtMsg, aEventData);
    }
}

void PVMFSubNodeContainerBaseMp3::CancelCommandDone(PVMFStatus aStatus, PVInterface*aExtMsg, OsclAny*aEventData)
{
    // Sub-node cancel command is done: process the result
    OSCL_UNUSED_ARG(aExtMsg);
    OSCL_UNUSED_ARG(aEventData);

    OSCL_ASSERT(aStatus != PVMFPending);
    iCancelCmdState = EIdle;
    //print and ignore any failed sub-node cancel commands.
    if (aStatus != PVMFSuccess)
    {
        PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iContainer->iLogger, PVLOGMSG_ERR, (0,
                        "PVMFCPMContainerMp3::CancelCommandDone CPM Node Cancel failed"));
    }

    //Node cancel command is now done.
    OSCL_ASSERT(iContainer->IsCommandInProgress(iContainer->iCurrentCommand));
    iContainer->CommandComplete(iContainer->iCurrentCommand, PVMFErrCancelled);
    iContainer->CommandComplete(iContainer->iCancelCommand, aStatus);
}

/**
 * From PvmiDataStreamObserver: callback when the DataStreamCommand is completed
 */
void PVMFMP3FFParserNode::DataStreamCommandCompleted(const PVMFCmdResp& aResponse)
{
    if (PVMF_GENERIC_NODE_INIT == iCurrentCommand.iCmd)
    {
        PVMFStatus cmdStatus = PVMFFailure;
        if (aResponse.GetCmdId() == iRequestReadCapacityNotificationID)
        {
            cmdStatus = aResponse.GetCmdStatus();
            if (cmdStatus == PVMFSuccess)
            {
                // set flag
                iCheckForMP3HeaderDuringInit = true;
                // read capacity notification is received by node,
                // i.e. amount of data that was requested by node
                // has been downloaded, reschedule now
                Reschedule();
            }
            else
            {
                LOGINFO((0, "PVMFMP3FFParserNode::DataStreamCommandCompleted() RequestReadCapacityNotification failed %d", cmdStatus));
                // command init command with some kind of failure
                CompleteInit(cmdStatus);
            }
        }
        return;
    }
    // Handle Autopause
    if (iAutoPaused)
    {
        if (aResponse.GetCmdStatus() == PVMFSuccess)
        {
            if (iTrack.iState == PVMP3FFNodeTrackPortInfo::TRACKSTATE_DOWNLOAD_AUTOPAUSE)
            {
                iTrack.iState = PVMP3FFNodeTrackPortInfo::TRACKSTATE_TRANSMITTING_GETDATA;
            }

            iAutoPaused = false;
            // Node was in auto pause state, since data
            // availability is signalled reschedule now
            Reschedule();
        }
        else
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFMP3FFParserNode::DataStreamReadCapacityNotificationCallBack() Reporting failure"));
            ReportErrorEvent(PVMFErrResource, NULL);
        }
    }
    else
    {
        // unrecognized callback
        OSCL_ASSERT(false);
    }
}

/*
 * From PvmiDataStreamObserver: callback for info event from DataStream
 */
void PVMFMP3FFParserNode::DataStreamInformationalEvent(const PVMFAsyncEvent& aEvent)
{
    //if Datadownload is complete then send PVMFInfoBufferingComplete event from DS to parser node
    if (aEvent.GetEventType() == PVMFInfoBufferingComplete)
    {
        iDownloadComplete = true;
    }
}

/**
 * From PvmiDataStreamObserver: callback for error event from DataStream
 */
void PVMFMP3FFParserNode::DataStreamErrorEvent(const PVMFAsyncEvent& aEvent)
{
    OSCL_UNUSED_ARG(aEvent);
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFMP3FFParserNode::DataStreamErrorEvent() \
		Doing an assert here. Unexpected error callback from DataStream"));
    //Should never be called
    OSCL_ASSERT(false);
}

PVMFStatus PVMFMP3FFParserNode::CheckForMP3HeaderAvailability()
{

    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode::CheckForMP3HeaderAvailability In"));
    if (iMP3File == NULL)
    {
        PVMFStatus retval = SetupParserObject();
        if (retval != PVMFSuccess)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFMP3FFParserNode::CheckForMP3HeaderAvailability setup parser object failed."));
            return retval;
        }
    }

    if (iDataStreamInterface != NULL)
    {
        uint32 minBytesRequired = 0;
        minBytesRequired = iMP3File->GetMinBytesRequired();

        /*
         * First check if we have minimum number of bytes to recognize
         * the file and determine the header size.
         */
        uint32 currCapacity = 0;
        PvmiDataStreamStatus status = iDataStreamInterface->QueryReadCapacity(iDataStreamSessionID,
                                      currCapacity);

        if ((PVDS_SUCCESS == status) && (currCapacity <  minBytesRequired))
        {
            iRequestReadCapacityNotificationID =
                iDataStreamInterface->RequestReadCapacityNotification(iDataStreamSessionID,
                        *this,
                        minBytesRequired);
            return PVMFPending;
        }

        MP3ErrorType retCode = MP3_ERROR_UNKNOWN;
        if (iMP3File)
        {

            if (iSourceFormat != PVMF_MIME_DATA_SOURCE_SHOUTCAST_URL)
            {
                retCode = iMP3File->GetMetadataSize(iMP3MetaDataSize);
                if (retCode == MP3_SUCCESS)
                {
                    /* Fetch the id3 tag size, if any and make it persistent in cache*/
                    iDataStreamInterface->MakePersistent(0, iMP3MetaDataSize);
                    if (currCapacity < iMP3MetaDataSize)
                    {
                        iRequestReadCapacityNotificationID =
                            iDataStreamInterface->RequestReadCapacityNotification(iDataStreamSessionID,
                                    *this,
                                    iMP3MetaDataSize + minBytesRequired);
                        return PVMFPending;
                    }
                }
                else
                {
                    iDataStreamInterface->MakePersistent(0, 0);
                }
            }
            else
            {
                iDataStreamInterface->MakePersistent(0, 0);
            }
        }
    }
    PVMFStatus status = ParseFile();
    return status;
}

void PVMFMP3FFParserNode::GetCPMMetaDataKeys()
{
    if (iCPMContainer.iCPMMetaDataExtensionInterface != NULL)
    {
        iCPMMetadataKeys.clear();
        iCPMGetMetaDataKeysCmdId =
            iCPMContainer.iCPMMetaDataExtensionInterface->GetNodeMetadataKeys(iCPMContainer.iSessionId,
                    iCPMMetadataKeys,
                    0,
                    PVMF_MP3_PARSER_NODE_MAX_CPM_METADATA_KEYS);
    }
}

bool PVMFCPMContainerMp3::GetCPMMetaDataExtensionInterface()
{
    PVInterface* temp = NULL;
    bool retVal =
        iCPM->queryInterface(KPVMFMetadataExtensionUuid, temp);
    iCPMMetaDataExtensionInterface = OSCL_STATIC_CAST(PVMFMetadataExtensionInterface*, temp);
    return retVal;
}
PVMp3DurationCalculator::PVMp3DurationCalculator(int32 aPriority, IMpeg3File* aMP3File, PVMFNodeInterface* aNode, bool aScanEnabled):
        OsclTimerObject(aPriority, "PVMp3DurationCalculator"), iNode(aNode)
{
    iErrorCode = MP3_SUCCESS;
    iMP3File = aMP3File;
    iScanComplete = false;
    iScanEnabled = aScanEnabled;
    if (!IsAdded())
    {
        AddToScheduler();
    }
}

PVMp3DurationCalculator::~PVMp3DurationCalculator()
{
    if (IsAdded())
    {
        RemoveFromScheduler();
    }
}

void PVMp3DurationCalculator::ScheduleAO()
{
    totalticks = 0;
    if (iScanEnabled && iMP3File)
    {
        RunIfNotReady();
    }
}

void PVMp3DurationCalculator::Run()
{
    // dont do the duration calculation scan in case of PS/PD or the file pointer is NULL
    if (((PVMFMP3FFParserNode*)iNode)->iDownloadProgressInterface || iMP3File == NULL)
    {
        return;
    }

    if (iErrorCode == MP3_DURATION_PRESENT)
    {
        // A valid duration is already present no need to scan the file, just send the duration event and return
        iScanComplete = true;
        int32 durationInMsec = iMP3File->GetDuration();
        int32 leavecode = 0;
        PVMFDurationInfoMessage* eventmsg = NULL;
        OSCL_TRY(leavecode, eventmsg = OSCL_NEW(PVMFDurationInfoMessage, (durationInMsec)));
        ((PVMFMP3FFParserNode*)iNode)->ReportInfoEvent(PVMFInfoDurationAvailable, NULL, OSCL_STATIC_CAST(PVInterface*, eventmsg));
        if (eventmsg)
        {
            eventmsg->removeRef();
        }
        return;
    }
    if (iErrorCode != MP3_SUCCESS)
    {
        iScanComplete = true;
        int32 durationInMsec = iMP3File->GetDuration();
        int32 leavecode = 0;
        PVMFDurationInfoMessage* eventmsg = NULL;
        OSCL_TRY(leavecode, eventmsg = OSCL_NEW(PVMFDurationInfoMessage, (durationInMsec)));
        ((PVMFMP3FFParserNode*)iNode)->ReportInfoEvent(PVMFInfoDurationAvailable, NULL, OSCL_STATIC_CAST(PVInterface*, eventmsg));
        if (eventmsg)
        {
            eventmsg->removeRef();
        }
        return;
    }
    else if (!iScanComplete)
    {
        RunIfNotReady(PVMF3FF_DURATION_SCAN_AO_DELAY);
    }
    else
    {
        return;
    }

    if (!(((PVMFMP3FFParserNode*)iNode)->iTrack.iSendBOS))
    {
        // Start the scan only when we have send first audio sample
        iErrorCode = iMP3File->ScanMP3File(PVMF3FF_DEFAULT_NUM_OF_FRAMES * 5);
    }
}

PVMFStatus PVMFMP3FFParserNode::CreateMP3FileObject(MP3ErrorType &aSuccess, PVMFCPMPluginAccessInterfaceFactory*aCPM)
{
    int32 leavecode = 0;
    OSCL_TRY(leavecode, iMP3File = OSCL_NEW(IMpeg3File,
                                            (iSourceURL, aSuccess,
                                             &iFileServer, aCPM,
                                             iFileHandle, false)));
    OSCL_FIRST_CATCH_ANY(leavecode, return PVMFErrNoMemory);
    return PVMFSuccess;
}

PVMFStatus PVMFMP3FFParserNode::PushBackCPMMetadataKeys(PVMFMetadataList *&aKeyListPtr, uint32 aLcv)
{
    int32 leavecode = 0;
    OSCL_TRY(leavecode, aKeyListPtr->push_back(iCPMMetadataKeys[aLcv]));
    OSCL_FIRST_CATCH_ANY(leavecode, PVLOGGER_LOGMSG(PVLOGMSG_INST_HLDBG, iLogger, PVLOGMSG_ERR, (0, "PVMFMP3FFParserNode::CompleteGetMetadataKeys() Memory allocation failure when copying metadata key")); return PVMFErrNoMemory);
    return PVMFSuccess;
}

#if PV_HAS_SHOUTCAST_SUPPORT_ENABLED
void PVMFMP3FFParserNode::MetadataUpdated(uint32 aMetadataSize)
{
    if (!iMetadataVector.empty())
    {
        iMetadataVector.clear();
    }

    iMetadataSize = aMetadataSize;
    int32 leavecode = OsclErrNone;
    PVMFMetadataInfoMessage* eventMsg = NULL;
    // parse the metadata to a kvp vector
    ParseShoutcastMetadata((char*) iMetadataBuf, iMetadataSize, iMetadataVector);
    // create a info msg
    OSCL_TRY(leavecode, eventMsg = OSCL_NEW(PVMFMetadataInfoMessage, (iMetadataVector)));
    // report the info msg to observer
    ReportInfoEvent(PVMFInfoMetadataAvailable, NULL, OSCL_STATIC_CAST(PVInterface*, eventMsg));

    uint32 i = 0;
    //cleanup the metadata vector
    while (i < iMetadataVector.size())
    {
        PvmiKvp kvp = iMetadataVector[i];
        if (kvp.key)
        {
            OSCL_ARRAY_DELETE(kvp.key);
            kvp.key = NULL;
        }
        if (kvp.value.pChar_value)
        {
            OSCL_ARRAY_DELETE(kvp.value.pChar_value);
            kvp.value.pChar_value = NULL;
        }
        i++;
    }

    while (!iMetadataVector.empty())
    {
        iMetadataVector.erase(iMetadataVector.begin());
    }
    if (eventMsg)
    {
        eventMsg->removeRef();
    }
}

PVMFStatus PVMFMP3FFParserNode::ParseShoutcastMetadata(char* aMetadataBuf, uint32 aMetadataSize, Oscl_Vector<PvmiKvp, OsclMemAllocator>& aKvpVector)
{
    // parse shoutcast metadata
    char* metadataPtr = NULL;
    metadataPtr = (char*)oscl_malloc(aMetadataSize);
    oscl_strncpy(metadataPtr, aMetadataBuf, aMetadataSize);

    char* bufPtr = metadataPtr;

    PvmiKvp kvp;

    char* key = NULL;
    char* valueStr = NULL;
    while (true)
    {
        key = bufPtr;
        char* tmpPtr = oscl_strchr(bufPtr, '=');
        if (NULL == tmpPtr)
        {
            break;
        }
        *tmpPtr = '\0';
        valueStr = tmpPtr + 2; // skip 2 bytes. '=' & '''
        tmpPtr = oscl_strchr(valueStr, ';');
        if (NULL == tmpPtr)
        {
            break;
        }
        *(tmpPtr - 1) = '\0'; // remove '''
        *tmpPtr = '\0';

        bufPtr = tmpPtr + 1;

        // make kvp from key & valueStr info
        OSCL_StackString<128> keyStr;
        keyStr = _STRLIT_CHAR("");

        if (!(oscl_strncmp(key, "StreamTitle", oscl_strlen("StreamTitle"))))
        {
            keyStr += _STRLIT_CHAR(KVP_KEY_TITLE);
            keyStr += SEMI_COLON;
            keyStr += _STRLIT_CHAR(KVP_VALTYPE_ISO88591_CHAR);
        }
        else if (!(oscl_strncmp(key, "StreamUrl", oscl_strlen("StreamUrl"))))
        {
            keyStr += _STRLIT_CHAR(KVP_KEY_DESCRIPTION);
            keyStr += SEMI_COLON;
            keyStr += _STRLIT_CHAR(KVP_VALTYPE_ISO88591_CHAR);
        }
        else
        {
            //not supported
        }

        keyStr += NULL_CHARACTER;

        int32 keylen = oscl_strlen(keyStr.get_cstr());
        int32 valuelen = oscl_strlen(valueStr);

        kvp.key = OSCL_ARRAY_NEW(char, (keylen + 1));
        kvp.value.pChar_value = OSCL_ARRAY_NEW(char, (valuelen + 1));

        oscl_strncpy(kvp.key, keyStr.get_cstr(), keylen + 1);
        oscl_strncpy(kvp.value.pChar_value, valueStr, valuelen + 1);

        aKvpVector.push_back(kvp);
    }

    if (metadataPtr)
    {
        oscl_free(metadataPtr);
        metadataPtr = NULL;
    }
    return PVMFSuccess;
}
#endif //PV_HAS_SHOUTCAST_SUPPORT_ENABLED

PVMFStatus PVMFMP3FFParserNode::SetupParserObject()
{
    PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                    (0, "PVMFMP3FFParserNode:SetupParserObject() In"));

    if (iMP3File == NULL)
    {
        // Instantiate the IMpeg3File object, this class represents the mp3ff library
        MP3ErrorType bSuccess = MP3_SUCCESS;

        PVMFDataStreamFactory* dsFactory = iCPMContainer.iCPMContentAccessFactory;
        if ((dsFactory == NULL) && (iDataStreamFactory != NULL))
        {
#if PV_HAS_SHOUTCAST_SUPPORT_ENABLED
            if (iSourceFormat == PVMF_MIME_DATA_SOURCE_SHOUTCAST_URL && iSCSPFactory != NULL && iSCSP != NULL)
            {
                dsFactory = iSCSPFactory;
                iSCSP->RequestMetadataUpdates(iDataStreamSessionID, *this, iMetadataBufSize, iMetadataBuf);
            }
            else
            {
                dsFactory = iDataStreamFactory;
            }
#else
            dsFactory = iDataStreamFactory;
#endif
        }

        // Try block start
        PVMFStatus returncode = CreateMP3FileObject(bSuccess, dsFactory);
        // Try block end

        if ((returncode != PVMFSuccess) || !iMP3File || (bSuccess != MP3_SUCCESS))
        {
            // creation of IMpeg3File object failed or resulted in error
            SetState(EPVMFNodeError);
            return PVMFErrResource;
        }
    }

    // enable the duration scanner for local playback only
    if (!iDataStreamFactory && !iDurationCalcAO)
    {
        int32 leavecode = 0;
        OSCL_TRY(leavecode, iDurationCalcAO = OSCL_NEW(PVMp3DurationCalculator,
                                              (OsclActiveObject::EPriorityIdle, iMP3File, this)));
        if (leavecode)
        {
            PVLOGGER_LOGMSG(PVLOGMSG_INST_LLDBG, iLogger, PVLOGMSG_STACK_TRACE,
                            (0, "PVMFMP3FFParserNode::DoInit() Duration Scan is disabled. DurationCalcAO not created"));
        }

        if (iDurationCalcAO)
        {
            iDurationCalcAO->ScheduleAO();
        }
    }
    return PVMFSuccess;
}


