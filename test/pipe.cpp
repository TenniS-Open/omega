//
// Created by kier on 2020/10/10.
//

#include "ohm/pipe/pipe.h"
#include "ohm/print.h"
#include "ohm/range.h"

int main() {
    ohm::Tap<int> input(ohm::range(0,  20));

    // There are 3 types of converter.
    // 1. inplace data process
    auto mapper_inplace = [](int &a) -> void {
        ohm::println("map: ", a);
    };

    // 2. convert data
    auto mapper_converter = [](int a) -> float {
        return float(a) / 2;
    };

    // 3. save data, with no return
    auto saver = [](float a) -> void {
        ohm::println("seal: ", a);
    };

    input.limit(50)
            .map(3, mapper_inplace).limit(10)
            .map(10, mapper_converter).limit(10)
            .seal(20, saver);

    input.loop();
    input.join();

    return 0;
}

