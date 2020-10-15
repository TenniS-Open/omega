//
// Created by kier on 2020/10/15.
//

#include "ohm/progress_bar.h"
#include "ohm/print.h"
#include <thread>

int main() {
    ohm::progress_bar bar(0, 1000);
    bar.start();
    for (int i = 0; i < 100000000000000; ++i) {
        if (bar.stat() == ohm::progress_bar::STOPPED) break;
        bar.wait_show(1000, std::cout);

        std::chrono::milliseconds dura(1);
        std::this_thread::sleep_for(dura);

        bar.next();
    }
    bar.show(std::cout) << std::endl;

    ohm::println("Full takes: ", bar.used_time());

    return 0;
}

