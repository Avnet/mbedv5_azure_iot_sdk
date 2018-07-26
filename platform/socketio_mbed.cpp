// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "azure_c_shared_utility/tcpsocketconnection_c.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"

#include "mbed.h"
#include "TCPSocket.h"

#define UNABLE_TO_COMPLETE          -2
#define MBED_RECEIVE_BYTES_VALUE    128

static OPTIONHANDLER_HANDLE socketio_retrieveoptions(CONCRETE_IO_HANDLE socket_io);

typedef enum IO_STATE_TAG
{
    IO_STATE_CLOSED,
    IO_STATE_OPENING,
    IO_STATE_OPEN,
    IO_STATE_CLOSING,
    IO_STATE_ERROR
} IO_STATE;

typedef struct PENDING_SOCKET_IO_TAG
{
    unsigned char* bytes;
    size_t size;
    ON_SEND_COMPLETE on_send_complete;
    void* callback_context;
    SINGLYLINKEDLIST_HANDLE pending_io_list;
} PENDING_SOCKET_IO;

typedef struct SOCKET_IO_INSTANCE_TAG
{
    TCPSOCKETCONNECTION_HANDLE socket;
    ON_BYTES_RECEIVED on_bytes_received;
    ON_IO_ERROR on_io_error;
    void* on_bytes_received_context;
    void* on_io_error_context;
    char* hostname;
    int port;
    IO_STATE io_state;
    SINGLYLINKEDLIST_HANDLE pending_io_list;
} SOCKET_IO_INSTANCE;

static const IO_INTERFACE_DESCRIPTION socket_io_interface_description =
{
    socketio_retrieveoptions,
    socketio_create,
    socketio_destroy,
    socketio_open,
    socketio_close,
    socketio_send,
    socketio_dowork,
    socketio_setoption
};

//
// The 'Option' functions are not used but maintained to ensure compatibility with other 
// Azure functionallity
//
static void* socketio_CloneOption(const char* name, const void* value)
{
    /* Not implementing any options */
    (void)name;
    (void)value;
    return NULL;
}

static void socketio_DestroyOption(const char* name, const void* value)
{
    /* Not implementing any options */
    (void)name;
    (void)value;
    return;
}

static OPTIONHANDLER_HANDLE socketio_retrieveoptions(CONCRETE_IO_HANDLE socket_io)
{
    /* Not implementing any options */
    (void)socket_io;
    return NULL;
}

int socketio_setoption(CONCRETE_IO_HANDLE socket_io, const char* optionName, const void* value)
{
    /* Not implementing any options */
    return __FAILURE__;
}

const IO_INTERFACE_DESCRIPTION* socketio_get_interface_description(void)
{
    return &socket_io_interface_description;
}

static void indicate_error(SOCKET_IO_INSTANCE* psock)
{
    if (psock->on_io_error != NULL)
        psock->on_io_error(psock->on_io_error_context);
}

static int add_pending_io(SOCKET_IO_INSTANCE* psock, const unsigned char* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    PENDING_SOCKET_IO* p_socket_io = (PENDING_SOCKET_IO*)malloc(sizeof(PENDING_SOCKET_IO));
    if (p_socket_io == NULL)
        return __FAILURE__;

    p_socket_io->bytes = (unsigned char*)malloc(size);
    if (p_socket_io->bytes == NULL) {
        free(p_socket_io);
        return __FAILURE__;
        }

    p_socket_io->size = size;
    p_socket_io->on_send_complete = on_send_complete;
    p_socket_io->callback_context = callback_context;
    p_socket_io->pending_io_list = psock->pending_io_list;
    memcpy(p_socket_io->bytes, buffer, size);

    if (singlylinkedlist_add(psock->pending_io_list, p_socket_io) == NULL) {
        free(p_socket_io->bytes);
        free(p_socket_io);
        return __FAILURE__;
        }
    return 0;
}

CONCRETE_IO_HANDLE socketio_create(void* io_create_parameters)
{
    SOCKETIO_CONFIG* socket_io_config = (SOCKETIO_CONFIG*)io_create_parameters;
    SOCKET_IO_INSTANCE* iop = (SOCKET_IO_INSTANCE*)malloc(sizeof(SOCKET_IO_INSTANCE));

    if (socket_io_config == NULL || iop == NULL)
        return NULL;
    iop->pending_io_list = singlylinkedlist_create();
    if (iop->pending_io_list == NULL) {
        free(iop);
        return NULL;
        }
    iop->hostname = (char*)malloc(strlen(socket_io_config->hostname) + 1);
    if (iop->hostname == NULL) {
        singlylinkedlist_destroy(iop->pending_io_list);
        free(iop);
        return NULL;
        }
    strcpy(iop->hostname, socket_io_config->hostname);
    iop->port = socket_io_config->port;
    iop->on_bytes_received = NULL;
    iop->on_io_error = NULL;
    iop->on_bytes_received_context = NULL;
    iop->on_io_error_context = NULL;
    iop->io_state = IO_STATE_CLOSED;
    iop->socket = NULL;
    return iop;
}

void socketio_destroy(CONCRETE_IO_HANDLE socket_io)
{
    if (socket_io == NULL)
        return;

    SOCKET_IO_INSTANCE* psock = (SOCKET_IO_INSTANCE*)socket_io;

    tcpsocketconnection_destroy(psock->socket);

    /* clear all pending IOs */
    LIST_ITEM_HANDLE first_pending_io = singlylinkedlist_get_head_item(psock->pending_io_list);
    while (first_pending_io != NULL) {
        PENDING_SOCKET_IO* p_socket_io = (PENDING_SOCKET_IO*)singlylinkedlist_item_get_value(first_pending_io);
        if (p_socket_io != NULL) {
            free(p_socket_io->bytes);
            free(p_socket_io);
            }
        singlylinkedlist_remove(psock->pending_io_list, first_pending_io);
        first_pending_io = singlylinkedlist_get_head_item(psock->pending_io_list);
        }

    singlylinkedlist_destroy(psock->pending_io_list);
    free(psock->hostname);
    free(socket_io);
}

int socketio_open(CONCRETE_IO_HANDLE socket_io, ON_IO_OPEN_COMPLETE on_io_open_complete, 
                  void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, 
                  void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    SOCKET_IO_INSTANCE* psock = (SOCKET_IO_INSTANCE*)socket_io;
    if (socket_io == NULL) 
        return __FAILURE__;
    psock->socket = tcpsocketconnection_create();
    if (psock->socket == NULL)
        return __FAILURE__;
    
    if (tcpsocketconnection_connect(psock->socket, psock->hostname, psock->port) != 0) {
        tcpsocketconnection_destroy(psock->socket);
        psock->socket = NULL;
        return __FAILURE__;
        }

    tcpsocketconnection_set_blocking(psock->socket, false, 0);

    psock->on_bytes_received = on_bytes_received;
    psock->on_bytes_received_context = on_bytes_received_context;

    psock->on_io_error = on_io_error;
    psock->on_io_error_context = on_io_error_context;

    psock->io_state = IO_STATE_OPEN;

    if (on_io_open_complete != NULL)
        on_io_open_complete(on_io_open_complete_context, IO_OPEN_OK);

    return 0;
}

int socketio_close(CONCRETE_IO_HANDLE socket_io, void (*on_io_close_complete)(void*), void* callback_context)
{
    int result = 0;

    if (socket_io == NULL)
        return __FAILURE__;
    
    SOCKET_IO_INSTANCE* psock = (SOCKET_IO_INSTANCE*)socket_io;

    if ((psock->io_state == IO_STATE_CLOSED) || (psock->io_state == IO_STATE_CLOSING)) 
        return __FAILURE__;
    tcpsocketconnection_close(psock->socket);
    psock->socket = NULL;
    psock->io_state = IO_STATE_CLOSED;

    if (on_io_close_complete != NULL)
        on_io_close_complete(callback_context);

    return 0;
}

int socketio_send(CONCRETE_IO_HANDLE socket_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result;

    if ((socket_io == NULL) || (buffer == NULL) || (size == 0))
        return __FAILURE__;

    SOCKET_IO_INSTANCE* psock = (SOCKET_IO_INSTANCE*)socket_io;
    if (psock->io_state != IO_STATE_OPEN)
        return __FAILURE__;
    else {
        LIST_ITEM_HANDLE first_pending_io = singlylinkedlist_get_head_item(psock->pending_io_list);
        if (first_pending_io != NULL) {
            if (add_pending_io(psock, (unsigned char*)buffer, size, on_send_complete, callback_context) != 0)
                return __FAILURE__;
            result = 0;
            }
        else{
            int send_result = tcpsocketconnection_send(psock->socket, (const char*)buffer, size);
            if (send_result != (int)size) {
                if (send_result < 0)
                    send_result = 0;
    
                /* queue data */
                if (add_pending_io(psock, (unsigned char*)buffer + send_result, size - send_result, on_send_complete, callback_context) != 0)
                    return __FAILURE__;
                result = 0;
                }
            else {
                if (on_send_complete != NULL)
                    on_send_complete(callback_context, IO_SEND_OK);
                result = 0;
                }
            }
        }

    return result;
}

void socketio_dowork(CONCRETE_IO_HANDLE socket_io)
{
    int received = 1;

    if (socket_io == NULL)
        return;

    SOCKET_IO_INSTANCE* psock = (SOCKET_IO_INSTANCE*)socket_io;
    if (psock->io_state != IO_STATE_OPEN) 
        return;

    LIST_ITEM_HANDLE first_pending_io = singlylinkedlist_get_head_item(psock->pending_io_list);
    while (first_pending_io != NULL) {
        PENDING_SOCKET_IO* p_socket_io = (PENDING_SOCKET_IO*)singlylinkedlist_item_get_value(first_pending_io);
        if (p_socket_io == NULL) {
            psock->io_state = IO_STATE_ERROR;
            indicate_error(psock);
            break;
            }

        int send_result=tcpsocketconnection_send(psock->socket, (const char*)p_socket_io->bytes, p_socket_io->size);
        if (send_result != (int)p_socket_io->size) {
            if( send_result == NSAPI_ERROR_WOULD_BLOCK ) {
                /* just wait */;
                }
            else if (send_result < 0) {
                if (send_result < UNABLE_TO_COMPLETE) {
                    // Bad error.  Indicate as much.
                    psock->io_state = IO_STATE_ERROR;
                    indicate_error(psock);
                    }
                break;
                }
            else {
                /* send something, wait for the rest */
                memmove(p_socket_io->bytes, p_socket_io->bytes+send_result, p_socket_io->size-send_result);
                }
            }
        else {
            if (p_socket_io->on_send_complete != NULL)
                p_socket_io->on_send_complete(p_socket_io->callback_context, IO_SEND_OK);

            free(p_socket_io->bytes);
            free(p_socket_io);
            if (singlylinkedlist_remove(psock->pending_io_list, first_pending_io) != 0) {
                psock->io_state = IO_STATE_ERROR;
                indicate_error(psock);
                }
            }
        first_pending_io = singlylinkedlist_get_head_item(psock->pending_io_list);
        }

    while (received > 0) {
        unsigned char* recv_bytes = (unsigned char*)malloc(MBED_RECEIVE_BYTES_VALUE);
        if (recv_bytes == NULL) {
            LogError("Socketio_Failure: NULL allocating input buffer.");
            indicate_error(psock);
            }
        else {
            received = tcpsocketconnection_receive(psock->socket, (char*)recv_bytes, MBED_RECEIVE_BYTES_VALUE);
            if (received > 0) {
                if (psock->on_bytes_received != NULL) {
                    /* explictly ignoring here the result of the callback */
                    (void)psock->on_bytes_received(psock->on_bytes_received_context, recv_bytes, received);
                    }
                }
            free(recv_bytes);
            }
        }
}

