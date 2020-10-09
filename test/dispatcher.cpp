//
// Created by kier on 2020/10/9.
//

#include "ohm/thread/dispatcher.h"
#include "ohm/print.h"

void print_value(int thread, int value) {
    ohm::println("thread-", thread, ": value = ", value);
};

int main() {
    ohm::Dispatcher<int> oh;
    for (int i = 0; i < 10; ++i) {
        oh.bind(std::bind(print_value, i, std::placeholders::_1));
    }

    for (int i = 0; i < 100; ++i) {
        oh.call(i);
    }

    oh.clear();

    oh.bind(10, print_value);

    for (int i = 0; i < 100; ++i) {
        oh.call(i);
    }

    return 0;
}

