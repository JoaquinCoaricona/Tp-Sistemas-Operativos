#include "instrucciones.h"

t_queue* queue_instrucciones;

void initialize_queue_and_semaphore_memoria() {
    queue_instrucciones = queue_create();
}

void recibir_proceso(int client_socket,t_instruccion *instRec) {
   
    t_instrucciones *instruccionRecibida = malloc(sizeof(t_instrucciones)); 
    fetch_instruccion(client_socket,instruccionRecibida);
   
    

}

void *fetch_instruccion(int client_socket,t_instruccion *instRec)
{
    int total_size;
    int offset = 0;
    
    void *buffer2;
   
    int tama; //Solo para recibir el size que esta al principio del buffer
    
    buffer2 = fetch_buffer(&total_size, client_socket);
    
    memcpy(&tama,buffer2 + offset, sizeof(int)); //RECIBO EL TAMAÑO
    offset += sizeof(int);
    

    memcpy(&(PCBrec->pid),buffer2 + offset, sizeof(int)); //RECIBO EL PID
    offset += sizeof(int);

    memcpy(&(PCBrec->program_counter), buffer2 + offset, sizeof(int)); // RECIBO EL PATH
    offset += sizeof(int);


    memcpy(&(PCBrec->program_counter), buffer2 + offset, sizeof(int)); // RECIBO EL PATH
    offset += sizeof(int);

    //en el tp resuelto son las funciones leer_pseudo y recibir_path_y_pid(cliente_fd, &archivo_path, &pid);
    //el path es una mezcla de lo que te mandan y algo que tenes que tener en configu que va a ser donde esta 
    //una carpeta donde tenes guardads las pruebas en general
    free(buffer2);
}


