//
// Created by kier on 2020/7/31.
//

#ifndef OMEGA_ASSERT_H
#define OMEGA_ASSERT_H

#include "print.h"
#include "logger.h"

namespace ohm {
    /**
     * This print is work for assert, incase auto generate never achive codes.
     * well, now version assert compile speed very slow. OMG.
     */
    template<>
    struct printable<Logger> {
        using type = Logger;

        void print(std::ostream &out, const type &x) {}
    };

    template <typename T>
    inline auto __forward_assert_logger(T &&t) -> decltype(std::forward<T>(t)) {
        return std::forward<T>(t);
    }

    inline auto __forward_assert_logger(const Logger &logger) -> LogStream {
        return logger(LOG_ERROR);
    }

    inline auto __forward_assert_logger(Logger &logger) -> LogStream {
        return logger(LOG_ERROR);
    }
}

#define __ohm_assert(cond, cont, log, ...) \
    if (!(cond)) do { \
        auto constexpr ohm_auto_name(__is_logger) = \
                std::is_convertible<decltype(log), const ohm::Logger&>::value; \
        if (ohm_auto_name(__is_logger)) { \
            ohm_log(ohm::__forward_assert_logger(log), "Assertion failed: ", "(" #cont ")", \
            ". ", ## __VA_ARGS__); \
        } else { \
            ohm_log(ohm::LOG_ERROR, "Assertion failed: ", "(" #cont, ")", \
            ". ", ## __VA_ARGS__); \
        } \
    } while (false)

#define ohm_assert(cond, log, ...) __ohm_assert(cond, cond, log, ## __VA_ARGS__)

#define ohm_assert_eq(a, b, log, ...) __ohm_assert((a) == (b), a == b, log, ## __VA_ARGS__)
#define ohm_assert_ne(a, b, log, ...) __ohm_assert((a) != (b), a != b, log, ## __VA_ARGS__)
#define ohm_assert_gt(a, b, log, ...) __ohm_assert((a) > (b), a > b, log, ## __VA_ARGS__)
#define ohm_assert_lt(a, b, log, ...) __ohm_assert((a) < (b), a < b, log, ## __VA_ARGS__)
#define ohm_assert_ge(a, b, log, ...) __ohm_assert((a) >= (b), a >= b, log, ## __VA_ARGS__)
#define ohm_assert_le(a, b, log, ...) __ohm_assert((a) <= (b), a <= b, log, ## __VA_ARGS__)

#endif //OMEGA_ASSERT_H
