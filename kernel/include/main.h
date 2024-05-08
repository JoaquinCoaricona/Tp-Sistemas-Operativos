#ifndef MAIN_H_
#define MAIN_H_
#include "../include/utils.h"
#include "recepcion.h"

extern t_log *logger;
extern int gradoMultiprogramacion;
extern t_queue* queue_ready;

extern int cpu_dispatch_socket;


void* manage_request_from_input_output(void *args);
void* manage_request_from_dispatch(void *args);
void create_process(char* path);
void end_process();
t_interfaz_registrada *recibir_interfaz(int client_socket);

#endif /* MAIN_H_ */