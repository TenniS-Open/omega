//
// Created by kier on 2020/10/10.
//

#include "ohm/pipe/pipe.h"
#include "ohm/print.h"
#include "ohm/range.h"

int main() {
    ohm::Tap<int> input(ohm::range(0, 10));

    // There are 3 types of converter.
    // 1. inplace data process
    auto mapper_inplace = [](int &a) -> void {
        ohm::println("map: ", a);
    };

    // 2. convert data, there are 3 types data converter
    // 2.1 convert data from 1 to 1
    auto mapper_converter11 = [](int a) -> float {
        return float(a) / 2;
    };

    // 2.2 convert data from 1 to N
    auto mapper_converter1n = [](int a) -> std::vector<float> {
        return {float(a) / 2, float(a) / 3};
    };

    // 2.3 convert data from 1 to X
    auto mapper_converter1x = [](int a) -> std::function<float()> {
        std::vector<float> data = {float(a) / 2, float(a) / 3};
        return ohm::make_generator(std::move(data));
    };

    // 3. save data, with no return
    auto saver = [](float a) -> void {
        ohm::println("seal: ", a);
    };

    input.limit(50)
            .map(3, mapper_inplace).limit(10)
            .map(5, mapper_converter1x).limit(10)
            .seal(20, saver);

    input.loop();
    input.join();

    return 0;
}

