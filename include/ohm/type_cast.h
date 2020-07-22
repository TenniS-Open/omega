//
// Created by kier on 2020/7/22.
//

#ifndef OMEGA_TYPE_CAST_H
#define OMEGA_TYPE_CAST_H

#include <type_traits>

namespace ohm {
    template <typename T>
    struct remove_cr {
        using type = typename std::remove_reference<typename std::remove_const<T>::type>::type;
    };
}

#endif //OMEGA_TYPE_CAST_H
