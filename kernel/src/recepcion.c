#include "recepcion.h"

 sem_t procesosEsperandoIO;

char *interfazBUSCADA;//HAGO ESTO PARA PODER USAR EL LIST_FIND PORQUE SOLO LE PODES PASAR UN PARAMETRO AL BOOL;

t_pcb *fetch_pcb_con_sleep(int server_socket,int *tiempoDormir,char **nomrebInterfaz){
    
    int total_size;
    int offset = 0;
    t_pcb *PCBrec = pcbEJECUTANDO;
    pcbEJECUTANDO = NULL;

    //t_pcb *PCBrec = malloc(sizeof(t_pcb)); //aca esto es lo que estaba antes y funciona

    //aca habria que hacer que pcbRec sea igual a pcbEjectuando asi actualiza el pcb enviado, osea
    //el actual y despues le hace push a la cola de la IO y desppues de haber 
    //actualizado el valor de PCBrec por pcbejectuando, agarro a pcb ejecutando y le pongo NULL
    void *buffer;
    int length_nombre_inter;
    
    //char *nomrebInterfaz LOS RECIBO COMO PARAMETRO PARA MODULARIZAR EN MAS FUNCIONES
    //int tiempoDormir
    
    //tengo que crear un struct para la interfaz
    
    
    buffer = fetch_buffer(&total_size, server_socket); //RECIBO EL BUFFER 
   
    memcpy(&length_nombre_inter,buffer + offset, sizeof(int));  //RECIBO EL LENGTH DEL NOMBRE  DE LA INTERFAZ
    offset += sizeof(int); 

    *nomrebInterfaz = malloc(length_nombre_inter);
    memcpy(*nomrebInterfaz,buffer + offset,length_nombre_inter); //RECIBO EL NOMBRE DE LA INTERFAZ
    offset += length_nombre_inter; 

    offset += sizeof(int);  //me salteo el tamaño del tiempo
    memcpy(tiempoDormir,buffer + offset,sizeof(int)); //RECIBO EL TIEMPO A DORMIR que como tengo un puntero, no pongo &
    offset += sizeof(int); 
  
    

    offset += sizeof(int); //Salteo El tamaño del PCB

    memcpy(&(PCBrec->pid),buffer + offset, sizeof(int)); //RECIBO EL PID
    offset += sizeof(int);

    memcpy(&(PCBrec->program_counter), buffer + offset, sizeof(int)); // RECIBO EL PROGRAM COUNTER
    offset += sizeof(int);
    
    memcpy(&(PCBrec->quantum), buffer + offset, sizeof(int)); //RECIBO EL QUANTUM
    offset += sizeof(int);

    memcpy(&(PCBrec->state), buffer + offset, sizeof(t_process_state)); //RECIBO EL PROCESS STATE
    offset += sizeof(t_process_state);

    memcpy(&(PCBrec->registers.PC), buffer + offset, sizeof(uint32_t)); //RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(PCBrec->registers.AX), buffer + offset, sizeof(uint8_t)); //RECIBO CPUREG
    offset += sizeof(uint8_t);
    memcpy(&(PCBrec->registers.BX), buffer + offset, sizeof(uint8_t)); //RECIBO CPUREG
    offset += sizeof(uint8_t);
    memcpy(&(PCBrec->registers.CX), buffer + offset, sizeof(uint8_t)); //RECIBO CPUREG
    offset += sizeof(uint8_t);
    memcpy(&(PCBrec->registers.DX), buffer + offset, sizeof(uint8_t)); //RECIBO CPUREG
    offset += sizeof(uint8_t);
    memcpy(&(PCBrec->registers.EAX), buffer + offset, sizeof(uint32_t)); //RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(PCBrec->registers.EBX), buffer + offset, sizeof(uint32_t)); //RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(PCBrec->registers.ECX), buffer + offset, sizeof(uint32_t)); //RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(PCBrec->registers.EDX), buffer + offset, sizeof(uint32_t)); //RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(PCBrec->registers.SI), buffer+ offset, sizeof(uint32_t)); //RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(PCBrec->registers.DI), buffer + offset, sizeof(uint32_t)); //RECIBO CPUREG
    offset += sizeof(uint32_t);

   
    log_info(logger, "SE RECIBIO UN SLEEP A UNA INTERFAZ Y DEVOLVEMOS PCB");
    log_info(logger, "PID RECIBIDO : %i",PCBrec->pid);
    log_info(logger, "PC RECIBIDO : %i",PCBrec->program_counter);
    log_info(logger, "REGISTRO AX : %i",PCBrec->registers.AX);
    log_info(logger, "REGISTRO BX : %i",PCBrec->registers.BX);
    // A PARTIR DE ACA EL PCB RECIBIDO VA A ENTRAR EN LA COLA DE ESPERA DE LA INTERFAZ 
    //Y SI LE INTERFAZ ESTA LIBRE SE CREA EL HILO PARA ENVIARLE EL SLEEP

    return PCBrec;
}

t_interfaz_registrada *buscar_interfaz(char *nombreInterfaz){  
    
    interfazBUSCADA = nombreInterfaz;
    log_info(logger, "NOMBRE INTERFAZ BUSCADA: %s",interfazBUSCADA);
    
    t_interfaz_registrada *interfaz = list_find(listaInterfaces,(void*)esLaInterfazBuscada);
    log_info(logger, "NOMBRE INTERFAZ OBTENIDA: %s",interfaz->nombre);
    
    return interfaz;
}
    
void cargarEnListaIO(t_pcb *receptorPCB,t_interfaz_registrada *interfaz,int tiempoDormir){
    t_pcbYtiempo *agregarACola = malloc(sizeof(t_pcbYtiempo));
    agregarACola->PCB = receptorPCB;
    agregarACola->tiempoDormir = tiempoDormir;

    pthread_mutex_lock(&(interfaz->mutexColaIO));
    queue_push(interfaz->listaProcesosEsperando, agregarACola);
    pthread_mutex_unlock(&(interfaz->mutexColaIO));

    sem_post(&(interfaz->semaforoContadorIO)); //aca solo le hago el post porque sume un pcb a la lista

}

bool esLaInterfazBuscada(t_interfaz_registrada *recibida){
    if(strcmp(recibida->nombre,interfazBUSCADA)==0){
        return true;
    }else{ 
        return false;
    }
}

void llamadas_io(t_interfaz_registrada *interfaz){

    t_pcbYtiempo *pcbEnviado = NULL;
    sem_t soloUnoEnvia;
    sem_init(&(soloUnoEnvia), 0, 1); //ESTE SEMAFORO ES PARA QUE SOLO UNO HAGA EL ENVIO
    while(1){
        
        t_buffer *bufferTiempoDormir;
        t_packet *packetTiempoDormir;
        
        sem_wait(&(interfaz->semaforoContadorIO));//a este solo le hago signal cuando entra un Nuevo PCB, no al final de esto
        sem_wait(&(soloUnoEnvia));
        //aca el semaforo soloUnoEnvia lo que hace es bloquear que solo uno envie a IO, pero 
        //el mutex de abajo es para que a la cola solo entre uno, ese es mas chico porque bloquea el acceso a la cola
        //y si solamente uso soloUnoEnvia entonces me quedaria esperando el recv de la IO y no se podrian merter
        //mas cosas a la cola y congelaria todo el kernel. Por eso uso dos, este que cube todo el envio y
        //el mutex que una vez que saco lo que queria ya hace el unlock
        
        pthread_mutex_lock(&(interfaz->mutexColaIO));
        pcbEnviado = queue_pop(interfaz->listaProcesosEsperando); //ENCAPSULO ENTRE DOS MUTEX SACAR DE LA COLA
        pthread_mutex_unlock(&(interfaz->mutexColaIO));
        
        bufferTiempoDormir = create_buffer();
        packetTiempoDormir = create_packet(TIEMPO_DORMIR, bufferTiempoDormir);
        add_to_packet(packetTiempoDormir,&(pcbEnviado->tiempoDormir), sizeof(int));
        send_packet(packetTiempoDormir,interfaz->socket_de_conexion);     //armo el paquete para enviar a la IO 
        log_info(logger, "SE ENVIO SLEEP DE %i A LA INTERFAZ: %s\n",pcbEnviado->tiempoDormir,interfaz->nombre); 
        int operation_code = fetch_codop(interfaz->socket_de_conexion); //aca se queda bloqueante esperando la respuesta
    
        int total_size;
        void *buffer2 = fetch_buffer(&total_size,interfaz->socket_de_conexion); // recibo porque puse un numero en el buffer
        free(buffer2);  
                            //para no enviarlo vacio
        
        if(operation_code == CONFIRMACION_SLEEP_COMPLETO){
        log_info(logger, "%s CONFIRMA CODIGO FIN SLEEP\n",interfaz->nombre); 

        }                                    
        log_info(logger, "LA INTERFAZ: %s TERMINO SLEEP DE : %i\n",interfaz->nombre,pcbEnviado->tiempoDormir); 

        
        addEstadoReady(pcbEnviado->PCB);//meto en ready el pcb 

        
        sem_post(&(soloUnoEnvia));
       

    }
}

void crear_hilo_interfaz(t_interfaz_registrada *interfaz){
    pthread_t hilo_manejo_io;
    pthread_create(&hilo_manejo_io, NULL, (void* ) llamadas_io, interfaz);
    pthread_detach(hilo_manejo_io);
}