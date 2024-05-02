#include "interfaz_generica.h"

// Levantar interfaz generica
void initialize_kernel_connection(kernel_socket)
{
    char *PORT_kernel;
    char *IP_kernel;
    int operation_code;
    t_list *sleepTime
    t_buffer *buffer;
    t_packet *packet;
    t_log *logger;

    // Config
    logger = initialize_logger("entradasalida.log", "entradasalida", true, LOG_LEVEL_INFO);
    t_config *config = initialize_config(logger, "entradasalida.config");

    PORT_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    IP_kernel = config_get_string_value(config, "IP_KERNEL");

    // Conect to Kernel
    while (1)
    {
        operation_code = fetch_codop(kernel_socket);
        switch (operation_code)
        {
        case KERNEL_A_IO:
            sleepTime = fetch_packet(kernel_socket);
            //usleep(tiempo);
        break;

        case -1:
            log_error(logger, "Error al recibir el codigo de operacion");
            close_conection(kernel_socket);

            return;

        default:
            log_error(logger, "Algun error inesperado ");
            close_conection(kernel_socket);
            return;
        }
    }
}

// Espero que me mande un mensaje (operación a realizar)
// TODO: Atiendo el mensaje (realizo la operación)
// TODO: Le respondo que terminé.
// TODO: Vuelvo al paso 2


