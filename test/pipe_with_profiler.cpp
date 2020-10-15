//
// Created by kier on 2020/10/10.
//

#include "ohm/pipe/pipe.h"
#include "ohm/print.h"
#include "ohm/range.h"
#include "ohm/type_name.h"
#include "ohm/thread/loop_thread.h"

int main() {
    srand(4481);
    ohm::Tap<int> input(ohm::range(0, 100));

    // There are 3 types of converter.
    // 1. inplace data process
    auto mapper_inplace = [](int &a) -> void {
        std::this_thread::sleep_for(ohm::time::ms(rand() / 100));
        a *= 6;
    };

    // 2. convert data, there are 3 types data converter
    // 2.1 convert data from 1 to 1
    auto mapper_converter11 = [](float a) -> float {
        std::this_thread::sleep_for(ohm::time::ms(rand() / 100));
        return a / 2;
    };

    // 2.3 convert data from 1 to X
    auto mapper_converter1x = [](int a) -> std::function<float()> {
        std::vector<float> data = {float(a) / 2, float(a) / 3};
        return ohm::make_generator(std::move(data));
    };

    auto saver = [](float a) -> void {
        std::this_thread::sleep_for(ohm::time::ms(rand() / 100));
    };

    input.limit(50)
            .profile("1: x*6")
            .map(2, mapper_inplace).limit(10)
            .profile("2: {x/2,x/3}")
            .map(2, mapper_converter1x).limit(10)
            .profile("3: x/10")
            .map(0, mapper_converter11)
            .profile("4: output")
            .seal(2, saver);

    ohm::LoopThread log(1, [&]() {
        auto report = input.report();
        ohm::println("==================================");
        for (auto &pair : report.report) {
            auto &name = pair.first;
            auto &line = pair.second;
            ohm::println(line.name, ": time = ", line.threads, " * ", line.average_time);
        }
    });

    input.loop();
    input.join();

    log.dispose();

    return 0;
}

