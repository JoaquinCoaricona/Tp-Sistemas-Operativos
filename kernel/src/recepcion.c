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

    /*en realidad es lo que hice. PCB ejecutando es la variable global entonces se la asigno a PCBrec
    porque todos los memcpy que estan abajo ya estaban con eso. Despues agarro pcbEjecutando
    y lo apunto a NULL porque ya no se esta ejecutando a nadie en CPU, entonces agarro a PCB rec
    que es el pcb del proceso que acaba de llegar y le hago todos los memcpy para ir actualizandolo
    despues hago todos los logs y devuelvo PCBrec y pcbEjecutando quedo en null
    */
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

t_pcb *fetch_pcb_con_STDOUT(int server_socket, char **nombreInter, void *contenido, int *tamanio, int *bytesMalloc) {
    int total_size;
    int offset = 0;
    t_pcb *PCBrec = pcbEJECUTANDO;
    pcbEJECUTANDO = NULL;
    void *buffer;
    int length_nombre_inter;
    int contenidoACopiar;

    buffer = fetch_buffer(&total_size, server_socket); //RECIBO EL BUFFER 

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++     
                //RECIBO EL NOMBRE DE LA INTERFAZ
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 

    memcpy(&length_nombre_inter,buffer + offset, sizeof(int));  //RECIBO EL LENGTH DEL NOMBRE  DE LA INTERFAZ
    offset += sizeof(int); 

    *nombreInter = malloc(length_nombre_inter);

    memcpy(*nombreInter,buffer + offset,length_nombre_inter); //RECIBO EL NOMBRE DE LA INTERFAZ
    offset += length_nombre_inter;

    //Ahora recibo el tamaño del malloc que tengo que hacer para recibir el contenido:
    memcpy(bytesMalloc,buffer + offset, sizeof(int));
    offset += sizeof(int);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++=++++++++++CALCULOS PARA NO DESERIALIZAR Y VOLVER A SERIALIZAR LO MISMO++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ +++++++++++++++++
    //Aca calculo la parte que voy a leer de lo que queda del buffer
    //Contenidoacopiar guarda la cantidad de bytes que tienen dentro a 
    //las direcciones fisicas y al int de la cantidad de direccciones fisicas que hay

    contenidoACopiar = total_size; //Igual al tamaño total del buffer 
    contenidoACopiar = contenidoACopiar -length_nombre_inter;//Resto el lengthNoombre que ya lei
    contenidoACopiar = contenidoACopiar -sizeof(int);//Resto el tamaño del int Length nombre que lei
    contenidoACopiar = contenidoACopiar - sizeof(int);//Resto el int que indica el tamaño del PCB
    contenidoACopiar = contenidoACopiar - sizeof(t_pcb);//Resto los bytes que ocupa el pcb

    //Aca reservo el espacio justo en contenido
    contenido = malloc(contenidoACopiar);

    //copio todas las direcciones fisicas y el entero que dice cuantas direcciones fisicas tengo
    memcpy(contenido,buffer + offset,contenidoACopiar);

    //Salteo el contenido de las direcciones fisicas
    offset += contenidoACopiar;

    //Asigno el tamaño del void* a la variable tamanio
    tamanio = contenidoACopiar;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++RECIBO PCB++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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

void cargarEnListaSTDOUT(t_pcb *receptorPCB,t_interfaz_registrada *interfaz,void *contenido,int tamanio, int bytesMalloc){

    t_colaStdOUT *agregarACola = malloc(sizeof(t_colaStdOUT));

    agregarACola->PCB = receptorPCB;
    agregarACola->contenido = contenido;
    agregarACola->cantidadBytes = tamanio;
    agregarACola->bytesMalloc = bytesMalloc;

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
        //ACLARACION: es fetch_buffer de protocol.c , no es fetch pcb, solo recibe el numero que se envio para confirmar 

        void *buffer2 = fetch_buffer(&total_size,interfaz->socket_de_conexion); // recibo porque puse un numero en el buffer

        free(buffer2);  //para no enviarlo vacio

        if(operation_code == CONFIRMACION_SLEEP_COMPLETO){
            log_info(logger, "%s CONFIRMA CODIGO FIN SLEEP\n",interfaz->nombre);
        }                                    

        log_info(logger, "LA INTERFAZ: %s TERMINO SLEEP DE : %i\n",interfaz->nombre,pcbEnviado->tiempoDormir); 
        //Este if lo que hace es ver si estamos en RR o VRR, en caso que estemos en RR nunca vamos a usar
        // el valor del quantum y como siempre que los creamos le asignamos el quantumGlobal ese no se
        // va a modificar nunca y todos los pcb van a tener ese valor y asi los idenfitico para ponerlo en la 
        // cola de ready. Pero en VRR el quantum cambia y siempre que vuelven de io tenemos que meterlo a la 
        // cola prioritaria, porque salieron antes de fin de quantum y tienen el restante en su PCB entonces 
        // asi los identifico y los pongo en la cola de prioridad
        //Tengo una duda sobre si esta bien hacer el semPost ready en el else, pero creo que esta bien porque el
        //unico lugar donde se controla la cola prioritaria es cuando miro ready, ahi si hay 
        //
        if(pcbEnviado->PCB->quantum == quantumGlobal){
            //No necesariamente es round robin, podria pasar que estaba en quantum negativo
            //entonces antes de enviarlo a io le cambiaron el quantum al original para que 
            //no se vaya a la cola prioritaria, entonces podria estar viniendo aca dentro de VRR
            //la explicacion de porque podria llegar a quantum Negativo esta en el case SLEEP_IO
            //en main.c Ahi se hace el control y esta la explicacion
            log_info(logger,"Como estoy en RoundRobin lo agrego a ready directamente");
            addEstadoReady(pcbEnviado->PCB);//meto en ready el pcb 
            sem_post(&sem_ready); 
        }else{
            log_info(logger,"Como estoy en Virtual RoundRobin lo agrego a la cola prioridad");
            addColaPrioridad(pcbEnviado->PCB);
            sem_post(&sem_ready); 
        }

        sem_post(&(soloUnoEnvia));
        destroy_packet(packetTiempoDormir);
    }
}

void llamadasIOstdout(t_interfaz_registrada *interfaz){

    t_colaStdOUT *pcbEnviado = NULL;

    sem_t soloUnoEnvia;
    sem_init(&(soloUnoEnvia), 0, 1); //ESTE SEMAFORO ES PARA QUE SOLO UNO HAGA EL ENVIO

    while(1){

        t_buffer *bufferEscribir;
        t_packet *packetEscribir;

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

        bufferEscribir = create_buffer();
        packetEscribir = create_packet(STDOUT_ESCRIBIR, bufferEscribir);

        add_to_packet(packetEscribir,&(pcbEnviado->PCB->pid), sizeof(int));
        add_to_packet(packetEscribir,&(pcbEnviado->bytesMalloc), sizeof(int));
        add_to_packet(packetEscribir,&(pcbEnviado->contenido),pcbEnviado->cantidadBytes);

        send_packet(packetEscribir,interfaz->socket_de_conexion);     //armo el paquete para enviar a la IO 

        log_info(logger, "SE ENVIARON LAS DIRECCIONES FISICAS A LA INTERFAZ: %s",interfaz->nombre); 

        int operation_code = fetch_codop(interfaz->socket_de_conexion); //aca se queda bloqueante esperando la respuesta

        int total_size;
        //ACLARACION: es fetch_buffer de protocol.c , no es fetch pcb, solo recibe el numero que se envio para confirmar 
        void *buffer2 = fetch_buffer(&total_size,interfaz->socket_de_conexion); // recibo porque puse un numero en el buffer
        free(buffer2);  //para no enviarlo vacio

        if(operation_code == CONFIRMACION_STDOUT){
            log_info(logger, "%s CONFIRMA FIN STDOUT",interfaz->nombre); 
        }                                    

        //Este if lo que hace es ver si estamos en RR o VRR, en caso que estemos en RR nunca vamos a usar
        // el valor del quantum y como siempre que los creamos le asignamos el quantumGlobal ese no se
        // va a modificar nunca y todos los pcb van a tener ese valor y asi los idenfitico para ponerlo en la 
        // cola de ready. Pero en VRR el quantum cambia y siempre que vuelven de io tenemos que meterlo a la 
        // cola prioritaria, porque salieron antes de fin de quantum y tienen el restante en su PCB entonces 
        // asi los identifico y los pongo en la cola de prioridad

        //Tengo una duda sobre si esta bien hacer el semPost ready en el else, pero creo que esta bien porque el
        //unico lugar donde se controla la cola prioritaria es cuando miro ready, ahi si hay 
        //
        if(pcbEnviado->PCB->quantum == quantumGlobal){
            //No necesariamente es round robin, podria pasar que estaba en quantum negativo
            //entonces antes de enviarlo a io le cambiaron el quantum al original para que 
            //no se vaya a la cola prioritaria, entonces podria estar viniendo aca dentro de VRR
            //la explicacion de porque podria llegar a quantum Negativo esta en el case SLEEP_IO
            //en main.c Ahi se hace el control y esta la explicacion
            log_info(logger,"Como estoy en RoundRobin lo agrego a ready directamente");
            addEstadoReady(pcbEnviado->PCB);//meto en ready el pcb 
            sem_post(&sem_ready); 
        }else{
            log_info(logger,"Como estoy en Virtual RoundRobin lo agrego a la cola prioridad");
            addColaPrioridad(pcbEnviado->PCB);
            sem_post(&sem_ready); 
        }
        free(pcbEnviado->contenido);
        sem_post(&(soloUnoEnvia));
        destroy_packet(packetEscribir);
    }
}

void crear_hilo_interfaz(t_interfaz_registrada *interfaz){

    pthread_t hilo_manejo_io;

    if(string_equals_ignore_case(interfaz->tipo,"GENERICA")){
        pthread_create(&hilo_manejo_io, NULL, (void* ) llamadas_io, interfaz);
        pthread_detach(hilo_manejo_io);
    }else if(string_equals_ignore_case(interfaz->tipo,"GENERICA")){
        pthread_create(&hilo_manejo_io, NULL, (void* ) llamadasIOstdout, interfaz);
        pthread_detach(hilo_manejo_io);
    }else{
        log_info(logger,"ERROR");
    }
}