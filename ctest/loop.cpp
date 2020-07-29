//
// Created by kier on 2020/7/23.
//

#include "ohm/loop.h"
#include "ohm/zip.h"
#include "ohm/range.h"
#include "ohm/print.h"
#include "ohm/type.h"


#include <vector>
#include <string>
#include <map>

int main() {
    using namespace ohm;
    auto a = range(4);

    int x = 1;
    const float y = 2;
    auto b = loop(x);
    auto c = loop(y);
    auto d = loop(3);
    auto e = loop("4");

    println(classname<decltype(a)>());
    println(classname<decltype(b)>());
    println(classname<decltype(c)>());
    println(classname<decltype(d)>());
    println(classname<decltype(e)>());

    println(classname<decltype(zip(a, b, c, d, e))::value_type>());
    println(classname<decltype(zipped(a, b, c, d, e))::value_type>());

    for (auto i : zip(a, b, c, d, e)) {
        println(i);
    }

    for (auto i : zipped(a, b, c, d, e)) {
        println(i);
    }

    println();
}
