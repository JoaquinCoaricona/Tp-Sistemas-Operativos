#ifndef SHORT_TERM_SCHEDULER_H_
#define SHORT_TERM_SCHEDULER_H_

#include "../include/utils.h"
#include "../include/long_term_scheduler.h"

#include <sys/types.h>
#include <unistd.h>

extern int id_counter;
extern t_temporal *timer;
extern int ms_transcurridos;
extern char* algoritmo_planificacion;

void *planificadorCortoPlazo(void *arg);
void planificador_corto_plazo_FIFO();
void manejoHiloQuantum(void *pcb);
void planificador_corto_plazo_RoundRobin();
void enviarInterrupcion(char *motivo, int pid);
void planificador_corto_plazo_Virtual_RoundRobin();
void manejoHiloQuantumVRR(void *pcb);
void obtenerDatosTemporal();
void enviar_proceso_cpu(t_pcb *proceso);
t_pcb *initializePCB(int PID);

#endif //SHORT_TERM_SCHEDULER_H_
