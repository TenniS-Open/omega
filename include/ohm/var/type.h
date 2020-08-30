//
// Created by seetadev on 2020/8/20.
//

#ifndef OMEGA_VAR_TYPE_H
#define OMEGA_VAR_TYPE_H

#include <string>

namespace ohm {
    namespace notation {
        using DataType = int;

        namespace type {
            enum MainType {
                None = 0x0100,
                Scalar = 0x0200,
                String = 0x0300,
                Boolean = 0x0400,
                Array = 0x0500,
                Object = 0x0600,

                Vector = 0x0700,
                Binary = 0x0800,

                Defined = 0x0900,

                Undefined = 0xFF00,
            };

            enum SubType {
                VOID = 0,
                INT8 = 1,
                UINT8 = 2,
                INT16 = 3,
                UINT16 = 4,
                INT32 = 5,
                UINT32 = 6,
                INT64 = 7,
                UINT64 = 8,
                FLOAT16 = 9,
                FLOAT32 = 10,
                FLOAT64 = 11,
                PTR = 12,              ///< for ptr type, with length of sizeof(void*) bytes
                CHAR8 = 13,            ///< for char saving string
                CHAR16 = 14,           ///< for char saving utf-16 string
                CHAR32 = 15,           ///< for char saving utf-32 string
                UNKNOWN8 = 16,        ///< for self define type, with length of 1 byte
                UNKNOWN16 = 17,
                UNKNOWN32 = 18,
                UNKNOWN64 = 19,
                UNKNOWN128 = 20,

                BOOLEAN = 21,    // bool type, using byte in native
                COMPLEX32 = 22,  // complex 32(16 + 16)
                COMPLEX64 = 23,  // complex 64(32 + 32)
                COMPLEX128 = 24,  // complex 128(64 + 64)
            };
        }

        template<size_t N, typename=void>
        struct placeholder;

        template<size_t N>
        struct placeholder<N, typename std::enable_if<N % 8 == 0>::type> {
            uint8_t data[N / 8];
        };

        namespace scalar {
            struct Void {
                constexpr operator bool() const { return false; }
            };

            /**
             * Use Boolean because binary define must has 1 byte.
             * Spicelly in std::vector<Boolean>
             */
            struct Boolean {
            public:
                using Type = uint8_t;
                Type data = 0;

                Boolean() = default;

                Boolean(Type b) : data(b) {}

                Boolean(bool b) : data(b ? 1 : 0) {}

                operator bool() const { return data != 0; }
            };

            namespace {
                Boolean TRUE = true;
                Boolean FALSE = false;
            }
        }

        struct float16 : public placeholder<16> {
        };
        struct complex32 : public placeholder<32> {
        };
        struct complex64 : public placeholder<64> {
        };
        struct complex128 : public placeholder<128> {
        };
        struct unknown8 : public placeholder<8> {
        };
        struct unknown16 : public placeholder<16> {
        };
        struct unknown32 : public placeholder<32> {
        };
        struct unknown64 : public placeholder<64> {
        };
        struct unknown128 : public placeholder<128> {
        };

        template<typename T>
        struct remove_cr {
            using type = typename std::remove_const<typename std::remove_reference<T>::type>::type;
        };

        template<typename T, typename=void>
        struct type_code {
        };

        template<typename T>
        struct type_code<const T, typename std::enable_if<
                std::is_integral<decltype(type_code<T>::code)>::value>::type> {
            static constexpr DataType code = type_code<T>::code;
        };

        template<typename T>
        struct type_code<const T &, typename std::enable_if<
                std::is_integral<decltype(type_code<T>::code)>::value>::type> {
            static constexpr DataType code = type_code<T>::code;
        };

        template<typename T>
        struct type_code<T &, typename std::enable_if<
                std::is_integral<decltype(type_code<T>::code)>::value>::type> {
            static constexpr DataType code = type_code<T>::code;
        };

        template<typename T>
        struct type_code<T &&, typename std::enable_if<
                std::is_integral<decltype(type_code<T>::code)>::value>::type> {
            static constexpr DataType code = type_code<T>::code;
        };

        template<typename T, typename=void>
        struct is_notation_type : public std::false_type {
        };

        template<typename T>
        struct is_notation_type<T, typename std::enable_if<
                std::is_integral<decltype(type_code<T>::code)>::value>::type> : public std::true_type {
        };

        template<DataType T>
        struct code_type;

        template<typename T, typename=void>
        struct type_type;

        template<typename T>
        struct type_type<T, typename std::enable_if<
                std::is_integral<decltype(type_code<T>::code)>::value>::type> {
            using type = typename code_type<type_code<T>::code>::type;
        };

        inline std::string sub_type_string(type::SubType type) {
            using namespace type;
            switch (type) {
                case VOID:
                    return "void";
                case INT8:
                    return "int8";
                case UINT8:
                    return "uint8";
                case INT16:
                    return "int64";
                case UINT16:
                    return "uint64";
                case INT32:
                    return "int32";
                case UINT32:
                    return "uint32";
                case INT64:
                    return "int64";
                case UINT64:
                    return "uint64";
                case FLOAT16:
                    return "float16";
                case FLOAT32:
                    return "float32";
                case FLOAT64:
                    return "float64";
                case PTR:
                    return "pointer";
                case CHAR8:
                    return "char8";
                case CHAR16:
                    return "char16";
                case CHAR32:
                    return "char32";
                case UNKNOWN8:
                    return "unknown8";
                case UNKNOWN16:
                    return "unknown16";
                case UNKNOWN32:
                    return "unknown32";
                case UNKNOWN64:
                    return "unknown64";
                case UNKNOWN128:
                    return "unknown128";
                case BOOLEAN:
                    return "bool";
                case COMPLEX32:
                    return "complex32";
                case COMPLEX64:
                    return "complex64";
                case COMPLEX128:
                    return "complex128";
            }
            return "unknown";
        }

        inline size_t sub_type_size(type::SubType type) {
            using namespace type;
            switch (type) {
                case VOID:
                    return 0;
                case INT8:
                    return 1;
                case UINT8:
                    return 1;
                case INT16:
                    return 2;
                case UINT16:
                    return 2;
                case INT32:
                    return 4;
                case UINT32:
                    return 4;
                case INT64:
                    return 8;
                case UINT64:
                    return 8;
                case FLOAT16:
                    return 2;
                case FLOAT32:
                    return 4;
                case FLOAT64:
                    return 8;
                case PTR:
                    return sizeof(decltype(std::declval<void *>()));
                case CHAR8:
                    return 1;
                case CHAR16:
                    return 2;
                case CHAR32:
                    return 4;
                case UNKNOWN8:
                    return 1;
                case UNKNOWN16:
                    return 2;
                case UNKNOWN32:
                    return 4;
                case UNKNOWN64:
                    return 8;
                case UNKNOWN128:
                    return 16;
                case BOOLEAN:
                    return 1;
                case COMPLEX32:
                    return 4;
                case COMPLEX64:
                    return 8;
                case COMPLEX128:
                    return 16;
            }
            return 0;
        }

        inline std::string type_string(DataType type) {
            switch (type & 0xFF00) {
                default:
                    return "unknown";
                case type::None:
                    return "null";
                case type::String:
                    return "string";
                case type::Boolean:
                    return "boolean";
                case type::Array:
                    return "array";
                case type::Object:
                    return "object";
                case type::Binary:
                    return "binary";
                case type::Undefined:
                    return "undefined";

                case type::Scalar:
                    return sub_type_string(type::SubType(type & 0xFF));
                case type::Vector:
                    return "vector<" + sub_type_string(type::SubType(type & 0xFF)) + ">";
            }
        }

        inline std::string main_type_string(DataType type) {
            switch (type & 0xFF00) {
                default:
                    return "unknown";
                case type::None:
                    return "null";
                case type::String:
                    return "string";
                case type::Boolean:
                    return "boolean";
                case type::Array:
                    return "array";
                case type::Object:
                    return "object";
                case type::Binary:
                    return "binary";
                case type::Undefined:
                    return "undefined";
                case type::Scalar:
                    return "scalar";
                case type::Vector:
                    return "vector";
            }
        }

        struct Empty {
            constexpr operator bool() const { return false; }
        };    // for empty data, well it will use 1 byte, but represent zero size;

        template<typename T>
        struct __element_size {
            static constexpr size_t value = sizeof(T);
        };

        template<>
        struct __element_size<Empty> {
            static constexpr size_t value = 0;
        };

        template<typename T>
        inline constexpr size_t element_size(const T &) { return __element_size<T>::value; }

        template<typename T>
        inline constexpr size_t element_size() { return __element_size<T>::value; }

        template<typename T>
        struct sub_type_code;

        template<type::SubType T>
        struct code_sub_type;

#pragma push_macro("__DEFINE_SUB_TYPE")
#define __DEFINE_SUB_TYPE(_type, _code) \
        template <> struct sub_type_code<_type> { static constexpr type::SubType code = _code; }; \
        template <> struct code_sub_type<_code> { using type = _type; };

        __DEFINE_SUB_TYPE(scalar::Void, type::VOID)
        __DEFINE_SUB_TYPE(int8_t, type::INT8)
        __DEFINE_SUB_TYPE(uint8_t, type::UINT8)
        __DEFINE_SUB_TYPE(int16_t, type::INT16)
        __DEFINE_SUB_TYPE(uint16_t, type::UINT16)
        __DEFINE_SUB_TYPE(int32_t, type::INT32)
        __DEFINE_SUB_TYPE(uint32_t, type::UINT32)
        __DEFINE_SUB_TYPE(int64_t, type::INT64)
        __DEFINE_SUB_TYPE(uint64_t, type::UINT64)
        __DEFINE_SUB_TYPE(float16, type::FLOAT16)
        __DEFINE_SUB_TYPE(float, type::FLOAT32)
        __DEFINE_SUB_TYPE(double, type::FLOAT64)
        __DEFINE_SUB_TYPE(void*, type::PTR)
        __DEFINE_SUB_TYPE(char, type::CHAR8)
        __DEFINE_SUB_TYPE(char16_t, type::CHAR16)
        __DEFINE_SUB_TYPE(char32_t, type::CHAR32)
        __DEFINE_SUB_TYPE(unknown8, type::UNKNOWN8)
        __DEFINE_SUB_TYPE(unknown16, type::UNKNOWN16)
        __DEFINE_SUB_TYPE(unknown32, type::UNKNOWN32)
        __DEFINE_SUB_TYPE(unknown64, type::UNKNOWN64)
        __DEFINE_SUB_TYPE(unknown128, type::UNKNOWN128)
        __DEFINE_SUB_TYPE(scalar::Boolean, type::BOOLEAN)
        __DEFINE_SUB_TYPE(complex32, type::COMPLEX32)
        __DEFINE_SUB_TYPE(complex64, type::COMPLEX64)
        __DEFINE_SUB_TYPE(complex128, type::COMPLEX128)
#pragma pop_macro("__DEFINE_SUB_TYPE")

        template<typename T, typename=void>
        struct is_scalar_type : public std::false_type {
        };

        template<typename T>
        struct is_scalar_type<T, typename std::enable_if<
                std::is_convertible<decltype(sub_type_code<T>::code), type::SubType>::value>::type> : public std::true_type {
        };
    }
}

#endif //OMEGA_VAR_TYPE_H
