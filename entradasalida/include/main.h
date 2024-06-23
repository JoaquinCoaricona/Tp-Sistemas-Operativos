#ifndef MAIN_H_
#define MAIN_H_

#include "../include/utils.h"
#include <sys/mman.h>

typedef struct {
    char* nombre_archivo;
    int bloque_inicial;
    int tamano;
}t_metadata;


#endif /* MAIN_H_ */