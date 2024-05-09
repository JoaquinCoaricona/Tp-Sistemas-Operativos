#include "short_term_scheduler.h"
int id_counter = 1;


//Planificador de Corto Plazo General
void *planificadorCortoPlazo(void *arg){

	while(1){
		if(string_equals_ignore_case(algoritmo_planificacion, "FIFO")){
			planificador_corto_plazo_FIFO();
		}else if(string_equals_ignore_case(algoritmo_planificacion, "RR")){
			planificador_corto_plazo_RoundRobin();
		}else {
			//planificar_corto_plazo_round_robbin();
            printf("ERROR");
		}
	}

	return NULL;
}


//FIFO
void planificador_corto_plazo_FIFO() {      
    
    sem_wait(&sem_ready); 
    sem_wait(&short_term_scheduler_semaphore);//esto es para despertar al planificador de corto plazo
    pthread_mutex_lock(&m_planificador_corto_plazo);
     
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
    
    usleep(1000 * quantumGlobal);

    if(procesoEjectuandoActualmente == proceso->pid){
        char *motivo = "Fin de Quantum";
        enviarInterrupcion(motivo,proceso->pid);
    }else{
    log_info(logger, "El Proceso %i termino antes del Quantum",proceso->pid);
    }
    
    free(pcb);
    pthread_cancel(pthread_self());
   
}

//Round Robin
void planificador_corto_plazo_RoundRobin() {
    
    sem_wait(&sem_ready);
    sem_wait(&short_term_scheduler_semaphore);//esto es para despertar al planificador de corto plazo
    pthread_mutex_lock(&m_planificador_corto_plazo);
    
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
    add_to_packet(packetINTERRUPCION,pid,sizeof(int));

    send_packet(packetINTERRUPCION, cpu_interrupt_socket);
    destroy_packet(packetINTERRUPCION);
}

// //VRR
// void short_term_scheduler_virtual_round_robin() {
//     sem_wait(&short_term_scheduler_semaphore);
//     sem_wait(&m_ready_queue);

//     if(queue_size(queue_ready) == 0) {
//         sem_post(&m_ready_queue);
//         sem_post(&short_term_scheduler_semaphore);
//         return;
//     }

//     sem_post(&m_ready_queue);
//     sem_post(&short_term_scheduler_semaphore);
// }

// void send_process(t_pcb *process) {
//     sem_wait(&m_execute_process);
//     if(process -> state != "EXEC") {
//         string_append(&(process->state), "EXEC");
//     }
//     send_execution_context_to_CPU(process);
//     sem_post(&m_execute_process);
// }

void enviar_proceso_cpu(t_pcb *proceso){


    t_pcb PCBPRUEBA;
    int sizePCB = sizeof(PCBPRUEBA);

    t_buffer *bufferPCB;
    t_packet *packetPCB;
    
    bufferPCB = create_buffer();
    packetPCB = create_packet(PCB_REC, bufferPCB);
    add_to_packet(packetPCB,proceso, sizePCB);
    send_packet(packetPCB, cpu_dispatch_socket);

}


t_pcb *initializePCB(int PID){
        t_pcb *pcb = malloc(sizeof(t_pcb));
        
        pcb->pid = PID;
        //id_counter++;  Aumenta el pid para que no haya 2 procesos iguales
        pcb->program_counter = 1;
        pcb->quantum = 5;
        pcb->state = 0;
        //pcb->registers = malloc(sizeof( t_cpu_registers));
        //pcb->instruction = malloc(sizeof( t_instruction));
        // pcb->prueba=5;
        
        return pcb;
}




    
   
  
    

   
	
	
