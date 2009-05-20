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

#include "twowaysocket.h"

#ifndef PVMF_CLIENTSERVER_SOCKET_NODE_FACTORY_H_INCLUDED
#include "pvmf_clientserver_socket_factory.h"
#endif


//socket object IDs.
#define TCPCLIENTID 1
#define TCPSERVERID 2
#define EPVServer 1
#define EPVClient 2

#define TCPSOCKETACCEPTID 3
#define LISTEN_QUEUE_SIZE 12


TwoWaySocket::TwoWaySocket(TwoWaySocketObserver *aObserver) :
        iServ(NULL),
        iTCPServerSocket(NULL),
        iTCPClientSocket(NULL),
        iTCPAcceptSocket(NULL),
        iClientTimeout(-1),
        iServerTimeout(-1),
        iCheckConnection(false),
        iTerminalType((enum PVTerminalType)EPVClient),
        iDisconnected(false),
        iCommServer(NULL),
        iObserver(aObserver)
{

}

TwoWaySocket::~TwoWaySocket()
{
}

void TwoWaySocket::DeleteCommServer()
{
    PVMFClientServerSocketNodeFactory::DeleteClientServerSocketNode(iCommServer);
    iCommServer = NULL;
}

bool TwoWaySocket::ConnectSocket(bool aIsServer, int aPort, char* aIpAddr)
{
    ServConnect();
    if (aIsServer)
    {
        iTerminalType = (enum PVTerminalType)EPVServer;
        iLocalAddrStr.ipAddr.Set(aIpAddr);
        iLocalAddrStr.port = aPort;
        if (CreateTcpServ() != OsclErrNone)
        {
            // do cleanup for failure
            CleanupTcp();
            ServExit();
            return false;
        }
        //wait for HandleSocketEvent for connection success
    }
    else    // it is a client
    {
        iTerminalType = (enum PVTerminalType)EPVClient;
        SetRemoteAddress(aIpAddr, aPort);

        if (ConnectSocket(iClientTimeout) != OsclErrNone)
        {
            // do cleanup for failure
            CleanupTcp();
            ServExit();
            return false;
        }
        //wait for HandleSocketEvent for connection success
    }
    return true;
}


////////////////////////////////////////////////////////////////////////
int32 TwoWaySocket::ServConnect()
{
    //create & connect server
    int32 err = OsclErrNone;
    if (!iServ)
    {
        OSCL_TRY(err, iServ = OsclSocketServ::NewL(iSockAlloc););
        if (iServ && (err == OsclErrNone))
        {
            err = iServ->Connect();
        }
        else
        {
            printf("Error in ServConnect %d!\n", err);
        }
    }
    //check for socket connection
    iCheckConnection = true;
    return err;
}

// destroy iServ
void TwoWaySocket::ServExit()
{
    //close & delete server
    if (iServ)
    {
        iServ->Close();
        iServ->~OsclSocketServ();
        iSockAlloc.deallocate(iServ);
        iServ = NULL;
    }
}

//----------------------------------------------------------------
/************************* client side **************************/
//----------------------------------------------------------------
int32 TwoWaySocket::ConnectSocket(int timeout)
{
    int32 err = OsclErrNone;
    //create...
    if (!iTCPClientSocket && iServ)
    {
        OSCL_TRY(err, iTCPClientSocket = OsclTCPSocket::NewL(iSockAlloc, *iServ, this, TCPCLIENTID););

        if (iTCPClientSocket && (err == OsclErrNone))
        {
            err = iTCPClientSocket->Connect(iConnectAddr, timeout);
            if (err == EPVSocketPending)
            {
                err = OsclErrNone;
            }
        }
        else
        {
            printf("Error in ConnectSocket %d!\n", err);
        }
    }
    return err;
}
bool TwoWaySocket::Connect()
{
    return iCheckConnection;
}

//----------------------------------------------------------------
/************************* server side **************************/
//----------------------------------------------------------------
int32 TwoWaySocket::CreateTcpServ()
{
    //create & bind...
    if (iTCPServerSocket)
        DeleteTcp();

    int32 err = OsclErrNone;

    if (iServ)
    {
        OSCL_TRY(err, iTCPServerSocket = OsclTCPSocket::NewL(iSockAlloc, *iServ, this, TCPSERVERID););
        if (iTCPServerSocket && (err == OsclErrNone))
        {
            err = iTCPServerSocket->Bind(iLocalAddrStr);
            if (err == OsclErrNone)
            {
                err = Listen();
            }
            else
            {
                printf("Error in CreateTcpServ: binding socket %d!\n", err);
            }
        }
        else
        {
            printf("Error in CreateTcpServ: creating new server socket %d!\n", err);
        }
    }
    return err;
}

int32 TwoWaySocket::Listen()
{
    //listen...
    int32 err = iTCPServerSocket->Listen(LISTEN_QUEUE_SIZE);    //DEFAULT_MESSAGE_SLOTS
    if (err == OsclErrNone)
    {
        //continue to Accept
        err = Accept(iServerTimeout);
        if (err)
        {
            printf("Error accepting on socket %d!\n", err);
        }
    }
    return err;
}

int32 TwoWaySocket::Accept(int timeout)
{
    int err = OsclErrNone;
    if (iTCPServerSocket)
    {
        err = iTCPServerSocket->Accept(timeout);
        if (err == EPVSocketPending)
        {
            err = OsclErrNone;
        }
    }
    return err;
}

//--------------------------------------------------------------------
/************************* cleanup session **************************/
//--------------------------------------------------------------------
// destroy cleanup client n server
void TwoWaySocket::CleanupTcp()
{
    CloseTcpAccept();
    CloseTcp();
    DeleteTcp();

    //for socket connection closed
    iCheckConnection = false;
}

void TwoWaySocket::CloseTcpAccept()
{
    if (iTCPAcceptSocket)
    {
        int err = iTCPAcceptSocket->Close();
        if (err == OsclErrNone)
            return;
    }
    return;
}

void TwoWaySocket::CloseTcp()
{
    if (iTCPServerSocket)
    {
        int err = iTCPServerSocket->Close();
        if (err == OsclErrNone)
        {
            return;
        }
    }
    return;
}

void TwoWaySocket::DeleteTcp()
{
    if (iTCPServerSocket)
    {
        iTCPServerSocket->~OsclTCPSocket();
        iSockAlloc.deallocate(iTCPServerSocket);
        iTCPServerSocket = NULL;
    }
    if (iTCPClientSocket)
    {
        iTCPClientSocket->~OsclTCPSocket();
        iSockAlloc.deallocate(iTCPClientSocket);
        iTCPClientSocket = NULL;
    }
}

void TwoWaySocket::DeleteTcpAccept()
{
    if (iTCPAcceptSocket)
    {
        iTCPAcceptSocket->~OsclTCPSocket();
        iSockAlloc.deallocate(iTCPAcceptSocket);
        iTCPAcceptSocket = NULL;
    }
}

void TwoWaySocket::CancelConnect()
{
    //connect...
    iTCPClientSocket->CancelConnect();
}

//----------------------------------------------------------------------
/************************* Handle Sock Event **************************/
//----------------------------------------------------------------------

///////////////////////////////////////////////////////////////////
void TwoWaySocket::HandleSocketEvent(int32 aId,
                                     TPVSocketFxn aFxn,
                                     TPVSocketEvent aEvent,
                                     int32 aSocketError)
{
    OSCL_UNUSED_ARG(aId);
    // Supress the error that socket returns during shutdown because of not being connected
    //Occurs during remote disconnect.
    if ((aFxn == EPVSocketShutdown) && (aSocketError == ENOTCONNECTED))
    {
        aEvent = EPVSocketSuccess;
    }

    if (aEvent == EPVSocketSuccess)
    {
        switch (aFxn)
        {
            case EPVSocketConnect:
            {
                // pass up to observer
                PVMFNodeInterface* commServer =
                    PVMFClientServerSocketNodeFactory::CreateClientServerSocketNode(iTCPClientSocket);
                iObserver->SocketConnected(commServer);
                iCommServer = (PVMFClientServerSocketNode*)commServer;
            }
            break;

            case EPVSocketAccept:
            {
                iTCPAcceptSocket = iTCPServerSocket->GetAcceptedSocketL(TCPSOCKETACCEPTID);
                // pass up to observer
                PVMFNodeInterface* commServer =
                    PVMFClientServerSocketNodeFactory::CreateClientServerSocketNode(iTCPAcceptSocket);
                iObserver->SocketConnected(commServer);
                iCommServer = (PVMFClientServerSocketNode*)commServer;
            }
            break;

            case EPVSocketRecv:
            {
                if (iCommServer)
                {
                    iCommServer->HandleSocketEvent(aId, aFxn, aEvent, aSocketError);
                }
            }
            break;

            case EPVSocketSend:
            {
                if (iCommServer)
                {
                    iCommServer->HandleSocketEvent(aId, aFxn, aEvent, aSocketError);
                }
            }
            break;

            case EPVSocketShutdown:
            {
                //delete sockets here
                CleanupTcp();
                ServExit();

                if (iCommServer)
                {
                    iCommServer->HandleSocketEvent(aId, aFxn, aEvent, aSocketError);
                }
                iCheckConnection = false;   //socket connection closed
            }
            break;

            default:
                OSCL_ASSERT(0);
                break;
        }
    }
    else
    {// for socket cancel/failure/timeout
        if (aFxn == EPVSocketAccept ||
                aFxn == EPVSocketConnect)
        {
            CleanupTcp();
            ServExit();
            printf("Socket error\n");
        }
        else
        {//for send/ recv failure's
            switch (aEvent)
            {
                case EPVSocketFailure:
                    if (!iDisconnected)
                    {
                        //intimate socketNode about it
                        if (iCommServer)
                        {
                            iCommServer->HandleSocketEvent(aId, aFxn, aEvent, aSocketError);
                        }
                        iObserver->SocketDisconnected();
                    }
                    break;
                case EPVSocketCancel:
                case EPVSocketTimeout:
                default:
                    if (iCommServer)
                    {
                        iCommServer->HandleSocketEvent(aId, aFxn, aEvent, aSocketError);
                    }
                    break;
            }
        }
    }
}

void TwoWaySocket::SetRemoteAddress(char* aAddr, int port)
{
    iConnectAddr.ipAddr.Set(aAddr);
    iConnectAddr.port = port;
}

