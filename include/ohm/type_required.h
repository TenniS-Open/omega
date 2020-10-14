//
// Created by kier on 2020/10/14.
//

#ifndef OMEGA_TYPE_REQUIRED_H
#define OMEGA_TYPE_REQUIRED_H

#include <type_traits>

namespace ohm {
    template <typename CHECK, typename T=void>
    using Required = typename std::enable_if<CHECK::value, T>::type;
}

#endif //OMEGA_TYPE_REQUIRED_H
