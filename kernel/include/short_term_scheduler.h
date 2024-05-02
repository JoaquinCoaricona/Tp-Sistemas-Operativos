#ifndef SHORT_TERM_SCHEDULER_H_
#define SHORT_TERM_SCHEDULER_H_

#include "../include/utils.h"
#include "../include/long_term_scheduler.h"
#include "../include/short_term_scheduler.h"
#include <sys/types.h>
#include <unistd.h>

extern int id_counter;
extern int quantum;

void *run_short_term_scheduler(void *arg);


//FIFO
void short_term_scheduler_fifo();

//RR
void short_term_scheduler_round_robin();

//VRR
void short_term_scheduler_virtual_round_robin();

void send_process(t_pcb *process);

void send_execution_context_to_CPU(t_pcb *process);

t_pcb *initializePCB();

#endif //SHORT_TERM_SCHEDULER_H_
