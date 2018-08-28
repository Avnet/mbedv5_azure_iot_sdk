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

static int  bufCnt = 0;


void rxData(void)
{
    gettingData = false;
}

int tcpsocketconnection_receive(TCPSOCKETCONNECTION_HANDLE tcpSocketHandle, char* data, int length)
{
    TCPSocket* socket = (TCPSocket*)tcpSocketHandle;
    static char loc_data[MBED_RECEIVE_BYTES_VALUE];

    int    cnt, ocnt = length; 

    if( gettingData )
        return NSAPI_ERROR_WOULD_BLOCK;

    if( ocnt > MBED_RECEIVE_BYTES_VALUE-bufCnt )
        ocnt = MBED_RECEIVE_BYTES_VALUE-bufCnt;
    
    cnt = socket->recv(&loc_data[bufCnt], ocnt);
    if( cnt == NSAPI_ERROR_WOULD_BLOCK ) {
        gettingData = true;
        socket->sigio(rxData);
        }

    if( cnt > 0 || bufCnt > 0) {
        cnt += bufCnt;
        printf("      JMF:we have %3d bytes to return,asked for %3d. ",cnt,length);
        if( cnt > length ) {
            bufCnt = cnt-length;
            cnt = length;
            memcpy(data,loc_data,cnt);
            memcpy(loc_data, &loc_data[length], bufCnt);
            printf("We have %d more bytes than asked for, ",bufCnt);
            }
         else if (cnt < length ) {
            memcpy(data,loc_data,cnt);
            }
         else if (cnt == length)
            memcpy(data,loc_data,cnt);
         printf("returning %d bytes\n",cnt);
         }
    return cnt;
}

