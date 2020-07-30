//
// Created by kier on 2020/7/30.
//

#include <ohm/print.h>
#include "ohm/memory.h"
#include "ohm/need.h"

void print_int(int a) {
    ohm::println(a);
}

class A {
public:
    A() {
        ohm::println("A::A()");
    }
    ~A() {
        ohm::println("A::~A()");
    }
    void dispose() {
        ohm::println("A::dispose()");
    }
};

int main() {
    using namespace ohm;
    println(mstring(12334523));
    ohm::void_call(print_int, 1);
    auto func = void_bind(print_int, 2);
    func();
    A *a = new A;
    ohm_need(std::default_delete<A>(), a);
    ohm_need(&A::dispose, a);
}

