#include "main.h"

int main(int argc, char *argv[])
{
    char *PORT;
    int MEM_SIZE;
    int PAGE_SIZE;
    char *IP;
    t_log *logger;

    // Initialize logger
    logger = initialize_logger("memoria.log", "memoria", true, LOG_LEVEL_INFO);

    log_info(logger, "Logger working");
    // Get config values
    t_config *config = initialize_config(logger, "../memoria.config"); // TODO: Arreglar path en makefiles

    PORT = config_get_string_value(config, "PUERTO_ESCUCHA");
    IP = config_get_string_value(config, "IP");

    // Create socket
    int server_fd = initialize_server(logger, "memory_server", IP, PORT);
    printf("hola\n");
    int client_fd = wait_client(logger, "memory_server", server_fd);
    printf("hola\n");
    // log_destroy(logger);
    // config_destroy(config);

    if (fetch_codop(client_fd) == HANDSHAKE)
    {
        log_info(logger, "Handshake successful");
    }
    else
    {
        log_error(logger, "Handshake failed");
    }

    // Receive packet
    t_list *packet = fetch_packet(client_fd);
    log_info(logger, "Packet received");

    //close_conection(client_fd);
    end_program(logger, config, client_fd);
    return 0;
}

void *handshake(t_log *logger)
{
    log_info(logger, "Handshake started");
}
void end_program(t_log *logger, t_config *config, int conection)
{
    log_destroy(logger);
    config_destroy(config);
    close_conection(conection);
}