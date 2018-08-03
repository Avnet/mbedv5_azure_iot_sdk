// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stddef.h>
#include "mbed.h"
#include "TCPSocket.h"
#include "azure_c_shared_utility/tcpsocketconnection_c.h"

static bool              is_connected = false;
extern NetworkInterface* platform_network;

TCPSOCKETCONNECTION_HANDLE tcpsocketconnection_create(void)
{
    TCPSocket* ptr = new TCPSocket();
    ptr->open(platform_network);
    return ptr;
}

int tcpsocketconnection_connect(TCPSOCKETCONNECTION_HANDLE tcpSocketHandle, const char* host, const int port)
{
    TCPSocket* socket = (TCPSocket*)tcpSocketHandle;
    is_connected = socket->connect(host, port);
    return is_connected;
}

void tcpsocketconnection_set_blocking(TCPSOCKETCONNECTION_HANDLE tcpSocketHandle, bool blocking, unsigned int timeout)
{
    TCPSocket* socket = (TCPSocket*)tcpSocketHandle;

    if( !blocking && !timeout )  //blocking == false with timeout == 0 is equivelent to blocking = true;
        socket->set_blocking(true);
    else{
        socket->set_timeout(timeout);	
        socket->set_blocking(blocking);
        }
}

void tcpsocketconnection_destroy(TCPSOCKETCONNECTION_HANDLE tcpSocketHandle)
{
    delete (TCPSocket*)tcpSocketHandle;
}

bool tcpsocketconnection_is_connected(TCPSOCKETCONNECTION_HANDLE tcpSocketHandle)
{
    return is_connected;
}

void tcpsocketconnection_close(TCPSOCKETCONNECTION_HANDLE tcpSocketHandle)
{
    TCPSocket* socket = (TCPSocket*)tcpSocketHandle;
    socket->close();
}

int tcpsocketconnection_send(TCPSOCKETCONNECTION_HANDLE tcpSocketHandle, const char* data, int length)
{
    TCPSocket* socket = (TCPSocket*)tcpSocketHandle;
    return socket->send((char*)data, length);
}

int tcpsocketconnection_send_all(TCPSOCKETCONNECTION_HANDLE tcpSocketHandle, const char* data, int length)
{
    TCPSocket* socket = (TCPSocket*)tcpSocketHandle;
    return socket->send((char*)data, length);
}

int tcpsocketconnection_receive(TCPSOCKETCONNECTION_HANDLE tcpSocketHandle, char* data, int length)
{
    TCPSocket* socket = (TCPSocket*)tcpSocketHandle;
    int cnt = socket->recv(data, length);
    return cnt;
}

int tcpsocketconnection_receive_all(TCPSOCKETCONNECTION_HANDLE tcpSocketHandle, char* data, int length)
{
    TCPSocket* socket = (TCPSocket*)tcpSocketHandle;
    return socket->recv(data, length);
}
