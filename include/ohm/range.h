//
// Created by seetadev on 2020/7/20.
//

#ifndef OMEGA_RANGE_H
#define OMEGA_RANGE_H

#include <type_traits>
#include <cstdint>
#include <vector>
#include <iterator>

namespace ohm {
    template<typename T, typename K, typename Enable=void>
    struct up_type {
        using type = void;
    };

    template<typename T, typename K>
    struct up_type<T, K, typename std::enable_if<
            std::is_arithmetic<T>::value && std::is_arithmetic<K>::value
    >::type> {
    public:
        using type = decltype(std::declval<T>() + std::declval<K>());
    };

    template<typename T, typename K>
    struct up_type<T, K, typename std::enable_if<
            !(std::is_arithmetic<T>::value && std::is_arithmetic<K>::value) &&
            std::is_convertible<K, T>::value
    >::type> {
    public:
        using type = T;
    };

    template<typename T, typename K>
    struct up_type<T, K, typename std::enable_if<
            !(std::is_arithmetic<T>::value && std::is_arithmetic<K>::value) &&
            !std::is_convertible<K, T>::value &&
            std::is_convertible<T, K>::value
    >::type> {
    public:
        using type = K;
    };

    template<typename T, typename... Args>
    struct serial_type {
    public:
        using type = typename up_type<T, typename serial_type<Args...>::type>::type;
    };

    template<typename T>
    struct serial_type<T> {
    public:
        using type = T;
    };

    class RangeStepZeroException : public std::logic_error {
    public:
        using self = RangeStepZeroException;
        using supper = std::logic_error;

        RangeStepZeroException() : supper("Range step got zero, must be positive or negative.") {}
    };

    template<typename T>
    struct is_rangeable : public std::integral_constant<bool, std::is_integral<T>::value> {
    };

    template<typename T, typename Enable=void>
    class Range;

    template<typename T>
    class Range<T, typename std::enable_if<
            std::is_integral<T>::value && std::is_unsigned<T>::value
    >::type> {
    public:
        using self = Range;

        explicit Range(T end) : self(T(0), end, T(1)) {}

        Range(T begin, T end) : self(begin, end, T(1)) {}

        Range(T begin, T end, T step) : m_begin(begin), m_end(end), m_step(step) {
            if (step == 0) throw RangeStepZeroException();
            m_begin_it = Iterator(m_begin, m_step);
            m_end_it = Iterator(m_end, m_step);
        }

        class Iterator : public std::iterator<std::input_iterator_tag,
                T, T, const T *, const T &> {
        public:
            Iterator() : Iterator(0, 0) {}

            Iterator(T value, T step) : m_value(value), m_step(step) {}

            const Iterator operator++(int) {
                auto tmp = *this;
                m_value += m_step;
                return tmp;
            }

            Iterator &operator++() {
                m_value += m_step;
                return *this;
            }

            const T &operator*() const { return m_value; }

            const T *operator->() const { return &m_value; }

            bool operator!=(const Iterator &other) { return this->m_value < other.m_value; }

            bool operator==(const Iterator &other) { return !operator!=(other); }

        private:
            T m_value;
            T m_step;
        };

        const Iterator begin() const { return m_begin_it; }

        const Iterator end() const { return m_end_it; }

    private:
        T m_begin;
        T m_end;
        T m_step;

        Iterator m_begin_it;
        Iterator m_end_it;
    };

    template<typename T>
    class Range<T, typename std::enable_if<
            std::is_integral<T>::value && std::is_signed<T>::value
    >::type> {
    public:
        using self = Range;

        explicit Range(T end) : self(T(0), end, T(1)) {}

        Range(T begin, T end) : self(begin, end, T(1)) {}

        Range(T begin, T end, T step) : m_begin(begin), m_end(end), m_step(step) {
            if (step > 0) {
                m_begin_it = Iterator(m_begin, m_step, Iterator::PositiveStepNE);
                m_end_it = Iterator(m_end, m_step, Iterator::PositiveStepNE);
            } else if (step < 0) {
                m_begin_it = Iterator(m_begin, m_step, Iterator::NegativeStepNE);
                m_end_it = Iterator(m_end, m_step, Iterator::NegativeStepNE);
            } else {
                throw RangeStepZeroException();
            }
        }

        class Iterator : public std::iterator<std::input_iterator_tag,
                T, T, const T *, const T &> {
        public:
            using Comparer = std::function<bool(T, T)>;

            static bool PositiveStepNE(T a, T b) { return a < b; }

            static bool NegativeStepNE(T a, T b) { return a > b; }

            Iterator() : Iterator(0, 0) {}

            Iterator(T value, T step, Comparer cmp = PositiveStepNE)
                    : m_value(value), m_step(step), m_cmp(cmp) {}

            const Iterator operator++(int) {
                auto tmp = *this;
                m_value += m_step;
                return tmp;
            }

            Iterator &operator++() {
                m_value += m_step;
                return *this;
            }

            const T &operator*() const { return m_value; }

            const T *operator->() const { return &m_value; }

            bool operator!=(const Iterator &other) { return m_cmp(this->m_value, other.m_value); }

            bool operator==(const Iterator &other) { return !operator!=(other); }

        private:
            T m_value;
            T m_step;
            Comparer m_cmp;
        };

        const Iterator begin() const { return m_begin_it; }

        const Iterator end() const { return m_end_it; }

    private:
        T m_begin;
        T m_end;
        T m_step;

        Iterator m_begin_it;
        Iterator m_end_it;
    };

    template<typename _Begin, typename _End, typename _Step>
    typename std::enable_if<
            is_rangeable<_Begin>::value && is_rangeable<_End>::value && is_rangeable<_Step>::value,
            Range<typename serial_type<_Begin, _End, _Step>::type>>::type
    inline range(_Begin begin, _End end, _Step step) {
        using T = typename serial_type<_Begin, _End, _Step>::type;
        return Range<T>(T(begin), T(end), T(step));
    }

    template<typename _Begin, typename _End>
    typename std::enable_if<
            is_rangeable<_Begin>::value && is_rangeable<_End>::value,
            Range<typename serial_type<_Begin, _End>::type>>::type
    inline range(_Begin begin, _End end) {
        using T = typename serial_type<_Begin, _End>::type;
        return Range<T>(T(begin), T(end));
    }

    template<typename _Begin>
    typename std::enable_if<
            is_rangeable<_Begin>::value,
            Range<_Begin>>::type
    inline range(_Begin begin) {
        using T = _Begin;
        return Range<T>(begin);
    }

    inline std::vector<int32_t> ruler();

    inline std::vector<std::pair<int32_t, int32_t>> chunk();
}

#endif //OMEGA_RANGE_H
