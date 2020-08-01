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
    class EjectionException : public std::logic_error {
    public:
        using self = EjectionException;
        using supper = std::logic_error;

        explicit EjectionException(const std::string &msg) : supper(msg) {}
    };
}

#endif //OMEGA_EXCEPT_H
