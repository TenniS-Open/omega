//
// Created by kier on 2020/7/28.
//

#ifndef OMEGA_ITERATING_H
#define OMEGA_ITERATING_H

#include "ohm/type_iterable.h"

namespace ohm {
    template <typename T, typename Enable = void>
    class Iterating;

    template <typename T>
    class Iterating<T, typename std::enable_if<has_iterator_tag<T, std::input_iterator_tag>::value>::type> {
    public:
        using self = Iterating;
        using value_type = typename std::iterator_traits<T>::value_type;
        using iterator = T;

        Iterating(iterator begin, iterator end)
                : m_begin(begin), m_end(end) {}

        iterator begin() const { return m_begin; }

        iterator end() const { return m_end; }

    private:
        iterator m_begin;
        iterator m_end;
    };

    template <typename T>
    typename std::enable_if<
            has_iterator_tag<T, std::input_iterator_tag>::value,
            Iterating<T>>::type
    iterating(T beg, T end) {
        return Iterating<T>(beg, end);
    }
}

#endif //OMEGA_ITERATING_H
