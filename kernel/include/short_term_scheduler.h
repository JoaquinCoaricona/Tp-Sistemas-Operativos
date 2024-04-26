#ifndef SHORT_TERM_SCHEDULER_H_
#define SHORT_TERM_SCHEDULER_H_

#include "../include/utils.h"
#include "../include/long_term_scheduler.h"
extern int id_counter;

//Iniciar PCB
t_pcb *initializePCB();
//FIFO
void short_term_scheduler_fifo();

//RR
void short_term_scheduler_round_robin();

//VRR
void short_term_scheduler_virtual_round_robin();

void execute_process(t_pcb *process);

#endif //SHORT_TERM_SCHEDULER_H_
