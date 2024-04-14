#ifndef OPCODE_H
#define OPCODE_H


// STRUCTS

typedef enum
{
    MENSAJE,
    HANDSHAKE,
    // entrada - salida
    EJECUTAR_SCRIPT,
    INICIAR_PROCESO,
    FINALIZAR_PROCESO,
    DETENER_PLANIFICACION,
    INICIAR_PLANIFICACION,
    PROCESO_ESTADO,
    PACKET,

} op_code;

#endif // OPCODE_H