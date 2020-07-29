//
// Created by kier on 2020/7/23.
//

#include "ohm/grid.h"
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

    println(classname<decltype(grid(a, b, c))::value_type>());
    println(classname<decltype(grided(a, b, c))::value_type>());

    for (auto i : grid(a, b, c)) {
        println(i);
    }

    for (auto i : grided(a, b, c)) {
        println(i);
    }


    println();
}
