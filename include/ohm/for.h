//
// Created by kier on 2020/7/30.
//

#ifndef OMEGA_FOR_H
#define OMEGA_FOR_H

#include "range.h"

#define ohm_loop() while (true)

#define __ohm_loop_var_cat__(a, b, c) a##b##c

#define __ohm_loop_var_cat(a, b, c) __ohm_loop_var_cat__(a, b, c)

#define __ohm_loop_var __ohm_loop_var_cat(__ohm_times_line_, __LINE__, _i__)

#define ohm_times(N) for (decltype(N) __ohm_loop_var = 0; __ohm_loop_var < N; ++__ohm_loop_var)

#define ohm_for(var, N, ...) for (auto var : ohm::range(N, ## __VA_ARGS__))

#endif //OMEGA_FOR_H
