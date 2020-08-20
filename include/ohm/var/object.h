//
// Created by kier on 2020/8/20.
//

#ifndef OMEGA_VAR_OBJECT_H
#define OMEGA_VAR_OBJECT_H

#include "notation.h"
#include <map>

namespace ohm {
    namespace notation {
        using ObjectType = std::map<std::string, TypedField::shared>;

        using Object = Element<type::Object, ObjectType>;

        template<>
        struct type_code<ObjectType> {
            static const DataType code = type::Object;
        };

        template<>
        struct code_type<type::Object> {
            using type = Object;
        };
    }
}

#endif //OMEGA_VAR_OBJECT_H
