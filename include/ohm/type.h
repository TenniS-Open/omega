//
// Created by kier on 2020/7/21.
//

#ifndef OMEGA_TYPE_H
#define OMEGA_TYPE_H

#include "type_callable.h"
#include "type_iterable.h"

#include "platform.h"
#include <string>


#if OHM_PLATFORM_CC_GCC
#include <cxxabi.h>
#endif

namespace ohm {
#if OHM_PLATFORM_CC_GCC
    static inline ::std::string classname_gcc(const ::std::string &name) {
        size_t size = 0;
        int status = 0;
        char *demangled = abi::__cxa_demangle(name.c_str(), nullptr, &size, &status);
        if (demangled != nullptr) {
            ::std::string parsed = demangled;
            ::std::free(demangled);
            return parsed;
        } else {
            return name;
        }
    }
#endif

    inline ::std::string classname(const ::std::string &name) {
#if OHM_PLATFORM_CC_MSVC
        return name;
#elif OHM_PLATFORM_CC_MINGW
        return name;
#elif OHM_PLATFORM_CC_GCC
        return classname_gcc(name);
#else
        return name;
#endif
    }

    template <typename T>
    inline ::std::string classname() {
        return classname(typeid(T).name());
    }
}

#endif //OMEGA_TYPE_H
