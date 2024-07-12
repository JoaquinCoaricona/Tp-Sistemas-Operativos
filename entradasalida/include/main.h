#ifndef MAIN_H_
#define MAIN_H_

#include "../include/utils.h"
#include <sys/mman.h> 
#include <stdbool.h>

void interfazGenerica();
int fetch_tiempoDormir(int socket_kernel);
void enviarAvisoAKernel(int socket_kernel,op_code codigo);
void interfazStdout();
void recibirYejecutarDireccionesFisicas(int socket_kernel);
void interfazStdin();
void recibirYejecutarDireccionesFisicasSTDIN(int socket_kernel);
void mandarALeer(int dirFisica, int cantidadBits, int pid, void *contenido);
void mandarAescribirEnMemoria(int dirFisica,void *contenidoAescribir, int cantidadBits,int pid);
void interfazDialFS();
void crear_archivo_bloques();
void crear_archivo_bitmap();
void crear_archivo(int socket_kernel);
int primer_bit_libre(t_bitarray *bitarray);
void borrar_archivo(int socket_kernel);
bool buscar_de_lista(void *nombre_archivo);
void hacer_libre_bloques(t_bitarray *bitarray, int bloque_inicial, int bloques_ocupados);
void truncar_archivo(int socket_kernel);
bool existe_espacio_para_agrandar(t_bitarray *bitarray, int bloque_final, int cantidad_bloques_agrandar);
void verifcar_los_bits_ocupado(t_bitarray *bitarray);
void compactar(t_bitarray *bitarray, char* nombre_archivo_a_truncar, int tamano_nuevo, int tamano_actual);
void agregar_a_lista_archivos(char* path_archivos, t_list* lista_archivos);
void escribir_archivo(int socket_kernel);
void leer_archivo(int socket_kernel);


#endif /* MAIN_H_ */