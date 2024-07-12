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

typedef struct{
	int numeroPagina;
}t_registroTLB; //Esto es para limpiar la TLB, no es un registro TLB como tal

bool esMarcoLibre(void* args);
void ampliarProceso(int pid, int cantidadAgregar);
void limpiarRegistrosTLB(void *registro);
t_situacion_marco* buscarMarcoLibre();
bool encontrarPidALiberar(void *registrado);
void destroy_instuccion_unitaria(t_instruccion_unitaria *instruccion);
void destroyPaginasYLiberarMarco(void *pagina);

#endif /* MAIN_H_ */