//
// Created by kier on 2020/7/20.
//
#include "ohm/range.h"
#include "ohm/print.h"

int main() {
    using namespace ohm;

    println(range(1, 4, 1));
    println(range(1, -4, -2));
    println(range(1L, 12L, 2L));

    return 0;
}

