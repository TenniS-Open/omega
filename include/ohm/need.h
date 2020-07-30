//
// Created by kier on 2020/7/31.
//

#ifndef OMEGA_NEED_H
#define OMEGA_NEED_H

#include "void_bind.h"
#include "type_callable.h"
#include "except.h"

namespace ohm {
    class need {
    public:
        need(const need &that) = delete;

        need &operator=(const need &that) = delete;

        template<typename FUNC>
        explicit need(FUNC func)
            : task(void_bind(func)) {}

        template<typename FUNC, typename... Args,
                typename = typename std::enable_if<can_be_bind<FUNC, Args...>::value>::type>
        explicit need(FUNC func, Args &&... args)
            : task(void_bind(func, std::forward<Args>(args)...)) {}

        ~need() { if (task) task(); }

        need(need &&that) OHM_NOEXCEPT { std::swap(task, that.task); }

        need &operator=(need &&that) OHM_NOEXCEPT {
            std::swap(task, that.task);
            return *this;
        }

        /**
         * danger action, do not use this.
         */
        void release() { task = nullptr; }

        void emit() {
            if (task) task();
            task = nullptr;
        }

    private:
        VoidOperator task;
    };
}

#define __ohm_need_var_cat__(a, b, c) a##b##c

#define __ohm_need_var_cat(a, b, c) __ohm_need_var_cat__(a, b, c)

#define __ohm_need_var __ohm_need_var_cat(__ohm_need_line_, __LINE__, __)

#define ohm_need(func, ...) ohm::need __ohm_need_var((func), ## __VA_ARGS__)

#endif //OMEGA_NEED_H
