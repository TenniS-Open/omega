//
// Created by kier on 2020/8/23.
//

#ifndef OMEGA_VAR_DUMPS_H
#define OMEGA_VAR_DUMPS_H

#include "array.h"
#include "object.h"

#include <sstream>
#include <iomanip>

namespace ohm {
    namespace notation {
        inline std::string dumps(Element::shared element, const std::string &indent);

        inline std::string dumps(const std::string &str) {
            std::ostringstream oss;
            oss << "\"" << encode(str) << "\"";
            return oss.str();
        }

        inline std::string inline_dumps(const Array &arr) {
            return repr(arr);
        }

        inline std::string dumps(const Array &arr, const std::string &indent = "") {
            auto next = indent + "  ";
            if (arr.empty()) {
                return "[]";
            }

            // could inline
            bool could_inline = arr.size() < 16;
            if (could_inline) {
                for (const auto &v : arr) {
                    if (v->is_undefined() ||
                        v->is_null() ||
                        v->is_integer() ||
                        v->is_float() ||
                        v->is_scalar() ||
                        v->is_boolean()) {
                        continue;
                    } else {
                        could_inline = false;
                        break;
                    }
                }
            }
            if (could_inline) {
                auto s = inline_dumps(arr);
                if (s.size() <= 120) return s;
            }

            std::ostringstream oss;
            oss << "[" << std::endl << next;
            for (size_t i = 0; i < arr.size(); ++i) {
                oss << dumps(arr[i], next);
                if (i + 1 < arr.size()) {  // not last
                    oss << "," << std::endl << next;
                } else {
                    oss << std::endl << indent;
                }
            }
            oss << "]";
            return oss.str();
        }

        inline std::string dumps(const Object &obj, const std::string &indent = "") {
            auto next = indent + "  ";
            if (obj.empty()) {
                return "{}";
            }

            std::ostringstream oss;
            oss << "{" << std::endl << next;
            auto n = obj.size();
            decltype(n) i = 0;
            for (auto &kv : obj) {
                oss << '\"' << encode(kv.first) << "\": " << dumps(kv.second, next);
                ++i;
                if (i < n) {    // not last
                    oss << "," << std::endl << next;
                } else {
                    oss << std::endl << indent;
                }
            }
            oss << "}";
            return oss.str();
        }

        template<typename T, typename=typename std::enable_if<
                std::is_integral<T>::value || std::is_floating_point<T>::value>::type>
        inline std::string dumps(T t) {
            std::ostringstream oss;
            oss << t;
            return oss.str();
        }

        inline std::string dumps(bool b) {
            return b ? "true" : "false";
        }

        inline std::string dumps(DataType code, const void *data, size_t size) {
            std::ostringstream oss;
            oss << "\"@" << type_string(code);
            if (data && size > 0) {
                auto int8_data = reinterpret_cast<const int8_t *>(data);
                oss << "@0x";
                oss << std::hex << std::setfill('0') << std::setw(2);
                for (size_t i = 0; i < size; ++i) {
                    oss << int(int8_data[i]);
                }
            }
            oss << "\"";
            return oss.str();
        }

        template<typename T>
        inline typename std::enable_if<
                !std::is_integral<T>::value &&
                !std::is_floating_point<T>::value, std::string>::type
        scalar_dumps(const T &t) {
            return dumps(type::Scalar | sub_type_code<T>::code, &t, element_size<T>());
        }

        template<typename T>
        inline typename std::enable_if<
                std::is_integral<T>::value ||
                std::is_floating_point<T>::value, std::string>::type
        scalar_dumps(const T &t) {
            return dumps(t);
        }

        inline std::string dumps(const Binary &bin) {
            std::ostringstream oss;
            oss << "\"@binary@" << bin.size() << "\"";
            return oss.str();
        }

        template<typename T>
        inline typename std::enable_if<
                !std::is_integral<T>::value &&
                !std::is_floating_point<T>::value, std::string>::type
        dumps(const T *vector, size_t size, const std::string &indent = "") {
            auto next = indent + "  ";
            if (!size) {
                return "[]";
            }

            std::ostringstream oss;
            oss << "[" << std::endl << next;
            for (size_t i = 0; i < size; ++i) {
                oss << dumps(type_code<T>::code, &vector[i], element_size<T>());
                if (i + 1 < size) {  // not last
                    oss << "," << std::endl << next;
                } else {
                    oss << std::endl << indent;
                }
            }
            oss << "]";
            return oss.str();
        }

        template<typename T>
        inline typename std::enable_if<
                std::is_integral<T>::value ||
                std::is_floating_point<T>::value, std::string>::type
        dumps(const T *vector, size_t size, const std::string &indent = "") {
            auto next = indent + "  ";
            if (!size) {
                return "[]";
            }

            std::ostringstream oss;
            oss << "[" << std::endl << next;
            for (size_t i = 0; i < size; ++i) {
                oss << vector[i];
                if (i + 1 < size) {  // not last
                    oss << "," << std::endl << next;
                } else {
                    oss << std::endl << indent;
                }
            }
            oss << "]";
            return oss.str();
        }
    }
}

#endif //OMEGA_VAR_DUMPS_H
