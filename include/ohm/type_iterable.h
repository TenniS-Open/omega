//
// Created by kier on 2020/7/21.
//

#ifndef OMEGA_TYPE_ITERABLE_H
#define OMEGA_TYPE_ITERABLE_H

#include <iterator>
#include "type_cast.h"

namespace ohm {
    template<typename T>
    struct has_begin {
    private:
        template<typename U>
        static auto Check(int) -> decltype(std::declval<U>().begin(), std::true_type());

        template<typename U>
        static auto Check(...) -> decltype(std::false_type());

        template<typename U>
        static auto Type(int) -> decltype(std::declval<U>().begin());

        template<typename U>
        static auto Type(...) -> void;

    public:
        using forward_type = decltype(Type<T>(0));
        using type = typename std::decay<forward_type>::type;
        static constexpr bool value = std::is_same<decltype(Check<T>(0)), std::true_type>::value;
    };

    template<typename T>
    struct has_end {
    private:
        template<typename U>
        static auto Check(int) -> decltype(std::declval<U>().end(), std::true_type());

        template<typename U>
        static auto Check(...) -> decltype(std::false_type());

        template<typename U>
        static auto Type(int) -> decltype(std::declval<U>().end());

        template<typename U>
        static auto Type(...) -> void;

    public:
        using forward_type = decltype(Type<T>(0));
        using type = typename std::decay<forward_type>::type;
        static constexpr bool value = std::is_same<decltype(Check<T>(0)), std::true_type>::value;
    };

    template<class _Tp>
    struct has_iterator_category {
    private:
        struct __two {
            char __lx;
            char __lxx;
        };

        template<class _Up>
        static __two __test(...);

        template<class _Up>
        static char __test(typename _Up::iterator_category * = 0);

    public:
        static const bool value = sizeof(__test<_Tp>(0)) == 1;
    };

    template<class _Tp, class _Up, bool = has_iterator_category<std::iterator_traits<_Tp> >::value>
    struct has_iterator_category_convertible_to
            : public std::integral_constant<bool, std::is_convertible<typename std::iterator_traits<_Tp>::iterator_category, _Up>::value> {
    };

    template<class _Tp, class _Up>
    struct has_iterator_category_convertible_to<_Tp, _Up, false> : public std::false_type {
    };

    template<typename T, typename Tag>
    struct has_iterator_tag : has_iterator_category_convertible_to<T, Tag> {
    };

    template<typename T>
    struct is_iterable : public std::integral_constant<bool,
            std::is_same<typename has_begin<T>::type, typename has_end<T>::type>::value &&
            has_iterator_tag<typename has_begin<T>::type, std::input_iterator_tag>::value> {
    };

    template<typename T, typename Enable = void>
    struct has_iterator {
    public:
//        using forward_value_type = void;
//        using value_type = void;
//        static constexpr bool value = false;
    };

    template<typename T>
    struct has_iterator<T, typename std::enable_if<is_iterable<T>::value>::type> {
    public:
        static constexpr bool value = true;
        using forward_value_type = decltype(*std::declval<T>().begin());
        using value_type = typename std::decay<forward_value_type>::type;
    };


    template<typename... Args>
    struct is_all_iterable : public std::false_type {
    };

    template<typename T>
    struct is_all_iterable<T> : public std::integral_constant<bool, is_iterable<T>::value> {
    };

    template<typename T, typename... Args>
    struct is_all_iterable<T, Args...>
            : public std::integral_constant<bool, is_iterable<T>::value && is_all_iterable<Args...>::value> {
    };
}

#endif //OMEGA_TYPE_ITERABLE_H
