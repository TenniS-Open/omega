//
// Created by kier on 2020/10/9.
//

#ifndef OMEGA_DISPATCHER_H
#define OMEGA_DISPATCHER_H

#include <vector>
#include <deque>

#include "cartridge.h"

namespace ohm {
    // used to provide thread pool for dispatcher, support dynamic threads
    /**
     * IncreasingThreadPool can provide any number of threads in cross sense.
     * But notice that, the stored thread number can only increase.
     * If you really what clean stored thread, use cleanup method.
     * Notice that, cleanup not thread safe, make sure no job fired when cleanup.
     */
    class IncreasingThreadPool {
    public:
        using self = IncreasingThreadPool;

        /**
         * @brief DispatcherEngine
         */
        IncreasingThreadPool() = default;

        ~IncreasingThreadPool() {
            this->dispose();
        }

        IncreasingThreadPool(const IncreasingThreadPool &) = delete;

        const IncreasingThreadPool &operator=(const IncreasingThreadPool &) = delete;

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
                                   std::bind(&self::recycling_cartridge, this, std::placeholders::_1)));
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
         * make sure engine will has `clip_size` thread initailzed.
         * @param clip_size capacity size
         */
        void reserve(size_t clip_size) {
            std::unique_lock<std::mutex> locker(m_chest_mutex);
            this->no_mutex_reserve(clip_size);
        }

        /**
         * @brief size Get number of threads
         * @return Number of threads
         */
        size_t size() const {
            return m_size;
        }

        /**
         * @brief size Get number of stored threads
         * @return Number of threads
         */
        size_t capacity() const {
            std::unique_lock<std::mutex> locker(m_chest_mutex);
            return m_clip.size();
        }

        /**
         * after resize, it make sure only clip_size thread will use
         * @param clip_size
         */
        void resize(size_t clip_size) {
            std::unique_lock<std::mutex> locker(m_chest_mutex);
            this->no_mutex_reserve(clip_size);
            m_size = clip_size;
            std::vector<int> to_load;
            std::vector<int> to_save;
            for (auto it = m_backup.begin(); it != m_backup.end();) {
                if (size_t(*it) < clip_size) {
                    to_load.push_back(*it);
                    it = m_backup.erase(it);
                } else {
                    ++it;
                }
            }
            for (auto it = m_chest.begin(); it != m_chest.end();) {
                if (size_t(*it) >= clip_size) {
                    to_save.push_back(*it);
                    it = m_chest.erase(it);
                } else {
                    ++it;
                }
            }
            m_backup.insert(m_backup.end(), to_save.begin(), to_save.end());
            m_chest.insert(m_chest.end(), to_load.begin(), to_load.end());
            if (to_load.size() == 1) {
                m_chest_cond.notify_one();
            } else {
                m_chest_cond.notify_all();
            }
        }

        /**
         * same resize(0)
         */
        void clear() {
            resize(0);
        }

        /**
         * cleanup all store threads
         * Notice that, cleanup not thread safe, make sure no job fired when cleanup.
         */
        void cleanup() {
            this->dispose();
            m_clip.clear();
            m_chest.clear();
            m_size = 0;
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
            if (signet < int(m_size)) {
                this->m_chest.push_back(signet);
                m_chest_cond.notify_one();
            } else {
                m_backup.push_back(signet);
            }
        }

        void dispose() {
            for (int i = 0; i < static_cast<int>(m_clip.size()); ++i) {
                delete m_clip[i];
            }
        }

        void no_mutex_reserve(size_t clip_size) {
            if (m_clip.size() >= clip_size) return;
            auto modified = clip_size - m_clip.size();
            for (auto i = m_clip.size(); i < clip_size; ++i) {
                m_clip.push_back(new Cartridge);
                m_chest.push_back(int(i));
            }
        }

        std::vector<Cartridge *> m_clip;          ///< all cartridges

        mutable std::mutex m_chest_mutex;                 ///< mutex to get cartridges
        mutable std::condition_variable m_chest_cond;     ///< active when cartridge pushed in chest
        std::deque<int> m_chest;                 ///< save all cartridge ready to fire

        std::deque<int> m_backup;   ///< save not used id to chest

        std::atomic<size_t> m_size; // using size
    };

    template<typename ...Args>
    class Dispatcher {
    public:
        using self = Dispatcher;

        using Action = std::function<void(Args...)>;

        explicit Dispatcher(std::shared_ptr<IncreasingThreadPool> threads)
                : m_threads(std::move(threads)) {}

        explicit Dispatcher(IncreasingThreadPool *threads)
                : self(std::shared_ptr<IncreasingThreadPool>(threads, [](IncreasingThreadPool *) {})) {}

        Dispatcher() {
            m_threads.reset(new IncreasingThreadPool);
        }

        ~Dispatcher() = default;

        Dispatcher(const Dispatcher &) = delete;

        const Dispatcher &operator=(const Dispatcher &) = delete;

        template<typename FUNC, typename=typename std::enable_if<
                std::is_constructible<Action, FUNC>::value>::type>
        void bind(FUNC func) {
            m_actions.push_back(Action(func));
            m_threads->resize(m_actions.size());
        }

        template<typename FUNC, typename=typename std::enable_if<
                std::is_constructible<std::function<void(int, Args...)>, FUNC>::value>::type>
        void bind(size_t N, FUNC func) {
            for (size_t i = 0; i < N; ++i) {
                auto action = [i, func](Args... args) {
                    func(int(i), args...);
                };
                this->bind(std::move(action));
            }
        }

        template<typename ...XArgs, typename=typename std::enable_if<
                std::is_copy_assignable<std::tuple<XArgs...>>::value &&
                std::is_same<void, decltype(std::declval<Action>()(std::declval<XArgs>()...))>::value>::type>
        void call(XArgs ...args) {
            m_threads->fire([=](int i) {
                m_actions[i](args...);
            });
        }

        void clear() {
            m_threads->clear();
            m_actions.clear();
        }

        /**
         * Call start if using shared `IncreasingThreadPool` before `call`.
         */
        void start() {
            m_threads->resize(m_actions.size());
        }

        void join() {
            m_threads->join();
        }

        void size() const {
            return m_actions.size();
        }

    private:
        std::shared_ptr<IncreasingThreadPool> m_threads;
        std::vector<Action> m_actions;
    };
}

#endif //OMEGA_DISPATCHER_H
