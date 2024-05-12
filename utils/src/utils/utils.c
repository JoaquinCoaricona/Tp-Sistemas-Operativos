#include "utils.h"

const char *getProcessState(enum t_process_state process) {
    switch(process) {
        case NEW:
            return "NEW";
        case READY:
            return "READY";
        case EXEC:
            return "EXEC";
        case BLOCKED:
            return "BLOCKED";
        case EXIT:
            return "EXIT";
        default:
            return "Estado no reconocido";
    }
}
