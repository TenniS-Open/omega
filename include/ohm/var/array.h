//
// Created by kier on 2020/8/20.
//

#ifndef OMEGA_VAR_ARRAY_H
#define OMEGA_VAR_ARRAY_H

#include "notation.h"

namespace ohm {
    namespace notation {
        using Array = std::vector<Element::shared>;

        using ElementArray = ElementBase<type::Array, Array>;

        template<>
        struct type_code<Array> {
            static const DataType code = type::Array;
        };

        template<>
        struct code_type<type::Array> {
            using type = ElementArray;
        };
    }
}

#endif //OMEGA_VAR_ARRAY_H
