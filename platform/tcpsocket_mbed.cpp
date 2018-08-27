// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stddef.h>
#include "mbed.h"
#include "TCPSocket.h"
#include "azure_c_shared_utility/tcpsocketconnection_c.h"

#define MBED_RECEIVE_BYTES_VALUE    128

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

    if( blocking ) 
        socket->set_blocking(true);
    else
        socket->set_timeout(timeout);	
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

int tcpsocketconnection_receive_all(TCPSOCKETCONNECTION_HANDLE tcpSocketHandle, char* data, int length)
{
    return tcpsocketconnection_receive(tcpSocketHandle, data, length);
}

int tcpsocketconnection_send_all(TCPSOCKETCONNECTION_HANDLE tcpSocketHandle, const char* data, int length)
{
    return tcpsocketconnection_send(tcpSocketHandle,data,length);
}

int tcpsocketconnection_send(TCPSOCKETCONNECTION_HANDLE tcpSocketHandle, const char* data, int length)
{
    TCPSocket* socket = (TCPSocket*)tcpSocketHandle;
    return socket->send((char*)data, length);
}

static bool gettingData = false;
static int  left_over_data = 0;

void rxData(void)
{
    gettingData = false;
}

int tcpsocketconnection_receive(TCPSOCKETCONNECTION_HANDLE tcpSocketHandle, char* data, int length)
{
    TCPSocket* socket = (TCPSocket*)tcpSocketHandle;
    static char loc_data[MBED_RECEIVE_BYTES_VALUE];
    int    cnt = length;

    if( left_over_data>0 ) {
        cnt = left_over_data;
        left_over_data = 0;
        memcpy(data, loc_data, cnt);
        return cnt;
        }

    if( gettingData )
        return 0;

    if( cnt > MBED_RECEIVE_BYTES_VALUE )
        cnt = MBED_RECEIVE_BYTES_VALUE;
    
    cnt = socket->recv(loc_data, cnt);
    if( cnt == NSAPI_ERROR_WOULD_BLOCK ) {
        gettingData = true;
        socket->sigio(rxData);
        }
    else if ( cnt == NSAPI_ERROR_TIMEOUT )
        printf("Poor signal strength? RX timed out.\n");

    if( cnt > 0 ) {
        if( cnt > length ) {
            memcpy(data,loc_data,length);
            memcpy(loc_data, &loc_data[length], cnt-length);
            left_over_data = cnt-length;
printf("JMF:have %d bytes received that weren't asked for\n",left_over_data);
            }
         else
            memcpy(data,loc_data,cnt);
        }

    return cnt;
}

