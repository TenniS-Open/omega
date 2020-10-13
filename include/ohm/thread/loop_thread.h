//
// Created by kier on 2020/10/13.
//

#ifndef OMEGA_LOOP_THREAD_H
#define OMEGA_LOOP_THREAD_H

#include "../void_bind.h"

#include <thread>
#include <atomic>
#include <condition_variable>

namespace ohm {
    /**
     * TODO: support fps control
     */
    class LoopThread {
    public:
        using self = LoopThread;

        enum Status {
            RUNNING,
            SUSPEND,
            DEAD,
        };

        template<typename _Callable, typename... _Args, typename=typename std::enable_if<
                can_be_bind<typename std::decay<_Callable>::type,
                        typename std::decay<_Args>::type...>::value>::type>
        explicit LoopThread(_Callable &&_f, _Args &&... _args)
                : m_action(std::forward<_Callable>(_f), std::forward<_Args>(_args)...), m_status(RUNNING) {
            m_thread = std::thread(&self::operating, this);
        }

        ~LoopThread() {
            dispose();
        }

        LoopThread(const LoopThread &) = delete;

        LoopThread &operator=(const LoopThread &) = delete;

        void start() {
            std::unique_lock<decltype(m_mutex)> _lock(m_mutex);
            switch (m_status.load()) {
                default:
                    return;
                case SUSPEND: {
                    m_status = RUNNING;
                    m_wake_cond.notify_one();
                    break;
                }
            }
        }

        void pause() {
            std::unique_lock<decltype(m_mutex)> _lock(m_mutex);
            switch (m_status.load()) {
                default:
                    break;
                case RUNNING: {
                    m_status = SUSPEND;
                    break;
                }
            }
        }

        void stop() {
            std::unique_lock<decltype(m_mutex)> _lock(m_mutex);
            switch (m_status.load()) {
                default:
                    break;
                case RUNNING: {
                    m_status = DEAD;
                    break;
                }
                case SUSPEND: {
                    m_status = DEAD;
                    m_wake_cond.notify_one();
                    break;
                }
            }
        }

        void join() {
            if (m_thread.joinable()) m_thread.join();
        }

        void dispose() {
            stop();
            join();
        }

    private:
        std::thread m_thread;
        VoidOperator m_action;
        std::condition_variable_any m_wake_cond;
        std::atomic<int> m_status;

        struct {
            void lock() {}

            void unlock() {}
        } m_mutex;

        void operating() {
            std::unique_lock<decltype(m_mutex)> _lock(m_mutex);
            while (true) {
                switch (m_status.load()) {
                    default: {
                        break;
                    }
                    case RUNNING: {
                        m_action();
                        break;
                    }
                    case SUSPEND: {
                        m_wake_cond.wait(_lock);
                        break;
                    }
                    case DEAD: {
                        return;
                    }
                }
            }
        }
    };
}

#endif //OMEGA_LOOP_THREAD_H
