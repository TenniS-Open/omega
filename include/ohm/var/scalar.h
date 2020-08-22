//
// Created by seetadev on 2020/8/20.
//

#ifndef OMEGA_VAR_SCALAR_H
#define OMEGA_VAR_SCALAR_H

#include "notation.h"

namespace ohm {
    namespace notation {
        template<typename T>
        using Scalar = ElementBase<type_code<T>::code, T>;

        /**
         * Use Boolean because binary define must has 1 byte.
         * Spicelly in std::vector<Boolean>
         */
        struct Boolean {
        public:
            using Type = uint8_t;
            Type data;

            using self = Boolean;

            Boolean() : data(0) {}

            Boolean(bool b) : data(b ? 1 : 0) {}

            template <typename T, typename=typename std::enable_if<
                    std::is_integral<T>::value || std::is_floating_point<T>::value>::type>
            Boolean(T b) : data(Type(b)) {}

            Boolean(const Boolean&) = default;

            template <typename T, typename=typename std::enable_if<
                    std::is_integral<T>::value || std::is_same<T, bool>::value>::type>
            operator T() const { return T(data); }
        };

        namespace {
            Boolean TRUE = Boolean(1);
            Boolean FALSE = Boolean(0);
        }

#define __DEFINE_TYPE_CODE(_type, _code) \
        template <> struct type_code<_type> { static const DataType code = type::Scalar | _code; }; \
        template <> struct code_type<type::Scalar | _code> { using type = Scalar<_type>; }; \
        template <> struct code_type<_code> { using type = _type; };

        __DEFINE_TYPE_CODE(int8_t, type::INT8)
        __DEFINE_TYPE_CODE(uint8_t, type::UINT8)
        __DEFINE_TYPE_CODE(int16_t, type::INT16)
        __DEFINE_TYPE_CODE(uint16_t, type::UINT16)
        __DEFINE_TYPE_CODE(int32_t, type::INT32)
        __DEFINE_TYPE_CODE(uint32_t, type::UINT32)
        __DEFINE_TYPE_CODE(int64_t, type::INT64)
        __DEFINE_TYPE_CODE(uint64_t, type::UINT64)
        __DEFINE_TYPE_CODE(Boolean, type::BOOLEAN)
        __DEFINE_TYPE_CODE(float, type::FLOAT32)
        __DEFINE_TYPE_CODE(double, type::FLOAT64)
        __DEFINE_TYPE_CODE(char, type::CHAR8)
        __DEFINE_TYPE_CODE(char16_t, type::CHAR16)
        __DEFINE_TYPE_CODE(char32_t, type::CHAR32)

        __DEFINE_TYPE_CODE(void*, type::PTR)
        __DEFINE_TYPE_CODE(float16, type::FLOAT16)
        __DEFINE_TYPE_CODE(complex32, type::COMPLEX32)
        __DEFINE_TYPE_CODE(complex64, type::COMPLEX64)
        __DEFINE_TYPE_CODE(complex128, type::COMPLEX128)

#undef __DEFINE_TYPE_CODE

        struct ElementVoid {};

        template <> struct type_code<void> { static const DataType code = type::Scalar; }; \
        template <> struct code_type<type::Scalar> { using type = ElementBase<type::Scalar, ElementVoid>; };

    }
}


#endif //OMEGA_VAR_SCALAR_H
