#ifndef LONG_TERM_SCHEDULER_H_
#define LONG_TERM_SCHEDULER_H_

#include <commons/collections/queue.h>
#include "../include/utils.h"

extern t_queue* queue_new;
extern t_queue* queue_ready;
extern t_queue* queue_exit;
extern char* scheduler_algorithm;

void ingresarANew(t_pcb *pcb,t_log *logger);

void initialize_queue_and_semaphore();
void create_process();
void end_process();
void long_term_scheduler();


extern pthread_mutex_t mutex_estado_new;
#endif // LONG_TERM_SCHEDULER_H_