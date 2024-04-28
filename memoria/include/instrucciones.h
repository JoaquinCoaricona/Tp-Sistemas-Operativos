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


#endif /* MAIN_H_ */