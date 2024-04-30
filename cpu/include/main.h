#ifndef MAIN_H_
#define MAIN_H_

#include "../include/utils.h"

void* manage_interrupt_request(void *args);
void manage_dispatch_request();
void pedirInstruccion(int pid, int pc,int client_fd);
#endif /* MAIN_H_ */
