#include "main.h"

char *PATH_CONFIG;
void *espacioUsuario;
//lo reservo asi porque necesito la cadena de bytes pero no se que tipo
//de dato voy a guardar
t_list* situacionMarcos;
t_dictionary* tabla_paginas_por_PID;
t_log *logger;
int memoriaDisponible;
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
    memoriaTotal = config_get_string_value(config, "TAM_MEMORIA");
    tamaPagina = config_get_string_value(config, "TAM_PAGINA");

    //Creacion de Tabla De Marcos 
    int cantidadMarcos = memoriaTotal / tamaPagina;
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
    nuevaCantidadPaginas = ceil(nuevoTama / tamaPagina);
    //Redondeo hacia arriba para obtener la cantidad de paginas
//-------------------------------------
//--------Comparacion tamaños------
    if(nuevaCantidadPaginas == tamaActualProceso) {
        log_info(logger, "El proceso con su nuevo tamaño ocupa la misma cantidad de paginas que antes");
    }else if(nuevaCantidadPaginas > tamaActualProceso){
        log_info(logger, "Ampliacion de Proceso PID: %i Tamaño Actual: %i Tamaño a Ampliar: %i",pid,tamaActualProceso,nuevaCantidadPaginas - tamaActualProceso);
        ampliarProceso(pid , nuevaCantidadPaginas - tamaActualProceso);
    }else{
        log_info(logger, "Reduccion de Proceso PID: %i Tamaño Actual: %i Tamaño a Reducir: %i",pid,tamaActualProceso,tamaActualProceso - nuevaCantidadPaginas);
        reducirProceso(pid , tamaActualProceso - nuevaCantidadPaginas);
    }
//-------------------------------------

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

        //Una vez libre el marco ya puedo borrar la pagina de la tabla del proceso
        //Borro la ultima pagina que agregue porque hay que borrar del final hacia adelante 
        list_remove(tablaPagina, cantPaginas - 1);
        log_info(logger,"Borro pagina del proceso");
    }
}

//Te devuelve un marco libre
t_situacion_marco* buscarMarcoLibre(){
    t_situacion_marco *marcoLibre = list_find(situacionMarcos,esMarcoLibre);
    return marcoLibre;
}

//Funcion Para usar en el list_find de buscarMarcoLibre
bool esMarcoLibre(void* args){
    //Hacemos esto para cumplir con el tipo de funcion que acepta list_find
	t_situacion_marco* marco_x =(t_situacion_marco*)args;
	return marco_x->esLibre;
}