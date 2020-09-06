//
// Created by kier on 2020/8/24.
//

#ifndef OMEGA_VAR_OSTREAM_H
#define OMEGA_VAR_OSTREAM_H

#include "var.h"
#include "stream.h"
#include <fstream>

#include <string>
#include <functional>

namespace ohm {
    namespace vario {
        inline size_t write_var(const Var &var, const VarWriter &writer);

        inline size_t write_var_type(const Var &var, const VarWriter &writer) {
            auto type = int16_t(var.type());
            return writer(&type, sizeof(type));
        }

        inline size_t write_var_body(const void *t, size_t size, const VarWriter &writer) {
            return writer(t, size);
        }

        template<typename T>
        inline size_t write_var_body(const T &t, const VarWriter &writer) {
            return writer(&t, sizeof(T));
        }

        template<>
        inline size_t write_var_body<bool>(const bool &t, const VarWriter &writer) {
            uint8_t b = t ? 1 : 0;
            return writer(&b, 1);
        }

        template<>
        inline size_t write_var_body<std::string>(const std::string &t, const VarWriter &writer) {
            Var size = int32_t(t.size());
            size_t writen = 0;
            writen += write_var(size, writer);
            writen += writer(t.data(), t.size());
            return writen;
        }

        template<>
        inline size_t write_var_body<std::vector<notation::Element::shared>>(
                const std::vector<notation::Element::shared> &t, const VarWriter &writer) {
            Var size = int32_t(t.size());
            size_t writen = 0;
            writen += write_var(size, writer);
            for (auto &var : t) {
                writen += write_var(Var::From(var), writer);
            }
            return writen;
        }

        template<>
        inline size_t write_var_body<std::map<std::string, notation::Element::shared>>(
                const std::map<std::string, notation::Element::shared> &t, const VarWriter &writer) {
            Var size = int32_t(t.size());
            size_t writen = 0;
            writen += write_var(size, writer);
            for (auto &pair : t) {
                writen += write_var(Var(pair.first), writer);
                writen += write_var(Var::From(pair.second), writer);
            }
            return writen;
        }

        inline size_t write_var_binary(const void *t, size_t size, const VarWriter &writer) {
            size_t writen = 0;
            writen += write_var(Var(int32_t(size)), writer);
            writen += writer(t, size);
            return writen;
        }

        inline size_t write_var_vector(notation::DataType type, const void *t, size_t size, const VarWriter &writer) {
            size_t writen = 0;
            writen += write_var(Var(int32_t(
                    size / notation::sub_type_size(notation::type::SubType(type & 0xFF)))), writer);
            writen += writer(t, size);
            return writen;
        }

        inline size_t write_var(const Var &var, const VarWriter &writer) {
            void *data;
            size_t size;
            var.unsafe(&data, &size);
            size_t writen = 0;
            writen += write_var_type(var, writer);
            switch (var.type() & 0xFF00) {
                case notation::type::Undefined:
                    break;
                case notation::type::None:
                    break;
                case notation::type::Boolean:
                    writen += write_var_body<bool>(__ref<bool>(data), writer);
                    break;
                case notation::type::String:
                    writen += write_var_body<std::string>(__ref<std::string>(data), writer);
                    break;
                case notation::type::Array:
                    writen += write_var_body<notation::Array>(__ref<notation::Array>(data), writer);
                    break;
                case notation::type::Object:
                    writen += write_var_body<notation::Object>(__ref<notation::Object>(data), writer);
                    break;
                case notation::type::Scalar:
                    writen += write_var_body(data, size, writer);
                    break;
                case notation::type::Binary:
                    writen += write_var_binary(data, size, writer);
                    break;
                case notation::type::Vector:
                    writen += write_var_vector(var.type(), data, size, writer);
                    break;
            }
            return writen;
        }
    }

    namespace var {
        /**
         * write binary format into stream, chose to write magic number or not
         * @param var var ready to write
         * @param writer write stream
         * @param write_magic if write magic number
         * @return number of writen bytes
         */
        size_t write(const Var &var, const VarWriter &writer, bool write_magic = false) {
            size_t writen = 0;
            if (write_magic) {
                uint32_t fake = 0;
                uint32_t magic = var_magic();
                writen += vario::write_var_body(fake, writer);
                writen += vario::write_var_body(magic, writer);
            }
            writen += vario::write_var(var, writer);
            return writen;
        }
    }
}

#endif //OMEGA_VAR_OSTREAM_H
