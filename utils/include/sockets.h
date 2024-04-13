#ifndef SOCKETS_H
#define SOCKETS_H

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

// Function declarations
int create_connection(char *ip, char *port); // Create a client_socket
int initialize_server(t_log *logger, const char *name, char *ip, char *port);
int wait_client(t_log *logger, const char *name, int server_socket);
int wait_client_threaded(t_log *logger, const char *name, int server_socket, void *(*serve_client));
void close_connection(int *client_socket);

#endif /* SOCKETS_H */