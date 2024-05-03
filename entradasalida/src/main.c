#include "main.h"

int socket_kernel;
int socket_memoria ;

int main(int argc, char *argv[])
{   
    //ARGV ES UN VECTOR D DE TODAS LAS VARIABLES DE ENTORNO, EN LA POSICION 0 ESTA
    //LA LLAMADA AL ARCHIVO QUE EN ESTE CASO ES EL ./bin/entradasalida A PARTIR DEL 
    //1 YA ESTA LO QUE LE PASO, INT ARGC ES LA CANTIDAD DE VALORES QUE TIENE EL VECTOR
    //printf("PARAMETRO: %s\n",argv[0]);  //./bin/entradasalida
    printf("PARAMETRO: %s\n",argv[1]); // NOMBRE DE LA INTERFAZ
    printf("PARAMETRO: %s\n",argv[2]); // CONFIG AL QUE LO CONECTO
    
    //char *nombreInterfaz = argv[1];
   // char *configRecibido = argv[2];

    char *nombreInterfaz = "nombre1";
    char *configRecibido = "entradasalida.config";


    char *PORT_memoria;
    char *IP_memoria;
    t_buffer *buffer;
    t_packet *packet;
    char *PORT_kernel;
    char *IP_kernel;
    char *tipo;
    int operation_code;



    t_log *logger;
    logger = initialize_logger("entradasalida.log", "entradasalida", true, LOG_LEVEL_INFO);

    printf("Prueba desde una Interfaz de Entrada/Salida\n");
    log_info(logger, "Logger working");

    t_config *config = initialize_config(logger, configRecibido);

    tipo =  config_get_string_value(config, "TIPO_INTERFAZ");
    PORT_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    IP_memoria = config_get_string_value(config, "IP_MEMORIA");
    PORT_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    IP_kernel = config_get_string_value(config, "IP_KERNEL");

    //COMENTO TODA LA CONEXION A MEMORIA
    // Conect to server
    //socket_memoria = create_conection(logger, IP_memoria, PORT_memoria);
    //log_info(logger, "Conectado al servidor de memoria %s:%s", IP_memoria, PORT_memoria);

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

      while (1)
    {
        operation_code = fetch_codop(socket_kernel);
        switch (operation_code)
        {
        case KERNEL_A_IO:
            //sleepTime = fetch_packet(kernel_socket);
            //usleep(tiempo);
        break;

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
    //}




 

