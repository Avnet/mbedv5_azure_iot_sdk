// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "mbed.h"

#include <stddef.h>
//#include "TCPSocketConnection.h"
#include "azure_c_shared_utility/tcpsocketconnection_c.h"


TCPSOCKETCONNECTION_HANDLE tcpsocketconnection_create(void)
{
    return new TCPSocket();
}

void tcpsocketconnection_set_blocking(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, bool blocking, unsigned int timeout)
{
	TCPSocket* tsc = (TCPSocket*)tcpSocketConnectionHandle;
	tsc->set_blocking(blocking);
}

void tcpsocketconnection_destroy(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle)
{
	delete (TCPSocket*)tcpSocketConnectionHandle;
}

int tcpsocketconnection_connect(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, const char* host, const int port)
{
	TCPSocket* tsc = (TCPSocket*)tcpSocketConnectionHandle;
	return tsc->connect(host, port);
}

bool tcpsocketconnection_is_connected(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle)
{
        return true;
//	TCPSocket* tsc = (TCPSocket*)tcpSocketConnectionHandle;
//	return tsc->is_connected();
}

void tcpsocketconnection_close(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle)
{
	TCPSocket* tsc = (TCPSocket*)tcpSocketConnectionHandle;
	tsc->close();
}

int tcpsocketconnection_send(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, const char* data, int length)
{
	TCPSocket* tsc = (TCPSocket*)tcpSocketConnectionHandle;
	return tsc->send((char*)data, length);
}

int tcpsocketconnection_send_all(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, const char* data, int length)
{
	TCPSocket* tsc = (TCPSocket*)tcpSocketConnectionHandle;
	return tsc->send((char*)data, length);
}

int tcpsocketconnection_receive(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, char* data, int length)
{
	TCPSocket* tsc = (TCPSocket*)tcpSocketConnectionHandle;
	return tsc->recv(data, length);
}

int tcpsocketconnection_receive_all(TCPSOCKETCONNECTION_HANDLE tcpSocketConnectionHandle, char* data, int length)
{
	TCPSocket* tsc = (TCPSocket*)tcpSocketConnectionHandle;
	return tsc->recv(data, length);
}
