#ifndef MAIN_H_
#define MAIN_H_
#include "../include/utils.h"
#include "recepcion.h"
#include "short_term_scheduler.h"

extern int memory_socket;
extern int cpu_dispatch_socket;
extern int cpu_interrupt_socket;
extern int PID; // Global
extern t_log *logger;
extern t_pcb *pcbEJECUTANDO;
extern t_list *listaInterfaces;
extern int gradoMultiprogramacion;
extern int salteoPostAlSemaforo;
extern int quantumGlobal;
extern int procesoEjectuandoActualmente;
extern char *algoritmo_planificacion;
extern bool planificacion_detenida;

void* manage_request_from_input_output(void *args);
void* manage_request_from_dispatch(void *args);
void enviar_path_a_memoria(char *path);
void create_process(char* path);
void end_process();
void fetch_pcb_actualizado(server_socket);
void fetch_pcb_actualizado_fin_quantum(int server_socket);
void fetch_pcb_actualizado_A_eliminar(int server_socket);
t_interfaz_registrada *recibir_interfaz(int client_socket);
void detener_planificacion();
void iniciar_planificacion();
void finalizar_proceso(char *parametro);
void multiprogramacion(char *parametro);
void controlGradoMultiprogramacion();

#endif /* MAIN_H_ */