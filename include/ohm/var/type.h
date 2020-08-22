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
                None = 0,
                Scalar = 0x0100,
                String = 0x0200,
                Boolean = 0x0400,
                Array = 0x0500,
                Object = 0x0600,

                Repeat = 0x0700,
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

        struct float16 : public placeholder<16> {
        };
        struct complex32 : public placeholder<32> {
        };
        struct complex64 : public placeholder<64> {
        };
        struct complex128 : public placeholder<128> {
        };


        template<typename T, typename=void>
        struct type_code {
        };

        template<typename T>
        struct type_code<const T, typename std::enable_if<
                std::is_integral<decltype(type_code<T>::code)>::value>::type>  {
            static constexpr DataType code = type_code<T>::code;
        };

        template<typename T>
        struct type_code<const T &, typename std::enable_if<
                std::is_integral<decltype(type_code<T>::code)>::value>::type>  {
            static constexpr DataType code = type_code<T>::code;
        };

        template<typename T>
        struct type_code<T &, typename std::enable_if<
                std::is_integral<decltype(type_code<T>::code)>::value>::type>  {
            static constexpr DataType code = type_code<T>::code;
        };

        template<typename T>
        struct type_code<T&&, typename std::enable_if<
                std::is_integral<decltype(type_code<T>::code)>::value>::type>  {
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


        template <typename T, typename=void>
        struct type_type;

        template <typename T>
        struct type_type<T, typename std::enable_if<
                std::is_integral<decltype(type_code<T>::code)>::value>::type> {
            using type = typename code_type<type_code<T>::code>::type;
        };

        inline std::string sub_type_string(type::SubType type) {
            using namespace type;
            switch (type) {
                case VOID: return "void";
                case INT8: return "int8";
                case UINT8: return "uint8";
                case INT16: return "int64";
                case UINT16: return "uint64";
                case INT32: return "int32";
                case UINT32: return "uint32";
                case INT64: return "int64";
                case UINT64: return "uint64";
                case FLOAT16: return "float16";
                case FLOAT32: return "float32";
                case FLOAT64: return "float64";
                case PTR: return "pointer";
                case CHAR8: return "char8";
                case CHAR16: return "char16";
                case CHAR32: return "char32";
                case UNKNOWN8: return "unknown8";
                case UNKNOWN16: return "unknown16";
                case UNKNOWN32: return "unknown32";
                case UNKNOWN64: return "unknown64";
                case UNKNOWN128: return "unknown128";
                case BOOLEAN: return "bool";
                case COMPLEX32: return "complex32";
                case COMPLEX64: return "complex64";
                case COMPLEX128: return "complex128";
            }
            return "unknown";
        }

        inline std::string type_string(DataType type) {
            switch (type & 0xFF00) {
                default: return "unknown";
                case type::None: return "null";
                case type::String: return "string";
                case type::Boolean: return "boolean";
                case type::Array: return "array";
                case type::Object: return "object";
                case type::Binary: return "binary";
                case type::Undefined: return "undefined";

                case type::Scalar: return sub_type_string(type::SubType(type & 0xFF));
                case type::Repeat: return "repeat<" + sub_type_string(type::SubType(type & 0xFF)) + ">";
            }
        }

        inline std::string main_type_string(DataType type) {
            switch (type & 0xFF00) {
                default: return "unknown";
                case type::None: return "null";
                case type::String: return "string";
                case type::Boolean: return "boolean";
                case type::Array: return "array";
                case type::Object: return "object";
                case type::Binary: return "binary";
                case type::Undefined: return "undefined";
                case type::Scalar: return "scalar";
                case type::Repeat: return "repeat";
            }
        }
    }
}

#endif //OMEGA_VAR_TYPE_H
