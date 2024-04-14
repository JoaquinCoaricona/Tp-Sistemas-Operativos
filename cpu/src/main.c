#include "main.h"
// #include "../include/utils.h"

int main(int argc, char *argv[])
{
    char *PORT;
    char *IP;

    t_log *logger;
    logger = initialize_logger("cpu.log", "cpu", true, LOG_LEVEL_INFO);

    printf("Prueba desde CPU\n");
    log_info(logger, "Logger working");
    t_config *config = initialize_config(logger, "cpu.config");

    PORT = config_get_string_value(config, "PUERTO_MEMORIA");
    IP = config_get_string_value(config, "IP_MEMORIA");

    int client_fd = create_conection(logger, IP, PORT);

    t_buffer *buffer = create_buffer();
    t_packet *packet = create_packet(HANDSHAKE, buffer);
    // serialize_packet(packet, buffer->size);
    add_to_packet(packet, buffer->stream, buffer->size);
    send_packet(packet, client_fd);

    return 0;
}
