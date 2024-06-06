#include "operaciones.h"

//Estas variables las uso en las busquedas en la TLB
int paginaTLB = -1;
int pidTLB = -1;

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
	else if(strcmp(registro,"SI")==0)
	{
		contexto->registers.SI=valor;
		//log_info(logger, "Se setea en %s el valor: %d",registro,  contexto->registers->EDX);
	}
	else if(strcmp(registro,"DI")==0)
	{
		contexto->registers.DI=valor;
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
	}else if(strcmp(registro_a_leer,"SI")==0)
	{
		valor_leido= contexto_actual->registers.SI;
	}else if(strcmp(registro_a_leer,"DI")==0)
	{
		valor_leido= contexto_actual->registers.DI;
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
bool busqueda_tlb(void *entradaTLB) {
		t_entrada_TLB *entrada = (t_entrada_TLB*) entradaTLB;

		if (entrada->pid == pidTLB && entrada->pagina == paginaTLB) {
			return true;
		}
		return false;
}
int solicitarMarco(int numeroPagina, int pid){

	//Variable que va a recibir el marco
	int marcoEncontrado;

	//+++++++++++++BUSCO EN LA TLB++++++++++++++++++++++++++++++++++++++++++++++
	pidTLB = pid;
	paginaTLB = numeroPagina;
	t_entrada_TLB *nuevaEntrada = list_find(TLB->elements,busqueda_tlb);

	if(nuevaEntrada != NULL){
		//Las dejo asi para que no molesten en futuras busquedas
		pidTLB = -1;
		paginaTLB = -1;
		log_info(logger,"PID: %i - TLB HIT - Pagina: %i",pid,numeroPagina);
		return nuevaEntrada->marco;
	}
	log_info(logger,"PID: %i - TLB MISS - Pagina: %i",pid,numeroPagina);
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	

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

		//Guardo el numero de marco en la TLB
		t_entrada_TLB *nuevaEntrada = malloc(sizeof(t_entrada_TLB));
		nuevaEntrada->pid = pid;
		nuevaEntrada->pagina = numeroPagina;
		nuevaEntrada->marco = marcoEncontrado;
		queue_push(TLB,nuevaEntrada);
		log_info(logger, "Obtener Marco: PID: %d - OBTENER MARCO - Página: %d - Marco: %d", pid, numeroPagina, marcoEncontrado);

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

	return desplazamiento + marco_pagina * tamaPagina;
}
//No necesita el pid, lo puse porque copie otra funcion como base
//y ahora ya esta siendo usada en varios lugares asi que lo dejo asi
int obtenerDesplazamiento(int pid, int direccion_logica){

	int numeroPagina = (int) floor(direccion_logica / tamaPagina);
	int desplazamiento = direccion_logica - numeroPagina * tamaPagina;
	return desplazamiento;
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
	//La direccion logica se obtiene directamente del registro
	int dirLogica = obtener_valor_del_registro(instruccion->parametros[0],contexto);
	//El desplazamiento solo serviria para el calculo de la dirFisica pero lo uso mas abajo
	int desplazamiento = obtenerDesplazamiento(contexto->pid,dirLogica);
	//esta funcion solo calcula la direccion fisica
    int dirFisica = traduccionLogica(contexto->pid,dirLogica);
	//Esto tambien solo deberia usarse para el calculo de la dirFisica pero lo termino usando aca
	int numeroPagina = (int) floor(dirLogica / tamaPagina);

	//Esta variable se usa en caso que tenga que escribir mas de una pagina
	//porque guarda lo que me queda por escribir de la primer pagina
	int diferencia = tamaPagina - desplazamiento;
	//Este va a ser el delimitador de el malloc contenido, la parte que falta escribir y la que no
	int desplazamientoContenido = 0;


	//Estas variables se usan cuando tengo que escribir mas de una pagina
	int nuevoMarco;
	int nuevaDirFisica;

	//Hago esto para poder mandarlo por partes, cadena de bytes
	void *contenidoAescribir = malloc(cantidadBits);
	//Copio el valor del registro en la cadena de bytes
	memcpy(contenidoAescribir,&valorEscribir, cantidadBits);


	//++++++++++++Calculo cantidad de Paginas a escribir++++++++++++++
	//Aca hago una comprobacion: si lo que tengo que escribir excede una pagina
	//entonces separo en casos. Aca desplazamiento seria lo que me muevo
	//Desde la base de la paginaa y si le sumo la cantidad de bytes (es un error en
	// el nombre de la variable) si le sumo eso me daria la direccion donde terminaria
	// de escribir, pero si eso se pasa de la pagina es que tengo que separar en casos
	//por un lado si se cumple esto entro por aca y si entra en una pagina voy por el else
	//y lo mando todo de una porque entra
		if((cantidadBits + desplazamiento) > tamaPagina){
			//Primero escribo lo que falta de la primera pagina
			//Escribo la cantidad diferencia porque es lo que falta de la primer pagina
			//osea el cachito que queda entre el desplazamineto y el fin de la pagina
			mandarAescribirEnMemoria(dirFisica,contenidoAescribir,diferencia,contexto);
			//Despues actualizo el desplazamietno contenido que es el delimitador entre
			//lo que ya escribir y lo que falta escribir del void (cadena de bytes)
			desplazamientoContenido = desplazamientoContenido + diferencia;
			//tambien actualizo la cantiydad de bytes que falta escribir
			cantidadBits = cantidadBits - diferencia;
			//Aumento el numero de pagina porque es la que sigue
			numeroPagina++;
			//Ahora hago otra comprobacion:ponele que tengo que escribir dos paginas y el cachito
			//de la primera, entonces como ya escribi ese pedacito de la primera,
			//ahora me fijo si la cantida de bytes que me falta es mayor a una pagina, si es
			//mayor entonces entro al bucle y hago lo mismo de vuelta
			while(cantidadBits > tamaPagina){
				//aca solicito el nuevo marco de la pagina nueva, le sume uno antes para 
				//marcar que ahora estoy en la siguiente
				nuevoMarco = solicitarMarco(numeroPagina,contexto->pid);
				//armo la nueva dir fisica pero sin desplazamiento porque 
				//empiezo desde el inicio en esta hoja, el desplazamiento solo era en la primera
				//que escribia
				nuevaDirFisica = nuevoMarco * tamaPagina;
				//No hay desplazamiento porque arranco la pagina nueva de 0
				//ahora mando a escribir la pagina completa
				mandarAescribirEnMemoria(nuevaDirFisica,contenidoAescribir + desplazamientoContenido,tamaPagina,contexto);
				//actualizo el desplazamiento sumando el tamaño de pagina que fue lo que escribi
				desplazamientoContenido = desplazamientoContenido + tamaPagina;
				//lo mismo, actualizo la cantidad de bytes que falta escribir
				cantidadBits = cantidadBits - tamaPagina;
				//paso a la siguiente pagina
				numeroPagina++;
			}
			//cuando salgo del bucle es porque me falta escribir una pagina pero no completa
			//entonces en la variable cantidad de bytes tengo lo que me falta de esa pagina
			//Primero pido el nuevo marco
			nuevoMarco = solicitarMarco(numeroPagina,contexto->pid);
			//ahora armo la direccion fisica nueva
			nuevaDirFisica = nuevoMarco * tamaPagina;
			//No hay desplazamiento porque arranco la pagina nueva de 0
			//Mando a escribir lo que falta
			mandarAescribirEnMemoria(nuevaDirFisica,contenidoAescribir + desplazamientoContenido,cantidadBits,contexto);
		}else{
			//En este caso es que todo entra en una pagina y no hay que hacer nada extra
			mandarAescribirEnMemoria(dirFisica,contenidoAescribir,cantidadBits,contexto);

		}
	
}

// MOV_IN (Registro Datos, Registro Dirección): Lee el valor de memoria
// correspondiente a la Dirección Lógica que se encuentra en el Registro
// Dirección y lo almacena en el Registro Datos.
void operacion_mov_in(t_pcb* contexto, t_instruccion_unitaria* instruccion)
{   
	//La cantidad de bits que voy a escribir en memoria es esto
	int cantidadBits;
	int tamaRegistro;
	int nuevoMarco;
	int nuevaDirFisica;
	//Separo en casos Segun sea un registro de 4 o 1 byte
	if(
	string_equals_ignore_case(instruccion->parametros[0],"AX") || 
	string_equals_ignore_case(instruccion->parametros[0],"BX") ||
	string_equals_ignore_case(instruccion->parametros[0],"CX") ||
	string_equals_ignore_case(instruccion->parametros[0],"DX"))
	{
		cantidadBits = sizeof(uint8_t);
		tamaRegistro = sizeof(uint8_t);
	}else{
		cantidadBits = sizeof(uint32_t);
		tamaRegistro = sizeof(uint32_t);
	}
	int dirLogica = obtener_valor_del_registro(instruccion->parametros[1],contexto);
	int dirFisica = traduccionLogica(contexto->pid,dirLogica);

	int desplazamiento = obtenerDesplazamiento(contexto->pid,dirLogica);
	int numeroPagina = (int) floor(dirLogica / tamaPagina);
	//Esto para leer lo que resta de la primera pagina
	int diferencia = tamaPagina - desplazamiento;

	//Este es el offset para escribir en contenidoLeido
	int desplazamientoContenido = 0;
	//Esto es para poder ir leyendo por partes, porque quizas tengo que leer varias paginas
	void *contenidoLeido = malloc(cantidadBits);
	//Entro por este if en el caso que tenga que leer mas de una pagina, es la misma cuenta
	//que es mov_out
	if((desplazamiento + cantidadBits) > tamaPagina){
		//mando a leer lo que resta de la primera pagina y actualizo los contadores
		mandarALeer(dirFisica,diferencia,contexto,contenidoLeido);
		cantidadBits = cantidadBits - diferencia;
		desplazamientoContenido = desplazamientoContenido + diferencia;
		numeroPagina++;
		//por aca entro cuando tengo que leer paginas enteras todavia
		while(cantidadBits > tamaPagina){
			nuevoMarco = solicitarMarco(numeroPagina,contexto->pid);
			nuevaDirFisica = nuevoMarco * tamaPagina;
			mandarALeer(nuevaDirFisica,tamaPagina,contexto,contenidoLeido + desplazamientoContenido);
			cantidadBits = cantidadBits - tamaPagina;
			desplazamientoContenido = desplazamientoContenido + tamaPagina;
			numeroPagina++;
		}
		//se llega aca cuando me queda leer una pagina mas pero no completa
		//entonces leo el pedazo que falta de la ultima
		nuevoMarco = solicitarMarco(numeroPagina,contexto->pid);
		nuevaDirFisica = nuevoMarco * tamaPagina;
		mandarALeer(nuevaDirFisica,cantidadBits,contexto,contenidoLeido + desplazamientoContenido);
		
	}else{
		//Se va por este caso cuando no tengo que leer dos paginas, todo de una
		mandarALeer(dirFisica,cantidadBits,contexto,contenidoLeido);
	}

	//Una vez terminado de leer, guardamos el contenido en el registro
	//Hago esto porque en el malloc reserve la memoria justa
	//y tengo que pasarle a setear registro un uint32, entonces hago el memcpy
	//en la variable que corresponde segun hice el malloc, y despues en el caso del
	//uint8 lo guardo en un uint32 porque se ppuede y ahi hago el set del registro
	if(tamaRegistro == sizeof(uint8_t)){
		//PONER LOS & EN EL PRIMER PARAMETRO DE MEMCPY SI PASAS UNA VARIABLE Y NO UN PUNTERO !!!!!!!!!!!!
		uint8_t tama8;
		memcpy(&tama8,contenidoLeido,sizeof(uint8_t)); 
		uint32_t valorAGuardar = tama8;
		setear_registro(contexto,instruccion->parametros[0],valorAGuardar);
	}else{
		//PONER LOS & EN EL PRIMER PARAMETRO DE MEMCPY SI PASAS UNA VARIABLE Y NO UN PUNTERO !!!!!!!!!!!!
		uint32_t valorAGuardar;
		memcpy(&valorAGuardar,contenidoLeido,sizeof(uint32_t)); 
		setear_registro(contexto,instruccion->parametros[0],valorAGuardar);	
	}

}

void mandarAescribirEnMemoria(int dirFisica,void *contenidoAescribir, int cantidadBits,t_pcb *contexto){
	t_buffer *bufferEscritura;
    t_packet *packetEscritura;
    bufferEscritura = create_buffer();
    packetEscritura = create_packet(SOLICITUD_ESCRIBIR, bufferEscritura);

    add_to_packet(packetEscritura,&dirFisica,sizeof(int));
    add_to_packet(packetEscritura,&cantidadBits,sizeof(int));
    add_to_packet(packetEscritura,contenidoAescribir,cantidadBits);
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
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}

void mandarALeer(int dirFisica, int cantidadBits, t_pcb *contexto, void *contenido){

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

		memcpy(contenido,buffer2 + offset,cantidadBits); 
    	offset += cantidadBits; //Este offset no tiene sentido pero lo pongo por las dudas
		//Esto iria pero como son numeros quizas justo leo la mitad y no tendria sentido imprimir la mitad
		//log_info(logger,"Valor: %i", contenido);

		free(buffer2);
		log_info(logger,"Confirmacion Lectura");
	}else{
		int total_size;
		void *buffer2 = fetch_buffer(&total_size, client_fd_memoria);
		free(buffer2);
		log_info(logger,"Error en la lectura");
	}
}
// COPY_STRING (Tamaño): Toma del string apuntado por el registro SI y copia la
// cantidad de bytes indicadas en el parámetro tamaño a la posición de memoria apuntada
// por el registro DI. 
void operacion_copy_string(t_pcb* contexto, t_instruccion_unitaria* instruccion){

//*********************************************************************************************
//***********************PRIMERA ETAPA: LECTURA*************************************
//*********************************************************************************************

	//La cantidad de bits que voy a escribir en memoria es esto
	int cantidadBits = atoi(instruccion->parametros[0]);
	//Esta variable la guardo para la etapa de la escritura
	int bytesTotal = atoi(instruccion->parametros[0]);

	int nuevoMarco;
	int nuevaDirFisica;

	int dirLogica = obtener_valor_del_registro("SI",contexto);
	int dirFisica = traduccionLogica(contexto->pid,dirLogica);

	int desplazamiento = obtenerDesplazamiento(contexto->pid,dirLogica);
	int numeroPagina = (int) floor(dirLogica / tamaPagina);

	//Esto para leer lo que resta de la primera pagina, solo en caso que se escriba mas de una pagina
	int diferencia = tamaPagina - desplazamiento;

	//Este es el offset para escribir en contenidoLeido
	int desplazamientoContenido = 0;

	void *contenidoLeido = malloc(cantidadBits);

	if((desplazamiento + cantidadBits) > tamaPagina){
		mandarALeer(dirFisica,diferencia,contexto,contenidoLeido);
		cantidadBits = cantidadBits - diferencia;
		desplazamientoContenido = desplazamientoContenido + diferencia;
		numeroPagina++;

		while(cantidadBits > tamaPagina){
			nuevoMarco = solicitarMarco(numeroPagina,contexto->pid);
			nuevaDirFisica = nuevoMarco * tamaPagina;
			mandarALeer(nuevaDirFisica,tamaPagina,contexto,contenidoLeido + desplazamientoContenido);
			cantidadBits = cantidadBits - tamaPagina;
			desplazamientoContenido = desplazamientoContenido + tamaPagina;
			numeroPagina++;
		}
		nuevoMarco = solicitarMarco(numeroPagina,contexto->pid);
		nuevaDirFisica = nuevoMarco * tamaPagina;
		mandarALeer(nuevaDirFisica,cantidadBits,contexto,contenidoLeido + desplazamientoContenido);
		
	}else{
		mandarALeer(dirFisica,cantidadBits,contexto,contenidoLeido);
	}
//*********************************************************************************************
//***********************SEGUNDA ETAPA: ESCRITURA**********************************************
//*********************************************************************************************

	int dirLogicaEscritura = obtener_valor_del_registro("DI",contexto);
	int desplazamientoEscritura = obtenerDesplazamiento(contexto->pid,dirLogicaEscritura);
    int dirFisicaEscritura = traduccionLogica(contexto->pid,dirLogicaEscritura);
	int numeroPaginaEscritura = (int) floor(dirLogicaEscritura / tamaPagina);

	//Solo se usa si escribo mas de una pagina
	int diferenciaEscritura = tamaPagina - desplazamientoEscritura;

	//Esto es para delimitar la parte que ya escribi, si es mas de una pagina
	//Uso la misma variable de la primera etapa, solo que la vuelvo a poner en 0
	desplazamientoContenido = 0;

	//CONTENIDOLEIDO ES EL PUNTERO DONDE TENGO LO LEIDO Y LO QUE VOY A COPIAR ACA
	
	//Esto se usa cuando el dato no entra en una pagina
	int nuevoMarcoEscritura;
	int nuevaDirFisicaEscritura;
	//Vuelvo a asginarle a cantidadBits el valor original, porque cambio en la lectura 
	cantidadBits = bytesTotal;

	//++++++++++++Calculo cantidad de Paginas a escribir++++++++++++++
		if((bytesTotal + desplazamientoEscritura) > tamaPagina){
			//Como entro por este if significa que ...
			//Primero escribo lo que falta de la primera pagina
			mandarAescribirEnMemoria(dirFisicaEscritura,contenidoLeido,diferenciaEscritura,contexto);
			desplazamientoContenido = desplazamientoContenido + diferenciaEscritura;
			cantidadBits = cantidadBits - diferenciaEscritura;
			numeroPaginaEscritura++;
			//Ahora escribo el resto
			while(cantidadBits > tamaPagina){
				
				nuevoMarcoEscritura = solicitarMarco(numeroPaginaEscritura,contexto->pid);
				nuevaDirFisicaEscritura = nuevoMarcoEscritura * tamaPagina;
				//No hay desplazamiento porque arranco la pagina nueva de 0
				mandarAescribirEnMemoria(nuevaDirFisicaEscritura,contenidoLeido + desplazamientoContenido,tamaPagina,contexto);
				desplazamientoContenido = desplazamientoContenido + tamaPagina;
				cantidadBits = cantidadBits - tamaPagina;
				numeroPaginaEscritura++;
			}
			
			nuevoMarcoEscritura = solicitarMarco(numeroPaginaEscritura,contexto->pid);
			nuevaDirFisicaEscritura = nuevoMarcoEscritura * tamaPagina;
			//No hay desplazamiento porque arranco la pagina nueva de 0
			mandarAescribirEnMemoria(nuevaDirFisicaEscritura,contenidoLeido + desplazamientoContenido,cantidadBits,contexto);
		}else{
			//En este caso es que todo entra en una pagina y no hay que hacer nada extra
			mandarAescribirEnMemoria(dirFisicaEscritura,contenidoLeido,cantidadBits,contexto);

		}
//*********************************************************************************************

}

// IO_STDOUT_WRITE (Interfaz, Registro Dirección, Registro Tamaño):
// Esta instrucción solicita al Kernel que mediante la interfaz seleccionada,
// se lea desde la posición de memoria indicada por la Dirección Lógica almacenada
// en el Registro Dirección, un tamaño indicado por el Registro Tamaño y se imprima por pantalla.


void operacion_io_stdout_write(t_pcb *contexto,int socket,t_instruccion_unitaria* instruccion){
	
	t_buffer *buffer_rta;
	t_packet *packet_rta;
	buffer_rta = create_buffer();
	packet_rta = create_packet(STDOUT_ESCRIBIR,buffer_rta);
	add_to_packet(packet_rta,instruccion->parametros[0], instruccion->parametro1_lenght); //CARGO EL NOMBRE DE LA INTERFAZ
	
	
	int dirLogica = obtener_valor_del_registro(instruccion->parametros[1],contexto);
	int cantidadBytes = obtener_valor_del_registro(instruccion->parametros[2],contexto);

	int nuevoMarco;
	int nuevaDirFisica;

	int dirFisica = traduccionLogica(contexto->pid,dirLogica);
	int desplazamiento = obtenerDesplazamiento(contexto->pid,dirLogica);
	int numeroPagina = (int) floor(dirLogica / tamaPagina);
	add_to_packet(packet_rta,&cantidadBytes,sizeof(int));

	//Calculo y agrego la cantidad de direcciones Fisicas que voy a enviar
	int cantidadDireccionesFisicas = calcularCantDirFisicas(desplazamiento,cantidadBytes);
	add_to_packet(packet_rta,&cantidadDireccionesFisicas,sizeof(int));

	//Esto para leer lo que resta de la primera pagina, solo en caso que se escriba mas de una pagina
	int diferencia = tamaPagina - desplazamiento;


	if((desplazamiento + cantidadBytes) > tamaPagina){
		
		add_to_packet(packet_rta,&diferencia,sizeof(int));
		add_to_packet(packet_rta,&dirFisica,sizeof(int));
		cantidadBytes = cantidadBytes - diferencia;
		numeroPagina++;

		while(cantidadBits > tamaPagina){
			nuevoMarco = solicitarMarco(numeroPagina,contexto->pid);
			nuevaDirFisica = nuevoMarco * tamaPagina;
			add_to_packet(packet_rta,&tamaPagina,sizeof(int));
			add_to_packet(packet_rta,&nuevaDirFisica,sizeof(int));
			cantidadBytes = cantidadBytes - tamaPagina;
			numeroPagina++;
		}
		nuevoMarco = solicitarMarco(numeroPagina,contexto->pid);
		nuevaDirFisica = nuevoMarco * tamaPagina;
		add_to_packet(packet_rta,&cantidadBytes,sizeof(int));
		add_to_packet(packet_rta,&nuevaDirFisica,sizeof(int));
		
		
	}else{
		add_to_packet(packet_rta,&cantidadBytes,sizeof(int));
		add_to_packet(packet_rta,&dirFisica,sizeof(int));
	}
	
	//Ahora agrego el PCB al paquete

	contexto->state = BLOCKED;
	int tamanioPCB = sizeof(t_pcb);
    add_to_packet(packet_rta, contexto, tamanioPCB); //CARGO EL PCB ACTUALIZADO
	
	send_packet(packet_rta,socket);		//ENVIO EL PAQUETE
	destroy_packet(packet_rta);

}

int calcularCantDirFisicas(int desplazamiento, int cantidadBytes){

	//Empieza en 0 aunque deberia empezar en 1 quizas
		int contadorPaginas = 0;
		int diferencia = tamaPagina - desplazamiento;
		//Lo mismo que al leer, si entra por el if es que se va a pasar de una pagina
		if((desplazamiento + cantidadBytes) > tamaPagina){
		//Aca hace la resta para actualizar lo que falta
		cantidadBytes = cantidadBytes - diferencia;
		//Sumo uno a la cantidad de paginas porque "lei" lo que restaba de la primera
		contadorPaginas++;
		//Ahora voy a "leer" paginas enteras solo en caso tenga paginas enteras restantes
		//y sumo uno por cada una
		while(cantidadBits > tamaPagina){
			cantidadBytes = cantidadBytes - tamaPagina;
			contadorPaginas++;
		}
		//En caso que no tenga paginas enteras para leer y solo sea un cachito de la siguiente
		//sumo uno y devuelvo
		contadorPaginas++;
		//por este lado como minimo vas a devolver dos paginas porque te pasabas de la original
		return contadorPaginas;
	}else{
		//por aca solo "lees" una pagina, porque no te pasabas del limite. Asi que devuelvo uno
		contadorPaginas++;
		return contadorPaginas;
	}
}