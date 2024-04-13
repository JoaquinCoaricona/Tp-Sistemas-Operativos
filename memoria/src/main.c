#include <utils/hello.h>
#include "main.h"
#include "utils/logger.c"


int main(int argc, char *argv[])
{
    t_log *logger;
    logger = initialize_logger("memoria.log", "memoria", true, LOG_LEVEL_INFO);

    log_info(logger, "Logger working");



    

    return 0;
}
