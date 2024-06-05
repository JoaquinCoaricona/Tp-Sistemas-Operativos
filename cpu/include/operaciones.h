#ifndef OPERACIONES_H_
#define OPERACIONES_H_

#include "../include/utils.h"
#include "main.h"

extern t_log *logger;
extern int client_fd_memoria;
extern int tamaPagina;
typedef struct {
	int opcode_lenght;
	char* opcode;
	int parametro1_lenght;
	int parametro2_lenght;
	int parametro3_lenght;
	int parametro4_lenght;
	int parametro5_lenght;
	char* parametros[5];

}t_instruccion_unitaria;

void setear_registro(t_pcb *contexto, char* registro, uint32_t valor);
uint32_t obtener_valor_del_registro(char* registro_a_leer, t_pcb* contexto_actual);
void operacion_set(t_pcb* contexto, t_instruccion_unitaria* instruccion);
void operacion_sum(t_pcb* contexto, t_instruccion_unitaria* instruccion);
void operacion_sub(t_pcb* contexto, t_instruccion_unitaria* instruccion);
void operacion_jnz(t_pcb* contexto, t_instruccion_unitaria* instruccion);
void operacion_sleep(t_pcb *contexto,int socket,t_instruccion_unitaria* instruccion);
int solicitarMarco(int numeroPagina, int pid);

#endif // OPERACIONES_H_
