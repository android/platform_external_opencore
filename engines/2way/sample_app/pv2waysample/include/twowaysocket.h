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
#ifndef TWOWAYSOCKET_H
#define TWOWAYSOCKET_H
////////////////////////////////////////////////////////////////////////////
// This file includes the class definition determining TwoWaySocket
//
// This class encapsulates everything needed in a two way application for sockets.
//
////////////////////////////////////////////////////////////////////////////

#ifdef SUPPORT_ISDN
#include "pvmf_capi_isdn_node.h"
#endif

#ifndef PVMF_CLIENTSERVER_SOCKET_NODE_FACTORY_H_INCLUDED
#include "pvmf_clientserver_socket_factory.h"
#endif

#include "errno.h"
#define ENOTCONNECTED ENOTCONN

class TwoWaySocketObserver
{
    public:
        virtual void SocketConnected(PVMFNodeInterface* aCommServer) = 0;
        virtual void SocketDisconnected() = 0;
};

enum PVTerminalType
{
    EPVClient,
    EPVServer
};


class TwoWaySocket: public OsclSocketObserver
{
    public:

        TwoWaySocket(TwoWaySocketObserver *aObserver);
        ~TwoWaySocket();

        bool ConnectSocket(bool aIsServer, int aPort, char* aIpAddr = NULL);
        void DeleteCommServer();

        bool Connect();


    private:

////////////////////////////////////////////////////////////////////////
        int32 ServConnect();

        // destroy iServ
        void ServExit();

        //----------------------------------------------------------------
        /************************* client side **************************/
        //----------------------------------------------------------------
        int32 ConnectSocket(int timeout);

        //----------------------------------------------------------------
        /************************* server side **************************/
        //----------------------------------------------------------------
        int32 CreateTcpServ();

        int32 Listen();

        int32 Accept(int timeout);

        //--------------------------------------------------------------------
        /************************* cleanup session **************************/
        //--------------------------------------------------------------------
        // destroy cleanup client n server
        void CleanupTcp();

        void CloseTcpAccept();

        void CloseTcp();

        void DeleteTcp();
        void DeleteTcpAccept();
        void CancelConnect();
        //----------------------------------------------------------------------
        /************************* Handle Sock Event **************************/
        //----------------------------------------------------------------------

        ///////////////////////////////////////////////////////////////////
        void HandleSocketEvent(int32 aId, TPVSocketFxn aFxn,
                               TPVSocketEvent aEvent, int32 aSocketError);
        void SetRemoteAddress(char* aAddr, int port);
        ///////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////
        OsclSocketServ *iServ;
        OsclTCPSocket *iTCPServerSocket;
        OsclTCPSocket *iTCPClientSocket;
        OsclTCPSocket *iTCPAcceptSocket;

        class SockAlloc: public Oscl_DefAlloc
        {
            public:
                OsclAny* allocate(const uint32 s)
                {
                    return malloc(s);
                }
                void deallocate(OsclAny* p)
                {
                    free(p);
                }
        } iSockAlloc;

        OsclNetworkAddress iLocalAddrStr;
        OsclNetworkAddress iConnectAddr;
        int iClientTimeout;
        int iServerTimeout;
        bool iCheckConnection;
        PVTerminalType iTerminalType;
        bool iDisconnected;

        PVMFClientServerSocketNode* iCommServer;
        TwoWaySocketObserver *iObserver;
        ///////////////////////////////////////////////////////////////////////
};


#endif
