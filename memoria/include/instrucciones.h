#ifndef INSTRUCCIONES_H_
#define INSTRUCCIONES_H_
#include "../include/utils.h"
#include <commons/collections/queue.h>
#include "../../utils/include/protocol.h" estas dos lineas estaban aca pero no se porque funciona al sacarlas


void leer_pseudo();
void fetch_instruccion(int client_socket,t_instrucciones *instRec); 
typedef struct {
	int opcode_lenght;
	char* opcode;
	int parametro1_lenght;
	int parametro2_lenght;
	int parametro3_lenght;
	char* parametros[3];

}t_instruccion_unitaria;

extern char *PATH_CONFIG;
extern t_queue* queue_instrucciones;

#endif /* MAIN_H_ */