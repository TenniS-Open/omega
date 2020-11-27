//
// Created by kier on 2020/11/20.
//

#include "ohm/filesys.h"
#include "ohm/print.h"

#include "ohm/time_summary.h"

int main() {
    using namespace ohm;

    IfTimeSummary<true> _time("Test");

    auto test_abs = [](const std::string &path) {
        println(path, " is absolute: ", is_absolute(path));
    };

    test_abs("a\\b.c");
    test_abs("a/b.c");
    test_abs("/fea/ef.c");
    test_abs("D:/fe/efa.cpp");
    test_abs("d:\\wef\\fea.json");
    test_abs("\\fea\\fe\\fe.txt");

    _time.done("A");
    _time.summary();

    return 0;
}

