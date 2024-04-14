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
int create_conection(t_log *logger, char *ip, char *port);
void close_conection(int *client_socket);

// SERVER
int initialize_server(t_log *logger, const char *name, char *ip, char *port);
int wait_client(t_log *logger, const char *name, int server_socket);
int wait_client_threaded(t_log *logger, const char *name, int server_socket, void *(*serve_client));

#endif /* SOCKETS_H */