#include <utils/hello.h>
#include "main.h"
#include "utils/logger.c"

int main(int argc, char *argv[])
{
    t_log *logger;
    logger = initialize_logger("cpu.log", "cpu", true, LOG_LEVEL_INFO);


    decir_hola("CPU");
    printf("Prueba desde CPU\n");
    log_info(logger, "Logger working");

    return 0;
}
