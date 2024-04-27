#ifndef LONG_TERM_SCHEDULER_H_
#define LONG_TERM_SCHEDULER_H_

#include <semaphore.h>
#include <commons/collections/queue.h>
#include "../include/utils.h"

extern t_queue* queue_new;
extern t_queue* queue_ready;
extern t_queue* queue_exit;
extern char* scheduler_algorithm;

void enterNew(t_pcb *pcb,t_log *logger);

void initialize_queue_and_semaphore();
void create_process();
void end_process();
void long_term_scheduler();

extern sem_t m_execute_process;
extern sem_t short_term_scheduler_semaphore;
extern sem_t m_ready_queue;
extern sem_t sem_hay_pcb_esperando_ready;
extern sem_t sem_multiprogramacion;
extern pthread_mutex_t mutex_state_new;
pthread_mutex_t mutex_state_ready;
#endif // LONG_TERM_SCHEDULER_H_
