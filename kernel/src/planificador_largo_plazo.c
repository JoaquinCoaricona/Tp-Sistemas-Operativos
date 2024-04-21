#include "planificador_largo_plazo.h"


//Estado EXEC y BLOCKED no usan queue
t_queue* cola_new;
t_queue* cola_ready;
t_queue* cola_exit;

//FIFO, RR, VRR
char* algoritmo_planificacion;

//Asignar valores principiantes a colas y semaforos
void inicializar_colas_y_semaforos() {
    cola_new = queue_create();
    cola_ready = queue_create();
    cola_exit = queue_create();

}