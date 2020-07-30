//
// Created by kier on 2020/7/30.
//

#include "ohm/time.h"
#include "ohm/print.h"

int main() {
    using namespace ohm;
    println("Now time: ", now());
    println("Spent time: ", time::ns(1567));
    println("Spent time: ", time::ms(156));
    println("Spent time: ", time::ms(1567));
    println("Spent time: ", time::sec(1567));
    auto time = time::sec(4) + time::min(5);
    println("Count seconds of 5m4s: ", time::count<time::sec>(time));
}

