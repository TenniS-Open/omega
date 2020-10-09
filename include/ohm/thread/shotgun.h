#ifndef OMEGA_THREAD_SHOTGUN_H
#define OMEGA_THREAD_SHOTGUN_H

#include "cartridge.h"

#include <vector>
#include <deque>

namespace ohm {
/**
 * @brief The Shotgun class the thread pool
 */
    class Shotgun {
    public:
        /**
         * @brief Shotgun
         * @param clip_size The cartridge number in clip. Number of threads
         */
        Shotgun(size_t clip_size)
                : m_clip(clip_size) {
            for (int i = 0; i < static_cast<int>(clip_size); ++i) {
                m_clip[i] = new Cartridge();
                m_chest.push_back(i);   // push all cartridge into chest
            }
        }

        ~Shotgun() {
            for (int i = 0; i < static_cast<int>(m_clip.size()); ++i) {
                delete m_clip[i];
            }
        }

        Shotgun(const Shotgun &that) = delete;

        const Shotgun &operator=(const Shotgun &that) = delete;

        /**
         * @brief fire Find ready cartridge, build bullet and fire.
         * @param bullet the work ready to run
         * @return The cartridge running bullet
         */
        Cartridge *fire(const Cartridge::bullet_type &bullet) {
            if (m_clip.size() == 0) {
                bullet(0);
                return nullptr;
            } else {
                int signet = load();
                Cartridge *cart = this->m_clip[signet];
                cart->fire(signet, bullet,
                           Cartridge::shell_type(
                                   std::bind(&Shotgun::recycling_cartridge, this, std::placeholders::_1)));
                return cart;
            }
        }

        /**
         * @brief fire Find ready cartridge, build bullet and fire.
         * @param bullet the work ready to run
         * @return The cartridge running bullet
         */
        template<typename FUNC, typename=typename std::enable_if<
                std::is_constructible<std::function<void()>, FUNC>::value>::type>
        Cartridge *fire(FUNC func) {
            auto tmp = std::function<void()>(func);
            return this->fire(Cartridge::bullet_type([tmp](int) { tmp(); }));
        }

        /**
         * @brief fire Find ready cartridge, build bullet and fire.
         * @param bullet the work ready to run
         * @param shell the work after bullet finished
         * @return The cartridge running bullet
         */
        Cartridge *fire(const Cartridge::bullet_type &bullet, const Cartridge::shell_type &shell) {
            if (m_clip.size() == 0) {
                bullet(0);
                return nullptr;
            } else {
                int signet = load();
                Cartridge *cart = this->m_clip[signet];
                cart->fire(signet, bullet, [this, shell](int id) -> void {
                    shell(id);
                    this->recycling_cartridge(id);
                });
                return cart;
            }
        }

        /**
         * @brief join Wait all cartridge working finish.
         */
        void join() {
            std::unique_lock<std::mutex> locker(m_chest_mutex);
            while (this->m_chest.size() != this->m_clip.size()) m_chest_cond.wait(locker);
        }

        /**
         * @brief busy Return if there are work running in thread
         * @return True if busy
         */
        bool busy() {
            if (!m_chest_mutex.try_lock()) return false;
            bool is_busy = this->m_chest.size() != this->m_clip.size();
            m_chest_mutex.unlock();
            return is_busy;
        }

        /**
         * @brief size Get number of threads
         * @return Number of threads
         */
        size_t size() const {
            return m_clip.size();
        }

    private:
        /**
         * @brief load Get cartridge ready to fire
         * @return Get ready cartridge
         */
        int load() {
            std::unique_lock<std::mutex> locker(m_chest_mutex);
            while (this->m_chest.empty()) m_chest_cond.wait(locker);
            int signet = this->m_chest.front();
            this->m_chest.pop_front();
            return signet;
        }

        /**
         * @brief recycling_cartridge Recycle cartridge
         * @param signet cartridge index
         */
        void recycling_cartridge(int signet) {
            std::unique_lock<std::mutex> locker(m_chest_mutex);
            this->m_chest.push_back(signet);
            m_chest_cond.notify_all();
        }

        std::vector<Cartridge *> m_clip;          ///< all cartridges

        std::mutex m_chest_mutex;                 ///< mutex to get cartridges
        std::condition_variable m_chest_cond;     ///< active when cartridge pushed in chest
        std::deque<int> m_chest;                 ///< save all cartridge ready to fire
    };
}

#endif // OMEGA_THREAD_SHOTGUN_H
