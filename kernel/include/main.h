#ifndef MAIN_H_
#define MAIN_H_
#include "../include/utils.h"
#include "recepcion.h"
#include "short_term_scheduler.h"
#include "long_term_scheduler.h"


void iniciar_planificacion();
void detener_planificacion();
void liberarRecursos(int socket);
bool buscarRecursoUsado(void* args);
void liberacionProceso(void *cola);


t_resource* new_resource(char* resource_name);

void* manage_request_from_input_output(void *args);
void* manage_request_from_dispatch(void *args);
void create_process(char* path);
t_interfaz_registrada *recibir_interfaz(int client_socket);
t_list* duplicar_recursos(t_list* lista_recursos);

t_list* obtener_recursos_por_pid(t_dictionary **matriz,char *pid, int cantidad);
t_resource *obtener_recurso_mediante_nombre(t_list *recursos,char *recurso_a_buscar);
void controlGradoMultiprogramacion();

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



#endif /* MAIN_H_ */