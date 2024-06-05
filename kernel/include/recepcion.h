#ifndef RECEPCION_H_
#define RECEPCION_H_
#include "../include/utils.h"
#include "long_term_scheduler.h"// agrego esto porque no encuentra en Recepcion.C el PCBejecutando Global

extern sem_t procesosEsperandoIO;
extern char *interfazBUSCADA; 

extern t_list *listaInterfaces;
extern t_queue* queue_ready;
extern t_log *logger;

t_pcb *fetch_pcb_con_sleep(int server_socket,int *tiempoDormir,char **nomrebInterfaz);
t_interfaz_registrada *buscar_interfaz(char *nombreInterfaz);
void cargarEnListaIO(t_pcb *receptorPCB, t_interfaz_registrada *interfaz, int tiempoDormir);
bool esLaInterfazBuscada(t_interfaz_registrada *recibida);
void llamadas_io(t_interfaz_registrada *interfaz);
void crear_hilo_interfaz(t_interfaz_registrada *interfaz);
#endif /* RECEPCION_H_ */