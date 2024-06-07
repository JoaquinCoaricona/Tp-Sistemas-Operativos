#ifndef RECEPCION_H_
#define RECEPCION_H_
#include "../include/utils.h"
#include "long_term_scheduler.h"// agrego esto porque no encuentra en Recepcion.C el PCBejecutando Global

extern t_list *listaInterfaces;
extern t_queue* queue_ready;

extern t_log *logger;
extern sem_t procesosEsperandoIO;


t_pcb *fetch_pcb_con_sleep(int server_socket,int *tiempoDormir,char **nomrebInterfaz);
bool esLaInterfazBuscada(t_interfaz_registrada *recibida);
t_interfaz_registrada *buscar_interfaz(char *nombreInterfaz);
void crear_hilo_interfaz(t_interfaz_registrada *interfaz);
void llamadasIOstdout(t_interfaz_registrada *interfaz);
void llamadasIOstdin(t_interfaz_registrada *interfaz);
#endif /* RECEPCION_H_ */