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
#ifndef PVRTSP_CLIENT_ENGINE_NODE_H
#define PVRTSP_CLIENT_ENGINE_NODE_H


#ifndef OSCL_BASE_H_INCLUDED
#include "oscl_base.h"
#endif

#ifndef OSCL_UTF8CONV_H
#include "oscl_utf8conv.h"
#endif

#ifndef OSCL_ERROR_CODES_H_INCLUDED
#include "oscl_error_codes.h"
#endif

#ifndef OSCL_SCHEDULER_AO_H_INCLUDED
#include "oscl_scheduler_ao.h"
#endif

#ifndef OSCL_SOCKET_TYPES_H_INCLUDED
#include "oscl_socket_types.h"
#endif

#ifndef OSCL_SOCKET_H_INCLUDED
#include "oscl_socket.h"
#endif

#ifndef OSCL_DNS_H_INCLUDED
#include "oscl_dns.h"
#endif

#ifndef OSCL_STRING_CONTAINERS_H_INCLUDED
#include "oscl_string_containers.h"
#endif

#ifndef PVMF_MEDIA_CLOCK_H_INCLUDED
#include "pvmf_media_clock.h"
#endif

#ifndef OSCL_TIMER_H_INCLUDED
#include "oscl_timer.h"
#endif

#ifndef PVMF_FORMAT_TYPE_H_INCLUDED
#include "pvmf_format_type.h"
#endif

#ifndef PVMF_NODE_INTERFACE_H_INCLUDED
#include "pvmf_node_interface.h"
#endif

#ifndef PVMF_NODE_UTILS_H_INCLUDED
#include "pvmf_node_utils.h"
#endif

#ifndef OSCL_PRIQUEUE_H_INCLUDED
#include "oscl_priqueue.h"
#endif

#ifndef PVMF_SIMPLE_MEDIA_BUFFER_H_INCLUDED
#include "pvmf_simple_media_buffer.h"
#endif

#ifndef OSCL_MEM_MEMPOOL_H_INCLUDED
#include "oscl_mem_mempool.h"
#endif

#ifndef PVLOGGER_H_INCLUDED
#include "pvlogger.h"
#endif

#ifndef PVRTSP_ENGINE_NODE_EXTENSION_INTERFACE_H_INCLUDED
#include "pvrtspenginenodeextensioninterface.h"
#endif

#ifndef RTSP_PAR_COM_MESSAGE_DS_H_
#include "rtsp_par_com_message.h"
#endif

#ifndef RTSP_PARSER_H_
#include "rtsp_parser.h"
#endif

#ifndef SDP_PARSER_H
#include "sdp_parser.h"
#endif

#ifndef PVMF_RTSP_PORT_H_INCLUDED
#include "pvrtsp_client_engine_port.h"
#endif

#ifndef OSCL_FILEIO_H_INCLUDED
#include "oscl_file_io.h"
#endif

#ifndef PVMF_STREAMING_REAL_INTERFACES_INCLUDED
#include "pvmf_streaming_real_interfaces.h"
#endif

#ifndef PAYLOAD_PARSER_H_INCLUDED
#include "payload_parser.h"
#endif

//Default vector reserve size
#define PVMF_RTSP_ENGINE_NODE_COMMAND_VECTOR_RESERVE 10

//Starting value for command IDs
#define PVMF_RTSP_ENGINE_NODE_COMMAND_ID_START 6000

#define MAX_TOTAL_TRACKS 4 /* maximum number of channels per session */

//The RTSP state enumeration
typedef enum
{
    STATE_INIT,
    STATE_READY,
    STATE_PLAYING,
} SessionState;

typedef struct _DataChannelInfo
{
    uint8 *hexSID;

    bool   ssrc_is_set;
    uint32 ssrc;

    bool   seqbase_is_set;
    uint32 seq;

    bool   rtptime_is_set;
    uint32 rtptime;

    int32  offset;
    uint32 timescale;
    uint32 rtcp_interval;
} DataChannelInfo;

//memory allocator type for this node.
typedef OsclMemAllocator PVRTSPEngineNodeAllocator;

class SessionInfo
{
    public:

        OSCL_HeapString<PVRTSPEngineNodeAllocator> iSessionURL;
        OSCL_HeapString<PVRTSPEngineNodeAllocator> iContentBaseURL;// Per session control URL from content base field

        OSCL_HeapString<PVRTSPEngineNodeAllocator> iServerName;    // could be either DNS or ip address

        OSCL_HeapString<PVRTSPEngineNodeAllocator> iProxyName;     // could be either DNS or ip address
        OSCL_HeapString<PVRTSPEngineNodeAllocator> iUuid;

        uint32  iProxyPort;

        OsclNetworkAddress iSrvAdd;

        bool bExternalSDP;          //true if got SDP through external(NOT DESCRIBE) methods

        RtspRangeType   iReqPlayRange;
        RtspRangeType   iActPlayRange;

        OsclRefCounterMemFrag   pSDPBuf;

        OsclSharedPtr<SDPInfo>  iSDPinfo;
        Oscl_Vector<StreamInfo, PVRTSPEngineNodeAllocator> iSelectedStream;

        int number_of_channels;                 // Used for interaction with the media buffer class
        int channel_tbl[MAX_TOTAL_TRACKS];      // Index from SDP track IDs to tracks actually selected

        OSCL_HeapString<PVRTSPEngineNodeAllocator> iSID;    // alphanumeric session ID

        bool serverReplyFlag;           // Used for communication between the PVStream modules (network module and the task scheduler)
        bool getStateFlag;              // Used to ensure that the PE's main loop has been called
        bool pvServerIsSetFlag;         // Used to indicate if we are streaming from PVServer or not
        bool tSIDIsSetFlag;             // Used to send session id from second SETUP request
        uint32 iServerVersionNumber;    // Version number of PVSS
        int32 prerollDuration;          // Saves the jitter buffer size

        int32 fwp_counter;              // Id of the first returned firewall packet
        int32 rtt;                      // Saves the round trip time for firewall exchange
        uint32 roundTripDelay;          // Saves the round trip delay for a DESCRIBE request
        uint64 clientServerDelay;       // Saves the client server delay during a DESCRIBE request

        bool pipeLineFlag;              // Used to indicate a pipe lined request

        OSCL_HeapString<PVRTSPEngineNodeAllocator>  iUserAgent;
        OSCL_HeapString<PVRTSPEngineNodeAllocator>  iUserNetwork;
        OSCL_HeapString<PVRTSPEngineNodeAllocator>  iDeviceInfo;

        OSCL_HeapString<PVRTSPEngineNodeAllocator>  iUserID;
        OSCL_HeapString<PVRTSPEngineNodeAllocator>  iAuthentication;
        OSCL_HeapString<PVRTSPEngineNodeAllocator>  iExpiration;
        OSCL_HeapString<PVRTSPEngineNodeAllocator>  iApplicationSpecificString;
        OSCL_HeapString<PVRTSPEngineNodeAllocator>  iVerification;
        OSCL_HeapString<PVRTSPEngineNodeAllocator>  iSignature;

        enum PVRTSPStreamingType iStreamingType;

        RtspRangeType                              iPlaylistRange;
        PVEffectiveTime                            iPlaylistPlayTime;
        OSCL_HeapString<PVRTSPEngineNodeAllocator> iPlaylistQueryString;    // Playlist query string
        bool                                       iPlaylistQueryStringFlag;// Flag to indicate query string's presence
        OSCL_HeapString<PVRTSPEngineNodeAllocator> iPlaylistUri;            // Playlist uri
        bool                                       iPlaylistUriFlag;        // Flag to indicate playlist uri's presence
        OsclSharedPtr<SDPInfo>                     iSwitchSDPInfo;          // Saves switch SDP info
        Oscl_Vector<StreamInfo, OsclMemAllocator>  iSwitchSelectedStream;   // Saves selected track media information
        bool                                       iSwitchSDPAvailable;     // Flag to indicate if SDP is available or not
        bool                                       iSessionCompleted;
        void UpdateSessionCompletionStatus(bool aSessionCompleted)
        {
            iSessionCompleted = aSessionCompleted;
        }

    public:
        SessionInfo():
                iProxyPort(0),
                bExternalSDP(false),
                pvServerIsSetFlag(false),
                iServerVersionNumber(0),
                roundTripDelay(0),
                iPlaylistQueryStringFlag(false),
                iPlaylistUriFlag(false),
                iSessionCompleted(false)
        {
            iUserAgent += _STRLIT_CHAR("PVPlayer ");
            iReqPlayRange.format = RtspRangeType::INVALID_RANGE;
            iPlaylistRange.iPlaylistUrl[0] = '\0';
        };

} ;

class RTSPNodeMemDestructDealloc : public OsclDestructDealloc
{
    public:
        virtual void destruct_and_dealloc(OsclAny *ptr)
        {
            OSCL_FREE(ptr);
        }
};

typedef PVMFGenericNodeCommand<PVRTSPEngineNodeAllocator> PVRTSPEngineCommandBase;

enum TPVMFRtspNodeCommand //TPVMFGenericNodeCommand
{
    PVMF_RTSP_NODE_ERROR_RECOVERY = PVMF_GENERIC_NODE_COMMAND_LAST + 1
    , PVMF_RTSP_NODE_PLAYLIST_PLAY
    , PVMF_RTSP_NODE_CANCELALLRESET
};
class PVRTSPEngineCommand: public PVRTSPEngineCommandBase
{
    public:

        void Construct(PVMFSessionId s, int32 cmd, int32 arg1, int32 arg2, int32& arg3, \
                       const OsclAny*aContext)
        {
            PVRTSPEngineCommandBase::Construct(s, cmd, aContext);
            iParam1 = (OsclAny*)arg1;
            iParam2 = (OsclAny*)arg2;
            iParam3 = (OsclAny*) & arg3;
        }
        void Parse(int32&arg1, int32&arg2, int32*&arg3)
        {
            arg1 = (int32)iParam1;
            arg2 = (int32)iParam2;
            arg3 = (int32*)iParam3;
        }
        virtual bool hipri()
        {
            if (iCmd == PVMF_RTSP_NODE_ERROR_RECOVERY)
                return true;

            return PVRTSPEngineCommandBase::hipri();
        }
};

//Command queue type
typedef PVMFNodeCommandQueue<PVRTSPEngineCommand, PVRTSPEngineNodeAllocator> PVRTSPEngineNodeCmdQ;

class PVRTSPGenericMessageCompareLess
{
    public:
        /**
        * The algorithm used in OsclPriorityQueue needs a compare function
        * that returns true when A's priority is less than B's
        * @return true if A's priority is less than B's, else false
            */
        int compare(RTSPGenericMessage* a, RTSPGenericMessage* b) const
        {
            return (PVRTSPGenericMessageCompareLess::GetPriority(*a) > PVRTSPGenericMessageCompareLess::GetPriority(*b));
        }

        /**
        * Returns the priority of each command
        * @return A 0-based priority number. A lower number indicates lower priority.
        */
        static int GetPriority(RTSPGenericMessage &aCmd)
        {//for cseq numbers, the low numbers come first. so higher priority
            return aCmd.cseq;
        }
};


class PVRTSPEngineNode
        : public PVInterface,
        public PVMFNodeInterface,
        public OsclTimerObject,
        public OsclSocketObserver,
        public OsclDNSObserver,
        public OsclTimerObserver,
        public OsclMemPoolFixedChunkAllocatorObserver
{//note: this class is for internal use only, but must be public to avoid ADS v1.2 compile error.
    public:
        //To keep track of current socket op
        class SocketState
        {
            public:
                SocketState()
                        : iPending(false)
                        , iCanceled(false)
                {}
                bool iPending;
                bool iCanceled;
                void Reset()
                {
                    iPending = iCanceled = false;
                }
        };


    protected:
        class SocketContainer
        {
            public:
                SocketContainer(): iSocket(NULL)
                {}
                OsclTCPSocket* iSocket;
                SocketState iConnectState;
                SocketState iSendState;
                SocketState iRecvState;
                SocketState iShutdownState;
                void Reset(OsclTCPSocket* aSock)
                {
                    iSocket = aSock;
                    iConnectState.Reset();
                    iSendState.Reset();
                    iRecvState.Reset();
                    iShutdownState.Reset();
                }
                bool IsBusy()
                {
                    return iSocket
                           && (iConnectState.iPending
                               || iSendState.iPending
                               || iRecvState.iPending
                               || iShutdownState.iPending);
                }
        };
        OSCL_IMPORT_REF virtual PVMFStatus DoInitNode(PVRTSPEngineCommand &aCmd);
        OSCL_IMPORT_REF virtual PVMFStatus SendRtspDescribe(PVRTSPEngineCommand &aCmd);
        OSCL_IMPORT_REF virtual PVMFStatus SendRtspSetup(PVRTSPEngineCommand &aCmd);
        OSCL_IMPORT_REF PVMFStatus composeAndSendSetupRequest();
        OSCL_IMPORT_REF PVMFStatus processSetupResponse();
        OSCL_IMPORT_REF PVMFStatus processOtherPossibleSetupStates();
        OSCL_IMPORT_REF virtual PVMFStatus DoResetNode(PVRTSPEngineCommand &aCmd);
        OSCL_IMPORT_REF PVMFStatus SendRtspTeardown(PVRTSPEngineCommand &aCmd);
        OSCL_IMPORT_REF virtual bool parseURL(const char* aURL);
        OSCL_IMPORT_REF PVMFStatus processIncomingMessage(RTSPIncomingMessage &iIncomingMsg);
        OSCL_IMPORT_REF PVMFStatus populatePlayRequestFields(RTSPOutgoingMessage &aMsg);
        OSCL_IMPORT_REF virtual PVMFStatus composeSetupRequest(RTSPOutgoingMessage &iMsg, StreamInfo &aSelected);
        OSCL_IMPORT_REF PVMFStatus populateCommonSetupFields(RTSPOutgoingMessage &iMsg, StreamInfo &aSelected);
        OSCL_IMPORT_REF PVMFStatus composeSetupMessage(RTSPOutgoingMessage &iMsg);
        OSCL_IMPORT_REF virtual PVMFStatus sendSocketOutgoingMsg(SocketContainer &aSock, RTSPOutgoingMessage &aMsg);
        OSCL_IMPORT_REF virtual PVMFStatus DoQueryInterface(PVRTSPEngineCommand &aCmd);
        OSCL_IMPORT_REF PVMFStatus waitForConnectComplete();
        OSCL_IMPORT_REF PVMFStatus checkForSDPAvailability();
        OSCL_IMPORT_REF PVMFStatus processOptionsResponse();
        OSCL_IMPORT_REF PVMFStatus composeAndSendDescribeRequest();
        OSCL_IMPORT_REF void SetSendPending(SocketContainer&);
        OSCL_IMPORT_REF void SetRecvPending(SocketContainer&);
        OSCL_IMPORT_REF bool clearEventQueue(void);
        OSCL_IMPORT_REF virtual PVMFStatus sendSocketOutgoingMsg(SocketContainer &aSock, const uint8* aSendBuf, uint32 aSendLen);
        OSCL_IMPORT_REF void ReportErrorEvent(PVMFEventType aEventType,
                                              OsclAny* aEventData = NULL,
                                              PVUuid* aEventUUID = NULL,
                                              int32* aEventCode = NULL);
        OSCL_IMPORT_REF virtual bool DispatchEmbeddedData(uint32 aChannelID);
        OSCL_IMPORT_REF PVMFStatus resetSocket(bool aImmediate = false);
        OSCL_IMPORT_REF virtual PVMFStatus processEntityBody(RTSPIncomingMessage &aMsg, OsclMemoryFragment &aEntityMemFrag);
        OSCL_IMPORT_REF virtual bool supportedProtocolAndProfileCheck();
        SessionInfo iSessionInfo;
        /* Round trip delay calculation */
        PVMFTimebase_Tickcount iRoundTripClockTimeBase;
        int32 iCurrentErrorCode;
        //only for http cloaking, string to store the text to send until send completes
        OSCL_HeapString<PVRTSPEngineNodeAllocator> iRecvChannelMsg, iSendChannelMsg;
        PVLogger* iLogger;
        uint32  iNumHostCallback, iNumConnectCallback, iNumSendCallback, iNumRecvCallback;
        const int TIMEOUT_CONNECT_AND_DNS_LOOKUP, TIMEOUT_SEND, TIMEOUT_RECV;
        int REQ_TIMER_WATCHDOG_ID, REQ_TIMER_KEEPALIVE_ID;
        int REQ_DNS_LOOKUP_ID;
        OsclTimer<PVRTSPEngineNodeAllocator>    *iWatchdogTimer;
        OsclPriorityQueue<RTSPOutgoingMessage*, PVRTSPEngineNodeAllocator, Oscl_Vector<RTSPOutgoingMessage*, PVRTSPEngineNodeAllocator>, PVRTSPGenericMessageCompareLess> iOutgoingMsgQueue;
        RTSPIncomingMessage iIncomingMsg;
        SocketContainer iSendSocket, iRecvSocket;
        RTSPParser::ParserState iRTSPParserState;
        static const int REQ_SEND_SOCKET_ID;
        static const int REQ_RECV_SOCKET_ID;
    protected:
        int TIMEOUT_WATCHDOG;
        PVMFMediaFragGroupCombinedAlloc<OsclMemAllocator>* ipFragGroupAllocator;
        // allocator for outgoing media frag groups
        OsclMemPoolFixedChunkAllocator* ipFragGroupMemPool;
        bool ibBlockedOnFragGroups;
        PVMFPortVector<PVMFRTSPPort, PVRTSPEngineNodeAllocator> iPortVector;
        PVMFPortInterface* iTheBusyPort;
        int setupTrackIndex;
        bool bNoSendPending;//a Send() is pending on RTSP socket
    public:
        OSCL_IMPORT_REF PVRTSPEngineNode(int32 aPriority);
        OSCL_IMPORT_REF virtual ~PVRTSPEngineNode();

        //************ begin PVMFNodeInterface

        OSCL_IMPORT_REF virtual  PVMFStatus ThreadLogon();
        OSCL_IMPORT_REF virtual  PVMFStatus ThreadLogoff();

        /**
        GetCapability can be invoked only when after a node is initialized
        **/
        OSCL_IMPORT_REF virtual  PVMFStatus GetCapability(PVMFNodeCapability& aNodeCapability);

        /**
         * Returns a list of ports currently available in the node that meet the filter criteria
         * We can add fancier iterators and filters as needed.
         * For now we return all the available ports.  If no ports are present, NULL is returned
         **/
        OSCL_IMPORT_REF virtual  PVMFPortIter* GetPorts(const PVMFPortFilter* aFilter = NULL);

        /**
         * This API is to allow for extensibility of the PVMF Node interface.
         * It allows a caller to ask for all UUIDs associated with a particular MIME type.
         * If interfaces of the requested MIME type are found within the system, they are added
         * to the UUIDs array.
         *
         * Also added to the UUIDs array will be all interfaces which have the requested MIME
         * type as a base MIME type.  This functionality can be turned off.
         *
         * @param aMimeType The MIME type of the desired interfaces
         * @param aUuids A vector to hold the discovered UUIDs
         * @param aExactUuidsOnly Turns on/off the retrival of UUIDs with aMimeType as a base type
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         */
        OSCL_IMPORT_REF virtual PVMFCommandId QueryUUID(PVMFSessionId aSession
                , const PvmfMimeString& aMimeType
                , Oscl_Vector<PVUuid, PVRTSPEngineNodeAllocator>& aUuids
                , bool aExactUuidsOnly = false
                                         , const OsclAny* aContext = NULL);

        /**
         * This API is to allow for extensibility of the PVMF Node interface.
         * It allows a caller to ask for an instance of a particular interface object to be returned.
         * The mechanism is analogous to the COM IUnknown method.  The interfaces are identified with
         * an interface ID that is a UUID as in DCE and a pointer to the interface object is
         * returned if it is supported.  Otherwise the returned pointer is NULL.
         *
         * @param aUuid The UUID of the desired interface
         * @param aInterfacePtr The output pointer to the desired interface
         * @param aContext Optional opaque data to be passed back to user with the command response
         * @returns A unique command id for asynchronous completion
         */
        OSCL_IMPORT_REF virtual PVMFCommandId QueryInterface(PVMFSessionId aSession
                , const PVUuid& aUuid
                , PVInterface*& aInterfacePtr
                , const OsclAny* aContext = NULL);

        /**
         * Requests the node to return a port meeting certain criteria for format types and buffering
         * capabilities.  The node may return a reference to an already created unused port or it may dynamically
         * create one if it has the capability to do so.  Since there might be some port specific initializations
         * that might need to be done for ports created on demand, it will be most flexible to have this as an
         * asynchronous API.
         * A reference to the port interface is returned with the the command completion.  It is passed as an auto ptr
         * carrying opaque data that needs to be cast to PVMFPortInterface*
         * @exception PVMFErrNotSupported leaves if this is not supported.
         **/
        OSCL_IMPORT_REF virtual PVMFCommandId RequestPort(PVMFSessionId aSession
                , int32 aPortTag
                , const PvmfMimeString* aPortConfig = NULL
                                                      , const OsclAny* aContext = NULL);

        /**
         * Releases a port back to the owning node.
         * @exception PVMFErrArgument leaves if this node is not the owner.
         **/
        OSCL_IMPORT_REF virtual PVMFCommandId ReleasePort(PVMFSessionId aSession
                , PVMFPortInterface& aPort
                , const OsclAny* aContext = NULL);

        //Describe,
        OSCL_IMPORT_REF virtual PVMFCommandId Init(PVMFSessionId aSession
                , const OsclAny* aContext = NULL);

        OSCL_IMPORT_REF virtual PVMFCommandId Prepare(PVMFSessionId aSession,
                const OsclAny* aContext = NULL);

        //Setup, Firewall pkts exchange, Play
        /**
         * Causes the node to start servicing all connected ports.
         **/
        OSCL_IMPORT_REF virtual PVMFCommandId Start(PVMFSessionId aSession
                , const OsclAny* aContext = NULL);

        //Pause
        /**
         * Causes the node to pause servicing all connected ports without
         * discarding un-processed data.
         **/
        OSCL_IMPORT_REF virtual PVMFCommandId Pause(PVMFSessionId aSession
                , const OsclAny* aContext = NULL);

        //Teardown
        OSCL_IMPORT_REF virtual PVMFCommandId Stop(PVMFSessionId aSession
                , const OsclAny* aContext = NULL);

        /**
         * Resets the node.  The node should relinquish all resources that is has acquired as part of the
         * initialization process and should be ready to be deleted when this completes.
         **/
        OSCL_IMPORT_REF virtual PVMFCommandId Reset(PVMFSessionId aSession
                , const OsclAny* aContext = NULL);

        /**
         * Causes the node to stop servicing all connected ports as
         * soon as current data is processed.
         **/
        OSCL_IMPORT_REF virtual PVMFCommandId Flush(PVMFSessionId aSession
                , const OsclAny* aContext = NULL);


        /**
         * Cancel all pending requests. The current request being processed, if any, will also be aborted.
         *
         * @param aContextData Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         */
        OSCL_IMPORT_REF virtual PVMFCommandId CancelAllCommands(PVMFSessionId aSession
                , const OsclAny* aContextData = NULL);

        /**
         * Cancels pending command with the specified ID.
         *
         * @param aCmdId Command Id of the command to be cancelled
         * @param aContextData Optional opaque data that will be passed back to the user with the command response
         * @returns A unique command id for asynchronous completion
         */
        OSCL_IMPORT_REF virtual PVMFCommandId CancelCommand(PVMFSessionId aSession
                , PVMFCommandId aCmdId
                , const OsclAny* aContextData = NULL);

        /**
         * Ports call this API to report activity to the node.
         *
         * @param aActivity Information regarding the activity.
         */
        OSCL_IMPORT_REF void HandlePortActivity(const PVMFPortActivity& aActivity);

        //************ end PVMFNodeInterface

        //************ begin PVRTSPEngineNodeExtensionInterface
        OSCL_IMPORT_REF virtual void addRef();
        OSCL_IMPORT_REF virtual void removeRef(void);
        OSCL_IMPORT_REF virtual bool queryInterface(const PVUuid& uuid, PVInterface*& iface);

        OSCL_IMPORT_REF virtual PVMFStatus SetStreamingType(PVRTSPStreamingType aType = PVRTSP_3GPP_UDP);

        OSCL_IMPORT_REF virtual PVMFStatus SetSessionURL(OSCL_wString& aURL);

        OSCL_IMPORT_REF virtual PVMFStatus SetRtspProxy(OSCL_String& aRtspProxyName, uint32 aRtspProxyPort);
        OSCL_IMPORT_REF virtual PVMFStatus GetRtspProxy(OSCL_String& aRtspProxyName, uint32& aRtspProxyPort);

        OSCL_IMPORT_REF virtual PVMFStatus GetSDP(OsclRefCounterMemFrag& aSDPBuf);
        OSCL_IMPORT_REF virtual PVMFStatus SetSDPInfo(OsclSharedPtr<SDPInfo>& aSDPinfo, Oscl_Vector<StreamInfo, PVRTSPEngineNodeAllocator> &aSelectedStream);
        OSCL_IMPORT_REF virtual PVMFStatus GetServerInfo(PVRTSPEngineNodeServerInfo& aServerInfo);
        OSCL_IMPORT_REF virtual PVMFStatus GetStreamInfo(Oscl_Vector<StreamInfo, PVRTSPEngineNodeAllocator> &aSelectedStream);

        OSCL_IMPORT_REF virtual PVMFStatus GetUserAgent(OSCL_wString& aUserAgent);

        OSCL_IMPORT_REF virtual PVMFStatus SetClientParameters(OSCL_wString& aUserAgent,
                OSCL_wString&  aUserNetwork,
                OSCL_wString&  aDeviceInfo);


        OSCL_IMPORT_REF virtual PVMFStatus SetAuthenticationParameters(OSCL_wString& aUserID,
                OSCL_wString& aAuthentication,
                OSCL_wString& aExpiration,
                OSCL_wString& aApplicationSpecificString,
                OSCL_wString& aVerification,
                OSCL_wString& aSignature);

        OSCL_IMPORT_REF virtual PVMFStatus SetRequestPlayRange(const RtspRangeType& aRange);
        OSCL_IMPORT_REF virtual PVMFStatus GetActualPlayRange(RtspRangeType& aRange);

        OSCL_IMPORT_REF virtual PVMFStatus SetKeepAliveMethod_timeout(int32 aTimeout = 0);
        OSCL_IMPORT_REF virtual PVMFStatus SetKeepAliveMethod_use_SET_PARAMETER(bool aUseSetParameter = false);
        OSCL_IMPORT_REF virtual PVMFStatus SetKeepAliveMethod_keep_alive_in_play(bool aKeepAliveInPlay = false);

        OSCL_IMPORT_REF virtual PVMFStatus GetKeepAliveMethod(int32 &aTimeout, bool &aUseSetParameter, bool &aKeepAliveInPlay);

        OSCL_IMPORT_REF virtual PVMFCommandId PlaylistPlay(PVMFSessionId aSession, const RtspRangeType& aRange, PVEffectiveTime aEffectiveTime, const OsclAny* aContext = NULL);
        OSCL_IMPORT_REF virtual PVMFStatus SetPlaylistUri(const char* aPlaylistUri);
        OSCL_IMPORT_REF virtual void SetSwitchSDPInfo(OsclSharedPtr<SDPInfo>& aSDPinfo, Oscl_Vector<StreamInfo, OsclMemAllocator>& aTrackIdMapping, bool aSDPAvailable);
        OSCL_IMPORT_REF virtual PVMFStatus GetSwitchStreamInfo(Oscl_Vector<StreamInfo, PVRTSPEngineNodeAllocator> &aSelectedStream);

        OSCL_IMPORT_REF virtual PVMFStatus GetRTSPTimeOut(int32 &aTimeout);
        OSCL_IMPORT_REF virtual PVMFStatus SetRTSPTimeOut(int32 aTimeout);

        //************ end PVRTSPEngineNodeExtensionInterface

        //************ begin OsclTimerObserver
        OSCL_IMPORT_REF virtual void TimeoutOccurred(int32 timerID, int32 timeoutInfo);

        //************ begin OsclSocketObserver
        OSCL_IMPORT_REF virtual void HandleSocketEvent(int32 aId, TPVSocketFxn aFxn, TPVSocketEvent aEvent, int32 aError);
        //************ end OsclSocketObserver

        //************ begin OsclDNSObserver
        OSCL_IMPORT_REF virtual void HandleDNSEvent(int32 aId, TPVDNSFxn aFxn, TPVDNSEvent aEvent, int32 aError);
        //************ end OsclDNSObserver

        //************ begin OsclMemPoolFixedChunkAllocatorObserver
        OSCL_IMPORT_REF void freechunkavailable(OsclAny*);
        //************ end OsclMemPoolFixedChunkAllocatorObserver

        void UpdateSessionCompletionStatus(bool aSessionCompleted)
        {
            iSessionInfo.iSessionCompleted = aSessionCompleted;
        }

        bool IsSessionCompleted() const
        {
            return iSessionInfo.iSessionCompleted;
        }

        typedef struct _SocketEvent
        {
            int32           iSockId;
            TPVSocketFxn    iSockFxn;
            TPVSocketEvent  iSockEvent;
            int32           iSockError;
        } SocketEvent;

        enum PVRTSPEngineState
        {
            //for async request
            PVRTSP_ENGINE_NODE_STATE_IDLE,
            PVRTSP_ENGINE_NODE_STATE_DNS_RESOLVING,

            PVRTSP_ENGINE_NODE_STATE_CONNECT,
            PVRTSP_ENGINE_NODE_STATE_CONNECTING,

            PVRTSP_ENGINE_NODE_STATE_HTTP_CLOAKING_GET_URL,
            PVRTSP_ENGINE_NODE_STATE_HTTP_CLOAKING_SETUP,

            PVRTSP_ENGINE_NODE_STATE_SEND_OPTIONS,
            PVRTSP_ENGINE_NODE_STATE_SEND_DESCRIBE,
            PVRTSP_ENGINE_NODE_STATE_OPTIONS_WAITING,
            PVRTSP_ENGINE_NODE_STATE_DESCRIBE_WAITING,

            PVRTSP_ENGINE_NODE_STATE_DESCRIBE_DONE,
            PVRTSP_ENGINE_NODE_STATE_PROCESS_REST_SETUP,

            PVRTSP_ENGINE_NODE_STATE_SETUP_DONE,
            PVRTSP_ENGINE_NODE_STATE_WAIT_PLAY,

            PVRTSP_ENGINE_NODE_STATE_PLAY_DONE,
            PVRTSP_ENGINE_NODE_STATE_WAIT_PLAYLIST_PLAY,

            PVRTSP_ENGINE_NODE_STATE_WAIT_PAUSE,
            PVRTSP_ENGINE_NODE_STATE_PAUSE_DONE,

            PVRTSP_ENGINE_NODE_STATE_WAIT_STOP,

            PVRTSP_ENGINE_NODE_STATE_WAIT_CALLBACK,

            PVRTSP_ENGINE_NODE_STATE_INVALID
        } iState;
    private:
        Oscl_Vector<SocketEvent, PVRTSPEngineNodeAllocator> iSocketEventQueue;
        PVMFCommandId iCurrentCmdId;
    protected:
        OSCL_IMPORT_REF OsclSharedPtr<PVMFMediaDataImpl> AllocateMediaData(int32& errCode);
        OSCL_IMPORT_REF void ChangeInternalState(PVRTSPEngineState aNewState);
        PVRTSPEngineNodeAllocator iAlloc;
        PVMFSharedMediaDataPtr iEmbeddedDataPtr;
        OsclSocketServ  *iSockServ;
    private:

        //Container for a TCP socket that can connect, send, & recv.
        class DnsContainer
        {
            public:
                DnsContainer(): iDns(NULL)
                {}
                OsclDNS* iDns;
                SocketState iState;
                bool IsBusy()
                {
                    return (iDns
                            && iState.iPending);
                }
        };

        DnsContainer iDNS;

        //To keep track of socket reset sequence.
        enum TSocketCleanupState
        {
            ESocketCleanup_Idle
            , ESocketCleanup_CancelCurrentOp
            , ESocketCleanup_WaitOnCancel
            , ESocketCleanup_Shutdown
            , ESocketCleanup_WaitOnShutdown
            , ESocketCleanup_Delete
        };
        TSocketCleanupState iSocketCleanupState;


        RTSPParser *iRTSPParser;
    protected:
        uint32 iOutgoingSeq;
    private:

        bool bNoRecvPending;//an Recv() is pending on RTSP socket

        // Reference counter for extension
        uint32 iExtensionRefCount;
        uint32 iNumRedirectTrials;

        //socket server will callback even if Cancel() is called
        //most likely this is the case for OsclDNS as well
        //But this is NOT the case for OsclTimer
        int BASE_REQUEST_ID;
    protected:
        const int DEFAULT_RTSP_PORT, DEFAULT_HTTP_PORT;
    private:

        //these three are in milliseconds
        const int TIMEOUT_SHUTDOWN;

        //these two are in seconds
        const int TIMEOUT_WATCHDOG_TEARDOWN;
        int TIMEOUT_KEEPALIVE;
        const int RECOMMENDED_RTP_BLOCK_SIZE;

        bool bRepositioning;

        class PVRTSPErrorContext
        {
            public:
                SocketEvent     iErrSockEvent;
                PVRTSPEngineState   iErrState;
        };


    protected:
        //temp string compose buffer for internal use RTSP_MAX_FULL_REQUEST_SIZE
        OsclMemoryFragment  iRTSPEngTmpBuf;
        OsclMemoryFragment  iEntityMemFrag;
    private:
        // Queue of commands for cancel
        PVRTSPEngineNodeCmdQ iCancelCmdQueue;
        // Queue of commands user requested
        PVRTSPEngineNodeCmdQ iPendingCmdQueue;

        //Queue for cmds which are running.
        //normally this node will not start processing one command
        //until the prior one is finished.  However, a hi priority
        //command such as Cancel must be able to interrupt a command
        //in progress.
        PVRTSPEngineNodeCmdQ iRunningCmdQueue;
        /**
         * Queue holding port activity. Only incoming and outgoing msg activity are
         * put on the queue.  For each port, there should only be at most one activity
         * of each type on the queue.
         */
        Oscl_Vector<PVMFPortActivity, PVRTSPEngineNodeAllocator> iPortActivityQueue;
        PVMFNodeCapability iCapability;

        RTSPOutgoingMessage* iSrvResponse;
        bool    bSrvRespPending;


        PVUuid iEventUUID;

        bool    bKeepAliveInPlay;
        RTSPMethod  iKeepAliveMethod;

        bool    bAddXStrHeader;

        OsclMemPoolResizableAllocator *iMediaDataResizableAlloc;
        PVMFSimpleMediaBufferCombinedAlloc *iMediaDataImplAlloc;


        int32   iErrorRecoveryAttempt;

    private:
        PVRTSPEngineNode();
        PVRTSPEngineNode&  operator = (const PVRTSPEngineNode&);
        PVRTSPEngineNode(const PVRTSPEngineNode&);

        //OsclActiveObject
        OSCL_IMPORT_REF virtual void Run();
        OSCL_IMPORT_REF virtual OsclLeaveCode RunError(OsclLeaveCode aError);


        void ChangeExternalState(TPVMFNodeInterfaceState aNewState);

    protected:
        // Handle command and data events
        OSCL_IMPORT_REF PVMFCommandId AddCmdToQueue(PVRTSPEngineCommand& aCmd);
        OSCL_IMPORT_REF virtual PVMFStatus DispatchCommand(PVRTSPEngineCommand& aCmd);
        OSCL_IMPORT_REF PVMFStatus processDispatchCommand(PVRTSPEngineCommand& aCmd, PVMFStatus iRet, bool bRevertToPreviousStateImpossible, bool bErrorRecoveryImpossible);

    private:

        bool ProcessCommand(PVRTSPEngineCommand& aCmd);
        bool rtspParserLoop(void);
        bool parseURL(const OSCL_wString& aURL);

        PVMFStatus DoPrepareNode(PVRTSPEngineCommand &aCmd);
        PVMFStatus DoStartNode(PVRTSPEngineCommand &aCmd);
        PVMFStatus DoPauseNode(PVRTSPEngineCommand &aCmd);
        PVMFStatus DoStopNode(PVRTSPEngineCommand &aCmd);
        PVMFStatus DoQueryUuid(PVRTSPEngineCommand &aCmd);
        PVMFStatus DoCancelCommand(PVRTSPEngineCommand &aCmd);
        PVMFStatus DoCancelAllCommands(PVRTSPEngineCommand &aCmd);
        PVMFStatus DoFlush(PVRTSPEngineCommand &aCmd);

        PVMFStatus DoErrorRecovery(PVRTSPEngineCommand &aCmd);

        PVMFStatus DoRequestPort(PVRTSPEngineCommand &aCmd, PVMFRTSPPort* &aPort);
        PVMFStatus DoAddPort(int32 id, bool isMedia, int32 tag, PVMFRTSPPort* &aPort);
        PVMFStatus DoReleasePort(PVRTSPEngineCommand &aCmd);
        PVMFStatus DoPlaylistPlay(PVRTSPEngineCommand &aCmd);

        bool FlushPending();
        bool ProcessPortActivity();
        void QueuePortActivity(const PVMFPortActivity &aActivity);
        PVMFStatus ProcessOutgoingMsg(PVMFPortInterface* aPort);
        PVMFStatus SendRtspPlay(PVRTSPEngineCommand &aCmd);
        PVMFStatus SendRtspPause(PVRTSPEngineCommand &aCmd);
        /*
         * SendRtspPipelinedRequest implements pipelining of RTSP SETUP and PLAY requests. It is
         * a part of the 3GPP compliant fast content switch implementation.
         */
        PVMFStatus SendRtspPipelinedRequest(PVRTSPEngineCommand &aCmd);
        PVMFStatus SendRtspPlaylistPlay(PVRTSPEngineCommand &aCmd);

        void MoveCmdToCancelQueue(PVRTSPEngineCommand& aCmd);
        void CommandComplete(PVRTSPEngineNodeCmdQ&,
                             PVRTSPEngineCommand&,
                             PVMFStatus, OsclAny* aData = NULL,
                             PVUuid* aEventUUID = NULL,
                             int32* aEventCode = NULL);


        PVMFStatus composeOptionsRequest(RTSPOutgoingMessage&);
        PVMFStatus composeDescribeRequest(RTSPOutgoingMessage&);
        PVMFStatus compose3GPPPlaylistPlayRequest(RTSPOutgoingMessage &iMsg);
        PVMFStatus composeOldAndNewURLs(RTSPOutgoingMessage &iMsg);
        PVMFStatus composeSwitchMediaURL(int aTrackID, StrPtrLen &aMediaURI);

    protected:
        OSCL_IMPORT_REF virtual PVMFStatus composePlayRequest(RTSPOutgoingMessage &iMsg);
    private:
        PVMFStatus composeStopRequest(RTSPOutgoingMessage &iMsg);
        PVMFStatus composePauseRequest(RTSPOutgoingMessage &iMsg);
        PVMFStatus composeKeepAliveRequest(RTSPOutgoingMessage &aMsg);

        PVMFStatus processServerRequest(RTSPIncomingMessage &aMsg);

        PVMFStatus processCommonResponse(RTSPIncomingMessage &aMsg);

        //should merge togather and move to jitter buffer
        PVMFStatus composeSessionURL(RTSPOutgoingMessage &aMsg);
        PVMFStatus composeMediaURL(int aTrackID, StrPtrLen &aMediaURI);
    protected:
        OSCL_IMPORT_REF virtual PVMFStatus checkForAbsoluteMediaURL(const char *sdpMediaURL, StrPtrLen &aMediaURI);
        OSCL_IMPORT_REF virtual PVMFStatus composeAbsoluteMediaURL(const char *sdpMediaURL, StrPtrLen &aMediaURI);
        OSCL_IMPORT_REF void ReportInfoEvent(PVMFEventType aEventType,
                                             OsclAny* aEventData = NULL,
                                             PVUuid* aEventUUID = NULL,
                                             int32* aEventCode = NULL);
    private:
        PVMFStatus MapRTSPCodeToPVMFStatusCode(RTSPStatusCode aStatusCode);
        void MapRTSPCodeToEventCode(RTSPStatusCode aStatusCode,
                                    int32& aEventCode);
        PVMFStatus CancelCurrentOps();
        void CheckCancelAllSuccess();
        //allocate aReqBufSize memory for iEmbeddedData
        bool PrepareEmbeddedDataMemory(uint32 aReqBufSize, OsclMemoryFragment &);

        void clearOutgoingMsgQueue(void);
        void partialResetSessionInfo(void);


        void ResetSessionInfo(void);
        PVRTSPEngineNodeExtensionInterface* iExtensionInterface;
    protected:
        OSCL_HeapString<PVRTSPEngineNodeAllocator> iRtspPrefixedURL;
    private:

        RTSPEntityBody iEmbeddedData;
        Oscl_Vector <uint16, PVRTSPEngineNodeAllocator> pipelinedRequestResponse;
        uint8* iPipelinedMessage;
        bool iPipelining;
        bool i3gppSwitchSupported;
        bool i3gppSwitchWithoutSDPSupported;
        bool i3gppSwitchStreamSupported;
        int32 iStartupId;
        OSCL_HeapString<PVRTSPEngineNodeAllocator> iTrackURLArray[3];
        void ParseSupportedHeaderFor3GPPFCSFeatures(const StrPtrLen* serverSupportedFeatures);
        bool IsSubstring(StrPtrLen masterString, StrPtrLen searchString);


};

#endif //PVRTSP_CLIENT_ENGINE_NODE_H
