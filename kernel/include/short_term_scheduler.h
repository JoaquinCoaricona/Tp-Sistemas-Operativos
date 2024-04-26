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
void send_execution_context_to_CPU(t_pcb *process);
t_pcb *initializePCB();

#endif //SHORT_TERM_SCHEDULER_H_
