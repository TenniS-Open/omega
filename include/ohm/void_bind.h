//
// Created by kier on 2020/7/31.
//

#ifndef OMEGA_VOID_BIND_H
#define OMEGA_VOID_BIND_H

#include <functional>
#include "type_callable.h"

namespace ohm {
    template<typename T, typename ...Args>
    struct can_be_bind {
    private:
        template<typename U>
        static auto Type(int) -> decltype(std::bind(std::declval<U>(), std::declval<Args>()...)());

        template<typename U>
        static auto Type(...) -> void;

        template<typename U>
        static auto Check(int) -> decltype(std::bind(std::declval<U>(), std::declval<Args>()...)(), std::true_type());

        template<typename U>
        static auto Check(...) -> decltype(std::false_type());

    public:
        static constexpr bool value = std::is_same<decltype(Check<T>(0)), std::true_type>::value;
        using return_type = decltype(Type<T>(0));
    };

    using VoidOperator = std::function<void()>;

    template<typename FUNC, typename... Args>
    inline typename std::enable_if<
            can_be_bind<FUNC, Args...>::value &&
            std::is_same<typename can_be_bind<FUNC, Args...>::return_type, void>::value,
            VoidOperator>::type
    void_bind(FUNC func, Args &&... args) {
        return std::bind(func, std::forward<Args>(args)...);
    }

    template<typename FUNC, typename... Args>
    inline typename std::enable_if<
            can_be_bind<FUNC, Args...>::value &&
            !std::is_same<typename can_be_bind<FUNC, Args...>::return_type, void>::value,
            VoidOperator>::type
    void_bind(FUNC func, Args &&... args) {
        auto void_func = std::bind(func, std::forward<Args>(args)...);
        /// TODO: const cast is not popular method, just to fix some dangerous action and be avoid of obscure error.
        return [void_func]() -> void { const_cast<decltype(void_func)&>(void_func)(); };
    }

    template<typename FUNC, typename... Args,
            typename = typename std::enable_if<can_be_bind<FUNC, Args...>::value>::type>
    inline void void_call(FUNC func, Args &&... args) {
        void_bind(func, std::forward<Args>(args)...)();
    };
}

#endif //OMEGA_VOID_BIND_H
