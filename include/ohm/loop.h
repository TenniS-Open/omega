//
// Created by kier on 2020/7/28.
//

#ifndef OMEGA_LOOP_H
#define OMEGA_LOOP_H

#include "ohm/type_iterable.h"
#include <memory>

namespace ohm {
    template <typename T>
    class LoopRValue {
    public:
        using self = LoopRValue;
        using value_type = T;
        using iterator_value_type = T;
        using iterator_difference_type = size_t;
        using iterator_pointer = const T *;
        using iterator_reference = const T &;

        LoopRValue(T &&value)
                : m_value(std::make_shared<T>(std::forward<T>(value))) {}

        class Iterator : public std::iterator<std::input_iterator_tag,
                iterator_value_type, iterator_difference_type,
                iterator_pointer, iterator_reference> {
        public:
            Iterator(T *pv) : m_pv(pv) {}

            Iterator operator++(int) { return *this; }

            Iterator &operator++() { return *this; }

            iterator_reference operator*() const { return *m_pv; }

            iterator_pointer operator->() const { return m_pv; }

            bool operator!=(const Iterator &other) { return true; }

            bool operator==(const Iterator &other) { return !operator!=(other); }

        private:
            T *m_pv;
        };

        Iterator begin() const { return Iterator(m_value.get()); }

        Iterator end() const { return Iterator(m_value.get()); }

    private:
        std::shared_ptr<T> m_value;
    };

    template <typename T>
    class LoopRValueCopy {
    public:
        using self = LoopRValueCopy;
        using value_type = T;
        using iterator_value_type = T;
        using iterator_difference_type = size_t;
        using iterator_pointer = const T *;
        using iterator_reference = T;

        LoopRValueCopy(const T &value)
                : m_value(value) {}

        class Iterator : public std::iterator<std::input_iterator_tag,
                iterator_value_type, iterator_difference_type,
                iterator_pointer, iterator_reference> {
        public:
            Iterator(const T *pv) : m_pv(pv) {}

            Iterator operator++(int) { return *this; }

            Iterator &operator++() { return *this; }

            iterator_reference operator*() const { return *m_pv; }

            iterator_pointer operator->() const { return nullptr; }

            bool operator!=(const Iterator &other) { return true; }

            bool operator==(const Iterator &other) { return !operator!=(other); }

        private:
            const T *m_pv;
        };

        Iterator begin() const { return Iterator(&m_value); }

        Iterator end() const { return Iterator(&m_value); }

    private:
        T m_value;
    };

    template <typename T>
    class LoopConstLValue {
    public:
        using self = LoopConstLValue;
        using value_type = T;
        using iterator_value_type = T;
        using iterator_difference_type = size_t;
        using iterator_pointer = const T *;
        using iterator_reference = const T &;

        LoopConstLValue(const T &value)
                : m_iterator(&value) {}

        class Iterator : public std::iterator<std::input_iterator_tag,
                iterator_value_type, iterator_difference_type,
                iterator_pointer, iterator_reference> {
        public:
            Iterator(const T *pv) : m_pv(pv) {}

            Iterator operator++(int) { return *this; }

            Iterator &operator++() { return *this; }

            iterator_reference operator*() const { return *m_pv; }

            iterator_pointer operator->() const { return m_pv; }

            bool operator!=(const Iterator &other) { return true; }

            bool operator==(const Iterator &other) { return !operator!=(other); }

        private:
            const T *m_pv;
        };

        Iterator begin() const { return m_iterator; }

        Iterator end() const { return m_iterator; }

    private:
        Iterator m_iterator;
    };
    template <typename T>
    class LoopLValue {
    public:
        using self = LoopLValue;
        using value_type = T;
        using iterator_value_type = T;
        using iterator_difference_type = size_t;
        using iterator_pointer = T *;
        using iterator_reference = T &;

        LoopLValue(T &value)
                : m_iterator(&value) {}

        class Iterator : public std::iterator<std::input_iterator_tag,
                iterator_value_type, iterator_difference_type,
                iterator_pointer, iterator_reference> {
        public:
            Iterator(T *pv) : m_pv(pv) {}

            Iterator operator++(int) { return *this; }

            Iterator &operator++() { return *this; }

            iterator_reference operator*() const { return *const_cast<T*&>(m_pv); }

            iterator_pointer operator->() const { return const_cast<T*&>(m_pv); }

            bool operator!=(const Iterator &other) { return true; }

            bool operator==(const Iterator &other) { return !operator!=(other); }

        private:
            T *m_pv;
        };

        Iterator begin() const { return m_iterator; }

        Iterator end() const { return m_iterator; }

    private:
        Iterator m_iterator;
    };

    template <typename T>
    typename std::enable_if<
            std::is_copy_constructible<T>::value,
            LoopRValueCopy<T>>::type
    loop(T &&value) {
        return LoopRValueCopy<T>(value);
    }

    template <typename T>
    typename std::enable_if<
            std::is_move_constructible<T>::value &&
            !std::is_copy_constructible<T>::value,
            LoopRValue<T>>::type
    loop(T &&value) {
        return LoopRValue<T>(std::forward<T>(value));
    }

    template <typename T>
    typename std::enable_if<
            true,
            LoopConstLValue<T>>::type
    loop(const T &value) {
        return LoopConstLValue<T>(value);
    }

    template <typename T>
    typename std::enable_if<
            true,
            LoopLValue<T>>::type
    loop(T &value) {
        return LoopLValue<T>(value);
    }
}

#endif //OMEGA_LOOP_H
