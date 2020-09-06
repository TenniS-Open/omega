//
// Created by kier on 2020/9/6.
//

#include "ohm/filesys.h"

int main() {
    ohm_cd("..");
    ohm::println("self: ", ohm::getself());
    ohm::println("cwd: ", ohm::getcwd());
    ohm::println("exed: ", ohm::getexed());
    return 0;
}

