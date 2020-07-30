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

namespace ohm {
    using time_point = decltype(std::chrono::system_clock::now());

    inline struct tm time2tm(std::time_t from) {
        std::tm to = {0};
#if OHM_PLATFORM_CC_MSVC
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
}

#endif //OMEGA_TIME_H
