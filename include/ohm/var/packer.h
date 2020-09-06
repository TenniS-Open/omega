//
// Created by kier on 2020/9/6.
//

#ifndef OMEGA_VAR_PACKER_H
#define OMEGA_VAR_PACKER_H

#include "var.h"
#include "../time.h"
#include "context.h"
#include "stream.h"
#include "../print.h"

namespace ohm {
    namespace parser {
        using vario::Context;

        static inline notation::Binary read_file(const std::string &filename) {
            notation::Binary bin;
            std::ifstream in(filename, std::ios::binary);
            if (!in.is_open()) return bin;
            in.seekg(0, std::ios::end);
            auto size = in.tellg();
            bin.resize(size_t(size));
            in.seekg(0, std::ios::beg);
            in.read(bin.data<char>(), bin.size());
            in.close();
            return bin;
        }

        static inline Var pack_date(const Context &ctx, const std::vector<std::string> &args) {
            return to_string(now(), "%Y-%m-%d");
        }

        static inline Var pack_time(const Context &ctx, const std::vector<std::string> &args) {
            return to_string(now(), "%H:%M:%S");
        }

        static inline Var pack_datetime(const Context &ctx, const std::vector<std::string> &args) {
            return to_string(now(), "%Y-%m-%d %H:%M:%S");
        }

        static inline Var pack_nil(const Context &ctx, const std::vector<std::string> &args) {
            return Var(nullptr);
        }

        static inline Var pack_error(const Context &ctx, const std::vector<std::string> &args) {
            throw VarIOExcpetion(ctx, sprint("Not supported command: ", args));
        }

        static inline Var pack_file(const Context &ctx, const std::vector<std::string> &args) {
            if (args.size() < 2) {
                throw VarIOExcpetion(ctx, "Command format error, should be @file@...");
            }
            auto data = read_file(args[1]);
            if (data.empty()) {

                throw VarIOExcpetion(ctx, sprint(args[1], " is not a valid file."));
            }
            return data;
        }
    }
}

#endif //OMEGA_VAR_PACKER_H
