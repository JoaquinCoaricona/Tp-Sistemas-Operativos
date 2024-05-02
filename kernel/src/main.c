#include "main.h"
#include "short_term_scheduler.h"
#include "long_term_scheduler.h"

int memory_socket;
int cpu_dispatch_socket;
int cpu_interrupt_socket;
int PID; //Global
t_log *logger;
t_pcb *pcbEJECUTANDO;


int main(int argc, char *argv[])
{

    char *memory_PORT;
    char *cpu_dispatch_PORT;
    char *cpu_interrupt_PORT;
    char *kernel_PORT;
    char *memory_IP;
    char *cpu_IP;
    char *kernel_IP;
    t_buffer *buffer;
    t_packet *packet_handshake;
    PID = 0;


    // LOGGER
    logger = initialize_logger("kernel.log", "kernel", true, LOG_LEVEL_INFO);

    // CONFIG
//    t_config *config = initialize_config(logger, "../kernel.config");
    t_config *config = initialize_config(logger, "kernel.config");

    memory_PORT = config_get_string_value(config, "PUERTO_MEMORIA");
    cpu_dispatch_PORT = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
    cpu_interrupt_PORT = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");
    kernel_PORT = config_get_string_value(config, "PUERTO_KERNEL"); //! Averiguar se deberia estar
    cpu_IP = config_get_string_value(config, "IP_CPU");
    memory_IP = config_get_string_value(config, "IP_MEMORIA"); //! Averiguar se deberia estar
    kernel_IP = config_get_string_value(config, "IP_MEMORIA");// hay que ver si se tiene que cambiar a futuro pero en el archivo de configuración no hay una ip de kernel
   
    //PRUEBAAAA
    initialize_queue_and_semaphore();

    //Iniciar Proceso
    //create_process("src/programa1.txt");
    //create_process("src/programa2.txt");


    
    
    // Conect to server
    memory_socket = create_conection(logger, memory_IP, memory_PORT);
    log_info(logger, "Conectado al servidor de memoria %s:%s", memory_IP, memory_PORT);

    // Send handshake
    buffer = create_buffer();
    packet_handshake = create_packet(HANDSHAKE_KERNEL,buffer);

    //printf("TAMAÑO %i",sizeof(*PCB));



    // int tamanioPCB = sizeof(PRUEBA);
    // ENVIAR PCB
    // bufferPCB = create_buffer();
    // packetPCB = create_packet(PCB_REC, bufferPCB);
    // add_to_packet(packetPCB, PCB, tamanioPCB);

   
    

    add_to_packet(packet_handshake, buffer->stream, buffer->size);
    //packet = serialize_packet(packet, buffer->size);
    //send_packet(packet_handshake, memory_socket);

    log_info(logger, "Handshake enviado");

    cpu_dispatch_socket = create_conection(logger, cpu_IP, cpu_dispatch_PORT);
    log_info(logger, "Conectado al servidor de cpu %s:%s", cpu_IP, cpu_dispatch_PORT);

    cpu_interrupt_socket = create_conection(logger, cpu_IP, cpu_interrupt_PORT);
    log_info(logger, "Conectado al servidor de cpu %s:%s", cpu_IP, cpu_interrupt_PORT);

    //send_packet(packet_handshake, cpu_dispatch_socket);
    send_packet(packet_handshake, cpu_interrupt_socket);

    //create_process("prueba1");

    //send_packet(packetPCB, cpu_dispatch_socket);


    
    int server_fd = initialize_server(logger, "kernel_server", kernel_IP, kernel_PORT);
    log_info(logger, "Server initialized");

    pthread_t thread_memory_peticions;
    t_process_conection_args *process_conection_arguments= malloc(sizeof(t_process_conection_args));
    process_conection_arguments->server_name = "kernel_server";
    process_conection_arguments->fd = server_fd;

    pthread_create(&thread_memory_peticions,NULL,manage_request_from_input_output,process_conection_arguments);
    pthread_detach(thread_memory_peticions);

    //HILO DISPATCH
    
    pthread_t thread_dispatch;
    t_process_conection_args *process_conection_arguments_dispatch= malloc(sizeof(t_process_conection_args));
    process_conection_arguments_dispatch->fd = cpu_dispatch_socket;
    process_conection_arguments->server_name = "dispatch_escucha";

    pthread_create(&thread_dispatch,NULL,manage_request_from_dispatch,process_conection_arguments_dispatch);
    pthread_detach(thread_dispatch);



    levantar_consola(logger);
    return 0;
 }

void* manage_request_from_input_output(void *args)
{
    int server_socket;
    char *server_name;
    t_list *packet;

    t_process_conection_args *arguments = (t_process_conection_args *)args;

    // Pasa los arguments para poder crear el thread
    
    server_socket = arguments->fd;
    server_name = arguments->server_name;
   

    free(args);

    int client_socket = wait_client(logger, server_name, server_socket);
    while (1)
    {
        int operation_code = fetch_codop(client_socket);

        switch (operation_code)
        {
        case HANDSHAKE_ENTRADA_SALIDA:
        case HANDSHAKE_CPU:
            log_info(logger, "handshake %d recibido %s",operation_code, server_name);
            packet = fetch_packet(client_socket);
            log_info(logger, "Packet received");

            close_conection(client_socket);
            client_socket = -1;
            break;

        case -1:
            log_error(logger, "Error al recibir el codigo de operacion %s...", server_name);
            return;

        default:
            log_error(logger, "Alguno error inesperado %s", server_name);
            return;
        }
    }

    log_warning(logger, "Conexion cerrada %s", server_name);
    return;
}

void* manage_request_from_dispatch(void *args)
{   


    t_process_conection_args * arguments = (t_process_conection_args *) args;

    int server_socket;
    char *server_name;

    server_socket = arguments->fd;
    server_name = arguments->server_name;

    free(args);

    while (1)
    {
        
        int operation_code = fetch_codop(server_socket);
        //ACLARACION: Si yo hago fetchcodop y solo saco el codop, el resto del buffer que me mandaron
        //sigue en el socket, entonces si cuando entro al case solo hago un imprimir y no leo el resto del buffer
        //entonces cuando vuelva a entrar al bucle y lea fetchcodop de vuelta lo que va a hacer el leer el codop
        //pero leyendo lo que falto deserializar del buffer y si tenia que quedar esperando no va a esperar
        //porque el socket todavia tiene algo cargado

        switch (operation_code)
        {
   
        case INTERRUPCION_RTA_CON_PCB:
            //printf("Llego Un PCB");
            log_info(logger,"LLEGO UN PCB");
            fetch_pcb_actualizado(server_socket);
        break;
        case -1:
            //log_error(logger, "Error al recibir el codigo de operacion %s...", server_name);
            return;

        default:
            //log_error(logger, "Alguno error inesperado %s", server_name);
            return;
        }
    }

    log_warning(logger, "Conexion cerrada %s", server_name);
    return;
}

void enviar_path_a_memoria(char *path){
    //ENVIAR PATH A MEMORIA

    //Declaraciones
    t_buffer *bufferMemoria;
    t_packet *packetMemoria;
    //Inicializar Buffer y Packet
    bufferMemoria   = create_buffer();
    packetMemoria = create_packet(PATH_A_MEMORIA, bufferMemoria);
    add_to_packet(packetMemoria,&PID, sizeof(int)); //! PID global
    add_to_packet(packetMemoria,path,(strlen(path)+1));
    send_packet(packetMemoria, memory_socket);

}

void create_process(char* path) {

    
    //CREACION DE UN NUEVO PROCESO
    t_pcb *PCB = initializePCB(PID); 
    enterNew(PCB);
    t_pcb PCBPRUEBA;
    int sizePCB = sizeof(PCBPRUEBA);

    t_buffer *bufferPCB;
 
    t_packet *packetPCB;
    enviar_path_a_memoria(path);
    PID += 1;  // CAMBIO DE ORDEN, primer creo el pcb y envio a memoria y despues sumo el pid. PARA QUE a cpu y memoria
                //lleguen con el mismo pid, sino llegaba uno con uno y el otro con +1
                
    sleep(3);
    
    //ENVIAR PCB esto en realidad se deberia hacer cuando le toque ejecturse
    bufferPCB = create_buffer();
    packetPCB = create_packet(PCB_REC, bufferPCB);
    add_to_packet(packetPCB, PCB, sizePCB);
    send_packet(packetPCB, cpu_dispatch_socket);
}

void end_process(){

}


void fetch_pcb_actualizado(server_socket){
    int total_size;
    int offset = 0;
    t_pcb *PCBrec = malloc(sizeof(t_pcb));
    void *buffer;
    int length_motivo;
    char *motivo;
    int tama; //Solo para recibir el size que esta al principio del buffer
    
    buffer = fetch_buffer(&total_size, server_socket);
   
    memcpy(&length_motivo,buffer + offset, sizeof(int)); 
    offset += sizeof(int);  

    
    
    motivo = malloc(length_motivo);
    memcpy(motivo,buffer + offset, length_motivo); //SI TENGO QUE COPIAR EL LENGTH, NO TENGO QUE PONER SIZEOF(LENGTH)
    offset += length_motivo;        //tengo que poner directamente el length en el ultimo param de memcpy
                                   // y lo mismo en el offset al sumarle, tengo que sumar lo que copie en memcpy
    
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

   
    log_info(logger, "Motivo Recibido : %s",motivo);
    log_info(logger, "PID RECIBIDO : %i",PCBrec->pid);
    log_info(logger, "PC RECIBIDO : %i",PCBrec->program_counter);
    log_info(logger, "REGISTRO AX : %i",PCBrec->registers.AX);
    log_info(logger, "REGISTRO BX : %i",PCBrec->registers.BX);




    free(buffer);   
    free(motivo);

}

 
