#include "sockets.h"

/* MARK: --- CLIENT SIDE --- */
int create_conection(t_log *logger, char *ip, char *port)
{
    int client_socket;
    struct addrinfo hints;
    struct addrinfo *server_info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, port, &hints, &server_info);

    // Create client socket
    client_socket = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

    if (client_socket == -1)
    {
        log_error(logger, "Error al crear el socket");
        exit(1);
    }

    // Connect socket
    int conection = connect(client_socket, server_info->ai_addr, server_info->ai_addrlen);

    if (conection == -1)
    {
        log_error(logger, "Error al conectar el socket");
        exit(1);
    }

    freeaddrinfo(server_info);

    return client_socket;
}

/* MARK: --- SERVER SIDE --- */
int initialize_server(t_log *logger, const char *name, char *ip, char *port)
{
    int server_socket;
    bool conection_succesful;

    struct addrinfo hints;
    struct addrinfo *server_info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, port, &hints, &server_info);
    //LA UNICA PARTE EN LA QUE NO HABIA PODIDO ESTAR, QUE ERA EL CHECKPOINT 1 Y ESTABA MAL HECHA
    //ACA EL PRIMER PARAMETRO DEL GETADDRINFO TIENE QUE SER NULL SI QUIERO LEVANTAR UN SERVER
    //OSEA SI QUIERO QUE ESE SOCKET SEA DE ESCUCHA. TENGO QUE PASARLE UN NULL, NO ME IMPORTA MANDARLE
    //UNA IP TIENE QUE SER NULL. Y ADEMAS TENES EN AI_PASSIVE EL FLAG DE ARRIBA. PERO ESTO HIZO UN LIO EL 
    //DIA ANTERIOR A LA ENTREGA. MEMORIA EL PARAMETRO DE SU IP LEIA ALGO INVALIDO DE LA CONFIG
    //QUE NOSE PORQUE LO HICIERNO ASI. PERO LEI VACIO Y ENTONCES QUEDABA VACIO Y ACA LLEGABAUN NULL Y TODO BIEN
    //OSEA LEIA IP NULL Y ENTONCES LLEGABA ACA UN NULL Y LIST. PERO CPU SI LEIA ALGO PORQUE LE PASABAMOS LOCALHOST
    //PÁRA QUE USE LE IP LOCAL, MAS ALLA DE QUE PARA LEVANTAR UN SERVER NO HAY QUE PASAR NINGUNA IP PORQUE
    //ES OBVIO DONDE LO VAMOS A LEVANTAR, LO ESTABAMOS PASANDO, PER NO IMPORTA PDOAMOS PASARLO O NNO
    //LA COSA ERA NO USARLA ACA, ACA HABIA QUE PONER UN NULL EN EL PRIMER PARAMETOR Y ESTABA EL CHAR *IP
    //EN LA GUIA DICE CLARAMENTE QUE TIEN QUESER  NULL Y LO HABIAN PUESTO
    //HORAS PERDIDAS EN ESTO.
    if (server_info == NULL)
    {
        log_error(logger, "Error al obtener la informacion del servidor %s", name);
        return -1;
    }

    // Recorre los addrinfo
    for (struct addrinfo *info = server_info; info != NULL; info = info->ai_next)
    {
        server_socket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

        if (server_socket == -1)
        {
            log_error(logger, "Error al crear el socket %s", name);
            continue;
        }
        //Este if no estaba, es para que no tengamos que esperar un minuto para
        //volver a levantar todos los modulos, lo configuramos como reusable
        //solo cambiamos el primer parametro por el resultado de la funcion socket de arriba
        if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        error("setsockopt(SO_REUSEADDR) failed");
        //hasta aca es el codigo para no esperar el minuto extra

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

    if (!conection_succesful)
    {
        log_error(logger, "Error al crear el socket %s", name);
        free(server_info);
        exit(1);
    }

    // Listen
    listen(server_socket, SOMAXCONN);
    log_info(logger, "LISTENING IN... %s:%s (%s)\n", ip, port, name);

    freeaddrinfo(server_info);

    return server_socket;
}

int wait_client(t_log *logger, const char *name, int server_socket)
{
    struct sockaddr_in client_addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);

    // Accept client
    int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_size);

    if (client_socket == -1)
    {
        log_error(logger, "Error al aceptar la conexion %s", name);
        exit(1);
    }

    log_info(logger, "Conexion aceptada %s", name);

    return client_socket;
}



void close_conection(int *client_socket)
{
    close(client_socket);
    client_socket = -1;
}
