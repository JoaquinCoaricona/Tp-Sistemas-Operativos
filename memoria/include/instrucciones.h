#ifndef MAIN_H_
#define MAIN_H_
#include "../include/utils.h"
#include <commons/collections/queue.h>



extern t_queue *queue_instrucciones;
typedef struct
{
int pid;
char* path;
}t_instrucciones;

typedef struct {
	int opcode_lenght;
	char* opcode;
	int parametro1_lenght;
	int parametro2_lenght;
	int parametro3_lenght;
	char* parametros[3];

}t_instruccion_unitaria;

#endif /* MAIN_H_ */