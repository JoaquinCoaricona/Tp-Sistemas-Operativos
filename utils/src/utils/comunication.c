#include "comunication.h"
// #include "utils.h"
// #include "../../memoria/include/instrucciones.h" estos dos estaban incluidos






static void process_conection(void *args)
{
    int client_socket;
    char *server_name;
    t_log *logger;
    t_packet *packet;
    //ACA HAY QUE ARREGLAR LO DE T_LIST COMO EN CPU, AHI FUNCIONO

    t_process_conection_args *arguments = (t_process_conection_args *)args;

    // Pasa los arguments para poder crear el thread
    logger = arguments->logger;
    client_socket = arguments->fd;
    server_name = arguments->server_name;
   

    free(args);

    while (client_socket != -1)
    {
        int operation_code = fetch_codop(client_socket);

        switch (operation_code)
        {
        case HANDSHAKE_KERNEL:
        case HANDSHAKE_ENTRADA_SALIDA:
        case HANDSHAKE_CPU:
            log_info(logger, "handshake %d recibido %s",operation_code, server_name);
            packet = fetch_packet(client_socket);
            list_destroy(packet);
            log_info(logger, "Packet received");

            //close_conection(client_socket);
            //client_socket = -1;
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
            //close_conection(client_socket);
            //client_socket = -1;
            break;
        case SOL_INSTRUCCION:
            devolverInstruccion(client_socket);
        break;
        case PATH_A_MEMORIA:
            
            //t_instrucciones *instruccionREC = malloc(sizeof(t_instrucciones));
            leer_pseudo(client_socket);
            //Dentro de leer_pseudo voy a crear la tabla de pagina porque ahi hago la lectura
            //del mensaje y ahi obtengo el Pid, podria cambiar la funcion y hacer que 
            // devuelvan a este archivo el pid pero para no cambiar lo que ya esta hecho lo hago dentro
        break;
        case RESIZE:
            resizePaginas(client_socket);
        break;
        case SOLICITAR_MARCO:
            buscarMarco(client_socket);
        break;
        case SOLICITUD_ESCRIBIR:
            escribirMemoria(client_socket);
        break;
        case SOLICITUD_LECTURA:
            leerMemoria(client_socket);
        break;
        case LIBERAR_ESTRUCTURAS:
            liberarEstructuras(client_socket);
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

int server_listen(t_log *logger, const char *server_name, int server_socket)
{
    log_info(logger, "listening...");
    pthread_t thread;

    int client_socket = wait_client(logger, server_name, server_socket);
    t_process_conection_args *args = malloc(sizeof(t_process_conection_args));

    args->logger = logger;
    args->server_name = server_name;
    args->fd = client_socket;

    pthread_create(&thread, NULL, (void *)process_conection, (void *)args);
    pthread_detach(thread);

    return 1;
}