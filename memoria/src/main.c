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
int retardoRespuesta;
int pidBuscadoParaLiberar;
t_list* paginasBorradas;//Esto es para Limpiar la TLB
//Para evitar TLB Hits de mas en la prueba FINAL
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
    retardoRespuesta = atoi(config_get_string_value(config, "RETARDO_RESPUESTA"));

    //Creacion de Tabla De Marcos 
    cantidadMarcos = memoriaTotal/tamaPagina;
    situacionMarcos = list_create();

    //Lista para limpiar la TLB
    paginasBorradas = list_create();

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
   int numeroConfirmacion = 0; //Para limpiar la TLB, si hay reduccion cambia a 1 y entrar por un if, sino no
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
    char *pidbuscc = string_itoa(pid); //hago esto por valgrind
    tablaPaginasBuscada = dictionary_get(tabla_paginas_por_PID,pidbuscc);
    free(pidbuscc);
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
        numeroConfirmacion = 1;
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
        //Aca en caso que se haya reducido el proceso tengo que borrar algunas
        //Paginas de la TLB, osea puede ser que algunas de las paginas que se borraron
        //estaban en la TLB y tengo qeu borrarlas para que no haga HITS de mas
        //este numero lo pongo en 1 solo si hice reuduccion
        if(numeroConfirmacion == 1){
            //LE CARGO EL NUMERO DE CONFIRMACION AL PRINCIPIO DEL PAQUETE
            add_to_packet(packet_resize,&numeroConfirmacion,sizeof(int));

            int cantRegistros = list_size(paginasBorradas);
            //Agrego la cantidad de paginas borradas
            add_to_packet(packet_resize,&cantRegistros,sizeof(int));

            //Aca agrego todos los numeros de paginas que borre para limpiar la TLB
            for (int i = 0;i < cantRegistros;i++){
                //como las posiciones de la lista arrancan en 0
                //hago list get desde 0 hasta el total -1 
                t_registroTLB* registroActual = list_get(paginasBorradas,i);
                int paginaNumero = registroActual->numeroPagina;
                add_to_packet(packet_resize,&paginaNumero,sizeof(int));
            }
            //La funcion que le paso le hace free a los registros
            list_iterate(paginasBorradas,limpiarRegistrosTLB);
            //Por las dudas lo borro asi tambien
            list_clean(paginasBorradas);
        }else{
            //ACLARACION MUY IMPORTANTE, LO HAGO ACA Y TAMBIEN EN EL FFETCHCODOP
            //DE OPERACIONES.C DE LA FUNCION RESIZE. 
            //CUANDO YO CREO UN PAQUETE LE PONGO UN CODOP QUE ES COMO EL SELLO DEL SOBRE
            //ENTONCES SI YO LO MANDO SIN ENVIARLE NADA (OSEA CON BUFFER VACIO)
            //DEL OTRO LADO CUANDO YO HAGA FETCH BUFFER SE VA TRABAR Y SE VA A BLOQUEAR AHI
            //PORQUE SI BIEN AL CREAR EL PAQUETE LE ASOCIO UN BUFFER, DESPUES SI NO
            //LE HAGO NINGUN ADD TO PACKET ENTONCES EL BUFFER LLEGA VACIO Y 
            //SI LLEGA VACIO LA FUNCION FETCHBUFFER NO RECIBE NADA Y SE QUEAD BLOQUEADO AHI
            //PORQUE ESA FUNCION (PARECE, NO ESTOY SEGURO) ES BLOQUEANTE

            //ENTONCES POR ESO ACA HICE ESTE ELSE. PORQUE SI SE DABA EL CASO 
            //QUE NO ENTRABA POR EL IF, OSEA QUE NO HUBO REDUCCION ENTONCES NO SE HACIA NINGUN
            //ADD TO PACKET DE NADA Y DEL OTRO LADO SE QUEDABA BLOQUEADO EN FETCH BUFFER PORQUE
            //ENVIE UN BUFFER VACIO
            //POR ESO LE CARGO ESTO QUE ES BASURA. DEL OTRO LADO RECIBO EL NUMERO DE CONFIRMACION
            //Y SOLO SI ES IGUAL A UNO RECIBO LAS PAGINAS BORRADAS, SINO NO
            add_to_packet(packet_resize,&numeroConfirmacion,sizeof(int));
        }
    
    }else{
        packet_resize = create_packet(OUT_MEMORY, buffer_resize);
        add_to_packet(packet_resize,&numeroConfirmacion,sizeof(int)); //Agrego algo para enviar
        //por las dudas por la misma explicacion de arriba.
    }

    send_packet(packet_resize, client_socket);
    destroy_packet(packet_resize);
//---------------------------------------
}

void ampliarProceso(int pid, int cantidadAgregar){
    char *pidAmpliar = string_itoa(pid); //Hago esto por valgrind
    t_list *tablaPagina = dictionary_get(tabla_paginas_por_PID,pidAmpliar);
    free(pidAmpliar);
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
    char *pidReducir = string_itoa(pid);
    t_list *tablaPagina = dictionary_get(tabla_paginas_por_PID,pidReducir);
    free(pidReducir);

    for (int i = 0; i < cantidadReducir; i++){
        int cantPaginas = list_size(tablaPagina);
        
        //Busco la ultima pagina de la tabla del Proceso
        t_paginaMarco *paginaBuscada = list_get(tablaPagina,cantPaginas - 1);

        //Aca guardo las paginas que se borraron en una lista
        //Para enviarselas a la TLB
        t_registroTLB *guardarRegistro = malloc(sizeof(t_registroTLB));
        //Aca me guardo el cantPaginas -1 porque en las paginas 
        //empece a contar desde 0. Sino si hubiese emmpezado en 1 entonces
        //tendria que manddarlo sin el -1 pero como empece con las paginas en 
        //0 entonces lo mando asi.
        guardarRegistro->numeroPagina = cantPaginas-1;
        list_add(paginasBorradas,guardarRegistro);

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
        free(paginaBuscada); //borro por valgrind
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
    char *pidV = string_itoa(pid); //Pongo esto por valgrind
    t_list *tablaPaginas = dictionary_get(tabla_paginas_por_PID,pidV);
    free(pidV); //porque stringItoa si lo pongo directo ahi se pierde la referencia
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

    free(contenidoAescribir);

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
    free(contenido); //Borro por valgrind


}
//Solo la uso para liberar memoria de los elementos
void limpiarRegistrosTLB(void *registro){
    t_registroTLB *registroTLB = (t_registroTLB *) registro;
    free(registroTLB);
}

void liberarEstructuras(int client_socket){
    
    int pid;
    //+++++++++++++++++Recibo los datos+++++++++++++++++++
    int total_size;
    int offset = 0;
    void *buffer2;

    buffer2 = fetch_buffer(&total_size, client_socket);

    offset += sizeof(int); //Salteo el tamaño del INT
    
    memcpy(&pid,buffer2 + offset, sizeof(int)); //RECIBO EL PID
    offset += sizeof(int);
    free(buffer2);

    //Primero tengo que eliminar la estructura que lee las intrucciones y lo que guardo al 
    //crear el proceso, en el primer llamado
    pidBuscadoParaLiberar = pid;
    //Le pongo ese nombre
    //Porque en realidad es un proceso, no una instruccion. Puse mal el nombre. Lo hice al principio
    t_instrucciones *procesoBuscado = list_find(listaINSTRUCCIONES,encontrarPidALiberar);

    if(procesoBuscado != NULL){
        log_info(logger,"Se encontro el proceso para liberar estructuras");
        list_remove_element(listaINSTRUCCIONES,procesoBuscado);
        //Libero el path
        free(procesoBuscado->path);
        //Itero esa funcion en la lista, esa funcion libera cada campo de instruccion
        //unitaria. Porque tiene muchos campos
        list_iterate(procesoBuscado->lista_de_instrucciones,destroy_instuccion_unitaria);
        //Por las dudas le paso list_clean
        list_clean(procesoBuscado->lista_de_instrucciones);
        //Destruyo la lista
        list_destroy(procesoBuscado->lista_de_instrucciones);
        
        //Ahora al final le hago free al puntero que tenia este struct
        free(procesoBuscado);

    }else{
        log_info(logger,"No encontro el proceso Para liberar Estructuras Iniciales");
    }
    
    //Ahora tengo que liberar la tabla de paginas del proceso
    char *pidTablaBuscada = string_itoa(pid); //Hago esto por valgrind
    t_list *tablaDePaginasBuscada = dictionary_get(tabla_paginas_por_PID,pidTablaBuscada);
    free(pidTablaBuscada);
    if(tablaDePaginasBuscada == NULL){
        log_info(logger,"Error al buscar la tabla de paginas");
    }
    //Borro todas las estructuras t_paginaMarco que tiene dentro de la tabla
    //esta funcion que le paso al iterate, libera la estrcutura y tambien
    //libera el marco que ocupaba
    list_iterate(tablaDePaginasBuscada,destroyPaginasYLiberarMarco);
    //Borro la tabla de paginas del diccionario
    char *pidRemover = string_itoa(pid); //Hago esto por valgrind
    dictionary_remove(tabla_paginas_por_PID,pidRemover);
    free(pidRemover);
    //Hago el list_clean por las dudas aunque no se si sirve algo
    list_clean(tablaDePaginasBuscada);
    //Ahora destruyo la lista
    list_destroy(tablaDePaginasBuscada);
    

}

bool encontrarPidALiberar(void *registrado){

	t_instrucciones *procesoRegistrado = (t_instrucciones*) registrado;
	
	return procesoRegistrado->pid == pidBuscadoParaLiberar;
	
}

//Para borrar instrucciones unitarias con los campos de adentro
void destroy_instuccion_unitaria(t_instruccion_unitaria *instruccion){
    
    free(instruccion->opcode);
    if(instruccion->parametro1_lenght != 0){

        free(instruccion->parametros[0]);

          if(instruccion->parametro2_lenght != 0){
            free(instruccion->parametros[1]);
            if(instruccion->parametro3_lenght != 0){
                	free(instruccion->parametros[2]);
                    if(instruccion->parametro4_lenght != 0){
                	    free(instruccion->parametros[3]);
                        if(instruccion->parametro5_lenght != 0){
                	        free(instruccion->parametros[4]);
                            }
                    }
                }
            } 
        }
    
    free(instruccion);
    log_info(logger,"Destruyo instruccion unitaria");
}
void destroyPaginasYLiberarMarco(void *pagina){
    t_paginaMarco *paginaBorrar = (t_paginaMarco*) pagina;

    //De la tabla de Situacion de marcos busco el marco donde estaba la pagina que voy a borrar
    //Aca no hago el -1 en el indice porque los numeros de marco arrancan en 0
    //Cuando cree la lita de marcos la primera tiene su campo numeroMarco = 0
    //Por eso recomendaban usar una lista para esto para directamente buscar el marco por el indice
    t_situacion_marco *marcoBuscado = list_get(situacionMarcos,paginaBorrar->numeroMarco);
    log_info(logger,"Elimino Pagina de la lista de paginas PID: %i",marcoBuscado->pid);

    //Libero el Marco
    marcoBuscado->esLibre = true;
    marcoBuscado->pid = -1;
    //Actualizo contador de marcos libres
    cantidadMarcos = cantidadMarcos + 1;


    free(paginaBorrar);
}