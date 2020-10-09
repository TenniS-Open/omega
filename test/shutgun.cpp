//
// Created by kier on 2020/10/9.
//

#include "ohm/thread/shotgun.h"
#include "ohm/print.h"
#include "ohm/thread/canyon.h"

int main() {
    ohm::Shotgun gun(10);
    for (int i = 0; i < 100; ++i) {
        gun.fire([i]() {
            ohm::println(i);
        });
    }
    gun.join();
    return 0;
}

