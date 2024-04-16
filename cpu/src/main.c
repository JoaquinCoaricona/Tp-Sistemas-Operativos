#include "main.h"
// #include "../include/utils.h"

int main(int argc, char *argv[])
{
    char *memory_PORT;
    char *dispatch_PORT;

    char *memory_IP;
    t_buffer *buffer;
    t_packet *packet;

    // LOGGER
    t_log *logger;
    logger = initialize_logger("cpu.log", "cpu", true, LOG_LEVEL_INFO);

    // CONFIG
    t_config *config = initialize_config(logger, "cpu.config");
    memory_PORT = config_get_string_value(config, "PUERTO_MEMORIA");
    dispatch_PORT = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
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

    int server_fd = initialize_server(logger, "cpu_server", memory_IP, dispatch_PORT);
    log_info(logger, "Server initialized");

    while (1){
        server_listen(logger, "cpu_server", server_fd);
        
    }

    return 0;
}