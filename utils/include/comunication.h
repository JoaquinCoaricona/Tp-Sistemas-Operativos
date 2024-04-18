/**
 * @file comunication.h
 * @author KernelCrafters
 * @brief Handles the comunication between the server and the client using Multithreading
 * @version 1.0
 * @date 2024-04-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */

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

/**
 * @brief Struct to pass arguments to the process_conection function
 * 
 */
typedef struct
{
    t_log *logger;
    int fd;
    char *server_name;
} t_process_conection_args;

// Function declarations

/**
 * @brief Make the server listen in Multithreading, uses process_conection function
 * 
 * @param logger logger from commons library
 * @param server_name Name of the server
 * @param server_socket server socket to listen
 * @return int 
 */
int server_listen(t_log *logger, const char *server_name, int server_socket);
#endif // COMUNICATION_H