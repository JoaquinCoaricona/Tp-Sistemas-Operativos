/**
 * @file protocol.h
 * @author your name (you@domain.com)
 * @brief Handles the protocol of the communication between the client and the server
 * @version 1.0
 * @date 2024-04-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

// INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <string.h>
#include <assert.h>
#include "opcode.h"

// STRUCTS
typedef struct
{
    int size;
    void *stream;

} t_buffer;

typedef struct
{
    op_code code;
    t_buffer *buffer;

} t_packet;

// FUNCTIONS

// Client
/**
 * @brief Create a buffer object
 * 
 * @return t_buffer* 
 */
t_buffer *create_buffer();

/**
 * @brief Create a packet object that contains the operation code and the buffer to send
 * 
 * @param code operation code
 * @param buffer buffer to send
 * @return t_packet* 
 */
t_packet *create_packet(op_code code, t_buffer *buffer);

/**
 * @brief Add the stream to the packet
 * 
 * @param packet Packet to add the stream
 * @param stream Data of the buffer
 * @param size size of the stream
 */
void add_to_packet(t_packet *packet, void *stream, int size);

/**
 * @brief Serializes the packet to send it
 * 
 * @param packet packet to serialize
 * @param buffer_size size of the buffer
 * @return void* 
 */
void *serialize_packet(t_packet *packet, int buffer_size);

/**
 * @brief Destroys the packet freeing the memory. It is used by send_packet function
 * 
 * @param packet packet to destroy
 */
void destroy_packet(t_packet *packet);

/**
 * @brief Sends the packet to the server 
 * 
 * @param packet packet to send
 * @param client_socket client file descriptor
 */
void send_packet(t_packet *packet, int client_socket);

// Server
/**
 * @brief Fetch the packet from the client
 * 
 * @param client_socket client file descriptor
 * @return t_list*  List of packets
 */
t_list *fetch_packet(int client_socket);

/**
 * @brief Fetch the buffer from the client. It is used by fetch_packet function
 * 
 * @param size size of the buffer
 * @param client_socket client file descriptor
 * @return void* 
 */
void *fetch_buffer(int *size, int client_socket);

/**
 * @brief Fetch the operation code from the client
 * 
 * @param client_socket client file descriptor
 * @return int Returns operation code or -1 if there is an error
 */
int fetch_codop(int client_socket);

#endif // PROTOCOL_H