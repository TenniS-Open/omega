//
// Created by kier on 2020/8/25.
//

#ifndef OMEGA_VAR_STA_H
#define OMEGA_VAR_STA_H

#include "var.h"
#include "context.h"
#include "stream.h"

namespace ohm {
    namespace sta {
        using vario::Context;

        enum Type {
            NIL = 0,
            INT = 1,
            FLOAT = 2,
            STRING = 3,
            BINARY = 4,
            LIST = 5,
            DICT = 6,
            BOOLEAN = 7
        };

        inline constexpr int32_t magic() {
            return 0x19910929;
        }

        inline Var read_sta(Context &ctx, const VarReader &reader);

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
        template <typename T>
        inline T *read_buffer(Context &ctx, T *data, size_t wanted, const VarReader &reader) {
            auto size = notation::element_size<T>() * wanted;
            auto read = reader(data, size);
            if (read != size) {
                throw VarIOEndOfStream(ctx);
            }
            return data;
        }

        inline Var read_nil(Context &ctx, const VarReader &reader) {
            read<uint8_t>(ctx, reader);
            return nullptr;
        }

        inline Var read_int(Context &ctx, const VarReader &reader) {
            auto value = read<int32_t>(ctx, reader);
            return value;
        }

        inline Var read_float(Context &ctx, const VarReader &reader) {
            auto value = read<float>(ctx, reader);
            return value;
        }

        inline Var read_string(Context &ctx, const VarReader &reader) {
            auto size = read<int32_t>(ctx, reader);
            std::vector<char> buffer(size);
            read_buffer(ctx, buffer.data(), size, reader);
            buffer.push_back('\0');
            return Var(std::string(buffer.data()));
        }

        inline Var read_boolean(Context &ctx, const VarReader &reader) {
            auto value = read<uint8_t>(ctx, reader);
            return value != 0;
        }

        inline Var read_binary(Context &ctx, const VarReader &reader) {
            auto size = read<int32_t>(ctx, reader);
            notation::Binary bin;
            bin.resize(size);
            reader(bin.data<char>(), bin.size());
            return Var(bin);
        }

        inline Var read_list(Context &ctx, const VarReader &reader) {
            auto size = read<int32_t>(ctx, reader);
            std::vector<notation::Element::shared> array;
            for (int32_t i = 0; i < size; ++i) {
                ctx.push("[", i, "]");
                array.push_back(read_sta(ctx, reader));
                ctx.pop();
            }
            return Var(std::move(array));
        }

        inline Var read_dict(Context &ctx, const VarReader &reader) {
            auto size = read<int32_t>(ctx, reader);
            std::map<std::string, notation::Element::shared> object;
            for (int32_t i = 0; i < size; ++i) {
                std::string key = read_string(ctx, reader);
                ctx.push(".", key);
                auto value = read_sta(ctx, reader);
                ctx.pop();
                object[key] = value._element();
            }
            return Var(std::move(object));
        }

        inline Var read_sta(Context &ctx, const VarReader &reader) {
            auto code = read<uint8_t>(ctx, reader);
            switch (code) {
                case NIL:
                    return read_nil(ctx, reader);
                case INT:
                    return read_int(ctx, reader);
                case FLOAT:
                    return read_float(ctx, reader);
                case STRING:
                    return read_string(ctx, reader);
                case BINARY:
                    return read_binary(ctx, reader);
                case LIST:
                    return read_list(ctx, reader);
                case DICT:
                    return read_dict(ctx, reader);
                case BOOLEAN:
                    return read_boolean(ctx, reader);
            }
            return Var();
        }
    }
}

#endif //OMEGA_VAR_STA_H
