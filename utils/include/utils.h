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
} cpu_registers;

//Instruccion
typedef struct {
    char* opcode;
	int opcode_length;
	int parameter1_length;
	int parameter2_length;
	int parameter3_length;
    int parameter4_length;
    int parameter5_length;
	char* parameters[5];

}t_instruction;


//PCB
typedef struct {
    int pid;
    int program_counter;
    int quantum;
    cpu_registers* cpu_registers;

    char* process_state; // pueden ser "NEW", "READY", "EXEC", "BLOCKED" y "EXIT"
	int64_t tiempo_llegada_ready;
	t_instruction* instruction;

} t_pcb;
#endif // UTILS_H