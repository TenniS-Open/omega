//
// Created by kier on 2020/7/24.
//

#include "ohm/type_tuple.h"
#include "ohm/type.h"
#include "ohm/print.h"

int main() {
    using A = std::tuple<int, float, size_t&, double>;

    ohm::println("A                  = ", ohm::classname<A>());
    ohm::println("sub_tuple<A, 1, 3> = ", ohm::classname<ohm::sub_tuple<A, 1, 3>::type>());
    ohm::println("sub_tuple<A, 2>    = ", ohm::classname<ohm::sub_tuple<A, 2>::type>());

    return 0;
}

