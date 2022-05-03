//
// Created by kier on 2020/8/25.
//

#ifndef OMEGA_VAR_PARSER_H
#define OMEGA_VAR_PARSER_H

#include "var.h"
#include "context.h"
#include "stream.h"
#include "packer.h"
#include "istream.h"

#include <iostream>
#include <cctype>

namespace ohm {
    namespace parser {
        using vario::Context;

        class json_iterator {
        public:
            using self = json_iterator;

            json_iterator(const char *data, int size, int index = 0)
                    : data(data), size(size), index(index) {
            }

            const char &operator*() const {
                if (index < 0 || index >= size) std::cerr << "index out of range" << std::endl;
                return data[index];
            }

            const json_iterator begin() const {
                return json_iterator(data, size, 0);
            }

            const json_iterator end() const {
                return json_iterator(data, size, size);
            }

            json_iterator &operator++() {
                ++index;
                return *this;
            }

            const json_iterator operator++(int) {
                return json_iterator(data, size, index++);
            }

            json_iterator &operator+=(int shift) {
                index += shift;
                return *this;
            }

            json_iterator &operator-=(int shift) {
                index -= shift;
                return *this;
            }

            bool operator==(const json_iterator &it) const {
                return self::data == it.data && self::size == it.size && self::index == it.index;
            }

            bool operator!=(const json_iterator &it) const {
                return !self::operator==(it);
            }

            friend const json_iterator operator+(const json_iterator &it, int shift) {
                return json_iterator(it.data, it.size, it.index + shift);
            }

            friend const json_iterator operator+(int shift, const json_iterator &it) {
                return json_iterator(it.data, it.size, it.index + shift);
            }

            friend const json_iterator operator-(const json_iterator &it, int shift) {
                return json_iterator(it.data, it.size, it.index - shift);
            }

            friend int operator-(const json_iterator &lhs, const json_iterator &rhs) {
                if (lhs.data != rhs.data) std::cerr << "can not sub iterators from different init" << std::endl;
                return lhs.index - rhs.index;
            }

            const std::string cut(json_iterator end) const {
                int length = end - *this;
                int over = end - this->end();
                if (over > 0) length -= over;
                if (length <= 0) return std::string();
                return std::string(data + index, length);
            }

        private:
            const char *data;
            int size;
            int index;
        };

        inline bool is_space(char ch) {
            return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
        }

        inline json_iterator jump_space(json_iterator it) {
            while (it != it.end() && is_space(*it)) ++it;
            return it;
        }

        inline bool parse_null(Context &ctx, json_iterator &beg, Var &value) {
            beg = jump_space(beg);
            if (beg == beg.end()) throw VarIOExcpetion(ctx, "syntax error: converting empty json to null");
            if (beg.cut(beg + 4) == "null") {
                beg += 4;
                value = nullptr;
                return true;
            }
            return false;
        }

        inline Var parse_boolean(Context &ctx, json_iterator &beg) {
            beg = jump_space(beg);
            if (beg == beg.end()) throw VarIOExcpetion(ctx, "syntax error: converting empty json to boolean");
            Var result;
            if (beg.cut(beg + 4) == "true") {
                beg += 4;
                result = true;
            } else if (beg.cut(beg + 5) == "false") {
                beg += 5;
                result = false;
            }
            return result;
        }

        inline int char2hex(char ch) {
            int lch = std::tolower(ch);
            if ('0' <= lch && lch <= '9') return lch - '0';
            if ('a' <= lch && lch <= 'f') return lch - 'a' + 10;
            return -1;
        }

        inline std::string parse_string(Context &ctx, json_iterator &beg) {
            beg = jump_space(beg);
            if (beg == beg.end()) throw VarIOExcpetion(ctx, "syntax error: converting empty json to string");
            if (*beg != '"') throw VarIOExcpetion(ctx, std::string("syntax error: string begin with ") + *beg);
            std::string value;
            auto it = beg;
            bool slant = false;
            int unicode_index = 0;
            char unicode = 0;
            while (++it != it.end()) {
                if (unicode_index > 0) {
                    int ch = char2hex(*it);
                    if (ch < 0) throw VarIOExcpetion(ctx, "syntax error: unrecognized unicode");
                    switch (unicode_index) {
                        case 1:
                            unicode |= (ch << 4);
                            unicode_index++;
                            break;
                        case 2:
                            unicode |= ch;
                            value.push_back(char(unicode));
                            unicode = 0;
                            unicode_index++;
                            break;
                        case 3:
                            unicode |= (ch << 4);
                            unicode_index++;
                            break;
                        case 4:
                            unicode |= ch;
                            value.push_back(char(unicode));
                            unicode = 0;
                            unicode_index = 0;
                            break;
                        default:
                            break;
                    }
                    continue;
                } else if (slant) {
                    switch (*it) {
                        case '\"':
                            value.push_back(*it);
                            break;
                        case '\\':
                            value.push_back(*it);
                            break;
                        case '/':
                            value.push_back(*it);
                            break;
                        case 'b':
                            value.push_back('\b');
                            break;
                        case 'f':
                            value.push_back('\f');
                            break;
                        case 'n':
                            value.push_back('\n');
                            break;
                        case 'r':
                            value.push_back('\r');
                            break;
                        case 't':
                            value.push_back('\t');
                            break;
                        case 'u':
                            unicode_index = 1;
                            break;
                        default:
                            value.push_back(*it);
                            break;
                    }
                    slant = false;
                    continue;
                } else if (*it == '\\') {
                    slant = true;
                    continue;
                } else if (*it == '"') {
                    beg = it + 1;
                    return std::move(value);
                }
                value.push_back(*it);
            }
            throw VarIOExcpetion(ctx, "syntax error: can not find match \"");
            return std::string();
        }

        inline Var parse_number(Context &ctx, json_iterator &beg) {
            beg = jump_space(beg);
            if (beg == beg.end()) throw VarIOExcpetion(ctx, "syntax error: converting empty json to number");
            Var result;
            const char *number_c_string = &(*beg);
            char *end_ptr = nullptr;
            double value = std::strtod(number_c_string, &end_ptr);
            if (end_ptr == number_c_string) return result;
            auto ivalue = static_cast<long>(value);
            if (double(ivalue) == value) result = ivalue;
            else result = value;
            beg += int(end_ptr - number_c_string);
            return result;
        }

        inline Var parse_value(Context &ctx, json_iterator &beg);

        inline Var parse_list(Context &ctx, json_iterator &beg) {
            beg = jump_space(beg);
            if (beg == beg.end()) throw VarIOExcpetion(ctx, "syntax error: converting empty json to list");
            if (*beg != '[') throw VarIOExcpetion(ctx, std::string("syntax error: array begin with ") + *beg);
            Var value = notation::Array();
            auto it = beg;
            size_t index = 0;
            while (++it != it.end()) {
                it = jump_space(it);
                if (it == it.end() || *it == ']') break;
                ctx.push("[", index, "]");
                Var local_value = parse_value(ctx, it);
                ctx.pop();
                ++index;
                value.append(local_value);
                it = jump_space(it);
                if (it != it.end() && *it == ',') continue;
                break;
            }
            if (it == it.end() || *it != ']') throw VarIOExcpetion(ctx, "syntax error: can not find match ]");
            beg = it + 1;
            return std::move(value);
        }

        inline Var parse_dict(Context &ctx, json_iterator &beg) {
            beg = jump_space(beg);
            if (beg == beg.end()) throw VarIOExcpetion(ctx, "syntax error: converting empty json to dict");
            if (*beg != '{') throw VarIOExcpetion(ctx, std::string("syntax error: dict begin with ") + *beg);
            Var value = notation::Object();
            auto it = beg;
            while (++it != it.end()) {
                it = jump_space(it);
                if (it == it.end() || *it == '}') break;
                std::string local_key = parse_string(ctx, it);
                it = jump_space(it);
                if (it == it.end() || *it != ':')
                    throw VarIOExcpetion(ctx, "syntax error: dict key:value must split with :");
                ++it;
                ctx.push(".", local_key);
                Var local_value = parse_value(ctx, it);
                ctx.pop();
                value[local_key] = local_value;
                it = jump_space(it);
                if (it != it.end() && *it == ',') continue;
                break;
            }
            if (it == it.end() || *it != '}') throw VarIOExcpetion(ctx, "syntax error: can not find match }");
            beg = it + 1;
            return std::move(value);
        }

        using command_handler = std::function<Var(const Context &ctx, const std::vector<std::string> &args)>;

        static command_handler registered_command(const std::string &command) {
            static std::unordered_map<std::string, command_handler> command_map = {
                    {"date", pack_date},
                    {"time", pack_time},
                    {"datetime", pack_datetime},
                    {"nil", pack_nil},
                    {"binary", pack_error},
                    {"file", pack_file},
                    {"base64", pack_base64},
            };
            auto it = command_map.find(command);
            if (it == command_map.end()) return nullptr;
            return it->second;
        }

        inline std::vector<std::string> split(const std::string &str, char ch, size_t _size = 2) {
            std::vector<std::string> result;
            std::string::size_type left = 0, right;

            result.reserve(_size);
            while (true) {
                right = str.find(ch, left);
                result.push_back(str.substr(left, right == std::string::npos ? std::string::npos : right - left));
                if (right == std::string::npos) break;
                left = right + 1;
            }
            return std::move(result);
        }

        inline Var parse_sta_string(Context &ctx, json_iterator &beg) {
            auto str = parse_string(ctx, beg);
            if (str.empty() || str[0] != '@') return str;
            auto key_args = split(str, '@', 2);
            auto &key = key_args[1];
            auto args = std::vector<std::string>(key_args.begin() + 1, key_args.end());
            auto commond = registered_command(key);
            if (commond == nullptr) return str;
            return commond(ctx, args);
        }

        inline Var parse_value(Context &ctx, json_iterator &beg) {
            beg = jump_space(beg);
            if (beg == beg.end()) throw VarIOExcpetion(ctx, "syntax error: converting empty json");
            Var value;
            auto it = beg;
            value = parse_number(ctx, beg);
            if (value.type() != notation::type::Undefined) return value;
            if (*it == '"') return parse_sta_string(ctx, beg);
            if (*it == '[') return parse_list(ctx, beg);
            if (*it == '{') return parse_dict(ctx, beg);
            value = parse_boolean(ctx, beg);
            if (value.type() != notation::type::Undefined) return value;
            if (parse_null(ctx, beg, value)) return value;
            throw VarIOExcpetion(ctx, std::string("syntax error: unrecognized symbol ") + *it);
            return Var();
        }

        inline Var read_json(Context &ctx, const VarReader &reader) {
            std::vector<char> buffer;
            size_t size = 0;
            static const auto N = 1024;
            char data[N];
            while (true) {
                auto read = reader(data, N);
                if (read < N) {
                    auto base = buffer.size();
                    buffer.resize(base + read + 1);
                    datacopy(&buffer[base], data, read);
                    buffer.back() = '\0';
                    break;
                } else {
                    auto base = buffer.size();
                    buffer.resize(base + read);
                    datacopy(&buffer[base], data, read);
                    size += read;
                }
            }
            json_iterator it(buffer.data(), int(buffer.size()) - 1);
            return parse_value(ctx, it);
        }

        inline Var read_json(Context &ctx, const char *buff, size_t size) {
            json_iterator it(buff, int(size));
            return parse_value(ctx, it);
        }

        inline size_t write_json(const Var &var, const VarWriter &writer) {
            auto body = var.repr();
            return writer(body.data(), body.size());
        }

        inline Var from_string(const std::string &str) {
            Context ctx;
            ctx.push("<>");
            auto c_str = str.c_str();
            return read_json(ctx, VarMemoryReader(c_str, str.length() + 1));
        }

        inline Var from_string(const char *buff, size_t size) {
            Context ctx;
            ctx.push("<>");
            return read_json(ctx, buff, size);
        }

        inline Var from_string(const char *c_str) {
            Context ctx;
            ctx.push("<>");
            return read_json(ctx,c_str, strlen(c_str) + 1);
        }

        inline std::string to_string(const Var &var) {
            return var.repr();
        }
    }
}

#endif //OMEGA_VAR_PARSER_H
