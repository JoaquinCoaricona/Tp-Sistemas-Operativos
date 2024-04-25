#include "short_term_scheduler.h"
int id_counter = 1;
//FIFO
void short_term_scheduler_fifo() {

}

//RR
void short_term_scheduler_round_robin() {

}

//VRR
void short_term_scheduler_virtual_round_robin() {

}

t_pcb *initializePCB(){
        
        t_pcb *pcb = malloc(sizeof(t_pcb));

        pcb->pid = id_counter;
        id_counter++;
        pcb->program_counter = 1;
        pcb->quantum = 5;
        pcb->process_state = malloc(strlen("NEW") + 1); strcpy(pcb->process_state, "NEW");
        pcb->length_process_state = strlen("NEW")+1;
        pcb->cpu_registers = NULL;
        pcb->instruction = NULL;
        
        return pcb;
}



    
   
  
    

   
	
	
