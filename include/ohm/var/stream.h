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
object := <var:size><[<string><var>]*int($size)>
repeat{sub} := <var:size><byte*type_bytes($sub)*int($size)>
binary := <var:size><byte*int($size)>
element{code} := switch(code & 0xff00) begin
    0 -> null
    0x0100 -> scalar{code & 0xff}
    0x0200 -> string
    0x0300 -> boolean
    0x0400 -> array
    0x0500 -> object
    0x0600 -> repeat{code & 0xff}
    0x0700 -> binary
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

namespace ohm {
    using VarWriter = std::function<size_t(const void *, size_t)>;
    enum VarFormat {
        VarBinary = 0,
        VarJSON = 1,
    };

    constexpr inline int32_t var_magic() {
        return 0x19900714;
    }
}

#endif //OMEGA_VAR_STREAM_H
