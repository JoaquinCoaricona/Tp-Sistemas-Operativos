#ifndef COMUNICATION_H
#define COMUNICATION_H

// INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <inttypes.h>
#include <pthread.h>
#include "protocol.h"
#include "sockets.h"
#include "opcode.h"

// STRUCTS
typedef struct
{
    t_log *logger;
    int fd;
    char *server_name;
} t_process_conection_args;

// Function declarations
int server_listen(t_log *logger, const char *server_name, int server_socket);
#endif // COMUNICATION_H