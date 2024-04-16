#include <utils/hello.h>
#include "main.h"
#include "utils/logger.c"

int main(int argc, char *argv[])
{
    t_log *logger;

    // Intialize logger
    logger = initialize_logger("kernel.log", "kernel", true, LOG_LEVEL_INFO);

    decir_hola("Kernel");
    printf("Esto es una prueba \n");

    log_info(logger, "Logger working");
    log_destroy(logger);
    return 0;

    

}

