#include "recepcion.h"


void recibir_interfaz(client_socket){
    t_interfaz_registrada *interfazNueva = malloc(sizeof(t_interfaz_registrada));

    int total_size;
    int offset = 0;
   
    void *buffer;
   
    int strlen_nombre;
    buffer = fetch_buffer(&total_size, client_socket);

    memcpy(&strlen_nombre,buffer + offset, sizeof(int)); //RECIBO EL TAMAÑO DEL NOMBRE
    offset += sizeof(int);
    
    interfazNueva->nombre = malloc(strlen_nombre);
    memcpy(interfazNueva->nombre,buffer + offset, strlen_nombre); //RECIBO EL NOMBRE
    //aca arriba tener CUIDADO con recibir strings, no tengo que poner
    //&(interfazNueva->nombre) porque es un char, ya es un puntero en si
    //el & lo pongo por ejemplo ints que no neceisto su valor sino su ubicacion
    //por eso aca va sin el &
    offset += strlen_nombre;

    memcpy(&strlen_nombre,buffer + offset, sizeof(int)); //RECIBO EL TAMAÑO DEL TIPO INTERFAZ
    offset += sizeof(int);
    
    interfazNueva->tipo = malloc(strlen_nombre);
    memcpy(interfazNueva->tipo,buffer + offset, strlen_nombre); //RECIBO EL TIPO DE INTERFAZ
    offset += strlen_nombre;    

    interfazNueva->disponible = true;
    interfazNueva->socket_de_conexion = client_socket;

    list_add(listaInterfaces,interfazNueva);
    
    printf("LLEGO UNA NUEVA INTERFAZ\n");
    printf("NOMBRE DE LA INTERFAZ: %s\n",interfazNueva->nombre);
    printf("TIPO DE INTERFAZ: %s\n",interfazNueva->tipo);

    free(buffer);



    
}