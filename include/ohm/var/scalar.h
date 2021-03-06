//
// Created by seetadev on 2020/8/20.
//

#ifndef OMEGA_VAR_SCALAR_H
#define OMEGA_VAR_SCALAR_H

#include "notation.h"
#include <cstring>

namespace ohm {
    namespace notation {
        class ElementScalar : public TypeElement<type::Scalar, placeholder<128>> {
        public:
            using self = ElementScalar;
            using supper = TypeElement<type::Scalar, placeholder<128>>;

            ElementScalar() { std::memset(&content, 0, sizeof(content)); }

            template<typename T, typename=typename std::enable_if<
                    is_scalar_type<T>::value>::type>
            ElementScalar(T t)
                : supper(type::Scalar | sub_type_code<T>::code) {
                *reinterpret_cast<T *>(&content) = t;
            }

            template<typename T, typename=typename std::enable_if<
                    is_scalar_type<T>::value>::type>
            ElementScalar &operator=(T t) {
                this->type = type::Scalar | sub_type_code<T>::code;
                *reinterpret_cast<T *>(&content) = t;
                return *this;
            }

            template<typename T, typename=typename std::enable_if<
                    is_scalar_type<T>::value>::type>
            operator T() const {
                /// no check here, cause it checked outside
                return *reinterpret_cast<const T *>(&content);
            }

            template <typename T>
            T *at() { return reinterpret_cast<T*>(&content); }

            template <typename T>
            const T *at() const { return reinterpret_cast<const T*>(&content); }

            template <typename T>
            T &ref() { return *reinterpret_cast<T*>(&content); }

            template <typename T>
            const T &at() const { return *reinterpret_cast<T*>(&content); }

            static std::shared_ptr<self> Make() {
                return std::make_shared<self>();
            }

            template<typename T>
            static typename std::enable_if<
                    is_scalar_type<T>::value,
                    std::shared_ptr<self>>::type
            Make(T t) {
                return std::make_shared<self>(t);
            }
        };

#define __DEFINE_TYPE_CODE(_type, _code) \
        template <> struct type_code<_type> { static const DataType code = type::Scalar | _code; }; \
        template <> struct code_type<type::Scalar | _code> { using type = ElementScalar; };

        __DEFINE_TYPE_CODE(int8_t, type::INT8)
        __DEFINE_TYPE_CODE(uint8_t, type::UINT8)
        __DEFINE_TYPE_CODE(int16_t, type::INT16)
        __DEFINE_TYPE_CODE(uint16_t, type::UINT16)
        __DEFINE_TYPE_CODE(int32_t, type::INT32)
        __DEFINE_TYPE_CODE(uint32_t, type::UINT32)
        __DEFINE_TYPE_CODE(int64_t, type::INT64)
        __DEFINE_TYPE_CODE(uint64_t, type::UINT64)
        __DEFINE_TYPE_CODE(scalar::Boolean, type::BOOLEAN)
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

        __DEFINE_TYPE_CODE(unknown8, type::UNKNOWN8)
        __DEFINE_TYPE_CODE(unknown16, type::UNKNOWN16)
        __DEFINE_TYPE_CODE(unknown32, type::UNKNOWN32)
        __DEFINE_TYPE_CODE(unknown64, type::UNKNOWN64)
        __DEFINE_TYPE_CODE(unknown128, type::UNKNOWN128)

#undef __DEFINE_TYPE_CODE

        template<>
        struct type_code<scalar::Void> {
            static const DataType code = type::Scalar | type::VOID;
        };

        template<>
        struct type_code<Empty> {
            static const DataType code = type::Scalar | type::VOID;
        };

        template<>
        struct code_type<type::Scalar | type::VOID> {
            using type = ElementScalar;
        };

        template<>
        struct type_code<void> {
            static const DataType code = type::Scalar;
        };

        template<typename T, typename=void>
        struct other_int;

        template<typename T>
        struct other_int<T, typename std::enable_if<
                std::is_integral<T>::value &&
                std::is_signed<T>::value && sizeof(T) == 1>::type> {
            using type = int8_t;
        };

        template<typename T>
        struct other_int<T, typename std::enable_if<
                std::is_integral<T>::value &&
                std::is_unsigned<T>::value && sizeof(T) == 1>::type> {
            using type = uint8_t;
        };

        template<typename T>
        struct other_int<T, typename std::enable_if<
                std::is_integral<T>::value &&
                std::is_signed<T>::value && sizeof(T) == 2>::type> {
            using type = int16_t;
        };

        template<typename T>
        struct other_int<T, typename std::enable_if<
                std::is_integral<T>::value &&
                std::is_unsigned<T>::value && sizeof(T) == 2>::type> {
            using type = uint16_t;
        };

        template<typename T>
        struct other_int<T, typename std::enable_if<
                std::is_integral<T>::value &&
                std::is_signed<T>::value && sizeof(T) == 4>::type> {
            using type = int32_t;
        };

        template<typename T>
        struct other_int<T, typename std::enable_if<
                std::is_integral<T>::value &&
                std::is_unsigned<T>::value && sizeof(T) == 4>::type> {
            using type = uint32_t;
        };

        template<typename T>
        struct other_int<T, typename std::enable_if<
                std::is_integral<T>::value &&
                std::is_signed<T>::value && sizeof(T) == 8>::type> {
            using type = int64_t;
        };

        template<typename T>
        struct other_int<T, typename std::enable_if<
                std::is_integral<T>::value &&
                std::is_unsigned<T>::value && sizeof(T) == 8>::type> {
            using type = uint64_t;
        };
    }
}


#endif //OMEGA_VAR_SCALAR_H
