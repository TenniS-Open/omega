//
// Created by kier on 2020/8/20.
//

#ifndef OMEGA_VAR_ARRAY_H
#define OMEGA_VAR_ARRAY_H

#include "notation.h"

namespace ohm {
    namespace notation {
        using ArrayType = std::vector<TypedField::shared>;

        using Array = Element<type::Array, ArrayType>;

        template<>
        struct type_code<ArrayType> {
            static const DataType code = type::Array;
        };

        template<>
        struct code_type<type::Array> {
            using type = Array;
        };
    }
}

#endif //OMEGA_VAR_ARRAY_H
