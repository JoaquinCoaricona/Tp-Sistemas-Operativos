#ifndef LONG_TERM_SCHEDULER_H_
#define LONG_TERM_SCHEDULER_H_

#include <semaphore.h>
#include <commons/collections/queue.h>
#include "../include/utils.h"

extern sem_t m_execute_process;
extern sem_t short_term_scheduler_semaphore;
extern sem_t long_term_scheduler_semaphore;
extern sem_t sem_ready;
extern sem_t m_ready_queue;
extern sem_t sem_hay_pcb_esperando_ready;
extern sem_t sem_multiprogramacion;
extern pthread_mutex_t mutex_state_new;
extern pthread_mutex_t mutex_state_ready;
extern pthread_mutex_t m_pid_evicted;
extern pthread_mutex_t m_short_term_scheduler;
extern t_pcb* current_executing_process; 

extern t_queue* queue_new;
extern t_queue* queue_ready;
extern t_queue* queue_ready_with_priority;
extern t_queue* queue_exit;
extern t_queue* queue_block;

extern t_pcb* executing_process;

extern char* scheduler_algorithm;

void enterNew(t_pcb *pcb);

void initialize_queue_and_semaphore();
void long_term_scheduler();
void enterNew(t_pcb *pcb);
t_pcb *obtenerSiguienteAready();
void addEstadoReady(t_pcb *pcb);
void Aready();


#endif // LONG_TERM_SCHEDULER_H_
