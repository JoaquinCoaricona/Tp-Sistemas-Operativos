#include "recepcion.h"


char *interfazBUSCADA;//HAGO ESTO PARA PODER USAR EL LIST_FIND PORQUE SOLO LE PODES PASAR UN PARAMETRO AL BOOL;
void fetch_pcb_con_sleep(int server_socket){
    
    int total_size;
    int offset = 0;
    t_pcb *PCBrec = malloc(sizeof(t_pcb));
    void *buffer;
    int length_nombre_inter;
    
    char *nomrebInterfaz;
    int tiempoDormir
    
    //tengo que crear un struct para la interfaz
    
    
    buffer = fetch_buffer(&total_size, server_socket); //RECIBO EL BUFFER 
   
    memcpy(&length_nombre_inter,buffer + offset, sizeof(int));  //RECIBO EL LENGTH DEL NOMBRE  DE LA INTERFAZ
    offset += sizeof(int); 

    nomrebInterfaz = malloc(length_nombre_inter);
    memcpy(nomrebInterfaz,buffer + offset,length_nombre_inter); //RECIBO EL NOMBRE DE LA INTERFAZ
    offset += length_nombre_inter; 

    memcpy(&tiempoDormir,buffer + offset,sizeof(int)); //RECIBO EL TIEMPO A DORMIR
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

    
    interfazBUSCADA = nombreInteraz;
    t_interfaz_registrada *interfaz = list_find(listaInterfaces,(void*)esLaInterfazBuscada);

    //armoProcesoPara

    if(interfaz->disponible == true){

    }else{
        list_add(interfaz->listaProcesosEsperando,PCBrec);
    }
    


    free(buffer);   
    free(interfazBUSCADA);   
    
    return PCBrec;

}

bool esLaInterfazBuscada(t_interfaz_registrada *recibida){
    if(strcmp(recibida->nombre,interfazBUSCADA)==0){
        return true;
    }else{ 
        return false;
    }
}