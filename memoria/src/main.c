#include "main.h"

char *PATH_CONFIG;
void *espacioUsuario;
//lo reservo asi porque necesito la cadena de bytes pero no se que tipo
//de dato voy a guardar
t_list* situacionMarcos;
t_dictionary* tabla_paginas_por_PID;
t_log *logger;
int memoriaDisponible;
int cantidadMarcos;
int memoriaTotal;
int tamaPagina;
int main(int argc, char *argv[])
{
    char *PORT;
    char *IP;
   
    t_list *packet;
    

    initialize_queue_and_semaphore_memoria();

    // LOGGER
    logger = initialize_logger("memoria.log", "memoria", true, LOG_LEVEL_INFO);

    // CONFIG
    //t_config *config = initialize_config(logger, "../memoria.config"); // TODO: Arreglar path en makefiles

    t_config *config = initialize_config(logger, "memoria.config");

    PORT = config_get_string_value(config, "PUERTO_ESCUCHA");
    IP = config_get_string_value(config, "IP");
    PATH_CONFIG = config_get_string_value(config, "PATH_INSTRUCCIONES");
    //Estos dos van con atoi porque esta funcion devuelve string
    memoriaTotal = atoi(config_get_string_value(config, "TAM_MEMORIA"));
    tamaPagina = atoi(config_get_string_value(config, "TAM_PAGINA"));

    //Creacion de Tabla De Marcos 
    cantidadMarcos = memoriaTotal/tamaPagina;
    situacionMarcos = list_create();

    for(int i =0; i<cantidadMarcos; i++){
		t_situacion_marco * marco_n = malloc(sizeof(t_situacion_marco));

		marco_n->numero_marco = i;
		marco_n->posicion_inicio_marco = i*tamaPagina;
		marco_n->esLibre = true;
		marco_n->pid=-1; //Pongo esto porque el pid inicial es 0
		list_add(situacionMarcos, marco_n);
	}

    //Reserva Espacio Usuario
    espacioUsuario = malloc(memoriaTotal);
    //BORRAR ESTO, esto era para probar el FS para leer sin escribir en memoria
    //igualmente habia que hacer el resize pero escribiamos esto aca directamente sin mandarlo por
    //instrucion o interfaz
    //  int tot = strlen("HOLA COMO ESTASX"); 
    //  memcpy(espacioUsuario,"HOLA COMO ESTASX",tot);

    //Asigno la memoria total a la variable memoriaDisponible
    memoriaDisponible = memoriaTotal;

    //Diccionario de las tablas de Paginas
    //Cuando se crea un proceso creo un nuevo par (key,listaTablaPagina) en el diccionario
    //Y despues cuando quiero agregar una fila a la tabla de paginas pido la lista con la llave
    tabla_paginas_por_PID = dictionary_create();

    initialize_queue_and_semaphore_memoria();

    // SERVER
    int server_fd = initialize_server(logger, "memory_server", IP, PORT);
    log_info(logger, "Server initialized");

    //leer_pseudo();

    while (1)
    {
        server_listen(logger, "memory_server", server_fd);
        
    }
    
        

    end_program(logger, config);

    return 0;
}


void resizePaginas(int client_socket){
   
//-------Declaracion Varibales y Estructuras------------   
   int nuevoTama;
   int nuevaCantidadPaginas;
   bool resizeExitoso = false; //Empieza en falso y cambia si se puede realizar
   int pid;
   int tamaActualProceso; //la cuento en cantidad de paginas
   t_list *tablaPaginasBuscada; 
//--------Recibo El Nuevo Tamaño y el Pid---------------
    int total_size;
    int offset = 0;
    void *buffer2;
    int tama; //Solo para recibir el size que esta al principio del buffer

    buffer2 = fetch_buffer(&total_size, client_socket);

    memcpy(&tama,buffer2 + offset, sizeof(int)); //RECIBO EL TAMAÑO DEL PID(UN INT)
    offset += sizeof(int);
    
    memcpy(&pid,buffer2 + offset, sizeof(int)); //RECIBO EL PID REAL
    offset += sizeof(int);

    memcpy(&tama,buffer2 + offset, sizeof(int)); //RECIBO EL TAMAÑO DEL INT
    offset += sizeof(int);
    
    memcpy(&nuevoTama,buffer2 + offset, sizeof(int)); //RECIBO EL NUEVO TAMAÑO DEL PROCESO
    offset += sizeof(int);
    
    free(buffer2);

//--------------------------------------

//--------Calculo el tamaño del proceso-----
    tablaPaginasBuscada = dictionary_get(tabla_paginas_por_PID,string_itoa(pid));
    tamaActualProceso = list_size(tablaPaginasBuscada); //Obtengo su tamaño en cantidad de paginas
//-----------------------------------------

//--------Calculo el tamaño Nuevo------
    //Hago esto porque ceil recibe un float por parametro
    float resDivision = (float)nuevoTama / tamaPagina;
    nuevaCantidadPaginas = (int)ceil(resDivision);
    //Redondeo hacia arriba para obtener la cantidad de paginas
//-------------------------------------
//--------Comparacion tamaños------
    if(nuevaCantidadPaginas == tamaActualProceso) {
        log_info(logger, "El proceso con su nuevo tamaño ocupa la misma cantidad de paginas que antes");
        resizeExitoso = true;
    }else if(nuevaCantidadPaginas > tamaActualProceso && (cantidadMarcos >= (nuevaCantidadPaginas - tamaActualProceso)) ){
        log_info(logger, "Ampliacion de Proceso PID: %i Tamaño Actual: %i Tamaño a Ampliar: %i",pid,tamaActualProceso,nuevaCantidadPaginas - tamaActualProceso);
        ampliarProceso(pid , nuevaCantidadPaginas - tamaActualProceso);
        resizeExitoso = true;
    }else if(nuevaCantidadPaginas < tamaActualProceso){
        log_info(logger, "Reduccion de Proceso PID: %i Tamaño Actual: %i Tamaño a Reducir: %i",pid,tamaActualProceso,tamaActualProceso - nuevaCantidadPaginas);
        reducirProceso(pid , tamaActualProceso - nuevaCantidadPaginas);
        resizeExitoso = true;
    }
    //La unica forma en que no entre por ninguno es que deba entrar por el segundo
    //pero que no haya marcos disponibles, entonces devuelvo out of memory
//---------------------------------------
//--------Respuesta a CPU ---------------
    t_buffer *buffer_resize;
    t_packet *packet_resize;
    buffer_resize = create_buffer();
    if(resizeExitoso){
        packet_resize = create_packet(RESIZE_EXITOSO, buffer_resize);
    }else{
        packet_resize = create_packet(OUT_MEMORY, buffer_resize);
    }
    //+++++++++++++++SOLO PARA ENVIAR ALGO+++++++++++++++++++++++++++++
    int numeroConfirmacion;
    add_to_packet(packet_resize,&numeroConfirmacion,sizeof(int));
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    send_packet(packet_resize, client_socket);
    destroy_packet(packet_resize);
//---------------------------------------
}

void ampliarProceso(int pid, int cantidadAgregar){
    t_list *tablaPagina = dictionary_get(tabla_paginas_por_PID,string_itoa(pid));

    for (int i = 0; i < cantidadAgregar ; i++){
        //Busco un marco libre y tambien creo un puntero a una variable paginaMarco
        t_situacion_marco *marcoLibre = buscarMarcoLibre();
        t_paginaMarco *nuevaPagina = malloc(sizeof(t_paginaMarco));
        
        //Completo los campos de las dos variables
        marcoLibre->esLibre = false;
        marcoLibre->pid = pid;

        nuevaPagina->numeroMarco = marcoLibre->numero_marco;
        nuevaPagina->bitValidez = 1;

        //Agrego la la pagina nueva en la tabla de paginas del PID
        list_add(tablaPagina,nuevaPagina);

        //Actualizo el contador de marcos libres
        cantidadMarcos = cantidadMarcos - 1;
        
        log_info(logger,"Agrego Pagina a la lista de paginas PID: %i",pid);
    }
}
void reducirProceso(int pid, int cantidadReducir){
    t_list *tablaPagina = dictionary_get(tabla_paginas_por_PID,string_itoa(pid));

    for (int i = 0; i < cantidadReducir; i++){
        int cantPaginas = list_size(tablaPagina);
        
        //Busco la ultima pagina de la tabla del Proceso
        t_paginaMarco *paginaBuscada = list_get(tablaPagina,cantPaginas - 1);

        //De la tabla de Situacion de marcos busco el marco donde estaba la pagina que voy a borrar
        //Aca no hago el -1 en el indice porque los numeros de marco arrancan en 0
        //Cuando cree la lita de marcos la primera tiene su campo numeroMarco = 0
        //Por eso recomendaban usar una lista para esto para directamente buscar el marco por el indice
        t_situacion_marco *marcoBuscado = list_get(situacionMarcos,paginaBuscada->numeroMarco);
        
        //Libero el Marco
        marcoBuscado->esLibre = true;
        marcoBuscado->pid = -1;

        //Actualizo contador de marcos libres
        cantidadMarcos = cantidadMarcos + 1;
        //Una vez libre el marco ya puedo borrar la pagina de la tabla del proceso
        //Borro la ultima pagina que agregue porque hay que borrar del final hacia adelante 
        list_remove(tablaPagina, cantPaginas - 1);
        log_info(logger,"Elimino Pagina de la lista de paginas PID: %i",pid);
    }
}

//Funcion Para usar en el list_find de buscarMarcoLibre
bool esMarcoLibre(void* args){
    //Hacemos esto para cumplir con el tipo de funcion que acepta list_find
	t_situacion_marco* marco_x =(t_situacion_marco*)args;
	return marco_x->esLibre;
}

//Te devuelve un marco libre
t_situacion_marco* buscarMarcoLibre(){
    t_situacion_marco *marcoLibre = list_find(situacionMarcos,esMarcoLibre);
    return marcoLibre;
}

void buscarMarco(int client_socket){

    //+++++++++++++ Declaracion Variables Necesarias ++++++++++++++++++
    int paginaBuscada;
    int pid;
    //+++++++++++++ Recibo el Numero Pagina y PID++++++++++++++++++
    int total_size;
    int offset = 0;
    void *buffer2;

    buffer2 = fetch_buffer(&total_size, client_socket);

    offset += sizeof(int); //Salteo el tamaño del INT
    
    memcpy(&paginaBuscada,buffer2 + offset, sizeof(int)); //RECIBO LA PAGINA BUSCADA 
    offset += sizeof(int);

    offset += sizeof(int); //Salteo el tamaño del INT
    
    memcpy(&pid,buffer2 + offset, sizeof(int)); //RECIBO EL PID
    offset += sizeof(int);
    
    free(buffer2);

    //+++++++++++++++++++Busqueda++++++++++++++++++++++++++++
    //Busco la tabla de paginas
    t_list *tablaPaginas = dictionary_get(tabla_paginas_por_PID,string_itoa(pid));
    //Busco Dentro de la tabla de paginas
    t_paginaMarco *paginaEncontrada = list_get(tablaPaginas,paginaBuscada);

    //Aca deberia controlar el bit de validez para devolver

    t_buffer *bufferMarco;
    t_packet *packetMarco;
    bufferMarco = create_buffer();
    packetMarco = create_packet(DEVOLVER_MARCO, bufferMarco);

    //Solo envio el numero de marco en el que esta la pagina
    add_to_packet(packetMarco,&(paginaEncontrada->numeroMarco),sizeof(int));
    
    send_packet(packetMarco, client_socket);
    destroy_packet(packetMarco);

    log_info(logger,"Acceso a Tabla de Páginas: PID: %d - Pagina: %d - Marco: %d",pid,paginaBuscada,paginaEncontrada->numeroMarco);
    void *pointer = espacioUsuario;

}

void escribirMemoria(int client_socket){
    
    //+++++++++++++++++CREO VARIABLES+++++++++++++++++++++++++++++++++++++++
    int dirFisica;
    int cantBits;
    int pid;
    void *contenidoAescribir;
    //+++++++++++++++++Recibo los datos para la escritura+++++++++++++++++++
    int total_size;
    int offset = 0;
    void *buffer2;

    buffer2 = fetch_buffer(&total_size, client_socket);

    offset += sizeof(int); //Salteo el tamaño del INT
    
    memcpy(&dirFisica,buffer2 + offset, sizeof(int)); //RECIBO LA DIRECCION FISICA
    offset += sizeof(int);


    offset += sizeof(int); //Salteo el tamaño del INT
    
    memcpy(&cantBits,buffer2 + offset, sizeof(int)); //RECIBO LA CANTIDAD DE BITS
    offset += sizeof(int);

    contenidoAescribir = malloc(cantBits);

    offset += sizeof(int); //Salteo el tamaño del INT
    
    memcpy(contenidoAescribir,buffer2 + offset, cantBits); //RECIBO EL VALOR A ESCRIBIR
    offset += cantBits;

    offset += sizeof(int); //Salteo el tamaño del INT

    memcpy(&pid,buffer2 + offset, sizeof(int)); //RECIBO EL PID
    offset += sizeof(int);

    free(buffer2);

    //+++++++++++Realizo la escritura+++++++++++++++++++++++
    memcpy(espacioUsuario+dirFisica,contenidoAescribir,cantBits);
    log_info(logger,"Acceso a espacio de usuario: PID: %d - Accion: ESCRIBIR - Direccion fisica: %d",pid,dirFisica);

    //+++++++++++Log para saber si lo que escribi esta bien+++++++++++++++++++
    char *contenidoEscrito = malloc(cantBits + 1);
    contenidoEscrito[cantBits] = '\0'; // Asegúrate de que el string esté terminado en '\0'
    memcpy(contenidoEscrito,contenidoAescribir,cantBits); 
    log_info(logger,"%s",contenidoEscrito);
    free(contenidoEscrito);

    //+++++++++++Mando confirmacion a CPU+++++++++++++++++++
    t_buffer *bufferConfirmacion;
    t_packet *packetConfirmacion;
    bufferConfirmacion = create_buffer();
    packetConfirmacion = create_packet(CONFIRMACION_ESCRITURA, bufferConfirmacion);

    add_to_packet(packetConfirmacion,&dirFisica,sizeof(int)); //Agrego solo para enviar algo y que no quede vacio

    send_packet(packetConfirmacion, client_socket);
    destroy_packet(packetConfirmacion);


}

void leerMemoria(int client_socket){
       //+++++++++++++++++CREO VARIABLES+++++++++++++++++++++++++++++++++++++++
    int dirFisica;
    int cantBits;
    int pid;
    //+++++++++++++++++Recibo los datos para la escritura+++++++++++++++++++
    int total_size;
    int offset = 0;
    void *buffer2;

    buffer2 = fetch_buffer(&total_size, client_socket);

    offset += sizeof(int); //Salteo el tamaño del INT
    
    memcpy(&dirFisica,buffer2 + offset, sizeof(int)); //RECIBO LA DIRECCION FISICA
    offset += sizeof(int);

    offset += sizeof(int); //Salteo el tamaño del INT
    
    memcpy(&cantBits,buffer2 + offset, sizeof(int)); //RECIBO LA CANTIDAD DE BITS
    offset += sizeof(int);

    offset += sizeof(int); //Salteo el tamaño del INT

    memcpy(&pid,buffer2 + offset, sizeof(int)); //RECIBO EL PID
    offset += sizeof(int);

    free(buffer2);
    //Memoria reservda Para guardar lo leido
    void* contenido = malloc(cantBits); 
    //+++++++++++Copio el contenido+++++++++++++++++++++++
    memcpy(contenido,espacioUsuario+dirFisica,cantBits);
	log_info(logger,"Acceso a espacio de usuario: “PID: %d - Accion: LEER - Direccion fisica: %d“",pid,dirFisica);

    //+++++++++++Mando lo leido a CPU+++++++++++++++++++
    t_buffer *bufferConfirmacion;
    t_packet *packetConfirmacion;
    bufferConfirmacion = create_buffer();
    packetConfirmacion = create_packet(CONFIRMACION_LECTURA, bufferConfirmacion);

    add_to_packet(packetConfirmacion,contenido,cantBits);

    send_packet(packetConfirmacion, client_socket);
    destroy_packet(packetConfirmacion);


}