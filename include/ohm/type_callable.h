//
// Created by kier on 2020/7/21.
//

#ifndef OMEGA_TYPE_CALLABLE_H
#define OMEGA_TYPE_CALLABLE_H

#include <type_traits>
#include <functional>

namespace ohm {
    template<typename T, typename ...Args>
    struct has_operator_implicit_arguments {
    private:
        template<typename U>
        static auto Check(int) -> decltype(std::declval<U>().operator()(std::declval<Args>()...), std::true_type());
        template<typename U>
        static auto Check(...) -> decltype(std::false_type());
    public:
        static constexpr bool value = std::is_same<decltype(Check<T>(0)), std::true_type>::value;
    };

    template<typename T, typename FUNC, typename Enable = void>
    struct is_implicit_callable_object : std::false_type {};

    template<typename T, typename RET, typename ...Args>
    struct is_implicit_callable_object<T, RET(
            Args...), typename std::enable_if<has_operator_implicit_arguments<T, Args...>::value>::type> {
    private:
        using T_RET = decltype(std::declval<T>().operator()(std::declval<Args>()...));
    public:
        static constexpr bool value = std::is_same<T_RET, RET>::value || std::is_convertible<T_RET, RET>::value;
    };
    template<typename T, typename FUNC, typename Enable = void>
    struct is_callable_object : std::false_type {};

    template<typename T, typename RET, typename ...Args>
    struct is_callable_object<T, RET(Args...), typename std::enable_if<std::is_class<T>::value>::type> {
    private:
        template<typename U>
        static auto Check(int) -> decltype((RET(T::*)(Args...)) (&U::operator()), std::true_type());
        template<typename U>
        static auto Check(...) -> decltype(std::false_type());
    public:
        static constexpr bool value = std::is_same<decltype(Check<T>(0)), std::true_type>::value;
    };

    template<typename T, typename FUNC, typename Enable = void>
    struct is_callable_function : std::false_type {};

    template<typename T, typename RET, typename ...Args>
    struct is_callable_function<T, RET(Args...), void>
            : std::integral_constant<bool, std::is_same<T, RET(*)(Args...)>::value> {};

    template<typename FUNC>
    struct is_function_pointer : std::false_type {};

    template<typename RET, typename ...Args>
    struct is_function_pointer<RET(*)(Args...)> : std::true_type {};

    template<typename T, typename FUNC, typename Enable = void>
    struct is_implicit_callable_function : std::false_type {};

    template<typename T, typename RET, typename ...Args>
    struct is_implicit_callable_function<T, RET(Args...), typename std::enable_if<is_function_pointer<T>::value>::type>
            : std::integral_constant<bool,
            is_implicit_callable_object<std::function<typename std::remove_pointer<T>::type>, RET(Args...)>::value> {};

    template<typename T, typename FUNC>
    struct is_callable
            : std::integral_constant<bool,
                    is_callable_function<T, FUNC>::value ||
                    is_callable_object<T, FUNC>::value> {
    };
    template<typename T, typename FUNC>
    struct is_implicit_callable
            : std::integral_constant<bool,
                    is_implicit_callable_function<T, FUNC>::value ||
                    is_implicit_callable_object<T, FUNC>::value> {
    };
}

#endif //OMEGA_TYPE_CALLABLE_H
