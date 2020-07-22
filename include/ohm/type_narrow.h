//
// Created by kier on 2020/7/22.
//

#ifndef OMEGA_TYPE_NARROW_H
#define OMEGA_TYPE_NARROW_H

#include <type_traits>

namespace ohm {
    template<typename T, typename K, typename Enable=void>
    struct narrow_type {
        using type = void;
    };

    template<typename T, typename K>
    struct narrow_type<T, K, typename std::enable_if<
            std::is_arithmetic<T>::value && std::is_arithmetic<K>::value
    >::type> {
    public:
        using type = decltype(std::declval<T>() + std::declval<K>());
    };

    template<typename T, typename K>
    struct narrow_type<T, K, typename std::enable_if<
            !(std::is_arithmetic<T>::value && std::is_arithmetic<K>::value) &&
            std::is_convertible<K, T>::value
    >::type> {
    public:
        using type = T;
    };

    template<typename T, typename K>
    struct narrow_type<T, K, typename std::enable_if<
            !(std::is_arithmetic<T>::value && std::is_arithmetic<K>::value) &&
            !std::is_convertible<K, T>::value &&
            std::is_convertible<T, K>::value
    >::type> {
    public:
        using type = K;
    };

    template<typename T, typename... Args>
    struct serial_narrow_type {
    public:
        using type = typename narrow_type<T, typename serial_narrow_type<Args...>::type>::type;
    };

    template<typename T>
    struct serial_narrow_type<T> {
    public:
        using type = T;
    };
}

#endif //OMEGA_TYPE_NARROW_H
