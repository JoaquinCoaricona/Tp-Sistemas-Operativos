#include "main.h"

int main(int argc, char *argv[])
{
    char *PUERTO_ESCUCHA;
    int MEM_SIZE;
    int PAGE_SIZE;
    char IP[10] = "127.0.0.1";
    t_log *logger;

    // Initialize logger
    logger = initialize_logger("memoria.log", "memoria", true, LOG_LEVEL_INFO);

    log_info(logger, "Logger working");
    // Get config values
    t_config *config = initialize_config(logger, "memoria.config");


    PUERTO_ESCUCHA = config_get_string_value(config, "PUERTO_ESCUCHA");
    printf("PUERTO_ESCUCHA: %s\n", PUERTO_ESCUCHA); //!

    // Create socket
    int server_fd = initialize_server(logger, "memory_server", IP, "8002");
    printf("hola\n");
    int client_fd = wait_client_threaded(logger, "memory_server", server_fd, handshake(logger));
    printf("hola\n");
    close_connection(client_fd);
    log_destroy(logger);
    config_destroy(config);

    return 0;
}

void *handshake(t_log *logger)
{
    log_info(logger, "Handshake started");
}