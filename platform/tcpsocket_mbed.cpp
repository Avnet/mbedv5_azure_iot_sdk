// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stddef.h>
#include "mbed.h"
#include "TCPSocket.h"
#include "azure_c_shared_utility/tcpsocketconnection_c.h"

#include "jimdbg.h"

#define MBED_RECEIVE_BYTES_VALUE    512

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
    return tcpsocketconnection_receive(tcpSocketHandle,data,length);
}

int tcpsocketconnection_send_all(TCPSOCKETCONNECTION_HANDLE tcpSocketHandle, const char* data, int length)
{
    return tcpsocketconnection_send(tcpSocketHandle,data,length);
}

static bool sendingData = false;
static char *txdata;
void txdata_cb(void)
{
    sendingData = false;
    free(txdata);
}

int tcpsocketconnection_send(TCPSOCKETCONNECTION_HANDLE tcpSocketHandle, const char* data, int length)
{
    TCPSocket* socket = (TCPSocket*)tcpSocketHandle;
    int sndlen        =  MBED_RECEIVE_BYTES_VALUE;

    if( sendingData )
        return NSAPI_ERROR_WOULD_BLOCK;

    if( length < sndlen )
        sndlen = length;

    txdata = (char*)malloc(sndlen);
    memcpy(txdata,data,sndlen);

    int cnt = socket->send(txdata, sndlen);
    if( cnt == NSAPI_ERROR_WOULD_BLOCK ) {
JMF_INFO("Tx'ing data\n");
        sendingData = true;
        socket->sigio(txdata_cb);
        }
    else if (cnt == sndlen)
        free(txdata);
    else{
        free(txdata);
        cnt=NSAPI_ERROR_DEVICE_ERROR;
        }

    return cnt;
}

static bool try2getdata = false;
void rxdata_cb(void)
{
    try2getdata = false;
}

int tcpsocketconnection_receive(TCPSOCKETCONNECTION_HANDLE tcpSocketHandle, char* data, int length)
{
    static char loc_data[MBED_RECEIVE_BYTES_VALUE];
    TCPSocket* socket = (TCPSocket*)tcpSocketHandle;
    int getlen        =  MBED_RECEIVE_BYTES_VALUE;

    if( try2getdata ) {
        Thread::yield(); //wait for data to be received
        return NSAPI_ERROR_WOULD_BLOCK;
        }

    if( length < getlen )
        getlen = length;

    int cnt = socket->recv(loc_data, getlen);
    if( cnt == NSAPI_ERROR_WOULD_BLOCK ) {
        try2getdata = true;
        socket->sigio(rxdata_cb);
        }
    else if( cnt > 0 )
        memcpy(data,loc_data,cnt);
    return cnt;
}

