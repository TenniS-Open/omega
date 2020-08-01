//
// Created by kier on 2020/7/31.
//

#include "ohm/logger.h"

int main() {
    using namespace ohm;
    Logger logger(LOG_INFO);

    ohm_log(logger(LOG_INFO), "INFO");
    ohm_log(logger(LOG_DEBUG), "DEBUG");
    ohm_log(logger(LOG_ERROR), "ERROR");

    return 0;
}

