#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <string.h>
#include <assert.h>

// Define your protocol constants and data structures here

//TODO: Implementar op_code
typedef enum
{
    MESSAGE,
    PACKET,
} op_code;

typedef struct {
    int size;
    void* stream;
}t_buffer

typedef struct {
    op_code code;
    t_buffer* buffer;
} t_packet;



#endif // PROTOCOL_H