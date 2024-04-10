#include "src/utils/sockets.h"

//*---- CLIENT SIDE ----*//
int create_conection(char *ip, char *port)
{
    int client_socket;
    struct addrinfo hints;
    struct addrinfo *server_info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &server_info);

    // Create client socket
    client_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

    if (client_socket == -1)
    {
        perror("Error al crear el socket");
        exit(1);
    }

    // Connect socket
    int conection = connect(client_socket, server_info->ai_addr, server_info->ai_addrlen);

    if (conection == -1)
    {
        perror("Error al conectar el socket");
        exit(1);
    }

    freeaddrinfo(server_info);

    return client_socket;
}

//*---- SERVER SIDE ----*//
int initialize_server(t_log *logger, const char *name, char *ip, char *port)
{
    int server_socket;
    bool conection_succesful;

    struct addrinfo hints;
    struct *server_info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    // Recibe los addrinfo
    getaddrinfo(ip, port, &hints, &server_info); // TODO: Get PORT from config

    // Recorre los addrinfo
    for (struct addrinfo *info = server_info; info != NULL; info = info->ai_next)
    {
        server_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

        if (server_socket == -1)
        {
            log_error(logger, "Error al crear el socket %s", name);
            continue;
        }

        // Bind socket
        if (bind(server_socket, info->ai_addr, info->ai_addrlen) == -1)
        {
            log_error(logger, "Error al bindear el socket %s", name);
            close(server_socket);
            continue;
        }
        else
        {
            conection_succesful = true;
            break;
        }
    }

    if (!connection_succesful)
    {
        log_error(logger, "Error al crear el socket %s", name);
        free(server_info);
        exit(1);
    }

    // Listen
    listen(server_socket, SOMAXCONN);
    log_info(logger, "Escuchando en %s:%s (%s)\n", ip, port, name);

    freeaddrinfo(server_info);

    return server_socket;
}

int wait_client(t_log *logger, const char *name, int server_socket)
{
    struct sockaddr_in client_addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);

    int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);

    if (client_socket == -1)
    {
        log_error(logger, "Error al aceptar la conexion %s", name);
        exit(1);
    }

    log_info(logger, "Conexion aceptada %s", name);

    return client_socket;
}

void close_connection(int *client_socket)
{
    close(client_socket);
    *client_socket = -1;
}
