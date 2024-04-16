#include "main.h"

int memory_socket;
int cpu_socket;
int main(int argc, char *argv[])
{

    char *memory_PORT;
    char *cpu_PORT;
    char *kernel_PORT;
    char *memory_IP;
    char *cpu_IP;
    char *kernel_IP;
    t_buffer *buffer;
    t_packet *packet;

    // LOGGER
    t_log *logger;
    logger = initialize_logger("kernel.log", "kernel", true, LOG_LEVEL_INFO);

    // CONFIG
    t_config *config = initialize_config(logger, "kernel.config");
    memory_PORT = config_get_string_value(config, "PUERTO_MEMORIA");
    cpu_PORT = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
    kernel_PORT = config_get_string_value(config, "PUERTO_KERNEL");
    cpu_IP = config_get_string_value(config, "IP_CPU");
    memory_IP = config_get_string_value(config, "IP_MEMORIA");
    kernel_IP = config_get_string_value(config, "IP_MEMORIA");// hay que ver si se tiene que cambiar a futuro pero en el archivo de configuración no hay una ip de kernel
    // Conect to server
    memory_socket = create_conection(logger, memory_IP, memory_PORT);
    log_info(logger, "Conectado al servidor de memoria %s:%s", memory_IP, memory_PORT);

    // Send handshake
    buffer = create_buffer();
    packet = create_packet(HANDSHAKE_KERNEL,buffer);

    add_to_packet(packet, buffer->stream, buffer->size);
    //packet = serialize_packet(packet, buffer->size);
    send_packet(packet, memory_socket);

    log_info(logger, "Handshake enviado");

    cpu_socket = create_conection(logger, cpu_IP, cpu_PORT);
    log_info(logger, "Conectado al servidor de cpu %s:%s", cpu_IP, cpu_PORT);

    send_packet(packet, cpu_socket);


    int server_fd = initialize_server(logger, "kernel_server", kernel_IP, kernel_PORT);
    log_info(logger, "Server initialized");

    while (1){
        server_listen(logger, "kernel_server", server_fd);
        
    }

    return 0;

    

}

