//
// Created by kier on 2020/8/20.
//

#ifndef OMEGA_VAR_STRING_H
#define OMEGA_VAR_STRING_H

#include "notation.h"

#include <string>
#include <sstream>
#include <unordered_map>

namespace ohm {
    namespace notation {
        using String = std::string;

        using StringBase = ElementBase<type::String, String>;

        template<>
        struct type_code<std::string> {
            static const DataType code = type::String;
        };
        template<>
        struct code_type<type::String> {
            using type = StringBase;
        };

        inline std::string encode(const std::string &str) {
            std::ostringstream oss;
            static const std::unordered_map<char, std::string> escape = {
                    {'\"', R"(\")"},
                    {'\\', R"(\\)"},
                    {'/', R"(/)"},
                    {'\b', R"(\b)"},
                    {'\f', R"(\f)"},
                    {'\n', R"(\n)"},
                    {'\r', R"(\r)"},
                    {'\t', R"(\t)"},
            };
            for (auto &ch : str) {
                auto it = escape.find(ch);
                if (it != escape.end()) {
                    oss << it->second;
                } else {
                    oss << ch;
                }
            }
            return oss.str();
        }
    }
}

#endif //OMEGA_VAR_STRING_H
