//
// Created by kier on 2020/7/31.
//

#include "ohm/logger.h"

int main() {
    ohm::Logger logger(ohm::LOG_INFO);

    ohm_log(logger(ohm::LOG_INFO), "INFO");
    ohm_log(logger(ohm::LOG_DEBUG), "DEBUG");

    try {
        ohm_log(logger(ohm::LOG_ERROR), "ERROR");
    } catch (const std::exception &e) {
        ohm::println("Got Exception: ", e.what());
    }

    try {
        ohm_log(ohm::log_error(), " Not use logger log");
    } catch (const std::exception &e) {
        ohm::println("Got Exception: ", e.what());
    }

    ohm_log(logger(ohm::LOG_FATAL), "FATAL");

    return 0;
}

