#include "operaciones.h"

void setear_registro(t_pcb *contexto, char* registro, uint32_t valor)
{
	if(strcmp(registro,"AX")==0)
	{
		contexto->registers.AX=valor;
		//log_info(logger, "Se setea en %s el valor: %d",registro,  contexto->registers->AX);

	}else if(strcmp(registro,"BX")==0)
	{
		contexto->registers.BX=valor;
		//log_info(logger, "Se setea en %s el valor: %d",registro,  contexto->registers->BX);
	}else if(strcmp(registro,"CX")==0)
	{
		contexto->registers.CX=valor;
		//log_info(logger, "Se setea en %s el valor: %d",registro,  contexto->registers->CX);
	}else if(strcmp(registro,"DX")==0)
	{
		contexto->registers.DX=valor;
		//log_info(logger, "Se setea en %s el valor: %d",registro,  contexto->registers->DX);
	}else if(strcmp(registro,"EAX")==0)
	{
		contexto->registers.EAX=valor;
		//log_info(logger, "Se setea en %s el valor: %d",registro,  contexto->registers->EAX);
	}else if(strcmp(registro,"EBX")==0)
	{
		contexto->registers.EBX=valor;
		//log_info(logger, "Se setea en %s el valor: %d",registro,  contexto->registers->EBX);
	}
    else if(strcmp(registro,"ECX")==0)
	{
		contexto->registers.ECX=valor;
		//log_info(logger, "Se setea en %s el valor: %d",registro,  contexto->registers->ECX);
	}
    else if(strcmp(registro,"EDX")==0)
	{
		contexto->registers.EDX=valor;
		//log_info(logger, "Se setea en %s el valor: %d",registro,  contexto->registers->EDX);
	}
    

}

uint32_t obtener_valor_del_registro(char* registro_a_leer, t_pcb* contexto_actual){
	uint32_t valor_leido = -1; //valor devuelto si se escribe mal el nombre del registro

	if(strcmp(registro_a_leer,"AX")==0)
	{
		valor_leido= contexto_actual->registers.AX;

	}else if(strcmp(registro_a_leer,"BX")==0)
	{

		valor_leido= contexto_actual->registers.BX;

	}else if(strcmp(registro_a_leer,"CX")==0)
	{

		valor_leido= contexto_actual->registers.CX;

	}else if(strcmp(registro_a_leer,"DX")==0)
	{
		valor_leido= contexto_actual->registers.DX;
	}else if(strcmp(registro_a_leer,"EAX")==0)
	{
		valor_leido= contexto_actual->registers.EAX;
	}else if(strcmp(registro_a_leer,"EBX")==0)
	{
		valor_leido= contexto_actual->registers.EBX;
	}else if(strcmp(registro_a_leer,"ECX")==0)
	{
		valor_leido= contexto_actual->registers.ECX;
	}else if(strcmp(registro_a_leer,"EDX")==0)
	{
		valor_leido= contexto_actual->registers.EDX;
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

    uint32_t origen_valor =   obtener_valor_del_registro(origen,contexto);
    uint32_t destino_valor =  obtener_valor_del_registro(destino,contexto);

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

    uint32_t origen_valor =   obtener_valor_del_registro(origen,contexto);
    uint32_t destino_valor =  obtener_valor_del_registro(destino,contexto);

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
    free(cadena);

}

// RESIZE (Tamaño): Solicitará a la Memoria ajustar el tamaño del proceso al tamaño 
// pasado por parámetro. En caso de que la respuesta de la memoria sea Out of Memory,
// se deberá devolver el contexto de ejecución al Kernel informando de esta situación.
void operacion_resize(t_pcb* contexto, t_instruccion_unitaria* instruccion)
{	
	//Recibo el nuevo tamaño del proceso que es el unico parametro de la instruccion
	int nuevoTamaProceso = atoi(instruccion->parametros[0]);

	//Envio el Pid y el nuevo tamaño a memoria
	t_buffer *buffer_resize;
    t_packet *packet_resize;
    buffer_resize = create_buffer();
    packet_resize = create_packet(RESIZE, buffer_resize);

    add_to_packet(packet_resize,&(contexto->pid),sizeof(int));
    add_to_packet(packet_resize,&nuevoTamaProceso,sizeof(int));
    
    send_packet(packet_resize, client_fd_memoria);
    destroy_packet(packet_resize);

	//Aca se bloquea esperando la respuesta de memoria
	int operation_code = fetch_codop(client_fd_memoria);

	//++++++++++++Recibo Buffer y Libero+++++++++++++++++++++++++++
	//Lo libero porque lo unico que me interesaba era el codigo de operacion
	int total_size;
	void *buffer2 = fetch_buffer(&total_size, client_fd_memoria);
	free(buffer2);
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	if(operation_code == RESIZE_EXITOSO){
		log_info(logger,"RESIZE EXITOSO");
	}else{
		//REZISE FALLIDO
	}
}

//IO_GEN_SLEEP (Interfaz, Unidades de trabajo): Esta instrucción solicita al Kernel que se
//envíe a una interfaz de I/O a que realice un sleep por una cantidad de unidades de trabajo.

void operacion_sleep(t_pcb *contexto,int socket,t_instruccion_unitaria* instruccion){
	t_buffer *buffer_rta;
    t_packet *packet_rta;
    buffer_rta = create_buffer();
    packet_rta = create_packet(SLEEP_IO,buffer_rta);

	contexto->state = BLOCKED;

	add_to_packet(packet_rta,instruccion->parametros[0], instruccion->parametro1_lenght); //CARGO EL NOMBRE DE LA INTERFAZ
	
	int valor = atoi(instruccion->parametros[1]);
    add_to_packet(packet_rta,&valor,sizeof(int)); //CARGO EL TIEMPO A DORMIR

	int tamanioPCB = sizeof(t_pcb);
    add_to_packet(packet_rta, contexto, tamanioPCB); //CARGO EL PCB ACTUALIZADO
	
	send_packet(packet_rta, socket);		//ENVIO EL PAQUETE
	destroy_packet(packet_rta);

}
