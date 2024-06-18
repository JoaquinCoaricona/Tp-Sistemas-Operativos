#include "main.h"

int socket_kernel;
int socket_memoria ;
t_log *logger;
char *PORT_memoria;
char *IP_memoria;
char *PORT_kernel;
char *IP_kernel;
int tiempoUnidad;
char* path_base;
int block_size;
int block_count;
int retraso_compactacion;
t_dictionary* fcb_dictionary;

int main(int argc, char *argv[])
{   
    //ARGV ES UN VECTOR D DE TODAS LAS VARIABLES DE ENTORNO, EN LA POSICION 0 ESTA
    //LA LLAMADA AL ARCHIVO QUE EN ESTE CASO ES EL ./bin/entradasalida A PARTIR DEL 
    //1 YA ESTA LO QUE LE PASO, INT ARGC ES LA CANTIDAD DE VALORES QUE TIENE EL VECTOR
    //printf("PARAMETRO: %s\n",argv[0]);  //./bin/entradasalida
    // printf("PARAMETRO: %s\n",argv[1]); // NOMBRE DE LA INTERFAZ
    // printf("PARAMETRO: %s\n",argv[2]); // CONFIG AL QUE LO CONECTO
    
    char *nombreInterfaz = argv[1];
    char *configRecibido = argv[2];


    //  char *nombreInterfaz = "nombre2";
    //  char *configRecibido = "pantalla.config";

    // ESTO ES PARA TENER LOS NOMBRE YA PUESTOS ACA Y NO ESTAR PASANDOLOS AL LLAMAR AL EXE
    //char *nombreInterfaz = "nombre1";
    //char *configRecibido = "entradasalida.config";
    t_buffer *buffer;
    t_packet *packet;
    char *tipo;

    logger = initialize_logger("entradasalida.log", "entradasalida", true, LOG_LEVEL_INFO);

    t_config *config = initialize_config(logger, configRecibido);

    tipo =  config_get_string_value(config, "TIPO_INTERFAZ");
    PORT_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    IP_memoria = config_get_string_value(config, "IP_MEMORIA");
    PORT_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    IP_kernel = config_get_string_value(config, "IP_KERNEL");
    tiempoUnidad = atoi(config_get_string_value(config, "TIEMPO_UNIDAD_TRABAJO"));
    path_base = config_get_string_value(config, "PATH_BASE_DIALFS");
    block_size = atoi(config_get_string_value(config, "BLOCK_SIZE"));
    block_count = atoi(config_get_string_value(config, "BLOCK_COUNT"));
    retraso_compactacion = atoi(config_get_string_value(config, "RETRASO_COMPACTACION"));

    fcb_dictionary = dictionary_create();

    //ARMO PAQUETE PARA CONEXION CON KERNEL     
    buffer = create_buffer();
    packet = create_packet(NUEVA_INTERFAZ, buffer);
    add_to_packet(packet,nombreInterfaz,strlen(nombreInterfaz)+1);
    add_to_packet(packet,tipo,strlen(tipo)+1);

    //INICIO CONEXION CON KERNEL
    socket_kernel = create_conection(logger, IP_kernel, PORT_kernel);
    log_info(logger, "Conectado al servidor de Kernel %s:%s", IP_kernel, PORT_kernel);

    //Envio paquete a kernel
    send_packet(packet, socket_kernel);
    destroy_packet(packet);
    if(string_equals_ignore_case(tipo,"GENERICA")){
        interfazGenerica();
    }else if(string_equals_ignore_case(tipo,"STDIN")){
        interfazStdin();
    }else if(string_equals_ignore_case(tipo,"STDOUT")){
        interfazStdout();
    }else if(string_equals_ignore_case(tipo,"DIALFS")){
        interfazDialFS();
    }
    else{
        log_info(logger,"Error");
    }
    
    
}



void interfazGenerica(){

   while (1)
    {
        int operation_code = fetch_codop(socket_kernel);
        switch (operation_code)
        {
        case TIEMPO_DORMIR:
            int tiempo = fetch_tiempoDormir(socket_kernel);
            log_info(logger, "RECIBI UN SLEEP DE %i",tiempo);
            usleep(tiempoUnidad * tiempo); 
            log_info(logger, "TERMINE UN SLEEP DE %i",tiempo);
            enviarAvisoAKernel(socket_kernel,CONFIRMACION_SLEEP_COMPLETO);
            //aca antes pasaba que me decia algun error inesperado, no se porque
            //lo debugee y empezo a funcionar, pero pasaba que se iba por el default
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
void enviarAvisoAKernel(int socket_kernel,op_code codigo){
    t_buffer *bufferRespuesta;
    t_packet *packetRespuesta;

    int hola = 1; //ENVIO ALGO PARA NO ENVIAR EL BUFFER VACIO  AVERIGUAR SI SE PUEDE ENVIAR VACIO
    bufferRespuesta = create_buffer();
    packetRespuesta = create_packet(codigo, bufferRespuesta);
    add_to_packet(packetRespuesta,&hola, sizeof(int));
    send_packet(packetRespuesta,socket_kernel);   
    destroy_packet(packetRespuesta);

    //destroy(packetRespuesta); hay que incluir esta funcion destroy
}

void interfazStdout(){
    
    // Conexion a Memoria
    //Hago el handshake con memoria porque en este tipo de interfaz tengo que hacerlo
    socket_memoria = create_conection(logger, IP_memoria, PORT_memoria);
    log_info(logger, "Conectado al servidor de memoria %s:%s", IP_memoria, PORT_memoria);
    t_buffer *buffer_handshake = create_buffer();
    t_packet *packet_handshake = create_packet(HANDSHAKE_ENTRADA_SALIDA, buffer_handshake);
    add_to_packet(packet_handshake, buffer_handshake->stream, buffer_handshake->size);
    send_packet(packet_handshake,socket_memoria); 
    log_info(logger, "Handshake enviado");   
    destroy_packet(packet_handshake);
    //------------------------------------------------------------------------------
    //LOOP INFINITO DE ESCUCHA A KERNEL
    while (1)
    {   
        int operation_code = fetch_codop(socket_kernel);
        switch (operation_code)
        {
        case STDOUT_ESCRIBIR:
            recibirYejecutarDireccionesFisicas(socket_kernel);
            enviarAvisoAKernel(socket_kernel,CONFIRMACION_STDOUT);
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

void recibirYejecutarDireccionesFisicas(int socket_kernel){
    int total_size;
    int offset = 0;
    int pid;
    int cantidadDireccionesFisicas;
    void *buffer2;
    int dirFisica;
    int cantidadBytesLeer;
    buffer2 = fetch_buffer(&total_size, socket_kernel);
    void *contenido;
    int cantidadBytesMalloc;
    int desplazamientoParteLeida = 0;

    offset += sizeof(int);//ME SALTEO EL TAMAÑO DEL INT;

    memcpy(&pid,buffer2 + offset, sizeof(int)); //RECIBO EL PID
    offset += sizeof(int);
    
    offset += sizeof(int);//ME SALTEO EL TAMAÑO DEL INT;

    memcpy(&cantidadBytesMalloc,buffer2 + offset, sizeof(int)); //RECIBO LA CANTIDAD DE BYTES MALLOC
    offset += sizeof(int);

    log_info(logger,"Cantidad Bytes Malloc %i",cantidadBytesMalloc);
    contenido = malloc(cantidadBytesMalloc + 1); //sumo uno para poner el /0 despues al final
    offset += sizeof(int);//ME SALTEO EL TAMAÑO DEL INT QUE INDICA EL TAMAÑO DEL VOID* CONTENIDO
    offset += sizeof(int);//ME SALTEO EL TAMAÑO DEL INT DIR FISICAS;

    memcpy(&cantidadDireccionesFisicas,buffer2 + offset, sizeof(int)); //Recibo cantidad dir Fisicas
    offset += sizeof(int);
    log_info(logger,"Cantidad Direcciones fisicas %i",cantidadDireccionesFisicas);


    for(int i = 0; i < cantidadDireccionesFisicas; i++){
        offset += sizeof(int); //Aca salteo el tamaño del int cantidadBytesLee
        memcpy(&cantidadBytesLeer,buffer2 + offset, sizeof(int)); 
        offset += sizeof(int);
        offset += sizeof(int);//Aca salteo el tamaño del int dirfisica
        memcpy(&dirFisica,buffer2 + offset, sizeof(int)); 
        offset += sizeof(int);
        mandarALeer(dirFisica,cantidadBytesLeer,pid,contenido + desplazamientoParteLeida);
        desplazamientoParteLeida = desplazamientoParteLeida + cantidadBytesLeer;
    }

    char *cadena = (char *)contenido;

    cadena[cantidadBytesMalloc] = '\0'; // Asegúrate de que el string esté terminado en '\0'

    log_info(logger,"%s",cadena);

    free(buffer2);
    
}
//ESTA FUNCION NO RECIBE NADA Y ESTABA RECIBIENDO ACA COMO PRAMEtro al socket kernel
// ESO ES GLOBAL Y NO TENIA PORQUE RECIBIRILO, Y ES COMO QUE ESCRIBIA ALGO EN EN ESA VARIABLES
//PORQUE SE SALTEABA EL FETchccodop
void interfazStdin(){
    // Conexion a Memoria
    //Hago el handshake con memoria porque en este tipo de interfaz tengo que hacerlo
    socket_memoria = create_conection(logger, IP_memoria, PORT_memoria);
    log_info(logger, "Conectado al servidor de memoria %s:%s", IP_memoria, PORT_memoria);
    t_buffer *buffer_handshake = create_buffer();
    t_packet *packet_handshake = create_packet(HANDSHAKE_ENTRADA_SALIDA, buffer_handshake);
    add_to_packet(packet_handshake, buffer_handshake->stream, buffer_handshake->size);
    send_packet(packet_handshake,socket_memoria); 
    log_info(logger, "Handshake enviado");   
    destroy_packet(packet_handshake);
    //------------------------------------------------------------------------------
    //LOOP INFINITO DE ESCUCHA A KERNEL
    while (1)
    {

        int operation_code = fetch_codop(socket_kernel);
        switch (operation_code)
        {
        case STDIN_LEER:
            recibirYejecutarDireccionesFisicasSTDIN(socket_kernel);
            enviarAvisoAKernel(socket_kernel,CONFIRMACION_STDIN);
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

void recibirYejecutarDireccionesFisicasSTDIN(int socket_kernel){
    //char *cadenaPrueba = "hola como estas";
    log_info(logger,"Ingrese cadena a escribir. No se va escribir todo (solo lo indicado en la instruccion)");
    char *cadenaPrueba = readline(">");
    
    int limiteAEscribir;

    int total_size;
    int offset = 0;
    int pid;
    int cantidadDireccionesFisicas;
    void *buffer2;
    int dirFisica;
    int cantidadBytesEscribir;

    buffer2 = fetch_buffer(&total_size, socket_kernel);
    void *contenido;
    int cantidadBytesMalloc;
    int parteEscrita = 0;

    offset += sizeof(int);//ME SALTEO EL TAMAÑO DEL INT;

    memcpy(&pid,buffer2 + offset, sizeof(int)); //RECIBO EL PID
    offset += sizeof(int);

    int observador;
    memcpy(&observador,buffer2 + offset, sizeof(int)); 


    offset += sizeof(int); //Salteo el tamaño del INT void
    memcpy(&observador,buffer2 + offset, sizeof(int)); 

    offset += sizeof(int); //Salteo el tamaño del int cantBytes que esta dentro del void

    memcpy(&limiteAEscribir,buffer2 + offset, sizeof(int)); //Recibo el tamaño de bytes
    //a copiar que mandaron en la instruccion
    offset += sizeof(int);

    offset += sizeof(int);//salteo el tamaño del int
    memcpy(&cantidadDireccionesFisicas,buffer2 + offset, sizeof(int)); //Recibo cantidad dir Fisicas
    offset += sizeof(int);

    while((cantidadDireccionesFisicas > 0) && (parteEscrita < limiteAEscribir)){
        offset += sizeof(int);//Salteo el tamaño del int
        memcpy(&cantidadBytesEscribir,buffer2 + offset, sizeof(int)); 
        offset += sizeof(int);
        offset += sizeof(int);//Salteo el tamaño del int
        memcpy(&dirFisica,buffer2 + offset, sizeof(int)); 
        offset += sizeof(int);        
        mandarAescribirEnMemoria(dirFisica,cadenaPrueba + parteEscrita,cantidadBytesEscribir,pid);
        
        //Log para controlar que escribi bien
        char *contenidoEscrito = malloc(cantidadBytesEscribir + 1);
        contenidoEscrito[cantidadBytesEscribir] = '\0'; // Asegúrate de que el string esté terminado en '\0'
        memcpy(contenidoEscrito,cadenaPrueba + parteEscrita, cantidadBytesEscribir); 
        log_info(logger,"%s",contenidoEscrito);
        free(contenidoEscrito);

        parteEscrita = parteEscrita + cantidadBytesEscribir;
    }

    free(buffer2);
    free(cadenaPrueba);

}

void mandarALeer(int dirFisica, int cantidadBits, int pid, void *contenido){

	t_buffer *bufferLectura;
    t_packet *packetLectura;
    bufferLectura = create_buffer();
    packetLectura = create_packet(SOLICITUD_LECTURA, bufferLectura);

    add_to_packet(packetLectura,&dirFisica,sizeof(int));
    add_to_packet(packetLectura,&cantidadBits,sizeof(int));
	add_to_packet(packetLectura,&pid,sizeof(int));
    
    send_packet(packetLectura, socket_memoria);
    destroy_packet(packetLectura);

	//-------------Aca se bloquea esperando el codop-------
	int operation_code = fetch_codop(socket_memoria);

	if(operation_code == CONFIRMACION_LECTURA){
		int total_size;
		int offset = 0;
		void *buffer2 = fetch_buffer(&total_size, socket_memoria);

		offset += sizeof(int); //Salteo el tamaño del INT

		memcpy(contenido,buffer2 + offset,cantidadBits); 
    	offset += cantidadBits; //Este offset no tiene sentido pero lo pongo por las dudas
		//Esto iria pero como son numeros quizas justo leo la mitad y no tendria sentido imprimir la mitad
		//log_info(logger,"Valor: %i", contenido);

		free(buffer2);
		log_info(logger,"Confirmacion Lectura");
	}else{
		int total_size;
		void *buffer2 = fetch_buffer(&total_size, socket_memoria);
		free(buffer2);
		log_info(logger,"Error en la lectura");
	}
}

void mandarAescribirEnMemoria(int dirFisica,void *contenidoAescribir, int cantidadBits,int pid){
	t_buffer *bufferEscritura;
    t_packet *packetEscritura;
    bufferEscritura = create_buffer();
    packetEscritura = create_packet(SOLICITUD_ESCRIBIR, bufferEscritura);

    add_to_packet(packetEscritura,&dirFisica,sizeof(int));
    add_to_packet(packetEscritura,&cantidadBits,sizeof(int));
    add_to_packet(packetEscritura,contenidoAescribir,cantidadBits);
	add_to_packet(packetEscritura,&pid,sizeof(int));
    
    send_packet(packetEscritura, socket_memoria);
    destroy_packet(packetEscritura);

	//-------------Aca se bloquea esperando el codop-------
	int operation_code = fetch_codop(socket_memoria);
	if(operation_code == CONFIRMACION_ESCRITURA){
		int total_size;
		void *buffer2 = fetch_buffer(&total_size, socket_memoria);
		free(buffer2);
		log_info(logger,"Confirmacion Escritura");
	}else{
		int total_size;
		void *buffer2 = fetch_buffer(&total_size, socket_memoria);
		free(buffer2);
		log_info(logger,"Error en la escritura");
	}
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}

void interfazDialFS(){
    socket_memoria = create_conection(logger, IP_memoria, PORT_memoria);
    log_info(logger, "Conectado al servidor de memoria %s:%s", IP_memoria, PORT_memoria);
    t_buffer *buffer_handshake = create_buffer();
    t_packet *packet_handshake = create_packet(HANDSHAKE_ENTRADA_SALIDA, buffer_handshake);
    add_to_packet(packet_handshake, buffer_handshake->stream, buffer_handshake->size);
    send_packet(packet_handshake,socket_memoria); 
    log_info(logger, "Handshake enviado");   
    destroy_packet(packet_handshake);

    while (1)
    {   
        int operation_code = fetch_codop(socket_kernel);
        switch (operation_code)
        {
        case DIALFS_CREATE:
            usleep(tiempoUnidad);
            fetch_nombre_archivo_y_crear_archivo(socket_kernel);
            enviarAvisoAKernel(socket_kernel,CONFIRMACION_CREATE_ARCHIVO);
            break;
        case DIALFS_DELETE:
            usleep(tiempoUnidad);
            fetch_nombre_archivo_y_delete_archivo(socket_kernel);
            enviarAvisoAKernel(socket_kernel,CONFIRMACION_DELETE_ARCHIVO);
            break;
        case DIALFS_TRUNCATE:
            usleep(tiempoUnidad);

            break;
        case DIALFS_WRITE:
            usleep(tiempoUnidad);

            break;
        case DIALFS_READ:
            usleep(tiempoUnidad);
            
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

t_fcb* initialize_fcb(char* nombre_archivo, char* path) {
    t_config* config = malloc(sizeof(t_config));

    config->path = strdup(path);
    config->properties = dictionary_create();

    int tamanio = block_size * block_count;

    config_set_value(config, "NOMBRE", string_duplicate(nombre_archivo));
    config_set_value(config, "TAMANIO", string_itoa(tamanio));
    config_set_value(config, "BLOQUE_INICIAL", "");
    config_save_in_file(config, path);
	t_fcb* fcb = malloc(sizeof(t_fcb));

    fcb->nombre = string_duplicate(config_get_string_value(config, "NOMBRE"));
	fcb->tamanio = config_get_int_value(config, "TAMANIO");
	fcb->bloque_inicial = config_get_int_value(config, "BLOQUE_INICIAL");

	config_destroy(config);
	return fcb;

}

void fetch_nombre_archivo_y_crear_archivo(int socket_kernel){

    int total_size;
    int offset = 0;

    int pid;
    char* nombre_archivo;
    int length_nombre_archivo;

    void *buffer2;
    buffer2 = fetch_buffer(&total_size, socket_kernel);

    offset += sizeof(int);
    memcpy(&pid,buffer2 + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&length_nombre_archivo,buffer2 + offset, sizeof(int));
    offset += sizeof(int); 
    *nombre_archivo = malloc(length_nombre_archivo);
    memcpy(&nombre_archivo,buffer2 + offset, length_nombre_archivo);
    offset += length_nombre_archivo;

    free(buffer2);
    
    log_info(logger, "PID: %i - Crear Archivo: %s", pid, nombre_archivo);

    char* direccion_fcb = string_new();
    string_append(&direccion_fcb, path_base);
	string_append(&direccion_fcb, "/");
	string_append(&direccion_fcb, nombre_archivo);

	t_fcb* fcb = malloc(sizeof(t_fcb));
	fcb = initialize_fcb(nombre_archivo, direccion_fcb);

	dictionary_put(fcb_dictionary, nombre_archivo, fcb);

    free(direccion_fcb);
    free(fcb);
}

void fetch_nombre_archivo_y_delete_archivo(int socket_kernel){

    int total_size;
    int offset = 0;

    int pid;
    char* nombre_archivo;
    int length_nombre_archivo;
   
    void *buffer2;
    buffer2 = fetch_buffer(&total_size, socket_kernel);

    offset += sizeof(int);
    memcpy(&pid,buffer2 + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&length_nombre_archivo,buffer2 + offset, sizeof(int));
    offset += sizeof(int); 
    *nombre_archivo = malloc(length_nombre_archivo);
    memcpy(&nombre_archivo,buffer2 + offset, length_nombre_archivo);
    offset += length_nombre_archivo;

    free(buffer2);

    log_info(logger, "PID: %i - Eliminar Archivo: %s", pid, nombre_archivo);

    char* direccion_fcb = string_new();
    string_append(&direccion_fcb, path_base);
	string_append(&direccion_fcb, "/");
	string_append(&direccion_fcb, nombre_archivo);

    t_fcb* fcb = malloc(sizeof(t_fcb));
    fcb = dictionary_remove(fcb_dictionary, nombre_archivo);

    if (fcb == NULL) {
        log_error(logger, "Archivo %s no existe", nombre_archivo);
        free(direccion_fcb);
        free(fcb);
        return;
    }

    if (remove(direccion_fcb) == 0) {
        log_info(logger, "Archivo %s eliminado", nombre_archivo);
    } else {
        log_error(logger, "Error eliminando el archivo %s", nombre_archivo);
    }

    free(direccion_fcb);
    free(fcb);
}
