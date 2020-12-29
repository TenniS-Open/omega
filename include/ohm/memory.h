//
// Created by kier on 2020/7/30.
//

#ifndef OMEGA_MEMORY_H
#define OMEGA_MEMORY_H

#include <string>
#include <cstddef>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <cstring>

namespace ohm {
    inline std::string mstring(uint64_t memory_size) {
        static const char *base[] = {"B", "KB", "MB", "GB", "TB"};
        static const uint64_t base_size = sizeof(base) / sizeof(base[0]);
        auto number = double(memory_size);
        uint64_t base_time = 0;
        while (number >= 1024.0 && base_time + 1 < base_size) {
            number /= 1024.0;
            base_time++;
        }
        number = std::round(number * 10.0) / 10.0;
        std::ostringstream oss;
        if (uint64_t(number * 10) % 10 == 0) {
            oss << std::fixed << std::setprecision(0) << number << base[base_time];
        } else {
            oss << std::fixed << std::setprecision(1) << number << base[base_time];
        }
        return oss.str();
    }

    inline void *datacopy(void *dst, const void *src, size_t n) {
#if defined(_MSC_VER) && _MSC_VER >= 1400
        return memcpy_s(dst, n, src, n);
#else
        return memcpy(dst, src, n);
#endif
    }
}

#endif //OMEGA_MEMORY_H
