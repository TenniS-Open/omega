//
// Created by kier on 2020/7/23.
//

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

    std::vector<std::string> b = {"a", "b", "c"};

    std::map<float, std::string> c = {{1.f, "1"}, {2.f, "2"}};

    println(classname<decltype(zip(a, b, c))::value_type>());
    println(classname<decltype(zipped(a, b, c))::value_type>());

    for (auto i : zip(a, b, c)) {
        println(i);
        std::get<1>(i) = "c";
    }

    for (auto i : zipped(a, b, c)) {
        println(i);
    }


    println();
}
