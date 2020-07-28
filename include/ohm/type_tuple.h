//
// Created by kier on 2020/7/24.
//

#ifndef OMEGA_TYPE_TUPLE_H
#define OMEGA_TYPE_TUPLE_H

#include "type_variable.h"

#include <tuple>
#include <type_traits>
#include <limits>

namespace ohm {
    template<typename T, size_t _Start, size_t _End = std::numeric_limits<size_t>::max(), typename Enable = void>
    struct sub_tuple;

    template<size_t _Start, size_t _End, typename... Args>
    struct sub_tuple<std::tuple<Args...>, _Start, _End,
            typename std::enable_if<
                    __GE(_Start, _End) || __GE(_Start, std::tuple_size<std::tuple<Args...>>::value)
            >::type> {
        using type = std::tuple<>;
    };

    template<size_t _Start, size_t _End, typename... Args>
    struct sub_tuple<std::tuple<Args...>, _Start, _End,
            typename std::enable_if<
                    __LT(_Start, _End) && __LT(_Start, std::tuple_size<std::tuple<Args...>>::value)
            >::type> {
        using _Tuple = std::tuple<Args...>;
        using type = decltype(std::tuple_cat(
                std::declval<std::tuple<typename std::tuple_element<_Start, _Tuple>::type>>(),
                std::declval<typename sub_tuple<_Tuple, _Start + 1, _End>::type>()
        ));
    };
}

#endif //OMEGA_TYPE_TUPLE_H
