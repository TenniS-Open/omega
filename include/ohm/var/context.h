//
// Created by kier on 2020/8/25.
//

#ifndef OMEGA_VAR_CONTEXT_H
#define OMEGA_VAR_CONTEXT_H

#include <vector>
#include <stack>
#include <sstream>

namespace ohm {
    namespace vario {
        class Context {
        public:
            Context(size_t size = 64) {
                if (size <= 0) size = 1;
                m_size = size;
                m_data.resize(m_size);
                m_data[0] = '\0';
            }

            Context(const Context &) = delete;
            Context &operator=(const Context &) = delete;

            void push(const std::string &seg) {
                push_seg(seg);
            }

            template <typename T>
            static void pack(std::ostream &out, T &&t) {
                out << t;
            }

            template <typename T, typename...Args>
            static void pack(std::ostream &out, T &&t, Args &&...args) {
                out << t;
                pack(out, std::forward<Args>(args)...);
            }

            template <typename T, typename... Args>
            void push(T &&t, Args &&...args) {
                std::ostringstream oss;
                pack(oss, std::forward<T>(t), std::forward<Args>(args)...);
                push_seg(oss.str());
            }

            void pop() {
                m_stack.pop();
                auto base = m_stack.empty() ? 0 : m_stack.top();
                m_data[base] = '\0';
            }

            std::string str() const {
                return m_data.data();
            }

            operator std::string() const {
                return this->str();
            }

            const std::string &sysroot() const { return m_sysroot; }

            void sysroot(std::string sysroot) {
                m_sysroot = std::move(sysroot);
            }

        private:
            void push_seg(const std::string &seg) {
                auto base = m_stack.empty() ? 0 : m_stack.top();
                auto next = base + seg.size();
                if (next > m_size) {
                    do {
                        m_size *= 2;
                    } while (next > m_size);
                    m_data.resize(m_size);
                }
                std::snprintf(m_data.data() + base, m_size - base, "%s", seg.c_str());
                m_stack.push(next);
            }

            std::vector<char> m_data;
            std::stack<size_t> m_stack;
            size_t m_size;
            std::string m_sysroot;
        };
    }
}

#endif //OMEGA_VAR_CONTEXT_H
