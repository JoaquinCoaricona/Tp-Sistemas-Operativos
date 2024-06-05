#ifndef LONG_TERM_SCHEDULER_H_
#define LONG_TERM_SCHEDULER_H_

#include <semaphore.h>
#include <commons/collections/queue.h>
#include "../include/utils.h"
#include "recepcion.h"

extern sem_t m_execute_process;
extern sem_t short_term_scheduler_semaphore;
extern sem_t sem_ready;
extern sem_t m_ready_queue;
extern sem_t sem_hay_pcb_esperando_ready; 
extern sem_t sem_multiprogramacion;			
extern pthread_mutex_t mutex_state_exit;
extern pthread_mutex_t mutex_state_new;
extern pthread_mutex_t mutex_state_ready;
extern pthread_mutex_t mutex_state_prioridad;
extern pthread_mutex_t m_planificador_corto_plazo;
extern pthread_mutex_t m_planificador_largo_plazo;
extern pthread_mutex_t m_procesoEjectuandoActualmente;
extern t_queue *queue_new;
extern t_queue *queue_ready;
extern t_queue *queue_prioridad;
extern t_queue *queue_exit;
extern t_queue* queue_block;
extern char *scheduler_algorithm;

extern t_pcb *pcbEJECUTANDO;
extern int quantumGlobal;
extern t_log *logger;
extern int gradoMultiprogramacion;
extern int cpu_dispatch_socket;
extern int cpu_interrupt_socket;
extern int procesoEjectuandoActualmente;

void initialize_queue_and_semaphore();
void long_term_scheduler();
void agregarANew(t_pcb *pcb);
t_pcb *obtenerSiguienteAready();
void addEstadoReady(t_pcb *pcb);
void *Aready(void *arg);
void addEstadoExit(t_pcb *pcb);
void addColaPrioridad(t_pcb *pcb);
t_pcb *obtenerSiguienteColaPrioridad();

#endif // LONG_TERM_SCHEDULER_H_

