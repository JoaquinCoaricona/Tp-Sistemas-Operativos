/**
 * @file utils.h
 * @author your name (you@domain.com)
 * @brief Include all the headers from the project
 * @version 1.0
 * @date 2024-04-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef UTILS_H
#define UTILS_H

// INCLUDES
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <readline/readline.h>
#include "sockets.h"
#include "logger.h"
#include "configs.h"
#include "protocol.h"
#include "opcode.h"
#include "comunication.h"

// STRUCTS


//PCB
typedef struct {
    int pid;
    int program_counter;
    int quantum;
    registros_CPU* registros_CPU;

    char* proceso_estado; // pueden ser "NEW", "READY", "EXEC", "BLOCKED" y "EXIT"
	int64_t tiempo_llegada_ready;
	t_instruccion* instruccion;

} t_pcb;

//Registros
typedef struct {
    uint32_t PC; //Program Counter: Indica proxima instruccion a ejecutar
    uint8_t AX;
    uint8_t BX;
    uint8_t CX;
    uint8_t DX;
    uint32_t EAX;
    uint32_t EBX;
    uint32_t ECX;
    uint32_t EDX;
    uint32_t SI;
    uint32_t DI;
} registros_CPU;

//Instruccion
typedef struct {
    char* opcode;
	int opcode_length;
	int parametro1_length;
	int parametro2_length;
	int parametro3_length;
	char* parametros[3];

}t_instruccion;

#endif // UTILS_H