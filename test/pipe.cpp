//
// Created by kier on 2020/10/10.
//

#include "ohm/pipe/pipe.h"
#include "ohm/print.h"
#include "ohm/range.h"
#include "ohm/type_name.h"

int main() {
    ohm::Tap<int> input(ohm::range(0, 100));

    // There are 3 types of converter.
    // 1. inplace data process
    auto mapper_inplace = [](int &a) -> void {
        ohm::println("map: ", a);
    };

    // 2. convert data, there are 3 types data converter
    // 2.1 convert data from 1 to 1
    auto mapper_converter11 = [](float a) -> float {
        return a / 2;
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

    auto d1 = input.limit(2)
            .map(3, mapper_inplace).limit(10)
            .map(5, mapper_converter1x).limit(10)
                    // use if use 0 data processor, the converter will called by parent thread.
            .map(0, mapper_converter11)
            .dispatch(0, 2, [](float a) { return a > 20 ? 1 : 0;}); // use dispatch filter value.
    d1.at(1).seal(20, saver);
    d1.at(0).flush();   // remember flush, ignore this pipe data.

    input.loop();
    input.join();

    return 0;
}

