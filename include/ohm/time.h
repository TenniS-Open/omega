//
// Created by kier on 2020/7/30.
//

#ifndef OMEGA_TIME_H
#define OMEGA_TIME_H

#include "platform.h"
#include "print.h"

#include <string>
#include <chrono>
#include <ctime>
#include <iomanip>

namespace ohm {
    using time_point = decltype(std::chrono::system_clock::now());

    inline struct tm time2tm(std::time_t from) {
        std::tm to = {0};
#if OHM_PLATFORM_CC_MSVC || OHM_PLATFORM_CC_MINGW
        localtime_s(&to, &from);
#else
        localtime_r(&from, &to);
#endif
        return to;
    }

    inline std::string to_string(time_point tp, const std::string &format = "%Y-%m-%d %H:%M:%S") {
        std::time_t tt = std::chrono::system_clock::to_time_t(tp);
        std::tm even = time2tm(tt);
        char tmp[64];
        std::strftime(tmp, sizeof(tmp), format.c_str(), &even);
        return std::string(tmp);
    }

    inline time_point now() { return std::chrono::system_clock::now(); }

    template<>
    struct printable<time_point> {
        using type = time_point;

        void print(std::ostream &out, const type &x) {
            out << to_string(x);
        }
    };

    namespace time {
        using us = std::chrono::microseconds;
        using ms = std::chrono::milliseconds;
        using ns = std::chrono::nanoseconds;
        using sec = std::chrono::seconds;
        using min = std::chrono::minutes;
        using hour = std::chrono::hours;
        using day = std::chrono::duration<int, std::ratio_multiply<std::ratio<24>, hour::period>>;
        using week = std::chrono::duration<int, std::ratio_multiply<std::ratio<7>, day::period>>;

        template<typename T, typename _Rep, typename _Period>
        auto count(const std::chrono::duration<_Rep, _Period> &duration)
        -> typename std::enable_if<
                std::is_arithmetic<decltype(std::chrono::duration_cast<T>(duration).count())>::value,
                decltype(std::chrono::duration_cast<T>(duration).count())>::type {
            return std::chrono::duration_cast<T>(duration).count();
        }
    }

    template<typename _Rep, typename _Period>
    struct printable<std::chrono::duration<_Rep, _Period>> {
        using type = std::chrono::duration<_Rep, _Period>;

        void print(std::ostream &out, const type &x) {
            auto ms = time::count<time::ms>(x);
            if (ms < 1000) {
                auto us = time::count<time::us>(x);
                auto ns = time::count<time::ns>(x);
                if (us == 0 && ns != 0) {
                    out << ns << "ns";
                    return;
                }
                out << us / 1000.f << "ms";
                return;
            }
            if (ms < 60000) {
                out << ms / 1000.f << "s";
                return;
            }
            // now got over minute
            auto sec = time::count<time::sec>(x);
            auto d = sec / 86400;
            auto h = sec % 86400 / 3600;
            auto m = sec % 3600 / 60;
            auto s = sec % 60;
            if (d) { out << d << "d"; }
            if (h) { out << h << "h"; }
            if (m) { out << m << "m"; }
            if (s) { out << s << "s"; }
        }
    };
}

#endif //OMEGA_TIME_H
