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
        using return_type = typename remove_cr<decltype(Return<T>(0))>::type;
        static constexpr bool value = !std::is_same<decltype(Return<T>(0)), void>::value;
    };

    template<typename T, typename FUNC, typename Iter, typename Enable = void>
    class MappingRange;

    template<typename T, typename FUNC, typename Iter>
    class MappingRange<T, FUNC, Iter,
            typename std::enable_if<
                    has_iterator_tag<Iter, std::input_iterator_tag>::value &&
                    is_mapper_of<FUNC, typename remove_cr<
                            typename std::iterator_traits<Iter>::value_type>::type>::value &&
                    std::is_convertible<typename
                    is_mapper_of<FUNC, typename remove_cr<
                            typename std::iterator_traits<Iter>::value_type>::type>::return_type, T>::value>::type> {
    public:
        using value_type = T;
        using iterator_value_type = typename remove_cr<
                typename std::iterator_traits<Iter>::value_type>::type;

        MappingRange(FUNC func, Iter begin, Iter end)
            : m_begin_it(func, begin), m_end_it(func, end) {}

        class Iterator : public std::iterator<std::input_iterator_tag,
                T, T, const T *, const T &> {
        public:
            Iterator(FUNC mapper, Iter raw)
                    : m_mapper(mapper), m_raw(raw) {}

            const Iterator operator++(int) {
                auto tmp = *this;
                m_raw++;
                m_mapped = false;
                return tmp;
            }

            Iterator &operator++() {
                ++m_raw;
                m_mapped = false;
                return *this;
            }

            const T &operator*() const {
                do_map<T>();
                return *m_value;
            }

            const T *operator->() const {
                do_map<T>();
                return &*m_value;
            }

            bool operator!=(const Iterator &other) { return this->m_raw != other.m_raw; }

            bool operator==(const Iterator &other) { return !operator!=(other); }

        private:
            FUNC m_mapper;
            Iter m_raw;
            mutable std::shared_ptr<T> m_value;
            mutable bool m_mapped = false;

            template <typename S>
            void do_map(typename std::enable_if<std::is_copy_assignable<S>::value, bool>::type = true) const {
                if (m_mapped) return;
                if (m_value) {
                    *m_value = T(m_mapper(*m_raw));
                } else {
                    m_value = std::make_shared<T>(m_mapper(*m_raw));
                }
                m_mapped = true;
            }

            template <typename S>
            void do_map(typename std::enable_if<!std::is_copy_assignable<S>::value, bool>::type = true) const {
                if (m_mapped) return;
                if (m_value) {
                    // 显式调用析构和构造，两步之间不能够异常打断
                    try {
                        m_value->~T();
                        new(m_value.get())T(m_mapper(*m_raw));
                    } catch (...) {
                        std::cerr << "[FATAL] Got unexpected Error, please use mapped instead map!" << std::endl;
                        exit(11);
                    }
                } else {
                    m_value = std::make_shared<T>(m_mapper(*m_raw));
                }
                m_mapped = true;
            }
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
            is_mapper_of<FUNC, typename has_iterator<Iter>::value_type>::return_type, T>::value,
            MappingRange<T, FUNC, typename remove_cr<typename has_begin<const Iter>::type>::type>>::type
    map_to(FUNC func, const Iter &iter) {
        using Mapped = MappingRange<T, FUNC, typename remove_cr<typename has_begin<const Iter>::type>::type>;
        return Mapped(func, iter.begin(), iter.end());
    }

    template<typename FUNC, typename Iter>
    inline typename std::enable_if<
            is_iterable<Iter>::value &&
            is_mapper_of<FUNC, typename has_iterator<Iter>::value_type>::value,
            MappingRange<typename is_mapper_of<FUNC, typename has_iterator<Iter>::value_type>::return_type,
            FUNC, typename remove_cr<typename has_begin<const Iter>::type>::type>>::type
    map(FUNC func, const Iter &iter) {
        using T = typename is_mapper_of<FUNC, typename has_iterator<Iter>::value_type>::return_type;
        return map_to<T>(func, iter);
    }
}

#endif //OMEGA_MAP_H
