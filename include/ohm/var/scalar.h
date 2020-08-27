//
// Created by seetadev on 2020/8/20.
//

#ifndef OMEGA_VAR_SCALAR_H
#define OMEGA_VAR_SCALAR_H

#include "notation.h"

namespace ohm {
    namespace notation {
        class Scalar {
        public:
            char data[16];

            Scalar() { std::memset(data, 0, sizeof(data)); }

            template<typename T, typename=typename std::enable_if<
                    std::is_literal_type<T>::value &&
                    sizeof(T) <= 16>::type>
            Scalar(T t) { *reinterpret_cast<T *>(data) = t; }

            template<typename T, typename=typename std::enable_if<
                    std::is_literal_type<T>::value &&
                    sizeof(T) <= 16>::type>
            Scalar &operator=(T t) { *reinterpret_cast<T *>(data) = t; return *this; }

            template<typename T, typename=typename std::enable_if<
                    std::is_literal_type<T>::value &&
                    sizeof(T) <= 16>::type>
            operator T() const { return *reinterpret_cast<const T *>(data); }

            template <typename T>
            T *at() { return reinterpret_cast<T*>(data); }

            template <typename T>
            const T *at() const { return reinterpret_cast<const T*>(data); }

            template <typename T>
            T &ref() { return *reinterpret_cast<T*>(data); }

            template <typename T>
            const T &at() const { return *reinterpret_cast<T*>(data); }
        };

        namespace scalar {
            /**
             * Use Boolean because binary define must has 1 byte.
             * Spicelly in std::vector<Boolean>
             */
            struct Boolean {
            public:
                using Type = uint8_t;
                Type data;
            };
        }

        namespace {
            scalar::Boolean TRUE = {1};
            scalar::Boolean FALSE = {0};
        }

        class ElementScalar : public Element {
        public:
            using self = ElementScalar;
            using supper = Element;

            Scalar data;

            ElementScalar() : supper({type::Scalar}) {}

            template<typename T>
            ElementScalar(T t) : supper({type_code<T>::code}), data(t) {}

            template<typename T>
            ElementScalar &operator=(T t) {
                static constexpr auto new_code = type_code<T>::code;
                code = new_code;
                data = t;
                return *this;
            }

            template<typename T>
            operator T() const { return *reinterpret_cast<T *>(data); }

            static std::shared_ptr<ElementScalar> Make() {
                return std::make_shared<self>();
            }

            template<typename T>
            static std::shared_ptr<ElementScalar> Make(T t) {
                return std::make_shared<self>(t);
            }
        };

        template<typename T>
        class ScalarBase : public ElementScalar {
        public:
            using self = ScalarBase;
            using supper = ElementScalar;

            using Content = T;

            ScalarBase() : supper() {}

            template<typename S>
            ScalarBase(S s) : supper(s) {}
        };

#define __DEFINE_TYPE_CODE(_type, _code) \
        template <> struct type_code<_type> { static const DataType code = type::Scalar | _code; }; \
        template <> struct code_type<type::Scalar | _code> { using type = ElementScalar; }; \
        template <> struct code_type<_code> { using type = _type; };

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

        namespace scalar {
            struct Void {
            };
        }

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
            using type = ScalarBase<Empty>;
        };

        template<>
        struct code_type<type::VOID> {
            using type = Empty;
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
