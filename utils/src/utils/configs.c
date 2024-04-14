#include "configs.h"

t_config *initialize_config(t_log *logger, char *path)
{
    t_config *new_config = NULL;

    new_config = config_create(path);

    if (new_config == NULL)
    {
        log_error(logger, "Error al crear el archivo de configuracion");
        exit(1);
    }

    log_info(logger, "Archivo de configuracion creado correctamente");

    return new_config;
}

void end_program(t_log *logger, t_config *config, int conection)
{
    log_destroy(logger);
    config_destroy(config);
    close_conection(conection);
}