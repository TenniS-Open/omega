//
// Created by kier on 2020/7/23.
//

#ifndef OMEGA_ZIP_H
#define OMEGA_ZIP_H

#include <tuple>
#include <memory>

#include "type_iterable.h"
#include "type_tuple.h"
#include "list.h"
#include "print.h"
#include "type.h"
#include "tuple_iterator.h"
#include "type_variable.h"

namespace ohm {
    template<size_t N, typename... Args>
    inline void __tuple_iterator_forward_at(
            typename std::enable_if<N == std::tuple_size<std::tuple<Args...>>::value, std::tuple<Args...>>::type &it) {}

    template<size_t N, typename... Args>
    inline void __tuple_iterator_forward_at(
            typename std::enable_if<N < std::tuple_size<std::tuple<Args...>>::value, std::tuple<Args...>>::type &it) {
        ++std::get<N>(it);
        __tuple_iterator_forward_at<N + 1, Args...>(it);
    }

    template<typename... Args>
    inline std::tuple<Args...> __tuple_iterator_forward(const std::tuple<Args...> &it) {
        auto next = it;
        __tuple_iterator_forward_at<0, Args...>(next);
        return next;
    }

    template<typename Iter>
    class ZippingRange;

    /**
     *
     * @tparam Args tuple of iterators
     */
    template<typename... Args>
    class ZippingRange<std::tuple<Args...>> {
    public:
        using Iter = std::tuple<Args...>;
        using value_type = typename tuple_it_forward_star_type<Iter>::type;
        using iterator_value_type = typename tuple_it_star_type<Iter>::type;
        using iterator_difference_type = size_t;
        using iterator_pointer = typename std::add_const<typename std::add_pointer<value_type>::type>::type;
        using iterator_reference = value_type;

        ZippingRange(Iter begin, Iter end)
                : m_begin_it(begin), m_end_it(end) {}

        class Iterator : public std::iterator<std::input_iterator_tag,
                iterator_value_type, iterator_difference_type,
                iterator_pointer, iterator_reference> {
        public:
            Iterator(Iter raw)
                    : m_raw(raw) {}

            const Iterator operator++(int) {
                auto tmp = *this;
                m_raw = __tuple_iterator_forward(m_raw);
                return tmp;
            }

            Iterator &operator++() {
                m_raw = __tuple_iterator_forward(m_raw);
                return *this;
            }

            iterator_reference operator*() const {
                return tuple_it_forward_star(const_cast<Iter &>(m_raw));
            }

            iterator_pointer operator->() const {
                return nullptr;
            }

            bool operator!=(const Iterator &other) { return tuple_it_not_equal(this->m_raw, other.m_raw); }

            bool operator==(const Iterator &other) { return !operator!=(other); }

        private:
            Iter m_raw;
        };

        const Iterator begin() const { return m_begin_it; }

        const Iterator end() const { return m_end_it; }

    private:
        Iterator m_begin_it;
        Iterator m_end_it;
    };

    template<typename... Args>
    inline typename std::enable_if<
            is_all_iterable<Args...>::value && __GT(variable_count<Args...>::value, 0),
            List<typename tuple_it_iterable_value_type<Args...>::type>>::type
    zipped(Args &&...args) {
        auto beg = tuple_it_from_iterable_begin(std::forward<Args>(args)...);
        auto end = tuple_it_from_iterable_end(std::forward<Args>(args)...);

        List<typename tuple_it_iterable_value_type<Args...>::type> result;

        auto it = beg;
        while (tuple_it_not_equal(it, end)) {
            result.emplace_back(tuple_it_star(it));
            it = __tuple_iterator_forward(it);
        }

        return result;
    }

    template<typename... Args>
    inline typename std::enable_if<
            is_all_iterable<Args...>::value && __GT(variable_count<Args...>::value, 0),
            ZippingRange<typename tuple_it_from_iterable_type<Args...>::type>>::type
    zip(Args &&...args) {
        auto beg = tuple_it_from_iterable_begin(std::forward<Args>(args)...);
        auto end = tuple_it_from_iterable_end(std::forward<Args>(args)...);

        using Zipped = ZippingRange<typename tuple_it_from_iterable_type<Args...>::type>;

        return Zipped(beg, end);
    }
}

#endif //OMEGA_ZIP_H
