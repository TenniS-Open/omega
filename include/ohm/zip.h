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

namespace ohm {
    template<typename... Args>
    struct __is_all_iterable : public std::false_type {
    };

    template<typename T>
    struct __is_all_iterable<T> : public std::integral_constant<bool, is_iterable<T>::value> {
    };

    template<typename T, typename... Args>
    struct __is_all_iterable<T, Args...>
            : public std::integral_constant<bool, is_iterable<T>::value && __is_all_iterable<Args...>::value> {
    };

    template<typename Enable = void, typename...Args>
    struct __zip_range_value_type;

    template<typename Enable = void, typename...Args>
    struct __zip_range_forward_value_type;

    template<typename _Rg>
    inline auto __get_range_value() -> typename has_iterator<_Rg>::value_type;

    template<typename _Rg>
    inline auto __get_range_forward_value() -> typename has_iterator<_Rg>::forward_value_type;

    template<typename...Args>
    struct __zip_range_value_type<typename std::enable_if<__is_all_iterable<Args...>::value>::type, Args...> {
        using type = decltype(std::make_tuple(__get_range_value<Args>()...));
    };

    template<typename...Args>
    struct __zip_range_forward_value_type<typename std::enable_if<__is_all_iterable<Args...>::value>::type, Args...> {
        using type = decltype(std::forward_as_tuple(__get_range_forward_value<Args>()...));
    };

    template<typename _Rg>
    inline auto __get_range_iterator() -> typename has_begin<_Rg>::type;


    template<typename Enable = void, typename...Args>
    struct __zip_range_iterator_type;

    template<typename...Args>
    struct __zip_range_iterator_type<typename std::enable_if<__is_all_iterable<Args...>::value>::type, Args...> {
        using type = decltype(std::make_tuple(__get_range_iterator<Args>()...));
    };

    template<typename _It>
    inline auto __get_forward_star_value() -> decltype(*std::declval<_It>());

    template<typename T>
    struct __tuple_forward_star_value_type;

    template<typename T>
    struct __tuple_forward_star_value_type<std::tuple<T>> {
        using type = decltype(std::forward_as_tuple(__get_forward_star_value<T>()));
    };

    template<typename T, typename...Args>
    struct __tuple_forward_star_value_type<std::tuple<T, Args...>> {
        using type = decltype(std::tuple_cat(
                std::forward_as_tuple(__get_forward_star_value<T>()),
                std::declval<typename __tuple_forward_star_value_type<std::tuple<Args...>>::type>()
        ));
    };

    template<typename _It>
    inline auto __get_star_value() -> typename remove_cr<decltype(*std::declval<_It>())>::type;

    template<typename T>
    struct __tuple_star_value_type;

    template<typename T>
    struct __tuple_star_value_type<std::tuple<T>> {
        using type = decltype(std::make_tuple(__get_star_value<T>()));
    };

    template<typename T, typename...Args>
    struct __tuple_star_value_type<std::tuple<T, Args...>> {
        using type = decltype(std::tuple_cat(
                std::make_tuple(__get_star_value<T>()),
                std::declval<typename __tuple_star_value_type<std::tuple<Args...>>::type>()
        ));
    };

    template<typename _Rg>
    inline decltype(std::declval<_Rg>().begin()) __get_begin_iterator(_Rg &&rg) { return rg.begin(); }

    template<typename _Rg>
    inline decltype(std::declval<_Rg>().end()) __get_end_iterator(_Rg &&rg) { return rg.end(); }


    template<typename T, typename K>
    inline typename __zip_range_iterator_type<void, T, K>::type __zip_range_begin_iterator(T &&t, K &&k) {
        return std::make_tuple(t.begin(), k.begin());
    }

    template<typename T, typename... Args>
    inline typename __zip_range_iterator_type<void, T, Args...>::type
    __zip_range_begin_iterator(T &&t, Args &&...args) {
        return std::tuple_cat(std::make_tuple(t.begin()), __zip_range_begin_iterator(std::forward<Args>(args)...));
    }

    template<typename T, typename K>
    inline typename __zip_range_iterator_type<void, T, K>::type __zip_range_end_iterator(T &&t, K &&k) {
        return std::make_tuple(t.end(), k.end());
    }

    template<typename T, typename... Args>
    inline typename __zip_range_iterator_type<void, T, Args...>::type __zip_range_end_iterator(T &&t, Args &&...args) {
        return std::tuple_cat(std::make_tuple(t.end()), __zip_range_end_iterator(std::forward<Args>(args)...));
    }

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

    template<size_t N, typename... Args>
    inline typename std::enable_if<N == std::tuple_size<std::tuple<Args...>>::value, bool>::type
    __tuple_iterator_not_equal_at(
            const std::tuple<Args...> &, const std::tuple<Args...> &) {
        return true;
    }

    template<size_t N, typename... Args>
    inline typename std::enable_if<N < std::tuple_size<std::tuple<Args...>>::value, bool>::type
    __tuple_iterator_not_equal_at(
            const std::tuple<Args...> &a, const std::tuple<Args...> &b) {
        auto lhs = std::get<N>(a);
        auto rhs = std::get<N>(b);
        return lhs != rhs &&
               __tuple_iterator_not_equal_at<N + 1, Args...>(a, b);
    }

    template<typename... Args>
    inline bool __tuple_iterator_not_equal(const std::tuple<Args...> &a, const std::tuple<Args...> &b) {
        return __tuple_iterator_not_equal_at<0, Args...>(a, b);
    }

#pragma push_macro("TUPLE_START_VALUE")
#pragma push_macro("G")
#define TUPLE_START_VALUE(N) \
    template<typename T> \
    inline typename std::enable_if<std::tuple_size<T>::value == N, \
            typename __tuple_star_value_type<T>::type>::type \
    __tuple_start_value(T &a)
#define G(N) *std::get<N>(a)

    TUPLE_START_VALUE(1) {
        return std::make_tuple(G(0));
    }

    TUPLE_START_VALUE(2) {
        return std::make_tuple(G(0), G(1));
    }

    TUPLE_START_VALUE(3) {
        return std::make_tuple(G(0), G(1), G(2));
    }

    TUPLE_START_VALUE(4) {
        return std::make_tuple(G(0), G(1), G(2), G(3));
    }

    TUPLE_START_VALUE(5) {
        return std::make_tuple(G(0), G(1), G(2), G(3), G(4));
    }

    TUPLE_START_VALUE(6) {
        return std::make_tuple(G(0), G(1), G(2), G(3), G(4),
                               G(5));
    }

    TUPLE_START_VALUE(7) {
        return std::make_tuple(G(0), G(1), G(2), G(3), G(4),
                               G(5), G(6));
    }

    TUPLE_START_VALUE(8) {
        return std::make_tuple(G(0), G(1), G(2), G(3), G(4),
                               G(5), G(6), G(7));
    }

    TUPLE_START_VALUE(9) {
        return std::make_tuple(G(0), G(1), G(2), G(3), G(4),
                               G(5), G(6), G(7), G(8));
    }

    TUPLE_START_VALUE(10) {
        return std::make_tuple(G(0), G(1), G(2), G(3), G(4),
                               G(5), G(6), G(7), G(8), G(9));
    }

#undef TUPLE_START_VALUE
#undef G
#pragma pop_macro("TUPLE_START_VALUE")
#pragma pop_macro("G")

    template<typename... Args>
    inline typename std::enable_if<
            __is_all_iterable<Args...>::value,
            List<typename __zip_range_value_type<void, Args...>::type>>::type
    zipped(Args &&...args) {
        auto beg = __zip_range_begin_iterator(std::forward<Args>(args)...);
        auto end = __zip_range_end_iterator(std::forward<Args>(args)...);

        List<typename __zip_range_value_type<void, Args...>::type> result;

        auto it = beg;
        while (__tuple_iterator_not_equal(it, end)) {
            result.emplace_back(__tuple_start_value(it));
            it = __tuple_iterator_forward(it);
        }

        return result;
    }
}

#endif //OMEGA_ZIP_H
