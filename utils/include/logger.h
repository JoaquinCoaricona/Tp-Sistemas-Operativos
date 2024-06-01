/**
 * @file logger.h 
 * @author KernelCrafters 
 * @brief Logger Functions to log messages
 * @version 1.0
 * @date 2024-04-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */

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

/**
 * @brief Creates a logger from the commons library to log messages
 * 
 * @param logger_name Name of the logger
 * @param process_name Name of the process
 * @param visible True if visible or false otherwise
 * @param log_level log level defined in t_log_level
 * @return t_log* 
 */
t_log *initialize_logger(char *logger_name, char *process_name, bool visible, t_log_level log_level);

#endif /* LOGGER_H_ */