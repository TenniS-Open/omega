//
// Created by kier on 2020/7/29.
//

#ifndef OMEGA_TYPE_VARIABLE_H
#define OMEGA_TYPE_VARIABLE_H

namespace ohm {
    inline constexpr bool __GT(size_t a, size_t b) { return a > b; }

    inline constexpr bool __LT(size_t a, size_t b) { return a < b; }

    inline constexpr bool __GE(size_t a, size_t b) { return a >= b; }

    inline constexpr bool __LE(size_t a, size_t b) { return a <= b; }

    template <typename... Args>
    struct variable_count;

    template <>
    struct variable_count<> {
        static constexpr size_t value = 0;
    };

    template <typename T, typename... Args>
    struct variable_count<T, Args...> {
        static constexpr size_t value = 1 + variable_count<Args...>::value;
    };
}

#endif //OMEGA_TYPE_VARIABLE_H
