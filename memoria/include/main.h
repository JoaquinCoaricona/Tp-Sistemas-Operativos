#ifndef MAIN_H_
#define MAIN_H_

//#include "../include/utils.h"
#include "instrucciones.h"

extern t_log *logger;
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



#endif /* MAIN_H_ */