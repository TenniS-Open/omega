//
// Created by kier on 2020/8/27.
//

#ifndef OMEGA_VAR_VECTOR_H
#define OMEGA_VAR_VECTOR_H

#include "notation.h"

namespace ohm {
    namespace notation {
        class VectorData {
        public:
            explicit VectorData(size_t size)
                : m_data(new char[size], std::default_delete<char[]>())
                , m_size(size) {}

            void *data() { return m_data.get(); }

            const void *data() const { return m_data.get(); }

            template <typename T>
            T *data() { return reinterpret_cast<T*>(data()); }

            template <typename T>
            const T *data() const { return reinterpret_cast<const T*>(data()); }

            size_t capacity() const { return m_size; }

            template <typename T>
            T *at() { return reinterpret_cast<T*>(data()); }

            template <typename T>
            const T *at() const { return reinterpret_cast<const T*>(data()); }

            template <typename T>
            T &ref() { return *reinterpret_cast<T*>(data()); }

            template <typename T>
            const T &at() const { return *reinterpret_cast<T*>(data()); }
        private:
            std::shared_ptr<char> m_data;
            size_t m_size;
        };

        template <typename T>
        class Vector : public VectorData {
        public:
            using self = Vector;
            using supper = VectorData;

            using Content = T;

            explicit Vector(size_t size) : supper(element_size<T>() * size) {}

            T *data() { return supper::data<T>(); }

            const T *data() const { return supper::data<T>(); }

            T &operator[](size_t i) { return this->data()[i]; }

            const T &operator[](size_t i) const { return this->data()[i]; }

            template <typename I, typename=typename std::enable_if<
                    std::is_integral<I>::value &&
                    !std::is_same<I, size_t>::value>::type>
            T &operator[](I i) { return this->operator[](size_t(i)); }

            template <typename I, typename=typename std::enable_if<
                    std::is_integral<I>::value &&
                    !std::is_same<I, size_t>::value>::type>
            const T &operator[](I i) const { return this->operator[](size_t(i)); }

            size_t size() const { capacity() / sizeof(T); }
        };

        template <typename T>
        using ElementVector = ElementBase<type_code<Vector<T>>::code, Vector<T>>;

#define __DEFINE_TYPE_CODE(_type, _code) \
        template <> struct type_code<Vector<_type>> { static const DataType code = type::Vector | _code; }; \
        template <> struct code_type<type::Vector | _code> { using type = ElementVector<_type>; };

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
        struct type_code<Vector<scalar::Void>> {
            static const DataType code = type::Vector | type::VOID;
        };

        template<>
        struct type_code<Vector<Empty>> {
            static const DataType code = type::Vector | type::VOID;
        };

        template<>
        struct code_type<type::Vector | type::VOID> {
            using type = ElementVector<Empty>;
        };

        template<>
        struct type_code<Vector<void>> {
            static const DataType code = type::Scalar;
        };
    }
}

#endif //OMEGA_VAR_VECTOR_H
