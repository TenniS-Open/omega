//
// Created by kier on 2020/7/22.
//

#ifndef OMEGA_LIST_H
#define OMEGA_LIST_H

#include <vector>

#include "type_iterable.h"

namespace ohm {
    template<typename T>
    using List = std::vector<T>;

    template <typename Wanted, typename T>
    inline typename std::enable_if<
            is_iterable<T>::value &&
            !std::is_same<typename has_iterator<T>::value_type, Wanted>::value &&
            std::is_convertible<typename has_iterator<T>::value_type, Wanted>::value,
            List<Wanted>>::type
    list(const T &iter) {
        return List<Wanted>(iter.begin(), iter.end());
    }

    template <typename Wanted, typename T>
    inline typename std::enable_if<
            is_iterable<T>::value &&
            std::is_same<typename has_iterator<T>::value_type, Wanted>::value,
            List<Wanted>>::type
    list(const T &iter) {
        return list(iter);
    }

    template<typename T>
    inline typename std::enable_if<
            is_iterable<T>::value &&
            !std::is_convertible<T, List<typename has_iterator<T>::value_type>>::value,
            List<typename has_iterator<T>::value_type>>::type
    list(const T &iter) {
        using Item = typename has_iterator<T>::value_type;
        return List<Item>(iter.begin(), iter.end());
    }

    template<typename T>
    inline typename std::enable_if<
            is_iterable<T>::value &&
            std::is_convertible<T, List<typename has_iterator<T>::value_type>>::value,
            List<typename has_iterator<T>::value_type>>::type
    list(const T &iter) {
        return iter;
    }
}

#endif //OMEGA_LIST_H
