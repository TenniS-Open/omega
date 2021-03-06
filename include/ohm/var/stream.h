//
// Created by kier on 2020/8/24.
//

#ifndef OMEGA_VAR_STREAM_H
#define OMEGA_VAR_STREAM_H

/**

Here is the Var binary format
```
var := ...
undefined := <>
null := <>
scalar{sub} := <byte*type_bytes($sub)>
string := <var:size><byte*int($size)>
boolean := <byte>
array := <var:size><var*int($size)>
object := <var:size><[<var:key><var:value>]*int($size)>
vector{sub} := <var:size><byte*type_bytes($sub)*int($size)>
binary := <var:size><byte*int($size)>
element{code} := switch(code & 0xff00) begin
    0x0100 -> null
    0x0200 -> scalar{code & 0xff}
    0x0300 -> string
    0x0400 -> boolean
    0x0500 -> array
    0x0600 -> object
    0x0700 -> vector{code & 0xff}
    0x0800 -> binary
    0xff00 -> undefined
end
var := <int16:code><element{code}:data>
<var>
```
Well in file format, there is a header.
```
header := <int32:fake><int32:var_magic>
module := <header:header><var>
<module>
```
The `module.header.var_magic` is `0x19900714`。
 */

#include <exception>

namespace ohm {
    using VarWriter = std::function<size_t(const void *, size_t)>;
    using VarReader = std::function<size_t(void *, size_t)>;

    enum VarFormat {
        VarBinary = 0,
        VarJSON = 1,
    };

    constexpr inline int32_t var_magic() {
        return 0x19900714;
    }

    inline void *datacopy(void *dst, const void *src, size_t n) {
#if defined(_MSC_VER) && _MSC_VER >= 1400
        memcpy_s(dst, n, src, n);
        return dst;
#else
        return memcpy(dst, src, n);
#endif
    }

    class VarIOExcpetion : public std::logic_error {
    public:
        using self = VarIOExcpetion;
        using supper = std::logic_error;

        VarIOExcpetion(const std::string &msg) : supper(msg) {}

        VarIOExcpetion(const std::string &ctx, const std::string &msg)
                : supper(Message(ctx, msg)) {}

        static std::string Message(const std::string &ctx, const std::string &msg) {
            std::ostringstream oss;
            oss << "While import " << ctx + ", got exception: " << msg;
            return oss.str();
        }
    };

    class VarFileNotFound : public VarIOExcpetion {
    public:
        using self = VarFileNotFound;
        using supper = VarIOExcpetion;

        explicit VarFileNotFound(const std::string &filepath)
            : supper(Message(filepath)) {}

        static std::string Message(const std::string &filepath) {
            std::ostringstream oss;
            oss << "File not found or can not access with path: \"" << filepath << "\"";
            return oss.str();
        }
    };

    class VarIOEndOfStream : public VarIOExcpetion {
    public:
        using self = VarIOEndOfStream;
        using supper = VarIOExcpetion;

        VarIOEndOfStream(const std::string &ctx)
            : supper(ctx, "Unexpected end of stream.") {}
    };

    class VarIOUnexpectedType : public VarIOExcpetion {
    public:
        using self = VarIOUnexpectedType;
        using supper = VarIOExcpetion;

        VarIOUnexpectedType(const std::string &ctx, const std::string &expected, notation::DataType got)
                : supper(ctx, Message(expected, got)) {}

        static std::string Message(const std::string &expected, notation::DataType got) {
            std::ostringstream oss;
            oss << "Expecting " << expected << ", but got: " << notation::type_string(got);
            return oss.str();
        }
    };

    class VarIOUnrecognizedType : public VarIOExcpetion {
    public:
        using self = VarIOUnrecognizedType;
        using supper = VarIOExcpetion;

        VarIOUnrecognizedType(const std::string &ctx, notation::DataType got)
                : supper(ctx, Message(got)) {}

        static std::string Message(notation::DataType got) {
            std::ostringstream oss;
            oss << "Got unrecognized type: " << got;
            return oss.str();
        }
    };
}

#endif //OMEGA_VAR_STREAM_H
