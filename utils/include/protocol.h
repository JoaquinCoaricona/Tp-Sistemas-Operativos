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
t_buffer *create_buffer();
t_packet *create_packet(op_code code, t_buffer *buffer);
void add_to_packet(t_packet *packet, void *stream, int size);
void *serialize_packet(t_packet *packet, int buffer_size);
void destroy_packet(t_packet *packet);
void send_packet(t_packet *packet, int client_socket);

// Server
t_list *fetch_packet(int client_socket);
void *fetch_buffer(int *size, int client_socket);
int fetch_codop(int client_socket);

#endif // PROTOCOL_H