#include "instrucciones.h"



int pidBUSCADO;
t_list *listaINSTRUCCIONES;

void initialize_queue_and_semaphore_memoria() {
    //queue_instrucciones = queue_create();
	listaINSTRUCCIONES = list_create();
}

void recibir_proceso(int client_socket,t_instrucciones *instRec) {
   
    t_instrucciones *instruccionRecibida = malloc(sizeof(t_instrucciones)); 
    fetch_instruccion(client_socket,instruccionRecibida);
   
    

}

void fetch_instruccion(int client_socket,t_instrucciones *instRec) //aca era void *fetch...
{
    int total_size;
    int offset = 0;
    
    void *buffer2;
   
    int tama; //Solo para recibir el size que esta al principio del buffer
    
    buffer2 = fetch_buffer(&total_size, client_socket);
    
    memcpy(&tama,buffer2 + offset, sizeof(int)); //RECIBO EL TAMAÑO
    offset += sizeof(int);
    

    // memcpy(&(instRec->pid),buffer2 + offset, sizeof(int)); //RECIBO EL PID
    // offset += sizeof(int);

    // memcpy(&(instRec->program_counter), buffer2 + offset, sizeof(int)); // RECIBO EL PATH
    // offset += sizeof(int);


    // memcpy(&(instRec->program_counter), buffer2 + offset, sizeof(int)); // RECIBO EL PATH
    // offset += sizeof(int);

    //en el tp resuelto son las funciones leer_pseudo y recibir_path_y_pid(cliente_fd, &archivo_path, &pid);
    //el path es una mezcla de lo que te mandan y algo que tenes que tener en configu que va a ser donde esta 
    //una carpeta donde tenes guardads las pruebas en general
    free(buffer2);
}

void leer_pseudo(int client_socket){

	t_instrucciones *instruccionREC = malloc(sizeof(t_instrucciones));
    fetch_pathYpid(client_socket,instruccionREC);

	//*********CREACION DE LA TABLA DE PAGINAS**********************
	t_list *tablaDePaginas = list_create(); //Lista generica para cargar en el diccionario
	//Ahora agrego al diccionario usando como llave el pid (pasado a string) y le asocio
	//la lista generica con la tabla de paingas
	dictionary_put(tabla_paginas_por_PID,string_itoa(instruccionREC->pid),tablaDePaginas);
	log_info(logger,"Creacion Tabla De Paginas PID: %i Tam: 0",instruccionREC->pid);
	//Aca directamente puse el 0 porque como es nueva su tamaño va a ser 0
	//**************************************************************

	char* path = string_new();
	string_append(&path, PATH_CONFIG);
	string_append(&path, instruccionREC->path);
	

    FILE* archivo = fopen(path,"r");

	if(archivo == NULL){
		//log_error(logger, "Error en la apertura del archivo: Error: %d (%s)", errno, strerror(errno));
		//free(path);
		printf("Error al abrir el archivo");
        return;
	}

    char* cadena;
	instruccionREC->lista_de_instrucciones = list_create();

    while(feof(archivo) == 0)
	{

		cadena = malloc(300);
		char *resultado_cadena = fgets(cadena, 300, archivo);

        //si el char esta vacio, hace break.
		if(resultado_cadena == NULL)
		{
			//log_error(logger, "No encontre el archivo o esta vacio");
			break;
		}

        strtok(cadena,"\n"); //aca cadena tiene un bara n al final y el strtok lo reemplaza por/0
        
        //printf("\n");
        //printf("%s\n",cadena);
		

        t_instruccion_unitaria *ptr_inst = malloc(sizeof(t_instruccion_unitaria));
		
		


		char *token = strdup(strtok(cadena," "));
        //char *token = strtok(cadena," ");
        ptr_inst->opcode = token;
        ptr_inst->opcode_lenght = strlen(ptr_inst->opcode) + 1;
        
        ptr_inst->parametros[0] = NULL;
        ptr_inst->parametros[1] = NULL;
        ptr_inst->parametros[2] = NULL;
        ptr_inst->parametros[3] = NULL;
        ptr_inst->parametros[4] = NULL;

		if(strcmp(token,"EXIT")!=0){

		//token = strtok(NULL, " ");
		token = strdup(strtok(NULL, " "));
		int n = 0;
		

		while(token != NULL)
		{
			ptr_inst->parametros[n] = token; 
			
			char *auxiliar;
			auxiliar = strtok(NULL, " "); //AUXILIAR SE GUARDA EL VALOR DEL PARAMETRO
										  //EN CASO DE QUE NO HAYA MAS Y SEA NULL 
			if(auxiliar != NULL){		//HACE LA COMPROBACION Y si resulta que es null entonces
			token = strdup(auxiliar);	//le pasamos el valor NULL a token asi sale del bucle
			}else{						//y si no era null entonces le paso a token un strdup
			token = auxiliar;		//para que se guarde el parametro en un espacio nuevo
			}						//todo esto porque token al hacer el strtok lo hace sobre el espacio
									//de cadena, entnoces al final cuando libero cadena si lo iguale
			n++;	//token directamente con el strtok, cuando libere cadena pierdo lo que guarde en ptr_inst
		}		//porque directamente le habia pasado la direccion de cadena. Por eso uso strdup

		}
		
		if(ptr_inst->parametros[0] != NULL){
			ptr_inst->parametro1_lenght = strlen(ptr_inst->parametros[0])+1;
		} else {
			ptr_inst->parametro1_lenght = 0;
		}
		if(ptr_inst->parametros[1] != NULL){
			ptr_inst->parametro2_lenght = strlen(ptr_inst->parametros[1])+1;
		} else {
			ptr_inst->parametro2_lenght = 0;
		}
		if(ptr_inst->parametros[2] != NULL){
			ptr_inst->parametro3_lenght = strlen(ptr_inst->parametros[2])+1;
		} else {
			ptr_inst->parametro3_lenght = 0;
		}
		if(ptr_inst->parametros[3] != NULL){
			ptr_inst->parametro4_lenght = strlen(ptr_inst->parametros[3])+1;
		} else {
			ptr_inst->parametro4_lenght = 0;
		}
		if(ptr_inst->parametros[4] != NULL){
			ptr_inst->parametro5_lenght = strlen(ptr_inst->parametros[4])+1;
		} else {
			ptr_inst->parametro5_lenght = 0;
		}

		list_add((instruccionREC->lista_de_instrucciones),ptr_inst);
		t_instruccion_unitaria *seguro  = ptr_inst;
		free(cadena);
	}

	list_add(listaINSTRUCCIONES,instruccionREC);
	
	//aca se van guardando todos lo procesos
	//es una cola de la strcut que tiene el pid, el path y la lista con todas las intrscuinoes
        
	
    //Para imprimir cuantos elementos tiene la lista
    //int total = list_size(instruccionREC->lista_de_instrucciones);
    //printf("holaaaaaaaaaaaaa \n");
	//Despues de haber leido de consola, si queremos imprimir algo, poner un barra n para que se limpie el buffer de salida


	//dictionary_put(lista_instrucciones_porPID, string_itoa(pid),lista_instrucciones);

	//enviar_mensaje("OK", cliente_fd, INICIAR_PROCESO);

    //ENVIO CONFIRMACION LECTURA A KERNEL
	t_buffer *bufferRespuesta;
    t_packet *packetRespuesta;
	//devuelvo el pid Del archivo que lei su path
    
    bufferRespuesta = create_buffer();
    packetRespuesta = create_packet(MEMORIA_TERMINO_LECTURA, bufferRespuesta);
    add_to_packet(packetRespuesta,&(instruccionREC->pid), sizeof(int));
    send_packet(packetRespuesta,client_socket);   
	destroy_packet(packetRespuesta);

	free(path);
	fclose(archivo);
}


bool buscarPorPid(t_instrucciones* Instruccion) {
	return Instruccion->pid == pidBUSCADO;
}



void devolverInstruccion(int client_socket){

	//conseguir los valores del PID y el PC
	
	int total_size;
    int offset = 0;
    
    void *buffer;
	
   


	int pid;
	int pc;
    
    buffer = fetch_buffer(&total_size, client_socket);
    
   
    
    offset += sizeof(int); //para saltearme el tanaño de la variable int que me mando antes del pid
	memcpy(&pid,buffer + offset, sizeof(int)); //RECIBO EL pid
    offset += sizeof(int);
    offset += sizeof(int);//para saltearme el length de int
	memcpy(&pc,buffer + offset, sizeof(int)); //RECIBO EL pc
	
	free(buffer);
	//este free de arriba es para borrar lo recibido, despues abajo uso de devuelta la vairbale
	// FIN RECEPCION PID Y PC	

	//printf(" \n PID RECIBIDO = %i \n",pid);
	//printf("PC RECIBIDO = %i \n",pc);

	pidBUSCADO = pid;
	

	
	t_instruccion_unitaria* instruccionPC;
	//t_instrucciones *getter = list_get(listaINSTRUCCIONES, 0);

    t_instrucciones *procesoBuscado = list_find(listaINSTRUCCIONES,(void*)buscarPorPid);
	
    //t_instrucciones *procesoBuscado = list_get(listaINSTRUCCIONES, 0);
	
	instruccionPC = list_get(procesoBuscado->lista_de_instrucciones,pc-1); // list get empieza en 0, por eso el -1
	
	//printf(" \n %s \n",instruccionPC->opcode); 


	//Ahora vamos a enviar la instruccion a CPU

	//aca esto antes usaba el mismo buffer de arriba osea el void*, no se como funcionaba
	//porque directamente decia buffer =create_buffer() lo separe porque valgrind marco como error
	//que no liberabamos el de arriba, arriba al hacer fetch buffer recibimos el puntero 
	//y lo deserializamos pero aca al cambiarle el valor al puntero perdermos lo de arriba
	//y como se ejecutaba muchas veces lo de devoler instruccion perdiamos mucha memoria
	t_buffer *buffer_res = create_buffer();
	
    t_packet *paquete_instruccion = create_packet(MEMORIA_ENVIA_INSTRUCCION, buffer_res);

    add_to_packet(paquete_instruccion,instruccionPC->opcode,instruccionPC->opcode_lenght);
	add_to_packet(paquete_instruccion,instruccionPC->parametros[0],instruccionPC->parametro1_lenght);
	add_to_packet(paquete_instruccion,instruccionPC->parametros[1],instruccionPC->parametro2_lenght);
	add_to_packet(paquete_instruccion,instruccionPC->parametros[2],instruccionPC->parametro3_lenght);
	add_to_packet(paquete_instruccion,instruccionPC->parametros[3],instruccionPC->parametro4_lenght);
	add_to_packet(paquete_instruccion,instruccionPC->parametros[4],instruccionPC->parametro5_lenght);


	//Demora En devolver Instruccion:
	usleep(retardoRespuesta * 1000); //de prueba 1 segundo
	send_packet(paquete_instruccion,client_socket);
	destroy_packet(paquete_instruccion);
	

}



