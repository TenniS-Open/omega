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
}

#endif //OMEGA_FORMAT_H
