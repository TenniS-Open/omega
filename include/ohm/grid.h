//
// Created by kier on 2020/7/23.
//

#ifndef OMEGA_GRID_H
#define OMEGA_GRID_H

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
    inline typename std::enable_if<__EQ(N, std::tuple_size<std::tuple<Args...>>::value), bool>::type
    __grid_tuple_it_not_equal_at(
            const std::tuple<Args...> &, const std::tuple<Args...> &) {
        return true;
    }

    template<size_t N, typename... Args>
    inline typename std::enable_if<__LT(N, std::tuple_size<std::tuple<Args...>>::value), bool>::type
    __grid_tuple_it_not_equal_at(
            const std::tuple<Args...> &a, const std::tuple<Args...> &b) {
        auto lhs = std::get<N>(a);
        auto rhs = std::get<N>(b);
        return lhs != rhs;
    }

    template<typename... Args>
    inline bool __grid_tuple_it_not_equal(const std::tuple<Args...> &a, const std::tuple<Args...> &b) {
        return __grid_tuple_it_not_equal_at<0, Args...>(a, b);
    }
    
    template<size_t N, typename... Args>
    inline void __grid_tuple_iterator_forward_at(
            typename std::enable_if<N == std::tuple_size<std::tuple<Args...>>::value, std::tuple<Args...>>::type &it,
            const std::tuple<Args...> &begin,
            const std::tuple<Args...> &end) {}

    template<size_t N, typename... Args>
    inline void __grid_tuple_iterator_forward_at(
            typename std::enable_if<
                    __LT(N, std::tuple_size<std::tuple<Args...>>::value) &&
                    __GT(N, 0), std::tuple<Args...>>::type &it,
            const std::tuple<Args...> &begin,
            const std::tuple<Args...> &end) {
        ++std::get<N>(it);
        if (std::get<N>(it) == std::get<N>(end)) {
            std::get<N>(it) = std::get<N>(begin);
            __grid_tuple_iterator_forward_at<N - 1, Args...>(it, begin, end);
        }
    }


    template<size_t N, typename... Args>
    inline void __grid_tuple_iterator_forward_at(
            typename std::enable_if<__EQ(N, 0), std::tuple<Args...>>::type &it,
            const std::tuple<Args...> &begin,
            const std::tuple<Args...> &end) {
        ++std::get<N>(it);
    }


    template<typename... Args>
    inline std::tuple<Args...> __grid_tuple_iterator_forward(const std::tuple<Args...> &it,
                                                             const std::tuple<Args...> &begin,
                                                             const std::tuple<Args...> &end) {
        auto next = it;
        constexpr auto N = std::tuple_size<std::tuple<Args...>>::value;
        __grid_tuple_iterator_forward_at<N - 1, Args...>(next, begin, end);
        return next;
    }

    template<typename Iter>
    class GridRange;

    /**
     *
     * @tparam Args tuple of iterators
     */
    template<typename... Args>
    class GridRange<std::tuple<Args...>> {
    public:
        using Iter = std::tuple<Args...>;
        using value_type = typename __tuple_it_forward_star_type<Iter>::type;
        using iterator_value_type = typename __tuple_it_star_type<Iter>::type;
        using iterator_difference_type = size_t;
        using iterator_pointer = typename std::add_const<typename std::add_pointer<value_type>::type>::type;
        using iterator_reference = value_type;

        GridRange(Iter begin, Iter end)
                : m_begin_it(begin, begin, end), m_end_it(end, begin, end) {}

        class Iterator : public std::iterator<std::input_iterator_tag,
                iterator_value_type, iterator_difference_type,
                iterator_pointer, iterator_reference> {
        public:
            Iterator(Iter cur, Iter begin, Iter end)
                    : m_cur(cur), m_begin(begin), m_end(end) {}

            const Iterator operator++(int) {
                auto tmp = *this;
                m_cur = __grid_tuple_iterator_forward(m_cur, m_begin, m_end);
                return tmp;
            }

            Iterator &operator++() {
                m_cur = __grid_tuple_iterator_forward(m_cur, m_begin, m_end);
                return *this;
            }

            iterator_reference operator*() const {
                return __tuple_it_forward_star(const_cast<Iter &>(m_cur));
            }

            iterator_pointer operator->() const {
                return nullptr;
            }

            bool operator!=(const Iterator &other) { return __grid_tuple_it_not_equal(this->m_cur, other.m_cur); }

            bool operator==(const Iterator &other) { return !operator!=(other); }

        private:
            Iter m_cur;
            Iter m_begin;
            Iter m_end;
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
            List<typename __tuple_it_iterable_value_type<Args...>::type>>::type
    grided(Args &&...args) {
        auto beg = __tuple_it_from_iterable_begin(std::forward<Args>(args)...);
        auto end = __tuple_it_from_iterable_end(std::forward<Args>(args)...);

        List<typename __tuple_it_iterable_value_type<Args...>::type> result;

        auto it = beg;
        while (__grid_tuple_it_not_equal(it, end)) {
            result.emplace_back(__tuple_it_star(it));
            it = __grid_tuple_iterator_forward(it, beg, end);
        }

        return result;
    }

    template<typename... Args>
    inline typename std::enable_if<
            is_all_iterable<Args...>::value && __GT(variable_count<Args...>::value, 0),
            GridRange<typename __tuple_it_from_iterable_type<Args...>::type>>::type
    grid(Args &&...args) {
        auto beg = __tuple_it_from_iterable_begin(std::forward<Args>(args)...);
        auto end = __tuple_it_from_iterable_end(std::forward<Args>(args)...);

        using Grid = GridRange<typename __tuple_it_from_iterable_type<Args...>::type>;

        return Grid(beg, end);
    }
}


#endif //OMEGA_GRID_H
