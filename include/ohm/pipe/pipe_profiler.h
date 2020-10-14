//
// Created by kier on 2020/10/14.
//

#ifndef OMEGA_PIPE_PROFILER_H
#define OMEGA_PIPE_PROFILER_H

#include "../time.h"
#include "../thread/queue_watcher.h"
#include "../type_required.h"

#include <string>

namespace ohm {
    template <typename T, typename=Required<std::is_integral<T>>>
    class PipeAverageTime {
    public:
        using self = PipeAverageTime;

        /**
         *
         * @param N window size
         */
        explicit PipeAverageTime(size_t N = 60)
            : m_size(N), m_sum(0), m_average(0) {
        }

        PipeAverageTime(const PipeAverageTime &) = delete;

        PipeAverageTime &operator=(const PipeAverageTime &) = delete;

        void push(T value) {
            std::unique_lock<std::mutex> _lock(m_mutex);
            m_values.push(value);
            m_sum += value;
            if (m_values.size() > m_size) {
                m_sum -= m_values.front();
                m_values.pop();
            }
            m_average = m_sum / T(m_values.size());
        }

        T value() {
            return m_average;
        }

    private:
        size_t m_size;
        T m_sum;
        std::queue<T> m_values;
        std::mutex m_mutex;
        std::atomic<T> m_average;
    };

    class PipeTimeWatcher {
    public:
        PipeTimeWatcher()
            : m_average(new PipeAverageTime<int>(60)) {}

        ~PipeTimeWatcher() = default;

        std::function<void(time::ms)> time_reporter() {
            auto average = m_average;
            return [average](time::ms time) {
                average->push(int(time::count<time::ms>(time)));
            };
        }

        time::ms time() {
            return time::ms(m_average->value());
        }
    private:
        std::shared_ptr<PipeAverageTime<int>> m_average;
    };

    class PipeStatus {
    public:
        QueueWatcher io_count;
        PipeTimeWatcher process_time;
    };

    class PipeProfiler {
    public:
        struct Callback {
            std::function<void()> in;
            std::function<void()> out;
            std::function<void(time::ms)> time;
        };

        Callback callback(const std::string &name) {
            auto it = m_status.find(name);
            if (it == m_status.end()) {
                auto succeed = m_status.insert(std::make_pair(name, PipeStatus()));
                it = succeed.first;
            }
            auto &status = it->second;
            return {status.io_count.input_ticker(),
                    status.io_count.output_ticker(),
                    status.process_time.time_reporter()};
        }

        /**
         * log for each
         */
        struct Report {
            struct Line {
                std::string name;
                QueueWatcher::Report queue;
                time::ms each_process_time;
            };
            std::vector<Line> lines;
        };

        Report report() {
            Report result;
            for (auto &pair : m_status) {
                result.lines.emplace_back(
                        Report::Line({pair.first,
                                      pair.second.io_count.report(),
                                      pair.second.process_time.time()}));
            }
            return result;
        }

    private:
        std::map<std::string, PipeStatus> m_status;
    };
}

#endif //OMEGA_PIPE_PROFILER_H
