#include "main.h"
// #include "../include/utils.h"

    t_log *logger;
int server_dispatch_fd;
int main(int argc, char *argv[])
{
    char *memory_PORT;
    char *dispatch_PORT;
    char *interrupt_PORT;

    char *memory_IP;
    t_buffer *buffer;
    t_packet *packet;

    // LOGGER
    logger = initialize_logger("cpu.log", "cpu", true, LOG_LEVEL_INFO);

    // CONFIG
    t_config *config = initialize_config(logger, "cpu.config");
    memory_PORT = config_get_string_value(config, "PUERTO_MEMORIA");
    dispatch_PORT = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
    interrupt_PORT = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");
    memory_IP = config_get_string_value(config, "IP_MEMORIA");

    // Conect to server
    int client_fd = create_conection(logger, memory_IP, memory_PORT);
    log_info(logger, "Conectado al servidor de memoria %s:%s", memory_IP, memory_PORT);

    // Send handshake
    buffer = create_buffer();
    packet = create_packet(HANDSHAKE_CPU, buffer);

    add_to_packet(packet, buffer->stream, buffer->size);
    //packet = serialize_packet(packet, buffer->size);
    send_packet(packet, client_fd);

    log_info(logger, "Handshake enviado");

    server_dispatch_fd = initialize_server(logger, "cpu_dispatch_server", memory_IP, dispatch_PORT);
    log_info(logger, "Server dispatch initialized");


    int server_interrupt_fd = initialize_server(logger, "cpu_interrupt_server", memory_IP, interrupt_PORT);
    log_info(logger, "Server interrupt initialized");

    pthread_t thread_memory_peticions;
    t_process_conection_args *process_conection_arguments= malloc(sizeof(t_process_conection_args));
    process_conection_arguments->server_name = "cpu_interrupt_server";
    process_conection_arguments->fd = server_interrupt_fd;

    pthread_create(&thread_memory_peticions,NULL,manage_interrupt_request,process_conection_arguments);
    pthread_detach(thread_memory_peticions);
    
    manage_dispatch_request();

    return 0;
}
//INTERUPT
void* manage_interrupt_request(void *args)
{
    int server_socket;
    char *server_name;
    t_packet *packet;

    t_process_conection_args *arguments = (t_process_conection_args *)args;

    // Pasa los arguments para poder crear el thread
    
    server_socket = arguments->fd;
    server_name = arguments->server_name;
   

    free(args);

    while (1)
    {
        int client_socket = wait_client(logger, server_name, server_socket);
        int operation_code = fetch_codop(client_socket);

        switch (operation_code)
        {
        case HANDSHAKE_KERNEL:
            log_info(logger, "handshake %d recibido %s",operation_code, server_name);
            packet = fetch_packet(client_socket);
            log_info(logger, "Packet received");

            close_conection(client_socket);
            client_socket = -1;
            break;
        case INTERRUPCION:
            //Manejar una interrupcion
            break;
        case -1:
            log_error(logger, "Error al recibir el codigo de operacion %s...", server_name);
            return;

        default:
            log_error(logger, "Algun error inesperado %s", server_name);
            return;
        }
    }

    log_warning(logger, "Conexion cerrada %s", server_name);
    return;
}
//DISPATCH
void manage_dispatch_request()
{
    t_packet *packet;
    while (1)
    {
        int client_socket = wait_client(logger, "cpu_dispatch_server", server_dispatch_fd);
        int operation_code = fetch_codop(client_socket);

        switch (operation_code)
        {
        case HANDSHAKE_KERNEL:
            log_info(logger, "handshake %d recibido %s",operation_code, "cpu_dispatch_server");
            packet = fetch_packet(client_socket);
            log_info(logger, "Packet received");

            close_conection(client_socket);
            client_socket = -1;
            break;
        case PETICION_CPU:
            //Manejar una peticion a CPU
            break;
        case PCB_REC:
            //Manejar una peticion a CPU
        break;
        case -1:
            log_error(logger, "Error al recibir el codigo de operacion %s...", "cpu_dispatch_server");
            return;

        default:
            log_error(logger, "Algun error inesperado %s", "cpu_dispatch_server");
            return;
        }
    }

    log_warning(logger, "Conexion cerrada %s", "cpu_dispatch_server");
    return;
}

