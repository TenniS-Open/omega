//
// Created by kier on 2020/8/20.
//

#ifndef OMEGA_VAR_NULL_H
#define OMEGA_VAR_NULL_H

#include "notation.h"

namespace ohm {
    namespace notation {
        class None : public Element {
        public:
            using self = None;
            using supper = Element;

            int data[0];    // for

            None() : supper({type::None}) {}

            None(std::nullptr_t) : self() {}

            operator bool() const { return false; }

            static std::shared_ptr<None> Make() {
                return std::make_shared<self>();
            }
        };

        template<>
        struct type_code<std::nullptr_t> {
            static const DataType code = type::None;
        };

        template<>
        struct code_type<type::None> {
            using type = None;
        };
    }
}

#endif //OMEGA_VAR_NULL_H
