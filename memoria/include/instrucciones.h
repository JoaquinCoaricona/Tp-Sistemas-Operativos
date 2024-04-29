#ifndef MAIN_H_
#define MAIN_H_
#include "../include/utils.h"
#include <commons/collections/queue.h>

typedef struct {
	int opcode_lenght;
	char* opcode;
	int parametro1_lenght;
	int parametro2_lenght;
	int parametro3_lenght;
	char* parametros[3];

}t_instruccion_unitaria;

#endif /* MAIN_H_ */