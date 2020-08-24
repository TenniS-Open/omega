//
// Created by kier on 2020/8/24.
//

#ifndef OMEGA_VAR_OSTREAM_H
#define OMEGA_VAR_OSTREAM_H

#include "var.h"
#include "stream.h"

#include <string>
#include <functional>

namespace ohm {
    namespace vario {
        inline size_t write_binary(const Var &var, const VarWriter &writer);

        inline size_t write_binary_type(const Var &var, const VarWriter &writer) {
            auto type = int16_t(var.type());
            return writer(&type, sizeof(type));
        }

        inline size_t write_binary_body(const void *t, size_t size, const VarWriter &writer) {
            return writer(t, size);
        }

        template<typename T>
        inline size_t write_binary_body(const T &t, const VarWriter &writer) {
            return writer(&t, sizeof(T));
        }

        template<>
        inline size_t write_binary_body<bool>(const bool &t, const VarWriter &writer) {
            uint8_t b = t ? 1 : 0;
            return writer(&b, 1);
        }

        template<>
        inline size_t write_binary_body<std::string>(const std::string &t, const VarWriter &writer) {
            Var size = int32_t(t.size());
            size_t writen = 0;
            writen += write_binary(size, writer);
            writen += writer(t.data(), t.size());
            return writen;
        }

        template<>
        inline size_t write_binary_body<std::vector<notation::Element::shared>>(
                const std::vector<notation::Element::shared> &t, const VarWriter &writer) {
            Var size = int32_t(t.size());
            size_t writen = 0;
            writen += write_binary(size, writer);
            for (auto &var : t) {
                writen += write_binary(Var::From(var), writer);
            }
            return writen;
        }

        template<>
        inline size_t write_binary_body<std::map<std::string, notation::Element::shared>>(
                const std::map<std::string, notation::Element::shared> &t, const VarWriter &writer) {
            Var size = int32_t(t.size());
            size_t writen = 0;
            writen += write_binary(size, writer);
            for (auto &pair : t) {
                writen += write_binary(Var(pair.first), writer);
                writen += write_binary(Var::From(pair.second), writer);
            }
            return writen;
        }

        inline size_t write_binary(const Var &var, const VarWriter &writer) {
            void *data;
            size_t size;
            var.unsafe(&data, &size);
            size_t writen = 0;
            writen += write_binary_type(var, writer);
            switch (var.type()) {
                case notation::type::Undefined:
                    break;
                case notation::type::None:
                    break;
                case notation::type::Boolean:
                    writen += write_binary_body<bool>(__ref<bool>(data), writer);
                    break;
                case notation::type::String:
                    writen += write_binary_body<std::string>(__ref<std::string>(data), writer);
                    break;
                case notation::type::Array:
                    writen += write_binary_body<notation::Array>(__ref<notation::Array>(data), writer);
                    break;
                case notation::type::Object:
                    writen += write_binary_body<notation::Object>(__ref<notation::Object>(data), writer);
                    break;
                case notation::type::Scalar:
                    writen += write_binary_body(data, size, writer);
                    break;
            }
            return writen;
        }
    }

    size_t write(const Var &var, const VarWriter &writer, VarFormat format = VarBinary, bool write_magic = false) {
        size_t writen = 0;
        if (format == VarBinary) {
            if (write_magic) {
                uint32_t fake = 0;
                uint32_t magic = var_magic();
                writen += vario::write_binary_body(fake, writer);
                writen += vario::write_binary_body(magic, writer);
            }
            writen += vario::write_binary(var, writer);
            return writen;
        } else {
            /// TODO: not implement
            return 0;
        }
    }
}

#endif //OMEGA_VAR_OSTREAM_H
