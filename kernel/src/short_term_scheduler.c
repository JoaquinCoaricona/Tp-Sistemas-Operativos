#include "short_term_scheduler.h"
int id_counter = 1;

// //FIFO
// void short_term_scheduler_fifo() {
//     sem_wait(&short_term_scheduler_semaphore);
//     sem_wait(&m_ready_queue);

//     if(queue_size(queue_ready) == 0) {
//         sem_post(&m_ready_queue);
//         sem_post(&short_term_scheduler_semaphore);
//         return;
//     }

//     while(queue_size(queue_ready) != 0) {
//         t_pcb *process = queue_pop(queue_ready);
//         execute_process(process);
//     }

//     sem_post(&m_ready_queue);
//     sem_post(&short_term_scheduler_semaphore);
// }

// //RR
// void short_term_scheduler_round_robin() {
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

// void execute_process(t_pcb *process) {
//     sem_wait(&m_execute_process);
//     if(process -> state != "EXEC") {
//         free(process->state);//error al compilar
//         process->state = string_new(); //error al compilar
//         string_append(&(process->state), "EXEC");
//     }
//     send_execution_context_to_CPU(process);
//     sem_post(&m_execute_process);
// }

void send_execution_context_to_CPU(t_pcb *process) {

}

t_pcb *initializePCB(){
        
        t_pcb *pcb = malloc(sizeof(t_pcb));

        pcb->pid = id_counter;
        id_counter++;
        pcb->program_counter = 1;
        pcb->quantum = 5;
        pcb->state = 0;
        //pcb->registers = malloc(sizeof( t_cpu_registers));
        //pcb->instruction = malloc(sizeof( t_instruction));
        // pcb->prueba=5;
        
        return pcb;
}




    
   
  
    

   
	
	
