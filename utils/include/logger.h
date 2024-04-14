#ifndef LOGGER_H_
#define LOGGER_H_

// INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>

// FUNCTIONS
t_log *initialize_logger(char *logger_name, char *process_name, bool visible, t_log_level log_level);

#endif /* LOGGER_H_ */