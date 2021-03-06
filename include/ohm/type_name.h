//
// Created by kier on 2020/7/27.
//

#ifndef OMEGA_TYPE_NAME_H
#define OMEGA_TYPE_NAME_H


#include "platform.h"
#include <string>


#if OHM_PLATFORM_CC_GCC || OHM_PLATFORM_CC_MINGW
#include <cxxabi.h>
#include <memory>
#endif

#if OHM_PLATFORM_CC_MSVC
#include <regex>
#endif

namespace ohm {
#if OHM_PLATFORM_CC_GCC || OHM_PLATFORM_CC_MINGW
    static inline ::std::string demangle_gcc(const ::std::string &name) {
        size_t size = 0;
        int status = 0;
        std::shared_ptr<char> parsed(
                abi::__cxa_demangle(name.c_str(), nullptr, &size, &status),
                std::free);
        if (parsed.get()) {
            return std::string(parsed.get());
        } else {
            return name;
        }
    }
#endif

#if OHM_PLATFORM_CC_MSVC
    static inline ::std::string demangle_msvc(const ::std::string &name) {
		auto tmp = name;
		// ignore give default type
		static const std::regex regex_ignore(R"((?!\W|^)__cdecl(?=\W|$)|(?!\W|^)(class|struct|union|enum) )");
		tmp = std::regex_replace(tmp, regex_ignore, "");
		// ignore 64 given type
#if OHM_PLATFORM_BITS_64
		static const std::regex regex_replace_1(R"(([&\*]) __ptr64(?=[\W]|$))");
		tmp = std::regex_replace(tmp, regex_replace_1, "$1");
#endif
		return tmp;
	}
#endif

    inline ::std::string demangle(const ::std::string &name) {
#if OHM_PLATFORM_CC_MSVC
        return demangle_msvc(name);
#elif OHM_PLATFORM_CC_GCC || OHM_PLATFORM_CC_MINGW
        return demangle_gcc(name);
#else
        return name;
#endif
    }

    template <typename T>
    inline ::std::string classname() {
        static const std::string void_blank = demangle(typeid(void(bool)).name());
        auto tmp = demangle(typeid(void(T)).name());
        return tmp.substr(void_blank.size() - 5, tmp.size() + 4 - void_blank.size());
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

#endif //OMEGA_TYPE_NAME_H
