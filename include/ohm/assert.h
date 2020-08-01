//
// Created by kier on 2020/7/31.
//

#ifndef OMEGA_ASSERT_H
#define OMEGA_ASSERT_H

#include "print.h"
#include "logger.h"

#define ohm_assert(cond, log, ...) \
    if (!(cond)) do {\
        ohm::println(log, "Assertion failed: ", "(" #cond ")", \
            ", function ", __func__, \
            ", file ", __FILE__, \
            ", line ", __LINE__, \
            ".", ## __VA_ARGS__); \
    } while (false)

#endif //OMEGA_ASSERT_H
