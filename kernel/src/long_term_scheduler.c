#include "long_term_scheduler.h"

//Semaforo
sem_t m_execute_process;
sem_t short_term_scheduler_semaphore;
sem_t long_term_scheduler_semaphore;
sem_t sem_ready;
sem_t m_ready_queue;
sem_t sem_hay_pcb_esperando_ready; //esto es para contar los PCB de ready
sem_t sem_multiprogramacion; //hay que inicializarlo en 0
pthread_mutex_t mutex_state_new;
pthread_mutex_t mutex_state_ready;
pthread_mutex_t m_pid_evicted;
pthread_mutex_t m_short_term_scheduler;

t_pcb* current_executing_process; 

//Estado EXEC y BLOCKED no usan queue
t_queue* queue_new;
t_queue* queue_ready;
t_queue* queue_ready_with_priority;
t_queue* queue_exit;
t_queue* queue_block;

//proceso en ejecuccion
t_pcb* executing_process;

//FIFO, RR, VRR
char* scheduler_algorithm;

//Asignar valores principales a colas y semaforos
void initialize_queue_and_semaphore() {
    queue_new = queue_create();
    queue_ready = queue_create();
	queue_ready_with_priority = queue_create();
	queue_block = queue_create(); //Cola de procesos bloqueados
    queue_exit = queue_create();

    sem_init(&m_execute_process, 0, 1);
	sem_init(&short_term_scheduler_semaphore, 0, 0);
	sem_init(&long_term_scheduler_semaphore, 0, 0);
    sem_init(&sem_ready, 0, 0);
    sem_init(&m_ready_queue, 0, 1);
    sem_init(&sem_hay_pcb_esperando_ready,0,0);
    sem_init(&sem_multiprogramacion,0,0);//aca hay que poner en el segundo cero el grado de multipprogramacion
    pthread_mutex_init(&mutex_state_new, NULL);
	pthread_mutex_init(&mutex_state_ready, NULL);
	pthread_mutex_init(&m_pid_evicted, NULL);
	pthread_mutex_init(&m_short_term_scheduler, NULL);


}



void long_term_scheduler() {
	sem_wait(&long_term_scheduler_semaphore);

	while(1) {
		pthread_mutex_lock(&mutex_state_ready);
		pthread_mutex_lock(&mutex_state_new);
		sem_wait(sem_multiprogramacion);
		if(queue_size(queue_new) > 0 && queue_size(queue_ready) < grado_multiprogramacion) {
			//Si existe proceso en NEW y el grado de multiprogramacion no es lleno
			t_pcb * process_to_ready = queue_pop(queue_new);
			queue_push(queue_ready, process_to_ready);

		}
		sem_post(sem_multiprogramacion);
		pthread_mutex_unlock(&mutex_state_new);
		pthread_mutex_unlock(&mutex_state_ready);
	}
	sem_post(&long_term_scheduler_semaphore);
}

//Ingresar a NEW
void enterNew(t_pcb *pcb) //t_log *logger
{
	pthread_mutex_lock(&mutex_state_new);
	queue_push(queue_new,pcb);
	pthread_mutex_unlock(&mutex_state_new);

	//log_info(logger, "Se agrega el proceso:%d a new", pcb->pid);
	//sem_post(&sem_hay_pcb_esperando_ready); SEMAFORO CONTADOR
}

//saca uno de NEW y lo devuelve, que seria el que iria a READY
t_pcb *obtenerSiguienteAready()
{       t_pcb *pcb = NULL;
        pthread_mutex_lock(&mutex_state_new);
    	pcb = queue_pop(queue_new);
    	pthread_mutex_unlock(&mutex_state_new);
    	return pcb;
}

void addEstadoReady(t_pcb *pcb){
     pthread_mutex_lock(&mutex_state_ready);
	 queue_push(queue_ready,pcb);
	 pthread_mutex_unlock(&mutex_state_ready);

	//log_info(logger, "Se agrega el proceso:%d a ready", pcb->pid);
}

void Aready()
{
	while (1)
	{
    	sem_wait(&sem_hay_pcb_esperando_ready); //controla que haya pcbs en ready
    	sem_wait(&sem_multiprogramacion);//controla que se cumpla con los hilos de multiprogramacion, se va restando hasta
        //que llegue a 0 y ahi se bloquea y el signal lo haces cuando un proceso finaliza 

     	//log_info(logger, "Grado de multiprogramación permite agregar proceso a ready\n");

    	t_pcb *pcb = obtenerSiguienteAready();

    	addEstadoReady(pcb);
    	//log_info(logger, "Se elimino el proceso %d de New y se agrego a Ready", pcb->id);

    	sem_post(&sem_ready); //esto indicaria que hay uno mas en ready pero hay que hacer la declaracion del semaforo
	}
}