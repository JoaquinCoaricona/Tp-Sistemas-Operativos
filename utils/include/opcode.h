/**
 * @file opcode.h 
 * @author KernelCrafters
 * @brief Operation code enum to define the operation code of the packet sent so the make some specific actions 
 * @version 1.0
 * @date 2024-04-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef OPCODE_H
#define OPCODE_H


// STRUCTS

/**
 * @brief Enum to define the operation code of the packet sent
 * 
 */
typedef enum
{   
    PCB_REC,
    MENSAJE,
    HANDSHAKE_KERNEL,
    HANDSHAKE_ENTRADA_SALIDA,
    HANDSHAKE_CPU,
    CREAR_PROCESO,
    PATH_A_MEMORIA,
    SOL_INSTRUCCION,
    MEMORIA_ENVIA_INSTRUCCION,
    
    // entrada - salida
    EJECUTAR_SCRIPT,
    INICIAR_PROCESO,
    FINALIZAR_PROCESO,
    DETENER_PLANIFICACION,
    INICIAR_PLANIFICACION,
    PROCESO_ESTADO,
    PACKET,
    INTERRUPCION,
    PETICION_CPU,
    INTERRUPCION_RTA_FALLIDA,
    INTERRUPCION_RTA_CON_PCB,
    NUEVA_INTERFAZ,
    SLEEP_IO,
    TIEMPO_DORMIR,
    CONFIRMACION_SLEEP_COMPLETO,
    MEMORIA_TERMINO_LECTURA,
    INTERRUPCION_FIN_QUANTUM,
    INTERRUPCION_ELIMINAR_PROCESO,
    RESIZE,
    RESIZE_EXITOSO,
    OUT_MEMORY,
    SOLICITAR_MARCO,
    DEVOLVER_MARCO,
    SOLICITUD_ESCRIBIR,
    CONFIRMACION_ESCRITURA,
    SOLICITUD_LECTURA,
    CONFIRMACION_LECTURA,
    STDOUT_ESCRIBIR,
    CONFIRMACION_STDOUT,

} op_code;

#endif // OPCODE_H