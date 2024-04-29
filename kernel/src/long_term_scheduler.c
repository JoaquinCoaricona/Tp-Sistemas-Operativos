#include "long_term_scheduler.h"

//Semaforo
sem_t m_execute_process;
sem_t short_term_scheduler_semaphore;
sem_t sem_ready;
sem_t m_ready_queue;
sem_t sem_hay_pcb_esperando_ready; //esto es para contar los PCB de ready
sem_t sem_multiprogramacion; //hay que inicializarlo en 0
pthread_mutex_t mutex_state_new;
pthread_mutex_t mutex_state_ready;

//Estado EXEC y BLOCKED no usan queue
t_queue* queue_new;
t_queue* queue_ready;
t_queue* queue_exit;
t_queue* queue_block;

//FIFO, RR, VRR
char* scheduler_algorithm;

//Asignar valores principales a colas y semaforos
void initialize_queue_and_semaphore() {
    queue_new = queue_create();
    queue_ready = queue_create();
		queue_block = queue_create(); //Cola de procesos bloqueados
    queue_exit = queue_create();
    sem_init(&m_execute_process, 0, 1);
    sem_init(&sem_ready, 0, 0);
		sem_init(&short_term_scheduler_semaphore, 0, 0);
    sem_init(&sem_hay_pcb_esperando_ready,0,0);
    sem_init(&sem_multiprogramacion,0,0);//aca hay que poner en el segundo cero el grado de multipprogramacion
    sem_init(&m_ready_queue, 0, 1);
    pthread_mutex_init(&mutex_state_new, NULL);

}

//Creacion de Proceso
t_pcb create_process() { //Bruno

	//TODO: Asignar PID
	//TODO: Reservar espacio para estructuras (codigo, datos, pila, heap)
	//TODO: Inicializar PCB
	//TODO: Ubicar pcb en listas de planificacion


	pid_t PID = fork();
	t_pcb* PCB = initializePCB(PID);
	
	if (PCB->pid == 0) 
	{
		//Codigo del hijo 
	} else if (PCB->pid > 0) 
	{
		//Codigo del padre
	} else 
	{
		//Error
	}


	//Asginar el PID
	//Reservar espacio para estructuras (codigo, datos, pila, heap)
	//Inicializar PCB
	//Ubicar pcb en listas de planificacion



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