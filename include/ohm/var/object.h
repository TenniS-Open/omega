//
// Created by kier on 2020/8/20.
//

#ifndef OMEGA_VAR_OBJECT_H
#define OMEGA_VAR_OBJECT_H

#include "notation.h"
#include <map>

namespace ohm {
    namespace notation {
        using Object = std::map<std::string, Element::shared>;

        using ElementObject = ElementBase<type::Object, Object>;

        template<>
        struct type_code<Object> {
            static const DataType code = type::Object;
        };

        template<>
        struct code_type<type::Object> {
            using type = ElementObject;
        };
    }
}

#endif //OMEGA_VAR_OBJECT_H
