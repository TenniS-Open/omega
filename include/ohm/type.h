//
// Created by kier on 2020/7/21.
//

#ifndef OMEGA_TYPE_H
#define OMEGA_TYPE_H

#include "type_callable.h"
#include "type_iterable.h"
#include "type_narrow.h"

#include "platform.h"
#include <string>


#if OHM_PLATFORM_CC_GCC
#include <cxxabi.h>
#endif

namespace ohm {
#if OHM_PLATFORM_CC_GCC
    static inline ::std::string demangle_gcc(const ::std::string &name) {
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

    inline ::std::string demangle(const ::std::string &name) {
#if OHM_PLATFORM_CC_MSVC
        return name;
#elif OHM_PLATFORM_CC_MINGW
        return name;
#elif OHM_PLATFORM_CC_GCC
        return demangle_gcc(name);
#else
        return name;
#endif
    }

    template <typename T>
    inline ::std::string classname() {
        static std::string void_blank = demangle(typeid(void()).name());
        auto tmp = demangle(typeid(void(T)).name());
        return tmp.substr(void_blank.size() - 1, tmp.size() - void_blank.size());
    }

    template <>
    inline ::std::string classname<void>() {
        return "void";
    }

    template <typename T>
    inline ::std::string classname(const T &) {
        return classname<T>();
    }
}

#endif //OMEGA_TYPE_H
