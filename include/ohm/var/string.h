//
// Created by kier on 2020/8/20.
//

#ifndef OMEGA_VAR_STRING_H
#define OMEGA_VAR_STRING_H

#include "notation.h"

#include <string>

namespace ohm {
    namespace notation {
        using String = Element<type::String, std::string>;

        template<>
        struct type_code<std::string> {
            static const DataType code = type::String;
        };
        template<>
        struct code_type<type::String> {
            using type = String;
        };
    }
}

#endif //OMEGA_VAR_STRING_H
