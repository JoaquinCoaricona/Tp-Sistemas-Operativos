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

typedef enum
{
    MESSAGE,
    PACKET,
} op_code;



#endif // PROTOCOL_H