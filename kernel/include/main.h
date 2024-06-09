#ifndef MAIN_H_
#define MAIN_H_
#include "../include/utils.h"
#include "recepcion.h"
#include "short_term_scheduler.h"



void iniciar_planificacion();
void detener_planificacion();

t_resource* new_resource(char* resource_name){

void* manage_request_from_input_output(void *args);
void* manage_request_from_dispatch(void *args);
void create_process(char* path);
void end_process();
t_interfaz_registrada *recibir_interfaz(int client_socket);

#endif /* MAIN_H_ */