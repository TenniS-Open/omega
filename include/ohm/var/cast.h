//
// Created by kier on 2020/8/24.
//

#ifndef OMEGA_VAR_CAST_H
#define OMEGA_VAR_CAST_H

#include <type_traits>
#include <string>
#include <climits>

namespace ohm {
    namespace notation {
        class String2Exception: public std::logic_error {
        public:
            using self = String2Exception;
            using supper = std::logic_error;

            explicit String2Exception(const std::string &msg) : supper(msg) {}
        };

        class String2NotConvertible : public String2Exception {
        public:
            using self = String2NotConvertible;
            using supper = String2Exception;

            explicit String2NotConvertible(const std::string &msg) : supper(msg) {}
        };

        class String2OutOfLimits : public String2Exception {
        public:
            using self = String2OutOfLimits;
            using supper = String2Exception;

            explicit String2OutOfLimits(const std::string &msg) : supper(msg) {}
        };

        template <typename T>
        inline T string2(const typename std::enable_if<std::is_integral<T>::value, std::string>::type &str) {
            const char *number_c_string = str.c_str();
            char *end_ptr = nullptr;
            auto value = std::strtoll(number_c_string, &end_ptr, 10);
            if (*end_ptr != '\0') {
                throw String2NotConvertible("Can not convert \"" + str + "\" to integer.");
            }
            if (value > std::numeric_limits<T>::max() || value < std::numeric_limits<T>::min()) {
                throw String2OutOfLimits("Converted \"" + str + "\" out of limits.");
            }
            return T(value);
        }
        template <typename T>
        inline T string2(const typename std::enable_if<std::is_floating_point<T>::value, std::string>::type &str) {
            const char *number_c_string = str.c_str();
            char *end_ptr = nullptr;
            double value = std::strtod(number_c_string, &end_ptr);
            if (*end_ptr != '\0') {
                throw String2NotConvertible("Can not convert \"" + str + "\" to floating point.");
            }
            if (value > std::numeric_limits<T>::max() || value < std::numeric_limits<T>::min()) {
                throw String2OutOfLimits("Converted \"" + str + "\" out of limits.");
            }
            return T(value);
        }
    }
}

#endif //OMEGA_VAR_CAST_H
