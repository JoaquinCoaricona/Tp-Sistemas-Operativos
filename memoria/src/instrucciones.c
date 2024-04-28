#include "instrucciones.h"

t_queue* queue_instrucciones;

void initialize_queue_and_semaphore_memoria() {
    queue_instrucciones = queue_create();
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

void leer_pseudo(){
    FILE* archivo = fopen("/home/utnso/pruebas/prueba","r");
    
    if(archivo == NULL){
		//log_error(logger, "Error en la apertura del archivo: Error: %d (%s)", errno, strerror(errno));
		//free(path);
		printf("Error al abrir el archivo");
        return;
	}

    char* cadena;
	t_list* lista_instrucciones = list_create();

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

        char *token = strtok(cadena," ");
        ptr_inst->opcode = token;
        ptr_inst->opcode_lenght = strlen(ptr_inst->opcode) + 1;
        
        ptr_inst->parametros[2] = NULL;
        ptr_inst->parametros[0] = NULL;
        ptr_inst->parametros[1] = NULL;

		token = strtok(NULL, " ");
		int n = 0;
		while(token != NULL)
		{
			ptr_inst->parametros[n] = token;
			token = strtok(NULL, " ");
			n++;
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

		list_add(lista_instrucciones,ptr_inst);
        free(cadena);
	}
   
    //Para imprimir cuantos elementos tiene la lista
    // int total = list_size(lista_instrucciones);
    // printf("%i",total);

	//dictionary_put(lista_instrucciones_porPID, string_itoa(pid),lista_instrucciones);

	//enviar_mensaje("OK", cliente_fd, INICIAR_PROCESO);

	//free(path);
	fclose(archivo);
}



