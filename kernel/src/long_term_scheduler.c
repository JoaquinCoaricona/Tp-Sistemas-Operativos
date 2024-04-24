#include "long_term_scheduler.h"

//Semaforo
pthread_mutex_t mutex_state_new;
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
    pthread_mutex_init(&mutex_estado_new, NULL);

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

//Ingresar a NEW
void enterNew(t_pcb *pcb,t_log *logger)
{
	pthread_mutex_lock(&mutex_state_new);
	queue_push(queue_new,pcb);
	pthread_mutex_unlock(&mutex_state_new);

	log_info(logger, "Se agrega el proceso:%d a new", pcb->pid);
	//sem_post(&sem_hay_pcb_esperando_ready); SEMAFORO CONTADOR
}
