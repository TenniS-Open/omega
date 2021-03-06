//
// Created by kier on 2020/7/31.
//

#ifndef OMEGA_EXCEPT_H
#define OMEGA_EXCEPT_H

#include "platform.h"

#if OHM_PLATFORM_CC_MSVC
#define OHM_NOEXCEPT
#else
#define OHM_NOEXCEPT noexcept
#endif

namespace ohm {
    class Exception : public std::logic_error {
    public:
        using self = Exception;
        using supper = std::logic_error;

        explicit Exception(const std::string &msg) : supper(msg) {}
    };

    class EjectionException : public Exception {
    public:
        using self = EjectionException;
        using supper = Exception;

        explicit EjectionException(const std::string &msg) : supper(msg) {}
    };
}

#endif //OMEGA_EXCEPT_H
