#ifndef LONG_TERM_SCHEDULER_H_
#define LONG_TERM_SCHEDULER_H_

#include <semaphore.h>
#include <commons/collections/queue.h>
#include "../include/utils.h"
#include "recepcion.h"

extern t_queue* queue_new;
//extern t_queue* queue_ready; //LO LLEVO A RECEPCION.h para probar INTERFACES
extern t_queue* queue_exit;
extern t_queue* queue_block;
extern char* scheduler_algorithm;

extern t_log *logger;
extern int gradoMultiprogramacion;
extern int cpu_dispatch_socket;
extern int cpu_interrupt_socket;
extern int quantumGlobal;
extern int procesoEjectuandoActualmente;


void agregarANew(t_pcb *pcb);

void initialize_queue_and_semaphore();
void create_process(char* path);
void end_process();
void long_term_scheduler();


extern sem_t m_execute_process;
extern sem_t short_term_scheduler_semaphore;
extern sem_t m_ready_queue;
extern sem_t sem_hay_pcb_esperando_ready;
extern sem_t sem_multiprogramacion;
extern sem_t sem_ready;
extern pthread_mutex_t mutex_state_new;
extern pthread_mutex_t mutex_state_ready;
#endif // LONG_TERM_SCHEDULER_H_
