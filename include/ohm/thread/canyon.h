//
// Created by Lby on 2017/8/11.
//

#ifndef OMEGA_THREAD_CANYON_H
#define OMEGA_THREAD_CANYON_H

#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <queue>
#include <future>

#include "../void_bind.h"

namespace ohm {

    class Canyon {
    public:
        using self = Canyon;

        enum Action {
            DISCARD,
            WAITING
        };

        explicit Canyon(int size = -1, Action act = WAITING)
                : m_work(true), m_size(size), m_act(act) {
            this->m_core = std::thread(&Canyon::operating, this);
        }

        explicit Canyon(Action act)
                : self(-1, act) {}

        ~Canyon() {
            this->join();
            m_work = false;
            m_cond.notify_all();
            m_core.join();
        }

        Canyon(const Canyon &that) = delete;

        const Canyon &operator=(const Canyon &that) = delete;

        template<typename FUNC,
                typename=typename std::enable_if<can_be_bind<FUNC>::value>::type>
        void operator()(FUNC func) const {
            this->push(void_bind(func));
        }

        template<typename FUNC, typename... Args,
                typename=typename std::enable_if<can_be_bind<FUNC, Args...>::value>::type>
        void operator()(FUNC func, Args &&... args) const {
            this->push(void_bind(func, std::forward<Args>(args)...));
        }

        void join() const {
            std::unique_lock<std::mutex> _locker(m_mutex);
            while (!m_task.empty()) m_cond.wait(_locker);
        }

    private:
        void push(const VoidOperator &op) const {
            std::unique_lock<std::mutex> _locker(m_mutex);
            while (m_size > 0 && m_task.size() >= static_cast<size_t>(m_size)) {
                switch (m_act) {
                    case WAITING:
                        m_cond.wait(_locker);
                        break;
                    case DISCARD:
                        return;
                }
            }
            m_task.push(op);
            m_cond.notify_one();
        }

        void operating() const {
            std::unique_lock<std::mutex> _locker(m_mutex);
            while (m_work) {
                while (m_work && m_task.empty()) m_cond.wait(_locker);
                if (!m_work) break;
                auto func = m_task.front();
                m_task.pop();
                func();
                m_cond.notify_one();
            }
        }

        mutable std::queue<VoidOperator> m_task;
        mutable std::mutex m_mutex;
        mutable std::condition_variable m_cond;
        std::atomic<bool> m_work;
        int m_size;
        Action m_act;

        std::thread m_core;
    };
}

#endif //OMEGA_THREAD_CANYON_H
