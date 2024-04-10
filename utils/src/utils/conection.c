#include "conection.h"

int create_conection(char *ip, char *port)
{
    struct addrinfo hints;
    struct addrinfo *server_info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &server_info);

    // Create socket
    int client_socket = 0;

    client_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

    if (client_socket == -1)
    {
        perror("Error al crear el socket");
        exit(1);
    }


    //Connect socket
    int conexion = connect(client_socket, server_info->ai_addr, server_info->ai_addrlen);

    if (conexion == -1)
    {
        perror("Error al conectar el socket");
        exit(1);
    }

    freeaddrinfo(server_info);

    return client_socket;
}