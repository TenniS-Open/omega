//
// Created by kier on 2020/7/31.
//

#ifndef OMEGA_LOGGER_H
#define OMEGA_LOGGER_H

#include "print.h"
#include "macro.h"
#include "except.h"
#include "time.h"

namespace ohm {
    enum LogLevel {
        LOG_NONE = 0,
        LOG_DEBUG = 1,
        LOG_INFO = 2,
        LOG_WARNING = 3,
        LOG_ERROR = 4,
        LOG_FATAL = 5,
    };

    class __StdLogStream : public PrintStream {
    public:
        using self = __StdLogStream;

        __StdLogStream(LogLevel level) : m_level(level) {}

        LogLevel level() const { return m_level; }

        void print(const std::string &ms) const final {
            if (m_level >= LOG_ERROR) {
                if (ms.back() != '\n') {
                    std::cerr << ms + "\n" << std::flush;
                } else {
                    std::cerr << ms << std::flush;
                }
            } else {
                if (ms.back() != '\n') {
                    std::cout << ms + "\n" << std::flush;
                } else {
                    std::cout << ms << std::flush;
                }
            }
        }

    private:
        LogLevel m_level;
    };

    inline __StdLogStream L(LogLevel level) { return __StdLogStream(level); }

    inline __StdLogStream log_error() { return L(LOG_ERROR); }

    inline __StdLogStream log_fatal() { return L(LOG_FATAL); }

    class Logger;

    class LogStream : public PrintStream {
    public:
        using self = LogStream;

        LogStream(LogLevel level, const Logger &engine, const LogLevel *filter = nullptr)
            : m_level(level), m_engine(&engine), m_filter(filter) {}

        bool enable() const {
            return m_filter ? m_level >= *m_filter : true;
        }

        LogLevel level() const { return m_level; }

        void print(const std::string &msg) const final;

    private:
        const Logger *m_engine;
        const LogLevel *m_filter;
        LogLevel m_level;
    };


    class Logger : public PrintStream {
    public:
        explicit Logger(LogLevel level = LOG_INFO) {
            m_level = level;
        }
        
        void print(const std::string &msg) const final {
            writeline(LOG_INFO, msg);
        }

        void writeline(LogLevel level, const std::string &msg) const {
            // do log stuff, to terminal, also to file and other types
            auto line = msg.back() == '\n' ? msg : msg + "\n";
            ohm::print(line);
        }

        LogStream operator()(LogLevel level) const { return LogStream(level, *this, &m_level); }

        LogLevel change_level(LogLevel level) {
            auto tmp = m_level;
            m_level = level;
            return tmp;
        }

    private:
        LogLevel m_level;
    };

    inline void LogStream::print(const std::string &msg) const {
        if (!m_engine || !enable()) return;
        m_engine->writeline(this->level(), msg);
    }

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

    template <>
    inline LogLevel __log_level<__StdLogStream>(const __StdLogStream &stream) { return stream.level(); }

    template <>
    inline LogLevel __log_level<LogLevel>(const LogLevel &level) { return level; }

    /**
     *
     * @param level
     * @return
     */
    inline const char *__log_level_to_string(LogLevel level) {
        switch (level) {
            default:            return "[       ]";
            case LOG_NONE:      return "[       ]";
            case LOG_DEBUG:     return "[DEBUG]  ";
            case LOG_INFO:      return "[INFO]   ";
            case LOG_WARNING:   return "[WARNING]";
            case LOG_ERROR:     return "[ERROR]  ";
            case LOG_FATAL:     return "[FATAL]  ";
        }
    }

    template <typename T>
    inline const char *__log_level_string(const T &t) { return __log_level_to_string(__log_level<T>(t)); }

    template <typename T>
    inline bool __log_need_eject(const T &t) { return __log_level<T>(t) >= LOG_ERROR; }

    template <typename T>
    inline bool __log_need_fatal(const T &t) { return __log_level<T>(t) >= LOG_FATAL; }

    template <typename T, typename... Args>
    inline void __log_print(const T &t, const Args &...args) {
        ohm::println(t, args...);
    }

    template <typename... Args>
    inline void __log_print(LogLevel level, const Args &...args) {
        ohm::println(L(level), args...);
    }

/**
 * @param log can be PrintStream, std::ostream, Logger
 */
#define ohm_log(log, ...) do {\
        auto &&ohm_auto_name(__ohm_log) = log; \
        if (ohm::__log_enable(ohm_auto_name(__ohm_log)) \
                || ohm::__log_need_eject(log)) {\
            auto body = ohm::sprint(\
                "[", __func__, " ", ohm::__log_cut_filename(__FILE__), ":", __LINE__, "]", \
                ": ", ## __VA_ARGS__); \
            __log_print(ohm_auto_name(__ohm_log), \
                "[", ohm::now(), "]", \
                ohm::__log_level_string(ohm_auto_name(__ohm_log)), \
                body); \
            if (ohm::__log_need_fatal(log)) { \
                exit(0x7E); \
            } \
            if (ohm::__log_need_eject(log)) { \
                throw ohm::EjectionException(body); \
            } \
        } \
    } while (false)
}

#endif //OMEGA_LOGGER_H
