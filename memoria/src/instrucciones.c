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
        
        printf("\n");
        printf("%s\n",cadena);
		

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
			token = strtok(NULL, " ");
			n++;
		}

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
	
	// FIN RECEPCION PID Y PC	

	//printf(" \n PID RECIBIDO = %i \n",pid);
	//printf("PC RECIBIDO = %i \n",pc);

	pidBUSCADO = pid;
	

	
	t_instruccion_unitaria* instruccionPC;


    t_instrucciones *procesoBuscado = list_find(listaINSTRUCCIONES,buscarPorPid);
	instruccionPC = list_get(procesoBuscado->lista_de_instrucciones,pc-1); // list get empieza en 0, por eso el -1
	
	//printf(" \n %s \n",instruccionPC->opcode); 


	//Ahora vamos a enviar la instruccion a CPU

	buffer = create_buffer();
	
    t_packet *paquete_instruccion = create_packet(MEMORIA_ENVIA_INSTRUCCION, buffer);

    add_to_packet(paquete_instruccion,instruccionPC->opcode,instruccionPC->opcode_lenght);
	add_to_packet(paquete_instruccion,instruccionPC->parametros[0],instruccionPC->parametro1_lenght);
	add_to_packet(paquete_instruccion,instruccionPC->parametros[1],instruccionPC->parametro2_lenght);
	add_to_packet(paquete_instruccion,instruccionPC->parametros[2],instruccionPC->parametro3_lenght);
	add_to_packet(paquete_instruccion,instruccionPC->parametros[3],instruccionPC->parametro4_lenght);
	add_to_packet(paquete_instruccion,instruccionPC->parametros[4],instruccionPC->parametro5_lenght);

	send_packet(paquete_instruccion,client_socket);
	
	free(buffer);

}



