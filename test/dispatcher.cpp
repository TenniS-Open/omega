//
// Created by kier on 2020/10/9.
//

#include "ohm/thread/dispatcher.h"
#include "ohm/print.h"

void print_value(int thread, int value, int &out) {
    ohm::println("thread-", thread, ": value = ", value);
};

int main() {
    ohm::Dispatcher<int, int &> oh;
    int t;

    // bind function by hand
    for (int i = 0; i < 10; ++i) {
        oh.bind(std::bind(print_value, i, std::placeholders::_1, std::placeholders::_2));
        // use std::bind or lambda to add event deal function
        // oh.bind([=](int a, int &b) { print_value(i, a, b); });
    }
    for (int i = 0; i < 100; ++i) {
        oh.call(i, std::ref(t));
    }
    // call join to wait all action finish.
    oh.join();

    // clear all actions before add new actions
    oh.clear();

    // auto bind function, the first parameter is thread id
    oh.bind(10, print_value);
    for (int i = 0; i < 100; ++i) {
        oh.call(i, std::ref(t));
    }
    // call join to wait all action finish.
    oh.join();

    return 0;
}

