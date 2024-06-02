#include "main.h"

char *PATH_CONFIG;
void *espacioUsuario;
//lo reservo asi porque necesito la cadena de bytes pero no se que tipo
//de dato voy a guardar
int main(int argc, char *argv[])
{
    char *PORT;
    char *IP;
   
    int memoriaTotal;
    int tamaPagina;
    t_list *packet;
    t_log *logger;

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

    //Creacion de BitMap 
    int cantidadMarcos = memoriaTotal / tamaPagina;
    


    //Reserva Espacio Usuario
    espacioUsuario = malloc(memoriaTotal);

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


