//
// Created by kier on 2020/7/28.
//

#include "ohm/each.h"
#include <vector>
#include <ohm/type_name.h>
#include "ohm/print.h"

int main() {
    using namespace ohm;
    std::vector<float> x = {1, 2, 3};

    ohm::println(x);

    ohm::each([](float &a) { a += 2; }, x);

    ohm::println(x);
}

