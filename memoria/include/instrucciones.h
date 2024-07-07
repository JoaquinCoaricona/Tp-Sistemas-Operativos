#ifndef INSTRUCCIONES_H_
#define INSTRUCCIONES_H_
#include "../include/utils.h"
#include "../../utils/include/protocol.h" //estas dos lineas estaban aca pero no se porque funciona al sacarlas
//----- Estas dos son del main de memoria pero las declaro aca para poder usarla en la funcion leer_pseudo
extern t_log *logger;
extern t_dictionary* tabla_paginas_por_PID;
//----------
void leer_pseudo();
void fetch_instruccion(int client_socket,t_instrucciones *instRec); 
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
extern int retardoRespuesta;
extern char *PATH_CONFIG;
extern t_list *listaINSTRUCCIONES;

#endif /* MAIN_H_ */