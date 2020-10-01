//
// Created by kier on 2020/9/30.
//

#ifndef OMEGA_BYTE_ORDER_H
#define OMEGA_BYTE_ORDER_H

#include <cstdint>
#include <iostream>

#ifndef OHM_IS_LITTLE_ENDIAN
#if defined(__WINDOWS__) || defined(_WIN32) || defined(WIN32) || defined(_WIN64) || \
    defined(WIN64) || defined(__WIN32__) || defined(__TOS_WIN__)
#define OHM_IS_LITTLE_ENDIAN 1
#endif
#endif

#ifndef OHM_IS_LITTLE_ENDIAN
#ifdef __BYTE_ORDER__
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define OHM_IS_LITTLE_ENDIAN 1
#else
#define OHM_IS_LITTLE_ENDIAN 0
#endif
#else
#error "Can not get byte order by __BYTE_ORDER__, use OHM_IS_LITTLE_ENDIAN to define target byte order."
#endif
#else
// defined ENDIAN
#if OHM_IS_LITTLE_ENDIAN
//#pragma message("Using OHM_IS_LITTLE_ENDIAN = 1")
#else
//#pragma message("Using OHM_IS_LITTLE_ENDIAN = 0")
#endif
#endif

namespace ohm {
    inline constexpr bool is_little_endian() {
        return OHM_IS_LITTLE_ENDIAN;
    }

    template<typename T, typename=void>
    class big_endian;

    template<typename T, typename=void>
    class little_endian;

    template<typename T>
    using host_endian = typename std::conditional<
            is_little_endian(),
            little_endian<T>, big_endian<T>>::type;

    template<typename T>
    using net_endian = big_endian<T>;

    template<typename T, typename=void>
    struct is_endian_type : public std::false_type {
    };

    template<typename T>
    struct is_endian_type<big_endian<T>,
            typename std::enable_if<std::is_integral<T>::value>::type> : public std::true_type {
    };

    template<typename T>
    struct is_endian_type<little_endian<T>,
            typename std::enable_if<std::is_integral<T>::value>::type> : public std::true_type {
    };

    namespace _ {
        template<size_t N>
        inline void byte_swap(uint8_t *a, uint8_t *b) {
            std::swap(*a, *b);
            byte_swap<N - 1>(a + 1, b - 1);
        }

        template<>
        inline void byte_swap<0>(uint8_t *, uint8_t *) {}

        template<typename T, typename=typename std::enable_if<std::is_integral<T>::value>::type>
        inline T byte_reverse(T i) {
            auto b = reinterpret_cast<uint8_t *>(&i);
            byte_swap<sizeof(T) / 2>(b, b + sizeof(T) - 1);
            return i;
        }

        template<typename T, typename=void>
        struct byte_order_helper;

        template<typename T>
        struct byte_order_helper<big_endian<T>, typename std::enable_if<
                std::is_integral<T>::value &&
                std::is_same<host_endian<T>, big_endian<T>>::value>::type> {
        public:
            static T cast(T i) { return i; }

            static void increase(T &i) { ++i; }

            static void decrease(T &i) { --i; }
        };

        template<typename T>
        struct byte_order_helper<big_endian<T>, typename std::enable_if<
                std::is_integral<T>::value &&
                !std::is_same<host_endian<T>, big_endian<T>>::value>::type> {
        public:
            static T cast(T i) { return byte_reverse(i); }

            static void increase(T &i) { i = byte_reverse(byte_reverse(i) + 1); }

            static void decrease(T &i) { i = byte_reverse(byte_reverse(i) - 1); }
        };

        template<typename T>
        struct byte_order_helper<little_endian<T>, typename std::enable_if<
                std::is_integral<T>::value &&
                std::is_same<host_endian<T>, little_endian<T>>::value>::type> {
        public:
            static T cast(T i) { return i; }

            static void increase(T &i) { ++i; }

            static void decrease(T &i) { --i; }
        };

        template<typename T>
        struct byte_order_helper<little_endian<T>, typename std::enable_if<
                std::is_integral<T>::value &&
                !std::is_same<host_endian<T>, little_endian<T>>::value>::type> {
        public:
            static T cast(T i) { return byte_reverse(i); }

            static void increase(T &i) { i = byte_reverse(byte_reverse(i) + 1); }

            static void decrease(T &i) { i = byte_reverse(byte_reverse(i) - 1); }
        };
    }

    template<typename T, typename=typename std::enable_if<std::is_integral<T>::value>::type>
    class any_endian {
    public:
        using supper = any_endian;

        using HOST = T;

        any_endian() = default;

        template<typename I, typename=typename std::enable_if<
                std::is_integral<I>::value && sizeof(I) == sizeof(T)>::type>
        any_endian(I byte) :m_byte(byte) {}

        T byte() const { return m_byte; }

        template<typename I, typename=typename std::enable_if<
                std::is_integral<I>::value && sizeof(I) == sizeof(T)>::type>
        void set_byte(I byte) { m_byte = T(byte); }

        T *addr() { return &m_byte; }

        const T *addr() const { return &m_byte; }

    protected:
        T m_byte = T(0);
    };

    template<typename T>
    class big_endian<T, typename std::enable_if<std::is_integral<T>::value>::type>
            : public any_endian<T> {
    public:
        using self = big_endian;
        using supper = any_endian<T>;

        big_endian() = default;

        big_endian(const big_endian &) = default;

        big_endian(const little_endian<T> &i)
                : supper(_::byte_reverse(i.byte())) {}

        big_endian(T i)
                : supper(_::byte_order_helper<self>::cast(i)) {}

        operator T() const {
            return _::byte_order_helper<self>::cast(this->byte());
        }

        big_endian &operator++() {
            _::byte_order_helper<self>::increase(this->m_byte);
            return *this;
        }

        big_endian operator++(int) {
            auto tmp = *this;
            _::byte_order_helper<self>::increase(this->m_byte);
            return tmp;
        }

        big_endian &operator--() {
            _::byte_order_helper<self>::decrease(this->m_byte);
            return *this;
        }

        big_endian operator--(int) {
            auto tmp = *this;
            _::byte_order_helper<self>::decrease(this->m_byte);
            return tmp;
        }

        T host() const { return this->operator T(); }

        template<typename I, typename=typename std::enable_if<
                std::is_integral<I>::value && !std::is_same<T, I>::value>::type>
        big_endian(const big_endian<I> &i)
                : self(T(I(i))) {}

        template<typename I, typename=typename std::enable_if<
                std::is_integral<I>::value && !std::is_same<T, I>::value>::type>
        big_endian(const little_endian<I> &i)
                : self(T(I(i))) {}

        template<typename I, typename=typename std::enable_if<
                std::is_integral<I>::value && !std::is_same<T, I>::value>::type>
        operator I() const { return I(T(*this)); }
    };

    template<typename T>
    class little_endian<T, typename std::enable_if<std::is_integral<T>::value>::type>
            : public any_endian<T> {
    public:
        using self = little_endian;
        using supper = any_endian<T>;

        little_endian() = default;

        little_endian(const little_endian &) = default;

        little_endian(const big_endian<T> &i)
                : supper(_::byte_reverse(i.byte())) {}

        little_endian(T i)
                : supper(_::byte_order_helper<self>::cast(i)) {}

        operator T() const {
            return _::byte_order_helper<self>::cast(this->byte());
        }

        little_endian &operator++() {
            _::byte_order_helper<self>::increase(this->m_byte);
            return *this;
        }

        little_endian operator++(int) {
            auto tmp = *this;
            _::byte_order_helper<self>::increase(this->m_byte);
            return tmp;
        }

        little_endian &operator--() {
            _::byte_order_helper<self>::decrease(this->m_byte);
            return *this;
        }

        little_endian operator--(int) {
            auto tmp = *this;
            _::byte_order_helper<self>::decrease(this->m_byte);
            return tmp;
        }

        T host() const { return this->operator T(); }

        template<typename I, typename=typename std::enable_if<
                std::is_integral<I>::value && !std::is_same<T, I>::value>::type>
        little_endian(const big_endian<I> &i)
                : self(T(I(i))) {}

        template<typename I, typename=typename std::enable_if<
                std::is_integral<I>::value && !std::is_same<T, I>::value>::type>
        little_endian(const little_endian<I> &i)
                : self(T(I(i))) {}

        template<typename I, typename=typename std::enable_if<
                std::is_integral<I>::value && !std::is_same<T, I>::value>::type>
        operator I() const { return I(T(*this)); }
    };

    template<typename T>
    inline std::ostream &operator<<(std::ostream &out, const big_endian<T> &i) {
        return out << T(i);
    }

    template<typename T>
    inline std::ostream &operator<<(std::ostream &out, const little_endian<T> &i) {
        return out << T(i);
    }

#pragma push_macro("DEFINE_BYTE_ORDER_BINARY_OP")
#pragma push_macro("DEFINE_BYTE_ORDER_ASSIGN_OP")

#define DEFINE_BYTE_ORDER_BINARY_OP(op) \
    template <typename T1, typename T2> inline decltype(std::declval<T1>() op std::declval<T2>()) \
    operator op(const big_endian<T1> &lhs, const big_endian<T2> &rhs) { return lhs.host() op rhs.host(); } \
    template <typename T1, typename T2> inline decltype(std::declval<T1>() op std::declval<T2>()) \
    operator op(const big_endian<T1> &lhs, const little_endian<T2> &rhs) { return lhs.host() op rhs.host(); } \
    template <typename T1, typename T2> inline decltype(std::declval<T1>() op std::declval<T2>()) \
    operator op(const little_endian<T1> &lhs, const little_endian<T2> &rhs) { return lhs.host() op rhs.host(); } \
    template <typename T1, typename T2> inline decltype(std::declval<T1>() op std::declval<T2>()) \
    operator op(const little_endian<T1> &lhs, const big_endian<T2> &rhs) { return lhs.host() op rhs.host(); } \
    template <typename T, typename I, typename=typename std::enable_if<std::is_integral<I>::value>::type> \
    inline decltype(std::declval<T>() op std::declval<I>()) \
    operator op(const big_endian<T> &lhs, I rhs) { return lhs.host() op rhs; } \
    template <typename T, typename I, typename=typename std::enable_if<std::is_integral<I>::value>::type> \
    inline decltype(std::declval<T>() op std::declval<I>()) \
    operator op(I lhs, const big_endian<T> &rhs) { return lhs op rhs.host(); } \
    template <typename T, typename I, typename=typename std::enable_if<std::is_integral<I>::value>::type> \
    inline decltype(std::declval<T>() op std::declval<I>()) \
    operator op(const little_endian<T> &lhs, I rhs) { return lhs.host() op rhs; } \
    template <typename T, typename I, typename=typename std::enable_if<std::is_integral<I>::value>::type> \
    inline decltype(std::declval<T>() op std::declval<I>()) \
    operator op(I lhs, const little_endian<T> &rhs) { return lhs op rhs.host(); }

#define DEFINE_BYTE_ORDER_ASSIGN_OP(op, assign) \
    template <typename T1, typename T2> inline big_endian<T1> & \
    operator assign(big_endian<T1> &lhs, const big_endian<T2> &rhs) { return lhs = lhs.host() op rhs.host(); } \
    template <typename T1, typename T2> inline big_endian<T1> & \
    operator assign(big_endian<T1> &lhs, const little_endian<T2> &rhs) { return lhs = lhs.host() op rhs.host(); } \
    template <typename T1, typename T2> inline little_endian<T1> & \
    operator assign(little_endian<T1> &lhs, const little_endian<T2> &rhs) { return lhs = lhs.host() op rhs.host(); } \
    template <typename T1, typename T2> inline little_endian<T1> & \
    operator assign(little_endian<T1> &lhs, const big_endian<T2> &rhs) { return lhs = lhs.host() op rhs.host(); } \
    template <typename T, typename I, typename=typename std::enable_if<std::is_integral<I>::value>::type> \
    inline big_endian<T> &operator assign(big_endian<T> &lhs, I rhs) { return lhs = lhs.host() op rhs; } \
    template <typename T, typename I, typename=typename std::enable_if<std::is_integral<I>::value>::type> \
    inline I& operator assign(I &lhs, const big_endian<T> &rhs) { return lhs = lhs op rhs.host(); } \
    template <typename T, typename I, typename=typename std::enable_if<std::is_integral<I>::value>::type> \
    inline little_endian<T> &operator assign(little_endian<T> &lhs, I rhs) { return lhs = lhs.host() op rhs; } \
    template <typename T, typename I, typename=typename std::enable_if<std::is_integral<I>::value>::type> \
    inline I &operator assign(I &lhs, const little_endian<T> &rhs) { return lhs = lhs op rhs.host(); }

    DEFINE_BYTE_ORDER_BINARY_OP(+)

    DEFINE_BYTE_ORDER_BINARY_OP(-)

    DEFINE_BYTE_ORDER_BINARY_OP(*)

    DEFINE_BYTE_ORDER_BINARY_OP(/)

    DEFINE_BYTE_ORDER_ASSIGN_OP(+, +=)

    DEFINE_BYTE_ORDER_ASSIGN_OP(-, -=)

    DEFINE_BYTE_ORDER_ASSIGN_OP(*, *=)

    DEFINE_BYTE_ORDER_ASSIGN_OP(/, /=)

#pragma pop_macro("DEFINE_BYTE_ORDER_BINARY_OP")
#pragma pop_macro("DEFINE_BYTE_ORDER_ASSIGN_OP")

}

#endif //OMEGA_BYTE_ORDER_H
