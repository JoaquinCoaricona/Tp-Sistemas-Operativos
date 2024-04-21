#ifndef PLANIFICADOR_LARGO_PLAZO_H_
#define PLANIFICADOR_LARGO_PLAZO_H_

#include <commons/collections/queue.h>

extern t_queue* cola_new;
extern t_queue* cola_ready;
extern t_queue* cola_exit;
extern char* algoritmo_planificacion;

void inicializar_colas_y_semaforos();

#endif // PLANIFICADOR_LARGO_PLAZO_H_