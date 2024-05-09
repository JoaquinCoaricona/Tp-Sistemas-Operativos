#include "main.h"
// #include "../include/utils.h"

t_log *logger;
int server_dispatch_fd;
int client_fd_memoria;
bool continuar_con_el_ciclo_instruccion;
bool hay_interrupcion_pendiente = false;
int pid_ejecutando;
int pid_a_desalojar;

int main(int argc, char *argv[])
{
    char *memory_PORT;
    char *dispatch_PORT;
    char *interrupt_PORT;

    char *memory_IP;
    t_buffer *buffer;
    t_packet *packet;

    // LOGGER
    logger = initialize_logger("cpu.log", "cpu", true, LOG_LEVEL_INFO);

    // CONFIG
   
   // t_config *config = initialize_config(logger, "../cpu.config");
    t_config *config = initialize_config(logger, "cpu.config");
   
    memory_PORT = config_get_string_value(config, "PUERTO_MEMORIA");
    dispatch_PORT = config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH");
    interrupt_PORT = config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT");
    memory_IP = config_get_string_value(config, "IP_MEMORIA");

    // Conect to server
    client_fd_memoria = create_conection(logger, memory_IP, memory_PORT);
    log_info(logger, "Conectado al servidor de memoria %s:%s", memory_IP, memory_PORT);

    
    // Send handshake
    buffer = create_buffer();
    packet = create_packet(HANDSHAKE_CPU, buffer);

    add_to_packet(packet, buffer->stream, buffer->size);
    //packet = serialize_packet(packet, buffer->size);
    //send_packet(packet, client_fd);
    

    log_info(logger, "Handshake enviado");

    server_dispatch_fd = initialize_server(logger, "cpu_dispatch_server", memory_IP, dispatch_PORT); //ACA CPU Y MEMORIA  
    log_info(logger, "Server dispatch initialized");   //TIENEN LA MISMA IP PERO NO SON LA MISMA EN PC DISTINTIAS, TENER CUIDADO


    int server_interrupt_fd = initialize_server(logger, "cpu_interrupt_server", memory_IP, interrupt_PORT);
    log_info(logger, "Server interrupt initialized");

    pthread_t thread_memory_peticions;
    t_process_conection_args *process_conection_arguments= malloc(sizeof(t_process_conection_args));
    process_conection_arguments->server_name = "cpu_interrupt_server";
    process_conection_arguments->fd = server_interrupt_fd;

    pthread_create(&thread_memory_peticions,NULL,manage_interrupt_request,process_conection_arguments);
    pthread_detach(thread_memory_peticions);
    

    //ciclo_de_instruccion();
    //pedirInstruccion(5,2,client_fd_memoria);

    manage_dispatch_request();

    return 0;
}
//INTERUPT
void* manage_interrupt_request(void *args)
{
    int server_socket;
    char *server_name;
    t_packet *packet;

    t_process_conection_args *arguments = (t_process_conection_args *)args;

    // Pasa los arguments para poder crear el thread
    
    server_socket = arguments->fd;
    server_name = arguments->server_name;
   

    free(args);
    //aca el wait cliente espera un connect y eso se hace solo una vez, lo dejo afuera
    // el fetchcodop espera un send asi que ese lo dejo adentro
    int client_socket = wait_client(logger, server_name, server_socket);
    while (1)
    {
        int operation_code = fetch_codop(client_socket);

        switch (operation_code)
        {
        case HANDSHAKE_KERNEL:
            log_info(logger, "handshake %d recibido %s",operation_code, server_name);
            packet = fetch_packet(client_socket);
            log_info(logger, "Packet received");

            close_conection(client_socket);
            client_socket = -1;
            break;
        case INTERRUPCION:
            recibir_interrupcion(client_socket);
            break;
        // case MEMORIA_ENVIA_INSTRUCCION:
        //     fetch_instruccion_recibida_de_memoria(client_socket);
        // break;
        case -1:
            log_error(logger, "Error al recibir el codigo de operacion %s...", server_name);
            return;

        default:
            log_error(logger, "Algun error inesperado %s", server_name);
            return;
        }
    }

    log_warning(logger, "Conexion cerrada %s", server_name);
    return;
}
//DISPATCH
void manage_dispatch_request()
{
    t_packet *packet;
    int client_socket = wait_client(logger, "cpu_dispatch_server", server_dispatch_fd);
    //ACA ANTES ESTE CLIENT CONNECT ESTABA DENTRO DEL WHILE, ENTONCES TENIA UN WAIT CLIENTE
    //QUE SIEMPRE QUE EJECUTABA EL WHILE ESPERABA QUE LE MANDE UN CONNECT, PERO DEL OTRO LADO SOLO
    //SE MANDA UNA SOLA VEZ EN EL MAIN, DESPUES LA CONEXION QUEDA ABIERTA. ENTONCES POR ESO SOLAMENTE
    //SE EJECUTABA UNA VEZ Y DESPUES QUEDABA TRABAJO AHI EN EL WHILE PORQUE NO RECIBIA UN CONECT
    //AHORA QUE ESTA AFUERA SOLO ESPERA UNA VEZ EL CONECT Y DESPUES EL FETCHCODOP DE ABAJO ES BLOQUEANTE
    //PERO ES UN RECV Y ESPERA UN SEND QUE ES LO QUE LE MANDAN. Y POR ESO YA SE PUEDE PROBAR MAS DE UNA VEZ
    
    while (1)
    {
        int operation_code = fetch_codop(client_socket);

        switch (operation_code)
        {
        case HANDSHAKE_KERNEL:
            log_info(logger, "handshake %d recibido %s",operation_code, "cpu_dispatch_server");
            packet = fetch_packet(client_socket);
            log_info(logger, "Packet received");

            //close_conection(client_socket);
            //client_socket = -1;
            break;
        case PETICION_CPU:
            //Manejar una peticion a CPU
            break;
        case PCB_REC:
            ciclo_de_instruccion(client_socket);
            //t_pcb *PCBRECB = malloc(sizeof(t_pcb));
            //log_info(logger, "PCB %d recibido %s",operation_code, server_name);
            //fetch_PCB(client_socket,PCBRECB);
            //log_info(logger, "PCB RECIBIDO PID = %d PC = %d Q = %d ESTADO = %d",PCBRECB->pid,PCBRECB->program_counter,PCBRECB->quantum,PCBRECB->state);
             break;
        case -1:
            log_error(logger, "Error al recibir el codigo de operacion %s...", "cpu_dispatch_server");
            return;

        default:
            log_error(logger, "Algun error inesperado %s", "cpu_dispatch_server");
            return;
        }
    }

    log_warning(logger, "Conexion cerrada %s", "cpu_dispatch_server");
    return;
}

t_instruccion_unitaria *pedirInstruccion(int pid, int pc,int client_fd){
    t_buffer *bufferInstruccion;
    t_packet *packetInstruccion;

    
    //Inicializar Buffer y Packet
    
    bufferInstruccion   = create_buffer();
    packetInstruccion = create_packet(SOL_INSTRUCCION, bufferInstruccion);
    add_to_packet(packetInstruccion,&pid, sizeof(int));
    add_to_packet(packetInstruccion,&pc,sizeof(int));
    send_packet(packetInstruccion, client_fd); //client es el socket de memoria

    op_code opcode = recibir_operacion(client_fd); 

    if (opcode != MEMORIA_ENVIA_INSTRUCCION) {
		//log_error(logger,"No se pudo recibir la instruccion de memoria! codigo de operacion recibido: %d",opcode);
		printf("EROR AL RECIBIR EL CODIGO");
        exit ;
	}

    t_instruccion_unitaria *instruccion = malloc(sizeof(t_instruccion_unitaria));
    instruccion->parametro1_lenght = 0;
    instruccion->parametro2_lenght = 0;
    instruccion->parametro3_lenght = 0;

    int total_size;
    int offset = 0;
    void *buffer;
   
    buffer = fetch_buffer(&total_size, client_fd);

   
	memcpy(&(instruccion->opcode_lenght), buffer + offset, sizeof(int));
	offset+=sizeof(int);
	instruccion->opcode = malloc( instruccion->opcode_lenght);
	memcpy( instruccion->opcode, buffer+offset,  instruccion->opcode_lenght);
	offset+= instruccion->opcode_lenght;

	memcpy(&( instruccion->parametro1_lenght), buffer+offset, sizeof(int));
	offset+=sizeof(int);

    if(instruccion->parametro1_lenght != 0){

        instruccion->parametros[0] = malloc( instruccion->parametro1_lenght);
	    memcpy( instruccion->parametros[0], buffer + offset,  instruccion->parametro1_lenght);
        offset +=  instruccion->parametro1_lenght;

        memcpy(&(instruccion->parametro2_lenght), buffer+offset, sizeof(int));
	    offset+=sizeof(int);

        if(instruccion->parametro2_lenght != 0){
            instruccion->parametros[1] = malloc( instruccion->parametro2_lenght);
	        memcpy( instruccion->parametros[1], buffer + offset,  instruccion->parametro2_lenght);
	        offset +=  instruccion->parametro2_lenght;

            memcpy(&( instruccion->parametro3_lenght), buffer+offset, sizeof(int));
	        offset+=sizeof(int);

            if(instruccion->parametro3_lenght != 0){
                	instruccion->parametros[2] = malloc( instruccion->parametro3_lenght);
	                memcpy( instruccion->parametros[2], buffer + offset,  instruccion->parametro3_lenght);
	                offset +=  instruccion->parametro3_lenght;

                    memcpy(&( instruccion->parametro4_lenght), buffer+offset, sizeof(int));
	                offset+=sizeof(int);

                    if(instruccion->parametro4_lenght != 0){
                	    instruccion->parametros[3] = malloc( instruccion->parametro4_lenght);
	                    memcpy( instruccion->parametros[3], buffer + offset,  instruccion->parametro4_lenght);
	                    offset +=  instruccion->parametro4_lenght;

                        memcpy(&( instruccion->parametro5_lenght), buffer+offset, sizeof(int));
	                    offset+=sizeof(int);

                        if(instruccion->parametro5_lenght != 0){
                	        instruccion->parametros[4] = malloc( instruccion->parametro5_lenght);
	                        memcpy( instruccion->parametros[4], buffer + offset,  instruccion->parametro5_lenght);
	                        }
                    }
                }
            } 
        }

    free(buffer);
    return instruccion;

}

int recibir_operacion(int client_fd)
{
	int cod_op;
	if(recv(client_fd, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(client_fd);
		return -1;
	}
}

//hay que ver el tema de los hilos porque memoria no escucha en dispatch ni interrupt
// por otro lado cuando yo hago pedir instruccion aca en cpu lo envio por un puerto que despues no se si
// se usa como servidor para recibir las instrucciones, en el tp resuelto por el mismo socket que llego hacen
// la devolucion

// Revisar para que sirve dispath y interrurpot

// el archivo que hay que ver es el de cpu.c en el resuelto


//FETCH DECODE, EXCEC y CHECK INTERRUPT
void ciclo_de_instruccion(int socket_kernel){
    t_pcb *PCBACTUAL  =  malloc(sizeof(t_pcb));
    fetch_PCB(socket_kernel,PCBACTUAL);
    continuar_con_el_ciclo_instruccion = true;
    pid_ejecutando = PCBACTUAL->pid;
    t_instruccion_unitaria *instruccion_ACTUAL = NULL;

    while(continuar_con_el_ciclo_instruccion){
        //INICIO FASE FETCH
        
        instruccion_ACTUAL = pedirInstruccion(pid_ejecutando,PCBACTUAL->program_counter,client_fd_memoria);
		//log_info(logger, "Fetch Instrucción: PID: %d - FETCH - Program Counter: %d",contexto_actual->pid, contexto_actual->program_counter);
        PCBACTUAL->program_counter++;
        
        //INICIO FASES DECODE y EXECUTE
        if (strcmp(instruccion_ACTUAL->opcode, "SET") == 0) {
			operacion_set(PCBACTUAL, instruccion_ACTUAL);
		}
        if (strcmp(instruccion_ACTUAL->opcode, "SUM") == 0) {
			operacion_sum(PCBACTUAL, instruccion_ACTUAL);

		}
		if (strcmp(instruccion_ACTUAL->opcode, "SUB") == 0) {
			operacion_sub(PCBACTUAL, instruccion_ACTUAL);

		}
		if (strcmp(instruccion_ACTUAL->opcode, "JNZ") == 0) {

			operacion_jnz(PCBACTUAL, instruccion_ACTUAL); //PASAR EL PUNTERO DIRECTAMENTE, 
                                        //No pasar &PCBACTUAL porque eso es la direccion del punter
		}
        if (strcmp(instruccion_ACTUAL->opcode, "IO_GEN_SLEEP") == 0){
			//devolver_a_kernel(PCBACTUAL, SLEEP, socket_kernel); //esto estaba antes
			operacion_sleep(PCBACTUAL,socket_kernel,instruccion_ACTUAL);
            continuar_con_el_ciclo_instruccion = false;
        }    
        if (strcmp(instruccion_ACTUAL->opcode, "EXIT") == 0) {
            
            continuar_con_el_ciclo_instruccion = false;
            pid_ejecutando = 0;
			devolver_a_kernel(PCBACTUAL,socket_kernel,"INSTRUCCION EXIT");

		}   

        // //INICIO FASE CHECK INTERRUPT
        //el ultimo control del if, el de continuar con el ciclo
        //lo puse por el caso en el que sleep haga continuar_ciclo = false y justo llegue una interrupcion
        //porque ahi devolveria el kernel en sleep y aca devuelta y no estaria bien devolverlo dos veces
        //ese daria true todo el tiempo pero la cosa son los otros parametros del if que controlan 
        if(hay_interrupcion_pendiente && (pid_a_desalojar == pid_ejecutando && (continuar_con_el_ciclo_instruccion))) {
		//  log_info(logger, "Atendiendo interrupcion a %s y devuelvo a kernel", pid_a_desalojar);
		    continuar_con_el_ciclo_instruccion = false;
		    devolver_a_kernel_fin_quantum(PCBACTUAL, socket_kernel,"Fin de Quantum");
		    hay_interrupcion_pendiente = false;
	        pid_a_desalojar = 0;
		}

    }

	pid_ejecutando = 0;
	//contexto_ejecucion_destroy(PCBACTUAL);
    free(PCBACTUAL); //aca hay que crear una funcion destroy

}

void recibir_interrupcion(int socket_kernel) {

	

    int total_size;
    int offset = 0;
    
    void *buffer;
   
    int length_motivo;
    char *motivo = NULL;
   
    buffer = fetch_buffer(&total_size, socket_kernel);
    
    
    memcpy(&length_motivo,buffer + offset, sizeof(int)); //RECIBO EL TAMAÑO DEL STRING MOTIVO
    offset += sizeof(int);

    motivo = malloc(length_motivo);
    memcpy(motivo,buffer + offset, length_motivo); //RECIBO EL STRING MOTIVO
    offset += length_motivo;

    offset += sizeof(int); //SALTEO EL TAMAÑO DEL INT PID 
    
    memcpy(&pid_a_desalojar,buffer + offset, sizeof(int)); //RECIBO EL PID


    free(buffer);

    hay_interrupcion_pendiente = true;


	if(!continuar_con_el_ciclo_instruccion){//si no hay nadie ejecutando
		hay_interrupcion_pendiente = false;
        pid_a_desalojar = -1;
		responder_a_kernel("NO hay nadie", socket_kernel);
        
    }else if(pid_ejecutando && pid_a_desalojar != pid_ejecutando ){//si esta ejecutando otro proceso del que hay que desalojar
		hay_interrupcion_pendiente = false;
		pid_a_desalojar = -1;
		responder_a_kernel("El proceso ya fue desalojado, esta ejecutando otro proceso", socket_kernel);
	}
    //esto de arriba es casi imposible que se use porque si el proceso
    // que se esta ejecutando al final del quantum es distinto al que se estaba
    //ejecutnado al inicio entonces no se enviar la interrupcion



    //TENGO QUE IMPRIMIR EL MOTIVO DE LA INTERRUPCION QUE LLEGO DESDE KERNEL
	free(motivo);

}

void responder_a_kernel(char *mensaje ,int socket){

    t_buffer *buffer_rta;
    t_packet *packet_rta;
    buffer_rta = create_buffer();
    packet_rta = create_packet(INTERRUPCION_RTA_FALLIDA, buffer_rta);

    int length_rta = strlen(mensaje) + 1;

    add_to_packet(packet_rta, mensaje, length_rta);
    
    send_packet(packet_rta, socket);
    destroy_packet(packet_rta);
}
void devolver_a_kernel(t_pcb *contexto_actual, int socket_kernel,char *motivo){

    t_buffer *buffer_rta;
    t_packet *packet_rta;
    buffer_rta = create_buffer();
    packet_rta = create_packet(INTERRUPCION_RTA_CON_PCB,buffer_rta);

    int length_motivo = strlen(motivo) + 1;
    add_to_packet(packet_rta, motivo, length_motivo); //DEVUELVO EL MOTIVO DE INTERRUPCION

    int tamanioPCB = sizeof(t_pcb);
    add_to_packet(packet_rta, contexto_actual, tamanioPCB); //CARGO EL PCB ACTUALIZADO
    
    send_packet(packet_rta, socket_kernel);
    destroy_packet(packet_rta);

}

void devolver_a_kernel_fin_quantum(t_pcb *contexto_actual, int socket_kernel,char *motivo){
    
    t_buffer *buffer_rta;
    t_packet *packet_rta;
    buffer_rta = create_buffer();
    packet_rta = create_packet(INTERRUPCION_FIN_QUANTUM,buffer_rta);

    int length_motivo = strlen(motivo) + 1;
    add_to_packet(packet_rta, motivo, length_motivo); //DEVUELVO EL MOTIVO DE INTERRUPCION

    int tamanioPCB = sizeof(t_pcb);
    add_to_packet(packet_rta, contexto_actual, tamanioPCB); //CARGO EL PCB ACTUALIZADO
    
    send_packet(packet_rta, socket_kernel);
    destroy_packet(packet_rta);

}



