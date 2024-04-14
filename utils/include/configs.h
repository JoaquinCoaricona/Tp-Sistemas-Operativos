#ifndef CONFIGS_H
#define CONFIGS_H

// INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include "readline/readline.h"

// Functions
t_config *initialize_config(t_log *logger, char *name);
void end_program(t_log *logger, t_config *config, int conection);

#endif // CONFIGS_H