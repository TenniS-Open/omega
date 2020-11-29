//
// Created by kier on 2020/10/13.
//

#ifndef OMEGA_LOOP_THREAD_H
#define OMEGA_LOOP_THREAD_H

#include "../void_bind.h"
#include "../time.h"

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

        struct FPS {
            int fps;

            FPS(int fps) : fps(fps) {}
        };

        class Return : public std::exception {
        public:
            Status status;
            Return(Status status)
                : status(status) {}
        };

        template<typename... Args>
        struct is_action
                : public std::integral_constant<bool,
                        can_be_bind<typename std::decay<Args>::type...>::value> {
        };

        template<typename FUNC, typename... ARGS, typename=typename std::enable_if<
                is_action<FUNC, ARGS...>::value>::type>
        explicit LoopThread(Status status, FPS fps, FUNC &&func, ARGS &&... args)
                : m_action(std::forward<FUNC>(func), std::forward<ARGS>(args)...), m_status(status), m_fps(fps.fps) {
            m_thread = std::thread(&self::operating, this);
        }

        template<typename FUNC, typename... ARGS, typename=typename std::enable_if<
                is_action<FUNC, ARGS...>::value>::type>
        explicit LoopThread(FPS fps, Status status, FUNC &&func, ARGS &&... args)
                : self(status, fps, std::forward<FUNC>(func), std::forward<ARGS>(args)...) {}

        template<typename FUNC, typename... ARGS, typename=typename std::enable_if<
                is_action<FUNC, ARGS...>::value>::type>
        explicit LoopThread(Status status, FUNC &&func, ARGS &&... args)
                : self(status, FPS(0), std::forward<FUNC>(func), std::forward<ARGS>(args)...) {}

        template<typename FUNC, typename... ARGS, typename=typename std::enable_if<
                is_action<FUNC, ARGS...>::value>::type>
        explicit LoopThread(int fps, FUNC &&func, ARGS &&... args)
                : self(RUNNING, FPS(fps), std::forward<FUNC>(func), std::forward<ARGS>(args)...) {}

        template<typename FUNC, typename... ARGS, typename=typename std::enable_if<
                is_action<FUNC, ARGS...>::value>::type>
        explicit LoopThread(FPS fps, FUNC &&func, ARGS &&... args)
                : self(RUNNING, fps, std::forward<FUNC>(func), std::forward<ARGS>(args)...) {}

        template<typename FUNC, typename... ARGS, typename=typename std::enable_if<
                is_action<FUNC, ARGS...>::value>::type>
        explicit LoopThread(FUNC &&func, ARGS &&... args)
                : self(RUNNING, FPS(0), std::forward<FUNC>(func), std::forward<ARGS>(args)...) {}

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

        void setFPS(int fps) {
            m_fps = fps;
        }

        int getFPS() {
            return m_fps;
        }

    private:
        std::thread m_thread;
        VoidOperator m_action;
        std::condition_variable_any m_wake_cond;
        std::atomic<int> m_status;
        std::atomic<int> m_fps;

        time_point m_last_tick;

        struct {
            void lock() {}

            void unlock() {}
        } m_mutex;

        void operating() {
            std::unique_lock<decltype(m_mutex)> _lock(m_mutex);
            delay();    // first time would no delay, but init delay action.
            while (true) {
                switch (m_status.load()) {
                    default: {
                        break;
                    }
                    case RUNNING: {
                        try {
                            m_action();
                            delay();
                        } catch (const Return &ret) {
                            m_status = ret.status;
                        } catch (const std::exception &) {
                            // ...
                        }
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

        void delay() {
            auto now_tick = now();
            int fps = m_fps;
            if (fps == 0) {
                m_last_tick = now_tick; // still save last tick
                return;
            }
            auto wait_until = m_last_tick + time::us(1000000 / fps);
            m_last_tick = now_tick > wait_until ? now_tick : wait_until;
            std::this_thread::sleep_until(wait_until);
        }
    };
}

#endif //OMEGA_LOOP_THREAD_H
