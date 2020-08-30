//
// Created by kier on 2020/8/22.
//

#ifndef OMEGA_VAR_BOOLEAN_H
#define OMEGA_VAR_BOOLEAN_H

#include "notation.h"

namespace ohm {
    namespace notation {
        using Boolean = bool;

        using ElementBoolean = TrustElement<type::Boolean, Boolean>;

        template<>
        struct type_code<Boolean> {
            static const DataType code = type::Boolean;
        };

        template<>
        struct code_type<type::Boolean> {
            using type = ElementBoolean;
        };
    }
}

#endif //OMEGA_VAR_BOOLEAN_H
