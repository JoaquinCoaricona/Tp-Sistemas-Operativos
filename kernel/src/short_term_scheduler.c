#include "short_term_scheduler.h"
int id_counter = 1;
int pid_interrupt = 0;
t_pcb *process_on_execute;

//Loop infinito de planificacion
void *run_short_term_scheduler(void *arg) {
    while(1) {
        if(string_equals_ignore_case(scheduler_algorithm, "FIFO")) {
            short_term_scheduler_fifo();
        }
        else if(string_equals_ignore_case(scheduler_algorithm, "RR")) {
            short_term_scheduler_round_robin();
        }
        else if(string_equals_ignore_case(scheduler_algorithm, "VRR")) {
            short_term_scheduler_virtual_round_robin();
        }
    }

    return NULL;
}

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

    uslepp(quantum);//Transforma el quantum en milisegundos
    
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


void short_term_scheduler_virtual_round_robin() {
    bool priority_process = false;
    int quantum_after_blocked;

    sem_wait(&short_term_scheduler_semaphore);
    sem_wait(&m_ready_queue);
    sem_wait(&m_ready_with_priority_queue);

    if(queue_size(queue_ready_with_priority) != 0) {
        t_pcb *process = queue_pop(queue_ready_with_priority);
        priority_process = true;
        quantum_after_blocked = process-> quantum % quantum;
    }
    else if (queue_size(queue_ready) != 0){
        t_pcb *process = queue_pop(queue_ready);
    }
    else{
        sem_wait(&m_ready_with_priority_queue);
        sem_post(&m_ready_queue);
        sem_post(&short_term_scheduler_semaphore);
        return;
    }
    
    sem_wait(&m_ready_with_priority_queue);
    sem_post(&m_ready_queue);
    sem_post(&short_term_scheduler_semaphore);

    send_process(process);

    if(priority_process) {
        uslepp(quantum_after_blocked);//Transforma el quantum en milisegundos
    }
    else{
        uslepp(quantum);//Transforma el quantum en milisegundos
    }
    
    pthread_mutex_lock(&m_pid_evicted);
    pid_interrupt = process->pid;
    pthread_mutex_unlock(&m_pid_evicted);

    sem_wait(&m_execute_process);
    if(current_executing_process == NULL){
        sem_post(&m_execute_process);

        sem_wait(&m_ready_queue);
        int size_queue_ready=queue_size(queue_ready);
        sem_post(&m_ready_queue);

        sem_wait(&m_ready_with_priority_queue);
        int size_queue_ready=queue_size(queue_ready_with_priority);
        sem_post(&m_ready_with_priority_queue);

        sem_wait(&sem_hay_pcb_esperando_ready);
        int size_queue_new = queue_size(queue_new);
        sem_post(&sem_hay_pcb_esperando_ready);

        if(size_queue_ready > 0 || size_queue_ready > 0){
            sem_post(&short_term_scheduler_semaphore);
        }
        
        else if(size_queue_new > 0){
            sem_post(&long_term_scheduler_semaphore);
        }

        pthread_mutex_unlock(&m_short_term_scheduler);
        return;

    }else{
        sem_post(&m_execute_process);
    }

    sem_wait(&m_execute_process);
    if(priority_process) {
        log_info(logger, "PID: %d - ejecuto %d quantum con prioridad", pid_interrupt, quantum_after_blocked);
        log_info(logger, "PID: %d - Desalojado por fin de Quantum con prioridad", pid_interrupt);
    }
    else{
        log_info(logger, "PID: %d - Desalojado por fin de Quantum", pid_interrupt);
    }
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
    }
    else if(process->state == BLOCKED){
        log_info(logger,"PID: %d - Estado Anterior: %s - Estado Actual: %s",process->pid,"BLOCKED","READY");
        process->state=READY;
        current_executing_process=NULL;
        sem_post(&m_execute_process);
    }
    else
    {
        sem_post(&m_execute_process);
    }

    pthread_mutex_lock(&m_pid_evicted);
    pid_interrupt=0;
    pthread_mutex_unlock(&m_pid_evicted);

    pthread_mutex_lock(&m_short_term_scheduler);
}

void send_process(t_pcb *process) {

    sem_wait(&m_execute_process);
    executing_process = process;
    if(strcmp(process -> state, "EXEC") != 0) {
        log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", process -> pid, process -> state, "EXEC");
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




    
   
  
    

   
	
	
