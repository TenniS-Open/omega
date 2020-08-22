//
// Created by kier on 2020/8/20.
//

#ifndef OMEGA_VAR_NULL_H
#define OMEGA_VAR_NULL_H

#include "notation.h"

namespace ohm {
    namespace notation {
        class ElementNone : public Element {
        public:
            using self = ElementNone;
            using supper = Element;

            ElementNone() : supper({type::None}) {}

            ElementNone(std::nullptr_t) : self() {}

            operator bool() const { return false; }

            static std::shared_ptr<ElementNone> Make() {
                return std::make_shared<self>();
            }
        };

        template<>
        struct type_code<std::nullptr_t> {
            static const DataType code = type::None;
        };

        template<>
        struct code_type<type::None> {
            using type = ElementNone;
        };
    }
}

#endif //OMEGA_VAR_NULL_H
