//
// Created by kier on 2020/10/15.
//

#ifndef OMEGA_PROGRESS_BAR_H
#define OMEGA_PROGRESS_BAR_H

#include "./time.h"

namespace ohm {
    namespace _ {
        static bool append_string(std::string &base, const std::string &add, size_t limit) {
            if (base.length() + add.length() > limit) return false;
            base.insert(base.end(), add.begin(), add.end());
            return true;
        }

        static void output_string(std::ostream &out) { (decltype(output_string(out))()); }

        template<typename T, typename... Args>
        static void output_string(std::ostream &out, T &&t, Args &&... args) {
            output_string(out << std::forward<T>(t), std::forward<Args>(args)...);
        }

        template<typename... Args>
        inline std::string concat_string(std::ostringstream &oss, Args &&... args) {
            oss.str("");
            output_string(oss, std::forward<Args>(args)...);
            return oss.str();
        }
    }

    inline std::string to_string(time::us us, size_t limit) {
        auto count = us.count();
        count /= 1000L;    // ms
        auto day = count / (24L * 60L * 60L * 1000L);
        count %= (24L * 60L * 60L * 1000L);
        auto hour = count / (60L * 60L * 1000L);
        count %= (60L * 60L * 1000L);
        auto minute = count / (60L * 1000L);
        count %= (60L * 1000L);
        auto second = count / (1000L);
        count %= (1000L);
        auto ms = count;

        std::string format;
        std::ostringstream oss;
        if (day &&
            !_::append_string(format, _::concat_string(oss, day, 'd'), limit))
            return format;
        if (hour &&
            !_::append_string(format, _::concat_string(oss, hour, 'h'), limit))
            return format;
        if (minute &&
            !_::append_string(format, _::concat_string(oss, minute, 'm'), limit))
            return format;
        if (second &&
            !_::append_string(format, _::concat_string(oss, second, 's'), limit))
            return format;
        if (!day && !hour && !minute && ms &&
            !_::append_string(format, _::concat_string(oss, ms, "ms"), limit))
            return format;

        return format;
    }

    class progress_bar {
    public:
        using self = progress_bar;

        enum status {
            WAITING,
            RUNNING,
            PAUSED,
            STOPPED,
        };

        progress_bar(int64_t min, int64_t max, int64_t value)
                : m_min(min), m_max(max), m_value(value), m_paused_duration(0) {
            m_last_show_time_point = now() - time::sec(3600);
        }

        progress_bar(int64_t min, int64_t max) : progress_bar(min, max, min) {}

        explicit progress_bar(int64_t max) : progress_bar(0, max, 0) {}

        progress_bar() : progress_bar(0, 100, 0) {}

        status stat() const {
            return m_stat;
        }

        void start() {
            switch (m_stat) {
                default:
                    m_start_time_point = now();
                    reset();
                    break;
                case WAITING:
                    m_start_time_point = now();
                    m_paused_duration = time::us(0);
                    reset();
                    break;
                case RUNNING:
                    break;
                case PAUSED:
                    m_paused_duration += std::chrono::duration_cast<time::us>(
                            now() - m_pause_time_point);
                    reset();
                    break;
                case STOPPED:
                    m_start_time_point = now();
                    m_paused_duration = time::us(0);
                    reset();
                    break;
            }
            m_stat = RUNNING;
        }

        void stop() {
            switch (m_stat) {
                default:
                    m_stop_time_point = now();
                    break;
                case WAITING:
                    m_start_time_point = now();
                    m_stop_time_point = m_start_time_point;
                    break;
                case RUNNING:
                    m_stop_time_point = now();
                    break;
                case PAUSED:
                    m_paused_duration += std::chrono::duration_cast<time::us>(
                            now() - m_pause_time_point);
                    m_stop_time_point = now();
                    break;
                case STOPPED:
                    break;
            }
            m_stat = STOPPED;
        }

        void pause() {
            switch (m_stat) {
                default:
                    m_pause_time_point = now();
                    break;
                case WAITING:
                    m_start_time_point = now();
                    m_pause_time_point = m_start_time_point;
                    break;
                case RUNNING:
                    m_pause_time_point = now();
                    break;
                case PAUSED:
                    break;
                case STOPPED:
                    break;
            }
            m_stat = PAUSED;
        }

        void autostop(bool flag) {
            m_autostop = flag;
        }

        bool autostop() const {
            return m_autostop;
        }

        int64_t value() const {
            return m_value;
        }

        int64_t max() const {
            return m_max;
        }

        int64_t min() const {
            return m_min;
        }

        void set_value(int64_t value) {
            m_value = value;
        }

        void set_min(int64_t min) {
            m_min = min;
        }

        void set_max(int64_t max) {
            m_max = max;
        }

        int64_t next() {
            start();
            return next(m_step);
        }

        int64_t next(int64_t step) {
            start();
            m_value += step;
            if (m_value >= m_max && m_autostop) {
                self::stop();
                m_value = m_max;
            }

            if (stat() == RUNNING) {
                sample();
            }

            return m_value;
        }

        time::us used_time() const {
            switch (m_stat) {
                default:
                    return time::us(0);
                case WAITING:
                    return time::us(0);
                case RUNNING:
                    return std::chrono::duration_cast<time::us>(
                            now() - m_start_time_point) - m_paused_duration;
                case PAUSED:
                    return std::chrono::duration_cast<time::us>(
                            m_pause_time_point - m_start_time_point) - m_paused_duration;
                case STOPPED:
                    return std::chrono::duration_cast<time::us>(
                            m_stop_time_point - m_start_time_point) - m_paused_duration;
            }
        }

        time::us left_time() const {
            if (m_vpus == 0) {
                auto used_time = self::used_time();
                if (used_time.count() == 0) return time::us(0);
                auto processed_count = m_value - m_min;
                auto left_count = m_max - m_value;
                if (processed_count == 0) return time::us(0);
                return used_time * left_count / processed_count;
            }

            auto left_count = m_max - m_value;
            return time::us(int64_t(left_count / m_vpus));
        }

        /**
         * Data per seconds
         * @return
         */
        float dps() const {
            return float(m_vpus * 1000000);
        }

        int percent() const {
            auto fpercent = float(m_value - m_min) / (m_max - m_min) * 100;
            auto ipercent = static_cast<int>(fpercent);
            return ipercent;
        }

        std::ostream &show(std::ostream &out) const {
            static const char running_status[] = {'-', '\\', '|', '/'};
            static const auto running_status_num = sizeof(running_status) / sizeof(running_status[0]);

            std::ostringstream oss;

            switch (m_stat) {
                case WAITING:
                    oss << "[~]";
                    break;
                case RUNNING:
                    oss << '[' << running_status[m_show_count % running_status_num] << ']';
                    m_show_count++;
                    m_show_count %= running_status_num;
                    break;
                case PAUSED:
                    oss << "[=]";
                    break;
                case STOPPED:
                    oss << "[*]";
                    break;
            }

            int ps = percent();
            int processed = ps / 2;
            int left = (100 - ps) / 2;
            oss << '[';
            for (int i = 0; i < processed; ++i) oss << '>';
            if (ps % 2) oss << '=';
            for (int i = 0; i < left; ++i) oss << '-';
            oss << ']';

            if (ps == 100) oss << "[--%]";
            else oss << '[' << std::setw(2) << ps << "%]";

            oss << '[';
            oss << std::setw(8) << to_string(used_time(), 8);
            oss << '/';
            oss << std::setw(8) << to_string(left_time(), 8);
            oss << ']';

            oss << '\r';
            out << oss.str() << std::flush;

            return out;
        }

        std::ostream &wait_show(int ms, std::ostream &out) const {
            auto now_time_point = now();
            auto wait_duration = std::chrono::duration_cast<time::us>(
                    now_time_point - m_last_show_time_point);
            if (wait_duration.count() >= ms) {
                m_last_show_time_point = now_time_point;
                return show(out);
            }
            return out;
        }

    private:
        // reset sample
        void reset() {
            m_sample_value = value();
            m_sample_time_point = now();
            m_vpus = 0;
        }

        // sample value and time point, calculate speed
        void sample() {
            // 60 count or 1 secend simple rate
            auto now_value = value();
            auto now_time_point = now();
            auto sample_time_duration = std::chrono::duration_cast<time::us>(
                    now_time_point - m_sample_time_point);
            auto sample_value_duration = now_value - m_sample_value;
            if (sample_time_duration > time::sec(10) && sample_value_duration > 0) {
                m_vpus = double(sample_value_duration) / sample_time_duration.count();
                m_sample_value = now_value;
                m_sample_time_point = now_time_point;
            }
        }

        int64_t m_min;
        int64_t m_max;
        int64_t m_value;
        int64_t m_step = 1;

        bool m_autostop = true;

        status m_stat = WAITING;

        time_point m_start_time_point;
        time_point m_stop_time_point;
        time_point m_pause_time_point;
        time::us m_paused_duration;

        mutable int64_t m_show_count = 0;

        int64_t m_sample_value = 0;
        time_point m_sample_time_point;
        double m_vpus = 0; // values per microseconds

        mutable time_point m_last_show_time_point;
    };
}

#endif //OMEGA_PROGRESS_BAR_H
