//
// Created by kier on 2020/7/22.
//

#ifndef OMEGA_LIST_H
#define OMEGA_LIST_H

#include "type_iterable.h"

namespace ohm {
    template<typename T>
    using List = std::vector<T>;

    template<typename T>
    inline typename std::enable_if<is_iterable<T>::value, List<typename has_iterator<T>::value_type>>::type
    list(const T &iter) {
        using Item = typename has_iterator<T>::value_type;
        return List<Item>(iter.begin(), iter.end());
    }
}

#endif //OMEGA_LIST_H
