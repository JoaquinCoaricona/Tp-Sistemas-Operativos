#ifndef MAIN_H_
#define MAIN_H_

#include "../include/utils.h"
#include <sys/mman.h> 
int buscar_bit_libre(t_bitarray *bitarray);
void marcarBitOcupado(t_bitarray *bitarray, int bit_index);
void marcarBitLibre(t_bitarray *bitarray, int bit_index);
void truncarArchivo(int socket_kernel);
void borrarArchivo(int socket_kernel);
void crearArchivo(int socket_kernel);
void interfazFileSystem();
void mandarAescribirEnMemoria(int dirFisica,void *contenidoAescribir, int cantidadBits,int pid);
void mandarALeer(int dirFisica, int cantidadBits, int pid, void *contenido);
void recibirYejecutarDireccionesFisicasSTDIN(int socket_kernel);
void interfazStdin();
void recibirYejecutarDireccionesFisicas(int socket_kernel);
void interfazStdout();
void enviarAvisoAKernel(int socket_kernel,op_code codigo);
int fetch_tiempoDormir(int socket_kernel);
void interfazGenerica();


#endif /* MAIN_H_ */