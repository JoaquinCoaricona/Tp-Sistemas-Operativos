#ifndef SOCKETS_H
#define SOCKETS_H

#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <string.h>
#include <stdlib.h>

// Function declarations
int create_connection(char *ip, char *port); // Create a client_socket
// TODO: int initialize_server(t_log *logger, const char *name, char *ip, char *port);
int accept_connection(int sockfd);
int connect_socket(int sockfd, const char *ip, int port);
int send_data(int sockfd, const void *data, int size);
int receive_data(int sockfd, void *buffer, int size);
void close_socket(int sockfd);

#endif /* SOCKETS_H */