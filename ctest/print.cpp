//
// Created by kier on 2020/7/22.
//

#include "ohm/print.h"
#include "ohm/range.h"

int main() {
    int _a[12] = {1, 2};
    std::vector<int> a(_a, _a + 2);
    using A = decltype(a);

    auto b = ohm::Range<int>(-1, 4, 2);
    using B = decltype(b);

    std::map<int, std::string> c;
    c[1] = "a";
    c[2] = "b";
    using C = decltype(c);

    ohm::println(a);
    ohm::println(b);
    ohm::println(c);

    return 0;
}
