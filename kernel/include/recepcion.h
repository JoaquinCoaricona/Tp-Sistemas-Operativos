#ifndef RECEPCION_H_
#define RECEPCION_H_
#include "../include/utils.h"
extern t_list *listaInterfaces;
extern t_queue* queue_ready;

extern t_log *logger;
extern sem_t procesosEsperandoIO;


t_pcb *fetch_pcb_con_sleep(int server_socket,int *tiempoDormir,char **nomrebInterfaz);
bool esLaInterfazBuscada(t_interfaz_registrada *recibida);
t_interfaz_registrada *buscar_interfaz(char *nombreInterfaz);
void crear_hilo_interfaz(t_interfaz_registrada *interfaz);
#endif /* RECEPCION_H_ */