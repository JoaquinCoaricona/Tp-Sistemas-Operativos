#ifndef UTILS_H
#define UTILS_H

// INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include "sockets.h"
#include "logger.h"
#include "configs.h"

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

#endif /* UTILS_H */