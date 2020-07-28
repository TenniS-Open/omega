//
// Created by kier on 2020/7/28.
//

#include "ohm/each.h"
#include <vector>
#include "ohm/print.h"
#include "ohm/iterating.h"

int main() {
    using namespace ohm;
    std::vector<float> x = {1, 2, 3};

    int y[] = {4, 5, 6};

    ohm::println(x, iterating(y, y + 3));

    ohm::each([](float &a) { a += 2; }, x);
    ohm::each([](int &a) { a -= 2; }, iterating(y, y + 3));

    ohm::println(x, iterating(y, y + 3));
}

