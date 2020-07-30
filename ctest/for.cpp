//
// Created by kier on 2020/7/30.
//

#include "ohm/for.h"
#include "ohm/print.h"

int main() {
    ohm_loop() {
        ohm_times(2) {
            ohm::println("==================");
            ohm_for(i, 3) {
                ohm_for(j, 3, 4) {
                    ohm_for(k, 6, 3, -2) {
                        ohm::println(ohm::sep(", "), i, j, k);
                    }
                }
            }
        }
        break;
    }
}

