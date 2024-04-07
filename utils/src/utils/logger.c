#include "logger.h"

t_log *initialize_logger(char *logger_name, char *process_name, bool visible, t_log_level log_level)
{
    t_log *new_logger;

    new_logger = log_create(logger_name, process_name, visible, log_level);

    if (new_logger == NULL)
    {
        printf("Could not create logger\n");
        exit(1);
    }

    return new_logger;
}