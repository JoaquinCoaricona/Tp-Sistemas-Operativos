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

int solicitarMarco(int numeroPagina, int pid){

	//Variable que va a recibir el marco
	int marcoEncontrado;

	//Envio el Pid y el numero de pagina a memoria
	t_buffer *bufferMarco;
    t_packet *packetMarco;
    bufferMarco = create_buffer();
    packetMarco = create_packet(SOLICITAR_MARCO, bufferMarco);

    add_to_packet(packetMarco,&numeroPagina,sizeof(int));
    add_to_packet(packetMarco,&pid,sizeof(int));
    
    send_packet(packetMarco, client_fd_memoria);
    destroy_packet(packetMarco);

	//-------------Aca se bloquea esperando el codop-------
	int operation_code = fetch_codop(client_fd_memoria);

	//----Separo en casos y Devuelvo dependiendo si encontre o no-----
	if(operation_code == DEVOLVER_MARCO){
		int total_size;
		int offset = 0;
		void *buffer2 = fetch_buffer(&total_size, client_fd_memoria);
		
    	offset += sizeof(int); //Salto el tamaño del INT
    
    	memcpy(&marcoEncontrado,buffer2 + offset, sizeof(int)); //RECIBO EL NUMERO DE MARCO
    	offset += sizeof(int);

		
		free(buffer2);

		return marcoEncontrado;
	}else{
		int total_size;
		void *buffer2 = fetch_buffer(&total_size, client_fd_memoria);
		free(buffer2);
		log_info(logger,"Error al solicitar Marco");
		return -1;
	}
	//---------------------------------------------------------------


}

int traduccionLogica(int pid, int direccion_logica){

	int numeroPagina = (int) floor(direccion_logica / tamaPagina);
	int desplazamiento = direccion_logica - numeroPagina * tamaPagina;
	int marco_pagina = solicitarMarco(numeroPagina,pid);

	log_info(logger, "Obtener Marco: PID: %d - OBTENER MARCO - Página: %d - Marco: %d", pid, numeroPagina, marco_pagina);
	return desplazamiento + marco_pagina * tamaPagina;
}

// MOV_OUT (Registro Dirección, Registro Datos): Lee el valor del
// Registro Datos y lo escribe en la dirección física de memoria obtenida a
// partir de la Dirección Lógica almacenada en el Registro Dirección.
void operacion_mov_out(t_pcb* contexto, t_instruccion_unitaria* instruccion)
{   
	//La cantidad de bits que voy a escribir en memoria es esto
	int cantidadBits;
	//Variable que va guardar lo que voy a escribir
	int valorEscribir;
	//Separo en casos Segun sea un registro de 4 o 1 byte
	if(
	string_equals_ignore_case(instruccion->parametros[1],"AX") || 
	string_equals_ignore_case(instruccion->parametros[1],"BX") ||
	string_equals_ignore_case(instruccion->parametros[1],"CX") ||
	string_equals_ignore_case(instruccion->parametros[1],"DX"))
	{
		valorEscribir = obtener_valor_del_registro(instruccion->parametros[1],contexto);
		cantidadBits = sizeof(uint8_t);
	}else{
		valorEscribir = obtener_valor_del_registro(instruccion->parametros[1],contexto);
		cantidadBits = sizeof(uint32_t);
	}
	int dirLogica = obtener_valor_del_registro(instruccion->parametros[0],contexto);

    int dirFisica = traduccionLogica(contexto->pid,dirLogica);

	t_buffer *bufferEscritura;
    t_packet *packetEscritura;
    bufferEscritura = create_buffer();
    packetEscritura = create_packet(SOLICITUD_ESCRIBIR, bufferEscritura);

    add_to_packet(packetEscritura,&dirFisica,sizeof(int));
    add_to_packet(packetEscritura,&valorEscribir,sizeof(int));
    add_to_packet(packetEscritura,&cantidadBits,sizeof(int));
	add_to_packet(packetEscritura,&(contexto->pid),sizeof(int));
    
    send_packet(packetEscritura, client_fd_memoria);
    destroy_packet(packetEscritura);

	//-------------Aca se bloquea esperando el codop-------
	int operation_code = fetch_codop(client_fd_memoria);

	if(operation_code == CONFIRMACION_ESCRITURA){
		int total_size;
		void *buffer2 = fetch_buffer(&total_size, client_fd_memoria);
		free(buffer2);
		log_info(logger,"Confirmacion Escritura");
	}else{
		int total_size;
		void *buffer2 = fetch_buffer(&total_size, client_fd_memoria);
		free(buffer2);
		log_info(logger,"Error en la escritura");
	}
	

}
// MOV_IN (Registro Datos, Registro Dirección): Lee el valor de memoria
// correspondiente a la Dirección Lógica que se encuentra en el Registro
// Dirección y lo almacena en el Registro Datos.
void operacion_mov_in(t_pcb* contexto, t_instruccion_unitaria* instruccion)
{   
	//La cantidad de bits que voy a escribir en memoria es esto
	int cantidadBits;
	//Variable que va guardar lo que voy a escribir
	int valorEscribir;
	//Separo en casos Segun sea un registro de 4 o 1 byte
	if(
	string_equals_ignore_case(instruccion->parametros[0],"AX") || 
	string_equals_ignore_case(instruccion->parametros[0],"BX") ||
	string_equals_ignore_case(instruccion->parametros[0],"CX") ||
	string_equals_ignore_case(instruccion->parametros[0],"DX"))
	{
		cantidadBits = sizeof(uint8_t);
	}else{
		cantidadBits = sizeof(uint32_t);
		
	}
	int dirLogica = obtener_valor_del_registro(instruccion->parametros[1],contexto);

    int dirFisica = traduccionLogica(contexto->pid,dirLogica);

	t_buffer *bufferLectura;
    t_packet *packetLectura;
    bufferLectura = create_buffer();
    packetLectura = create_packet(SOLICITUD_LECTURA, bufferLectura);

    add_to_packet(packetLectura,&dirFisica,sizeof(int));
    add_to_packet(packetLectura,&cantidadBits,sizeof(int));
	add_to_packet(packetLectura,&(contexto->pid),sizeof(int));
    
    send_packet(packetLectura, client_fd_memoria);
    destroy_packet(packetLectura);

	//-------------Aca se bloquea esperando el codop-------
	int operation_code = fetch_codop(client_fd_memoria);

	

	if(operation_code == CONFIRMACION_LECTURA){
		int total_size;
		int offset = 0;
		void *buffer2 = fetch_buffer(&total_size, client_fd_memoria);

		offset += sizeof(int); //Salteo el tamaño del INT

			if(cantidadBits == sizeof(uint8_t)){
				uint32_t valorAGuardar;
				memcpy(&valorAGuardar,buffer2 + offset,cantidadBits); //RECIBO LA DIRECCION FISICA
    			offset += sizeof(int);
				setear_registro(contexto,instruccion->parametros[0],valorAGuardar);
				log_info(logger,"Valor: %i", valorAGuardar);

			}else{
				uint32_t valorAGuardar;
				memcpy(&valorAGuardar,buffer2 + offset,cantidadBits); //RECIBO LA DIRECCION FISICA
    			offset += sizeof(int);
				setear_registro(contexto,instruccion->parametros[0],valorAGuardar);
				log_info(logger,"Valor: %i", valorAGuardar);
				
			}

		

		free(buffer2);
		log_info(logger,"Confirmacion Lectura");
	}else{
		int total_size;
		void *buffer2 = fetch_buffer(&total_size, client_fd_memoria);
		free(buffer2);
		log_info(logger,"Error en la lectura");
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

void operacion_read(t_pcb *contexto,int socket,t_instruccion_unitaria* instruccion) {
	int tamano;
	int cantidadBits;
	t_buffer *buffer;
    t_packet *packet;
    buffer = create_buffer();
    packet = create_packet(READ_IO, buffer);

	contexto->state = BLOCKED;

	if(
		string_equals_ignore_case(instruccion->parametros[2],"AX") || 
		string_equals_ignore_case(instruccion->parametros[2],"BX") ||
		string_equals_ignore_case(instruccion->parametros[2],"CX") ||
		string_equals_ignore_case(instruccion->parametros[2],"DX"))
	{
		tamano = obtener_valor_del_registro(instruccion->parametros[2],contexto);
		cantidadBits = sizeof(uint8_t);
	}else{
		tamano = obtener_valor_del_registro(instruccion->parametros[2],contexto);
		cantidadBits = sizeof(uint32_t);
	}

	int dirLogica = obtener_valor_del_registro(instruccion->parametros[1],contexto);
    int dirFisica = traduccionLogica(contexto->pid,dirLogica);

	add_to_packet(packet,instruccion->parametros[0], instruccion->parametro1_lenght); //Nombre Interfaz
    add_to_packet(packet,&dirFisica,sizeof(int)); //Direccion Fisica 
    add_to_packet(packet,&tamano,sizeof(int)); //Tamano maxima de escritura
    add_to_packet(packet,&cantidadBits,sizeof(int)); //Bits de escritura a memoria
	add_to_packet(packet,&(contexto->pid),sizeof(int)); //PCB actualizado
    
    send_packet(packet, socket);
    destroy_packet(packet);

}

void operacion_write(t_pcb *contexto,int socket,t_instruccion_unitaria* instruccion) {
	t_buffer *buffer;
    t_packet *packet;
    buffer = create_buffer();
    packet = create_packet(WRITE_IO, buffer);

	contexto->state = BLOCKED;

	int tamano = obtener_valor_del_registro(instruccion->parametros[1],contexto);
	int dirLogica = obtener_valor_del_registro(instruccion->parametros[2],contexto);
    int dirFisica = traduccionLogica(contexto->pid,dirLogica);

	add_to_packet(packet,instruccion->parametros[0], instruccion->parametro1_lenght); //Nombre Interfaz
    add_to_packet(packet,&dirFisica,sizeof(int)); //Direccion Fisica 
    add_to_packet(packet,&tamano,sizeof(int)); //Tamano maxima de escritura
	add_to_packet(packet,&(contexto->pid),sizeof(int)); //PCB actualizado
    
    send_packet(packet, socket);
    destroy_packet(packet);
}