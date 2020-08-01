//
// Created by kier on 2020/7/31.
//

#ifndef OMEGA_LOGGER_H
#define OMEGA_LOGGER_H

#include "print.h"
#include "macro.h"

#include "time.h"

namespace ohm {
    enum LogLevel {
        LOG_NONE = 0,
        LOG_DEBUG = 1,
        LOG_STATUS = 2,
        LOG_INFO = 3,
        LOG_ERROR = 4,
        LOG_FATAL = 5,
    };

    class LogStream : public PrintStream {
    public:
        using self = LogStream;

        LogStream(LogLevel level, const PrintStream &engine, const LogLevel *filter = nullptr)
            : m_level(level), m_engine(&engine), m_filter(filter) {}

        bool enable() const {
            return m_filter ? m_level >= *m_filter : true;
        }

        LogLevel level() const { return m_level; }

        void print(const std::string &msg) const final {
            if (!m_engine || !enable()) return;
            m_engine->print(msg);
        }
    private:
        const PrintStream *m_engine;
        const LogLevel *m_filter;
        LogLevel m_level;
    };


    class Logger : public PrintStream {
    public:
        explicit Logger(LogLevel level = LOG_INFO) {
            m_level = level;
        }
        
        void print(const std::string &msg) const final {
            // do log stuff, to terminal, also to file and other types
            if (msg.back() != '\n') {
                ohm::println(msg);
            } else {
                ohm::print(msg);
            }
        }

        LogStream operator()(LogLevel level) const { return LogStream(level, *this, &m_level); }

    private:
        LogLevel m_level;
    };

    inline std::string __log_cut_filename(const std::string &filepath) {
        auto L = filepath.rfind('/');
        auto R = filepath.rfind('\\');
        if (L == std::string::npos) {
            if (R == std::string::npos) {
                return filepath;
            } else {
                return filepath.substr(R + 1);
            }
        } else {
            if (R == std::string::npos) {
                return filepath.substr(L + 1);
            } else {
                return filepath.substr(std::max(L, R) + 1);
            }
        }
    }

    template <typename T>
    inline bool __log_enable(const T &) { return true; }

    template <>
    inline bool __log_enable<Logger>(const Logger &) { return true; }

    template <>
    inline bool __log_enable<LogStream>(const LogStream &stream) { return stream.enable(); }

    template <typename T>
    inline LogLevel __log_level(const T &) { return LOG_NONE; }

    template <>
    inline LogLevel __log_level<Logger>(const Logger &) { return LOG_NONE; }

    template <>
    inline LogLevel __log_level<LogStream>(const LogStream &stream) { return stream.level(); }

    inline const char *__log_level_to_string(LogLevel level) {
        switch (level) {
            default:            return "[      ]";
            case LOG_NONE:      return "[      ]";
            case LOG_DEBUG:     return "[DEBUG] ";
            case LOG_STATUS:    return "[STATUS]";
            case LOG_INFO:      return "[INFO]  ";
            case LOG_ERROR:     return "[ERROR] ";
            case LOG_FATAL:     return "[FATAL] ";
        }
    }

    template <typename T>
    inline const char *__log_level_string(const T &t) { return __log_level_to_string(__log_level<T>(t)); }

#define ohm_log(log, ...) do {\
        auto &&ohm_auto_name(__ohm_log) = log; \
        if (ohm::__log_enable(ohm_auto_name(__ohm_log))) {\
            ohm::println(ohm_auto_name(__ohm_log), \
                "[", ohm::now(), "]", \
                ohm::__log_level_string(ohm_auto_name(__ohm_log)), \
                "[", __func__, " ", ohm::__log_cut_filename(__FILE__), ":", __LINE__, "]", \
                ": ", ## __VA_ARGS__); \
        } \
    } while (false)
}

#endif //OMEGA_LOGGER_H
