#ifndef MAIN_H_
#define MAIN_H_

#include "../include/utils.h"
#include "../include/opcode.h"


void* manage_interrupt_request(void *args);
void manage_dispatch_request();
void pedirInstruccion(int pid, int pc,int client_fd);
typedef struct {
	int opcode_lenght;
	char* opcode;
	int parametro1_lenght;
	int parametro2_lenght;
	int parametro3_lenght;
	int parametro4_lenght;
	int parametro5_lenght;
	char* parametros[5];

}t_instruccion_unitaria;

#endif /* MAIN_H_ */
