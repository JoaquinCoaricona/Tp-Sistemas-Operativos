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

/**
 * @brief Initializes the config functions of the Commons library to read the configuration files
 * 
 * @param logger logger from commons library
 * @param path path to the configuration file 
 * @return t_config* 
 */
t_config *initialize_config(t_log *logger, char *path);

/**
 * @brief Destroy config and logger
 * 
 * @param logger logger created by initialize_logger
 * @param config config var created by intialize_config
 */
void end_program(t_log *logger, t_config *config);

#endif // CONFIGS_H