//
// Created by kier on 2020/10/10.
//

#ifndef OMEGA_DISPATCHER_QUEUE_H
#define OMEGA_DISPATCHER_QUEUE_H

#include <queue>
#include <ohm/print.h>

#include "dispatcher.h"

#include "../time.h"

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
            explicit Thread(Action action, Args... args)
                : thread(args...)
                , action(action) {}

            ~Thread() {
                if (thread.joinable()) thread.join();
            }

            std::thread thread;
            Action action;
        };

        /**
         * Construct queue, set `limit` of queue max size.
         * @param limit
         */
        explicit DispatcherQueue(int64_t limit = -1)
                : m_running(true), m_limit(limit)
                , m_in_action([](){}), m_out_action([](){}) {
        }

        ~DispatcherQueue() {
            this->clear();
        }

        /**
         * Bind on action.
         * If intime is true, the `push` function will directly call given aciton with out queue push.
         * @tparam FUNC action function type
         * @param func action function
         * @param intime if true, push action will directly call `func` instead push to queue.
         */
        template<typename FUNC, typename=typename std::enable_if<
                std::is_constructible<Action, FUNC>::value>::type>
        void bind(FUNC func, bool intime=false) {
            auto action = Action(func);
            if (intime) {
                this->m_intime_action = action;
            } else {
                this->m_intime_action = nullptr;
                m_threads.emplace_back(std::make_shared<Thread>(action, &self::operating, this, action));
            }
        }

        /**
         * bind N processor with action(first parameter shuold be thread-id)
         * @tparam FUNC
         * @param N number of thread, if N is 0, it means bind intime action
         * @param func action function
         */
        template<typename FUNC, typename=typename std::enable_if<
                std::is_constructible<std::function<void(int, T)>, FUNC>::value>::type>
        void bind(size_t N, FUNC func) {
            if (N == 0) {
                bind(std::bind(func, 0, std::placeholders::_1), true);
                return;
            }
            for (size_t i = 0; i < N; ++i) {
                auto action = [i, func](T t) {
                    func(int(i), t);
                };
                this->bind(std::move(action));
            }
        }

        /**
         * Add data to queue, if zero thread with action.
         * this the data will invoke in this function.
         * @param data push data
         */
        void push(T data) {
            if (m_intime_action) {
                m_intime_action(std::move(data));
                return;
            }
            std::unique_lock<std::mutex> _lock(m_mutex);
            while (true) {
                auto limit = m_limit.load();
                if (limit > 0 && int64_t(m_deque.size()) >= limit) {
                    m_cond_push.wait(_lock);
                } else {
                    break;
                }
            }
            m_deque.push_front(std::move(data));
            m_cond_pop.notify_one();
            m_in_action();
        }

        /**
         * wait until queue empty
         */
        void join() {
            std::unique_lock<std::mutex> _lock(m_mutex);
            while (!m_deque.empty()) m_cond_push.wait(_lock);
        }

        /**
         * Clear all the binded actions.
         */
        void clear() {
            m_running = false;
            m_cond_pop.notify_all();
            m_threads.clear();
            m_intime_action = nullptr;
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
            m_out_action();
            return tmp;
        }

        /**
         * Return queue limit size, return -1 if no limits
         * @return queue limit size
         */
        int64_t capacity() {
            return m_limit;
        }

        /**
         * set queue limit size, if queue size equal or greater than limit, `push` would block.
         * @param size limit size
         */
        void limit(int64_t size) {
            m_limit = size;
        }

        /**
         * Set IO action, action will called when data input or output.
         * @param in_action called after data push
         * @param out_action called after data pop
         * @note only can be called when on data processing
         */
        void set_io_action(const std::function<void()> &in_action,
                           const std::function<void()> &out_action) {
            m_in_action = in_action;
            m_out_action = out_action;
        }

        /**
         * Clear IO action.
         * @note only can be called when on data processing
         */
        void clear_io_action() {
            m_in_action = []() {};
            m_out_action = []() {};
        }

        /**
         * Set time reporter, it will report each process time to reporter
         * @param reporter time reporter
         * @note only can be called when on data processing
         */
        void set_time_reporter(const std::function<void(time::ms)> &reporter) {
            m_action_report = reporter;
        }

        /**
         * Clear IO action.
         * @note only can be called when on data processing
         */
        void clear_time_reporter() {
            m_action_report = nullptr;
        }

        /**
         * get number of threads
         * @return threads
         */
        size_t threads() {
            return m_threads.size();
        }

    private:
        std::deque<T> m_deque;
        mutable std::mutex m_mutex;
        std::condition_variable m_cond_push;    // has space to push
        std::condition_variable m_cond_pop;     // has element to pop
        std::vector<std::shared_ptr<Thread>> m_threads;
        std::atomic<bool> m_running;
        Action m_intime_action;

        std::atomic<int64_t> m_limit;

        std::function<void()> m_in_action;
        std::function<void()> m_out_action;

        std::function<void(time::ms)> m_action_report;

        struct Reporter {
        public:
            Reporter(const std::function<void(time::ms)> &reporter)
                : m_reporter(reporter), m_start(now()) {
            }

            ~Reporter() {
                auto end = now();
                m_reporter(std::chrono::duration_cast<time::ms>(end - m_start));
            }

        private:
            std::function<void(time::ms)> m_reporter;
            time_point m_start;
        };

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
                m_out_action();
                _lock.unlock();
                if (m_action_report) {
                    Reporter reporter(m_action_report);
                    action(tmp);
                } else {
                    action(tmp);
                }
            }
        }
    };

    template <typename RET, typename ...ARGS>
    inline std::function<RET(ARGS...)> with_lock(const std::function<RET(ARGS...)> &func) {
        std::shared_ptr<std::mutex> pmutex(new std::mutex);
        return [pmutex, func](ARGS... args) -> RET{
            std::unique_lock<std::mutex> _lock(*pmutex);
            return func(args...);
        };
    }

    template <typename ...ARGS>
    inline std::function<void(ARGS...)> with_lock(const std::function<void(ARGS...)> &func) {
        std::shared_ptr<std::mutex> pmutex(new std::mutex);
        return [pmutex, func](ARGS... args) -> void{
            std::unique_lock<std::mutex> _lock(*pmutex);
            func(args...);
        };
    }

    template <typename DECL, typename FUNC, typename=typename std::enable_if<
            is_callable<FUNC, DECL>::value>::type>
    inline std::function<DECL> with_lock(FUNC func) {
        return with_lock(std::function<DECL>(func));
    }

}

#endif //OMEGA_DISPATCHER_QUEUE_H
