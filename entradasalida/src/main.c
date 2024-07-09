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
char *conservadorPATH;
t_bitarray *bitarray;
int block_size;
int block_count;
int bloquesLibres;
t_list *listaDeArchivos;
int bloqueABuscar;
void *contenidoFS;
int tiempoRetrasoCompactacion;
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


    //char *nombreInterfaz = "FS";
    //char *configRecibido = "filesystem.config";

    // ESTO ES PARA TENER LOS NOMBRE YA PUESTOS ACA Y NO ESTAR PASANDOLOS AL LLAMAR AL EXE
    //char *nombreInterfaz = "nombre1";
    //char *configRecibido = "entradasalida.config";
    t_buffer *buffer;
    t_packet *packet;
    char *tipo;
    listaDeArchivos = list_create();

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
    }else if(string_equals_ignore_case(tipo,"FS")){
        interfazFileSystem();
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
            usleep(tiempoUnidad * tiempo * 1000); 
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


void interfazFileSystem(){

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

    //Verifico si existen los archivos
    //Aclaracion: existe la funcion config_get_int. Se podria haber usado en lugar de los atoi
    tiempoRetrasoCompactacion = atoi(config_get_string_value(config, "RETRASO_COMPACTACION"));
    block_size = atoi(config_get_string_value(config, "BLOCK_SIZE"));
    block_count = atoi(config_get_string_value(config, "BLOCK_COUNT"));
    char *path = config_get_string_value(config, "PATH_BASE_DIALFS");
    char *pathBitMap = strdup(path);
    //Esto es para tener un contador de cuantos bloques libres hay
    bloquesLibres = block_count;

    conservadorPATH = strdup(path);
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++CREACION DEL ARCHIVO BLOQUES.DAT++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	string_append(&path,"bloques.dat"); 
    /*Aca lo que hago es primero intentar abrir el archivo en modo r+
    que lo que hace es abrir el archivo en modo lectura y escritura
    pero si no existe no lo crea, pero si existe, al abrirlo no lo trunca, osea
    no lo borra.
    Despues en caso que el archivo no exista entra por el else y ahi se lo abre en modo
    w+ que lo que hace es intentar abrir el archivo, si existe lo trunca, osea lo borra
    pero aca sabemos que si llegamos a tener que intentar abrirlo asi es porque no existe
    por eso lo puse en el else. En caso que no exista aca lo crea, por eso lo pongo en el else
    y despues de crearlo lo abre en modo lectura y escritura
    */
    FILE *archivoBloques = fopen(path, "rb+");
    
    if (archivoBloques != NULL){
        log_info(logger, "El archivo bloques.dat ya existe y está abierto en modo lectura/escritura.");
    }else{
        // El archivo no existe, crearlo y abrirlo en modo lectura/escritura
        archivoBloques = fopen(path, "wb+");
        
        if (archivoBloques == NULL) {
              log_info(logger,"Error al crear el archivo bloques.dat");
              exit(EXIT_FAILURE);
        }
        //Aca obtengo el FileDescriptor porque ftruncate pide eso
        int fdBloques = fileno(archivoBloques);
        //El block size esta en bytes
        ftruncate(fdBloques,(block_count * block_size));
        log_info(logger, "El archivo bloques.dat no existia, fue creado y abierto en modo lectura/escritura.");
    }
    //Aca abro el archivo con el mmap y me traigo el contenido a este espacio void contenido bloques para operarlo en memoria
    //y que los cambios que haga aca se vayan al archivo. El segundo parametro es la cantidad que tengo que leer del archivo parece
    //por eso puse directamente el tamaño que le estableci con ftruncate mas arriba. Porque tengo que tener todo el contenido
    //del archivo. Con esto seria algo parecido a memoria
    contenidoFS =  mmap(NULL,(block_count * block_size), PROT_READ | PROT_WRITE, MAP_SHARED,fileno(archivoBloques), 0);
    // truncar significa que se borra el contenido del archivo
    // "r": Abre un archivo para lectura. El archivo debe existir.
    // "r+": Abre un archivo para lectura y escritura. El archivo debe existir. 
    // No trunca el archivo, es decir, no borra su contenido.
    // "w": Abre un archivo para escritura. Si el archivo no existe, lo crea. Si
    //  el archivo existe, lo trunca (borra su contenido).
    // "w+": Abre un archivo para lectura y escritura. Si el archivo no existe,
    //  lo crea. Si el archivo existe, lo trunca (borra su contenido).
    // "a": Abre un archivo para escritura en modo adjuntar (append). Si el archivo 
    // no existe, lo crea. Si el archivo existe, escribe al final del archivo.
    // "a+": Abre un archivo para lectura y escritura en modo adjuntar (append). 
    // Si el archivo no existe, lo crea. Si el archivo existe, escribe al final del archivo.

    //Borrar esto -- Esto es para la prueba de FS para poder mandar algo a memoria directamente
    //a escribir y no tener que escribir primero el FS
    memcpy(contenidoFS + 1024,"ERROR ERROR",12);
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++CREACION DEL ARCHIVO BITMAP.DAT++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    string_append(&pathBitMap,"bitmap.dat");
    FILE *archivoBitMap = fopen(pathBitMap, "rb+");
    //Esto deberia funcionar con open seguro 
    if (archivoBitMap != NULL){
        log_info(logger, "El archivo BitMap.dat ya existe y está abierto en modo lectura/escritura.");
    }else{
        // El archivo no existe, crearlo y abrirlo en modo lectura/escritura
        archivoBitMap = fopen(pathBitMap, "wb+");
        
        if (archivoBitMap == NULL) {
              log_info(logger,"Error al crear el archivo BitMap.dat");
              exit(EXIT_FAILURE);
        }
        int fdBitMap = fileno(archivoBitMap);
        //Porque tengo que representar cada bloque con un bit, y el parametro que recibe ftruncate
        //es en bytes. Pero por las dudas dijeron que le sume uno al resultado de la division
        //para que no queden bits afuera
        int tamaArchivoBitMap = (block_count / 8) + 1;
        //El block size esta en bytes
        ftruncate(fdBitMap,tamaArchivoBitMap);
        log_info(logger, "El archivo BitMap.dat no existia, fue creado y abierto en modo lectura/escritura.");
    }

    //Declaro la cantidad de bits que va a tener el bitmap, por las dudas sumo 1
    //En realidad la cantidad de bits es el block count pero el mmap solo acepta bytes
    //Entonces por eso divido por 8, y sumo el 1 porque quizas la cifra es muy menor a 8
    //entonces se redondearia a 0 y perderia bits quizas tenia solo una pagina y quedaria en 0
    //por eso sumo el 1. Entonces ahora tendria bits de mas pero eso lo delimito en la
    //funcion buscar bit libre, lo delimito con la varibale global block_count
    int bitmap_size = (block_count / 8) + 1;
    //Aca asigno que en este espacio de memoria voy a tener la referencia al archivo
    //Mmap hace que el contenido del archivo lo tengamos en memoria RAM y que si cambiiamos algo
    //aca entonces se cambia en el archivo. Es como tener al archivo en RAM.
    char* bitmap_data = mmap(NULL, bitmap_size, PROT_READ | PROT_WRITE, MAP_SHARED,fileno(archivoBitMap), 0);
    
    if (bitmap_data == MAP_FAILED) {
        log_info(logger, "Error al mapear el archivo del bitmap a memoria");
        exit(EXIT_FAILURE);
    }

    // Inicializar el bitarray con el bitmap mapeado

    /*
    Aca lo que se hace es inicializar el bitarray en un espacio de memoria
    determinado, como el bitarray es una secuencia de bits aca 
    lee lo que tenias en esa posicion de memoria, la cantidad de bytes que le pasas
    y entonces crea el bitarray teniendo en cuenta eso. Porque lee los 1 y 0 que tenias
    y te crea el bitarray con eso. SI ya tenias una pagina en 1 bueno pone esa posicion 
    del bitarray en 1.
    */
    
    bitarray = bitarray_create_with_mode(bitmap_data, bitmap_size, LSB_FIRST);
    // Diferencia entre MSB First y LSB First Para el Bitarray
    // Interpretado en MSB first:

    // Bit	    7	6	5	4	3	2	1	0
    // Valor	0	1	1	1	1	1	1	1
    // Bitmap resultante en MSB First:
    // Bloques en uso: 7 bloques (bits 1 a 7)
    // Bloques libres: 1 bloque (bit 0)

    // Interpretación en formato LSB first
    // En formato LSB first, los bits se interpretan desde el bit menos significativo (derecho) hacia el más significativo (izquierdo). Esto nos da el siguiente orden:

    // 11111110

    // Bitmap resultante:
    // Bit	    7	6	5	4	3	2	1	0
    // Valor	1	1	1	1	1	1	1	0
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//+++++++++++++++++++CARGO LA LISTA DE ARCHIVOS EN CASO QUE YA HAYA ARCHIVOS CREADOS+++++++++++++
    //Hacemos esto para cargar la lista de archivos que usamos en la compactaacion
    //Porque cada vez que creo un archivo lo guardo en la lissta de archivos
    //pero si por ejemplo yo tengo un proceso que crea los archivos por
    //ejemplo crea 5, entonces se guardan los 5. Pero si por ejemplo bajan la interfaz FS
    //pierdo la lista y si depsues la vuelven a levantar y quieren compactar
    //entonces no puedo hacer nada porque perdi la lista y no puedo
    //compactar, por eso cada vez que levantamos la interfaz buscamos los archivos que exitan
    //excepcto bitmap y bloques. Leemos los que encontramos y guardamos la lista
    //para que no haya problemas
    
    // struct dirent {
    // ino_t          d_ino;       /* Número de inodo */
    // off_t          d_off;       /* Desplazamiento hasta la próxima `dirent` */
    // unsigned short d_reclen;    /* Longitud de este registro */
    // unsigned char  d_type;      /* Tipo de archivo */
    // char           d_name[256]; /* Nombre del archivo */
    // };

    struct dirent *entry;
    
    // La función opendir abre el directorio especificado
    // por conservadorPATH y devuelve un puntero a DIR, que es una estructura que representa
    // el directorio abierto.
    DIR *dp = opendir(conservadorPATH);

    if (dp == NULL) {
        log_error(logger,"Error al abrir el directorio");
        return;
    }

// La función readdir lee la siguiente entrada en el directorio dp y
// devuelve un puntero a una estructura dirent que contiene información sobre
// esa entrada. Si no hay más entradas, readdir devuelve NULL.
    while ((entry = readdir(dp))) {

        // entry->d_type: Verifica si la entrada es un archivo regular (DT_REG).
        // entry->d_name: Contiene el nombre del archivo.
        // Compara este nombre con "bloques.dat" y "bitmap.dat", y si no coincide
        // con ninguno de estos, construye la ruta completa al archivo y llama a
        // leer_archivo para leer su contenido.
        //Este log era para chequear que archivos abria porque quizas tenia 4 archivos
        //pero hasta aca llegaba 6 veces como si tendria otros archivos. Eso era porque
        //habria algunos archivos ocultos. Pero del if de mas abajo no pasaba
        //porque ahi filtramos lo que son archivos regulares asi que de ahi no pasa
        //osea que no llega a la parte donde abrimos los archivos pero era raro
        //log_info(logger,"Nombre de entrada: %s, Tipo: %d", entry->d_name, entry->d_type);
        
        //  los archivos . (directorio actual) y .. (directorio padre) no llegarían a la
        //  parte donde intentas abrir y procesar archivos, incluso si no se agrega el filtro
        //  adicional. Esto se debe a que readdir está devolviendo el tipo DT_DIR para estos
        //  directorios especiales, y tu código actual está filtrando explícitamente solo los
        //  archivos regulares (DT_REG).

        // Por lo tanto, en tu implementación actual, los archivos . y .. se omitirán
        // automáticamente porque no cumplen con la condición entry->d_type == DT_REG.

        if (entry->d_type == DT_REG) { // Verifica si es un archivo regular
            
            if (strcmp(entry->d_name, "bloques.dat") != 0 && strcmp(entry->d_name, "bitmap.dat") != 0) {
                
                //Armo el path para abrir el archivo
                char *pathLeer = strdup(conservadorPATH);
                //A la ruta del directorio agrego el nombre del archivo
                string_append(&pathLeer,entry->d_name);
                //Abro el archivo y me fijo si abrio bien
                FILE *archivoLeer = fopen(pathLeer, "r");
                if (archivoLeer == NULL) {
                    log_error(logger,"Error al abrir el archivo %s metadata",entry->d_name);
                    exit(EXIT_FAILURE);
                }

                int bloque_inicial;
                int tama_archivo;
                //Leo del archivo los datos especificos y lo cierro
                fscanf(archivoLeer, "BLOQUE_INICIAL=%d\n", &bloque_inicial);
                fscanf(archivoLeer, "TAMANIO_ARCHIVO=%d\n", &tama_archivo);
                fclose(archivoLeer);
                

                //Me guardo el path de archivos y los datos de tamaños y bloque inicial
                //esto para poder usarlo para compactar
                t_archivo *archivoAGuardar = malloc(sizeof(t_archivo));
                //Por las dudas hago el strdup y libero el original
                archivoAGuardar->pathArchivo = strdup(pathLeer);
                //Guardo el bloque inicial que lei
                archivoAGuardar->bloque_inicial = bloque_inicial;
                //Aca pongo tamaarchivo pero esta en cantidad de bloques, no en bytes
                //Este campo dentro de la lista no lo uso nunca creo
                //pero igual hay que aclarar que dice tamaarchivo 
                //pero el tamaño no esta en bytes, esta en bloques

                //Hago la conversion a bloques
                int cant_bloq = (tama_archivo + block_size - 1) / block_size;

                //No uso este dato, pero por las dudas, como al crear un archivo
                // tamaño dijeron que tenemos que ponerle un 0 pero en realidad ocupa
                //1 bloque (Porque apenas se crea le asignamos uno), en la compactacion
                //se toma como lo que ocupa de verdad, osea un bloque por eso hago esto
                if(cant_bloq == 0){
                    cant_bloq = 1;
                    log_info(logger,"Cambio de cantidad bloques 0 -> 1");
                }
                //guardo la cantidad de bloques
                archivoAGuardar->tama_archivo = cant_bloq;
                //Guardo en la lista los datos del metadata
                list_add(listaDeArchivos,archivoAGuardar);
                //Libero el path original que tenia porque guarde un strdup
                free(pathLeer);

            }
        }
    }

    closedir(dp);





//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//----------------------------------------------------------------------------------------------
//LOOP INFINITO DE ESCUCHA A KERNEL
    while (1)
    {

        int operation_code = fetch_codop(socket_kernel);
        switch (operation_code)
        {
        case CREAR_ARCHIVO:
            crearArchivo(socket_kernel);
            enviarAvisoAKernel(socket_kernel,CONFIRMACION_CREACION);
            usleep(tiempoUnidad * 1000); //Tiempo de demora por cada operacion
        break;
        case BORRAR_ARCHIVO:
            borrarArchivo(socket_kernel);
            enviarAvisoAKernel(socket_kernel,CONFIRMACION_ELIMINACION);
            usleep(tiempoUnidad * 1000);
        break;
        case TRUNCAR_ARCHIVO:
            truncarArchivo(socket_kernel);
            enviarAvisoAKernel(socket_kernel,CONFIRMACION_TRUNCAR);
            usleep(tiempoUnidad * 1000);
        break;
        case FS_WRITE:
            writeArchivo(socket_kernel);
            enviarAvisoAKernel(socket_kernel,FS_WRITE_CONFIRMACION);
            usleep(tiempoUnidad * 1000);
        break;
        case FS_READ:
            readArchivo(socket_kernel);
            enviarAvisoAKernel(socket_kernel,FS_READ_CONFIRMACION);
            usleep(tiempoUnidad * 1000);
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

void crearArchivo(int socket_kernel){
    int total_size;
    int offset = 0;
    int pid;
    void *buffer2;
  
    buffer2 = fetch_buffer(&total_size, socket_kernel);
    //Recibo el Length del nombre del archivo
    int tamaNombreArchivo;
    memcpy(&tamaNombreArchivo,buffer2 + offset, sizeof(int));
    offset += sizeof(int);

    //Recibo el nombre del archivo
    char *nombreArchivo = malloc(tamaNombreArchivo);
    memcpy(nombreArchivo,buffer2 + offset, tamaNombreArchivo);
    offset += tamaNombreArchivo;

    offset += sizeof(int);//ME SALTEO EL TAMAÑO DEL INT;

    memcpy(&pid,buffer2 + offset, sizeof(int)); //RECIBO EL PID
    offset += sizeof(int);


    char *pathNuevo = strdup(conservadorPATH);
    string_append(&pathNuevo,nombreArchivo);

    FILE *archivoACrear = fopen(pathNuevo, "w+");
    
    if (archivoACrear == NULL) {
          log_info(logger,"Error al crear el archivo BitMap.dat");
          exit(EXIT_FAILURE);
    }

    int bitLibre = buscar_bit_libre(bitarray);
    marcarBitOcupado(bitarray,bitLibre);

    //Me guardo el path de archivos y los datos de tamaños y bloque inicial
    //esto para poder usarlo para compactar
    t_archivo *archivoAGuardar = malloc(sizeof(t_archivo));
    archivoAGuardar->pathArchivo = strdup(pathNuevo);
    archivoAGuardar->bloque_inicial = bitLibre;
    //Aca pongo tamaarchivo pero esta en cantidad de bloques, no en bytes
    //Este campo dentro de la lista no lo uso nunca creo
    //pero igual hay que aclarar que dice tamaarchivo 
    //pero el tamaño no esta en bytes, esta en bloques
    archivoAGuardar->tama_archivo = 1;
    list_add(listaDeArchivos,archivoAGuardar);

    // Escribir los datos en el archivo, esto es lo que aclaracron en la consigna
    //Los archivos metadata estan para escribirles esto solamente
    fprintf(archivoACrear, "BLOQUE_INICIAL=%d\n", bitLibre);
    fprintf(archivoACrear, "TAMANIO_ARCHIVO=0\n");

    // Cerrar el archivo
    fclose(archivoACrear);


}

void borrarArchivo(int socket_kernel){

    int total_size;
    int offset = 0;
    int pid;
    void *buffer2;

    buffer2 = fetch_buffer(&total_size, socket_kernel);
    //Recibo el Length del nombre del archivo
    int tamaNombreArchivo;
    memcpy(&tamaNombreArchivo,buffer2 + offset, sizeof(int));
    offset += sizeof(int);

    //Recibo el nombre del archivo
    char *nombreArchivo = malloc(tamaNombreArchivo);
    memcpy(nombreArchivo,buffer2 + offset, tamaNombreArchivo);
    offset += tamaNombreArchivo;

    offset += sizeof(int);//ME SALTEO EL TAMAÑO DEL INT;

    memcpy(&pid,buffer2 + offset, sizeof(int)); //RECIBO EL PID
    offset += sizeof(int);

    char *pathNuevo = strdup(conservadorPATH);
    string_append(&pathNuevo,nombreArchivo);
    
    FILE *archivoMetadata = fopen(pathNuevo, "r");
    if (archivoMetadata == NULL) {
        log_error(logger,"Error al abrir el archivo de metadata");
        exit(EXIT_FAILURE);
    }

    int bloque_inicial;
    int tama_archivo;

    fscanf(archivoMetadata, "BLOQUE_INICIAL=%d\n", &bloque_inicial);
    fscanf(archivoMetadata, "TAMANIO_ARCHIVO=%d\n", &tama_archivo);

    fclose(archivoMetadata);

    liberar_bloques(bitarray, bloque_inicial, tama_archivo);

    //+++++++++++Borro el archivo de la lista de archivos+++++++++++++++++++
        //El bloque a buscar es lo que usa la funcion encontrarArchivo
        //por eso le pongo el bloqueInicial que lei del archivo, porque es el que quiero borrar
        bloqueABuscar = bloque_inicial;
        t_archivo *archivoEncontrado = list_find(listaDeArchivos,encontrarArchivo);
        if(archivoEncontrado == NULL){
            log_info(logger,"Archivo no encontrado");
        }else{
            log_info(logger,"Archivo encontrado");
            //Aca puse list_remove solamente y me rompio en una prueba
            //hay que usar remove element para pasarle el puntero
            //con remove se pasa un indice 
            list_remove_element(listaDeArchivos,archivoEncontrado);
            free(archivoEncontrado->pathArchivo);
            free(archivoEncontrado);
        }
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    if (remove(pathNuevo) == 0) {
        log_info(logger,"El archivo fue borrado");
    } else {
        log_info(logger,"Error al borrar el archivo");
    }
}

void truncarArchivo(int socket_kernel){
    int nuevoBloqueInicial;
    bool esNecesario = false; //Esto es para ver si es necesario compactar
    int total_size;
    int offset = 0;
    int pid;
    int nuevoTamaArchivo;
    void *buffer2;

    buffer2 = fetch_buffer(&total_size, socket_kernel);
    //Recibo el Length del nombre del archivo
    int tamaNombreArchivo;
    memcpy(&tamaNombreArchivo,buffer2 + offset, sizeof(int));
    offset += sizeof(int);

    //Recibo el nombre del archivo
    char *nombreArchivo = malloc(tamaNombreArchivo);
    memcpy(nombreArchivo,buffer2 + offset, tamaNombreArchivo);
    offset += tamaNombreArchivo;

    offset += sizeof(int);//ME SALTEO EL TAMAÑO DEL INT;

    memcpy(&pid,buffer2 + offset, sizeof(int)); //RECIBO EL PID
    offset += sizeof(int);

    offset += sizeof(int);//ME SALTEO EL TAMAÑO DEL INT;

    memcpy(&nuevoTamaArchivo,buffer2 + offset, sizeof(int)); //RECIBO EL NUEVO TAMAÑO DEL ARCHIVO
    offset += sizeof(int);

    char *pathNuevo = strdup(conservadorPATH);
    string_append(&pathNuevo,nombreArchivo);

    FILE *archivo = fopen(pathNuevo, "r+");

    int bloque_inicial;
    int tama_archivo;

    fscanf(archivo, "BLOQUE_INICIAL=%d\n", &bloque_inicial);
    fscanf(archivo, "TAMANIO_ARCHIVO=%d\n", &tama_archivo);

    //Mismo calculo para que se redondee al entero superior mas proximo en caso que sea con coma
    int cantidadBloques = (tama_archivo + block_size - 1) / block_size;  
    int nuevaCantidadBloques = (nuevoTamaArchivo + block_size - 1) / block_size;

    if(cantidadBloques == nuevaCantidadBloques){
        log_info(logger,"<%i> La nueva cantidad de bloques es igual a la actual",pid);
    }else if(cantidadBloques > nuevaCantidadBloques){
        log_info(logger,"<%i> La nueva cantidad de bloques es menor a la actual",pid);
        //Esta funcion solo la uso aca y es para borrar los bits que tiene de mas
        //ahora con su nuevo tamaño mas chico
        borrarUltimosbits(bitarray,cantidadBloques,nuevaCantidadBloques,bloque_inicial);
        
        //+++++++++++Actualizo la cantidad de bloques en la lista+++++++++++++++++++
            //El bloque a buscar es lo que usa la funcion encontrarArchivo
            //por eso le pongo el bloqueInicial que lei del archivo, porque es el que quiero buscar
            bloqueABuscar = bloque_inicial;
            t_archivo *archivoEncontrado = list_find(listaDeArchivos,encontrarArchivo);
            if(archivoEncontrado == NULL){
                log_info(logger,"Archivo no encontrado");
            }else{
                log_info(logger,"Actualizo la nueva cantidad de bloques");
                //Este campo dentro de la lista no lo uso nunca creo
                //pero igual hay que aclarar que dice tamaarchivo 
                //pero el tamaño no esta en bytes, esta en bloques
                archivoEncontrado->tama_archivo = nuevaCantidadBloques;
            }
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    }else{
        log_info(logger,"<%i> La nueva cantidad de bloques es mayor a la actual",pid);
        int diferencia = nuevaCantidadBloques - cantidadBloques;

        if(diferencia > bloquesLibres){
            log_info(logger,"La cantidad de bloques libres: %d es menor a los bloques nuevos que hay que agregar: %d",bloquesLibres,diferencia);
        }
        //Esto lo hago porque cuando creamos un archivo tenemos que poner que su tamaño es
        //0 y asignarle un bit inicial que lo marcamos como ocupado. Pero cuando
        //hacemos el calculo de cantidad de bloques nos da 0 y eso cambia 
        //todos los calculos de la funcion verificar necesidad compactar
        //Porque ahi se suma la cantidad de bloques para poder ver el siguiente bloque al ultimo
        //osea ver despues de donde termina, entonces como suma 0 se fija el bloque actual 
        //y como es el primero esta ocupado y marca que hay que compactar cuando no hay que hacerlo
        //entonces por eso hago esto solo en este caso para que funcione poqrue 
        //cuando pense esto tomaba como que el calculo me daba 1 porque crei que guardabamos el
        //tamaño del archivo igual a un bloque entonces esto no pasaba porque la cantidad
        //al ser nuevo era 1 y no 0
        //Este cambio funciona, pero hace algo raro, vuelve a marcar un bit
        //que esta ocupado como ocupado de vuelta, por lo que explique mas arriba
        //osea cuando se da el caso de hacer el primer truncate pasa esto. Debuggeando se ve
        //despues en la lista le asigna el nuevo tamaño igual al tamaño que ya tenia
        //hace cosas raras pero funciona y es coherente, no rompe ni desconfigura nada
        //Pero tuve que hacer esto porque en la consigna no aclaraba como guardarlo
        //y cuando lo dijeron ya lo habia implementado y la forma en que lo habia
        //pensado no servia entonces por eso hubo que hacer esto
        if(cantidadBloques == 0){
            cantidadBloques = 1;
        }
        //ACLARACION SOBRE LA COMPACTACION
        //la compactacion aca lo que hace es buscar un bit (o bloque )libre y en caso que
        //haya archivos o bits ocupados mas adelante de ese bit entonces
        //mueve todo un bloque para atras. Y asi hasta que ese bit que estaba libre
        //este ocupado. Puede ser que yo tengo un archivo que ocupa 50 bloques
        //lo trunco a 2 bloques y quizas tenia un archivo en el bloque 51. Entonces
        //voy al compactar muevo 48 veces todo hasta cubrir cerrar el hueco.
        //Una version que hubiera sido mejor, hubiese sido una funcion que busque los 
        //huecos y que por ejemplo si encuentra un bit libre se fije cuantos bits
        //libres tiene delante hasta el proximo archivo. En el ejemplo 
        //hubiera contado los 48 y hacia el movimiento de 48 bloques para atras,
        //osea movia todo para atras 48 bloques de una y no uno por uno 
        //como lo hago yo. Pero era mas complicada y como no controlan la eficiencia
        //con esta funciona, pero no es la mejor. Esa implementacion hubiese sido 
        //mucho mejor
        esNecesario = verificarNecesidadDeCompactar(bitarray,cantidadBloques,nuevaCantidadBloques,bloque_inicial);    

        if(esNecesario){
            log_info(logger,"<%i> Hay que compactar",pid);
            //Aca lo que hago es buscar el archivo por el que estamos compactando
            //lo busco en la lista y momentaneamte le cambio el bloque inicial a -1
            //solo para que no moleste en la funcion compactar (en la busqueda de archivos
            //que hay que mover para atras) en el filter de la funcion
            //mover para atras, para que no moleste ahi le pongo un -1 momentaneamente
            bloqueABuscar = bloque_inicial;
            t_archivo* archivoPorelQueCompactamos = list_find(listaDeArchivos,encontrarArchivo);
            archivoPorelQueCompactamos->bloque_inicial = -1;
            //Antes de compactar me guardo una copia del contenido de los bloques 
            //del archivo, el malloc es la cantidad de bloques por el tamaño
            //de un bloque, que me da el total en bytes. Y esa misma cantidad de bytes
            //es la que copio en el memcpy
            void *copiaContenidoBloques = malloc(cantidadBloques * block_size);
            //El segundo parametro marca el inicio, el 0 es contenidoFS y le sumo
            //el desplzamiento, que es el bloque inicial por el tamaño de cada bloque para llegar
            //al 0 del bloque (y del archivo) y desde ahi copiar
            memcpy(copiaContenidoBloques,contenidoFS + (bloque_inicial * block_size),cantidadBloques * block_size);
            //Compacto, ACLARACION SOBRE LA COMPACTACION MAS ARRIBA
            compactar(bitarray,bloque_inicial,cantidadBloques);
            //Tiempo de espera despues de la compactacion -- Esto lo pedia la consigna
            usleep(tiempoRetrasoCompactacion);
            //Despues de compactar ya le pongo el valor que tenia
            archivoPorelQueCompactamos->bloque_inicial = bloque_inicial;

            nuevoBloqueInicial = buscar_bit_libre(bitarray);
            //Ahora lo hago ocupar la nueva cantidad de bloques desde el nuevoBloqueInicial
            ocuparBits(bitarray,nuevoBloqueInicial,nuevaCantidadBloques);

            //Ahora le cargo el nuevo bloque inicial al archivo en la lista
            archivoPorelQueCompactamos->bloque_inicial = nuevoBloqueInicial;

            //Ahora que ya compacte y que tengo un nuevo Bloque inicial, copio lo que tenia y lo pongo
            //desde el nuevo bloque inicial en adelante
            //Hago el mismo calculo pero ahora cambia el orden de los paraemtros en el memcpy
            memcpy(contenidoFS + (nuevoBloqueInicial * block_size),copiaContenidoBloques,cantidadBloques * block_size);
            //Libero el void* que use para la copia
            free(copiaContenidoBloques);
        }else{
            log_info(logger,"<%i> No es necesario compactar",pid);
            marcarBitsOcupados(bitarray,bloque_inicial,cantidadBloques,(nuevaCantidadBloques - cantidadBloques));

            //+++++++++++Actualizo la cantidad de bloques en la lista+++++++++++++++++++
            //El bloque a buscar es lo que usa la funcion encontrarArchivo
            //por eso le pongo el bloqueInicial que lei del archivo, porque es el que quiero buscar
            bloqueABuscar = bloque_inicial;
            t_archivo *archivoEncontrado = list_find(listaDeArchivos,encontrarArchivo);
            if(archivoEncontrado == NULL){
                log_info(logger,"Archivo no encontrado");
            }else{
                log_info(logger,"<%i> Actualizo la nueva cantidad de bloques",pid);
                //Este campo dentro de la lista no lo uso nunca creo
                //pero igual hay que aclarar que dice tamaarchivo 
                //pero el tamaño no esta en bytes, esta en bloques
                archivoEncontrado->tama_archivo = nuevaCantidadBloques;
            }
            //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

        }

    }
    //Hago esto porque tengo que actualizar el valor del tamanio archivo
    //y para no borrar solo unalinea y eso. Trunco a 0 el archivo asi se borra
    //Todo y escribo todo de vuelta
    // Truncar el archivo al nuevo tamaño
    ftruncate(fileno(archivo), 0); // Trunca el archivo a 0 bytes, borrando su contenido

    //Aca despues de haber truncado el archivo a 0, tengo que mover el punter
    //Al inicio asi que con esto lo dejo en el incio para volver a escribir
    fseek(archivo, 0, SEEK_SET);  // Posicionar el puntero al inicio del archivo

    // Escribir los nuevos valores de metadata en el archivo
    //en caso que haya sido necesario compactar, el archivo tiene nuevo bloque inicial
    //sino solamente escribimos el anterior. Uso la misma variable bool
    //para definir en este caso
    if(esNecesario){
        fprintf(archivo, "BLOQUE_INICIAL=%d\n", nuevoBloqueInicial);
    }else{
        fprintf(archivo, "BLOQUE_INICIAL=%d\n", bloque_inicial);
    }

    fprintf(archivo, "TAMANIO_ARCHIVO=%d\n", nuevoTamaArchivo);


    //El metadata no se trunca, solo se trunca lo que ocupa el archivo como tal
    //En el bitmap y su contenido pero el metadata no porque ahi estan sus valores
    //y es como su FCB
    fclose(archivo);



}

int buscar_bit_libre(t_bitarray *bitarray){
    //Aca antes usaba esto pero lo que pasa es que por ejemplo:
    // si solo tengo un bloque, entonces eso se guardaria en un bit
    // pero no se puede crear un archivo de un bit, minimo es un byte
    // entonces por eso no puedo usar el getmaxbit, porque si solo tengo un bloque
    // eso se guardaria como un byte entero, y yo tengo que usar solo la cantidad de 
    // bloques maxima del config, entonces uso el block_count que lei del config y listo
    // porque sino tenniendo un solo bloque, el bit maximo seria 8 porque un byte tiene 8 bits
    // int cantidadMaximaDeBits = bitarray_get_max_bit(bitarray);

    for (int i = 0; i < block_count; i++) {
        //Si el bit esta libre entonces devuelve False y como eso es lo que buscamos
        //entonces por eso pongo el ! para que si da false lo haga true y entre
        if (!bitarray_test_bit(bitarray, i)) {
            return i;
        }
    }

    return -1;  // No se encontró ningún bit libre
}
void marcarBitOcupado(t_bitarray *bitarray, int bit_index){
    bitarray_set_bit(bitarray, bit_index);
    //Actualizo el contador de bloques libres
    bloquesLibres = bloquesLibres - 1;
    log_info(logger,"El bit %i fue ocupado", bit_index);
}
void marcarBitLibre(t_bitarray *bitarray, int bit_index){
    bitarray_clean_bit(bitarray, bit_index);
    //Actualizo el contador de bloques libres
    bloquesLibres = bloquesLibres + 1;
    log_info(logger,"El bit %i fue liberado", bit_index);
}
void liberar_bloques(t_bitarray *bitarray, int bloque_inicial, int tama_archivo){
    //Esta formula es para que al dividir, te asegures que siempre se redondea al 
    //al entero superior mas proximo. Se podria hacer esto o lo que hice en memoria
    //con ceil y con el casteo
    int bloques_ocupados = (tama_archivo + block_size - 1) / block_size;  

    for (int i = 0; i < bloques_ocupados; i++) {
        marcarBitLibre(bitarray, bloque_inicial + i);
    }
}
//Esta funcion solo se usa en caso que tengamos que achicar el archivo
void borrarUltimosbits(t_bitarray *bitarray,int cantidadBloques,int nuevaCantidadBloques,int bloque_inicial){
    //Aca lo que hago es calcular la diferencia entre la cantiadd de bloques actual y la nueva
    int diferencia = cantidadBloques - nuevaCantidadBloques;
    //Aca le resto a la cantidad de bloques la diferencia que calcule, por ejemplo
    //si yo tenia 6 bloques y la diferencia es 2, entonces significa que ahora voy
    //a tener 4 bloques, entonces a la cantidad de blqoues que era 6 le resto los 2 y queda
    //4. Esto lo hice para poder usar el for con i++, sino tenia que hacer -- o algo raro
    cantidadBloques = cantidadBloques - diferencia;
    //Aca en el for pongo como limite <diferencia para que solo libere
    //la cantidad de bits que vamos a achicar tomando en cuenta los bloques a borrar
    //Y para marcar el bit especifico tomo el bloque inicial, le sumo la cantidad de bloques
    //que esta variable ahora no tiene la cantidad de bloques origianl sino que tiene
    //la anterior cantidad de bloques - diferencia entonces tiene la nueva cantidad de bloques
    //porque esto solo se usa para achicar. Entonces tomando eso como base, osea 
    //bloque_inicial + cantidadBloques que seria donde termina el archivo
    //osea ahi seria el ultimo bloque, entonces de ahi empiezo a sumar i que va a ir
    //aumentando marcando la diferencia, osea cuando empiece en 1 va a marcar
    //Como libre el bit siguiente a donde termina ahora con el nuevo tamaño
    //y asi despues el que le sigue y asi hasta marcar como libre todos los que tiene
    //De mas ahora que achico su tamaño. Eso viene marcado por el limite que marca 
    //la variable diferencia
    //ACLARACION: HABIA PUESTO UN INT I = 1 Y <= PERO ESTABA MAL CALCULADO ESO Y DABA MAL
    //AL FINAL LO BORRE PERO QUIZAS QUEDO EN ALGUN COMENTARIO, ESO ESTA MAL
    for(int i = 0; i < diferencia;i++){
        int bloqueBorrar =bloque_inicial + cantidadBloques + i;
        marcarBitLibre(bitarray, bloque_inicial + cantidadBloques + i);
    }
}
bool verificarNecesidadDeCompactar(t_bitarray *bitarray,int cantidadBloques,int nuevaCantidadBloques,int bloque_inicial){
    bool esNecesario = false;
    //En este caso la nueva cantidad de bloques es mayor a la actual, por eso
    //vamos a comprobar que los bloques que le siguen esten libres para ver si es necesario
    //compactar o no. Son dos condiciones en  el segundo parametro del for.
    //La primera es que nos vamos a fijar tantos bloques delante como bloques nuevos
    //tengamos que agregar. osea que si tengo un archivo de 8 bloques actualmente
    //y lo voy a agrandar a 10 entonces me fijo si los proximos 2 bloques
    //Estan libres o no, me fijo en el bitmap. Por otro lado la segunda condicion
    //Es para que no nos pasemos de la cantidad total de bloques.
    //porque hay casos en los que el bitarray tiene mas bits que bloques realmente
    //Eso es por un redondeo y esas cosas que estan explicadas mas arriba.
    //bloque_inicial + cantidadBloques + i este calculo nos dice el bloque actual 
    //en el que estamos, por eso hago toda la suma, es para ver que no nos pasemos
    //del bloque maximo.
 
    int diferencia = nuevaCantidadBloques - cantidadBloques;
    //Creo la variable i aca porque despues del for voy a chequear si salio
    //porque no cumplio la primer condicion o porque no cumplio la segunda
    //si no cumplio la primera no entra por el if, pero si no cumplio la segunda
    //es porque queria seguir buscando un bloque vacio pero se choco con 
    //el block count es decir que no habia mas lugar para buscar
    //porque ya iba a pasar la cantidad de bloques totales, entonces en ese caso tambien
    //es necesario compactar porque es como que ocupo dos bloques y termino en el penultimo
    //y quiero expandirme 2 mas, y ponele que hay espacio antes mio
    //Pero al chequear para ver si solo puedo estirarme chequeo los que siguen, entonces
    //chequeo el ultimo y da libre entonces no entro al if, despues cuando quiero
    //volver a entrar al for para chquear el que sigue no cumplo la segunda condicion
    //porque ya me estaria pasando de la cnatidad de bloques y estaria excediendo
    //el bit array, osea que no hay mas bloques. entonces no entro al for y entonces
    //no tengo posibilidad de marcar que es necesasrio compactar. Entonces 
    //por eso despues, en el if me fijo si salio por la segunda condicion
    //en ese caso marco el esNecesario = true
    //Pongo < block count porque los bits arrancan en 0 y el block count es un contadr de bloques
    //totales, entonces no arranca en 0
    int i;
    for(i = 0; (i < diferencia) && ((bloque_inicial + cantidadBloques + i) < block_count);i++){
        
        //Si el bit esta libre entonces devuelve False 
        //el segundo parametro es el bit especifico que quiero probar
        //el calculo es el mismo que para comprobar que no me pase de total de bloques
        if(bitarray_test_bit(bitarray, bloque_inicial + cantidadBloques + i)){
            esNecesario = true;
        }
        //Si el bit esta libre entonces devuelve falso, entonces no entra al if
        //ahora cuando un bit esta ocupado devuelve true entonces entra al if
        //y ahi se marca el esNecesario = true, porque es necesario y ya con que
        //uno solo lo marque queda en true.

    }
    //Chequeo que explico mas arriba
    //Agrego un chequeo mas porque quizas se daba un caso borde que justo
    //se aumentaba la diferencia o algo raro por las dudas me fijo que 
    //realmente la suma estaba superando al block_count o igualandolo
    //no puede igualarlo por lo que explico un poco mas arriba
    if(i < diferencia && ((bloque_inicial + cantidadBloques + i) >= block_count)){
        esNecesario = true;
    }
    return esNecesario;
}
void marcarBitsOcupados(t_bitarray *bitarray,int bloque_inicial,int cantidadBloques,int diferencia){
    
    //CantidadBloques es la cantidad de bloques actual
    for(int i = 0;i < diferencia;i++){
        //Aca en el segundo parametro estoy haciendo el mismo calculo de siempre
        //para obtener la pagina
        marcarBitOcupado(bitarray,(bloque_inicial + cantidadBloques + i));
    }
}
void ocuparBits(t_bitarray *bitarray,int bloque_inicial,int cantidadBits){
    
    //Ocupo los bits desde la posicion indicada hasta la cantidad indicada
    for(int i = 0;i < cantidadBits;i++){
        marcarBitOcupado(bitarray,(bloque_inicial + i));
    }
}
void compactar(t_bitarray *bitarray,int bloque_inicial,int cantidadBloques){
    //Lo primero que hago es limpiar del bitarray a el archvo actual
    //me copio su contenido de bloques.dat, me lo guardo en algun lugar auxiliar,
    //los bits que ocupa actualmente el archivo, los pongo en 0 en el bitarray 
    //y ahora que los pongo en 0 paso el algoritmo, osea
    //busco bits libres y corro todos los bloques un bit o bloque atras
    //y dejo los libres al final. Despues ahi ya cuando tengo libre el final
    //ahi pego todo lo que tenia en la variable auxiliar.
    
    //Funcion para guardar en un void* auxiliar todo el contenido

    //Aca libero todos los bits, desde el bloque inicial hasta la cantiadd de bloques
    //del archivo
    for(int i = 0;i < cantidadBloques;i++){
        marcarBitLibre(bitarray,(bloque_inicial + i));
    }

    for (int i = 0; i < block_count; i++) {
        //Si el bit esta libre entonces devuelve False y como eso es lo que buscamos
        //entonces por eso pongo el ! para que si da false lo haga true y entre
        if (!bitarray_test_bit(bitarray, i)) {
            log_info(logger,"Bit libre encontrado en la posicion %i",i);
            //Aca lo que hago es que mientras ese bit devuelva falso osea que esta libre
            //entonces con el ! lo hago true, entonces mientras ese bit este libre
            //yo sigo haciendo lo de mover archivos para atras. si llega a entrar muchas
            //veces eso signiicara
            //que yo movi los archivos para atras pero que aun asi el bit ese sigue
            //libre porque tenia bits delante que tambien estaban libres, entonces
            //lo vuelvo a mover para atras hasta que ese bit este ocupado

            //Esto podria entrar en un loop infinito si es que delante de ese bit
            //no hay ningun archivo, entonces no vamos a estar moviendo nada para atras
            //y nunca vamos a salir de aca porque nunca se va a ocupar ese bit
            //entonces por eso hago el mismo filter que hago en la funcion mover archivos
            //para atras pero lo hago aca como una comprobacion de que hay archivos delante
            //asi de paso me ahorro entrar a la funcion en algunos casos
            
            bloqueABuscar = i;
            t_list *listaFiltrada = list_filter(listaDeArchivos,encontrarArchivoFilter);
            int cantidadArchivos = list_size(listaFiltrada);
            
            while((!bitarray_test_bit(bitarray, i)) && (cantidadArchivos > 0)){
                moverArchivosParaAtras(bitarray,i);
            }

        }else{
            log_info(logger,"Bit ocupado en la posicion %i",i);
        }

    }
}
//Esta es para el list find y encontrar por un bloque inicial en especifico
bool encontrarArchivo(void *datosArchivo){
	t_archivo *archivo = (t_archivo*) datosArchivo;
	if (archivo == NULL) {
		return false;
	}
	return archivo->bloque_inicial == bloqueABuscar;
}
//Esta es para el list filter y filtrar los que tienen un bloque incial mayor al bloque que busco
bool encontrarArchivoFilter(void *datosArchivo){
	t_archivo *archivo = (t_archivo*) datosArchivo;
	if (archivo == NULL) {
		return false;
	}
	return archivo->bloque_inicial > bloqueABuscar;
}
void moverArchivosParaAtras(t_bitarray *bitarray,int indice){
    //Aca establezco que el indice que me pasaron es el bloque desde el cual voy a buscar
    //a los archivos. Los de atras no sirven porque no los voy a mover.
    //Muevo desde el primer espacio vacio, que es el valor de Indice
    bloqueABuscar = indice;
    //PROBLEMA MUY DIFICIL DE ENCONTRAR, APARECIO EN LA PRUEBA DE FS
    //HORAS Y HORAS BUSCANDO DONDE FALLABA
    //ANTES DEL FILTER TENGO QUE ORDENAR DE MENOR A MAYOR PORQUE SE DAN CASOS EN LOS QUE
    //DEPENDIENDO COMO LOS GUARDE EN LA LISTA O COMO SE CREEN Y LAS COMPACTACIONES QUE SE
    //HAGAN ENTONCES QUIZAS APARECEN EN CUALQUIER ORDEN EN LA LISTA. Y COMO SOLO
    //HACEMOS UN FILTER ENTONCES PUEDE ESTAR EN PRIMER LUGAR EL QUE ES MAYOR PERO
    //NO EL MENOR DE LOS MAYORES, ENTONCES AHI EMPEZAMOS A PISAR COSAS QUE NO DEBERIAMOS
    //PISAR Y SE HACE UN LIO. PODRIAMOS ORDENAR, O UNA VEZ QUE HACEMOS EL FILTER
    //PODRIAMOS ELEGIR AL MENOR DE LOS DEL FILTER (CREO QUE ESO ES MAS FACIL)
    //Al final hice lo de ordenar porque con lo del minimo no sabia como iba a comparar
    //porque tenia que ir eliminando el minimo anterior y era un problema
    t_list *listaFiltrada = list_filter(listaDeArchivos,encontrarArchivoFilter);
    //Aca podria llegar a tener un problema segun el orden de listfilter
    //porque si al filtrar, a los ultimos que encuentra los pone en las primeras
    //posiciones entonces al sacar el indice 0 quizas saco el archivo que esta mas 
    //al final en el bitarray y al moverlo para atras pisaria el contenido de los que
    //estan antes y todavia no movi. Creo que no pasa pero 
    //en ese caso habria que cambiar el orden en que accedemos por indice a la lista
    //en el list get
    //AL FINAL NO PASA, LO HACE EN EL ORDEN QUE FUNCIONA, PERO PODRIA HABER PASADO
    //POR LAS DUDAS LO DEJO. QUIZAS LO GUARDABA EN OTRO ORDEN Y HUBIERA ARRUINADO TODO
    int cantidadArchivos = list_size(listaFiltrada);
    if(cantidadArchivos == 0){
        log_info(logger,"No hay archivos delante de esta posicion");
    }else{

        for(int i = 0;i < cantidadArchivos;i++){
            //Aca hay que hacer lo del memcpy
            
            //List Get empieza en 0
            //ANTES TENIA UN LIST GET PERO ESTABA EL PROBLEMA QUE QUIZAS NO ESTABA ORDENADA DEL
            //BLOQUE MAS CERCANO, OSEA ENTRE LOS MAYORES NO EMPEZABA POR EL MENOR
            //Ahora si funciona con el sort
            list_sort(listaFiltrada,(void *)archivoMasCercano);
            t_archivo *archivoActual = list_get(listaFiltrada,i);
            if(archivoActual == NULL){
                log_info(logger,"Error en el getter");
            }
            //Aca abro el archivo
            FILE *archivo = fopen(archivoActual->pathArchivo, "r+");
            //Leo los valores actuales
            int bloque_inicial;
            int tama_archivo;

            fscanf(archivo, "BLOQUE_INICIAL=%d\n", &bloque_inicial);
            fscanf(archivo, "TAMANIO_ARCHIVO=%d\n", &tama_archivo);

            ftruncate(fileno(archivo), 0); // Trunca el archivo a 0 bytes, borrando su contenido

            //Aca despues de haber truncado el archivo a 0, tengo que mover el punter
            //Al inicio asi que con esto lo dejo en el incio para volver a escribir
            fseek(archivo, 0, SEEK_SET);  // Posicionar el puntero al inicio del archivo

            // Escribir los nuevos valores de metadata en el archivo
            //Aca como estoy moviendo todo para atras, solo le resto 1 al bloque inicial
            //El tamaño sigue siendo el mismo
            fprintf(archivo, "BLOQUE_INICIAL=%d\n", (bloque_inicial - 1) );
            fprintf(archivo, "TAMANIO_ARCHIVO=%d\n", tama_archivo);
            //Cierro y guardo los cambios
            fclose(archivo);
            //Actualizo el bloque inicial en la lista de archivos
            archivoActual->bloque_inicial = bloque_inicial - 1;
            //Actualizo el bitmap
            //Primero calculo los bloques del archivo 
            int bloques = (tama_archivo + block_size - 1) / block_size;
            /*ARREGLO: ACA HAGO ESTO (ME REFIERO AL IF DE ABAJO) PORQUE FALLABA EN ESTE CASO:
            CREO 3 ARCHIVOS, Y COMO CREE LOS 3 Y NO LOS TRUNQUE, SOLO LOS CREE ENTONCES
            OCUPAN UN BLOQUE CADA UNO, Y QUIERO TRUNCAR EL DEL MEDIO EN 2 (PUEDEN SER MAS ES SOLO
            UN EJEMPLO) ENTONCES EL TECER ARCHIVO TENDRIA QUE MOVERSE A LA POSICION DEL SEGUNDO ARCHIVO
            Y EL SEGUNDO ARCHIVO A LA POSICION DEL TERCERO Y EXPANDIRSE AHI
            PERO COMO EN EL TAMAÑO ARCHIVO DICE QUE OCUPAN 0 BLOQUES
            ENTONCES AL LEER ESO DEL ARCHIVO ACA FALLABA, PORQUE EN EL CALCULO DE LOS BLOQUES
            TODO EMPIEZA POR EL TAMA_ARCHIVO Y SI ESO ES 0 ENTONCES LA VARIABLE BLOQUES DA 0
            Y YA EMPIEZA CON ERRORES AL MARCAR EL BITMAP PORQUE COMO SE ESTA SUMANDO UN 0 ENTONCES
            SE LIBERAN BLOQUES INCORRECTOS, SE LIBERABA UNO QUE YA HABIA SIDO LIBERADO OSEA EN EL EJE PLO
            SE ESTABA LIBERANDO DE VUELTA EL BLOQUE DONDE ESTABA EL SEGUNDO ARCHIVO, QUE LO LIBERAMOS ANTES DE LA
            COMPACTACION (PERO ESO LO LIBERO ANTES POR UN TEMA DEL ALGORITMO POR EL FILTER QUE HACEMOS
            PARA QUE NO INTERFIERA EN EL FILTER EL ARCHIVO POR EL CUAL ESTAMOS COMPACTANDO). ENTONCES SE MARCABA
            COMO LIBRE ALGO QUE YA ESTABA LIRE, DESPUES HABIA OTRO PROBLEMA EN EL MALLOC PORQUE COMO USA LA 
            VARIABLE BLOQUES Y COMO ERA 0 ENTONCES SE HACE UN MALLOC DE 0 DESPUES TAMBIEN EN EL MEMCPY
            HAY ERRORES PORQUE LA CANTIDAD A COPIAR SERIA 0 PORQUE BLOQUES VALE 0 Y EN EL TERCER PARAMETRO
            LA MULTIPLICACION DA 0 Y ENTONCES NO SE COPIARIA NADA.
            LO SOLUCIONAMOS IGUAL QUE SOLUCIONAMOS EL ERROR DE CANTIDAD DE BLOQUES MAS ARRIBA, 
            EN CASO QUE SEA 0 LO CAMBIAMOS A 1. PORQUE EL UNICO CASO EN EL QUE PODRIA SER 0 ES
            CUANDO RECIEN ESTA CREADO, EL TAMAÑO ES 0 PERO REALMENTE ESTA OCUPANDO 1 ENTONCES
            HACEMOS ESTO PARA QUE NO HAYA PROBLEMAS Y SOLO PARA ESE CASO. CON ESTE ARREGLO FUNCIONA
            
            ESTA PARTE DE LA CONSIGNA NO TIENE SENTIDO PORQUE DIJERON QUE APENAS ESTE CREADO EL ARCHIVO
            SE LE ASIGNA UN BLOQUE PERO TENEMOS QUE MARCAR QUE SU TAMAÑO ES 0. ENTONCES HAY UNA 
            "CONTRADICCION" PORQUE REALMENTE ESTA OCUPANDO UN BLOQUE, ENTONCES PORQUE EL TAMAÑO SERIA 0
            SI REALLMENTE SE ESTA OCUPANDO UN BLOQUE. QUIZAS DEBERIAMOS ASIGNARLE UN BLOQUE CUANDO LO TRUNQUEMOS
            Y CAMBIE DE 0, PERO BUENO ESTO GENERO UN MOTON DE ERRORES QUE TENEMOS QUE SOLUCIONARLO DE ESTA FORMA
            QUE NO ESTA TAN BUENA PERO QUIZAS SEA LA UNICA. LO PROBE TAMBIEN CON EL CONTENIDO QUE SE COPIA
            OSEA PARA VER SI SE DESPLZA BIEN EL CONTENIDO CON LOS MEMCPY Y FUNCIONA
            */
            if(bloques == 0){
                bloques = 1;
            }
            //Aca el ultimo bloque (de antes del corrimiento) pasa a estar libre ahora
            marcarBitLibre(bitarray,bloque_inicial - 1 + bloques);
            //Donde inicia ahora lo marco ocupado
            marcarBitOcupado(bitarray,bloque_inicial-1);
            //Muevo el los bytes en el archuv bloques.dat modificado el void *contenido
            //que obtuve con el mmap
            void *copiaContenido = malloc(bloques * block_size);
            //Esto es como lo que hago para guardar el contenido en la funcion que llama a esta
            memcpy(copiaContenido,contenidoFS + (bloque_inicial * block_size) ,bloques * block_size);
            //Aca copio desde el void* para copiar y lo pego en el contenidoFS pero desde
            //un bloque antes de donde estaba al principio, es moverlo un bloque para atras
            //solo inverti los parametros y le reste uno al bloque inicial, la cantiad 
            //de bytes a copiar es la misma
            memcpy(contenidoFS + ((bloque_inicial - 1)* block_size),copiaContenido,bloques * block_size);
            //Libero el void* auxiliar
            free(copiaContenido);


        }
    }
    
}

void writeArchivo(int socket_kernel){
    int total_size;
    int offset = 0;
    int pid;
    int cantidadDireccionesFisicas;
    void *buffer2;
    int registroPuntero;
    int dirFisica;
    int cantidadBytesLeer;

    buffer2 = fetch_buffer(&total_size, socket_kernel);
    int cantidadBytesMalloc;
    int desplazamientoParteLeida = 0;
    char *nombreArchivo;

    offset += sizeof(int);//ME SALTEO EL TAMAÑO DEL INT;

    memcpy(&pid,buffer2 + offset, sizeof(int)); //RECIBO EL PID
    offset += sizeof(int);

    int tamaVoid;
    memcpy(&tamaVoid,buffer2 + offset, sizeof(int)); 
    offset += sizeof(int); //Salteo el tamaño del int tamaVoid

    int lengthArchivo;
    memcpy(&lengthArchivo,buffer2 + offset, sizeof(int)); 
    offset += sizeof(int); 

    nombreArchivo = malloc(lengthArchivo);
    memcpy(nombreArchivo,buffer2 + offset, lengthArchivo);
    offset += lengthArchivo; 


    offset += sizeof(int);//ME SALTEO EL TAMAÑO DEL INT;
    memcpy(&registroPuntero,buffer2 + offset, sizeof(int)); //RECIBO EL REGISTRO PUNTERO
    offset += sizeof(int);
    
    offset += sizeof(int);//ME SALTEO EL TAMAÑO DEL INT;
    memcpy(&cantidadBytesMalloc,buffer2 + offset, sizeof(int)); //RECIBO LA CANTIDAD DE BYTES MALLOC
    offset += sizeof(int);

    offset += sizeof(int);//ME SALTEO EL TAMAÑO DEL INT DIR FISICAS;
    memcpy(&cantidadDireccionesFisicas,buffer2 + offset, sizeof(int)); //Recibo cantidad dir Fisicas
    offset += sizeof(int);

    log_info(logger,"Cantidad Direcciones fisicas %i",cantidadDireccionesFisicas);

    //+++++++++++++++Abro el archivo y leo los datos+++++++++++++++++++++++
    char *pathWrite = strdup(conservadorPATH);
    string_append(&pathWrite,nombreArchivo);

    FILE *archivo = fopen(pathWrite, "r+");

    int bloque_inicial;
    int tama_archivo;

    fscanf(archivo, "BLOQUE_INICIAL=%d\n", &bloque_inicial);
    fscanf(archivo, "TAMANIO_ARCHIVO=%d\n", &tama_archivo);

    fclose(archivo);
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

        
    //Con esto calculo donde empezaria el archivo en terminos de bytes
    //Desde donde empieza el void * que tiene almacendo bloques.dat con mmap
    //ese seria el 0 y le tengo que sumar esto para saber donde empieza 
    //el contenido de archivo
    int posicionInicioArchivo = bloque_inicial * block_size;

    //Tomo el contenidoFS que es donde empieza el void* que hice con el mmap
    //para tener el contenido de archivo en RAM. Eso lo hice al abrir bloques.dat
    //Despues le sumo la posicion donde arranca el archivo que calcule con su bloque inicial
    //Y despues le sumo el desplazamiento dentro del archivo que viene
    //Dado por registroPuntero
    void *punteroADondeTengoQueEscribirLoQueLeoDeMemoria = contenidoFS + posicionInicioArchivo + registroPuntero;


    for(int i = 0; i < cantidadDireccionesFisicas; i++){
        offset += sizeof(int); //Aca salteo el tamaño del int cantidadBytesLee
        memcpy(&cantidadBytesLeer,buffer2 + offset, sizeof(int)); 
        offset += sizeof(int);
        offset += sizeof(int);//Aca salteo el tamaño del int dirfisica
        memcpy(&dirFisica,buffer2 + offset, sizeof(int)); 
        offset += sizeof(int);
        mandarALeer(dirFisica,cantidadBytesLeer,pid,punteroADondeTengoQueEscribirLoQueLeoDeMemoria + desplazamientoParteLeida);
        desplazamientoParteLeida = desplazamientoParteLeida + cantidadBytesLeer;
    }
    //Esto no lo puedo hacer aca porque aca estamos trabajando sobre el void que nos da
    //el mmap, entonces no puedo modificarlo asi nomas como estaba haciendo 
    //con el casteo y despues poniendo el barra 0 porque estoy tocando 
    //la zona de memoria void* del mmap entonces estoy tocando el contenido del
    //archivo bloques.dat Para imprimir esto en el log y ver si esta bien tengo que hacerlo
    //de otra forma. Copiando el contenido a otro espacio que reserve solo para loggear
    //IMPLEMENTACION QUE NO FUNCIONA PORQUE MODIFICA EL ESPACIO VOID* DEL MMAP
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // //Aca creo el char* cadena para poder loggear lo que escribi en el puntero void*
    // //Que es lo que lei de memoria (para comprobar que salio bien)
    // char *cadena = (char *)punteroADondeTengoQueEscribirLoQueLeoDeMemoria;
    // //Lo pongo asi porque aca no tengo un void *conteido con su malloc
    // //Sino que estoy leyendo directamente del archivo
    // int tama = strlen(cadena);
    // cadena[cantidadBytesMalloc] = '\0'; // Asegúrate de que el string esté terminado en '\0'

    // log_info(logger,"%s",cadena);

    // free(buffer2);
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    //COMPROBACION DE QUE TODO HAYA SALIDO BIEN (IMPLEMENTACION QUE FUNCIONA)
    //podria hacer un void* pero como creo que solo van a usarlo para string lo hago asi
    //El + 1 lo pongo para poner un barra 0 al final para poder imprimirlo como string
    //Hago esto porque quizas el contenido no termina con un barra 0 entonces
    //tengo que ponerlo para poder imprimirlo como un string
    char *reservaParaLoggear = malloc(cantidadBytesMalloc + 1);
    //Copio el contenido justo, copio desde el void* del mmap osea desde el contenido
    //directo del archivo y lo guardo en la reserva que hice para loggear
    memcpy(reservaParaLoggear,punteroADondeTengoQueEscribirLoQueLeoDeMemoria,cantidadBytesMalloc);
    //Establezco el ultimo bytes con el barra 0 para poder loggearlo con %s
    reservaParaLoggear[cantidadBytesMalloc] = '\0';
    //Loggeo
    log_info(logger,"%s",reservaParaLoggear);
    //Libero la memoria que solo pedi para loggear que todo haya salido bien
    free(reservaParaLoggear);

    

}
void readArchivo(int socket_kernel){
    int total_size;
    int offset = 0;
    int pid;
    int cantidadDireccionesFisicas;
    void *buffer2;
    int dirFisica;
    int cantidadBytesEscribir;

    buffer2 = fetch_buffer(&total_size, socket_kernel);

    int parteEscrita = 0;
    char *nombreArchivo;
    int registroPuntero;
    int limiteAEscribir;

    offset += sizeof(int);//ME SALTEO EL TAMAÑO DEL INT;

    memcpy(&pid,buffer2 + offset, sizeof(int)); //RECIBO EL PID
    offset += sizeof(int);

    int tamaVoid;
    memcpy(&tamaVoid,buffer2 + offset, sizeof(int)); 
    offset += sizeof(int); //Salteo el tamaño del int tamaVoid

    int lengthArchivo;
    memcpy(&lengthArchivo,buffer2 + offset, sizeof(int)); 
    offset += sizeof(int); 

    nombreArchivo = malloc(lengthArchivo);
    memcpy(nombreArchivo,buffer2 + offset, lengthArchivo);
    offset += lengthArchivo; 

    offset += sizeof(int); //Salteo el tamaño del INT
    memcpy(&registroPuntero,buffer2 + offset,sizeof(int)); //Recibo el registro Puntero
    offset += sizeof(int);

    offset += sizeof(int); //Salteo el tamaño del INT
    memcpy(&limiteAEscribir,buffer2 + offset,sizeof(int));//Recibo la cantidad de bytes a escribir en memoria
    offset += sizeof(int);

    offset += sizeof(int); //Salteo el tamaño del INT
    memcpy(&cantidadDireccionesFisicas,buffer2 + offset,sizeof(int));//Recibo la cant Dir fisicas
    offset += sizeof(int);


    char *pathRead = strdup(conservadorPATH);
    string_append(&pathRead,nombreArchivo);

    FILE *archivo = fopen(pathRead, "r+");

    int bloque_inicial;
    int tama_archivo;

    fscanf(archivo, "BLOQUE_INICIAL=%d\n", &bloque_inicial);
    fscanf(archivo, "TAMANIO_ARCHIVO=%d\n", &tama_archivo);

    fclose(archivo);
    
    //Con esto calculo donde empezaria el archivo en terminos de bytes
    //Desde donde empieza el void * que tiene almacendo bloques.dat con mmap
    //ese seria el 0 y le tengo que sumar esto para saber donde empieza 
    //el contenido de archivo
    int posicionInicioArchivo = bloque_inicial * block_size;


    //Tomo el contenidoFS que es donde empieza el void* que hice con el mmap
    //para tener el contenido de archivo en RAM
    //Despues le sumo la posicion donde arranca el archivo que calcule con su bloque inicial
    //Y despues le sumo el desplazamiento dentro del archivo que viene
    //Dado por registroPuntero
    void *punteroALeerParaEscribirEnMemoria = contenidoFS + posicionInicioArchivo + registroPuntero;

    while((cantidadDireccionesFisicas > 0) && (parteEscrita < limiteAEscribir)){
        offset += sizeof(int);//Salteo el tamaño del int
        memcpy(&cantidadBytesEscribir,buffer2 + offset, sizeof(int)); 
        offset += sizeof(int);
        offset += sizeof(int);//Salteo el tamaño del int
        memcpy(&dirFisica,buffer2 + offset, sizeof(int)); 
        offset += sizeof(int);        
        mandarAescribirEnMemoria(dirFisica,punteroALeerParaEscribirEnMemoria + parteEscrita,cantidadBytesEscribir,pid);
        
        //Log para controlar que escribi bien
        char *contenidoEscrito = malloc(cantidadBytesEscribir + 1);
        contenidoEscrito[cantidadBytesEscribir] = '\0'; // Asegúrate de que el string esté terminado en '\0'
        memcpy(contenidoEscrito,punteroALeerParaEscribirEnMemoria + parteEscrita, cantidadBytesEscribir); 
        log_info(logger,"%s",contenidoEscrito);
        free(contenidoEscrito);
        //Actualizo la variable desplazamiento sumandole lo que acabo de escribir en memoria
        parteEscrita = parteEscrita + cantidadBytesEscribir;
    }

}

// * @fn    list_sort
// * @brief Ordena la lista segun el comparador
// *
// * @note El comparador devuelve si el primer parametro debe aparecer antes
// *       que el segundo en la lista
// */
// void list_sort(t_list *, bool (*comparator)(void *, void *));

//Funcion para ordenar lista de filtrados, si el primer archivo tiene un bloque inicial
//menor al del segundo archivo entonces tiene que estar antes
//En este caso si el bloque inicial del archivo 1 es menor al bloque incial del archivo 2 entonces
//devuelve true porque el primer parametro tiene que estar antes del segundo en la lista
bool archivoMasCercano(t_archivo *arch1, t_archivo *arch2){
    
    return arch1->bloque_inicial < arch2->bloque_inicial;
}

