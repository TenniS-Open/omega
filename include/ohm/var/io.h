//
// Created by kier on 2020/9/6.
//

#ifndef OMEGA_VAR_IO_H
#define OMEGA_VAR_IO_H

#include "istream.h"
#include "ostream.h"
#include "parser.h"
#include "../filesys.h"

namespace ohm {
    namespace var {
        /**
         * read var from stream, in json or binary format
         * @param reader read stream
         * @param sysroot read file root, should give if has json command
         * @return read var
         * @notice it throws VarIOException if file format has not recognized.
         */
        inline Var readf(const VarReader &reader, const std::string &sysroot = "") {
            // ohm_cd(sysroot);

            vario::Context ctx;
            ctx.sysroot(sysroot);
            ctx.push("<>");
            VarForwardReader _forward(reader, 8);
            auto forward = [&_forward](void *data, size_t size) -> size_t {
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
            return parser::read_json(ctx, forward);
        }

        /**
         * read var from memory block, in json or binary format
         * @param data memory buffer address
         * @param size memory buffer size
         * @param sysroot read file root, should give if has json command
         * @return read var
         * @notice it throws VarIOException if file format has not recognized.
         */
        inline Var readf(const void *data, size_t size, const std::string &sysroot = "") {
            return readf(VarMemoryReader(data, size), sysroot);
        }

        /**
         * read var from file, in json or binary format
         * @param filename read filename
         * @return read var
         * @notice it throws VarIOException if file format has not recognized.
         */
        inline Var readf(const std::string &filename) {
            std::ifstream f(filename, std::ios::binary);
            if (!f.is_open()) throw VarFileNotFound(filename);

            auto sysroot = cut_path_tail(filename);

            auto reader = [&](void *data, size_t size) -> size_t {
                f.read(reinterpret_cast<char *>(data), size);
                return f.gcount();
            };
            return readf(reader, sysroot);
        }

        /**
         * write var to stream, in given format
         * @param var read to write var
         * @param writer write stream
         * @param format write format
         * @return number of writen bytes
         */
        inline size_t writef(const Var &var, const VarWriter &writer, VarFormat format=VarBinary) {
            if (format == VarBinary)
            {
                return write(var, writer, true);
            } else {
                return parser::write_json(var, writer);
            }
        }

        /**
         * write var to file, in given format
         * @param var read to write var
         * @param filename filename
         * @param format write format
         * @return number of writen bytes
         * @notice it throws VarIOException if file can not access.
         */
        inline size_t writef(const Var &var, const std::string &filename, VarFormat format=VarBinary) {
            std::ofstream f(filename, std::ios::binary);
            if (!f.is_open()) throw VarFileNotFound(filename);
            auto writer = [&](const void *data, size_t size) -> size_t {
                if (!f.is_open()) return false;
                f.write(reinterpret_cast<const char *>(data), size);
                return size;
            };
            return writef(var, writer, format);
        }
    }
}

#endif //OMEGA_VAR_IO_H
