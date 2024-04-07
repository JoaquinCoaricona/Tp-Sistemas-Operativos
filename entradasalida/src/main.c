#include <utils/hello.h>
#include "main.h"
#include "utils/logger.c"


int main(int argc, char *argv[])
{
    t_log *logger;
    logger = initialize_logger("entradasalida.log", "entradasalida", true, LOG_LEVEL_INFO);

    decir_hola("una Interfaz de Entrada/Salida");
    printf("Prueba desde una Interfaz de Entrada/Salida\n");
    log_info(logger, "Logger working");

    return 0;
}
