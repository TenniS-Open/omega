//
// Created by kier on 2020/7/31.
//

#include "ohm/assert.h"
#include "ohm/logger.h"

#define N(n) (n)

int main() {
    try {
        // assert with empty message
        ohm_assert(2 == N(1), "");
    } catch (...) {
        ohm::println("(2 == N(1)) [Check]");
    }

    try {
        // assert with given message
        ohm_assert(2 == N(1), "2 must equal N(1).");
    } catch (...) {
        ohm::println("(2 == N(1)) [Check]");
    }

    ohm::Logger log("TestAssert");

    try {
        // assert with logger, write to log if failed.
        ohm_assert(3 == N(2), log, "3 must equal 2.");
    } catch (...) {
        ohm::println("(3 == N(2)) [Check]");
    }

    return 0;
}

