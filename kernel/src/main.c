#include "main.h"

int memory_socket;
int cpu_dispatch_socket;
int cpu_interrupt_socket;
int PID; // Global
t_log *logger;
t_pcb *pcbEJECUTANDO;
t_list *listaInterfaces;
int gradoMultiprogramacion;
int salteoPostAlSemaforo;
int quantumGlobal;
int procesoEjectuandoActualmente;
char *algoritmo_planificacion;
bool planificacion_detenida;
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
    salteoPostAlSemaforo = 0;
    planificacion_detenida = false;
    listaInterfaces = list_create();
    procesoEjectuandoActualmente = -2;
    // Aca le pongo -2 porque es un numero que se usa en el LongTermScheduler para
    // identificar que es la primera vez y necesita el Post el semaforo de corto plazo
    // Despues los proximos posts los va a recibir cuando reciben un pcb en dispatch
    // Pero si ponia -1 aca, en dispatch cuando queda sin ejecutar proceso le pongo -1
    // y entraria por el if del largo plazo y tendria dos post. Entonces le pongo
    //-2 solo para el primer acceso y despues es -1 cuando no hay proceso ejecutando
    // y todos los posts los recibe de dispatch y cuanndo se queda sin
    // procesos en ready se traba ahi pero ya tiene el post del semaforo corto plazo

    // LOGGER
    logger = initialize_logger("kernel.log", "kernel", true, LOG_LEVEL_INFO);

    // CONFIG
    // t_config *config = initialize_config(logger, "../kernel.config");
    t_config *config = initialize_config(logger, "kernel.config");

    memory_PORT = config_get_string_value(config, "PUERTO_MEMORIA");
    cpu_dispatch_PORT = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
    cpu_interrupt_PORT = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");
    kernel_PORT = config_get_string_value(config, "PUERTO_KERNEL"); //! Averiguar se deberia estar
    cpu_IP = config_get_string_value(config, "IP_CPU");
    memory_IP = config_get_string_value(config, "IP_MEMORIA"); //! Averiguar se deberia estar
    kernel_IP = config_get_string_value(config, "IP_MEMORIA"); // hay que ver si se tiene que cambiar a futuro pero en el archivo de configuración no hay una ip de kernel
    gradoMultiprogramacion = atoi(config_get_string_value(config, "GRADO_MULTIPROGRAMACION"));
    quantumGlobal = atoi(config_get_string_value(config, "QUANTUM"));
    algoritmo_planificacion = config_get_string_value(config, "ALGORITMO_PLANIFICACION");

    // PRUEBAAAA
    initialize_queue_and_semaphore();

    // Conect to server
    memory_socket = create_conection(logger, memory_IP, memory_PORT);
    log_info(logger, "Conectado al servidor de memoria %s:%s", memory_IP, memory_PORT);

    // Send handshake
    buffer = create_buffer();
    packet_handshake = create_packet(HANDSHAKE_KERNEL, buffer);

    add_to_packet(packet_handshake, buffer->stream, buffer->size);
    // packet = serialize_packet(packet, buffer->size);
    // send_packet(packet_handshake, memory_socket);

    log_info(logger, "Handshake enviado");

    cpu_dispatch_socket = create_conection(logger, cpu_IP, cpu_dispatch_PORT);
    log_info(logger, "Conectado al servidor de cpu %s:%s", cpu_IP, cpu_dispatch_PORT);

    cpu_interrupt_socket = create_conection(logger, cpu_IP, cpu_interrupt_PORT);
    log_info(logger, "Conectado al servidor de cpu %s:%s", cpu_IP, cpu_interrupt_PORT);

    send_packet(packet_handshake, cpu_dispatch_socket); // este send estaba comentado pero funciona
    send_packet(packet_handshake, cpu_interrupt_socket);

    // create_process("prueba1");

    // send_packet(packetPCB, cpu_dispatch_socket);

    int server_fd = initialize_server(logger, "kernel_server", kernel_IP, kernel_PORT);
    log_info(logger, "Server initialized");

    // HILO IO
    pthread_t thread_memory_peticions;
    t_process_conection_args *process_conection_arguments = malloc(sizeof(t_process_conection_args));
    process_conection_arguments->server_name = "kernel_server";
    process_conection_arguments->fd = server_fd;

    pthread_create(&thread_memory_peticions, NULL, manage_request_from_input_output, process_conection_arguments);
    pthread_detach(thread_memory_peticions);

    // HILO DISPATCH

    pthread_t thread_dispatch;
    t_process_conection_args *process_conection_arguments_dispatch = malloc(sizeof(t_process_conection_args));
    process_conection_arguments_dispatch->fd = cpu_dispatch_socket;
    process_conection_arguments->server_name = "dispatch_escucha";

    pthread_create(&thread_dispatch, NULL, manage_request_from_dispatch, process_conection_arguments_dispatch);
    pthread_detach(thread_dispatch);

    // HILO PLANIFICADOR LARGO PLAZO

    pthread_t planificadorLargoPlazo;
    pthread_create(&planificadorLargoPlazo, NULL, Aready, NULL);
    pthread_detach(planificadorLargoPlazo);

    // HILO PLANIFICADOR CORTO PLAZO

    pthread_t planificadorDeCortoPlazo;
    pthread_create(&planificadorDeCortoPlazo, NULL, planificadorCortoPlazo, NULL);
    pthread_detach(planificadorDeCortoPlazo);

    destroy_packet(packet_handshake);

    levantar_consola(logger);
    return 0;
}

void *manage_request_from_input_output(void *args)
{
    int server_socket;
    char *server_name;
    t_packet *packet;

    t_process_conection_args *arguments = (t_process_conection_args *)args;

    // Pasa los arguments para poder crear el thread

    server_socket = arguments->fd;
    server_name = arguments->server_name;

    free(args);
    // EN ESTE CASO COMO LEVANTAR UNA INTERFAZ NUEVA IMPLICA QUE ESA INTERFAZ SE CONECTE
    // Y HAGA TODO EL CONNECT CON EL SERVER DE VUELTA ENTONCES EL WAITCLIENT TIENE QUE ESTAR
    // DENTRO DEL WHILE PORQUE LE VA A LLEGAR UN CONNECT, NO UN SEND. PORQUE LA INTERFAZ SE LEVANTA
    //  DE CERO

    while (1)
    {
        int client_socket = wait_client(logger, server_name, server_socket);
        int operation_code = fetch_codop(client_socket);

        switch (operation_code)
        {
        case NUEVA_INTERFAZ:
            t_interfaz_registrada *recibida = NULL;
            recibida = recibir_interfaz(client_socket);
            crear_hilo_interfaz(recibida); // ESTA EN RECEPCION.C CREO EL HILO
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

void *manage_request_from_dispatch(void *args)
{

    t_process_conection_args *arguments = (t_process_conection_args *)args;

    int server_socket;
    char *server_name;

    server_socket = arguments->fd;
    server_name = arguments->server_name;

    free(args);

    while (1)
    {

        int operation_code = fetch_codop(server_socket);
        // ACLARACION: Si yo hago fetchcodop y solo saco el codop, el resto del buffer que me mandaron
        // sigue en el socket, entonces si cuando entro al case solo hago un imprimir y no leo el resto del buffer
        // entonces cuando vuelva a entrar al bucle y lea fetchcodop de vuelta lo que va a hacer el leer el codop
        // pero leyendo lo que falto deserializar del buffer y si tenia que quedar esperando no va a esperar
        // porque el socket todavia tiene algo cargado

        switch (operation_code)
        {
            // este caso es la salida Por instruccion EXIT
        case INTERRUPCION_RTA_CON_PCB:
            pthread_mutex_lock(&m_procesoEjectuandoActualmente);
            procesoEjectuandoActualmente = -1;
            pthread_mutex_unlock(&m_procesoEjectuandoActualmente);
            // printf("Llego Un PCB");
            log_info(logger, "LLEGO UN PCB");
            fetch_pcb_actualizado(server_socket);
            //Aca Controlo que solamente en VRR hago destroy al t_temporal
            if(string_equals_ignore_case(algoritmo_planificacion, "VRR")){
            temporal_destroy(timer);
            }
            sem_post(&short_term_scheduler_semaphore);
            controlGradoMultiprogramacion();
            //sem_post(&sem_multiprogramacion);
        break;
        case INTERRUPCION_FIN_QUANTUM:
            pthread_mutex_lock(&m_procesoEjectuandoActualmente);
            procesoEjectuandoActualmente = -1;
            pthread_mutex_unlock(&m_procesoEjectuandoActualmente);
            log_info(logger, "LLEGO UN PCB POR FIN DE QUANTUM");
            fetch_pcb_actualizado_fin_quantum(server_socket);
            //Aca Controlo que solamente en VRR hago destroy al t_temporal
            if(string_equals_ignore_case(algoritmo_planificacion, "VRR")){
            temporal_destroy(timer);
            }
            sem_post(&short_term_scheduler_semaphore);
            // sem_post(&sem_multiprogramacion);
            // el de multiprogramacion aca no, porque es fin de quantum, el proceso
            // no terminno, volvio a ready, solo cuando llega a exit podemos hacer
            // un post al grado de multiprogramacion
            break;
        case INTERRUPCION_ELIMINAR_PROCESO:
            pthread_mutex_lock(&m_procesoEjectuandoActualmente);
            procesoEjectuandoActualmente = -1;
            pthread_mutex_unlock(&m_procesoEjectuandoActualmente);
            log_info(logger, "LLEGO UN PCB PARA ELIMINARSE");
            fetch_pcb_actualizado_A_eliminar(server_socket);
            //Aca Controlo que solamente en VRR hago destroy al t_temporal
            if(string_equals_ignore_case(algoritmo_planificacion, "VRR")){
            temporal_destroy(timer);
            }
            sem_post(&short_term_scheduler_semaphore);
            controlGradoMultiprogramacion();
            //sem_post(&sem_multiprogramacion);
            //ACA SI MULTIPROGRAMACION PORQUE ELIMINAMOS A UN PROCESO
        break;
        case SLEEP_IO:
            pthread_mutex_lock(&m_procesoEjectuandoActualmente);
            procesoEjectuandoActualmente = -1;
            pthread_mutex_unlock(&m_procesoEjectuandoActualmente);
            t_pcb *receptorPCB = NULL;
            t_interfaz_registrada *interfaz = NULL;
            int tiempoDormir;
            char *nombreInter = NULL;
            receptorPCB = fetch_pcb_con_sleep(server_socket, &tiempoDormir, &nombreInter);
            // ACA HABIA UN ERROR CON EL PUNTERO NOMBREINTERFAZ PORQUE
            // LE ESTABA PASANDO EL PUNTERO PERO LA COPIA, OSEA QUE NO CAMBIABA EL DE ACA Y QUEDABA EN NULL
            // Y DESPUES EN EL FIND NO PODIA COMPAARAR CON NULL. TODO POR INTENTAR SEPARAR EN FUNCIONES
            // DEL OTRO LADO RECIBO COMO PUNTERO A PUNTERO ** Y AL HACERLE EL MEMCPY
            // DESREFERENCIO CON EL *
            
            //El control al T_temporal es solo si estamos en VRR
            if(string_equals_ignore_case(algoritmo_planificacion, "VRR")){
            obtenerDatosTemporal();
            //la multiplicacion es para pasarlo a microsegundos, que es lo usa usleep
            //t_temporal devuelve milisegundos. El quantumglobal esta en microsegundos
            receptorPCB->quantum = receptorPCB->quantum - (ms_transcurridos * 1000);
            //Hago Control de que el quantumRestante no sea negativo, en caso que sea neagativo, le cargo el original
            //para que al volver de IO, lo carguen a la cola de ready y no a la prioritaria
            //Esto pasaba cuando enviabamos varias veces a IO dentro de un mismo quantum, quizas entre que envia la 
            //interrupcion y que volvia pasaba mas tiempo del que realmente se demoro en ejecutar la instruccion en cpu
            //y al hacer las restas quedaba un valor negativo, y como al volver de IO solo se fije que sea igual al 
            //quantumgloabl entonces podias terminar en la cola prioritaria teniendo quantum negativo.
            if(receptorPCB->quantum < 0){
                receptorPCB->quantum = quantumGlobal;
                log_info(logger,"El Quantum era negativo, asigno el quantumGlobal");
            }
            }
            interfaz = buscar_interfaz(nombreInter);
            cargarEnListaIO(receptorPCB, interfaz, tiempoDormir);
            sem_post(&short_term_scheduler_semaphore);
            // sem_post(&sem_multiprogramacion);
            // aca el grado de multiprogramacion no cambia, porque los procesos en block tambien entratran dentro del
            // grado de multiprogramacion, solo cuando sale por exit se aumenta el grado de multiprogramacion
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

void enviar_path_a_memoria(char *path)
{
    // ENVIAR PATH A MEMORIA

    // Declaraciones
    t_buffer *bufferMemoria;
    t_packet *packetMemoria;
    // Inicializar Buffer y Packet
    bufferMemoria = create_buffer();
    packetMemoria = create_packet(PATH_A_MEMORIA, bufferMemoria);
    add_to_packet(packetMemoria, &PID, sizeof(int)); //! PID global
    add_to_packet(packetMemoria, path, (strlen(path) + 1));
    send_packet(packetMemoria, memory_socket);

    destroy_packet(packetMemoria);
}

void create_process(char *path)
{

    // CREACION DE UN NUEVO PROCESO
    t_pcb *PCB = initializePCB(PID);
    // t_pcb PCBPRUEBA;
    // int sizePCB = sizeof(PCBPRUEBA);

    // t_buffer *bufferPCB;

    // t_packet *packetPCB;
    enviar_path_a_memoria(path);

    // primero envio el path a memoria, espero que me confirme que termino de leer
    // y despues cargo el pcb en New y despues le sumo uno al global de Pid
    int pidTeminadaLectura;
    int operation_code = fetch_codop(memory_socket); // aca se queda bloqueante esperando la respuesta
    int total_size;
    int offset = 0;
    void *buffer2 = fetch_buffer(&total_size, memory_socket); // recibo porque puse un numero en el buffer

    offset += sizeof(int);                                      // salteo el tamano de int
    memcpy(&pidTeminadaLectura, buffer2 + offset, sizeof(int)); // RECIBO EL pid

    if (operation_code == MEMORIA_TERMINO_LECTURA)
    {
        log_info(logger, "Pid %d Lectura Terminada", pidTeminadaLectura);
    }
    free(buffer2);

    agregarANew(PCB);
    PID += 1; // CAMBIO DE ORDEN, primer creo el pcb y envio a memoria y despues sumo el pid. PARA QUE a cpu y memoria
              // lleguen con el mismo pid, sino llegaba uno con uno y el otro con +1

    // EL sem_post(&sem_hay_pcb_esperando_ready);lo hago en agregarAnew

    // //ENVIAR PCB esto en realidad se deberia hacer cuando le toque ejecturse
    // bufferPCB = create_buffer();
    // packetPCB = create_packet(PCB_REC, bufferPCB);
    // add_to_packet(packetPCB, PCB, sizePCB);
    // send_packet(packetPCB, cpu_dispatch_socket);
}

void end_process()
{
}

void fetch_pcb_actualizado(server_socket)
{
    int total_size;
    int offset = 0;
    t_pcb *PCBrec = pcbEJECUTANDO;
    void *buffer;
    int length_motivo;
    char *motivo;
    int tama; // Solo para recibir el size que esta al principio del buffer

    buffer = fetch_buffer(&total_size, server_socket);

    memcpy(&length_motivo, buffer + offset, sizeof(int));
    offset += sizeof(int);

    motivo = malloc(length_motivo);
    memcpy(motivo, buffer + offset, length_motivo); // SI TENGO QUE COPIAR EL LENGTH, NO TENGO QUE PONER SIZEOF(LENGTH)
    offset += length_motivo;                        // tengo que poner directamente el length en el ultimo param de memcpy
                             //  y lo mismo en el offset al sumarle, tengo que sumar lo que copie en memcpy

    offset += sizeof(int); // Salteo El tamaño del PCB
    // aca uso el puntero global que apunta al pcb actual: pcbEJECUTANDO
    // y actualizo ese pcb y despues lo pongo en NUll

    memcpy(&(pcbEJECUTANDO->pid), buffer + offset, sizeof(int)); // RECIBO EL PID
    offset += sizeof(int);

    memcpy(&(pcbEJECUTANDO->program_counter), buffer + offset, sizeof(int)); // RECIBO EL PROGRAM COUNTER
    offset += sizeof(int);

    memcpy(&(pcbEJECUTANDO->quantum), buffer + offset, sizeof(int)); // RECIBO EL QUANTUM
    offset += sizeof(int);

    memcpy(&(pcbEJECUTANDO->state), buffer + offset, sizeof(t_process_state)); // RECIBO EL PROCESS STATE
    offset += sizeof(t_process_state);

    memcpy(&(pcbEJECUTANDO->registers.PC), buffer + offset, sizeof(uint32_t)); // RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(pcbEJECUTANDO->registers.AX), buffer + offset, sizeof(uint8_t)); // RECIBO CPUREG
    offset += sizeof(uint8_t);
    memcpy(&(pcbEJECUTANDO->registers.BX), buffer + offset, sizeof(uint8_t)); // RECIBO CPUREG
    offset += sizeof(uint8_t);
    memcpy(&(pcbEJECUTANDO->registers.CX), buffer + offset, sizeof(uint8_t)); // RECIBO CPUREG
    offset += sizeof(uint8_t);
    memcpy(&(pcbEJECUTANDO->registers.DX), buffer + offset, sizeof(uint8_t)); // RECIBO CPUREG
    offset += sizeof(uint8_t);
    memcpy(&(pcbEJECUTANDO->registers.EAX), buffer + offset, sizeof(uint32_t)); // RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(pcbEJECUTANDO->registers.EBX), buffer + offset, sizeof(uint32_t)); // RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(pcbEJECUTANDO->registers.ECX), buffer + offset, sizeof(uint32_t)); // RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(pcbEJECUTANDO->registers.EDX), buffer + offset, sizeof(uint32_t)); // RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(pcbEJECUTANDO->registers.SI), buffer + offset, sizeof(uint32_t)); // RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(pcbEJECUTANDO->registers.DI), buffer + offset, sizeof(uint32_t)); // RECIBO CPUREG
    offset += sizeof(uint32_t);

    log_info(logger, "Motivo Recibido : %s", motivo);
    log_info(logger, "PID RECIBIDO : %i", pcbEJECUTANDO->pid);
    log_info(logger, "PC RECIBIDO : %i", pcbEJECUTANDO->program_counter);
    log_info(logger, "ESTADO PROCESO: %i", pcbEJECUTANDO->state);
    log_info(logger, "REGISTRO AX : %i", pcbEJECUTANDO->registers.AX);
    log_info(logger, "REGISTRO BX : %i", pcbEJECUTANDO->registers.BX);
    log_info(logger, "REGISTRO CX : %i", pcbEJECUTANDO->registers.CX);
    log_info(logger, "REGISTRO DX : %i", pcbEJECUTANDO->registers.DX);
    log_info(logger, "REGISTRO EAX : %i", pcbEJECUTANDO->registers.EAX);
    log_info(logger, "REGISTRO EBX : %i", pcbEJECUTANDO->registers.EBX);
    log_info(logger, "REGISTRO ECX : %i", pcbEJECUTANDO->registers.ECX);
    log_info(logger, "REGISTRO EDX : %i", pcbEJECUTANDO->registers.EDX);

    // aca le cambio el estado a exit
    pcbEJECUTANDO->state = EXIT;

    addEstadoExit(pcbEJECUTANDO);

    log_info(logger, "Finaliza el Proceso  %i, por SUCCESS", pcbEJECUTANDO->pid);
    // log_info(logger, "estado proceso %i",pcbEJECUTANDO->state);

    pcbEJECUTANDO = NULL;

    free(buffer);
    free(motivo);
}

// esta es la misma funcion fetch pcb actualizado pero esta no lo va a guardar en exit
// le va hacer un push a ready al final
// podria tener una funcion que sea fetch pcb que te reciba el pcb y lo devuelva
//
void fetch_pcb_actualizado_fin_quantum(int server_socket)
{
    int total_size;
    int offset = 0;
    t_pcb *PCBrec = pcbEJECUTANDO;
    void *buffer;
    int length_motivo;
    char *motivo;

    buffer = fetch_buffer(&total_size, server_socket);

    memcpy(&length_motivo, buffer + offset, sizeof(int));
    offset += sizeof(int);

    motivo = malloc(length_motivo);
    memcpy(motivo, buffer + offset, length_motivo); // SI TENGO QUE COPIAR EL LENGTH, NO TENGO QUE PONER SIZEOF(LENGTH)
    offset += length_motivo;                        // tengo que poner directamente el length en el ultimo param de memcpy
                             //  y lo mismo en el offset al sumarle, tengo que sumar lo que copie en memcpy

    offset += sizeof(int); // Salteo El tamaño del PCB
    // aca uso el puntero global que apunta al pcb actual: pcbEJECUTANDO
    // y actualizo ese pcb y despues lo pongo en NUll

    memcpy(&(pcbEJECUTANDO->pid), buffer + offset, sizeof(int)); // RECIBO EL PID
    offset += sizeof(int);

    memcpy(&(pcbEJECUTANDO->program_counter), buffer + offset, sizeof(int)); // RECIBO EL PROGRAM COUNTER
    offset += sizeof(int);

    memcpy(&(pcbEJECUTANDO->quantum), buffer + offset, sizeof(int)); // RECIBO EL QUANTUM
    offset += sizeof(int);

    memcpy(&(pcbEJECUTANDO->state), buffer + offset, sizeof(t_process_state)); // RECIBO EL PROCESS STATE
    offset += sizeof(t_process_state);

    memcpy(&(pcbEJECUTANDO->registers.PC), buffer + offset, sizeof(uint32_t)); // RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(pcbEJECUTANDO->registers.AX), buffer + offset, sizeof(uint8_t)); // RECIBO CPUREG
    offset += sizeof(uint8_t);
    memcpy(&(pcbEJECUTANDO->registers.BX), buffer + offset, sizeof(uint8_t)); // RECIBO CPUREG
    offset += sizeof(uint8_t);
    memcpy(&(pcbEJECUTANDO->registers.CX), buffer + offset, sizeof(uint8_t)); // RECIBO CPUREG
    offset += sizeof(uint8_t);
    memcpy(&(pcbEJECUTANDO->registers.DX), buffer + offset, sizeof(uint8_t)); // RECIBO CPUREG
    offset += sizeof(uint8_t);
    memcpy(&(pcbEJECUTANDO->registers.EAX), buffer + offset, sizeof(uint32_t)); // RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(pcbEJECUTANDO->registers.EBX), buffer + offset, sizeof(uint32_t)); // RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(pcbEJECUTANDO->registers.ECX), buffer + offset, sizeof(uint32_t)); // RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(pcbEJECUTANDO->registers.EDX), buffer + offset, sizeof(uint32_t)); // RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(pcbEJECUTANDO->registers.SI), buffer + offset, sizeof(uint32_t)); // RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(pcbEJECUTANDO->registers.DI), buffer + offset, sizeof(uint32_t)); // RECIBO CPUREG
    offset += sizeof(uint32_t);

    log_info(logger, "Motivo Recibido : %s", motivo);
    log_info(logger, "PID RECIBIDO : %i", pcbEJECUTANDO->pid);
    log_info(logger, "PC RECIBIDO : %i", pcbEJECUTANDO->program_counter);
    log_info(logger, "ESTADO PROCESO: %i", pcbEJECUTANDO->state);
    log_info(logger, "REGISTRO AX : %i", pcbEJECUTANDO->registers.AX);
    log_info(logger, "REGISTRO BX : %i", pcbEJECUTANDO->registers.BX);

    // aca le cambio el estado a REady porque fue desalojado por fin de quantum
    pcbEJECUTANDO->state = READY;

    addEstadoReady(pcbEJECUTANDO); // meto en ready el pcb
    sem_post(&sem_ready); //esto es para avisar que hay procesos en ready
    //porque el planificador de corto plazo tiene un semaforo para saber que por lo menos hay
    //algun proceso en ready, sino no sabe si hay y podrian pedirle que envie algo y no haya procesos
    //para enviar, por eso esto es un contador de cuantos procesos hay en ready

    // el puntero pcb global lo dejo en null
    // este no es el PID ejecutando, es el puntero al pcb que se envio
    pcbEJECUTANDO = NULL;

    free(buffer);
    free(motivo);
}

void fetch_pcb_actualizado_A_eliminar(int server_socket){
    
    int total_size;
    int offset = 0;
    t_pcb *PCBrec = pcbEJECUTANDO;
    void *buffer;
    int length_motivo;
    char *motivo;

    buffer = fetch_buffer(&total_size, server_socket);

    memcpy(&length_motivo, buffer + offset, sizeof(int));
    offset += sizeof(int);

    motivo = malloc(length_motivo);
    memcpy(motivo, buffer + offset, length_motivo); // SI TENGO QUE COPIAR EL LENGTH, NO TENGO QUE PONER SIZEOF(LENGTH)
    offset += length_motivo;                        // tengo que poner directamente el length en el ultimo param de memcpy
                             //  y lo mismo en el offset al sumarle, tengo que sumar lo que copie en memcpy

    offset += sizeof(int); // Salteo El tamaño del PCB
    // aca uso el puntero global que apunta al pcb actual: pcbEJECUTANDO
    // y actualizo ese pcb y despues lo pongo en NUll

    memcpy(&(pcbEJECUTANDO->pid), buffer + offset, sizeof(int)); // RECIBO EL PID
    offset += sizeof(int);

    memcpy(&(pcbEJECUTANDO->program_counter), buffer + offset, sizeof(int)); // RECIBO EL PROGRAM COUNTER
    offset += sizeof(int);

    memcpy(&(pcbEJECUTANDO->quantum), buffer + offset, sizeof(int)); // RECIBO EL QUANTUM
    offset += sizeof(int);

    memcpy(&(pcbEJECUTANDO->state), buffer + offset, sizeof(t_process_state)); // RECIBO EL PROCESS STATE
    offset += sizeof(t_process_state);

    memcpy(&(pcbEJECUTANDO->registers.PC), buffer + offset, sizeof(uint32_t)); // RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(pcbEJECUTANDO->registers.AX), buffer + offset, sizeof(uint8_t)); // RECIBO CPUREG
    offset += sizeof(uint8_t);
    memcpy(&(pcbEJECUTANDO->registers.BX), buffer + offset, sizeof(uint8_t)); // RECIBO CPUREG
    offset += sizeof(uint8_t);
    memcpy(&(pcbEJECUTANDO->registers.CX), buffer + offset, sizeof(uint8_t)); // RECIBO CPUREG
    offset += sizeof(uint8_t);
    memcpy(&(pcbEJECUTANDO->registers.DX), buffer + offset, sizeof(uint8_t)); // RECIBO CPUREG
    offset += sizeof(uint8_t);
    memcpy(&(pcbEJECUTANDO->registers.EAX), buffer + offset, sizeof(uint32_t)); // RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(pcbEJECUTANDO->registers.EBX), buffer + offset, sizeof(uint32_t)); // RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(pcbEJECUTANDO->registers.ECX), buffer + offset, sizeof(uint32_t)); // RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(pcbEJECUTANDO->registers.EDX), buffer + offset, sizeof(uint32_t)); // RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(pcbEJECUTANDO->registers.SI), buffer + offset, sizeof(uint32_t)); // RECIBO CPUREG
    offset += sizeof(uint32_t);
    memcpy(&(pcbEJECUTANDO->registers.DI), buffer + offset, sizeof(uint32_t)); // RECIBO CPUREG
    offset += sizeof(uint32_t);

    log_info(logger, "Motivo Recibido : %s", motivo);
    log_info(logger, "PID RECIBIDO : %i", pcbEJECUTANDO->pid);
    log_info(logger, "PC RECIBIDO : %i", pcbEJECUTANDO->program_counter);
    log_info(logger, "ESTADO PROCESO: %i", pcbEJECUTANDO->state);
    log_info(logger, "REGISTRO AX : %i", pcbEJECUTANDO->registers.AX);
    log_info(logger, "REGISTRO BX : %i", pcbEJECUTANDO->registers.BX);

    // aca le cambio el estado a exit
    pcbEJECUTANDO->state = EXIT;

    addEstadoExit(pcbEJECUTANDO); // meto en ready el pcb
    
    log_info(logger, "Finaliza el Proceso  %i, por SUCCESS", pcbEJECUTANDO->pid);

    // el puntero pcb global lo dejo en null
    // este no es el PID ejecutando, es el puntero al pcb que se envio
    pcbEJECUTANDO = NULL;


    free(buffer);
    free(motivo);
}


t_interfaz_registrada *recibir_interfaz(client_socket)
{
    t_interfaz_registrada *interfazNueva = malloc(sizeof(t_interfaz_registrada));

    int total_size;
    int offset = 0;

    void *buffer;

    int strlen_nombre;
    buffer = fetch_buffer(&total_size, client_socket);

    memcpy(&strlen_nombre, buffer + offset, sizeof(int)); // RECIBO EL TAMAÑO DEL NOMBRE
    offset += sizeof(int);

    interfazNueva->nombre = malloc(strlen_nombre);
    memcpy(interfazNueva->nombre, buffer + offset, strlen_nombre); // RECIBO EL NOMBRE
    // aca arriba tener CUIDADO con recibir strings, no tengo que poner
    //&(interfazNueva->nombre) porque es un char, ya es un puntero en si
    // el & lo pongo por ejemplo ints que no neceisto su valor sino su ubicacion
    // por eso aca va sin el &
    offset += strlen_nombre;

    memcpy(&strlen_nombre, buffer + offset, sizeof(int)); // RECIBO EL TAMAÑO DEL TIPO INTERFAZ
    offset += sizeof(int);

    interfazNueva->tipo = malloc(strlen_nombre);
    memcpy(interfazNueva->tipo, buffer + offset, strlen_nombre); // RECIBO EL TIPO DE INTERFAZ
    offset += strlen_nombre;

    interfazNueva->disponible = true;
    interfazNueva->socket_de_conexion = client_socket;

    interfazNueva->listaProcesosEsperando = queue_create();
    sem_init(&(interfazNueva->semaforoContadorIO), 0, 0);    //   EL CONTADOR LO INICIO EN CERO PORQUE RECINE LLEGO LA INTERFAZ
    pthread_mutex_init(&(interfazNueva->mutexColaIO), NULL); // EL MUTEX LO PONGO PARA CUANDO ACCEDAN A SU LISTA

    list_add(listaInterfaces, interfazNueva);

    printf("LLEGO UNA NUEVA INTERFAZ\n");
    printf("NOMBRE DE LA INTERFAZ: %s\n", interfazNueva->nombre);
    printf("TIPO DE INTERFAZ: %s\n", interfazNueva->tipo);

    free(buffer);
    return interfazNueva;
}

void detener_planificacion()
{
    // Se detiene la planificacion de largo y corto plazo (el proceso en EXEC continua hasta salir) (si se encuentrand detenidos ignorar)
    if (planificacion_detenida == true)
    {
        log_info(logger, "La planificacion ya se encuentra detenida");
    }
    else
    {
        pthread_mutex_lock(&m_planificador_largo_plazo);
        pthread_mutex_lock(&m_planificador_corto_plazo);
        planificacion_detenida = true;
        log_info(logger, "Pausa planificación: “PAUSA DE PLANIFICACIÓN“");
    }
}

void iniciar_planificacion()
{
    // Reanudar los planificadores (si no se encuentran detenidos ignorar)
    if (planificacion_detenida == false)
    {
        log_info(logger, "La planificacion ya se encuentra activa");
    }
    else
    {
        pthread_mutex_unlock(&m_planificador_largo_plazo);
        pthread_mutex_unlock(&m_planificador_corto_plazo);
        planificacion_detenida = false;
        log_info(logger, "Inicio de planificación: “INICIO DE PLANIFICACIÓN“");
    }
}
void finalizar_proceso(char *parametro)
{

    //detener_planificacion(); //detengo la planificacion para evitar que haya movimientos
    //mientras busco el proceso a eliminar

    int pidAeliminar = atoi(parametro);
    bool encontrado = false;
    t_pcb *punteroAEliminar = NULL;


    bool encontrar_por_pid(void *pcb) {
		t_pcb *pcb_n = (t_pcb*) pcb;

		if (pcb_n == NULL) {
			return false;
		}
		return pcb_n->pid == pidAeliminar;
	}

    // ACLARACION IMPORTANTE: T_QUEUE ES UNA ESTRUCTURA QUE DENTRO TIENE UN T_LIST QUE SE LLAMA
    // ELEMENTS, Y SOLO ESO. SOLAMENTE TIENE ESO Y POR ESO PUEDO PASAR ELEMENTS COMO PARAMETRO
    // Y USAR LAS FUNCIONES DE T_LIST

    // Me fijo si el proceso que esta corriendo en CPU es el que tengo que eliminar
    pthread_mutex_lock(&m_procesoEjectuandoActualmente);
    if (procesoEjectuandoActualmente == pidAeliminar)
    {
        // En caso que sea el que esta ejecutando en CPU mando el pid a eliminar
        pthread_mutex_unlock(&m_procesoEjectuandoActualmente);

        log_info(logger,"Es el que esta ejecutando actualmente");
        t_buffer *bufferEnvio;
        t_packet *eliminarProceso;

        bufferEnvio = create_buffer();
        eliminarProceso = create_packet(FINALIZAR_PROCESO, bufferEnvio);
        add_to_packet(eliminarProceso, &pidAeliminar, sizeof(int));
        //ACORDARSE QUE EL SEGUNDO PARAMETRO DEL ADDTOPACKET ES UN PUNTERO
        //ASI QUE LE PASO LA DIRECCION
        send_packet(eliminarProceso, cpu_interrupt_socket);
        destroy_packet(eliminarProceso);
    
    }else{ 

        // ACLARACION IMPORTANTE: T_QUEUE ES UNA ESTRUCTURA QUE DENTRO TIENE UN T_LIST QUE SE LLAMA
        // ELEMENTS, Y SOLO ESO. SOLAMENTE TIENE ESO Y POR ESO PUEDO PASAR ELEMENTS COMO PARAMETRO
        // Y USAR LAS FUNCIONES DE T_LIST

        // hago lo de abajo para el post al semaforo que esta afuera del if
        pthread_mutex_unlock(&m_procesoEjectuandoActualmente);

        //Busco en NEW
        pthread_mutex_lock(&mutex_state_new);
        punteroAEliminar = list_find(queue_new->elements,encontrar_por_pid);
        //aca voy a eliminar el PCB de la lista pero solo lo saco de la lista, no
        //le hago el destroy porqeu le tengo que hacer push en la cola de EXIT
        if(punteroAEliminar != NULL){
            //Lo Borro De NEW
            list_remove_element(queue_new->elements,punteroAEliminar);
            log_info(logger,"PID: %i encontrado en cola NEW",pidAeliminar);
            //Lo Agrego A
            addEstadoExit(punteroAEliminar);
            encontrado = true;
            //sem_wait(&sem_hay_pcb_esperando_ready);
            // Aca voy a comentar este wait por lo mismo que en el short term schuduerl
            // voy a hacer un control de que la lista dde ready no este vacia porque
            // se seguia colgando la consola 
        }
        pthread_mutex_unlock(&mutex_state_new);
            
            //en caso que no este en new voy a buscar en READY
            if(!encontrado){
                
                pthread_mutex_lock(&mutex_state_ready);
                
                punteroAEliminar = list_find(queue_ready->elements,encontrar_por_pid);

                if(punteroAEliminar != NULL){
                //Lo Borro De READY
                list_remove_element(queue_ready->elements,punteroAEliminar);
                log_info(logger,"PID: %i encontrado en cola READY",pidAeliminar);
                //Lo Agrego A
                addEstadoExit(punteroAEliminar);
                encontrado = true;

                //hago esto de multiprogramacion porque elimine un proceso de READY
                //y tengo que dejar entrar otro, no lo hago en NEW porque no tiene que haber uno ahi
                sem_post(&sem_multiprogramacion);
                //sem_wait(&sem_ready); //tambien le hago wait a esto porque
                //es un contador de cuantos hay en ready y acabo de sacar uno

                //ACA COMENTE ESTE WAIT SEM READY PORQUE AGREGUE EL IF EN SHORT TERM SCHEDUKER
                //ENTONCES NO IMPORTA SI PASA EL SEMREADY AUNQUE NO HAYA PROCESOS EN READY PORQUE
                // DESPUES CONTROLA QUE LA LISTA NO ESTE VACIA Y SI ESTA VACIA HACE UN return
                // Y ES COMO QUE LO VUELVE A TRABAR EN EL SEMAFORO SEMREADY ESPERANDO QUE ENTRE OTRO
                // ESTO ESTA MEJOR EXPLICADO EN EL VRR
                }
                pthread_mutex_unlock(&mutex_state_ready);

            }
            
            //Voy a Buscar en la cola de Prioridad del VRR
             if(!encontrado){
     
                pthread_mutex_lock(&mutex_state_prioridad);
     
                punteroAEliminar = list_find(queue_prioridad->elements,encontrar_por_pid);
                if(punteroAEliminar != NULL){
                //Lo Borro De PRIORIDAD
                list_remove_element(queue_prioridad->elements,punteroAEliminar);
                log_info(logger,"PID: %i encontrado en cola Prioridad",pidAeliminar);
                //Lo Agrego A
                addEstadoExit(punteroAEliminar);
                encontrado = true;
     
                //hago esto de multiprogramacion porque elimine un proceso de READY
                //y tengo que dejar entrar otro, no lo hago en NEW porque no tiene que haber uno ahi
                sem_post(&sem_multiprogramacion);

                //sem_wait(&sem_ready); //tambien le hago wait a esto porque
                //es un contador de cuantos hay en ready y acabo de sacar uno
                //ACA COMENTE ESTE WAIT SEM READY PORQUE AGREGUE EL IF EN SHORT TERM SCHEDUKER
                //ENTONCES NO IMPORTA SI PASA EL SEMREADY AUNQUE NO HAYA PROCESOS EN READY PORQUE
                // DESPUES CONTROLA QUE LA LISTA NO ESTE VACIA Y SI ESTA VACIA HACE UN return
                // Y ES COMO QUE LO VUELVE A TRABAR EN EL SEMAFORO SEMREADY ESPERANDO QUE ENTRE OTRO
                // ESTO ESTA MEJOR EXPLICADO EN EL VRR

                }  
                pthread_mutex_unlock(&mutex_state_prioridad);
                //este unlock estaba dentro del if, tiene que estar afuera para que se
                //desbloquee siempre. Lo encontre gracias al DEBUGGER

            }

            if(!encontrado){
                log_info(logger,"No se encontro el PID %i",pidAeliminar);
            }
    }

    //iniciar_planificacion();
}

void multiprogramacion(char *parametro){
    int nuevoValor = atoi(parametro);

    if(gradoMultiprogramacion == nuevoValor){
        log_info(logger,"El valor ingresado es igual al actual");
    }else if(nuevoValor > gradoMultiprogramacion){
        log_info(logger,"Valor Actual: %i Valor Ingresado: %i",gradoMultiprogramacion,nuevoValor);
        log_info(logger,"Expando el grado de Multiprogramacion en %i lugares",nuevoValor-gradoMultiprogramacion);
        for(int i = 0; i < (nuevoValor-gradoMultiprogramacion);i++){
            sem_post(&sem_multiprogramacion);
        }
        gradoMultiprogramacion = nuevoValor; //ACA CAMBIO REALMENTE EL GRADO DE MULTIPROGRAMACION
    }else{
        
        log_info(logger,"Valor Actual: %i Valor Ingresado: %i",gradoMultiprogramacion,nuevoValor);
        log_info(logger,"Reduzco el grado de Multiprogramacion en %i lugares",gradoMultiprogramacion-nuevoValor);
        salteoPostAlSemaforo = gradoMultiprogramacion-nuevoValor;
        gradoMultiprogramacion = nuevoValor; //ACA CAMBIO REALMENTE EL GRADO DE MULTIPROGRAMACION
        
       //NO HAGO LOS WAITS PORQUE CADA VEZ QUE AUMENTO EL GRADO DE MULTIPROGRAMACION ME FIJO QUE 
       //EL VALOR DEL SEMAFORO SEA MENOR AL GRADO DE MULTIPROGRAMACION
       //IMPORTANTE: CUANDO ELIMINAMOS UN PROCESO QUE ESTA EN LA COLA DE READY
       //AHI HAY UN SEMPOST AL GRADO DE MULTIPROGRAMACION Y AHI NO HACEMOS EL CHQUEOE
       //PORQUE SERIA UN CASO MUY RARO BORRAR Y JUSTO BAJAR EL GRADO DE MULTIRPOGRAMACION
       //PERO ACLARACION QUE PODRIA PASAR ALGO AHI IGUAL ES SOLO AGREGARLO
    }

}


//lo que se hace aca es que cuando tenemos que bajar el grado de multiprogamacion se le pone un valor a 
//la variable salteoPostAlsemaforo, ese valor es la cantidad de post que hay que evitar hacer 
//para que cambie el grado de multirpogramacion. Por ejemplo si tengo el grado en 3, y entran 3 procesos
// y tengo 4 esperando entrar a ready y en ese momento bajo el grao de multiprogramacion a 1, entonces
// cuando los 3 procesos lleguen a exit van a pasar por esta funcion, como el valor salteoPostAlSemaforo va 
// a ser 2 entonces van a entrar por el if y va  van a evitar hacer el post, y asi hsta que se haga 
// 0 el valor y va  a dar false el if y entonces va ir al else y ya va volver a hacer el post SIEMPRE
// de entrada esta varibale vale 0 porque tienen que hacer el post, solo toma un valor distinto
// de 0 cuando piden un grado de multiprogramacion menor al actual.

void controlGradoMultiprogramacion(){

    if(salteoPostAlSemaforo){
        log_info(logger,"Salteo un Post al Semaforo Grado Multiprogramacion");
        salteoPostAlSemaforo--;
        //PROBABLEMENTE ACA NECESITE UN MUTEX PARA EL SALTEOPOSTALSEMAFORO
    }else{
        sem_post(&sem_multiprogramacion);
    }
}

