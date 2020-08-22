//
// Created by kier on 2020/8/19.
//

#ifndef OMEGA_VAR_NOTATION_H
#define OMEGA_VAR_NOTATION_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <type_traits>
#include <vector>

#include "type.h"

namespace ohm {
    namespace notation {
        class Element {
        public:
            using shared = std::shared_ptr<Element>;
            using weak = std::weak_ptr<Element>;

            DataType code;

            bool is_undefined() const { return code >= type::Defined; }
            bool is_null() const { return (code & 0xFF00) == type::None; }
            bool is_scalar() const { return (code & 0xFF00) == type::Scalar; }
            bool is_string() const { return (code & 0xFF00) == type::String; }
            bool is_array() const { return (code & 0xFF00) == type::Array; }
            bool is_object() const { return (code & 0xFF00) == type::Object; }
            bool is_repeat() const { return (code & 0xFF00) == type::Repeat; }
            bool is_binary() const { return (code & 0xFF00) == type::Binary; }
        };

        template<DataType _Type, typename _Element>
        class ElementBase : public Element {
        public:
            using Type = _Element;
            _Element data;

            using self = ElementBase;
            using supper = Element;

            template<typename... Args>
            ElementBase(Args &&...args)
                    : supper({_Type}), data(std::forward<Args>(args)...) {}

            operator _Element() const { return data; }

            _Element &operator*() { return data; }

            const _Element &operator*() const { return data; }

            _Element *operator->() { return &data; }

            const _Element *operator->() const { return &data; }

            template<typename... Args>
            static
            typename std::enable_if<
                    std::is_constructible<_Element, Args...>::value,
                    std::shared_ptr<self>>::type
            Make(Args &&...args) {
                return std::make_shared<self>(std::forward<Args>(args)...);
            }
        };
    }
}

#endif //OMEGA_VAR_NOTATION_H
