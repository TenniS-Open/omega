//
// Created by kier on 2020/7/22.
//

#include "ohm/map.h"
#include "ohm/range.h"
#include "ohm/print.h"

int main() {
    using namespace ohm;

    auto a = ohm::range(10);
    ohm::println(a);

    auto b = ohm::mapped([](int x) { return x + 1; }, a);
    ohm::println(b);

    auto c = ohm::map([](int x) { return x * 2; }, b);
    ohm::println(c);

    return 0;
}

