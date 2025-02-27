#include "operaciones.h"

//Estas variables las uso en las busquedas en la TLB
int paginaTLB = -1;
int pidTLB = -1;
char *tlbAlgoritmo;
uint32_t numeroGlobalLog = 0;
char *auxiliarLog = NULL;
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
	else if(strcmp(registro,"PC")==0)
	{
		contexto->registers.PC=valor;
		//Aca cambio el Program counter del pcb. El programcounter registro
		//no lo uso nunca, por eso aca cambio el del pcb
		//El que esta en registros solo lo toco aca pero en ningun lado mas.
		//Lo importantes era actualizar el valor del PC del pcb que es el que se usa en el ciclo
		contexto->program_counter = valor;
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
	log_info(logOficialCpu,"Instrucción Ejecutada: PID: %i - Ejecutando: SET - %s %s",contexto->pid,instruccion->parametros[0],instruccion->parametros[1]);
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
	log_info(logOficialCpu,"Instrucción Ejecutada: PID: %i - Ejecutando: SUM - %s %s",contexto->pid,instruccion->parametros[0],instruccion->parametros[1]);
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
	log_info(logOficialCpu,"Instrucción Ejecutada: PID: %i - Ejecutando: SUB - %s %s",contexto->pid,instruccion->parametros[0],instruccion->parametros[1]);
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
	log_info(logOficialCpu,"Instrucción Ejecutada: PID: %i - Ejecutando: JNZ - %s %s",contexto->pid,instruccion->parametros[0],instruccion->parametros[1]);
    free(cadena);

}

// RESIZE (Tamaño): Solicitará a la Memoria ajustar el tamaño del proceso al tamaño 
// pasado por parámetro. En caso de que la respuesta de la memoria sea Out of Memory,
// se deberá devolver el contexto de ejecución al Kernel informando de esta situación.
void operacion_resize(t_pcb* contexto, t_instruccion_unitaria* instruccion,int socket_kernel)
{	
	//Recibo el nuevo tamaño del proceso que es el unico parametro de la instruccion
	int nuevoTamaProceso = atoi(instruccion->parametros[0]);
	log_info(logOficialCpu,"Instrucción Ejecutada: PID: %i - Ejecutando: RESIZE - %s",contexto->pid,instruccion->parametros[0]);
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

	//++++++++++++Recibo Buffer+++++++++++++++++++++++++++
	int total_size;
	int offset = 0;
	//ACLARACION MUY IMPORTANTE SOBRE ENVIO DE PAQUETE Y RECEPCION.
	//ESTA HECHA EN RESIZEPAGINAS() EN EL MAIN DE MEMORIA
	//CERCA DEL FINAL DE LA FUNCION DONDE SEPARO EN CASOS
	//POR SI HUBO REUDCCION DE PROCESO Y SE BORRARON PAGINAS
	void *buffer = fetch_buffer(&total_size, client_fd_memoria);

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	if(operation_code == RESIZE_EXITOSO){
		log_info(logger,"RESIZE EXITOSO");

		offset += sizeof(int);  //Salteo el tamaño de int
		//Recibo el numero de confirmacion que mando al inicio del paquete
		int numeroConfirmacion;
		memcpy(&numeroConfirmacion,buffer + offset, sizeof(int));
		offset += sizeof(int); 

		//Si es igual a 1 significa que hubo reudccion y algunas paginas se borraron
		//entonces tengo que ver si esas paginas estaban en la TLB porque sino tendriamos TLB
		//HITS de mas y accesos invalidos a memoria
		//HABIAN DICHO QUE ESTO NO IBA A PASAR. OSEA QUE NO IBA A HABER ACCESOS INVALIDOS
		//O PAGE FAULT ENTONCES ESTE CASO NO LO TENIAMOS EN CUENTA
		//PERO HICIERON UNA FUNCION QUE ERA UN LOOP Y QUE AL FINAL HACIA
		//RESIZE O Y AL PRINCIPIO RESIZE 60 (NO SE SI ESE NUMERO).
		//Y ESO GENERABA ACCESOS INVALIDOS PORQUE CUANDO HACES RESIZE 0 Y DESPUES
		//ASGINAS NO SIEMPRE VAN A SER LOS MISMOS MARCOS QUE TENIAS ANTES
		//ENTONCES LA TLB TE QUEDO DESACTUALIZADA Y TENES ACCESOS INVALIDOS
		//ESTO NO LO TOME EN CUENTA PORQUE HABIAN DICHO QUE NO IBA A PASAR PERO 
		//LO PUSIERON EN LA ULTIMA PRUEBA
		if(numeroConfirmacion == 1){

			offset += sizeof(int);  //Salteo el tamaño de int

			//Aca recibo la cantidad de paginas que borre
			int cantPaginasBorradas;
			memcpy(&cantPaginasBorradas,buffer + offset, sizeof(int));
			offset += sizeof(int); 

			for(int i = 0; i < cantPaginasBorradas; i++){

				offset += sizeof(int);  //Salteo el tamaño de int

				int paginaABorrar;
				memcpy(&paginaABorrar,buffer + offset, sizeof(int));
				offset += sizeof(int); 

				//Cargo los datos para la busqueda en la TLB
				pidTLB = contexto->pid;
				paginaTLB = paginaABorrar;
				//Busco en la TLB
				t_entrada_TLB *entradaABorrar = list_find(TLB->elements,busqueda_tlb);
				//Si encuentra algo entonces lo borro
				if(entradaABorrar != NULL){
					list_remove_element(TLB->elements,entradaABorrar);
					log_info(logger,"Se borro la Pagina: %i de PID: %i de la TLB",pidTLB,paginaTLB);
					free(entradaABorrar);	
				}else{
					log_info(logger,"Borraron una pagina pero no estaba en la TLB");
				}

			}
		}

	}else{
		continuar_con_el_ciclo_instruccion = false;
		pid_ejecutando = -1;
		
		//----------Envio el PCB a Kernel-----------------------
		t_buffer *buffer_rta;
    	t_packet *packet_rta;
    	buffer_rta = create_buffer();
    	packet_rta = create_packet(OUT_MEMORY,buffer_rta);

		int length_motivo = (strlen("Out of Memory") + 1);
		//No se si va funcionar el add to packet asi, pero quizas si passarle diectamente el ""
		//quizas devuelve el puntero a lo escrito
		add_to_packet(packet_rta,"Out of Memory", length_motivo);

    	int tamanioPCB = sizeof(t_pcb);
    	add_to_packet(packet_rta, contexto, tamanioPCB); //CARGO EL PCB ACTUALIZADO
	
    	send_packet(packet_rta, socket_kernel);
    	destroy_packet(packet_rta);
		//--------------------------------------------------------

		log_info(logger,"RESIZE FALLIDO, OUT OF MEMORY");
	}
	free(buffer);
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
	log_info(logOficialCpu,"Instrucción Ejecutada: PID: %i - Ejecutando: IO_GEN_SLEEP - %s %s",contexto->pid,instruccion->parametros[0],instruccion->parametros[1]);

}
bool busqueda_tlb(void *entradaTLB){
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
		log_info(logOficialCpu,"PID: %i - TLB HIT - Pagina: %i",pid,numeroPagina);
		//++++++++++++Si es LRU Actualizo la tabla+++++++++++++++++++++
		//Hago esto porque en LRU voy a borrar el que no se usa o el que se uso 
		//por ultima vez hace mucho tiempo. Priorizo guardar los que
		//se uasron mas seguido.
		//Entonces aca cuando encontre uno (osea que lo voy a usar)
		//lo saco de la lista y le vuelvo a hacer push y asi con todos los que use
		//entonces el que menos use va a quedar al final y lo saco con pop
		//como si fuera fifo. Es como ir reordenando la cola con cada uso.
		//si es fifo no hago nada porque los voy metiendo con push y al eliminar es con pop
		//haciendo esto al sacar tambien saco con un pop solamente
		//char *hola = algoritmoTLB;
		if(string_equals_ignore_case(tlbAlgoritmo,"LRU")){
			list_remove_element(TLB->elements,nuevaEntrada);
			queue_push(TLB,nuevaEntrada);
			log_info(logger,"Uso entrada TLB y muevo el orden de entradas");
        }
		//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		return nuevaEntrada->marco;
	}
	log_info(logger,"PID: %i - TLB MISS - Pagina: %i",pid,numeroPagina);
	log_info(logOficialCpu,"PID: %i - TLB MISS - Pagina: %i",pid,numeroPagina);
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
		agregarALaTLB(TLB,nuevaEntrada);
		log_info(logger, "Obtener Marco: PID: %d - OBTENER MARCO - Página: %d - Marco: %d", pid, numeroPagina, marcoEncontrado);
		log_info(logOficialCpu, "Obtener Marco: PID: %d - OBTENER MARCO - Página: %d - Marco: %d", pid, numeroPagina, marcoEncontrado);
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
void agregarALaTLB(t_queue *TLB ,t_entrada_TLB *nuevaEntrada){
	//Agrego el distinto de cero para el caso que la cant de entradas de la tlb sea 0
	//crei que no se podia pero lo incluyeron. Si es 0 no se tiene que agregar nada
	//en ninguno de los dos casos. Asi las busquedas en la tlb siempre dan NULL
	//Cambio el else que tenia por un elseif para agregar el caso de tlb cant != 0
	if((queue_size(TLB) >= cantEntradasTLB) && (cantEntradasTLB != 0)){
		//Aca hago lo mismo si es LRU o FIFO porque al usarlos
		//voy acomodando en caso que sea LRU. Asi en los dos casos
		//queda para eliminar con un pop, sea LRU o FIFO
		t_entrada_TLB *entradaAEliminar = NULL;
		entradaAEliminar = queue_pop(TLB);
		queue_push(TLB,nuevaEntrada);
		log_info(logger,"Borro una entrada de la TLB para agregar otra");
		free(entradaAEliminar);
	}else if(cantEntradasTLB != 0){
		queue_push(TLB,nuevaEntrada);
	}
	else{
		//En caso que la cant entradas TLB sea 0 entonces no pasaba nada
		//aca, pero tengo que liberar esto porque generaba muchos bytes perdidos
		//en valgrind. Ahora como no lo agregamos a la TLB porque tiene 0
		//espacios, entonces libero la memoria
		free(nuevaEntrada);
	}
	
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
	log_info(logOficialCpu,"Instrucción Ejecutada: PID: %i - Ejecutando: MOV_OUT - %s %s",contexto->pid,instruccion->parametros[0],instruccion->parametros[1]);
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
			memcpy(&numeroGlobalLog,contenidoAescribir,diferencia);
			log_info(logOficialCpu,"PID: <%i> - Acción: ESCRIBIR - Dirección Física: %i - Valor: %i",contexto->pid,dirFisica,numeroGlobalLog);
			//Despues actualizo el desplazamietno contenido que es el delimitador entre
			//lo que ya escribir y lo que falta escribir del void (cadena de bytes)
			desplazamientoContenido = desplazamientoContenido + diferencia;
			//tambien actualizo la cantiydad de bytes que falta escribirS
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
				memcpy(&numeroGlobalLog,contenidoAescribir + desplazamientoContenido,tamaPagina);
				log_info(logOficialCpu,"PID: <%i> - Acción: ESCRIBIR - Dirección Física: %i - Valor: %i",contexto->pid,nuevaDirFisica,numeroGlobalLog);
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
			memcpy(&numeroGlobalLog,contenidoAescribir + desplazamientoContenido,cantidadBits);
			log_info(logOficialCpu,"PID: <%i> - Acción: ESCRIBIR - Dirección Física: %i - Valor: %i",contexto->pid,nuevaDirFisica,numeroGlobalLog);
		}else{
			//En este caso es que todo entra en una pagina y no hay que hacer nada extra
			mandarAescribirEnMemoria(dirFisica,contenidoAescribir,cantidadBits,contexto);
			memcpy(&numeroGlobalLog,contenidoAescribir,cantidadBits);
			log_info(logOficialCpu,"PID: <%i> - Acción: ESCRIBIR - Dirección Física: %i - Valor: %i",contexto->pid,dirFisica,numeroGlobalLog);
		}
	free(contenidoAescribir); //Libero por valgrind
	
}

// MOV_IN (Registro Datos, Registro Dirección): Lee el valor de memoria
// correspondiente a la Dirección Lógica que se encuentra en el Registro
// Dirección y lo almacena en el Registro Datos.
void operacion_mov_in(t_pcb* contexto, t_instruccion_unitaria* instruccion)
{   
	log_info(logOficialCpu,"Instrucción Ejecutada: PID: %i - Ejecutando: MOV_IN - %s %s",contexto->pid,instruccion->parametros[0],instruccion->parametros[1]);
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
		//++++++++++++Solo para el LOG+++++++++++++++++++++++++++++++++++++++++++++++++++++++
		memcpy(&numeroGlobalLog,contenidoLeido,diferencia);
		log_info(logOficialCpu,"PID: <%i> - Acción: LEER - Dirección Física: %i - Valor: %d",contexto->pid,dirFisica,numeroGlobalLog);
		//++++++++++++++++++++++++++++++++++++++++
		cantidadBits = cantidadBits - diferencia;
		desplazamientoContenido = desplazamientoContenido + diferencia;
		numeroPagina++;
		//por aca entro cuando tengo que leer paginas enteras todavia
		while(cantidadBits > tamaPagina){
			nuevoMarco = solicitarMarco(numeroPagina,contexto->pid);
			nuevaDirFisica = nuevoMarco * tamaPagina;
			mandarALeer(nuevaDirFisica,tamaPagina,contexto,contenidoLeido + desplazamientoContenido);
			//++++++++++++++++++++++SOLO PARA EL LOG++++++++++++++++++++++++++++++++
			memcpy(&numeroGlobalLog,contenidoLeido + desplazamientoContenido,tamaPagina);
			log_info(logOficialCpu,"PID: <%i> - Acción: LEER - Dirección Física: %i - Valor: %d",contexto->pid,nuevaDirFisica,numeroGlobalLog);
			//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			cantidadBits = cantidadBits - tamaPagina;
			desplazamientoContenido = desplazamientoContenido + tamaPagina;
			numeroPagina++;
		}
		//se llega aca cuando me queda leer una pagina mas pero no completa
		//entonces leo el pedazo que falta de la ultima
		nuevoMarco = solicitarMarco(numeroPagina,contexto->pid);
		nuevaDirFisica = nuevoMarco * tamaPagina;
		mandarALeer(nuevaDirFisica,cantidadBits,contexto,contenidoLeido + desplazamientoContenido);
		//++++++++++++++++++++++SOLO PARA EL LOG++++++++++++++++++++++++++++++++
		//Aca habia sumando un puntero para escribri al final pero no va eso, solo quiero
		//imprimir la parte que se escribio, no reconstruilo. Eso se hace mas abajo y se guarda en el registro
		memcpy(&numeroGlobalLog,contenidoLeido + desplazamientoContenido,cantidadBits);
		log_info(logOficialCpu,"PID: <%i> - Acción: LEER - Dirección Física: %i - Valor: %d",contexto->pid,nuevaDirFisica,numeroGlobalLog);
		//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		
	}else{
		//Se va por este caso cuando no tengo que leer dos paginas, todo de una
		mandarALeer(dirFisica,cantidadBits,contexto,contenidoLeido);
		//++++++++++++++++++++++SOLO PARA EL LOG++++++++++++++++++++++++++++++++
		memcpy(&numeroGlobalLog,contenidoLeido,cantidadBits);
		log_info(logOficialCpu,"PID: <%i> - Acción: LEER - Dirección Física: %i - Valor: %d",contexto->pid,dirFisica,numeroGlobalLog);
		//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
	free(contenidoLeido); //Libero por valgrind

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
log_info(logOficialCpu,"Instrucción Ejecutada: PID: %i - Ejecutando: COPY_STRING - %s",contexto->pid,instruccion->parametros[0]);
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
		
		//++++++++++++++++++Esto es solo para el log++++++++++++++++++++
		auxiliarLog = malloc(diferencia + 1);
		memcpy(auxiliarLog,contenidoLeido,diferencia);
		auxiliarLog[diferencia] = '\0';
		log_info(logOficialCpu,"PID: <%i> - Acción: LEER - Dirección Física: %i - Valor: %s",contexto->pid,dirFisica,auxiliarLog);
		free(auxiliarLog);
		//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

		cantidadBits = cantidadBits - diferencia;
		desplazamientoContenido = desplazamientoContenido + diferencia;
		numeroPagina++;

		while(cantidadBits > tamaPagina){
			nuevoMarco = solicitarMarco(numeroPagina,contexto->pid);
			nuevaDirFisica = nuevoMarco * tamaPagina;
			mandarALeer(nuevaDirFisica,tamaPagina,contexto,contenidoLeido + desplazamientoContenido);
			
			//++++++++++++++++++Esto es solo para el log++++++++++++++++++++
			auxiliarLog = malloc(tamaPagina + 1);
			memcpy(auxiliarLog,contenidoLeido + desplazamientoContenido,tamaPagina);
			auxiliarLog[tamaPagina] = '\0';
			log_info(logOficialCpu,"PID: <%i> - Acción: LEER - Dirección Física: %i - Valor: %s",contexto->pid,nuevaDirFisica,auxiliarLog);
			free(auxiliarLog);
			//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			cantidadBits = cantidadBits - tamaPagina;
			desplazamientoContenido = desplazamientoContenido + tamaPagina;
			numeroPagina++;
		}
		nuevoMarco = solicitarMarco(numeroPagina,contexto->pid);
		nuevaDirFisica = nuevoMarco * tamaPagina;
		mandarALeer(nuevaDirFisica,cantidadBits,contexto,contenidoLeido + desplazamientoContenido);
		
		//++++++++++++++++++Esto es solo para el log++++++++++++++++++++
		auxiliarLog = malloc(cantidadBits + 1);
		memcpy(auxiliarLog,contenidoLeido + desplazamientoContenido,cantidadBits);
		auxiliarLog[cantidadBits] = '\0';
		log_info(logOficialCpu,"PID: <%i> - Acción: LEER - Dirección Física: %i - Valor: %s",contexto->pid,nuevaDirFisica,auxiliarLog);
		free(auxiliarLog);
		//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		
	}else{
		mandarALeer(dirFisica,cantidadBits,contexto,contenidoLeido);
		//++++++++++++++++++Esto es solo para el log++++++++++++++++++++
		auxiliarLog = malloc(cantidadBits + 1);
		memcpy(auxiliarLog,contenidoLeido,cantidadBits);
		auxiliarLog[cantidadBits] = '\0';
		log_info(logOficialCpu,"PID: <%i> - Acción: LEER - Dirección Física: %i - Valor: %s",contexto->pid,dirFisica,auxiliarLog);
		free(auxiliarLog);
		//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
			
			//++++++++++++++++++Esto es solo para el log++++++++++++++++++++
			auxiliarLog = malloc(diferenciaEscritura + 1);
			memcpy(auxiliarLog,contenidoLeido,diferenciaEscritura);
			auxiliarLog[diferenciaEscritura] = '\0';
			log_info(logOficialCpu,"PID: <%i> - Acción: ESCRIBIR - Dirección Física: %i - Valor: %s",contexto->pid,dirFisicaEscritura,auxiliarLog);
			free(auxiliarLog);
			//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

			desplazamientoContenido = desplazamientoContenido + diferenciaEscritura;
			cantidadBits = cantidadBits - diferenciaEscritura;
			numeroPaginaEscritura++;
			//Ahora escribo el resto
			while(cantidadBits > tamaPagina){
				
				nuevoMarcoEscritura = solicitarMarco(numeroPaginaEscritura,contexto->pid);
				nuevaDirFisicaEscritura = nuevoMarcoEscritura * tamaPagina;
				//No hay desplazamiento porque arranco la pagina nueva de 0
				mandarAescribirEnMemoria(nuevaDirFisicaEscritura,contenidoLeido + desplazamientoContenido,tamaPagina,contexto);
				//++++++++++++++++++Esto es solo para el log++++++++++++++++++++
				auxiliarLog = malloc(tamaPagina + 1);
				memcpy(auxiliarLog,contenidoLeido + desplazamientoContenido,tamaPagina);
				auxiliarLog[tamaPagina] = '\0';
				log_info(logOficialCpu,"PID: <%i> - Acción: ESCRIBIR - Dirección Física: %i - Valor: %s",contexto->pid,nuevaDirFisicaEscritura,auxiliarLog);
				free(auxiliarLog);
				//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				desplazamientoContenido = desplazamientoContenido + tamaPagina;
				cantidadBits = cantidadBits - tamaPagina;
				numeroPaginaEscritura++;
			}
			
			nuevoMarcoEscritura = solicitarMarco(numeroPaginaEscritura,contexto->pid);
			nuevaDirFisicaEscritura = nuevoMarcoEscritura * tamaPagina;
			//No hay desplazamiento porque arranco la pagina nueva de 0
			mandarAescribirEnMemoria(nuevaDirFisicaEscritura,contenidoLeido + desplazamientoContenido,cantidadBits,contexto);
			//++++++++++++++++++Esto es solo para el log++++++++++++++++++++
			auxiliarLog = malloc(cantidadBits + 1);
			memcpy(auxiliarLog,contenidoLeido + desplazamientoContenido,cantidadBits);
			auxiliarLog[cantidadBits] = '\0';
			log_info(logOficialCpu,"PID: <%i> - Acción: ESCRIBIR - Dirección Física: %i - Valor: %s",contexto->pid,nuevaDirFisicaEscritura,auxiliarLog);
			free(auxiliarLog);
			//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		}else{
			//En este caso es que todo entra en una pagina y no hay que hacer nada extra
			mandarAescribirEnMemoria(dirFisicaEscritura,contenidoLeido,cantidadBits,contexto);
			
			//++++++++++++++++++Esto es solo para el log++++++++++++++++++++
			auxiliarLog = malloc(cantidadBits + 1);
			memcpy(auxiliarLog,contenidoLeido,cantidadBits);
			auxiliarLog[cantidadBits] = '\0';
			log_info(logOficialCpu,"PID: <%i> - Acción: ESCRIBIR - Dirección Física: %i - Valor: %s",contexto->pid,dirFisicaEscritura,auxiliarLog);
			free(auxiliarLog);
			//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

		}
		free(contenidoLeido); //Libero por valgrind
//*********************************************************************************************

}

// IO_STDOUT_WRITE (Interfaz, Registro Dirección, Registro Tamaño):
// Esta instrucción solicita al Kernel que mediante la interfaz seleccionada,
// se lea desde la posición de memoria indicada por la Dirección Lógica almacenada
// en el Registro Dirección, un tamaño indicado por el Registro Tamaño y se imprima por pantalla.


void operacion_io_stdout_write(t_pcb *contexto,int socket,t_instruccion_unitaria* instruccion){
	log_info(logOficialCpu,"Instrucción Ejecutada: PID: %i - Ejecutando: IO_STDOUT_WRITE - %s %s %s",contexto->pid,instruccion->parametros[0],instruccion->parametros[1],instruccion->parametros[2]);
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

		while(cantidadBytes > tamaPagina){
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
		while(cantidadBytes > tamaPagina){
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
// IO_STDIN_READ (Interfaz, Registro Dirección, Registro Tamaño): Esta instrucción
// solicita al Kernel que mediante la interfaz ingresada se lea desde el STDIN (Teclado)
// un valor cuyo tamaño está delimitado por el valor del Registro Tamaño y el mismo se guarde
// a partir de la Dirección Lógica almacenada en el Registro Dirección.

void operacion_io_stdin_read(t_pcb *contexto,int socket,t_instruccion_unitaria* instruccion){
	log_info(logOficialCpu,"Instrucción Ejecutada: PID: %i - Ejecutando: IO_STDIN_READ - %s %s %s",contexto->pid,instruccion->parametros[0],instruccion->parametros[1],instruccion->parametros[2]);
	t_buffer *buffer_rta;
	t_packet *packet_rta;
	buffer_rta = create_buffer();
	packet_rta = create_packet(STDIN_LEER,buffer_rta);
	add_to_packet(packet_rta,instruccion->parametros[0], instruccion->parametro1_lenght); //CARGO EL NOMBRE DE LA INTERFAZ
	
	
	int dirLogica = obtener_valor_del_registro(instruccion->parametros[1],contexto);
	int cantidadBytes = obtener_valor_del_registro(instruccion->parametros[2],contexto);

	add_to_packet(packet_rta,&cantidadBytes,sizeof(int));
	int nuevoMarco;
	int nuevaDirFisica;

	int dirFisica = traduccionLogica(contexto->pid,dirLogica);
	int desplazamiento = obtenerDesplazamiento(contexto->pid,dirLogica);
	int numeroPagina = (int) floor(dirLogica / tamaPagina);

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

		while(cantidadBytes > tamaPagina){
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
// WAIT (Recurso): Esta instrucción solicita al Kernel que se asigne una
// instancia del recurso indicado por parámetro.
void operacion_wait(t_pcb *contexto,int socket,t_instruccion_unitaria* instruccion){
	log_info(logOficialCpu,"Instrucción Ejecutada: PID: %i - Ejecutando: WAIT - %s",contexto->pid,instruccion->parametros[0]);
	t_buffer *bufferWait;
	t_packet *packetWait;
	bufferWait = create_buffer();
	packetWait = create_packet(WAIT_SOLICITUD,bufferWait);
	add_to_packet(packetWait,instruccion->parametros[0], instruccion->parametro1_lenght); //CARGO EL NOMBRE DEL RECURSO
	add_to_packet(packetWait,contexto,sizeof(t_pcb)); //CARGO EL PCB
	send_packet(packetWait,socket);		//ENVIO EL PAQUETE
	destroy_packet(packetWait);

	int operation_code = fetch_codop(socket);

	if((operation_code == WAIT_BLOQUEO) || (operation_code == RECURSO_EXIT)){
		int total_size;
		void *buffer = fetch_buffer(&total_size, socket); //RECIBO EL BUFFER 
		free(buffer);
		continuar_con_el_ciclo_instruccion = false;
		pid_ejecutando = -1;
	}else{
		int total_size;
		void *buffer = fetch_buffer(&total_size, socket); //RECIBO EL BUFFER 
		free(buffer);
	}
	
}
// SIGNAL (Recurso): Esta instrucción solicita al Kernel que se libere
// una instancia del recurso indicado por parámetro.
void operacion_signal(t_pcb *contexto,int socket,t_instruccion_unitaria* instruccion){
	log_info(logOficialCpu,"Instrucción Ejecutada: PID: %i - Ejecutando: SIGNAL - %s",contexto->pid,instruccion->parametros[0]);
	t_buffer *bufferSignal;
	t_packet *packetSignal;
	bufferSignal = create_buffer();
	packetSignal = create_packet(SIGNAL_SOLICITUD,bufferSignal);
	add_to_packet(packetSignal,instruccion->parametros[0], instruccion->parametro1_lenght); //CARGO EL NOMBRE DEL RECURSO
	add_to_packet(packetSignal,contexto,sizeof(t_pcb)); //CARGO EL PCB
	send_packet(packetSignal,socket);		//ENVIO EL PAQUETE
	destroy_packet(packetSignal);

	int operation_code = fetch_codop(socket);

	if(operation_code == RECURSO_EXIT){
		int total_size;
		void *buffer = fetch_buffer(&total_size, socket); //RECIBO EL BUFFER 
		free(buffer);
		continuar_con_el_ciclo_instruccion = false;
		pid_ejecutando = -1;
	}else{
		int total_size;
		void *buffer = fetch_buffer(&total_size, socket); //RECIBO EL BUFFER 
		free(buffer);
	}

	
}
// IO_FS_CREATE (Interfaz, Nombre Archivo): Esta instrucción
// solicita al Kernel que mediante la interfaz seleccionada, se cree un
// archivo en el FS montado en dicha interfaz.
void operacion_io_fs_create(t_pcb *contexto,int socket,t_instruccion_unitaria* instruccion){
	log_info(logOficialCpu,"Instrucción Ejecutada: PID: %i - Ejecutando: IO_FS_CREATE - %s %s",contexto->pid,instruccion->parametros[0],instruccion->parametros[1]);
	t_buffer *bufferCreate;
	t_packet *packetCreate;
	bufferCreate = create_buffer();
	packetCreate = create_packet(CREAR_ARCHIVO,bufferCreate);
	//CARGO EL NOMBRE DE LA INTERFAZ
	add_to_packet(packetCreate,instruccion->parametros[0], instruccion->parametro1_lenght);
	//CARGO EL NOMBRE DEL ARCHIVO A CREAR
	add_to_packet(packetCreate,instruccion->parametros[1], instruccion->parametro2_lenght);
	
	//Bloqueado porque se va a ir a la cola de la interfaz
	contexto->state = BLOCKED;
	
	add_to_packet(packetCreate,contexto,sizeof(t_pcb)); //CARGO EL PCB
	send_packet(packetCreate,socket);		//ENVIO EL PAQUETE
	destroy_packet(packetCreate);

}
// IO_FS_DELETE (Interfaz, Nombre Archivo): Esta instrucción solicita al
// Kernel que mediante la interfaz seleccionada, se elimine un archivo en el
// FS montado en dicha interfaz
void operacion_io_fs_delete(t_pcb *contexto,int socket,t_instruccion_unitaria* instruccion){
	log_info(logOficialCpu,"Instrucción Ejecutada: PID: %i - Ejecutando: IO_FS_DELETE - %s %s",contexto->pid,instruccion->parametros[0],instruccion->parametros[1]);
	t_buffer *bufferDelete;
	t_packet *packetDelete;
	bufferDelete = create_buffer();
	packetDelete = create_packet(BORRAR_ARCHIVO,bufferDelete);
	//CARGO EL NOMBRE DE LA INTERFAZ
	add_to_packet(packetDelete,instruccion->parametros[0], instruccion->parametro1_lenght);
	//CARGO EL NOMBRE DEL ARCHIVO A BORRAR
	add_to_packet(packetDelete,instruccion->parametros[1], instruccion->parametro2_lenght);
	
	//Bloqueado porque se va a ir a la cola de la interfaz
	contexto->state = BLOCKED;
	
	add_to_packet(packetDelete,contexto,sizeof(t_pcb)); //CARGO EL PCB
	send_packet(packetDelete,socket);		//ENVIO EL PAQUETE
	destroy_packet(packetDelete);
}
// IO_FS_TRUNCATE (Interfaz, Nombre Archivo, Registro Tamaño): Esta instrucción
// solicita al Kernel que mediante la interfaz seleccionada, se modifique el tamaño
// del archivo en el FS montado en dicha interfaz, actualizando al valor que se encuentra
// en el registro indicado por Registro Tamaño.
void operacion_io_fs_truncate(t_pcb *contexto,int socket,t_instruccion_unitaria* instruccion){
	log_info(logOficialCpu,"Instrucción Ejecutada: PID: %i - Ejecutando: IO_FS_TRUNCATE - %s %s %s",contexto->pid,instruccion->parametros[0],instruccion->parametros[1],instruccion->parametros[2]);
	//LEO EL NUEVO TAMAÑO DEL ARCHIVO
	int  nuevoTamaArchivo =   obtener_valor_del_registro(instruccion->parametros[2],contexto);
	//CREACION DE BUFFER Y PAQUETE PARA ENVIAR
	t_buffer *bufferTruncate;
	t_packet *packetTruncate;
	bufferTruncate = create_buffer();
	packetTruncate = create_packet(TRUNCAR_ARCHIVO,bufferTruncate);
	//CARGO EL NOMBRE DE LA INTERFAZ
	add_to_packet(packetTruncate,instruccion->parametros[0], instruccion->parametro1_lenght);
	//CARGO EL NOMBRE DEL ARCHIVO A BORRAR
	add_to_packet(packetTruncate,instruccion->parametros[1], instruccion->parametro2_lenght);
	//CARGO EL NUEVO TAMAÑO DEL ARCHIVO
	add_to_packet(packetTruncate,&nuevoTamaArchivo,sizeof(int));
	//Bloqueado porque se va a ir a la cola de la interfaz
	contexto->state = BLOCKED;
	add_to_packet(packetTruncate,contexto,sizeof(t_pcb)); //CARGO EL PCB
	send_packet(packetTruncate,socket);		//ENVIO EL PAQUETE
	destroy_packet(packetTruncate);
}
// IO_FS_WRITE (Interfaz, Nombre Archivo, Registro Dirección, Registro Tamaño,
// Registro Puntero Archivo): Esta instrucción solicita al Kernel que mediante la
// interfaz seleccionada, se lea desde Memoria la cantidad de bytes indicadas por el
// Registro Tamaño a partir de la dirección lógica que se encuentra en el Registro Dirección
// y se escriban en el archivo a partir del valor del Registro Puntero Archivo.
void operacion_io_fs_write(t_pcb *contexto,int socket,t_instruccion_unitaria* instruccion){
	log_info(logOficialCpu,"Instrucción Ejecutada: PID: %i - Ejecutando: IO_FS_WRITE - %s %s %s %s %s",contexto->pid,instruccion->parametros[0],instruccion->parametros[1],instruccion->parametros[2],instruccion->parametros[3],instruccion->parametros[4]);
	int registroPuntero = obtener_valor_del_registro(instruccion->parametros[4],contexto);
	
	t_buffer *buffer_rta;
	t_packet *packet_rta;
	buffer_rta = create_buffer();
	packet_rta = create_packet(FS_WRITE,buffer_rta);
	add_to_packet(packet_rta,instruccion->parametros[0], instruccion->parametro1_lenght); //CARGO EL NOMBRE DE LA INTERFAZ
	add_to_packet(packet_rta,instruccion->parametros[1], instruccion->parametro2_lenght); //CARGO EL NOMBRE DEL ARCHIVO
	add_to_packet(packet_rta,&registroPuntero,sizeof(int)); //CARGO EL REGISTRO PUNTERO
	
	int dirLogica = obtener_valor_del_registro(instruccion->parametros[2],contexto);
	int cantidadBytes = obtener_valor_del_registro(instruccion->parametros[3],contexto);

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

		while(cantidadBytes > tamaPagina){
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
// IO_FS_READ (Interfaz, Nombre Archivo, Registro Dirección, Registro Tamaño,
// Registro Puntero Archivo): Esta instrucción solicita al Kernel que mediante
// la interfaz seleccionada, se lea desde el archivo a partir del valor del Registro
// Puntero Archivo la cantidad de bytes indicada por Registro Tamaño y se escriban en
// la Memoria a partir de la dirección lógica indicada en el Registro Dirección.
void operacion_io_fs_read(t_pcb *contexto,int socket,t_instruccion_unitaria* instruccion){
	log_info(logOficialCpu,"Instrucción Ejecutada: PID: %i - Ejecutando: IO_FS_READ - %s %s %s %s %s",contexto->pid,instruccion->parametros[0],instruccion->parametros[1],instruccion->parametros[2],instruccion->parametros[3],instruccion->parametros[4]);
	int registroPuntero = obtener_valor_del_registro(instruccion->parametros[4],contexto);
	
	t_buffer *buffer_rta;
	t_packet *packet_rta;
	buffer_rta = create_buffer();
	packet_rta = create_packet(FS_READ,buffer_rta);
	add_to_packet(packet_rta,instruccion->parametros[0], instruccion->parametro1_lenght); //CARGO EL NOMBRE DE LA INTERFAZ
	add_to_packet(packet_rta,instruccion->parametros[1], instruccion->parametro2_lenght); //CARGO EL NOMBRE DEL ARCHIVO
	add_to_packet(packet_rta,&registroPuntero,sizeof(int)); //CARGO EL REGISTRO PUNTERO
	
	int dirLogica = obtener_valor_del_registro(instruccion->parametros[2],contexto);
	int cantidadBytes = obtener_valor_del_registro(instruccion->parametros[3],contexto);

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

		while(cantidadBytes > tamaPagina){
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