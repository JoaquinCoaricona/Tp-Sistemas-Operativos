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
int main(int argc, char *argv[])
{   
    //ARGV ES UN VECTOR D DE TODAS LAS VARIABLES DE ENTORNO, EN LA POSICION 0 ESTA
    //LA LLAMADA AL ARCHIVO QUE EN ESTE CASO ES EL ./bin/entradasalida A PARTIR DEL 
    //1 YA ESTA LO QUE LE PASO, INT ARGC ES LA CANTIDAD DE VALORES QUE TIENE EL VECTOR
    //printf("PARAMETRO: %s\n",argv[0]);  //./bin/entradasalida
    // printf("PARAMETRO: %s\n",argv[1]); // NOMBRE DE LA INTERFAZ
    // printf("PARAMETRO: %s\n",argv[2]); // CONFIG AL QUE LO CONECTO
    
    // char *nombreInterfaz = argv[1];
    // char *configRecibido = argv[2];


    char *nombreInterfaz = "FS";
    char *configRecibido = "filesystem.config";

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
    void *contenidoBloques =  mmap(NULL,(block_count * block_size), PROT_READ | PROT_WRITE, MAP_SHARED,fileno(archivoBloques), 0);
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

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++CREACION DEL ARCHIVO BITMAP.DAT++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    string_append(&pathBitMap,"bitmap.dat");
    FILE *archivoBitMap = fopen(pathBitMap, "rb+");

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
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


    //------------------------------------------------------------------------------
    //LOOP INFINITO DE ESCUCHA A KERNEL
    while (1)
    {

        int operation_code = fetch_codop(socket_kernel);
        switch (operation_code)
        {
        case CREAR_ARCHIVO:
            crearArchivo(socket_kernel);
            enviarAvisoAKernel(socket_kernel,CONFIRMACION_CREACION);
        break;
        case BORRAR_ARCHIVO:
            borrarArchivo(socket_kernel);
            enviarAvisoAKernel(socket_kernel,CONFIRMACION_ELIMINACION);
        break;
        case TRUNCAR_ARCHIVO:
            truncarArchivo(socket_kernel);
            enviarAvisoAKernel(socket_kernel,CONFIRMACION_TRUNCAR);
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


    // Escribir los datos en el archivo, esto es lo que aclaracron en la consigna
    //Los archivos metadata estan para escribirles esto solamente
    fprintf(archivoACrear, "BLOQUE_INICIAL=%d\n", bitLibre);
    fprintf(archivoACrear, "TAMANIO_ARCHIVO=1\n");

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

    if (remove(pathNuevo) == 0) {
        log_info(logger,"El archivo fue borrado");
    } else {
        log_info(logger,"Error al borrar el archivo");
    }
}

void truncarArchivo(int socket_kernel){

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
        log_info(logger,"La nueva cantidad de bloques es igual a la actual");
    }else if(cantidadBloques > nuevaCantidadBloques){
        log_info(logger,"La nueva cantidad de bloques es menor a la actual");
        //Esta funcion solo la uso aca y es para borrar los bits que tiene de mas
        //ahora con su nuevo tamaño mas chico
        borrarUltimosbits(bitarray,cantidadBloques,nuevaCantidadBloques,bloque_inicial);
    }else{
        log_info(logger,"La nueva cantidad de bloques es mayor a la actual");
        int diferencia = nuevaCantidadBloques - cantidadBloques;

        if(diferencia > bloquesLibres){
            log_info(logger,"La cantidad de bloques libres: %d es menor a los bloques nuevos que hay que agregar: %d",bloquesLibres,diferencia);
        }
    
        bool esNecesario = verificarNecesidadDeCompactar(bitarray,cantidadBloques,nuevaCantidadBloques,bloque_inicial);    

        if(esNecesario){
            log_info(logger,"Hay que compactar");
            //compactar();
        }else{
            log_info(logger,"No es necesario compactar");
            marcarBitsOcupados(bitarray,bloque_inicial,cantidadBloques,(nuevaCantidadBloques - cantidadBloques));
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
    fprintf(archivo, "BLOQUE_INICIAL=%d\n", bloque_inicial);
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
    for(int i = 0; (i < diferencia), ((bloque_inicial + cantidadBloques + i) <= block_count);i++){
        
        //Si el bit esta libre entonces devuelve False 
        //el segundo parametro es el bit especifico que quiero probar
        //el calculo es el mismo que para comprobar que no me pase de total de bloques
        if(bitarray_test_bit(bitarray, bloque_inicial + cantidadBloques + i)){
            bool esNecesario = true;
        }
        //Si el bit esta libre entonces devuelve falso, entonces no entra al if
        //ahora cuando un bit esta ocupado devuelve true entonces entra al if
        //y ahi se marca el esNecesario = true, porque es necesario y ya con que
        //uno solo lo marque queda en true.

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
