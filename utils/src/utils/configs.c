t_config *initialize_config(t_log *logger, char *name)
{
    t_config *new_config = NULL;

    new_config = config_create(name);

    if (nuevo_config == NULL)
    {
        log_error(logger, "Error al crear el archivo de configuracion");
        exit(1);
    }

    return new_config;
}

