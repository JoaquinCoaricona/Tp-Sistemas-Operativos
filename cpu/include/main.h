#ifndef MAIN_H_
#define MAIN_H_

#include "../include/utils.h"
#include "../include/opcode.h"
#include "operaciones.h"

extern t_log *logger;
extern int server_dispatch_fd;
extern int client_fd_memoria;
extern bool continuar_con_el_ciclo_instruccion;
extern bool hay_interrupcion_pendiente;
extern int tipoInterrupcion;
extern int pid_ejecutando;
extern int pid_a_desalojar;
extern pthread_mutex_t mutex_interrupcion;


void* manage_interrupt_request(void *args);
void manage_dispatch_request();
t_instruccion_unitaria *pedirInstruccion(int pid, int pc, int client_fd);
int recibir_operacion(int client_fd);
void ciclo_de_instruccion(int socket_kernel);
void recibir_interrupcion(int socket_kernel);
void recibir_eliminar_proceso(int socket_kernel);
void responder_a_kernel(char *mensaje, int socket);
void devolver_a_kernel(t_pcb *contexto_actual, int socket_kernel, char *motivo);
void devolver_a_kernel_fin_quantum(t_pcb *contexto_actual, int socket_kernel, char *motivo);
void devolver_a_kernel_Eliminacion(t_pcb *contexto_actual, int socket_kernel, char *motivo);
void destroy_instuccion_actual(t_instruccion_unitaria *instruccion);

//aca deberia estar declarado esto, pero dice que no reconoce t_instruccion_unitaria
//por eso lo comento
//void destroy_instuccion_actual(t_instruccion_unitaria *instruccion);
//void pedirInstruccion(int pid, int pc,int client_fd);


#endif /* MAIN_H_ */
