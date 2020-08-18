//
// Created by kier on 2020/7/31.
//

#ifndef OMEGA_LOGGER_H
#define OMEGA_LOGGER_H

#include "print.h"
#include "macro.h"
#include "except.h"
#include "time.h"
#include "platform.h"

#if OHM_PLATFORM_OS_ANDROID
#include "sys/android.h"
#endif

namespace ohm {
    enum LogLevel {
        LOG_NONE = 0,
        LOG_DEBUG = 1,
        LOG_INFO = 2,
        LOG_WARNING = 3,
        LOG_ERROR = 4,
        LOG_FATAL = 5,
    };

    struct CodeLine {
        CodeLine() = default;
        CodeLine(std::string function, std::string code, int line)
                : function(std::move(function))
                , code(std::move(code))
                , line(line) {}

        std::string function;
        std::string code;
        int line = 0;
    };

    /**
     *
     * @param level
     * @return
     */
    inline const char *log_string(LogLevel level) {
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

    inline std::ostream &operator<<(std::ostream &out, const CodeLine &cl) {
        if (cl.line) {
            return out << "[" <<  cl.function << " " << cl.code << ":" << cl.line << "]";
        } else {
            return out << "[]";
        }
    }

    template <typename... Args>
    inline std::string slog(LogLevel level, const std::string &tag, CodeLine line, const Args &...args) {
        return sprint("[", now(), "]", log_string(level), "<", tag, ">", line, ": ", args...);
    }

    template <typename... Args>
    inline std::string log(LogLevel level, const std::string &tag, CodeLine line, const Args &...args) {
        auto &&stream = level >= LOG_ERROR ? std::cerr : std::cout;
        auto msg = slog(level, tag, line, args...);
        println(stream, msg);
        return msg;
    }

    template <typename... Args>
    inline std::string log(LogLevel level, CodeLine line, const Args &...args) {
        auto &&stream = level >= LOG_ERROR ? std::cerr : std::cout;
        auto msg = slog(level, "omega", line, args...);
        println(stream, msg);
        return msg;
    }

    inline std::string log(LogLevel level, CodeLine line) {
        auto &&stream = level >= LOG_ERROR ? std::cerr : std::cout;
        auto msg = slog(level, "omega", line);
        println(stream, msg);
        return msg;
    }
    template <typename T>
    inline typename std::enable_if<
            !std::is_convertible<T, CodeLine>::value,
            std::string>::type log(LogLevel level, const T &t) {
        auto &&stream = level >= LOG_ERROR ? std::cerr : std::cout;
        auto msg = slog(level, "", CodeLine(), t);
        println(stream, msg);
        return msg;
    }

    template <typename T, typename K, typename... Args>
    inline typename std::enable_if<
            !std::is_convertible<T, CodeLine>::value &&
            !(std::is_convertible<T, const std::string &>::value && std::is_convertible<K, CodeLine>::value) &&
            true,
            std::string>::type log(LogLevel level, const T &t, const K &k, const Args &...args) {
        auto &&stream = level >= LOG_ERROR ? std::cerr : std::cout;
        auto msg = slog(level, "", CodeLine(), t, k, args...);
        println(stream, msg);
        return msg;
    }

    class Logger;

    class LogStream {
    public:
        using self = LogStream;

        LogStream(LogLevel level, const Logger &logger)
            : m_level(level), m_logger(&logger) {}

        bool enable() const;

        LogLevel level() const { return m_level; }

        const Logger *operator->() { return m_logger; }
    private:
        const Logger *m_logger;
        LogLevel m_level;
    };


    class Logger : public PrintStream {
    public:
        explicit Logger(const std::string &tag, LogLevel level = LOG_INFO)
            : m_tag(tag), m_level(level) {
        }
        
        void print(const std::string &msg) const final {
            log2console(msg);
            log2platform(LOG_INFO, msg);
            log2file(msg);
        }

        template <typename... Args>
        std::string log(LogLevel level, CodeLine line, const Args &...args) const {
            auto &&stream = level >= LOG_ERROR ? std::cerr : std::cout;
            auto tag = m_tag;
            auto msg = slog(level, tag, line, args...);

            log2console(stream, msg);
            log2platform(level, msg);
            log2file(msg);

            return msg;
        }

        void log2console(std::ostream &console, const std::string &msg) const {
            println(console, msg);
        }

        void log2console(const std::string &msg) const {
            log2console(std::cout, msg);
        }

        void log2file(const std::string &msg) const {
            // TODO: add file writer
        }

        void log2platform(LogLevel level, const std::string &msg) const {
#if OHM_PLATFORM_OS_ANDROID
            // TODO: write log on android
#endif
        }

        LogStream operator()(LogLevel level) const { return LogStream(level, *this); }

        LogLevel change_level(LogLevel level) {
            auto tmp = m_level;
            m_level = level;
            return tmp;
        }

        LogLevel level() const { return m_level; }

    private:
        LogLevel m_level;
        std::string m_tag = "omega";
    };

    inline bool LogStream::enable() const {
        auto filter = m_logger->level();
        return m_level >= filter;
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
    inline LogLevel __log_level<LogLevel>(const LogLevel &level) { return level; }

    template <typename T>
    inline const char *__log_level_string(const T &t) { return log_string(__log_level<T>(t)); }

    template <typename T>
    inline bool __log_need_eject(const T &t) { return __log_level<T>(t) >= LOG_ERROR; }

    template <typename T>
    inline bool __log_need_fatal(const T &t) { return __log_level<T>(t) >= LOG_FATAL; }

    template <typename... Args>
    inline std::string log(LogStream logger, CodeLine line, const Args &...args) {
        if (!logger.enable()) return "";
        return logger->log(logger.level(), line, args...);
    }

    template <typename... Args>
    inline std::string log(const Logger &logger, CodeLine line, const Args &...args) {
        return logger.log(LOG_INFO, line, args...);
    }

    template <typename T, typename... Args>
    inline typename std::enable_if<
            !std::is_same<T, LogLevel>::value &&
            !std::is_convertible<T, LogStream>::value &&
            !std::is_convertible<T, const Logger &>::value,
            std::string>::type log(const T &t, const Args &...args) {
        return log(LOG_INFO, t, args...);
    }

#define ohm_codeline_construct(func, code, line) ohm::CodeLine({(func), (code), (line)})

#define ohm_codeline ohm_codeline_construct(__func__, ohm::__log_cut_filename(__FILE__), __LINE__)

/**
 * @param log can be PrintStream, Logger and LogLevel
 */
#define ohm_log(__log, ...) do {\
        auto &&ohm_auto_name(__ohm_log) = __log; \
        if (ohm::__log_enable(ohm_auto_name(__ohm_log)) \
                || ohm::__log_need_eject(ohm_auto_name(__ohm_log))) { \
            auto __message = ohm::log(ohm_auto_name(__ohm_log), ohm_codeline, ## __VA_ARGS__); \
            if (ohm::__log_need_fatal(ohm_auto_name(__ohm_log))) { \
                exit(0x7E); \
            } \
            if (ohm::__log_need_eject(ohm_auto_name(__ohm_log))) { \
                throw ohm::EjectionException(__message); \
            } \
        } \
    } while (false)
}

#endif //OMEGA_LOGGER_H
