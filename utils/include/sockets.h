/**
 * @file sockets.h
 * @author KernelCrafters 
 * @brief Socket Connection Functions
 * @version 1.0
 * @date 2024-04-16
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef SOCKETS_H
#define SOCKETS_H

// INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>

// FUNCTIONS

// CLIENT
/**
 * @brief Create conection with server
 * 
 * @param logger Logger from commons libraries 
 * @param ip ip to connect
 * @param port Port to connect
 * @return int Return client_socket
 */
int create_conection(t_log *logger, char *ip, char *port);

/**
 * @brief Close conection with server
 * 
 * @param client_socket client_fd to close connection
 */
void close_conection(int *client_socket);

// SERVER

/**
 * @brief initialize the server
 * 
 * @param logger logger from commons libraries
 * @param name name of the server
 * @param ip ip to connect 
 * @param port Ip port to connect   
 * @return int return server_socket
 */ 
int initialize_server(t_log *logger, const char *name, char *ip, char *port);

/**
 * @brief  Listen to the server. It is used by server_listen() 
 * 
 * @param logger logger from commons libraries  
 * @param name name of the server
 * @param server_socket server file descriptor to listen
 * @return int  return client_socket
 */
int wait_client(t_log *logger, const char *name, int server_socket);


#endif /* SOCKETS_H */

