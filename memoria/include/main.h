#ifndef MAIN_H_
#define MAIN_H_

//#include "../include/utils.h"
#include "instrucciones.h"
#include <math.h>


typedef struct{
	int pid;
	int numero_marco;
	bool esLibre;
	int posicion_inicio_marco;
}t_situacion_marco;

//Esta estructura la ahorro usando un t_diccionario
// typedef struct{
// 	int pid;
// 	t_list tablaDePaginas;
// }t_pidTabladePaginas;

typedef struct{
	int numeroMarco;
	int bitValidez;
}t_paginaMarco;

extern char *PATH_CONFIG;
extern void *espacioUsuario;
extern t_list *situacionMarcos;
extern t_dictionary *tabla_paginas_por_PID;
extern t_log *logger;
extern int memoriaDisponible;
extern int cantidadMarcos;
extern int memoriaTotal;
extern int tamaPagina;

void resizePaginas(int client_socket);
void ampliarProceso(int pid, int cantidadAgregar);
void reducirProceso(int pid, int cantidadReducir);
bool esMarcoLibre(void* args);
t_situacion_marco* buscarMarcoLibre();

#endif /* MAIN_H_ */