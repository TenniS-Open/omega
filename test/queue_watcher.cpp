//
// Created by kier on 2020/10/12.
//

#include "ohm/thread/queue_watcher.h"
#include "ohm/thread/dispatcher_queue.h"
#include "ohm/print.h"

void print_value(int thread, int value) {
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 1000));
    // ohm::println("thread-", thread, ": value = ", value);
};

int main() {
    srand(4482);
    ohm::QueueWatcher qw;
    ohm::DispatcherQueue<int> oh(50);
    oh.set_io_action(qw.input_ticker(), qw.output_ticker());

    // bind function by hand
    for (int i = 0; i < 10; ++i) {
        oh.bind(std::bind(print_value, i, std::placeholders::_1));
        // use std::bind or lambda to add event deal function
        // oh.bind([=](int a) { print_value(i, a); });
    }
    for (int i = 0; i < 100; ++i) {
        oh.push(i);
        // ohm::println("thread-main: size=", oh.size());
        auto report = qw.report();
        ohm::print("I/", report.in.dps,
                   " [", report.count, "/", oh.capacity(), "] ",
                   "O/", report.out.dps,
                   "                                              \r");
    }
    // call join to wait all action finish.
    oh.join();
    auto report = qw.report();
    ohm::print("I/", report.in.dps,
               " [", report.count, "/", oh.capacity(), "] ",
               "O/", report.out.dps,
               "                                              \r");

    return 0;
}

