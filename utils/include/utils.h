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
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <string.h>
#include "sockets.h"
#include "logger.h"
#include "configs.h"
#include "protocol.h"
#include "opcode.h"
#include "comunication.h"

// STRUCTS
//extern t_queue *queue_instrucciones;


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
} t_cpu_registers;

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

//Process state
typedef enum {
    NEW,
    READY,
    EXEC,
    BLOCKED,
    EXIT
} t_process_state;

//PCB
typedef struct {
    pid_t pid; //! Process ID pid_t from "https://man7.org/linux/man-pages/man2/fork.2.html"
    int program_counter;
    int quantum;
    t_process_state state; // pueden ser "NEW", "READY", "EXEC", "BLOCKED" y "EXIT"
    t_cpu_registers registers;


    // int prueba;
    //int length_process_state;
	//int64_t tiempo_llegada_ready;
	//t_instruction* instruction;

} t_pcb;


typedef struct
{
int pid;
char* path;
t_list* lista_de_instrucciones;
}t_instrucciones;




#endif // UTILS_H