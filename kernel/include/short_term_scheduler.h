#ifndef SHORT_TERM_SCHEDULER_H_
#define SHORT_TERM_SCHEDULER_H_

#include "../include/utils.h"
#include "../include/long_term_scheduler.h"

#include <sys/types.h>
#include <unistd.h>

extern int id_counter;
extern char* algoritmo_planificacion;


//Iniciar PCB
t_pcb *initializePCB();
//FIFO
void planificador_corto_plazo_FIFO();

//RR
void planificador_corto_plazo_RoundRobin();

//VRR
void short_term_scheduler_virtual_round_robin();

void execute_process(t_pcb *process);
void send_execution_context_to_CPU(t_pcb *process);
t_pcb *initializePCB();
void *planificadorCortoPlazo(void *arg);
void enviar_proceso_cpu(t_pcb *proceso);



#endif //SHORT_TERM_SCHEDULER_H_
