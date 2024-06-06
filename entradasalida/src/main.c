#include "main.h"

int socket_kernel;
int socket_memoria ;
t_log *logger;

int main(int argc, char *argv[])
{   
    //ARGV ES UN VECTOR D DE TODAS LAS VARIABLES DE ENTORNO, EN LA POSICION 0 ESTA
    //LA LLAMADA AL ARCHIVO QUE EN ESTE CASO ES EL ./bin/entradasalida A PARTIR DEL 
    //1 YA ESTA LO QUE LE PASO, INT ARGC ES LA CANTIDAD DE VALORES QUE TIENE EL VECTOR
    //printf("PARAMETRO: %s\n",argv[0]);  //./bin/entradasalida
    printf("PARAMETRO: %s\n",argv[1]); // NOMBRE DE LA INTERFAZ
    printf("PARAMETRO: %s\n",argv[2]); // CONFIG AL QUE LO CONECTO
    
   char *nombreInterfaz = argv[1];
   char *configRecibido = argv[2];


    // ESTO ES PARA TENER LOS NOMBRE YA PUESTOS ACA Y NO ESTAR PASANDOLOS AL LLAMAR AL EXE
    //char *nombreInterfaz = "nombre1";
    //char *configRecibido = "entradasalida.config";

    


    char *PORT_memoria;
    char *IP_memoria;
    t_buffer *buffer;
    t_packet *packet;
    char *PORT_kernel;
    char *IP_kernel;
    char *tipo;
    int operation_code;



    logger = initialize_logger("entradasalida.log", "entradasalida", true, LOG_LEVEL_INFO);

    printf("Prueba desde una Interfaz de Entrada/Salida\n");
    log_info(logger, "Logger working");

    t_config *config = initialize_config(logger, configRecibido);

    tipo =  config_get_string_value(config, "TIPO_INTERFAZ");
    PORT_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    IP_memoria = config_get_string_value(config, "IP_MEMORIA");
    PORT_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    IP_kernel = config_get_string_value(config, "IP_KERNEL");

    
    socket_memoria = create_conection(logger, IP_memoria, PORT_memoria);
    log_info(logger, "Conectado al servidor de memoria %s:%s", IP_memoria, PORT_memoria);

    // Send handshake
   
    //packet = serialize_packet(packet, buffer->size);
    //send_packet(packet, socket_memoria);

    //log_info(logger, "Handshake enviado");
    
    
    //DEPENDIENDO DEL TIPO DE INTERFAZ SE CONECTA A KERNEL O A MEMORIA

    //if(strcmp(tipo,"GENERICA")==0){
    //ARMO PAQUETE PARA CONEXION CON KERNEL     
    buffer = create_buffer();
    packet = create_packet(NUEVA_INTERFAZ, buffer);
    add_to_packet(packet,nombreInterfaz,strlen(nombreInterfaz)+1);
    add_to_packet(packet,tipo,strlen(tipo)+1);

    //INICIO CONEXION CON KERNEL
    socket_kernel = create_conection(logger, IP_kernel, PORT_kernel);
    log_info(logger, "Conectado al servidor de Kernel %s:%s", IP_kernel, PORT_kernel);
    send_packet(packet, socket_kernel);
    destroy_packet(packet);
    
    
    while (1)
    {
        operation_code = fetch_codop(socket_kernel);
        switch (operation_code)
        {
        case TIEMPO_DORMIR:
            int tiempo = fetch_tiempoDormir(socket_kernel);
            log_info(logger, "RECIBI UN SLEEP DE %i",tiempo);
            usleep(tiempo); //falta hacer el calculo bien
            log_info(logger, "TERMINE UN SLEEP DE %i",tiempo);
            enviarAvisoAKernel(socket_kernel);
            //aca antes pasaba que me decia algun error inesperado, no se porque
            //lo debugee y empezo a funcionar, pero pasaba que se iba por el default
        break;
        case TAMANO_DIRECCION_READ:
            int tamano = fetch_tamano(socket_kernel);
            int direccion = fetch_direccion(socket_kernel);
            char* inputs = leer_desde_stdin(tamano);
            int tamInputs = strlen(tipo)+1;
            int cantBits = fetch_cantBits(socket_kernel);
            int pid = fetch_pid(socket_kernel);

            t_buffer* buffer2 = create_buffer();
            t_packet* packet2 = create_packet(STDIN_READ, buffer2);

            add_to_packet(packet2,direccion,sizeof(int));
            add_to_packet(packet2,tamInputs,strlen(tamInput)+1);
            add_to_packet(packet2,inputs,tamInputs);
            add_to_packet(packet2,cantBits,sizeof(int));
            add_to_packet(packet2,pid,sizeof(int));

            send_packet(packet2, socket_memoria);
            destroy_packet(packet2);

            int operation_code = fetch_codop(socket_memoria);

            if(operation_code == CONFIRMACION_ESCRITURA_STDIN){
                
                int hoa = 1;
                int total_size;
                int offset = 0;

                t_buffer *buffer3 = create_buffer();
                t_packet *packet3 = create_packet(CONFIRMACION_STDIN_READ, buffer3)

                send_packet(packet3, socket_kernel);
                destroy_packet(packet3);
            }else{
                int total_size;
                void *buffer2 = fetch_buffer(&total_size, socket_memoria);
                free(buffer2);
                log_info(logger,"Error en la lectura");
            }

        case TAMANO_DIRECCION_WRITE:
            int tamano = fetch_tamano(socket_kernel);
            int direccion = fetch_direccion(socket_kernel);
            int pid = fetch_pid(socket_kernel);

            t_buffer *buffer2 = create_buffer();
            t_packet* packet2 = create_packet(STDOUT_WRITE, buffer2);

            add_to_packet(packet2,tamano,sizeof(int));
            add_to_packet(packet2,direccion,sizeof(int));
            add_to_packet(packet2,pid,sizeof(int));

            send_packet(packet2, socket_memoria);
            destroy_packet(packet2);

            
            int operation_code = fetch_codop(socket_memoria);

            if(operation_code == CONFIRMACION_LECTURA_STDOUT){
                
                char* contenido;
                int total_size;
                int offset = 0;

                t_buffer *buffer3 = fetch_buffer(&total_size, socket_memoria);
                buffer3 = fetch_buffer(&total_size, socket_memoria);

                memcpy(&contenido,buffer2 + offset, strlen(contenido) + 1); 
                offset += strlen(contenido) + 1; 

                printf("Contenido leido desde memoria: %s\n", contenido);

                free(buffer3);
                log_info(logger,"Confirmacion Lectura");

                t_buffer *buffer4 = create_buffer();
                t_packet *packet3 = create_packet(CONFIRMACION_STDOUT_WRITE, buffer4);

                add_to_packet(packet3,contenido,strlen(contenido) + 1);

                send_packet(packet3, socket_kernel);
                destroy_packet(packet3);
            }else{
                int total_size;
                void *buffer2 = fetch_buffer(&total_size, socket_memoria);
                free(buffer2);
                log_info(logger,"Error en la lectura");
            }




        case -1:
            log_error(logger, "Error al recibir el codigo de operacion");
            close_conection(socket_kernel);

            return;

        default:
            log_error(logger, "Algun error inesperado ");
            close_conection(socket_kernel);
            return;
        }
    }
}



int fetch_tiempoDormir(int socket_kernel){

    int total_size;
    int offset = 0;

    int tiempoSleep;
   
    void *buffer2;
    buffer2 = fetch_buffer(&total_size, socket_kernel);

    offset += sizeof(int);//ME SALTEO EL TAMAÑO DEL INT;
    
    memcpy(&tiempoSleep,buffer2 + offset, sizeof(int)); //RECIBO EL TAMAÑO
    

    free(buffer2);
    return tiempoSleep;

}
void enviarAvisoAKernel(int socket_kernel){
    t_buffer *bufferRespuesta;
    t_packet *packetRespuesta;

    int hola = 1; //ENVIO ALGO PARA NO ENVIAR EL BUFFER VACIO  AVERIGUAR SI SE PUEDE ENVIAR VACIO
    bufferRespuesta = create_buffer();
    packetRespuesta = create_packet(CONFIRMACION_SLEEP_COMPLETO, bufferRespuesta);
    add_to_packet(packetRespuesta,&hola, sizeof(int));
    send_packet(packetRespuesta,socket_kernel);   
    destroy_packet(packetRespuesta);

    //destroy(packetRespuesta); hay que incluir esta funcion destroy
}

int fetch_tamano(socket_kernel) {
    int total_size;
    int offset = 0;

    int tamano;
   
    void *buffer2;
    buffer2 = fetch_buffer(&total_size, socket_kernel);

    offset += sizeof(int); //No entiendo como esta salta al tamano de nombre interfaz
    offset += sizeof(int); //Salta direccion

    memcpy(&tamano,buffer2 + offset, sizeof(int)); 
    
    free(buffer2);
    return tamano;
}

int fetch_direccion(socket_kernel) {
    int total_size;
    int offset = 0;

    int direccion;
   
    void *buffer2;
    buffer2 = fetch_buffer(&total_size, socket_kernel);

    offset += sizeof(int);
    
    memcpy(&direccion,buffer2 + offset, sizeof(int)); 
    
    free(buffer2);
    return direccion;
}

int fetch_cantBits(socket_kernel) {
    int total_size;
    int offset = 0;

    int cantBits;
   
    void *buffer2;
    buffer2 = fetch_buffer(&total_size, socket_kernel);

    offset += sizeof(int);
    offset += sizeof(int);
    offset += sizeof(int);

    memcpy(&cantBits,buffer2 + offset, sizeof(int)); 
    
    free(buffer2);
    return cantBits;
}

int fetch_pid(socket_kernel) {
    int total_size;
    int offset = 0;

    int pid;
   
    void *buffer2;
    buffer2 = fetch_buffer(&total_size, socket_kernel);

    offset += sizeof(int);
    offset += sizeof(int);
    offset += sizeof(int);
    offset += sizeof(int);

    memcpy(&proceso,buffer2 + offset, sizeof(int)); 
    
    free(buffer2);
    return proceso;
}

char* leer_desde_stdin(int tamano) {
    char* input = malloc((tamano + 1) * sizeof(char));
    if (input == NULL) {
        return NULL;
    }
    fgets(input, tamano + 1, stdin);

    input[strcspn(input, "\n")] = '\0';

    return input;
}