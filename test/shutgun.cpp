//
// Created by kier on 2020/10/9.
//

#include "ohm/thread/shotgun.h"
#include "ohm/print.h"
#include "ohm/thread/canyon.h"

int main() {
    ohm::Shotgun gun(10);
    srand(4482);
    for (int i = 0; i < 100; ++i) {
        gun.fire([i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 1000));
            ohm::println(i);
        });
    }
    gun.join();
    return 0;
}

