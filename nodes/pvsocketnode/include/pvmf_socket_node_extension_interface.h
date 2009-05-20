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
#ifndef PVMF_SOCKET_NODE_EXTENSION_INTERFACE_H_INCLUDED
#define PVMF_SOCKET_NODE_EXTENSION_INTERFACE_H_INCLUDED

#define PVMF_SOCKET_NODE_EXTENSION_INTERFACE_MIMETYPE "pvxxx/SocketNode/socketnodeextensioninterface"
#define PVMF_SOCKET_NODE_MIMETYPE "pvxxx/SocketNode"
#define PVMF_SOCKET_NODE_BASEMIMETYPE "pvxxx"

#define PVMF_SOCKET_NODE_EXTENSION_INTERFACE_UUID PVUuid(1,2,3,0xfe,0xcd,0xee,0x1f,0x00,0x11,0x22,0x33)

class PVMFSocketNode;

class PVMFSocketNodeExtensionInterface : public PVInterface
{
    public:
        virtual ~PVMFSocketNodeExtensionInterface() {}
        OSCL_IMPORT_REF virtual PVMFStatus AllocateConsecutivePorts(PvmfMimeString* aPortConfig,
                uint32& aLowerPortNum,
                uint32& aHigherPortNum, uint32& aStartPortNum) = 0;

        /**
        For TCP based data flow, data stream is read from the server in chunks.
        There are two channels of data flow, one for reciving the data stream streamed by the server.
        and another for the feedback.
        Chunk size for input port (for receiving datastream stream) and feedback channel can be configured separately,
        by specifying the port ptr.
        If the port ptr is specified to be NULL, then same tcp recv size is configured on all the channels.

        @aBufferSize[in] - Size of the tcp recv chunk
        @aPort[in] - Pointer to the port on which tcp recv size is to be configured.
        @return - PVMFErrArgument, if the size specified is not in the range (0, SNODE_DEFAULT_MAX_TCP_RECV_BUFFER_SIZE)
                                 , aPort is neither NULL nor a valid port ptr
                  PVMFFailure, if aPort is NULL and there is some problem in the configuration.
                  PVMFSuccess, if value is successfully configured
        */
        OSCL_IMPORT_REF virtual PVMFStatus SetMaxTCPRecvBufferSize(uint32 aBufferSize, PVMFPortInterface* aPort = NULL) = 0;

        /**
        For TCP based data flow, data stream is read from the server in chunks.
        There are two channels of data flow, one for reciving the data stream streamed by the server.
        and another for the feedback.
        Chunk size for input port (for receiving datastream stream) and feedback channel can be configured separately,
        by specifying the port ptr.
        When port ptr is specified to be NULL, then we expect only one channel of data flow (for reciving data from the server)

        @aSize[out] - Size of the tcp recv chunk
        @aPort[in] - Pointer to the port whose tcp recv size is to be retrieved.
        @return - PVMFErrArgument, aPort is neither NULL nor a valid port ptr
                  PVMFFailure, if aPort is NULL and there is some problem in the configuration (more than one chaneel of data flow)
                  PVMFSuccess, if value is successfully retrieved
        */
        OSCL_IMPORT_REF virtual PVMFStatus GetMaxTCPRecvBufferSize(uint32& aSize, PVMFPortInterface* aPort = NULL) const = 0;

        /**
        For TCP based data flow, data stream is read from the server in chunks.
        There are two channels of data flow, one for reciving the data stream streamed by the server.
        and another for the feedback.
        Data received from the server is persisted in the memory allocated from a pool of mem.
        The total mem available with the allocator used for allocating chunks for persisting server response is
        (tcp-recv buff size)* (expected no of chunks)
        Therefore, user can tune the no of chunks to tune the mem to be allocated to the allocator.
        @aCount[in] - No of chunks
        @aPort[in] - Pointer to the port on which noof chunks is configured.
        @return - PVMFErrArgument, if the size specified is not in the range (0, SNODE_DEFAULT_SOCKET_TCP_BUFFER_COUNT)
                                 , aPort is neither NULL nor a valid port ptr
                  PVMFFailure, if aPort is NULL and there is some problem in the configuration.
                  PVMFSuccess, if value is successfully configured
        */
        OSCL_IMPORT_REF virtual PVMFStatus SetMaxTCPRecvBufferCount(uint32 aCount, PVMFPortInterface* aPort = NULL) = 0;

        /**
        For TCP based data flow, data stream is read from the server in chunks.
        There are two channels of data flow, one for reciving the data stream streamed by the server.
        and another for the feedback.
        Data received from the server is persisted in the memory allocated from a pool of mem.
        The total mem available with the allocator used for allocating chunks for persisting server response is
        (tcp-recv buff size)* (expected no of chunks)
        Therefore, user can tune the no of chunks to tune the mem to be allocated to the allocator.
        This API helps retrieve no of the chunks configured for a specific port.
        If port is not specified, then the API expects only one channel of data flow (no feedback port)
        @aCount[out] - No of chunks
        @aPort[in] - Pointer to the port on which value of no of chunks configured is to be retrieved.
        @return - PVMFErrArgument, aPort is neither NULL nor a valid port ptr
                  PVMFFailure, if aPort is NULL and there is some problem in the configuration.
                  PVMFSuccess, if value is successfully retrieved
        */
        OSCL_IMPORT_REF virtual PVMFStatus GetMaxTCPRecvBufferCount(uint32& aCount, PVMFPortInterface* aPort = NULL) const = 0;

        OSCL_IMPORT_REF virtual OsclMemPoolResizableAllocator* CreateSharedBuffer(const PVMFPortInterface* aPort , uint32 aBufferSize, uint32 aExpectedNumberOfBlocksPerBuffer, uint32 aResizeSize, uint32 aMaxNumResizes) = 0;

};
#endif //PVMF_SOCKET_NODE_EXTENSION_INTERFACE_H_INCLUDED


