#include "short_term_scheduler.h"

//FIFO
void short_term_scheduler_fifo() {

}

//RR
void short_term_scheduler_round_robin() {

}

//VRR
void short_term_scheduler_virtual_round_robin() {

}

t_pcb *iniciarPCB(t_proceso *proceso){
        
        t_pcb *pcb = malloc(sizeof(t_pcb));

        pcb->pid = id_contador;
        id_contador++;
        pcb->program_counter = 1;
        pcb->quantum = 1;
        pcb->process_state = malloc(strlen("NEW") + 1); strcpy(pcb->process_state, "NEW");
        return pcb;
}

    
   
  
    

   
	
	