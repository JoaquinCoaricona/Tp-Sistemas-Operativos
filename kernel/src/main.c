#include "main.h"
#include "short_term_scheduler.h"
#include "long_term_scheduler.h"

int memory_socket;
int cpu_dispatch_socket;
int cpu_interrupt_socket;
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
    t_buffer *bufferPCB;
    t_packet *packet_handshake;
    t_packet *packetPCB;


    // LOGGER
    logger = initialize_logger("kernel.log", "kernel", true, LOG_LEVEL_INFO);

    // CONFIG
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
    // t_pcb *PCB = initializePCB();

    
    // enterNew(PCB,logger);
    // t_pcb PRUEBA;

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

    send_packet(packet_handshake, cpu_dispatch_socket);
    send_packet(packet_handshake, cpu_interrupt_socket);

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
    process_conection_arguments_dispatch->server_name = "dispatch_server";
    process_conection_arguments_dispatch->fd = cpu_dispatch_socket;

    pthread_create(&thread_dispatch,NULL,manage_request_from_dispatch,process_conection_arguments_dispatch);
    pthread_detach(thread_dispatch);

    

    levantar_consola(logger);
    return 0;
 }

void* manage_request_from_input_output(void *args)
{
    int server_socket;
    char *server_name;
    t_packet *packet;

    t_process_conection_args *arguments = (t_process_conection_args *)args;

    // Pasa los arguments para poder crear el thread
    
    server_socket = arguments->fd;
    server_name = arguments->server_name;
   

    free(args);

    while (1)
    {
        int client_socket = wait_client(logger, server_name, server_socket);
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
        case PCB_REC:
            t_pcb *PCBRECB = malloc(sizeof(t_pcb));
            log_info(logger, "PCB %d recibido %s",operation_code, server_name);
            fetch_PCB(client_socket,PCBRECB);
            log_info(logger, "PCB RECIBIDO PID = %d PC = %d Q = %d ESTADO = %d",PCBRECB->pid,PCBRECB->program_counter,PCBRECB->quantum,PCBRECB->state);
            //aca arriba intente loguear lo recibido en PCBRECB
            //aca lo que hacia era crear un puntero antes de llamar a fetch_pcb y despues igualaba ese puntero
            //al resultado de la funcion. Eso devolvia solo la direccion pero no se podia acceder a los campos
            //para solucionarlo habia que crear ese puntero y pasarlo como parametro directamente y que en la funcion
            //escriban sobre ese puntero y despues ya no te devuelve nada porque le pasaste el puntero
            close_conection(client_socket);
            client_socket = -1;
            break;
        case CREAR_PROCESO:
            
            log_info(logger,"Se Envio Un Proceso a Crear");
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
    int server_socket;
    char *server_name;
    t_packet *packet;

    t_process_conection_args *arguments = (t_process_conection_args *)args;

    // Pasa los arguments para poder crear el thread
    
    server_socket = arguments->fd;
    server_name = arguments->server_name;
   

    free(args);

    while (1)
    {
        int client_socket = wait_client(logger, server_name, server_socket);
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
        case PCB_REC:
            t_pcb *PCBRECB = malloc(sizeof(t_pcb));
            log_info(logger, "PCB %d recibido %s",operation_code, server_name);
            fetch_PCB(client_socket,PCBRECB);
            log_info(logger, "PCB RECIBIDO PID = %d PC = %d Q = %d ESTADO = %d",PCBRECB->pid,PCBRECB->program_counter,PCBRECB->quantum,PCBRECB->state);
            //aca arriba intente loguear lo recibido en PCBRECB
            //aca lo que hacia era crear un puntero antes de llamar a fetch_pcb y despues igualaba ese puntero
            //al resultado de la funcion. Eso devolvia solo la direccion pero no se podia acceder a los campos
            //para solucionarlo habia que crear ese puntero y pasarlo como parametro directamente y que en la funcion
            //escriban sobre ese puntero y despues ya no te devuelve nada porque le pasaste el puntero
            // close_conection(client_socket);
            // client_socket = -1;
            break;
        case CREAR_PROCESO:
            
            log_info(logger,"Se Envio Un Proceso a Crear");
            close_conection(client_socket);
            client_socket = -1;
        break;
        case INTERRUPCION_RTA_CON_PCB:
            printf("Llego Un PCB");
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

void enviar_path_a_memoria(char *path){
    //ENVIAR PATH A MEMORIA

    //Declaraciones
    t_buffer *bufferMemoria;
    t_packet *packetMemoria;
    int pid = 5;

    //Inicializar Buffer y Packet
    bufferMemoria   = create_buffer();
    packetMemoria = create_packet(PATH_A_MEMORIA, bufferMemoria);
    add_to_packet(packetMemoria,&pid, sizeof(int));
    add_to_packet(packetMemoria,path,(strlen(path)+1));
    send_packet(packetMemoria, memory_socket);

}

void iniciar_proceso(char *path){
    //ENVIO EL PATH A MEMORIA PARA QUE BUSRQUE LAS INTRUCCIONES EN EL ARCHIVO
    enviar_path_a_memoria(path);
    
    //CREO EL PCB
    t_pcb *PCB = initializePCB();
    enterNew(PCB);
    t_pcb PRUEBA;
    int tamanioPCB = sizeof(PRUEBA);
    
    //ENVIAR PCB esto en realidad se deberia hacer cuando le toque ejecturse
    
    bufferPCB = create_buffer();
    packetPCB = create_packet(PCB_REC, bufferPCB);
    add_to_packet(packetPCB, PCB, tamanioPCB);
    send_packet(packetPCB, cpu_dispatch_socket);

}