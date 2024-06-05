#ifndef MAIN_H_
#define MAIN_H_

#include "../include/utils.h"

extern int socket_kernel;
extern int socket_memoria;
extern t_log *logger;

int fetch_tiempoDormir(int socket_kernel);
void enviarAvisoAKernel(int socket_kernel);


#endif /* MAIN_H_ */