//
// Created by kier on 2020/9/21.
//
#include "ohm/list_files.h"
#include "ohm/print.h"

int main() {
    using namespace ohm;

    println(glob_files("."));

    return 0;
}

