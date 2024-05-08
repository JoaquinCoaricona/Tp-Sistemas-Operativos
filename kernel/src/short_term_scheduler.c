#include "short_term_scheduler.h"
int id_counter = 1;

//FIFO
void planificador_corto_plazo_FIFO() {      
    
    sem_wait(&short_term_scheduler_semaphore);//esto es para despertar al planificador de corto plazo
    //aca falta el semaforo de corto plazo para el detener planificacion
     
    pthread_mutex_lock(&mutex_state_ready;
    t_pcb *proceso = queue_pop(queue_ready); //semaforo mutex para entrar a la lista de READY
    pthread_mutex_unlock(&mutex_state_ready;

    proceso->state = EXEC;
    log_info(logger, "Cambio De Estado Proceso %d a %s\n", proceso->pid,proceso->state);
    
    enviar_proceso_cpu(proceso);

 }


////Round Robin
    void short_term_scheduler_round_robin() {
    
    sem_wait(&short_term_scheduler_semaphore);//esto es para despertar al planificador de corto plazo
    //aca falta el semaforo de corto plazo para el detener planificacion
    
    pthread_mutex_lock(&mutex_state_ready;
    t_pcb *proceso = queue_pop(queue_ready); //semaforo mutex para entrar a la lista de READY
    pthread_mutex_unlock(&mutex_state_ready;

    proceso->state = EXEC;
    log_info(logger, "Cambio De Estado Proceso %d a %s\n", proceso->pid,proceso->state);

    procesoEjectuandoActualmente = proceso->pid;
    enviar_proceso_cpu(proceso); //envio el proceso
        
    pthread_t hiloQuantum;
    t_quantum *quatnumHilo = malloc(sizeof(t_quantum));
    quatnumHilo->pid = proceso->pid;
    
    //Aca no es necesario pasarle un quantum porque hay un quantum Global

    pthread_create(&hiloQuantum,NULL,manejoHiloQuantum,quatnumHilo);
    pthread_detach(hiloQuantum);





    enviarInterrupcionQuantum();
    



}


void manejoHiloQuantum(void *pcb){

    t_quantum *proceso = (t_quantum *)pcb;
    proceso->pid = pcb->pid;
    usleep(1000 * quantumGlobal)

    if(procesoEjectuandoActualmente == proceso->pid){
        enviarInterrupcion();
    }
    
    
    free(pcb);
   
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

void enviar_proceso_cpu(t_pcb *proceso) {


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




    
   
  
    

   
	
	
