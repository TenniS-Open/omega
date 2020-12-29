//
// Created by Lby on 2017/10/31.
//

#ifndef OMEGA_VAR_BINARY_H
#define OMEGA_VAR_BINARY_H

#include <cstddef>
#include <memory>
#include <string>

#include "notation.h"
#include "../binary.h"

namespace ohm {
    namespace notation {
        using Binary = ohm::Binary;

        using ElementBinary = TrustElement<type::Binary, Binary>;

        template<>
        struct type_code<Binary> {
            static const DataType code = type::Binary;
        };

        template<>
        struct code_type<type::Binary> {
            using type = ElementBinary;
        };
    }
}


#endif //OMEGA_VAR_BINARY_H
