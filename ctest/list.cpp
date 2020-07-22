//
// Created by kier on 2020/7/22.
//

#include "ohm/list.h"
#include "ohm/range.h"
#include "ohm/print.h"

int main() {
    auto a = ohm::range(10);
    ohm::println(ohm::list(a));
    return 0;
}

