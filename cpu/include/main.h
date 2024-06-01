#ifndef MAIN_H_
#define MAIN_H_

#include "../include/utils.h"
#include "../include/opcode.h"
#include "operaciones.h"


void* manage_interrupt_request(void *args);
void manage_dispatch_request();
//aca deberia estar declarado esto, pero dice que no reconoce t_instruccion_unitaria
//por eso lo comento
//void destroy_instuccion_actual(t_instruccion_unitaria *instruccion);
//void pedirInstruccion(int pid, int pc,int client_fd);


#endif /* MAIN_H_ */
