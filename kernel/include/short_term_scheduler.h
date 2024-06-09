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
void planificador_corto_plazo_Virtual_RoundRobin();

void execute_process(t_pcb *process);
void send_execution_context_to_CPU(t_pcb *process);
t_pcb *initializePCB();
void *planificadorCortoPlazo(void *arg);
void enviar_proceso_cpu(t_pcb *proceso);
void manejoHiloQuantumVRR(void *pcb);

//Para VRR

extern t_temporal *timer;
extern int ms_transcurridos;


#endif //SHORT_TERM_SCHEDULER_H_
