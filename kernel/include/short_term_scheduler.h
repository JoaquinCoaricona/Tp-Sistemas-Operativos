#ifndef SHORT_TERM_SCHEDULER_H_
#define SHORT_TERM_SCHEDULER_H_

#include "../include/utils.h"

//FIFO
void short_term_scheduler_fifo();

//RR
void short_term_scheduler_round_robin();

//VRR
void short_term_scheduler_virtual_round_robin();

#endif //SHORT_TERM_SCHEDULER_H_