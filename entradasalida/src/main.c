#include "main.h"

int socket_kernel;
int socket_memoria ;
t_log *logger;
char *PORT_memoria;
char *IP_memoria;
char *PORT_kernel;
char *IP_kernel;
int tiempoUnidad;
t_config *config;

int block_size;
int block_count;
char* path_base_dialfs;
int retraso_compactacion;

char* path_archivo_comun;
char* path_bitmap;
char* path_bloques;

int bloques_libres;
void *bloques_map;
char* bitmap_map;
t_bitarray *bitarray;

t_list *lista_archivos;

char* nombre_archivo_a_buscar;

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

    // ESTO ES PARA TENER LOS NOMBRE YA PUESTOS ACA Y NO ESTAR PASANDOLOS AL LLAMAR AL EXE
    //char *nombreInterfaz = "nombre1";
    //char *configRecibido = "entradasalida.config";
    t_buffer *buffer;
    t_packet *packet;
    char *tipo;

    logger = initialize_logger("entradasalida.log", "entradasalida", true, LOG_LEVEL_INFO);

    config = initialize_config(logger, configRecibido);

    tipo =  config_get_string_value(config, "TIPO_INTERFAZ");
    PORT_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    IP_memoria = config_get_string_value(config, "IP_MEMORIA");
    PORT_kernel = config_get_string_value(config, "PUERTO_KERNEL");
    IP_kernel = config_get_string_value(config, "IP_KERNEL");
    tiempoUnidad = atoi(config_get_string_value(config, "TIEMPO_UNIDAD_TRABAJO"));
   
    //ARMO PAQUETE PARA CONEXION CON KERNEL     
    buffer = create_buffer();
    packet = create_packet(NUEVA_INTERFAZ, buffer);
    add_to_packet(packet,nombreInterfaz,strlen(nombreInterfaz)+1);
    add_to_packet(packet,tipo,strlen(tipo)+1);

    //INICIO CONEXION CON KERNEL
    //Aca este socket kernel es especifico para cada interfaz, por cada instancia que levantamos
    //al conectarse, se conecta con un socket diferente y la variable global es diferente para
    //cada interfaz o instancia que tengo
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
    }else{
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

    //Conexion a memoria 
    socket_memoria = create_conection(logger, IP_memoria, PORT_memoria);
    log_info(logger, "Conectado al servidor de memoria %s:%s", IP_memoria, PORT_memoria);

    t_buffer *buffer_handshake = create_buffer();
    t_packet *packet_handshake = create_packet(HANDSHAKE_ENTRADA_SALIDA, buffer_handshake);

    add_to_packet(packet_handshake, buffer_handshake->stream, buffer_handshake->size);

    send_packet(packet_handshake,socket_memoria); 

    log_info(logger, "Handshake enviado");   

    destroy_packet(packet_handshake);

    //Guardar los balores de Config a su variable respectables
    block_size = atoi(config_get_string_value(config, "BLOCK_SIZE"));
    block_count = atoi(config_get_string_value(config, "BLOCK_COUNT"));
    path_base_dialfs = config_get_string_value(config, "PATH_BASE_DIALFS");
    retraso_compactacion = atoi(config_get_string_value(config, "RETRASO_COMPACTACION"));

    //Path para cada tipo (Archivos normal, Bitmap, Bloques) de Archivos 
    path_archivo_comun;
    asprintf(&path_archivo_comun, "%s/", path_base_dialfs);

    path_bloques;
    asprintf(&path_bloques, "%s/bloques.dat", path_base_dialfs);

    path_bitmap;
    asprintf(&path_bitmap, "%s/bitmap.dat", path_base_dialfs);

    //Cantidad de bloques libre en el archivo bloques.dat
    bloques_libres = block_count;

    //Crear o Abrir, en caso de existir, Archivo bloques.dat
    crear_archivo_bloques();

    //Crear o Abrir, en caso de existir, Archivo bitmap.dat
    crear_archivo_bitmap();

    /*Crear la lista donde voy a poner los archivos
    Necesita para compactar*/
    lista_archivos = list_create();

    //Agregar nombre de todos los archivos que estan en este directorio
    agregar_a_lista_archivos(path_archivo_comun, lista_archivos);

    //Loop infinito de escucha a KERNEL
    while (1)
    {
        int operation_code = fetch_codop(socket_kernel);
        switch (operation_code)
        {
        case DIALFS_CREATE:
            usleep(tiempoUnidad * 1000);
            crear_archivo(socket_kernel);
            enviarAvisoAKernel(socket_kernel,CONFIRMACION_DIALFS_CREATE);
            break;
        case DIALFS_DELETE:
            usleep(tiempoUnidad * 1000);
            borrar_archivo(socket_kernel);
            enviarAvisoAKernel(socket_kernel,CONFIRMACION_DIALFS_DELETE);
            break;
        case DIALFS_TRUNCATE:
            usleep(tiempoUnidad * 1000);
            truncar_archivo(socket_kernel);
            enviarAvisoAKernel(socket_kernel,CONFIRMACION_DIALFS_TRUNCATE);
            break;
        case DIALFS_READ:
            usleep(tiempoUnidad * 1000);
            leer_archivo(socket_kernel);
            enviarAvisoAKernel(socket_kernel,CONFIRMACION_DIALFS_READ);
            break;
        case DIALFS_WRITE:
            usleep(tiempoUnidad * 1000);
            escribir_archivo(socket_kernel);
            enviarAvisoAKernel(socket_kernel,CONFIRMACION_DIALFS_WRITE);
            break;
        case -1:
            log_error(logger, "Error al recibir el codigo de operacion");
            close_conection(socket_kernel);
            return;
        default:
            log_error(logger, "Algun error inesperado");
            close_conection(socket_kernel);
            return;
        }
    }
}

void crear_archivo_bloques(){
    //length de archivo bloques.dat
    int length_archivo = block_count * block_size;

    FILE *archivo = fopen(path_bloques, "rb+");
    if (archivo == NULL){
        archivo = fopen(path_bloques, "wb+");
        
        //Si todavia no existe el archivo hay problema por lo que tira un EXIT_FAILURE
        if (archivo == NULL) {
            log_info(logger,"Error creando archivo bloques.dat");
            exit(EXIT_FAILURE);
        }

        //File Descriptor
        int file_descriptor = fileno(archivo);

        ftruncate(file_descriptor,length_archivo);

        log_info(logger, "Archivo de bloques creado y abierto");
    }else{
        log_info(logger, "Archivo de bloques abierto");
    }

    //Mapear bloques.dat a memoria
    bloques_map =  mmap(NULL, length_archivo, PROT_READ | PROT_WRITE, MAP_SHARED, fileno(archivo), 0);
    if (bloques_map == MAP_FAILED) {
        perror("Error mapeando bloques.dat");
        exit(EXIT_FAILURE);
    }
}

void crear_archivo_bitmap(){
    //length de archivo bloques.dat
    int length_archivo = (block_count / 8) + 1;

    FILE *archivo = fopen(path_bitmap, "rb+");
    if (archivo == NULL){
        archivo = fopen(path_bitmap, "wb+");
        
        //Si todavia no existe el archivo hay problema por lo que tira un EXIT_FAILURE
        if (archivo == NULL) {
            log_info(logger,"Error creando archivo bitmap.dat");
            exit(EXIT_FAILURE);
        }

        //File Descriptor
        int file_descriptor = fileno(archivo);

        ftruncate(file_descriptor,length_archivo);

        log_info(logger, "Archivo de bitmap creado y abierto");
    }else{
        log_info(logger, "Archivo de bitmap abierto");
    }

    //Mapear bitmap.dat a memoria
    bitmap_map = mmap(NULL, length_archivo, PROT_READ | PROT_WRITE, MAP_SHARED, fileno(archivo), 0);
    if (bitmap_map == MAP_FAILED) {
        log_info(logger, "Error mapeando bitmap.dat");
        exit(EXIT_FAILURE);
    }
    
    //Creacion de un bitarray
    bitarray = bitarray_create_with_mode(bitmap_map, length_archivo, LSB_FIRST);
}

void crear_archivo(int socket_kernel){
    int total_size;
    int offset = 0;
    int pid;
    char *nombre_archivo;
    int tamano_nombre_archivo;
    char* path_archivo;
    bool yaExiste = false;

    void *buffer2;
    buffer2 = fetch_buffer(&total_size, socket_kernel);

    //Tamano de Nombre de Archivo
    memcpy(&tamano_nombre_archivo,buffer2 + offset, sizeof(int));
    offset += sizeof(int);

    //Nombre de Archivo
    nombre_archivo = malloc(tamano_nombre_archivo);
    memcpy(nombre_archivo,buffer2 + offset, tamano_nombre_archivo);
    offset += tamano_nombre_archivo;

    //Saltar tamano de PID que no es necesario
    offset += sizeof(int);

    //PID
    memcpy(&pid,buffer2 + offset, sizeof(int));

    log_info(logger, "PID: %i - Crear Archivo: %s", pid, nombre_archivo);

    //Solo en caso donde la lista no contiene este archivo agregar a lista

    //Necesito para buscar_de_lista
    nombre_archivo_a_buscar = nombre_archivo;
    char* nombre_archivo_a_crear = list_find(lista_archivos, buscar_de_lista);

    if(nombre_archivo_a_crear == NULL) {
        //necesita para compactar 
        //Agregar nombre es suficiente pq despues hay que abrir el archivo de metadata
        list_add(lista_archivos, nombre_archivo_a_crear);
    }else{
        yaExiste = true;
        log_info(logger, "Ya existe el archivo a crear.");
    }

    //Path de archivo a Crear
    asprintf(&path_archivo, "%s/%s", path_archivo_comun, nombre_archivo);

    //Crear archivo
    FILE *archivo = fopen(path_archivo, "w+");
    if (archivo == NULL) {
        log_info(logger,"Error creando el archivo");
        exit(EXIT_FAILURE);
    }

    //Solo en casos donde archivo fue creada y no abierta

    if(!yaExiste) {
        /*Como crear_archivo solo crea un archivo sin ningun contexto no tiene sentido guardarlo en bloques.dat ya que no va a 
        guardar nada y no cambiara el bloques.dat. Solo dejar ocupado un bit en bitmap.dat es suficiente */

        //Buscar el primer bit libre desde bitarray
        int bit_libre = primer_bit_libre(bitarray);

        //Ocupar dicho bit 
        bitarray_set_bit(bitarray, bit_libre);

        //Disminuir bloques_libres solo por 1 ya que durante creacion el archivo ocupa un bit solo
        bloques_libres = bloques_libres - 1;

        //Escribir en archivo metadata el bloque inicial y tamano 
        fprintf(archivo, "BLOQUE_INICIAL=%i\n", bit_libre);
        fprintf(archivo, "TAMANIO_ARCHIVO=0\n");
    }

    fclose(archivo);
}


int primer_bit_libre(t_bitarray *bitarray){
    //Desde el primer bit hace un check de si esta ocupado o no (Si es 1) y solo en caso no ocupado retorna el primer bit libre
    for (int i = 0; i < block_count; i++) {
        if (!bitarray_test_bit(bitarray, i)) {
            return i;
        }
    }

    return -1; 
}

void borrar_archivo(int socket_kernel){
    int total_size;
    int offset = 0;
    int pid;
    char *nombre_archivo;
    int tamano_nombre_archivo;
    int bloque_inicial;
    int tamano;
    char* path_archivo;

    void *buffer2;
    buffer2 = fetch_buffer(&total_size, socket_kernel);

    //Tamano de Nombre de Archivo
    memcpy(&tamano_nombre_archivo,buffer2 + offset, sizeof(int));
    offset += sizeof(int);

    //Nombre de Archivo
    nombre_archivo = malloc(tamano_nombre_archivo);
    memcpy(nombre_archivo,buffer2 + offset, tamano_nombre_archivo);
    offset += tamano_nombre_archivo;

    //Saltar tamano de PID que no es necesario
    offset += sizeof(int);

    //PID
    memcpy(&pid,buffer2 + offset, sizeof(int));
    
    //Necesito para buscar_de_lista
    nombre_archivo_a_buscar = nombre_archivo;

    log_info(logger, "PID: %i - Eliminar Archivo: %s", pid, nombre_archivo);

    //necesita para compactar 
    //Busco el nombre de archivo desde la lista y borro dicho nombre
    char* nombre_archivo_a_borrar = list_find(lista_archivos, buscar_de_lista);

    //Solo si existe el archivo en lista se borra
    if(nombre_archivo_a_borrar != NULL) {
        list_remove(lista_archivos,nombre_archivo_a_borrar);
    }

    //Path de archivo a Borrar
    asprintf(&path_archivo, "%s/%s", path_archivo_comun, nombre_archivo);

    //Abrir archivo para leer el bloque inicial y tamano 
    FILE *archivo = fopen(path_archivo, "r");
    if (archivo == NULL) {
        log_info(logger,"Error abriendo el archivo");
        exit(EXIT_FAILURE);
    }

    //Leer y guardar Bloque Inicial
    fscanf(archivo, "BLOQUE_INICIAL=%i\n", &bloque_inicial);

    //Leer y guardar Tamano 
    fscanf(archivo, "TAMANIO_ARCHIVO=%i\n", &tamano);

    fclose(archivo);

    //Liberar los bloques que ocupaba el archivo desde bitrray 
    hacer_libre_bloques(bitarray, bloque_inicial, tamano);

    //Borrar el Archivo
    if(remove(path_archivo) == 0) {
        log_info(logger,"Archivo %s borrado", nombre_archivo);
    } else {
        log_info(logger,"Error borrando el archivo");
    }

}

//Necesito para list_find
bool buscar_de_lista(void *nombre_archivo){
	char *archivo_name = (char*) nombre_archivo;

	return nombre_archivo == nombre_archivo_a_buscar;
}

//Liberar bloques en bitmap.dat
void hacer_libre_bloques(t_bitarray *bitarray, int bloque_inicial, int tamano){
    //Calcular el cantidad de bloques ocupados para el archivo
    int bloques_ocupados = (tamano + block_size - 1) / block_size;  

    int bloque_mas_alta_ocupado = bloques_ocupados + bloque_inicial;

    for (int i = bloque_inicial; i < bloque_mas_alta_ocupado; i++) {
        bitarray_clean_bit(bitarray, i);

        //Sumar bloques_libres cada vez que libera un bit
        bloques_libres = bloques_libres + 1;
    }
}

void truncar_archivo(int socket_kernel){

    int total_size;
    int offset = 0;
    int pid;
    char *nombre_archivo;
    int tamano_nombre_archivo;
    int tamano_nuevo;
    int bloque_inicial;
    int tamano;
    char* path_archivo;

    void *buffer2;
    buffer2 = fetch_buffer(&total_size, socket_kernel);

    //Tamano de Nombre de Archivo
    memcpy(&tamano_nombre_archivo,buffer2 + offset, sizeof(int));
    offset += sizeof(int);

    //Nombre de Archivo
    nombre_archivo = malloc(tamano_nombre_archivo);
    memcpy(nombre_archivo,buffer2 + offset, tamano_nombre_archivo);
    offset += tamano_nombre_archivo;

    //Saltar tamano de PID que no es necesario
    offset += sizeof(int);

    //PID
    memcpy(&pid,buffer2 + offset, sizeof(int));
    offset += sizeof(int);

    //Saltar el tamano de tamano_nuevo que no es necesario
    offset += sizeof(int);

    //Tamano Nuevo de archivo a truncar
    memcpy(&tamano_nuevo,buffer2 + offset, sizeof(int));

    //Path de archivo a Borrar
    asprintf(&path_archivo, "%s/%s", path_archivo_comun, nombre_archivo);

    //Abrir archivo para leer el bloque inicial y tamano 
    FILE *archivo = fopen(path_archivo, "r");
    if (archivo == NULL) {
        log_info(logger,"Error abriendo el archivo");
        exit(EXIT_FAILURE);
    }

    //Guardar el bloque_inicial
    fscanf(archivo, "BLOQUE_INICIAL=%i\n", &bloque_inicial);

    //Guardar el tamano
    fscanf(archivo, "TAMANIO_ARCHIVO=%i\n", &tamano);

    //Tengo a este dos variables para casos donde tengo que actualizar el archivo de metadata de nuevo por no poder truncar por falta de espacio
    int bloque_inicial_truncate_fail = bloque_inicial;
    int tamano_truncate_fail = tamano;

    /*Diferencia entre tamano anterior de archivo y tamano nuevo de archivo.
    Si diferencia_tamano > 0 significa que tamano_nuevo es mayor que tamano anterior 
    Y si diferencia_tamano < 0 significa que tamano_nuevo es menor que tamano anterior
    Y si diferencia_tamano = 0 significa que tamano_nuevo es igual que tamano anterior*/
    int diferencia_tamano = tamano_nuevo - tamano;

    //Cantidad de bloques a agrandar 
    /*Como ya sabemos que diferencia_tamano es mayor que 0, no va a existir casos donde 
    cantidad_bloques_agrandar es un valor negativo*/
    int cantidad_bloques_agrandar = (diferencia_tamano * block_size - 1) / block_size;

    //Cantidad de bloque que ocupaba anteriormente de agrandar
    int cantidad_de_bloque_ultimo_anteriormente = (tamano * block_size - 1) / block_size;

    //Lo que usa despues para agrandar el archivo
    int bloque_final;

    log_info(logger, "PID: %i - Truncar Archivo: %s - Tamano a Leer: %i", pid, nombre_archivo, tamano);

    //Achicar el archivo
    //Como es achicar, no hay necesidad de organizar el bloqus.dat ni bitmap.dat
    if(diferencia_tamano < 0){
        log_info(logger,"Tamano nuevo a truncar el archivo menor que lo de tamano actual de archivo.");

        //Pasar la diferencia desde negativo a positivo para usar en 
        int diferencia_tamano_positivo = diferencia_tamano * (-1);

        //Esto es la cantidad de bloques que ocupa 
        int bloques_para_tamano_nuevo = (tamano_nuevo + block_size - 1) / block_size;

        //Aqui el primer bloque a borrar sera el bloque_inicial donde esta dado por el archivo mas el bloques_para_tamano_nuevo.
        int bloque_inicial_a_borrar = bloque_inicial + bloques_para_tamano_nuevo;

        /*Liberar el bitarray desde el bloque_inicial_a_borrar calculado el tamano de diferencia_tamano_positivo
        Libera en el bitmap.dat*/
        hacer_libre_bloques(bitarray, bloque_inicial_a_borrar, diferencia_tamano_positivo);

    }
    //Agrandar el archivo
    else if(diferencia_tamano > 0){
        log_info(logger,"Tamano nuevo a truncar el archivo mayor que lo de tamano actual de archivo.");

        //Aqui el primer bloque a agrandar sera el bloque_inicial donde esta dado por el archivo mas el cantidad_de_bloque_ultimo_anteriormente.
        bloque_final = bloque_inicial + cantidad_de_bloque_ultimo_anteriormente;

        //Primero verifico si puedo truncar desde donde estaba el archivo anteriormente
        if(existe_espacio_para_agrandar(bitarray, bloque_final, cantidad_bloques_agrandar)){
            
            //Necesita para fseek
            char *tamano_length;
            asprintf(&tamano_length, "TAMANIO_ARCHIVO=%i\n", tamano); 

            //Actualizar archivo tamano
            //Mover el puntero de archivo al inicio de tamano para cambiar el valor de tamano en el archivo
            fseek(archivo, ftell(archivo) - strlen(tamano_length), SEEK_SET);

            //Actualizar tamano archivo a tamano_nuevo
            fprintf(archivo, "TAMANIO_ARCHIVO=%i\n", tamano_nuevo);

            //Ocupar bits continuo adicional en bitmap.dat
            for(int i = 0; i < cantidad_bloques_agrandar; i++) {
                //El + 1 en el fin es pq no tengo que ocupar el bloque_final pq si va a estar ocupado siempre  
                bitarray_set_bit(bitarray, bloque_final + i + 1);
                
                //Disminuir bloques_libres por 1
                bloques_libres = bloques_libres - 1;
            }

            /*No es necesario actualizar el bloques.dat ya que si se agranda el archivo pero no es que su contenido cambia. 
            Por lo que ocupar bits en bitmap.dat es suficiente*/
        }
        //No hay suficiente espacio continuo asi que verifico si hay despues de compactar el bloques.dat y bitmap.dat
        else{
            log_info(logger, "No hay suficiente espacio para truncar el archivo. Inicio compactacion");

            log_info(logger, "PID: %i - Inicio Compactacion", pid);

            //Compactar el bloques.dat y bitmap.dat en formato 0011101010 a 1111100000 para tener maxima espacio continuo libre
            /*Seleccionar el bloques donde esta el archivo y mandalo mas frente
            Si fue 11 111(Archivo a agrandar) 1 00000 moverlo a 111 111(Archivo a agrandar) 00000*/
            compactar(bitarray, nombre_archivo);

            log_info(logger, "PID: %i - Fin Compactacion", pid);

            //Despues de compactar hay que esperar por un tiempo determinado por retraso_compactacion.
            //Multiplico por 1000 para microsegundos
            usleep(retraso_compactacion * 1000);

            //Puntero a parte inicial del archivo
            rewind(archivo);

            //Guardar el bloque_inicial encontrado despues de compactar
            fscanf(archivo, "BLOQUE_INICIAL=%i\n", &bloque_inicial);

            //No es necesario actualizar el tamano ya que solo bloque_inicial cambia cuando compacta

            //Como cambio el bloque_inicial tengo que actualizar bloque_final tambien
            bloque_final = bloque_inicial + cantidad_de_bloque_ultimo_anteriormente;

            //Verificar si hay suficiente espacio con existe_espacio_para_agrandar()
            if(existe_espacio_para_agrandar(bitarray, bloque_final, cantidad_bloques_agrandar)){
                
                //Necesita para fseek
                char *tamano_length2;
                asprintf(&tamano_length2, "TAMANIO_ARCHIVO=%i\n", tamano); 

                //Actualizar archivo metadata tamano
                //Mover el puntero de archivo al inicio de tamano para cambiar el valor de tamano en el archivo metadata
                fseek(archivo, ftell(archivo) - strlen(tamano_length2), SEEK_SET);

                //Actualizar tamano archivo a tamano_nuevo
                fprintf(archivo, "TAMANIO_ARCHIVO=%i\n", tamano_nuevo);

                //Ocupar bits continuo adicional en bitmap.dat
                for(int i = 0; i < cantidad_bloques_agrandar; i++) {
                    //El + 1 en el fin es pq no tengo que ocupar el bloque_final pq si va a estar ocupado siempre  
                    bitarray_set_bit(bitarray, bloque_final + i + 1);
                    
                    //Disminuir bloques_libres por 1
                    bloques_libres = bloques_libres - 1;
                }

                /*No es necesario actualizar el bloques.dat ya que si se agranda el archivo pero no es que su contenido cambia. 
                Por lo que ocupar bits en bitmap.dat es suficiente*/
            }
            /*En caso que no hay suficiente espacio sigfnifica que no puede por lo que logeo en logger y vuelve a cambiar los 
            valores de bloque_inicial y tamano escrito en el archivo de metadata*/            
            else{
                log_info(logger, "No existe bloques suficiente para agrandar. Truncate Fail");

                //Como no podia truncar entonces actualizo el archivo de metadata para que tiene los datos antes de truncar
                //Puntero a parte inicial del archivo
                rewind(archivo);

                //Actualizar tamano archivo a tamano_nuevo
                fprintf(archivo, "BLOQUE_INICIAL=%i\n", bloque_inicial_truncate_fail);

                //Actualizar tamano archivo a tamano_nuevo
                fprintf(archivo, "TAMANIO_ARCHIVO=%i\n", tamano_truncate_fail);
            }
        }
    }else{
        /*diferencia_tamano = 0
        Aqui no tengo que hacer nada ya que ni voy a achicar y agrandar el archivo a tamano nuevo*/
        log_info(logger,"No hay cambio al Archivo ya que su tamano actual y tamano nuevo son mismo");
    }

    fclose(archivo);
}

//Verificar en el bitmap.dat si existe suficiente espacio continuo para agrandar desde el posicion que esta 
bool existe_espacio_para_agrandar(t_bitarray *bitarray, int bloque_final, int cantidad_bloques_agrandar){
    
    for(int i = 0; i < cantidad_bloques_agrandar; i++) {
        //El + 1 en el fin es pq no tengo que verificar si esta ocupado el bloque_final pq si va a estar ocupado siempre  
        if(bitarray_test_bit(bitarray, bloque_final + i + 1)){
            return false;
        }
    }
    return true;
}

/*Compactar el bitarray y bloques_map y tener en cuenta dejar el archivo buscado en bloque_inicial con su tamano al fin.
Si fue 00 11 0 111(Archivo a agrandar) 1 00 moverlo a 111 111(Archivo a agrandar) 00000*/
void compactar(t_bitarray *bitarray, char* nombre_archivo_a_truncar){
    /*Estoy borrando el archivo a truncar desde lista primero para trabajr con los archivos
    que no son los que van a ir a fin de secuencia 1 11 11 000*/
    list_remove(lista_archivos,nombre_archivo_a_truncar);

    char* nombre_archivo;
    char* nombre_archivo_metadata_a_truncar;
    char* path_archivo;
    char* path_archivo_a_truncar;
    char* path_archivo_metadata_a_truncar;
    
    int bloque_inicial = 0;
    int tamano;
    int bloques_ocupados;
    int cantidad_archivos_en_lista = list_size(lista_archivos);
    int cant_archivos_leidos = 0;
    int tamano_a_saltar = 0;

    int bloque_fin_ocupado;
    int tamano_bloques_ocupados;

    t_list* lista_temp = list_create();

    //liberar todos los bits en bitarray que voy a reocuparlo con metadata nuevo
    hacer_libre_bloques(bitarray, 0, block_size * block_count);

    //Actualizar todos los archivos de metadata para usarlo despues en posicionar el bitmap.dat y bloques.dat
    while(cantidad_archivos_en_lista > cant_archivos_leidos){
        nombre_archivo = list_get_first(lista_archivos);

        //Path de archivo a Borrar
        asprintf(&path_archivo, "%s/%s", path_archivo_comun, nombre_archivo);

        //Abrir archivo para leer el bloque inicial y tamano 
        FILE *archivo = fopen(path_archivo, "r");
        if (archivo == NULL) {
            log_info(logger,"Error abriendo el archivo");
            exit(EXIT_FAILURE);
        }

        //Leer y guardar Bloque Inicial
        fprintf(archivo, "BLOQUE_INICIAL=%i\n", &bloque_inicial);

        //Leer tamano
        fscanf(archivo, "TAMANIO_ARCHIVO=%i\n", &tamano);

        //Buscar cantidad de bloques ocupados por el archivo
        bloques_ocupados = (tamano + block_size - 1) / block_size; 

        bloque_fin_ocupado = bloque_inicial + bloques_ocupados;

        //Necesito este tamano para actualizar bloques.dat
        tamano_bloques_ocupados = bloques_ocupados * block_size;

        //Ocupar bits en bitarray
        for(int i = bloque_inicial; i < bloque_fin_ocupado; i++){
            bitarray_set_bit(bitarray, i);
        }

        //Crear un buffer para guardar el contenido de archivo
        char* buffer = (char*) malloc(tamano_bloques_ocupados);
        if (buffer == NULL) {
            perror("Error asignando memoria a buffer");
            fclose(archivo);
            exit(EXIT_FAILURE);
        }

        //Guardar contenido de archivo al buffer
        fread(buffer, 1, tamano_bloques_ocupados, archivo);

        //Copiar datos a bloques_map
        memcpy(bloques_map + tamano_a_saltar, buffer, tamano_bloques_ocupados);

        free(buffer);

        //Actualizar tamano_a_saltar para proxima escritura a bloques.dat
        tamano_a_saltar += tamano_bloques_ocupados;

        //actualiza el bloque_inicial para ponerlo a siguiente archivo
        bloque_inicial += bloques_ocupados;

        //Agregar en lista_temp y borrar de la lista principal ya que hizo lo que tengo que a este archivo
        //Despues se agrega de nuevo hasi que no hay problema
        list_add(lista_temp, nombre_archivo);
        list_remove(lista_archivos,nombre_archivo);

        cant_archivos_leidos++;

        //Para proxima loop hago null dichos variables
        nombre_archivo = NULL;
        path_archivo = NULL;

        fclose(archivo);
    }

    //Aca estoy haciendo esto para archivo que tengo que truncar

    //Path de archivo a Borrar
    asprintf(&path_archivo_a_truncar, "%s/%s", path_archivo_comun, nombre_archivo_a_truncar);

    //Abrir archivo para leer el bloque inicial y tamano 
    FILE *archivo = fopen(path_archivo, "r");
    if (archivo == NULL) {
        log_info(logger,"Error abriendo el archivo");
        exit(EXIT_FAILURE);
    }

    //Leer y guardar Bloque Inicial
    fprintf(archivo, "BLOQUE_INICIAL=%i\n", &bloque_inicial);

    //Leer tamano
    fscanf(archivo, "TAMANIO_ARCHIVO=%i\n", &tamano);

    //Buscar cantidad de bloques ocupados por el archivo
    bloques_ocupados = (tamano + block_size - 1) / block_size; 

    bloque_fin_ocupado = bloque_inicial + bloques_ocupados;

    //Necesito este tamano para actualizar bloques.dat
    tamano_bloques_ocupados = bloques_ocupados * block_size;

    //Ocupar bits en bitarray
    for(int i = bloque_inicial; i < bloque_fin_ocupado; i++){
        bitarray_set_bit(bitarray, i);
    }

    //Crear un buffer para guardar el contenido de archivo
    char* buffer_a_truncar = (char*) malloc(tamano_bloques_ocupados);
    if (buffer_a_truncar == NULL) {
        perror("Error asignando memoria a buffer");
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    //Guardar contenido de archivo al buffer
    fread(buffer_a_truncar, 1, tamano_bloques_ocupados, archivo);

    fclose(archivo);

    //Copiar datos a bloques_map
    memcpy(bloques_map + tamano_a_saltar, buffer_a_truncar, tamano_bloques_ocupados);

    free(buffer_a_truncar);

    //Como despues de while es vacio hago esto para volver a contener los archivos que tenia anteriormente
    lista_archivos = lista_temp;

    //Agrego el archivo a truncar devuelta a la lista
    list_add(lista_archivos,nombre_archivo_a_truncar);

    fclose(archivo);
}

//No estaba en commons asi que creo un funcion que retorna primer elemento de la lista 
char* list_get_first(t_list *list) {
    if (list->head == NULL) {
        return NULL; 
    }
    return (char*)list->head->data; 
}

/*Cada vez que se corra de nuevo el codigo se pierda la lista por lo que creo una funcion
agrega los nombres de archivos */

void agregar_a_lista_archivos(char* path_archivos, t_list* lista_archivos) {
    DIR *direccion;
    struct dirent *dir;
    direccion = opendir(path_archivos);

    if (direccion) {
        while ((dir = readdir(direccion)) != NULL) {
            //No voy a agregar el bitmap.dat y bloques.dat a la lista
            if (strcmp(dir->d_name, "bloques.dat") == 0 || strcmp(dir->d_name, "bitmap.dat") == 0) {
                continue;
            } 

            char *nombre_archivo = strdup(dir->d_name);
            list_add(lista_archivos, nombre_archivo);
        }
        closedir(direccion);
    } else {
        perror("Error abriendo el directory");
    }
}

//Para IO_FS_WRITE
/*Verificar que pasa si tamano de escritura pasa el limite de tamanos(Cantidad de bloques) disponibles para el archivos.
Sera que debe esperar primera truncar para escribir o en momento de falata de espacio puede actualizar su tamano?*/
void escribir_archivo(int socket_kernel){
    int total_size;
    int offset = 0;

    int pid;
    char *nombre_archivo;
    int tamano_nombre_archivo;
    int puntero_archivo;
    int cantidad_bytes_cdf;
    int cantidad_direccioens_fisicas;
    char *path_archivo;

    int cantidad_bytes;
    int direccion_fisica;

    int desplazamiento = 0;

    int bloque_inicial;
    int tamano;

    void *buffer2;
    buffer2 = fetch_buffer(&total_size, socket_kernel);

    //Tamano de Nombre de Archivo
    memcpy(&tamano_nombre_archivo,buffer2 + offset, sizeof(int));
    offset += sizeof(int);

    //Nombre de Archivo
    nombre_archivo = malloc(tamano_nombre_archivo);
    memcpy(nombre_archivo,buffer2 + offset, tamano_nombre_archivo);
    offset += tamano_nombre_archivo;

    //Saltar tamano de PID que no es necesario
    offset += sizeof(int);

    //PID
    memcpy(&pid,buffer2 + offset, sizeof(int));
    
    //Saltar tamano de Puntero Archivo que no es necesario
    offset += sizeof(int);
    
    //Puntero Archivo
    memcpy(&puntero_archivo,buffer2 + offset, sizeof(int));
    offset += sizeof(int);

    //Saltar tamano de Cantidad Bytes que no es necesario
    offset += sizeof(int);

    //Cantidad Bytes
    memcpy(&cantidad_bytes_cdf,buffer2 + offset, sizeof(int));
    offset += sizeof(int);

    //Saltar tamano de Cantidad Direcciones Fisicas que no es necesario
    offset += sizeof(int);

    //Cantidad Direcciones Fisicas
    memcpy(&cantidad_direccioens_fisicas,buffer2 + offset, sizeof(int));
    offset += sizeof(int);

    log_info(logger, "PID: %i - Escribir Archivo: %s - Tamaño a Escribir: %i - Puntero Archivo: %i", pid, nombre_archivo, cantidad_bytes_cdf, puntero_archivo);

    //Necesito para buscar_de_lista
    nombre_archivo_a_buscar = nombre_archivo;
    char* nombre_archivo_a_escribir = list_find(lista_archivos, buscar_de_lista);

    //Si no existe el archivo a escribir entonces non vamnos a poder escribir por lo que hacemos un exit
    if(nombre_archivo_a_escribir == NULL) {
        log_info(logger,"Error buscando el archivo para escribir");
        exit(EXIT_FAILURE);
    }

    //Path de archivo a Borrar
    asprintf(&path_archivo, "%s/%s", path_archivo_comun, nombre_archivo);

    //Abrir archivo para leer el bloque inicial y tamano 
    FILE *archivo = fopen(path_archivo, "r");
    if (archivo == NULL) {
        log_info(logger,"Error abriendo el archivo");
        exit(EXIT_FAILURE);
    }

    //Leer y guardar Bloque Inicial
    fprintf(archivo, "BLOQUE_INICIAL=%i\n", &bloque_inicial);

    //Leer y guardar Tamano
    fprintf(archivo, "TAMANO=%i\n", &tamano);

    //Calculo la posicion de inicio de archivo en bloques.dat
    int posicion_archivo = bloque_inicial *block_size;

    //Posicion de escritura en bloques.dat
    int posicion_bloques_dat = posicion_archivo + puntero_archivo;

    //Posicion de donde tengo que leer en el memoria
    void* posicion_memoria = bloques_map + posicion_bloques_dat;

    /*Antes todo tengo que probar si hay suficiente bloque para escribir lo que lee desde memoria a archivo en bitmap.dat
    Verifico si tamano max archivo - puntero archivo > tamano de escritura pq sino cumple este condicion deben truncar priemro
    y no va a poder escribir.*/
    bool tiene_suficiente_espacio_para_escribir = tamano - puntero_archivo > cantidad_bytes_cdf;

    if(tiene_suficiente_espacio_para_escribir){

        //Un for loop para mandar a leer cada direcciones fisicas
        for(int i = 0; i < cantidad_direccioens_fisicas; i++){

            //Salto tamano de cantidad bytes
            offset += sizeof(int); 

            //Cantidad bytes
            memcpy(&cantidad_bytes,buffer2 + offset, sizeof(int)); 
            offset += sizeof(int);

            //Salto tamano de Direccion Fisica
            offset += sizeof(int);

            //Direccion Fisica
            memcpy(&direccion_fisica,buffer2 + offset, sizeof(int)); 
            offset += sizeof(int);

            //Mandar a leer 
            //Como desplazamiento = 0 en el primer loop, se va a guardar desde posicion_memoria
            mandarALeer(direccion_fisica, cantidad_bytes, pid, posicion_memoria + desplazamiento);

            //Actualizo desplazamiento para siguiente direccion Fisica y Cantidad bytes
            desplazamiento += cantidad_bytes;
        }

        //Copiar a bloques_map los que esta en posicion memoria
        //Hay que hacer + posicion_bloques_dat a bloques_map para la posicion actual de donde esta guardado el archivo
        memcpy(bloques_map + posicion_bloques_dat, posicion_memoria, cantidad_bytes_cdf);

    }else{
        log_info(logger,"No hay suficiente espacio. Trunca el archivo primero.");
    }

    fclose(archivo);
}

//Para IO_FS_READ 
//Se lea desde el archivo por lo que no va a tocar el bitmap.dat y bloques.dat
void leer_archivo(int socket_kernel){
    int total_size;
    int offset = 0;

    int pid;
    char *nombre_archivo;
    int tamano_nombre_archivo;
    int puntero_archivo;
    int cantidad_bytes_cdf;
    int cantidad_direccioens_fisicas;
    char* path_archivo;
    
    int cantidad_bytes;
    int direccion_fisica;

    int desplazamiento = 0;

    int bloque_inicial;
    int tamano;

    void *buffer2;
    buffer2 = fetch_buffer(&total_size, socket_kernel);

    //Tamano de Nombre de Archivo
    memcpy(&tamano_nombre_archivo,buffer2 + offset, sizeof(int));
    offset += sizeof(int);

    //Nombre de Archivo
    nombre_archivo = malloc(tamano_nombre_archivo);
    memcpy(nombre_archivo,buffer2 + offset, tamano_nombre_archivo);
    offset += tamano_nombre_archivo;

    //Saltar tamano de PID que no es necesario
    offset += sizeof(int);

    //PID
    memcpy(&pid,buffer2 + offset, sizeof(int));
    
    //Saltar tamano de Puntero Archivo que no es necesario
    offset += sizeof(int);
    
    //Puntero Archivo
    memcpy(&puntero_archivo,buffer2 + offset, sizeof(int));
    offset += sizeof(int);

    //Saltar tamano de Cantidad Bytes que no es necesario
    offset += sizeof(int);

    //Cantidad Bytes
    memcpy(&cantidad_bytes_cdf,buffer2 + offset, sizeof(int));
    offset += sizeof(int);

    //Saltar tamano de Cantidad Direcciones Fisicas que no es necesario
    offset += sizeof(int);

    //Cantidad Direcciones Fisicas
    memcpy(&cantidad_direccioens_fisicas,buffer2 + offset, sizeof(int));
    offset += sizeof(int);

    log_info(logger, "PID: %i - Leer Archivo: %s - Tamaño a Leer: %i - Puntero Archivo: %i", pid, nombre_archivo, cantidad_bytes_cdf, puntero_archivo);

    //Necesito para buscar_de_lista
    nombre_archivo_a_buscar = nombre_archivo;
    char* nombre_archivo_a_escribir = list_find(lista_archivos, buscar_de_lista);

    //Si no existe el archivo a escribir entonces non vamnos a poder escribir por lo que hacemos un exit
    if(nombre_archivo_a_escribir == NULL) {
        log_info(logger,"Error buscando el archivo para escribir");
        exit(EXIT_FAILURE);
    }

    //Path de archivo a Borrar
    asprintf(&path_archivo, "%s/%s", path_archivo_comun, nombre_archivo);

    //Abrir archivo para leer el bloque inicial y tamano 
    FILE *archivo = fopen(path_archivo, "r");
    if (archivo == NULL) {
        log_info(logger,"Error abriendo el archivo");
        exit(EXIT_FAILURE);
    }

    //Leer y guardar Bloque Inicial
    fprintf(archivo, "BLOQUE_INICIAL=%i\n", &bloque_inicial);

    //Leer y guardar Tamano
    fprintf(archivo, "TAMANO=%i\n", &tamano);

    //Calculo la posicion de inicio de archivo en bloques.dat
    int posicion_archivo = bloque_inicial *block_size;

    //Posicion de escritura en bloques.dat
    int posicion_bloques_dat = posicion_archivo + puntero_archivo;

    //Posicion de donde tengo que leer en el memoria
    void* posicion_memoria = bloques_map + posicion_bloques_dat;

    for (int i = 0; (i < cantidad_direccioens_fisicas) && (desplazamiento < cantidad_bytes_cdf); i++) {

        // Salto el tamano de cantidad bytes
        offset += sizeof(int);

        // Cantidad bytes 
        memcpy(&cantidad_bytes, buffer2 + offset, sizeof(int)); 
        offset += sizeof(int);

        // Salto el tamano de Direccion Fisica
        offset += sizeof(int);
        
        // Direccion Fisica
        memcpy(&direccion_fisica, buffer2 + offset, sizeof(int)); 
        offset += sizeof(int);       

        // Escribir en memoria lo que esta en el bloques.dat
        mandarAescribirEnMemoria(direccion_fisica, posicion_memoria + desplazamiento, cantidad_bytes, pid);

        // Actualizo la variable desplazamiento sumandole lo que acabo de escribir en memoria
        desplazamiento += cantidad_bytes;
    }

    fclose(archivo);
}