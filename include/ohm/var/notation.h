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

            DataType type = type::Undefined;

            Element() = default;

            explicit Element(DataType type) : type(type) {}

            virtual ~Element() = default;

            bool is_undefined() const { return type >= type::Defined; }

            bool is_null() const { return (type & 0xFF00) == type::None; }

            bool is_boolean() const { return (type & 0xFF00) == type::Boolean; }

            bool is_scalar() const { return (type & 0xFF00) == type::Scalar; }

            bool is_string() const { return (type & 0xFF00) == type::String; }

            bool is_array() const { return (type & 0xFF00) == type::Array; }

            bool is_object() const { return (type & 0xFF00) == type::Object; }

            bool is_vector() const { return (type & 0xFF00) == type::Vector; }

            bool is_binary() const { return (type & 0xFF00) == type::Binary; }

            bool is_integer() const {
                auto sub_code = type & 0xFF;
                return type::INT8 <= sub_code && sub_code <= type::UINT64;
            }

            bool is_float() const {
                auto sub_code = type & 0xFF;
                return type::FLOAT32 <= sub_code && sub_code <= type::FLOAT64;
            }
        };

        template<DataType _Type, typename _Element>
        class TypeElement : public Element {
        public:
            using Content = _Element;
            _Element content;

            using self = TypeElement;
            using supper = Element;

            TypeElement() : supper(_Type) {}

            explicit TypeElement(DataType type)
                : supper(type) {}

            TypeElement(DataType type, _Element content)
                : supper(type)
                , content(std::move(content)) {}

            operator _Element() const { return content; }

            operator const _Element &() const { return content; }

            operator _Element &() { return content; }

            _Element &operator*() { return content; }

            const _Element &operator*() const { return content; }

            _Element *operator->() { return &content; }

            const _Element *operator->() const { return &content; }
        };

        template<DataType _Type, typename _Element>
        class TrustElement : public TypeElement<_Type, _Element> {
        public:
            using self = TrustElement;
            using supper = TypeElement<_Type, _Element>;

            TrustElement() = default;

            template<typename... Args, typename=typename std::enable_if<
                    std::is_constructible<_Element, Args...>::value>::type>
            TrustElement(Args &&...args)
                    : supper(_Type, _Element(std::forward<Args>(args)...)) {}

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
