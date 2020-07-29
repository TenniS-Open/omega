//
// Created by kier on 2020/7/29.
//

#ifndef OMEGA_TUPLE_ITERATOR_H
#define OMEGA_TUPLE_ITERATOR_H

#include <tuple>
#include "type_variable.h"

namespace ohm {
    template<typename _Rg>
    inline auto __hidden_get_iterable_begin() -> typename has_begin<_Rg>::type;

    template<typename Enable = void, typename...Args>
    struct __hidden_tuple_it_from_iterable_type;

    template<typename...Args>
    struct __hidden_tuple_it_from_iterable_type<typename std::enable_if<is_all_iterable<Args...>::value>::type, Args...> {
        using type = decltype(std::make_tuple(__hidden_get_iterable_begin<Args>()...));
    };

    template<typename... Args>
    struct __tuple_it_from_iterable_type {
        using type = typename __hidden_tuple_it_from_iterable_type<void, Args...>::type;
    };

    template<typename T>
    inline typename __tuple_it_from_iterable_type<T>::type
    __tuple_it_from_iterable_begin(T &&t) {
        return std::make_tuple(t.begin());
    }

    template<typename T, typename K>
    inline typename __tuple_it_from_iterable_type<T, K>::type
    __tuple_it_from_iterable_begin(T &&t, K &&k) {
        return std::make_tuple(t.begin(), k.begin());
    }

    template<typename T, typename... Args>
    inline typename __tuple_it_from_iterable_type<T, Args...>::type
    __tuple_it_from_iterable_begin(T &&t, Args &&...args) {
        return std::tuple_cat(std::make_tuple(t.begin()), __tuple_it_from_iterable_begin(std::forward<Args>(args)...));
    }

    template<typename T>
    inline typename __tuple_it_from_iterable_type<T>::type
    __tuple_it_from_iterable_end(T &&t) {
        return std::make_tuple(t.end());
    }

    template<typename T, typename K>
    inline typename __tuple_it_from_iterable_type<T, K>::type
    __tuple_it_from_iterable_end(T &&t, K &&k) {
        return std::make_tuple(t.end(), k.end());
    }

    template<typename T, typename... Args>
    inline typename __tuple_it_from_iterable_type<T, Args...>::type
    __tuple_it_from_iterable_end(T &&t, Args &&...args) {
        return std::tuple_cat(std::make_tuple(t.end()), __tuple_it_from_iterable_end(std::forward<Args>(args)...));
    }

    // template<typename _It>
    // inline auto __hidden_tuple_it_forward_star_value() -> decltype(*std::declval<_It>());

    template<typename T>
    struct __tuple_it_forward_star_type;

    template<typename T>
    struct __tuple_it_forward_star_type<std::tuple<T>> {
        using type = std::tuple<decltype(*std::declval<T>())>;
    };

    template<typename T, typename...Args>
    struct __tuple_it_forward_star_type<std::tuple<T, Args...>> {
        using type = decltype(std::tuple_cat(
                std::declval<typename __tuple_it_forward_star_type<std::tuple<T>>::type>(),
                std::declval<typename __tuple_it_forward_star_type<std::tuple<Args...>>::type>()
        ));
    };

    // template<typename _It>
    // inline auto __hidden_tuple_it_star_value() -> typename remove_cr<decltype(*std::declval<_It>())>::type;

    template<typename T>
    struct __tuple_it_star_type;

    template<typename T>
    struct __tuple_it_star_type<std::tuple<T>> {
        using type = decltype(std::make_tuple(*std::declval<T>()));
    };

    template<typename T, typename...Args>
    struct __tuple_it_star_type<std::tuple<T, Args...>> {
        using type = decltype(std::tuple_cat(
                std::declval<typename __tuple_it_star_type<std::tuple<T>>::type>(),
                std::declval<typename __tuple_it_star_type<std::tuple<Args...>>::type>()
        ));
    };

    template<typename... Args>
    struct __tuple_it_iterable_value_type {
        using type = typename __tuple_it_star_type<typename __tuple_it_from_iterable_type<Args...>::type>::type;
    };

    template<typename... Args>
    struct __tuple_it_iterable_forward_value_type {
        using type = typename __tuple_it_forward_star_type<typename __tuple_it_from_iterable_type<Args...>::type>::type;
    };



    template<size_t N, typename... Args>
    inline typename std::enable_if<__EQ(N, std::tuple_size<std::tuple<Args...>>::value), bool>::type
    __hidden_tuple_it_not_equal_at(
            const std::tuple<Args...> &, const std::tuple<Args...> &) {
        return true;
    }

    template<size_t N, typename... Args>
    inline typename std::enable_if<__LT(N, std::tuple_size<std::tuple<Args...>>::value), bool>::type
    __hidden_tuple_it_not_equal_at(
            const std::tuple<Args...> &a, const std::tuple<Args...> &b) {
        auto lhs = std::get<N>(a);
        auto rhs = std::get<N>(b);
        return lhs != rhs &&
               __hidden_tuple_it_not_equal_at<N + 1, Args...>(a, b);
    }

    template<typename... Args>
    inline bool __tuple_it_not_equal(const std::tuple<Args...> &a, const std::tuple<Args...> &b) {
        return __hidden_tuple_it_not_equal_at<0, Args...>(a, b);
    }

#pragma push_macro("TUPLE_START_VALUE")
#pragma push_macro("TUPLE_FORWARD_START_VALUE")
#pragma push_macro("G")
#define TUPLE_START_VALUE(N) \
    template<typename T> \
    inline typename std::enable_if<std::tuple_size<T>::value == N, \
            typename __tuple_it_star_type<T>::type>::type \
    __tuple_it_star(T &a)
#define TUPLE_FORWARD_START_VALUE(N) \
    template<typename T> \
    inline typename std::enable_if<std::tuple_size<T>::value == N, \
            typename __tuple_it_forward_star_type<T>::type>::type \
    __tuple_it_forward_star(T &a)
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

    TUPLE_FORWARD_START_VALUE(1) {
        return std::forward_as_tuple(G(0));
    }

    TUPLE_FORWARD_START_VALUE(2) {
        return std::forward_as_tuple(G(0), G(1));
    }

    TUPLE_FORWARD_START_VALUE(3) {
        return std::forward_as_tuple(G(0), G(1), G(2));
    }

    TUPLE_FORWARD_START_VALUE(4) {
        return std::forward_as_tuple(G(0), G(1), G(2), G(3));
    }

    TUPLE_FORWARD_START_VALUE(5) {
        return std::forward_as_tuple(G(0), G(1), G(2), G(3), G(4));
    }

    TUPLE_FORWARD_START_VALUE(6) {
        return std::forward_as_tuple(G(0), G(1), G(2), G(3), G(4),
                                     G(5));
    }

    TUPLE_FORWARD_START_VALUE(7) {
        return std::forward_as_tuple(G(0), G(1), G(2), G(3), G(4),
                                     G(5), G(6));
    }

    TUPLE_FORWARD_START_VALUE(8) {
        return std::forward_as_tuple(G(0), G(1), G(2), G(3), G(4),
                                     G(5), G(6), G(7));
    }

    TUPLE_FORWARD_START_VALUE(9) {
        return std::forward_as_tuple(G(0), G(1), G(2), G(3), G(4),
                                     G(5), G(6), G(7), G(8));
    }

    TUPLE_FORWARD_START_VALUE(10) {
        return std::forward_as_tuple(G(0), G(1), G(2), G(3), G(4),
                                     G(5), G(6), G(7), G(8), G(9));
    }

#undef TUPLE_START_VALUE
#undef G
#pragma pop_macro("TUPLE_FORWARD_START_VALUE")
#pragma pop_macro("TUPLE_START_VALUE")
#pragma pop_macro("G")
}

#endif //OMEGA_TUPLE_ITERATOR_H
