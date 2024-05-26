#include "short_term_scheduler.h"
int id_counter = 1;
//Para VRR
t_temporal *timer;
int ms_transcurridos;


//Planificador de Corto Plazo General
void *planificadorCortoPlazo(void *arg){

	while(1){
		if(string_equals_ignore_case(algoritmo_planificacion, "FIFO")){
			planificador_corto_plazo_FIFO();
		}else if(string_equals_ignore_case(algoritmo_planificacion, "RR")){
			planificador_corto_plazo_RoundRobin();
		}else if(string_equals_ignore_case(algoritmo_planificacion, "VRR")){
            planificador_corto_plazo_Virtual_RoundRobin();
        }else{
			//planificar_corto_plazo_round_robbin();
            printf("ERROR");
		}
	}

	return NULL;
}


//FIFO
void planificador_corto_plazo_FIFO() {      
    
    sem_wait(&short_term_scheduler_semaphore);//esto es para despertar al planificador de corto plazo
    pthread_mutex_lock(&m_planificador_corto_plazo);
    sem_wait(&sem_ready); 
    /*aca lo pongo en este orden porque antes estaba al reves, entonces lo que pasaba es que
    como la funcion planificador de corto plazo es un while(1) entonces apenas se enviaba el proceso
    desde esta funcion FIFO, volvia a ejecutarse la de planificador de corto plazo que 
    esta arriba de la de fifo y como es un while 1 directamente hacia el wait a el sem
    ready y entonces se quedaba esperando al signal de shorttercmscheduler pero si por
    ejemplo eliminaas un proceso lo que pasa es que se le hace un wait al sem ready
    que es un contador de cuantos hya en ready: en el caso que tenas dos procesos unno este ejecutando 
    y el otro en ready y entonces vos eliminabas el que estaba en ready, al volver el otro
    despertaba aal planiicador de cort plazo par que envie otro, y cuando vos eliminstas
    hiciste el wait del semaforocontador semready pero como el sem ready  ya habia sido
    pasado por lo que explique al principio solamente con el signal al depsrttar
    planifiacodr corto plazo ya pedias que manden otro a ejecutara pero no habia mas en ready
    por eso lo di vuelta, ahora primero espera desportar y despues controla que haya en ready
    para que el semaforo de ready cuando le toque decidir este actualizado y no pase lo del ejemplo
    que puse, que por mas que le haga el wait a sem ready no servia porque ya habia sido superado
    y estaban esperando solo el signal a despetar planificador de corto plazo. Esto
    sse usa en FIFO RR y VRR
    */   
    pthread_mutex_lock(&mutex_state_ready);
    t_pcb *proceso = queue_pop(queue_ready); //semaforo mutex para entrar a la lista de READY
    pthread_mutex_unlock(&mutex_state_ready);

    proceso->state = EXEC;
    log_info(logger, "Cambio De Estado Proceso %d a %d\n", proceso->pid,proceso->state);
    
    pthread_mutex_lock(&m_procesoEjectuandoActualmente);
    procesoEjectuandoActualmente = proceso ->pid;
    int hoa = procesoEjectuandoActualmente;
    pthread_mutex_unlock(&m_procesoEjectuandoActualmente);
    
    pcbEJECUTANDO = proceso;
    enviar_proceso_cpu(proceso);
    
    pthread_mutex_unlock(&m_planificador_corto_plazo);

 }


void manejoHiloQuantum(void *pcb){

    t_quantum *proceso = (t_quantum *)pcb;
    
    usleep(quantumGlobal);

    pthread_mutex_lock(&m_procesoEjectuandoActualmente);
    if(procesoEjectuandoActualmente == proceso->pid){
        log_info(logger,"Envio Interupcion %i",procesoEjectuandoActualmente);
        pthread_mutex_unlock(&m_procesoEjectuandoActualmente);
        char *motivo = "Fin de Quantum";
        enviarInterrupcion(motivo,proceso->pid);
    }else{
        log_info(logger, "El Proceso %i termino antes del Quantum",proceso->pid);
        pthread_mutex_unlock(&m_procesoEjectuandoActualmente);

    }
    //EL UNLOCK TIENE QUE ESTAR EN LOS DOS CASOS, PORQUE SINO QUEDA BLOQUEADO
    
    free(pcb);
    pthread_cancel(pthread_self());
   
}

//Round Robin
void planificador_corto_plazo_RoundRobin() {
    
    sem_wait(&short_term_scheduler_semaphore);//esto es para despertar al planificador de corto plazo
    pthread_mutex_lock(&m_planificador_corto_plazo);
    log_info(logger,"Bloqueo");
    sem_wait(&sem_ready); //aclaracion esto estaba dado vuelta antes, aclaracion hecha en fifo
    
    
    pthread_mutex_lock(&mutex_state_ready);
    t_pcb *proceso = queue_pop(queue_ready); //semaforo mutex para entrar a la lista de READY
    pthread_mutex_unlock(&mutex_state_ready);

    proceso->state = EXEC;
    log_info(logger, "Cambio De Estado Proceso %d a %i\n", proceso->pid,proceso->state);

    pthread_mutex_lock(&m_procesoEjectuandoActualmente);
    procesoEjectuandoActualmente = proceso->pid;
    pthread_mutex_unlock(&m_procesoEjectuandoActualmente);
    pcbEJECUTANDO = proceso; // aca guardo en el puntero global el PCB que se va enviar
    enviar_proceso_cpu(proceso); //envio el proceso
        
    pthread_t hiloQuantum;
    t_quantum *quatnumHilo = malloc(sizeof(t_quantum));
    quatnumHilo->pid = proceso->pid;
    //solo le paso el pid, no le paso el puntero al pcb
    //el puntero al pcb esta en pcbEJECUTANDO
    //Aca no es necesario pasarle un quantum porque hay un quantum Global

    pthread_create(&hiloQuantum,NULL,manejoHiloQuantum,quatnumHilo);
    pthread_detach(hiloQuantum);

    pthread_mutex_unlock(&m_planificador_corto_plazo);
}



void enviarInterrupcion(char *motivo, int pid){
   

    t_buffer *bufferINTERRUPCION;
    t_packet *packetINTERRUPCION;
    
    bufferINTERRUPCION = create_buffer();
    packetINTERRUPCION = create_packet(INTERRUPCION, bufferINTERRUPCION);
    add_to_packet(packetINTERRUPCION,motivo,(strlen(motivo)+1));
    add_to_packet(packetINTERRUPCION,&pid,sizeof(int));
    //si tengo una variable, le tengo que pasar la direccion a addpacket

    send_packet(packetINTERRUPCION, cpu_interrupt_socket);
    destroy_packet(packetINTERRUPCION);
}

void planificador_corto_plazo_Virtual_RoundRobin(){
    
    t_pcb *proceso = NULL;

    sem_wait(&short_term_scheduler_semaphore);//esto es para despertar al planificador de corto plazo
    pthread_mutex_lock(&m_planificador_corto_plazo);
    log_info(logger,"Bloqueo");
    sem_wait(&sem_ready); //aclaracion esto estaba dado vuelta antes, aclaracion hecha en fifo
    
    
    //Me fijo si hay algo en la cola prioridad
    log_info(logger,"Momento de Eleccion");
    pthread_mutex_lock(&mutex_state_prioridad);
        if(queue_size(queue_prioridad) != 0 ){
            log_info(logger,"Elijo Cola Prioridad");
            //Aca desbloqueo el semaoro de la cola prioridad
            pthread_mutex_unlock(&mutex_state_prioridad);
            proceso = obtenerSiguienteColaPrioridad();
            log_info(logger,"Quantum Restante: %i",proceso->quantum);
        }else{
            log_info(logger,"Elijo Cola Ready");
            //Aca desbloqueo el semaoro de la cola prioridad
            pthread_mutex_unlock(&mutex_state_prioridad);
            
            //No tengo funcion para esto, pero para la cola prioridad si
            pthread_mutex_lock(&mutex_state_ready);
            proceso = queue_pop(queue_ready); //semaforo mutex para entrar a la lista de READY
            pthread_mutex_unlock(&mutex_state_ready);        
            // le cargo el QuantumGlobal porque esta viniendo de la cola de ready y no de prioridad
            proceso->quantum = quantumGlobal;
        }
    


    proceso->state = EXEC;
    log_info(logger, "Cambio De Estado Proceso %d a %i\n", proceso->pid,proceso->state);

    pthread_mutex_lock(&m_procesoEjectuandoActualmente);
    procesoEjectuandoActualmente = proceso->pid;
    pthread_mutex_unlock(&m_procesoEjectuandoActualmente);
    pcbEJECUTANDO = proceso; // aca guardo en el puntero global el PCB que se va enviar
    enviar_proceso_cpu(proceso); //envio el proceso
    //Este timer es una variable global, aca como recien envie el proceso
    //lo empiezo a correr pero la voy a frenar ...CONTINUAR
    timer = temporal_create();

    pthread_t hiloQuantum;
    t_quantum *quatnumHilo = malloc(sizeof(t_quantum));
    quatnumHilo->pid = proceso->pid;
    quatnumHilo->quantum = proceso->quantum;
    //solo le paso el pid, no le paso el puntero al pcb
    //el puntero al pcb esta en pcbEJECUTANDO
    //Aca no es necesario pasarle un quantum porque hay un quantum Global

    pthread_create(&hiloQuantum,NULL,manejoHiloQuantumVRR,quatnumHilo);
    pthread_detach(hiloQuantum);

    pthread_mutex_unlock(&m_planificador_corto_plazo);
}

void manejoHiloQuantumVRR(void *pcb){
//Este es Exclusivo para el VRR pero es casi igual a RR
//La unica diferencia es que aca en el caso que sea de cola prioritaria 
//solo va a ejecutar lo que le faltaba y si es de cola ready le cargo el quantum original
//podria haber usado el mismo pero lo puse para tratar de evitar tocar el codigo
//que funciona 
    t_quantum *proceso = (t_quantum *)pcb;
    
    usleep(proceso->quantum);

    pthread_mutex_lock(&m_procesoEjectuandoActualmente);
    if(procesoEjectuandoActualmente == proceso->pid){
        log_info(logger,"Envio Interupcion %i",procesoEjectuandoActualmente);
        pthread_mutex_unlock(&m_procesoEjectuandoActualmente);
        char *motivo = "Fin de Quantum";
        enviarInterrupcion(motivo,proceso->pid);
    }else{
        log_info(logger, "El Proceso %i termino antes del Quantum",proceso->pid);
        pthread_mutex_unlock(&m_procesoEjectuandoActualmente);

    }
    //EL UNLOCK TIENE QUE ESTAR EN LOS DOS CASOS, PORQUE SINO QUEDA BLOQUEADO
    
    free(pcb);
    pthread_cancel(pthread_self());
   
}

void obtenerDatosTemporal(){
    //Detengo el Temporal y guardo el tiempo transcurrido en la variable global
    temporal_stop(timer);
    ms_transcurridos = temporal_gettime(timer);
    log_info(logger,"Tiempo Transcurrido: %i",ms_transcurridos * 1000); //Multiplico Por mil porque me interesan
    //los microsegundos que son los que usa usleep y no milisegundos que es lo que devuelve t_temporal
    temporal_destroy(timer);
}

void enviar_proceso_cpu(t_pcb *proceso){


    t_pcb PCBPRUEBA;
    int sizePCB = sizeof(PCBPRUEBA);

    t_buffer *bufferPCB;
    t_packet *packetPCB;
    
    bufferPCB = create_buffer();
    packetPCB = create_packet(PCB_REC, bufferPCB);
    add_to_packet(packetPCB,proceso, sizePCB);
    send_packet(packetPCB, cpu_dispatch_socket);
    destroy_packet(packetPCB);

}


t_pcb *initializePCB(int PID){
        t_pcb *pcb = malloc(sizeof(t_pcb));
        
        pcb->pid = PID;
        //id_counter++;  Aumenta el pid para que no haya 2 procesos iguales
        pcb->program_counter = 1;
        pcb->quantum = quantumGlobal;
        pcb->state = 0;
        //pcb->registers = malloc(sizeof( t_cpu_registers));
        //pcb->instruction = malloc(sizeof( t_instruction));
        // pcb->prueba=5;
        
        return pcb;
}




    
   
  
    

   
	
	
