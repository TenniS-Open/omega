//
// Created by kier on 2020/7/28.
//

#ifndef OMEGA_EACH_H
#define OMEGA_EACH_H

#include "type_iterable.h"
#include "type_callable.h"

namespace ohm {
    template<typename FUNC, typename T, typename = typename std::enable_if<
            is_iterable<T>::value &&
            has_operator_implicit_arguments<FUNC, typename has_iterator<T>::forward_value_type>::value>::type>
    void each(FUNC func, T &&t) {
        for (auto &&x : t) {
            func(x);
        }
    }
}

#endif //OMEGA_EACH_H
