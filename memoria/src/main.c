#include "main.h"

int main(int argc, char *argv[])
{
    char *PORT;
    char *IP;
    int MEM_SIZE;
    int PAGE_SIZE;
    t_list *packet;
    t_log *logger;

    // LOGGER
    logger = initialize_logger("memoria.log", "memoria", true, LOG_LEVEL_INFO);

    // CONFIG
    t_config *config = initialize_config(logger, "../memoria.config"); // TODO: Arreglar path en makefiles

    PORT = config_get_string_value(config, "PUERTO_ESCUCHA");
    IP = config_get_string_value(config, "IP");

    // SERVER
    int server_fd = initialize_server(logger, "memory_server", IP, PORT);
    log_info(logger, "Server initialized");

    int client_fd = wait_client(logger, "memory_server", server_fd);
    // int client_fd = wait_client_threaded(logger, "memory_server", server_fd, handshake);

    log_info(logger, "Listening...");

    if (fetch_codop(client_fd) == HANDSHAKE)
    {
        log_info(logger, "Handshake successful");

        packet = fetch_packet(client_fd);
        log_info(logger, "Packet received");
    }
    else
    {
        log_error(logger, "Handshake failed");
    }

    end_program(logger, config, client_fd);

    return 0;
}

void *handshake(t_log *logger)
{
    log_info(logger, "Handshake started");
}

