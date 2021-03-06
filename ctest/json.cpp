//
// Created by kier on 2020/9/6.
//

#include "ohm/json.h"
#include "ohm/print.h"

class A : public ohm::JSONObject {
public:
    using self = A;
    JSONField(self, ohm::Var, o) = "undefined";
    JSONField(self, int, a, false) = 6;
    JSONField(self, float, b) = 8;
    JSONField(self, std::string, c) = "test";
    long d = 0;
    JSONBind(self, d, false);
    JSONFieldV2(self, int, e, "e", false) = 9;
    double f = 10;
    JSONBindV2(self, f, "f", false);
};

int main() {
    auto json = R"({"b":1.2, "c": "hello", "d": 1231, "o": {}})";
    // auto obj = ohm::json::from_string(json);

    ohm::println(json);

    A a;

    ohm::println(ohm::sep(", "), a.a, a.b, a.c, a.d, a.e, a.f, a.o);

    a.parse_string(json);

    ohm::println(ohm::sep(", "), a.a, a.b, a.c, a.d, a.e, a.f, a.o);

    a.c = "[\"A\"]";
    // Get dump var
    ohm::Var v = a.dump();

    // Get json
    std::string s = a.json();

    ohm::println(v);
    ohm::println(s);

    return 0;
}
