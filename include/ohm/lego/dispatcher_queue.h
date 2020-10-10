//
// Created by kier on 2020/10/10.
//

#ifndef OMEGA_DISPATCHER_QUEUE_H
#define OMEGA_DISPATCHER_QUEUE_H

#include <queue>
#include <ohm/print.h>

#include "../thread/dispatcher.h"

namespace ohm {
    class QueueEnd : public std::exception {};

    template<typename T, typename=typename std::enable_if<
            std::is_copy_assignable<T>::value &&
            std::is_copy_constructible<T>::value>::type>
    class DispatcherQueue {
    public:
        using self = DispatcherQueue;

        using Action = std::function<void(T)>;

        class Thread {
        public:
            template<typename... Args, typename = typename std::enable_if<
                    std::is_constructible<std::thread, Args...>::value>::type>
            Thread(Action action, Args... args)
                : thread(args...)
                , action(action) {}

            ~Thread() {
                if (thread.joinable()) thread.join();
            }

            std::thread thread;
            Action action;
        };

        explicit DispatcherQueue(int64_t limit = -1)
                : m_running(true), m_limit(limit) {
        }

        ~DispatcherQueue() {
            this->clear();
        }

        template<typename FUNC, typename=typename std::enable_if<
                std::is_constructible<Action, FUNC>::value>::type>
        void bind(FUNC func) {
            auto action = Action(func);
            m_threads.emplace_back(std::make_shared<Thread>(action, &self::operating, this, action));
        }

        template<typename FUNC, typename=typename std::enable_if<
                std::is_constructible<std::function<void(int, T)>, FUNC>::value>::type>
        void bind(size_t N, FUNC func) {
            for (size_t i = 0; i < N; ++i) {
                auto action = [i, func](T t) {
                    func(int(i), t);
                };
                this->bind(std::move(action));
            }
        }

        /**
         * add data to queue, if only one thread with action.
         * this the data will invoke in this function.
         * @param t
         */
        void push(T data) {
            std::unique_lock<std::mutex> _lock(m_mutex);
            if (m_threads.size() == 1) {
                m_threads[0]->action(std::move(data));
                return;
            }
            if (m_limit > 0) {
                while (m_deque.size() >= m_limit) m_cond_push.wait(_lock);
            }
            m_deque.push_front(std::move(data));
            m_cond_pop.notify_one();
        }

        /**
         * wait until queue empty
         */
        void join() {
            std::unique_lock<std::mutex> _lock(m_mutex);
            while (!m_deque.empty()) m_cond_push.wait(_lock);
        }

        void clear() {
            m_running = false;
            m_cond_pop.notify_all();
            m_threads.clear();
            m_running.store(true);
        }

        size_t size() const {
            std::unique_lock<std::mutex> _lock(m_mutex);
            return m_deque.size();

        }

        /**
         * After this function, no more action will be auto modify
         * call this function only no data will call and there are still some backend thread running.
         */
        void dispose() {
            m_running = false;
            m_cond_pop.notify_all();
        }

        /**
         * Return one value from queue, if no data will be add, there will throw QueueEnd exception.
         * @return top value of queue.
         */
        T pop() {
            std::unique_lock<std::mutex> _lock(m_mutex);
            while (true) {
                if (!m_running) throw QueueEnd();
                if (m_deque.empty()) {
                    m_cond_pop.wait(_lock);
                } else {
                    break;
                }
            }
            if (!m_running) throw QueueEnd();
            auto tmp = m_deque.back();
            m_deque.pop_back();
            m_cond_push.notify_one();
            return tmp;
        }

    private:
        std::deque<T> m_deque;
        mutable std::mutex m_mutex;
        std::condition_variable m_cond_push;    // has space to push
        std::condition_variable m_cond_pop;     // has element to pop
        std::vector<std::shared_ptr<Thread>> m_threads;
        std::atomic<bool> m_running;

        int64_t m_limit = -1;

        void operating(Action action) {
            while (true) {
                std::unique_lock<std::mutex> _lock(m_mutex);
                while (true) {
                    if (!m_running) return;
                    if (m_deque.empty()) {
                        m_cond_pop.wait(_lock);
                    } else {
                        break;
                    }
                }
                if (!m_running) break;
                auto tmp = m_deque.back();
                m_deque.pop_back();
                m_cond_push.notify_one();
                _lock.unlock();
                action(tmp);
            }
        }
    };
}

#endif //OMEGA_DISPATCHER_QUEUE_H
