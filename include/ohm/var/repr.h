//
// Created by kier on 2020/8/23.
//

#ifndef OMEGA_VAR_REPR_H
#define OMEGA_VAR_REPR_H

#include "array.h"
#include "object.h"

#include <sstream>
#include <iomanip>

namespace ohm {
    namespace notation {
        inline std::string repr(Element::shared element);

        inline std::string repr(const std::string &str) {
            std::ostringstream oss;
            oss << "\"" << encode(str) << "\"";
            return oss.str();
        }

        inline std::string repr(const Array &arr) {
            std::ostringstream oss;
            oss << "[";
            for (size_t i = 0; i < arr.size(); ++i) {
                if (i) oss << ", ";
                oss << repr(arr[i]);
            }
            oss << "]";
            return oss.str();
        }

        inline std::string repr(const Object &obj) {
            bool comma = false;
            std::ostringstream oss;
            oss << "{";
            for (auto &kv : obj) {
                if (comma) oss << ", ";
                else comma = true;
                oss << '\"' << kv.first << "\": " << repr(kv.second);
            }
            oss << "}";
            return oss.str();
        }

        template<typename T, typename=typename std::enable_if<
                std::is_integral<T>::value || std::is_floating_point<T>::value>::type>
        inline std::string repr(T t) {
            std::ostringstream oss;
            oss << t;
            return oss.str();
        }

        inline std::string repr(bool b) {
            return b ? "true" : "false";
        }

        inline std::string repr(DataType code, const void *data, size_t size) {
            std::ostringstream oss;
            oss << "\"@" << type_string(code);
            if (data && size > 0) {
                auto uint8_data = reinterpret_cast<const uint8_t *>(data);
                oss << "@0x";
                oss << std::hex << std::setfill('0') << std::setw(2);
                for (size_t i = 0; i < size; ++i) {
                    oss << uint8_data[i];
                }
            }
            oss << "\"";
            return oss.str();
        }

        inline std::string repr(const Binary &bin) {
            std::ostringstream oss;
            oss << "\"@binary@" << bin.size() << "\"";
            return oss.str();
        }

        template<typename T>
        inline typename std::enable_if<
                !std::is_integral<T>::value &&
                !std::is_floating_point<T>::value, std::string>::type
        repr(const T *vector, size_t size) {
            std::ostringstream oss;
            oss << "[";
            for (size_t i = 0; i < size; ++i) {
                if (i) oss << ", ";
                oss << repr(type_code<T>::code, &vector[i], element_size<T>());
            }
            oss << "]";
            return oss.str();
        }

        template<typename T>
        inline typename std::enable_if<
                std::is_integral<T>::value ||
                std::is_floating_point<T>::value, std::string>::type
        repr(const T *vector, size_t size) {
            std::ostringstream oss;
            oss << "[";
            for (size_t i = 0; i < size; ++i) {
                if (i) oss << ", ";
                oss << vector[i];
            }
            oss << "]";
            return oss.str();
        }
    }
}

#endif //OMEGA_VAR_REPR_H
