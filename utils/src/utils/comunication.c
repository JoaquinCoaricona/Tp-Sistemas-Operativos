#include "comunication.h"

static void process_conection(void *args)
{
    int client_socket;
    char *server_name;
    t_log *logger;
    t_packet *packet;

    t_process_conection_args *arguments = (t_process_conection_args *)args;

    // Pasa los arguments para poder crear el thread
    logger = arguments->logger;
    client_socket = arguments->fd;
    server_name = arguments->server_name;

    free(args);

    while (client_socket != -1)
    {
        int operation_code = fetch_codop(client_socket);

        switch (operation_code)
        {
        case HANDSHAKE:
            log_info(logger, "handshake recibido %s", server_name);
            packet = fetch_packet(client_socket);
            log_info(logger, "Packet received");
            close_conection(client_socket);
            client_socket = -1;
            break;

        case -1:
            log_error(logger, "Error al recibir el codigo de operacion %s...", server_name);
            return;

        default:
            log_error(logger, "Alguno error inesperado %s", server_name);
            return;
        }
    }

    log_warning(logger, "Conexion cerrada %s", server_name);
    return;
}

int server_listen(t_log *logger, const char *server_name, int server_socket)
{
    int client_socket = wait_client(logger, server_name, server_socket);
    log_info(logger, "listening...");

    if (client_socket != -1)
    {
        pthread_t thread;

        t_process_conection_args *args = malloc(sizeof(t_process_conection_args));

        args->logger = logger;
        int server_listen(t_log * logger, char *server_name, int server_socket)
        {
            int client_socket = wait_client(logger, server_name, server_socket);
            log_info(logger, "listening...");

            if (client_socket != -1)
            {
                pthread_t thread;

                t_process_conection_args *args = malloc(sizeof(t_process_conection_args));

                args->logger = logger;
                args->server_name = server_name;
                args->fd = client_socket;

                pthread_create(&thread, NULL, (void *)process_conection, (void *)args);
                pthread_detach(thread);

                return 1;
            }

            return 0;
        }
        args->server_name = server_name;

        pthread_create(&thread, NULL, (void *)process_conection, (void *)args);
        pthread_detach(thread);

        return 1;
    }

    return 0;
}