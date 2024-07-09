#include "long_term_scheduler.h"

//Semaforo
sem_t m_execute_process;
sem_t short_term_scheduler_semaphore;
sem_t sem_ready;
sem_t m_ready_queue;
sem_t sem_hay_pcb_esperando_ready; //esto es para contar los PCB de ready
sem_t sem_multiprogramacion; //hay que inicializarlo en 0
pthread_mutex_t mutex_state_exit;
pthread_mutex_t mutex_state_new;
pthread_mutex_t mutex_state_ready;
pthread_mutex_t mutex_state_prioridad;

pthread_mutex_t m_planificador_corto_plazo;
pthread_mutex_t m_planificador_largo_plazo;
pthread_mutex_t procesosBloqueados;
pthread_mutex_t m_procesoEjectuandoActualmente;

// Semaforos de asignacion de recursos
pthread_mutex_t m_recursos_asignados;
pthread_mutex_t m_recursos_pendientes;


//Estado EXEC y BLOCKED no usan queue
t_queue* queue_new;
t_queue* queue_ready;
t_queue* queue_prioridad;
t_queue* queue_exit;

t_dictionary *recursos_bloqueados;
//t_queue* queue_block;

//FIFO, RR, VRR
char* scheduler_algorithm;

//Asignar valores principales a colas y semaforos
void initialize_queue_and_semaphore() {


    queue_new = queue_create();
    queue_ready = queue_create();
	//queue_block = queue_create(); //Cola de procesos bloqueados
    queue_exit = queue_create();
	queue_prioridad = queue_create();
    sem_init(&m_execute_process, 0, 1);
    sem_init(&sem_ready, 0, 0);
	sem_init(&short_term_scheduler_semaphore, 0, 0);
    sem_init(&sem_hay_pcb_esperando_ready,0,0);
    sem_init(&sem_multiprogramacion,0,gradoMultiprogramacion);//aca hay que poner en el segundo cero el grado de multipprogramacion
    sem_init(&m_ready_queue, 0, 1);
    pthread_mutex_init(&mutex_state_new, NULL);
    pthread_mutex_init(&mutex_state_ready, NULL);
    pthread_mutex_init(&mutex_state_exit, NULL);
	pthread_mutex_init(&mutex_state_prioridad, NULL);
    pthread_mutex_init(&m_planificador_corto_plazo, NULL);
	pthread_mutex_init(&m_planificador_largo_plazo, NULL);
	pthread_mutex_init(&m_procesoEjectuandoActualmente, NULL);
	pthread_mutex_init(&procesosBloqueados, NULL);
	pthread_mutex_init(&m_recursos_asignados, NULL);
    pthread_mutex_init(&m_recursos_pendientes, NULL);
}



void long_term_scheduler() {

}

//Ingresar a NEW
void agregarANew(t_pcb *pcb) //t_log *logger
{
	pthread_mutex_lock(&mutex_state_new);
	queue_push(queue_new,pcb);
	pthread_mutex_unlock(&mutex_state_new);

	log_info(logger, "Se agrega el proceso: %d a new \n", pcb->pid);
	sem_post(&sem_hay_pcb_esperando_ready); //SEMAFORO CONTADOR
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

	log_info(logger, "Se agrega el proceso: %d a ready \n", pcb->pid);
	//char* listado_pid = get_pid_lidt(queue_ready);
	log_info(logger, "Cola Ready: %s \n", get_pid_lidt(queue_ready));

}



void *Aready(void *arg)
{
	while (1)
	{
		
		//aca puede ser que haya problemas cuando haga lo de grado de multipgraoamcion
		//quizas haciendo un semaforoDespertar Planificador Corto PLlazo funciona
		sem_wait(&sem_multiprogramacion);//controla que se cumpla con los hilos de multiprogramacion, se va restando hasta
        //que llegue a 0 y ahi se bloquea y el signal lo haces cuando un proceso finaliza 
		
		//lo doy vuelta porque pasa lo mismo que pasaba con FIFO y VRR y porque los di vuelta ahi  
		sem_wait(&sem_hay_pcb_esperando_ready); //controla que haya pcbs esperando entrar ready
    	
		pthread_mutex_lock(&m_planificador_largo_plazo);

		/*IMPORTANTE: Aca lo que hice fue lo mismo que en el shorttermScheduler, saque el wait m_planificador_largo_plazo
		que habia en caso de matar un proceso que este en new(esto puede generar que el semaforo
		hay pcb esperando ready no este actualizado y deje pasar aun si new esta vacio), entonces 
		agregue el control en caso que new este vacio lo que hace es desbloquea el semaforo ready que
		se usa para el chqequeo y ademas del log tambien le hace un post al grado de multiprogramacion
		que desperdicio para poder entrar aca y que la cola este vacia. Como eso no lo recupera
		le hago un post para que supere ese semaforo y quede trabado en sem_hay_pcb_esperando_ready 
		esperando que entren procesos a new, tambien hago el unlcokk del planificaodo largo plazo
		para que pueda detener la planificacion. En caso que haya entrado bien y la cola de new tenga
		elementos, entonces se hace lo que se hacia normalmente aca. Esto generaba problemas
		y se colgaba la consola porque hacia el wait en finalizar proceso si estaba en new.
		Pasaba algo parecido a lo que esta detallado en el planificador de corto plazo.
		*/
		//Un error que pareceria que podria tener es que cuando yo detengo la pllanificacion
		// y cargo dos procesos por ejemplo en new y borro los dos, entonces pareceria que 
		// si solo le hago un post al grado de multiporgramacion y una sola vez llega a entrar
		// por cola new vacia entonces se perderia un grado de multiprogramacion pero si
		// haces el seguimiento pensandolo es como que de la nada aparece con los dos 
		// lugares en sem multiprogramacion y no los pierde. Digo de la nada pero no es 
		// que no se como lo hace, hay que hacer el seguimiento pero los recupera. Funciona


		pthread_mutex_lock(&mutex_state_ready);
		
		if(queue_size(queue_new) == 0){
			
			pthread_mutex_unlock(&mutex_state_ready);
			
			log_info(logger,"Cola de New Vacia");
			sem_post(&sem_multiprogramacion);
			pthread_mutex_unlock(&m_planificador_largo_plazo);
		}else{
		
		pthread_mutex_unlock(&mutex_state_ready);
		log_info(logger, "Grado de multiprogramación permite agregar proceso a ready\n");
		
    	t_pcb *pcb = obtenerSiguienteAready();
		pcb->state = READY;
    	addEstadoReady(pcb);
    	log_info(logger, "Se elimino el proceso %d de New y se agrego a Ready", pcb->pid);

    	sem_post(&sem_ready); 
		
		pthread_mutex_lock(&m_procesoEjectuandoActualmente);
		if(procesoEjectuandoActualmente == -2){//si no hay nadie ejecutando
			procesoEjectuandoActualmente = -1;
			pthread_mutex_unlock(&m_procesoEjectuandoActualmente);

			log_info(logger, "Unico Llamado a Corto Plazo Desde Largo");
			sem_post(&short_term_scheduler_semaphore);
		}else{
			pthread_mutex_unlock(&m_procesoEjectuandoActualmente);
		}
		
		pthread_mutex_unlock(&m_planificador_largo_plazo);
		
		}
		
	}
}

char* get_pid_lidt(t_queue *queue){
	char* pids = string_new();
	t_list* list_queue = queue->elements;

    void concatenar_pids(void *args){
        t_pcb *pcb =(t_pcb *) args;

        if(string_length(pids)==0){
			string_append_with_format(&pids,"[ %d",pcb->pid);
		}else{
			string_append_with_format(&pids,", %d",pcb->pid);
		}
    }
    list_iterate(list_queue,concatenar_pids);
	string_append(&pids,"]");
	return pids;
}
void addEstadoExit(t_pcb *pcb){
	
	liberarRecursosProceso(pcb->pid);
    pthread_mutex_lock(&mutex_state_exit);
	queue_push(queue_exit,pcb);
	pthread_mutex_unlock(&mutex_state_exit);

	log_info(logger, "Se agrega el proceso: %d a Exit \n", pcb->pid);
}

//Agregar y Sacar PCB de Cola Prioridad

void addColaPrioridad(t_pcb *pcb){
    pthread_mutex_lock(&mutex_state_prioridad);
	queue_push(queue_prioridad,pcb);
	pthread_mutex_unlock(&mutex_state_prioridad);

	log_info(logger, "Se agrega el proceso: %d a la cola Prioridad", pcb->pid);

	log_info(logger, "Ready Prioridad: %s \n", get_pid_lidt(queue_prioridad));

}

t_pcb *obtenerSiguienteColaPrioridad(){
	    
	t_pcb *pcb = NULL;
    pthread_mutex_lock(&mutex_state_prioridad);
    pcb = queue_pop(queue_prioridad);
    pthread_mutex_unlock(&mutex_state_prioridad);
    log_info(logger, "Saco el proceso: %d de la cola Prioridad", pcb->pid);
	return pcb;
}



