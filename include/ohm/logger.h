//
// Created by kier on 2020/7/31.
//

#ifndef OMEGA_LOGGER_H
#define OMEGA_LOGGER_H

#include <algorithm>
#include <ostream>
#include <sstream>
#include <fstream>
#include <atomic>
#include <mutex>

#include "loglevel.h"
#include "print.h"
#include "macro.h"
#include "except.h"
#include "time.h"
#include "platform.h"
#include "filesys.h"

#if OHM_PLATFORM_OS_ANDROID
#include "sys/android.h"
#endif

namespace ohm {
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
        LogLevel m_level;
        const Logger *m_logger;
    };


    class Logger : public PrintStream {
    public:
        explicit Logger(const std::string &tag, LogLevel level = LOG_INFO)
            : m_level(level), m_tag(tag), m_enable_to_file(false) {
        }

        ~Logger() override {
            delete m_log2file;
        }

        void tag(std::string tag) {
            m_tag = std::move(tag);
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
            std::string body = sprint(line, ": ", args...);
            auto msg = sprint("[", now(), "]", log_string(level), "<", tag, ">", body);

            log2console(stream, msg);
            log2platform(level, body);
            log2file(msg);

            return msg;
        }

        LogStream operator()(LogLevel level) const { return LogStream(level, *this); }

        LogLevel change_level(LogLevel level) {
            auto tmp = m_level;
            m_level = level;
            return tmp;
        }

        LogLevel level() const { return m_level; }

        void stream2file(const std::string &filename) {
            std::lock_guard<decltype(m_mutex_file)> _lock(m_mutex_file);

            if (filename.empty()) {
                m_tofile = "";
                m_current = "";
                delete m_log2file;
                m_log2file = nullptr;
                m_enable_to_file.store(false);
                return;
            }
            m_tofile = get_absolute(getcwd(), filename);
            auto dot = m_tofile.rfind('.');
            if (dot == std::string::npos) {
                m_current_prefix = m_tofile;
                m_current_suffix = "";
            } else {
                m_current_prefix = m_tofile.substr(0, dot);
                m_current_suffix = m_tofile.substr(dot);
            }
            m_enable_to_file.store(true);
        }

    private:
        LogLevel m_level;
        std::string m_tag = "omega";

        std::atomic<bool> m_enable_to_file;
        std::string m_tofile;

        std::string m_current_prefix;
        std::string m_current_suffix;
        mutable std::string m_current;
        mutable std::ostream *m_log2file = nullptr;
        mutable std::mutex m_mutex_file;

        void log2console(std::ostream &console, const std::string &msg) const {
            println(console, msg);
        }

        void log2console(const std::string &msg) const {
            log2console(std::cout, msg);
        }

        void log2file(const std::string &msg) const {
            if (!m_enable_to_file.load()) {
                return;
            }
            std::lock_guard<decltype(m_mutex_file)> _lock(m_mutex_file);

            if (m_tofile.empty()) return;

            // check current
            auto current = current_with_date();
            if (current != m_current) {
                delete m_log2file;
                m_log2file = nullptr;

                auto root = cut_path_tail(current);
                mkdir(root);

                auto file = new std::ofstream(current, std::ios_base::app);
                if (!file->is_open()) {
                    delete file;
                    log2console("[ERROR] Can not open log: " + current);
                } else {
                    m_log2file = file;
                    m_current = current;
                }
            }

            // write log
            if (m_log2file) {
                println(*m_log2file, msg);
            }
        }

        void log2platform(LogLevel level, const std::string &msg) const {
#if OHM_PLATFORM_OS_ANDROID
            android::log(level, m_tag, msg);
#endif
        }

        std::string current_with_date() const {
            auto date = to_string(now(), "%Y-%m-%d");
            std::ostringstream oss;
            oss << m_current_prefix << "_" << date << m_current_suffix;
            return oss.str();
        }
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
