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
                m_size = size;
                m_data.resize(m_size);
                m_data[0] = '\0';
            }

            Context(const Context &) = delete;
            Context &operator=(const Context &) = delete;

            void push(const std::string &seg) {
                push_seg(seg);
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
                push_seg(oss.str());
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
            void push_seg(const std::string &seg) {
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
                throw VarIOEndOfStream(ctx);
            }
            return tmp;
        }

        template <>
        inline void read<void>(Context &ctx, const VarReader &reader) {
        }

        template <typename T>
        inline T *read_buffer(Context &ctx, T *data, size_t wanted, const VarReader &reader) {
            auto size = sizeof(T) * wanted;
            auto read = reader(data, size);
            if (read != size) {
                throw VarIOEndOfStream(ctx);
            }
            return data;
        }

        inline void *read_buffer(Context &ctx, void *data, size_t wanted, const VarReader &reader) {
            auto size = wanted;
            auto read = reader(data, size);
            if (read != size) {
                throw VarIOEndOfStream(ctx);
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
            std::vector<notation::Element::shared> array;
            for (size_t i = 0; i < size; ++i) {
                ctx.push("[", i, "]");
                array.push_back(read_var(ctx, reader));
                ctx.pop();
            }
            return Var(std::move(array));
        }

        inline Var read_object(Context &ctx, const VarReader &reader) {
            auto size = expect<size_t>(ctx, "integer", read_var(ctx, reader));
            std::map<std::string, notation::Element::shared> object;
            for (size_t i = 0; i < size; ++i) {
                auto key = expect<std::string>(ctx, "string", read_var(ctx, reader));
                ctx.push(".", key);
                auto value = read_var(ctx, reader);
                ctx.pop();
                object[key] = value._element();
            }
            return Var(std::move(object));
        }

        inline Var read_scalar(Context &ctx, notation::DataType type, const VarReader &reader) {
#pragma push_macro("READ_TYPE")
#define READ_TYPE(__type) \
        case __type: \
            return read<notation::code_type<notation::type::Scalar | __type>::type::Content>(ctx, reader);

            using namespace notation::type;
            switch (type & 0xff) {
                case VOID: return notation::Void();
                READ_TYPE(INT8)
                READ_TYPE(UINT8)
                READ_TYPE(INT16)
                READ_TYPE(UINT16)
                READ_TYPE(INT32)
                READ_TYPE(UINT32)
                READ_TYPE(INT64)
                READ_TYPE(UINT64)
                READ_TYPE(FLOAT16)
                READ_TYPE(FLOAT32)
                READ_TYPE(FLOAT64)
                READ_TYPE(PTR)
                READ_TYPE(CHAR8)
                READ_TYPE(CHAR16)
                READ_TYPE(CHAR32)
                READ_TYPE(UNKNOWN8)
                READ_TYPE(UNKNOWN16)
                READ_TYPE(UNKNOWN32)
                READ_TYPE(UNKNOWN64)
                READ_TYPE(UNKNOWN128)
                READ_TYPE(BOOLEAN)
                READ_TYPE(COMPLEX32)
                READ_TYPE(COMPLEX64)
                READ_TYPE(COMPLEX128)
            }
            return notation::Void();
#pragma pop_macro("READ_TYPE")
        }

        inline Var read_var(Context &ctx, const VarReader &reader) {
            auto datatype = notation::DataType(read<uint16_t>(ctx, reader));
            switch (datatype & 0xff00) {
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
                case notation::type::Object:
                    return read_object(ctx, reader);
                case notation::type::Scalar:
                    return read_scalar(ctx, datatype, reader);
            }
            throw VarIOUnrecognizedType(ctx, datatype);
        }
    }
    Var read(const VarReader &reader, VarFormat format = VarBinary, bool read_magic = false) {
        vario::Context ctx;
        ctx.push("<>");
        if (format == VarBinary) {
            if (read_magic) {
                auto fake = vario::read<int32_t>(ctx, reader);
                auto magic = vario::read<int32_t>(ctx, reader);
                if (magic != var_magic()) {
                    throw VarIOExcpetion(ctx, "Got unrecognized file type.");
                }
            }
            return vario::read_var(ctx, reader);
        } else {
            /// TODO: not implement
            return Var();
        }
    }
}

#endif //OMEGA_VAR_ISTREAM_H
