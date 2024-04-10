#ifndef UTILS_LOGGER_H_
#define UTILS_LOGGER_H_ 

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>

t_log *initialize_logger(char *logger_name, char *process_name, bool visible, t_log_level log_level);

#endif /* LOGGER_H_ */