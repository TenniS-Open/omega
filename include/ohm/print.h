//
// Created by kier on 2020/7/21.
//

#ifndef OMEGA_PRINT_H
#define OMEGA_PRINT_H

#include <utility>
#include <iostream>
#include <sstream>
#include <map>

#include "type_iterable.h"
#include "type_callable.h"
#include "type_cast.h"

namespace ohm {
    class PrintStream {
    public:
        using self = PrintStream;

        virtual ~PrintStream() = default;

        virtual void print(const std::string &msg) const = 0;
    };

    class Sep {
    public:
        explicit Sep(std::string sep, int n = -1) : sep(std::move(sep)), n(n) {}

        std::string sep;
        int n;
    };

    class SepNil {
    };

    template<typename T, typename Enable = void>
    struct printable {
        // void print(std::ostream &out, const T &);
    };

    template<typename...Args>
    struct is_printable : public std::false_type {
    };

    template<typename T>
    struct __has_defined_function_operator_left_shift {
    private:
        template<typename U>
        static auto Check(int) -> decltype(operator<<(std::declval<std::ostream &>(), std::declval<U>()));

        template<typename U>
        static auto Check(...) -> void;

    public:
        static constexpr bool value =
                std::is_convertible<decltype(Check<T>(0)), std::ostream &>::value;
    };

    template<typename T>
    struct has_defined_function_operator_left_shift
            : public std::integral_constant<bool, __has_defined_function_operator_left_shift<T>::value> {
    };

    template<typename T>
    struct __has_defined_member_operator_left_shift {
    private:
        template<typename U>
        static auto Check(int) -> decltype(std::declval<std::ostream>().operator<<(std::declval<U>()));

        template<typename U>
        static auto Check(...) -> void;

    public:
        static constexpr bool value =
                std::is_convertible<decltype(Check<T>(0)), std::ostream &>::value;
    };

    template<typename T>
    struct has_defined_member_operator_left_shift
            : public std::integral_constant<bool, __has_defined_member_operator_left_shift<T>::value> {
    };

    template<typename T>
    struct __has_defined_printable {
    private:
        template<typename U>
        static auto Check(int) -> decltype(std::declval<printable<typename remove_cr<U>::type>>().print(
                std::declval<std::ostream &>(), std::declval<U>()), std::declval<std::ostream &>());

        template<typename U>
        static auto Check(...) -> void;

    public:
        static constexpr bool value =
                std::is_convertible<decltype(Check<T>(0)), std::ostream &>::value;
    };

    template<typename T>
    struct has_defined_printable
            : public std::integral_constant<bool, __has_defined_printable<T>::value> {
    };

    template<typename T>
    struct is_printable<T> : public std::integral_constant<bool,
            has_defined_function_operator_left_shift<T>::value ||
            has_defined_member_operator_left_shift<T>::value ||
            has_defined_printable<T>::value> {
    };

    template<typename T, typename...Args>
    struct is_printable<T, Args...> : public std::integral_constant<bool,
            is_printable<T>::value && is_printable<Args...>::value> {
    };

    template<>
    struct is_printable<> : public std::true_type {
    };

    inline std::ostream &stream_print(std::ostream &out) {
        return out;
    }

    template<typename T>
    inline typename std::enable_if<
            has_defined_printable<T>::value,
            std::ostream &>::type
    stream_print(std::ostream &out, const T &t) {
        printable<typename remove_cr<T>::type>().print(out, t);
        return out;
    }

    template<typename T>
    inline typename std::enable_if<
            has_defined_function_operator_left_shift<T>::value &&
            !has_defined_printable<T>::value,
            std::ostream &>::type
    stream_print(std::ostream &out, const T &t) {
        return operator<<(out, t);
    }

    template<typename T>
    inline typename std::enable_if<
            has_defined_member_operator_left_shift<T>::value &&
            !has_defined_function_operator_left_shift<T>::value &&
            !has_defined_printable<T>::value,
            std::ostream &>::type
    stream_print(std::ostream &out, const T &t) {
        return out << t;
    }

    template<typename T, typename... Args>
    inline typename std::enable_if<is_printable<T>::value, std::ostream &>::type
    stream_print(std::ostream &out, const T &t, const Args &...args) {
        return stream_print(stream_print(out, t), args...);
    }

    inline std::ostream &stream_print(std::ostream &out, const SepNil &) { return out; }

    inline std::ostream &stream_print(std::ostream &out, const Sep &) { return out; }

    template<typename... Args>
    inline std::ostream &stream_print(std::ostream &out, const SepNil &, const Args &...args) {
        return stream_print(out, args...);
    }

    template<typename T>
    inline std::ostream &stream_print(std::ostream &out, const Sep &, const T &t) {
        return stream_print(out, t);
    }

    template<typename... Args>
    inline std::ostream &stream_print(std::ostream &out, const Sep &sep, const SepNil &, const Args &...args) {
        return stream_print(out, args...);
    }

    template<typename... Args>
    inline std::ostream &stream_print(std::ostream &out, const Sep &, const Sep &sep, const Args &...args) {
        return stream_print(out, sep, args...);
    }

    template<typename T, typename... Args>
    inline std::ostream &
    stream_print(std::ostream &out, const Sep &sep, const T &t, const SepNil &, const Args &...args) {
        return stream_print(out, t, args...);
    }

    template<typename T, typename... Args>
    inline std::ostream &
    stream_print(std::ostream &out, const Sep &sep1, const T &t, const Sep &sep2, const Args &...args) {
        if (sep1.n == 0) return stream_print(out, t, sep2, args...);
        stream_print(out, t);
        stream_print(out, sep1.sep);
        return stream_print(out, sep2, args...);
    }

    template<typename T, typename... Args>
    inline std::ostream &
    stream_print(std::ostream &out, const Sep &sep, const T &t1, const T &t2, const Args &...args) {
        if (sep.n == 0) return stream_print(out, t1, t2, args...);
        stream_print(out, t1);
        stream_print(out, sep.sep);
        if (sep.n < 0) {
            return stream_print(out, sep, t2, args...);
        } else {
            return stream_print(out, Sep(sep.sep, sep.n - 1), t2, args...);
        }
    }

    template<typename... Args>
    inline typename std::enable_if<is_printable<Args...>::value, std::string>::type
    sprint(const Args &..._) {
        std::ostringstream oss;
        stream_print(oss, _...);
        return oss.str();
    }

    template<typename... Args>
    inline void print(const Args &..._) {
        std::cout << sprint(_...);
    }

    template<typename... Args>
    inline std::string sprintln(const Args &..._) {
        return sprint(_..., "\n");
    }

    template<typename... Args>
    inline void println(const Args &..._) {
        std::cout << sprint(_..., "\n");
        std::cout.flush();
    }

    template<typename... Args>
    inline void print(std::ostream &stream, const Args &..._) {
        stream_print(stream, _...);
    }

    template<typename... Args>
    inline void println(std::ostream &stream, const Args &..._) {
        stream_print(stream, _..., "\n");
        stream.flush();
    }

    template<typename T, typename... Args,
            typename = typename std::enable_if<std::is_base_of<PrintStream, T>::value>::type>
    inline void print(const T &stream, const Args &..._) {
        stream.print(sprint(_...));
    }

    template<typename T, typename... Args,
            typename = typename std::enable_if<std::is_base_of<PrintStream, T>::value>::type>
    inline void println(const T &stream, const Args &..._) {
        stream.print(sprint(_..., "\n"));
    }

    template <typename T>
    struct is_print_stream {
        static constexpr bool value = std::is_base_of<PrintStream, T>::value ||
                std::is_base_of<std::ostream, T>::value;
    };

    template<typename T>
    struct printable<T, typename std::enable_if<
            is_iterable<T>::value &&
            is_printable<typename has_iterator<T>::value_type>::value &&
            !has_defined_function_operator_left_shift<T>::value &&
            !has_defined_member_operator_left_shift<T>::value
    >::type> {
        void print(std::ostream &out, const T &t) {
            auto beg = t.begin();
            auto end = t.end();
            bool comma = false;
            out << "[";
            for (auto it = beg; it != end; ++it) {
                if (comma) {
                    out << ", ";
                } else {
                    comma = true;
                }
                static_assert(is_printable<typename remove_cr<decltype(*it)>::type>::value, "What a Terrible Failure!");
                stream_print(out, *it);
            }
            out << "]";
        }
    };

    template<typename K, typename V>
    struct printable<std::pair<K, V>, typename std::enable_if<
            is_printable<K>::value && is_printable<V>::value>::type> {
        void print(std::ostream &out, const std::pair<K, V> &x) {
            static_assert(is_printable<decltype(x.first)>::value, "What a Terrible Failure!");
            static_assert(is_printable<decltype(x.second)>::value, "What a Terrible Failure!");
            out << "(";
            stream_print(out, x.first);
            out << ", ";
            stream_print(out, x.second);
            out << ")";
        }
    };

    template<typename... Args>
    struct printable<std::tuple<Args...>, typename std::enable_if<
            is_printable<Args...>::value>::type> {
        using type = std::tuple<Args...>;

        void print(std::ostream &out, const type &x) {
            _print<std::tuple_size<type>::value>(out, x);
        }

    private:
        template<size_t N>
        void _print(typename std::enable_if<0 == N, std::ostream &>::type out,
                    const type &) {
            out << "()";
        }

        template<size_t N>
        void _print(typename std::enable_if<0 < N, std::ostream &>::type out,
                    const type &x) {
            out << "(";
            _print_at<N, 0>(out, x);
            out << ")";
        }

        template<size_t N, size_t I>
        void _print_at(typename std::enable_if<I == N - 1, std::ostream &>::type out,
                       const type &x) {
            stream_print(out, std::get<I>(x));
        }

        template<size_t N, size_t I>
        void _print_at(typename std::enable_if<I < N - 1, std::ostream &>::type out,
                       const type &x) {
            stream_print(out, std::get<I>(x), ", ");
            _print_at<N, I + 1>(out, x);
        }
    };

    template<>
    struct printable<Sep> {
        using type = Sep;

        void print(std::ostream &out, const type &x) { out << x.sep; }
    };

    template<>
    struct printable<SepNil> {
        using type = SepNil;

        void print(std::ostream &out, const type &x) {}
    };

    template<>
    struct printable<std::ostream &(std::ostream &)> {
        using type = std::ostream &(std::ostream &);

        void print(std::ostream &out, type x) {
            x(out);
        }
    };

    inline Sep sep(const std::string &s) { return Sep(s); }

    inline Sep sep(const std::string &s, int n) { return Sep(s, n); }

    inline SepNil sep() { return {}; }

    inline std::ostream &endl(std::ostream &out) { return out << std::endl; }
}

#endif //OMEGA_PRINT_H
