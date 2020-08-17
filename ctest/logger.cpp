//
// Created by kier on 2020/7/31.
//

#include "ohm/logger.h"

int main() {
    // build logger object to control log dest
    ohm::Logger logger(ohm::LOG_INFO);

    // change log level, greater than level will output.
    // won't change error eject
    logger.change_level(ohm::LOG_DEBUG);

    // log with logger object
    ohm_log(logger(ohm::LOG_INFO), "Log [INFO] to logger");
    ohm_log(logger(ohm::LOG_DEBUG), "Log [DEBUG] to logger");

    // you can also log without logger, just print to std console
    ohm_log(ohm::LOG_INFO, "Log [INFO] just in console");
    ohm_log(ohm::LOG_DEBUG, "Log [DEBUG] just in console");

    try {
        // after log error, ohm_log will eject an EjectionException with log message
        ohm_log(logger(ohm::LOG_ERROR), "Throw EjectionException");
    } catch (const std::exception &e) {
        ohm::println("Got Exception: ", e.what());
    }

    try {
        // print error message to console, then throw EjectionException
        ohm_log(ohm::LOG_ERROR, " Not use logger log, still throw EjectionException");
    } catch (const std::exception &e) {
        ohm::println("Got Exception: ", e.what());
    }

    // print fatal message to console, the call exit(0x7E)
    ohm_log(logger(ohm::LOG_FATAL), "Program will exit with 0x7E = 126.");

    return 0;
}

