#include "short_term_scheduler.h"
int id_counter = 1;
int pid_interrupt = 0;
t_pcb *process_on_execute;
//FIFO
void short_term_scheduler_fifo() {
    sem_wait(&short_term_scheduler_semaphore);
    sem_wait(&m_ready_queue);

    if(queue_size(queue_ready) == 0) {
        sem_post(&m_ready_queue);
        sem_post(&short_term_scheduler_semaphore);
        return;
    }

    t_pcb *process = queue_pop(queue_ready);
    //TODO: Enviar lista de procesos a cpu

    send_process(process);
    sem_post(&m_ready_queue);
    sem_post(&short_term_scheduler_semaphore);
}
//RR
 void short_term_scheduler_round_robin() {
     sem_wait(&short_term_scheduler_semaphore);
     sem_wait(&m_ready_queue);

     if(queue_size(queue_ready) == 0) {
         sem_post(&m_ready_queue);
         sem_post(&short_term_scheduler_semaphore);
         return;
     }
    t_pcb *process = queue_pop(queue_ready);
    
    
     sem_post(&m_ready_queue);
     sem_post(&short_term_scheduler_semaphore);
    send_process(process);

    uslepp(quantum*1000);//Transforma el quantum en milisegundos
    
    pthread_mutex_lock(&m_pid_evicted);
    pid_interrupt = process->pid;
    pthread_mutex_unlock(&m_pid_evicted);

    sem_wait(&m_execute_process);
    if(current_executing_process ==NULL){
        sem_post(&m_execute_process);
        sem_wait(&m_ready_queue);
        int size_queue_ready=queue_size(queue_ready);
        sem_post(&m_ready_queue);
        sem_wait(&sem_hay_pcb_esperando_ready);
        int size_queue_new = queue_size(queue_new);
        sem_post(&sem_hay_pcb_esperando_ready);

        if(size_queue_ready>0){
            sem_post(&short_term_scheduler_semaphore);
        }else if(size_queue_new > 0){
            sem_post(&long_term_scheduler_semaphore);
        }
        pthread_mutex_unlock(&m_short_term_scheduler);
        return;

    }else{
        sem_post(&m_execute_process);
        

    }
    sem_wait(&m_execute_process);
    log_info(logger, "PID: %d - Desalojado por fin de Quantum", pid_interrupt);
    sem_post(&m_execute_process);

    char *message= malloc(300);
    pthread_mutex_lock(&m_pid_evicted);
    sprintf(message,"Desalojo por fin de Quantum a %d",pid_interrupt);
    pthread_mutex_unlock(&m_pid_evicted);

    free(message);

    sem_wait(&m_execute_process);
    if(process->state == EXEC){
        log_info(logger,"PID: %d - Estado Anterior: %s - Estado Actual: %s",process->pid,"EXEC","READY");
        process->state=READY;

        current_executing_process=NULL;
        sem_post(&m_execute_process);
    }else
    {
                sem_post(&m_execute_process);
    }
    pthread_mutex_lock(&m_pid_evicted);
    pid_interrupt=0;
    pthread_mutex_unlock(&m_pid_evicted);
    pthread_mutex_lock(&m_short_term_scheduler);

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

void send_process(t_pcb *process) {
    sem_wait(&m_execute_process);
    if(process -> state != "EXEC") {
        string_append(&(process->state), "EXEC");
    }
    send_execution_context_to_CPU(process);
    sem_post(&m_execute_process);
}

void send_execution_context_to_CPU(t_pcb *process) {

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




    
   
  
    

   
	
	
