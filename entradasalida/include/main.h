#ifndef MAIN_H_
#define MAIN_H_

#include "../include/utils.h"
#include <sys/mman.h> 
#include <stdbool.h>

bool existe_espacio_para_agrandar(t_bitarray *bitarray, int bloque_final, int cantidad_bloques_agrandar);
char* list_get_first(t_list *list);

#endif /* MAIN_H_ */