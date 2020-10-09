#ifndef OMEGA_THREAD_BULLET_H
#define OMEGA_THREAD_BULLET_H

#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>

namespace ohm {
    class Cartridge {
    public:
        using bullet_type = std::function<void(int)>;
        using shell_type = std::function<void(int)>;

        Cartridge()
                : m_dry(true), m_bullet(nullptr), m_shell(nullptr) {
            this->m_powder = std::thread(&Cartridge::operating, this);
        }

        ~Cartridge() {
            m_dry = false;
            m_fire_cond.notify_all();
            m_powder.join();
        }

        Cartridge(const Cartridge &that) = delete;

        const Cartridge &operator=(const Cartridge &that) = delete;

        /**
         * @brief fire Asynchronous build and fire bullet, first calls the bullet, then calls the shell.
         * @param signet the index to call `bullet(signet)` and `shell(signet)`
         * @param bullet the function call in thread
         * @param shell call it after bullet called
         */
        void fire(int signet, const bullet_type &bullet, const shell_type &shell = nullptr) {
            std::unique_lock<std::mutex> locker(m_fire_mutex);
            this->m_signet = signet;
            this->m_bullet = bullet;
            this->m_shell = shell;
            m_fire_cond.notify_all();
        }

        bool busy() {
            if (!m_fire_mutex.try_lock()) return false;
            bool is_busy = m_bullet != nullptr;
            m_fire_mutex.unlock();
            return is_busy;
        }

        void join() {
            std::unique_lock<std::mutex> locker(m_fire_mutex);
            while (m_bullet) m_fire_cond.wait(locker);
        }

    private:
        void operating() {
            std::unique_lock<std::mutex> locker(m_fire_mutex);
            while (m_dry) {
                while (m_dry && !m_bullet) m_fire_cond.wait(locker);
                if (!m_dry) break;
                m_bullet(m_signet);
                if (m_shell) m_shell(m_signet);
                m_bullet = nullptr;
                m_shell = nullptr;
                m_fire_cond.notify_all();
            }
        }

        std::mutex m_fire_mutex;              ///< mutex control each fire
        std::condition_variable m_fire_cond;  ///< condition to tell if fire finished
        std::atomic<bool> m_dry;              ///< object only work when dry is true

        int m_signet;                         ///< the argument to call `bullet(signet)` and `shell(signet)`
        bullet_type m_bullet = nullptr;      ///< main function call in thread
        shell_type m_shell = nullptr;        ///< side function call after `bullet` called

        std::thread m_powder;                 ///< working thread
    };
}

#endif // OMEGA_THREAD_BULLET_H
