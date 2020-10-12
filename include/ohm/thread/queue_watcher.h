//
// Created by kier on 2020/10/12.
//

#ifndef OMEGA_QUEUE_WATCHER_H
#define OMEGA_QUEUE_WATCHER_H

#include <atomic>
#include <deque>
#include <memory>

#include "../time.h"

namespace ohm {
    class InOutCounter {
    public:
        using self = InOutCounter;

        struct Report {
            int64_t count;
            struct {
                float dps;
                time::ms waited_time;
            } in, out;
        };

        explicit InOutCounter(size_t windows_size = 10)
                : m_beginning(now()), m_window_size(windows_size < 2 ? 2 : windows_size), m_count(0) {}

        InOutCounter(const InOutCounter &) = delete;

        InOutCounter &operator=(const InOutCounter &) = delete;

        void in() {
            ++m_count;
            tick(m_in, now());
        }

        void out() {
            --m_count;
            tick(m_out, now());
        }

        Report report() {
            auto now_time_point = time::count<time::ms>(now() - m_beginning);

            int64_t count = m_count;
            int64_t in_each_spent_time = m_in.each_spent_time;
            int64_t in_last_time_point = m_in.last_time_point;
            int64_t out_each_spent_time = m_out.each_spent_time;
            int64_t out_last_time_point = m_out.last_time_point;

            Report result = {};
            result.count = count;
            result.in.dps = in_each_spent_time == 0 ? 0 : float(1000) / in_each_spent_time;
            result.out.dps = out_each_spent_time == 0 ? 0 : float(1000) / out_each_spent_time;
            result.in.waited_time = time::ms(now_time_point - in_last_time_point);
            result.out.waited_time = time::ms(now_time_point - out_last_time_point);

            return result;
        }

    private:
        time_point m_beginning;
        size_t m_window_size;

        std::atomic<int64_t> m_count;

        struct Pot {
            std::deque<time_point> every_time_point;
            std::atomic<int64_t> last_time_point;
            std::atomic<int64_t> each_spent_time;

            Pot()
                    : last_time_point(0), each_spent_time(0) {}
        } m_in, m_out;

    private:
        void tick(Pot &pot, time_point now_time) {
            pot.last_time_point = time::count<time::ms>(now_time - m_beginning);
            if (pot.every_time_point.size() < 2) {
                pot.every_time_point.push_back(now_time);
                return;
            }
            pot.each_spent_time =
                    time::count<time::ms>(now_time - pot.every_time_point.front())
                    / pot.every_time_point.size();
            while (pot.every_time_point.size() > m_window_size
                   && now_time - pot.every_time_point.front() > time::sec(1)) {
                pot.every_time_point.pop_front();
            }
        }
    };

    class QueueWatcher {
    public:
        using self = QueueWatcher;
        using Report = InOutCounter::Report;

        std::function<void()> input_ticker() {
            std::weak_ptr<InOutCounter> weak_counter = m_counter;
            return [weak_counter]() {
                auto counter = weak_counter.lock();
                if (!counter) return;
                counter->in();
            };
        }

        std::function<void()> output_ticker() {
            std::weak_ptr<InOutCounter> weak_counter = m_counter;
            return [weak_counter]() {
                auto counter = weak_counter.lock();
                if (!counter) return;
                counter->out();
            };
        }

        Report report() {
            return m_counter->report();
        }

    private:
        std::shared_ptr<InOutCounter> m_counter;
    };
}

#endif //OMEGA_QUEUE_WATCHER_H
