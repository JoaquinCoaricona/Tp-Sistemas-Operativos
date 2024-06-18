#ifndef MAIN_H_
#define MAIN_H_
#include "../include/utils.h"
#include "recepcion.h"
#include "short_term_scheduler.h"

extern int cpu_dispatch_socket;



void iniciar_planificacion();
void detener_planificacion();
void liberarRecursos(int socket);
bool buscarRecursoUsado(void* args);
void liberacionProceso(void *cola);



void* manage_request_from_input_output(void *args);
void* manage_request_from_dispatch(void *args);
void create_process(char* path);
void end_process();
t_interfaz_registrada *recibir_interfaz(int client_socket);

typedef struct
{
    int instancias;
    t_queue *colaBloqueo;
}t_recurso;

typedef struct
{
    char* nombreRecurso;
    int pidUsuario;
}t_recursoUsado;

typedef struct
{
    char* nombreRecurso;
    t_queue *colaBloqueo;
}t_colayNombre;



#endif /* MAIN_H_ */