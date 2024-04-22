#include "long_term_scheduler.h"


//Estado EXEC y BLOCKED no usan queue
t_queue* queue_new;
t_queue* queue_ready;
t_queue* queue_exit;

//FIFO, RR, VRR
char* scheduler_algorithm;

//Asignar valores principiantes a colas y semaforos
void initialize_queue_and_semaphore() {
    queue_new = queue_create();
    queue_ready = queue_create();
    queue_exit = queue_create();

}

//Creacion de Proceso
void create_process() {

}


//Finalizacion de Proceso
void end_process() {

}

//Planificar largo plazo
void long_term_scheduler() {

}