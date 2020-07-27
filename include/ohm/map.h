//
// Created by kier on 2020/7/22.
//

#ifndef OMEGA_MAP_H
#define OMEGA_MAP_H

#include <iostream>

#include "type_iterable.h"
#include "list.h"
#include "type.h"

namespace ohm {
    template<typename T, typename ARG>
    struct is_mapper_of {
    private:
        template<typename U>
        static auto Return(int) -> decltype(std::declval<U>()(std::declval<ARG>()));

        template<typename U>
        static auto Return(...) -> void;

    public:
        using forward_return_type = decltype(Return<T>(0));
        using return_type = typename remove_cr<forward_return_type>::type;
        static constexpr bool value = !std::is_same<forward_return_type, void>::value;
    };

    template<typename FUNC, typename Iter, typename Enable = void>
    class MappingRange;

    template<typename FUNC, typename Iter>
    class MappingRange<FUNC, Iter,
            typename std::enable_if<
                    has_iterator_tag<Iter, std::input_iterator_tag>::value &&
                    is_mapper_of<FUNC, typename std::iterator_traits<Iter>::value_type>::value>::type> {
    public:
        using T = typename is_mapper_of<FUNC, typename std::iterator_traits<Iter>::value_type>::forward_return_type;
        using value_type = T;
        using iterator_value_type = typename remove_cr<T>::type;
        using iterator_difference_type = typename std::iterator_traits<Iter>::difference_type;
        using iterator_pointer = std::conditional<
                std::is_reference<T>::value,
                typename std::add_pointer<typename std::remove_reference<T>::type>::type,
                typename std::add_pointer<T>::type>;
        using iterator_reference = T;

        MappingRange(FUNC func, Iter begin, Iter end)
            : m_begin_it(func, begin), m_end_it(func, end) {}

        class Iterator : public std::iterator<std::input_iterator_tag,
                iterator_value_type, iterator_difference_type,
                iterator_pointer, iterator_reference> {
        public:
            Iterator(FUNC mapper, Iter raw)
                    : m_mapper(mapper), m_raw(raw) {}

            const Iterator operator++(int) {
                auto tmp = *this;
                m_raw++;
                return tmp;
            }

            Iterator &operator++() {
                ++m_raw;
                return *this;
            }

            iterator_reference operator*() const {
                return m_mapper(*const_cast<Iter&>(m_raw));
            }

            template <typename S>
            typename std::enable_if<std::is_reference<S>::value, iterator_pointer>::type
            get_pointer() const {
                return &operator*();
            }

            template <typename S>
            typename std::enable_if<!std::is_reference<S>::value, iterator_pointer>::type
            get_pointer() const {
                // static_assert
                return nullptr;
            }

            iterator_pointer operator->() const {
                return get_pointer<T>();
            }

            bool operator!=(const Iterator &other) { return this->m_raw != other.m_raw; }

            bool operator==(const Iterator &other) { return !operator!=(other); }

        private:
            FUNC m_mapper;
            Iter m_raw;
        };

        const Iterator begin() const { return m_begin_it; }

        const Iterator end() const { return m_end_it; }

    private:
        Iterator m_begin_it;
        Iterator m_end_it;
    };

    template<typename T, typename FUNC, typename Iter>
    inline typename std::enable_if<
            is_iterable<Iter>::value &&
            is_mapper_of<FUNC, typename has_iterator<Iter>::value_type>::value &&
            std::is_convertible<typename
            is_mapper_of<FUNC, typename has_iterator<Iter>::value_type>::return_type, T>::value &&
            !has_iterator_tag<typename has_begin<Iter>::type, std::random_access_iterator_tag>::value,
            List<T>>::type
    mapped_to(FUNC func, const Iter &iter) {
        List<T> mapped;
        auto beg = iter.begin();
        auto end = iter.end();
        for (auto it = beg; it != end; ++it) {
            mapped.emplace_back(T(func(*it)));
        }
        return mapped;
    }

    template<typename T, typename FUNC, typename Iter>
    inline typename std::enable_if<
            is_iterable<Iter>::value &&
            is_mapper_of<FUNC, typename has_iterator<Iter>::value_type>::value &&
            std::is_convertible<typename
            is_mapper_of<FUNC, typename has_iterator<Iter>::value_type>::return_type, T>::value &&
            has_iterator_tag<typename has_begin<Iter>::type, std::random_access_iterator_tag>::value,
            List<T>>::type
    mapped_to(FUNC func, const Iter &iter) {
        List<T> mapped;
        auto beg = iter.begin();
        auto end = iter.end();
        mapped.reserve(end - beg);
        for (auto it = beg; it != end; ++it) {
            mapped.emplace_back(T(func(*it)));
        }
        return mapped;
    }

    template<typename FUNC, typename Iter>
    inline typename std::enable_if<
            is_iterable<Iter>::value &&
            is_mapper_of<FUNC, typename has_iterator<Iter>::value_type>::value,
            List<typename is_mapper_of<FUNC, typename has_iterator<Iter>::value_type>::return_type>>::type
    mapped(FUNC func, const Iter &iter) {
        using T = typename is_mapper_of<FUNC, typename has_iterator<Iter>::value_type>::return_type;
        return mapped_to<T>(func, iter);
    }

    template<typename T, typename FUNC, typename Iter>
    inline typename std::enable_if<
            is_iterable<Iter>::value &&
            is_mapper_of<FUNC, typename has_iterator<Iter>::value_type>::value &&
            std::is_convertible<typename
            is_mapper_of<FUNC, typename has_iterator<Iter>::value_type>::forward_return_type, T>::value,
            MappingRange<T, FUNC, typename remove_cr<typename has_begin<const Iter>::type>::type>>::type
    map_to(FUNC func, const Iter &iter) {
        using Mapped = MappingRange<T, FUNC, typename remove_cr<typename has_begin<const Iter>::type>::type>;
        return Mapped(func, iter.begin(), iter.end());
    }

    template<typename FUNC, typename Iter>
    inline typename std::enable_if<
            is_iterable<Iter>::value &&
            is_mapper_of<FUNC, typename has_iterator<Iter>::forward_value_type>::value,
            MappingRange<FUNC, typename has_begin<const Iter>::type>>::type
    map(FUNC func, const Iter &iter) {
        auto beg = iter.begin();
        auto end = iter.end();
        using Mapped = MappingRange<FUNC, decltype(beg)>;
        return Mapped(func, beg, end);
    }
}

#endif //OMEGA_MAP_H
