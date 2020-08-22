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

#define __DEFINE_TYPE_CODE(_type, _code) \
        template <> struct type_code<_type> { static const DataType code = type::Scalar | _code; }; \
        template <> struct code_type<type::Scalar | _code> { using type = Scalar<_type>; };

        __DEFINE_TYPE_CODE(int8_t, type::INT8)
        __DEFINE_TYPE_CODE(uint8_t, type::UINT8)
        __DEFINE_TYPE_CODE(int16_t, type::INT16)
        __DEFINE_TYPE_CODE(uint16_t, type::UINT16)
        __DEFINE_TYPE_CODE(int32_t, type::INT32)
        __DEFINE_TYPE_CODE(uint32_t, type::UINT32)
        __DEFINE_TYPE_CODE(int64_t, type::INT64)
        __DEFINE_TYPE_CODE(uint64_t, type::UINT64)
        __DEFINE_TYPE_CODE(bool, type::BOOLEAN)
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


    }
}


#endif //OMEGA_VAR_SCALAR_H
