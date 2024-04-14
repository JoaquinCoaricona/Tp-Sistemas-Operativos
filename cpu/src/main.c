#include "main.h"
// #include "../include/utils.h"

int main(int argc, char *argv[])
{
    char *PORT;
    char *IP;
    t_buffer *buffer;
    t_packet *packet;

    // LOGGER
    t_log *logger;
    logger = initialize_logger("cpu.log", "cpu", true, LOG_LEVEL_INFO);

    // CONFIG
    t_config *config = initialize_config(logger, "cpu.config");
    PORT = config_get_string_value(config, "PUERTO_MEMORIA");
    IP = config_get_string_value(config, "IP_MEMORIA");

    // Conect to server
    int client_fd = create_conection(logger, IP, PORT);
    log_info(logger, "Conectado al servidor de memoria %s:%s", IP, PORT);

    // Send handshake
    buffer = create_buffer();
    packet = create_packet(HANDSHAKE, buffer);

    add_to_packet(packet, buffer->stream, buffer->size);
    send_packet(packet, client_fd);

    log_info(logger, "Handshake enviado");

    // TODO: serialize_packet(packet, buffer->size);

    return 0;
}
