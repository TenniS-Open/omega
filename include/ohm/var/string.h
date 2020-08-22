//
// Created by kier on 2020/8/20.
//

#ifndef OMEGA_VAR_STRING_H
#define OMEGA_VAR_STRING_H

#include "notation.h"

#include <string>

namespace ohm {
    namespace notation {
        using String = std::string;

        using StringBase = ElementBase<type::String, String>;

        template<>
        struct type_code<std::string> {
            static const DataType code = type::String;
        };
        template<>
        struct code_type<type::String> {
            using type = StringBase;
        };
    }
}

#endif //OMEGA_VAR_STRING_H
