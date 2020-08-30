//
// Created by kier on 2020/8/24.
//

#ifndef OMEGA_VAR_ISTREAM_H
#define OMEGA_VAR_ISTREAM_H

#include "context.h"
#include "var.h"
#include "stream.h"
#include "json.h"
#include "sta.h"

#include <string>
#include <functional>
#include <stack>
#include <cstdio>
#include <fstream>

namespace ohm {
    namespace vario {
        inline Var read_var(Context &ctx, const VarReader &reader);

        template <typename T>
        inline T read(Context &ctx, const VarReader &reader) {
            T tmp;
			constexpr auto size = notation::element_size<T>();
            auto read = reader(&tmp, size);
            if (read != size) {
                throw VarIOEndOfStream(ctx);
            }
            return tmp;
        }

        template <>
        inline void read<void>(Context &ctx, const VarReader &reader) {
        }

        template <typename T>
        inline T *read_buffer(Context &ctx, T *data, size_t wanted, const VarReader &reader) {
            auto size = notation::element_size<T>() * wanted;
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
            return Var(read<notation::code_sub_type<__type>::type>(ctx, reader));

            using namespace notation::type;
            switch (type & 0xff) {
                case VOID: return notation::scalar::Void();
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
            return notation::scalar::Void();
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

    namespace {
        class VarForwardReader {
        public:
            using self = VarForwardReader;

            VarForwardReader(const VarForwardReader &reader) = delete;
            VarForwardReader &operator=(const VarForwardReader &reader) = delete;

            VarForwardReader(const VarReader& reader, size_t size)
                : m_reader(reader)
                , m_size(size)
                , m_buffer(new char[size], std::default_delete<char[]>())
                , m_buffered(0)
                , m_read(0) {
            }

            void rewind() {
                m_read = 0;
            }

            size_t operator()(void *data, size_t size) const {
                if (size == 0) return 0;
                // check if use buffer and how many to use
                size_t i = 0;   // this time read
                auto bytes = reinterpret_cast<uint8_t *>(data);
                if (m_read < m_size) {
                    // here is each position
                    auto wanted = m_read + size;
                    // m_read, wanted, m_buffered, m_size
                    // or m_read, m_buffered, wanted, m_size
                    // or m_read, m_buffered, m_size, wanted
                    if (m_buffered >= wanted) {
                        json::datacopy(&bytes[i], &m_buffer.get()[m_read], size);
                        i += size;
                    } else if (m_size >= wanted) {
                        auto ready = m_buffered - m_read;
                        auto more = wanted - m_buffered;
                        json::datacopy(&bytes[i], &m_buffer.get()[m_read], ready);
                        i += ready;
                        auto tmp = m_reader(&m_buffer.get()[m_buffered], more);
                        json::datacopy(&bytes[i], &m_buffer.get()[m_buffered], more);
                        m_buffered += tmp;
                        i += tmp;
                    } else {
                        if (m_buffered < m_size) {
                            auto more = m_size - m_buffered;
                            m_buffered += m_reader(&m_buffer.get()[m_buffered], more);
                        }
                        auto ready = m_buffered - m_read;
                        auto last = wanted - m_buffered;
                        json::datacopy(&bytes[i], &m_buffer.get()[m_read], ready);
                        i += ready;
                        i += m_reader(&bytes[i], last);
                    }
                } else {
                    i = m_reader(data, size);
                }
                m_read += i;
                return i;
            }

        private:
            VarReader m_reader;
            size_t m_size;

            mutable std::shared_ptr<char> m_buffer;
            mutable size_t m_buffered;

            mutable size_t m_read = 0;
        };

        class VarMemoryReader {
        public:
            using self = VarMemoryReader;
            using byte = uint8_t;

            explicit VarMemoryReader(const void *data, size_t size)
                : m_data(reinterpret_cast<const byte *>(data))
                , m_size(size)
                , m_read(0) {}

            void rewind() {
                m_read = 0;
            }

            size_t operator()(void *data, size_t size) const {
                auto ready = m_read + size > m_size ? m_size - m_read : size;
                json::datacopy(data, &m_data[m_read], ready);
                m_read += ready;
                return ready;
            }
        private:
            const byte *m_data;
            size_t m_size;
            mutable size_t m_read = 0;
        };


    }

    namespace var {
        inline Var read(const VarReader &reader, VarFormat format = VarBinary, bool read_magic = false) {
            vario::Context ctx;
            ctx.push("<>");
            if (format == VarBinary) {
                if (read_magic) {
                    auto fake = vario::read<int32_t>(ctx, reader);
                    auto magic = vario::read<int32_t>(ctx, reader);
                    if (fake == sta::magic()) {
                        return sta::read_sta(ctx, reader);
                    } else if (magic != var_magic()) {
                        throw VarIOExcpetion(ctx, "Got unrecognized file type.");
                    }
                }
                return vario::read_var(ctx, reader);
            } else {
                return json::read_json(ctx, reader);
            }
        }

        inline Var readf(const VarReader &reader) {
            vario::Context ctx;
            ctx.push("<>");
            VarForwardReader _forward(reader, 8);
            auto forward = [&](void *data, size_t size) -> size_t {
                return _forward(data, size);
            };
            int32_t fake = 0, magic = 0;
            forward(&fake, 4);
            if (fake == sta::magic()) {
                return sta::read_sta(ctx, forward);
            }
            forward(&magic, 4);
            if (magic == var_magic()) {
                return vario::read_var(ctx, forward);
            }
            _forward.rewind();
            return json::read_json(ctx, forward);
        }

        inline Var readf(const void *data, size_t size) {
            return readf(VarMemoryReader(data, size));
        }

        inline Var readf(const std::string &filename) {
            std::ifstream f(filename, std::ios::binary);
            auto reader = [&](void *data, size_t size) -> size_t {
                f.read(reinterpret_cast<char *>(data), size);
                return f.gcount();
            };
            return readf(reader);
        }
    }
}

#endif //OMEGA_VAR_ISTREAM_H
