#include "operaciones.h"

void setear_registro(t_pcb *contexto, char* registro, uint32_t valor)
{
	if(strcmp(registro,"AX")==0)
	{
		contexto->registers->AX=valor;
		//log_info(logger, "Se setea en %s el valor: %d",registro,  contexto->registers->AX);

	}else if(strcmp(registro,"BX")==0)
	{
		contexto->registers->BX=valor;
		//log_info(logger, "Se setea en %s el valor: %d",registro,  contexto->registers->BX);
	}else if(strcmp(registro,"CX")==0)
	{
		contexto->registers->CX=valor;
		//log_info(logger, "Se setea en %s el valor: %d",registro,  contexto->registers->CX);
	}else if(strcmp(registro,"DX")==0)
	{
		contexto->registers->DX=valor;
		//log_info(logger, "Se setea en %s el valor: %d",registro,  contexto->registers->DX);
	}else if(strcmp(registro,"EAX")==0)
	{
		contexto->registers->EAX=valor;
		//log_info(logger, "Se setea en %s el valor: %d",registro,  contexto->registers->EAX);
	}else if(strcmp(registro,"EBX")==0)
	{
		contexto->registers->EBX=valor;
		//log_info(logger, "Se setea en %s el valor: %d",registro,  contexto->registers->EBX);
	}
    else if(strcmp(registro,"ECX")==0)
	{
		contexto->registers->ECX=valor;
		//log_info(logger, "Se setea en %s el valor: %d",registro,  contexto->registers->ECX);
	}
    else if(strcmp(registro,"EDX")==0)
	{
		contexto->registers->EDX=valor;
		//log_info(logger, "Se setea en %s el valor: %d",registro,  contexto->registers->EDX);
	}
    

}

uint32_t obtener_valor_del_registro(char* registro_a_leer, t_pcb* contexto_actual){
	uint32_t valor_leido = -1; //valor devuelto si se escribe mal el nombre del registro

	if(strcmp(registro_a_leer,"AX")==0)
	{
		valor_leido= contexto_actual->registers->AX;

	}else if(strcmp(registro_a_leer,"BX")==0)
	{

		valor_leido= contexto_actual->registers->BX;

	}else if(strcmp(registro_a_leer,"CX")==0)
	{

		valor_leido= contexto_actual->registers->CX;

	}else if(strcmp(registro_a_leer,"DX")==0)
	{
		valor_leido= contexto_actual->registers->DX;
	}else if(strcmp(registro_a_leer,"EAX")==0)
	{
		valor_leido= contexto_actual->registers->EAX;
	}else if(strcmp(registro_a_leer,"EBX")==0)
	{
		valor_leido= contexto_actual->registers->EBX;
	}else if(strcmp(registro_a_leer,"ECX")==0)
	{
		valor_leido= contexto_actual->registers->ECX;
	}else if(strcmp(registro_a_leer,"EDX")==0)
	{
		valor_leido= contexto_actual->registers->EDX;
	}

	//log_info(logger, "Se se lee en %s el valor: %d",registro_a_leer,  valor_leido);

	return valor_leido;
}


//SET (Registro, Valor): Asigna al registro el valor pasado como parámetro.
void operacion_set(t_pcb* contexto, t_instruccion_unitaria* instruccion)
{
	char* registro = strdup(instruccion->parametros[0]); //No copiar la direccion, usar en lo posible strdup
	uint32_t valor = atoi(instruccion->parametros[1]);
	setear_registro(contexto, registro, valor);

	free(registro);
}

//SUM (Registro Destino, Registro Origen): Suma al Registro Destino el Registro Origen y deja el 
//    resultado en el Registro Destino.
void operacion_sum(t_pcb* contexto, t_instruccion_unitaria* instruccion)
{   
    char* origen  =  strdup(instruccion->parametros[1]);
    char* destino =  strdup(instruccion->parametros[0]);

    uint32_t origen_valor =   obtener_valor_del_registro(origen,PCBACTUAL);
    uint32_t destino_valor =  obtener_valor_del_registro(destino,PCBACTUAL);

    destino_valor += origen_valor; 
    setear_registro(contexto,destino,destino_valor);
    free(origen);
    free(destino);

}

//SUB (Registro Destino, Registro Origen): Resta al Registro Destino el Registro Origen y 
//     deja el resultado en el Registro Destino.

void operacion_sub(t_pcb* contexto, t_instruccion_unitaria* instruccion)
{   
    char* origen  =  strdup(instruccion->parametros[1]);
    char* destino =  strdup(instruccion->parametros[0]);

    uint32_t origen_valor =   obtener_valor_del_registro(origen,PCBACTUAL);
    uint32_t destino_valor =  obtener_valor_del_registro(destino,PCBACTUAL);

    destino_valor -= origen_valor; 

    setear_registro(contexto,destino,destino_valor);
    free(origen);
    free(destino);

}

//JNZ (Registro, Instrucción): Si el valor del registro es distinto de cero,
//     actualiza el program counter al número de instrucción pasada por parámetro.
void operacion_jnz(t_pcb* contexto, t_instruccion_unitaria* instruccion)
{   
    
    uint32_t valor = atoi(instruccion->parametros[1]);
    char* cadena = strdup(instruccion->parametros[0]);
    if(obtener_valor_del_registro(cadena,contexto) != 0){
        contexto->program_counter = valor;
    }
    free()

}


