//
// Created by kier on 2020/9/6.
//

#ifndef OMEGA_FORMAT_H
#define OMEGA_FORMAT_H

#include <string>
#include <vector>
#include <sstream>

namespace ohm {
    namespace _ {
        inline std::string::size_type
        FindDecollator(const std::string &str, const std::string &sep, std::string::size_type off) {
            if (off == std::string::npos) return std::string::npos;
            std::string::size_type i = off;
            for (; i < str.length(); ++i) {
                if (sep.find(str[i]) != std::string::npos) return i;
            }
            return std::string::npos;
        }
    }

    inline std::vector<std::string> split(const std::string &str, char ch, size_t size = 0) {
        std::vector<std::string> result;
        std::string::size_type left = 0, right;

        result.reserve(size);
        while (true) {
            right = str.find(ch, left);
            result.push_back(str.substr(left, right == std::string::npos ? std::string::npos : right - left));
            if (right == std::string::npos) break;
            left = right + 1;
        }
        return std::move(result);
    }

    inline std::vector<std::string> split(const std::string &str, const std::string &sep, size_t size = 0) {
        std::vector<std::string> result;
        std::string::size_type left = 0, right;

        result.reserve(size);
        while (true) {
            right = _::FindDecollator(str, sep, left);
            result.push_back(str.substr(left, right == std::string::npos ? std::string::npos : right - left));
            if (right == std::string::npos) break;
            left = right + 1;
        }
        return std::move(result);
    }

    inline std::string join(const std::vector<std::string> &list, const std::string &sep) {
        std::ostringstream oss;
        for (size_t i = 0; i < list.size(); ++i) {
            if (i) oss << sep;
            oss << list[i];
        }
        return oss.str();
    }

    namespace _ {
        template<typename T, typename=void>
        struct has_member_operator : public std::false_type {
        };

        template<typename T>
        struct has_member_operator<T, typename
        std::enable_if<std::is_constructible<
                decltype(std::declval<std::ostream>().operator<<(std::declval<T>())),
                std::ostream &>::value>::type> : public std::true_type {
        };

        template<typename T, typename=void>
        struct has_function_operator : public std::false_type {
        };

        template<typename T>
        struct has_function_operator<T, typename
        std::enable_if<std::is_constructible<
                decltype(operator<<(std::declval<std::ostream &>(), std::declval<T>())),
                std::ostream &>::value>::type> : public std::true_type {
        };

        template<typename T>
        struct is_output_streamable : public std::integral_constant<bool,
                has_member_operator<T>::value || has_function_operator<T>::value> {
        };

        inline std::ostream &scat(std::ostream &out) {
            return out;
        }

        template<typename T, typename=typename
        std::enable_if<is_output_streamable<T>::value>::type>
        inline std::ostream &scat(std::ostream &out, const T &t) {
            return out << t;
        }

        template<typename T, typename... Args, typename=typename
        std::enable_if<is_output_streamable<T>::value>::type>
        inline std::ostream &scat(std::ostream &out, const T &t, const Args &...args) {
            out << t;
            return scat(out, args...);
        }
    }

    template<typename... Args, typename=typename
    std::enable_if<std::is_constructible<
            decltype(_::scat(std::declval<std::ostream &>(),
                             std::declval<Args>()...)), std::ostream &>::value>::type>
    inline std::string concat(const Args &...args) {
        std::ostringstream oss;
        _::scat(oss, args...);
        return oss.str();
    }
}

#endif //OMEGA_FORMAT_H
