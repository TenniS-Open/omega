//
// Created by kier on 2020/11/20.
//

#ifndef OMEGA_TIME_SUMMARY_H
#define OMEGA_TIME_SUMMARY_H

#include "./time.h"
#include <vector>

#ifndef OHM_TIME_SUMMARY
#define OHM_TIME_SUMMARY true
#endif

namespace ohm {
    template<bool= true>
    class IfTimeSummary;

    template<>
    class IfTimeSummary<true> {
    public:
        using self = IfTimeSummary;

        IfTimeSummary(const IfTimeSummary &) = delete;

        IfTimeSummary &operator=(const IfTimeSummary &) = delete;

        using Duration = decltype(ohm::now() - ohm::now());

        explicit IfTimeSummary(const std::string &module)
                : m_module(module) {
            auto now = ohm::now();
            m_start = now;
            m_last = now;
        }

        void nap() {
            m_last = ohm::now();
        }

        void done(const std::string &tag) {
            auto now = ohm::now();
            auto spent = now - m_last;
            m_last = now;
            m_summary.emplace_back(std::make_pair(tag, spent));
        }

        std::string summary() const {
            std::ostringstream oss;
            oss << "=========== Time summary of [" << m_module << "] ===========" << std::endl;
            oss << "---- Total: " << ohm::sprint(m_last - m_start) << std::endl;
            for (auto &line : m_summary) {
                oss << "----~ " << line.first << ": " << ohm::sprint(line.second) << std::endl;
            }
            return oss.str();
        }

        std::ostream &summary(std::ostream &out) const {
            return out << summary();
        }

    private:
        std::string m_module;
        std::vector<std::pair<std::string, Duration>> m_summary;
        ohm::time_point m_start;
        ohm::time_point m_last;
    };

    template<>
    class IfTimeSummary<false> {
    public:
        using self = IfTimeSummary;

        IfTimeSummary(const IfTimeSummary &) = delete;

        IfTimeSummary &operator=(const IfTimeSummary &) = delete;

        explicit IfTimeSummary(const std::string &module) {}

        void nap() {}

        void done(const std::string &tag) {}

        std::string summary() const {}

        std::ostream &summary(std::ostream &out) const {}
    };

    using DoTimeSummary = IfTimeSummary<true>;

    using TimeSummary = IfTimeSummary<OHM_TIME_SUMMARY>;
}

#endif //OMEGA_TIME_SUMMARY_H
