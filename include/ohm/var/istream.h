//
// Created by kier on 2020/8/24.
//

#ifndef OMEGA_VAR_ISTREAM_H
#define OMEGA_VAR_ISTREAM_H

#include "var.h"
#include "stream.h"

#include <string>
#include <functional>
#include <stack>
#include <cstdio>

namespace ohm {
    namespace vario {
        class Context {
        public:
            Context(size_t size = 64) {
                if (size <= 0) size = 1;
                m_data.resize(m_size);
                m_data[0] = '\0';
                m_size = size;
            }

            Context(const Context &) = delete;
            Context &operator=(const Context &) = delete;

            void push(const std::string &seg) {
                auto base = m_stack.empty() ? 0 : m_stack.top();
                auto next = base + seg.size();
                if (next > m_size) {
                    do {
                        m_size *= 2;
                    } while (next > m_size);
                    m_data.resize(m_size);
                }
                std::snprintf(m_data.data() + base, m_size - base, "%s", seg.c_str());
                m_stack.push(next);
            }

            template <typename T>
            static void pack(std::ostream &out, T &&t) {
                out << t;
            }

            template <typename T, typename...Args>
            static void pack(std::ostream &out, T &&t, Args &&...args) {
                out << t;
                pack(out, std::forward<Args>(args)...);
            }

            template <typename T, typename... Args>
            void push(T &&t, Args &&...args) {
                std::ostringstream oss;
                pack(oss, std::forward<T>(t), std::forward<Args>(args)...);
                push(oss.str());
            }

            void pop() {
                m_stack.pop();
                auto base = m_stack.empty() ? 0 : m_stack.top();
                m_data[base] = '\0';
            }

            std::string str() const {
                return m_data.data();
            }

            operator std::string() const {
                return this->str();
            }

        private:
            std::vector<char> m_data;
            std::stack<size_t> m_stack;
            size_t m_size;
        };

        inline Var read_var(Context &ctx, const VarReader &reader);

        template <typename T>
        inline T read(Context &ctx, const VarReader &reader) {
            T tmp;
            auto read = reader(&tmp, sizeof(tmp));
            if (read != sizeof(tmp)) {
                throw VarIOExcpetion(ctx);
            }
            return tmp;
        }

        template <typename T>
        inline T *read_buffer(Context &ctx, T *data, size_t wanted, const VarReader &reader) {
            auto size = sizeof(T) * wanted;
            auto read = reader(data, size);
            if (read != size) {
                throw VarIOExcpetion(ctx);
            }
            return data;
        }

        inline void *read_buffer(Context &ctx, void *data, size_t wanted, const VarReader &reader) {
            auto size = wanted;
            auto read = reader(data, size);
            if (read != size) {
                throw VarIOExcpetion(ctx);
            }
            return data;
        }

        inline Var read_bool(Context &ctx, const VarReader &reader) {
            auto b = read<uint8_t>(ctx, reader);
            return Var(b != 0);
        }

        template <typename T, typename=typename std::enable_if<is_var_convertible<T>::value>::type>
        inline T expect(Context &ctx, const std::string &expected, const Var &var) {
            try {
                return T(var);
            } catch (const VarOperatorNotSupported &) {
                throw VarIOUnexpectedType(ctx, expected, var.type());
            }
        }

        inline Var read_string(Context &ctx, const VarReader &reader) {
            auto size = expect<size_t>(ctx, "integer", read_var(ctx, reader));
            std::vector<char> buffer(size);
            read_buffer(ctx, buffer.data(), size, reader);
            return Var(std::string(buffer.data()));
        }

        inline Var read_array(Context &ctx, const VarReader &reader) {
            auto size = expect<size_t>(ctx, "integer", read_var(ctx, reader));
            std::vector<Var> array;
            for (size_t i = 0; i < size; ++i) {
                ctx.push("[", i, "]");
                array.push_back(read_var(ctx, reader));
                ctx.pop();
            }
            return array;
        }

        inline Var read_var(Context &ctx, const VarReader &reader) {
            auto datatype = read<notation::DataType>(ctx, reader);
            switch (datatype) {
                case notation::type::Undefined:
                    return Var();
                case notation::type::None:
                    return Var(nullptr);
                case notation::type::Boolean:
                    return read_bool(ctx, reader);
                case notation::type::String:
                    return read_string(ctx, reader);
                case notation::type::Array:
                    return read_array(ctx, reader);
//                case notation::type::Object:
//                    break;
//                case notation::type::Scalar:
//                    break;
            }
        }
    }

}

#endif //OMEGA_VAR_ISTREAM_H
