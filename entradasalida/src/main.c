#include "main.h"

int socket_kernel;
int socket_memoria;

int main(int argc, char *argv[])
{
    char *PORT_memoria;
    char *IP_memoria;
    t_buffer *buffer;
    t_packet *packet;
    char *port_kernel;
    char *IP_kernel;

    t_log *logger;
    logger = initialize_logger("entradasalida.log", "entradasalida", true, LOG_LEVEL_INFO);

    printf("Prueba desde una Interfaz de Entrada/Salida\n");
    log_info(logger, "Logger working");
    t_config *config = initialize_config(logger, "entradasalida.config");

    PORT_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    IP_memoria = config_get_string_value(config, "IP_MEMORIA");

    PORT_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    IP_kernel = config_get_string_value(config, "IP_KERNEL");

    //Conect to server MEMORIA
    socket_memoria = create_conection(logger, IP_memoria, PORT_memoria);
    log_info(logger, "Conectado al servidor de memoria %s:%s", IP_memoria, PORT_memoria);

    //Send handshake
    buffer = create_buffer();
    packet = create_packet(HANDSHAKE_ENTRADA_SALIDA, buffer);
    add_to_packet(packet, buffer->stream, buffer->size);
    send_packet(packet, socket_memoria);
    log_info(logger, "Handshake enviado");

    socket_kernel = create_conection(logger, IP_kernel, PORT_kernel);
    log_info(logger, "Conectado al servidor de kernel %s:%s", IP_kernel, PORT_kernel);

    return 0;
}
