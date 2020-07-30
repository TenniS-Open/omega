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

    template<typename Ret, typename FUNC>
    class __VoidOperatorBinder {
    public:
        static VoidOperator bind(FUNC func) { return [func]() -> void { func(); }; }
    };

    template<typename FUNC>
    class __VoidOperatorBinder<void, FUNC> {
    public:
        static VoidOperator bind(FUNC func) { return func; }
    };

    template<typename FUNC, typename... Args>
    inline typename std::enable_if<can_be_bind<FUNC, Args...>::value, VoidOperator>::type
    void_bind(FUNC func, Args &&... args) {
        auto inner_func = std::bind(func, std::forward<Args>(args)...);
        using Ret = decltype(inner_func());
        using RetOperator = __VoidOperatorBinder<Ret, decltype(inner_func)>;
        return RetOperator::bind(inner_func);
    }

    template<typename FUNC, typename... Args,
            typename = typename std::enable_if<can_be_bind<FUNC, Args...>::value>::type>
    inline void void_call(FUNC func, Args &&... args) {
        void_bind(func, std::forward<Args>(args)...)();
    };
}

#endif //OMEGA_VOID_BIND_H
