//
// Created by kier on 2020/10/10.
//

#ifndef OMEGA_PIPE_H
#define OMEGA_PIPE_H

#include "ohm/thread/dispatcher_queue.h"
#include "ohm/type_iterable.h"

namespace ohm {
    /**
     * data generator function may throw this exception when no data generate
     */
    class PipeBreak : public std::exception {
    };

    /**
     * data processor function may throw this exception when data should be ignore
     */
    class PipeLeak : public std::exception {
    };

    template<typename FUNC, typename ARG, typename=void>
    struct is_pipe_mapper {
    public:
        static constexpr bool value = false;
        using mapped_type = void;
    };

    template<typename FUNC, typename ARG>
    struct is_pipe_mapper<FUNC, ARG, typename std::enable_if<
            std::is_same<
                    decltype(std::declval<int>()),
                    decltype(std::declval<FUNC>()(std::declval<ARG>()), std::declval<int>())>::value
    >::type> {
    public:
        template<typename T>
        struct traits {
            using type = typename std::remove_const<typename std::remove_reference<T>::type>::type;
        };

        static constexpr bool value = true;
        using mapped_type = typename traits<decltype(std::declval<FUNC>()(std::declval<ARG>()))>::type;
    };

    /**
     * \brief Pipe for data process.
     * Support data discarding.
     * Comming feature: generate data when process.
     * TODO: support one input to multi output.
     * @tparam T processing data type
     */
    template<typename T>
    class Pipe {
    public:
        using self = Pipe;
        using Type = T;

        Pipe()
                : m_queue(new DispatcherQueue<T>)
                , m_join_links(new std::vector<std::function<void(void)>>) {}

        ~Pipe() = default;

        /**
         * Add data to pipe
         * @param data
         */
        void push(T data) {
            m_queue->push(std::move(data));
        }

        /**
         * got data from pipe, is function could block.
         * @return data from pipe
         * @notice is function will block if queue is empty until data come or throw QueueEnd exception.
         */
        T pop() {
            return m_queue->pop();
        }

        /**
         *
         * @tparam FUNC map function type
         * @param N number of thread using
         * @param func map function
         * @return mapped pipe, called child pipe.
         * Notice the child pipe has already relied on this parent pipe.
         * `SO` do not capture parent or parent's parent in map API of child pipe.
         * It may cause circular reference.
         */
        template<typename FUNC, typename=typename std::enable_if<
                is_pipe_mapper<FUNC, T>::value &&
                (std::is_copy_constructible<FUNC>::value ||
                 std::is_move_constructible<FUNC>::value)>::type>
        auto map(size_t N, FUNC func) -> Pipe<typename is_pipe_mapper<FUNC, T>::mapped_type> {
            using mapped_type = typename is_pipe_mapper<FUNC, T>::mapped_type;
            Pipe<mapped_type> mapped;
            for (decltype(N) i = 0; i < N; ++i) {
                m_queue->bind([this, mapped, func](T data) {
                    try {
                        const_cast<Pipe<mapped_type> &>(mapped).push(func(data));
                    } catch (PipeLeak) {}
                });
            }
            m_join_links->emplace_back([mapped]() { const_cast<Pipe<mapped_type> &>(mapped).join(); });
            return mapped;
        }

        /**
         *
         * @param N number of thread using
         * @param func map function
         * @return mapped pipe, called child pipe.
         * Notice the child pipe has already relied on this parent pipe.
         * `SO` do not capture parent or parent's parent in map API of child pipe.
         * It may cause circular reference.
         */
        template<typename FUNC, typename=typename std::enable_if<
                is_pipe_mapper<FUNC, T>::value &&
                (std::is_copy_constructible<FUNC>::value ||
                 std::is_move_constructible<FUNC>::value)>::type>
        auto map(FUNC func) -> Pipe<typename is_pipe_mapper<FUNC, T>::mapped_type> {
            return this->map(1, func);
        }

        /**
         *
         * @param N number of thread using
         * @param func mapper function
         * @return mapped pipe, called child pipe.
         * Notice the child pipe has already relied on this parent pipe.
         * `SO` do not capture parent or parent's parent in map API of child pipe.
         * It may cause circular reference.
         */
        Pipe<T> map(size_t N, const std::function<void(T &)> &mapper) {
            return this->map(N, [mapper](T data) -> T {
                mapper(data);
                return data;
            });
        }

        /**
         * @param func mapper function
         * @return mapped pipe, called child pipe.
         * Notice the child pipe has already relied on this parent pipe.
         * `SO` do not capture parent or parent'
         */
        Pipe<T> map(const std::function<void(T &)> &mapper) {
            return this->map(1, mapper);
        }

        /**
         *
         * @param N number of thread using
         * @param func seal function
         */
        template<typename FUNC, typename=typename std::enable_if<
                is_pipe_mapper<FUNC, T>::value &&
                (std::is_copy_constructible<FUNC>::value ||
                 std::is_move_constructible<FUNC>::value)>::type>
        void seal(size_t N, FUNC func) {
            for (decltype(N) i = 0; i < N; ++i) {
                m_queue->bind([this, func](T data) {
                    try {
                        func(data);
                    } catch (PipeLeak) {}
                });
            }
        }

        /**
         *
         * @param func seal function
         */
        template<typename FUNC, typename=typename std::enable_if<
                is_pipe_mapper<FUNC, T>::value &&
                (std::is_copy_constructible<FUNC>::value ||
                 std::is_move_constructible<FUNC>::value)>::type>
        void seal(FUNC func) {
            seal(1, func);
        }

        /**
         * set queue size limit
         * @param size limit size
         * @return self
         * @notice if no limit set, there will be no limit as well.
         */
        self &limit(int64_t size) {
            m_queue->limit(size);
            return *this;
        }

        /**
         * Join to wait all data finish. if `recursion`, wait all child finish.
         * @param recursion
         */
        void join(bool recursion = true) {
            m_queue->join();
            if (recursion) {
                for (auto &f : *m_join_links) {
                    f();
                }
            }
        }

        DispatcherQueue<T> &queue() { return *m_queue; }

        const DispatcherQueue<T> &queue() const { return *m_queue; }

    private:
        std::shared_ptr<DispatcherQueue<T>> m_queue;
        std::shared_ptr<std::vector<std::function<void(void)>>> m_join_links;
    };

    template<>
    class Pipe<void> {
    public:
        using Type = void;
    };

    template<typename T>
    class Tap : public Pipe<T> {
    public:
        using supper = Pipe<T>;
        using self = Tap;

        using Generator = std::function<T()>;

        /**
         * Set generater to generate data to Tap
         * @tparam FUNC generator function type
         * @param func generator function
         * @notice the generator could throw PipeBreak to tell system no data will generate.
         */
        template<typename FUNC, typename=typename std::enable_if<
                std::is_constructible<Generator, FUNC>::value>::type>
        explicit Tap(FUNC func)
                : m_generator(func) {}

        /**
         * give initializer_list to generate
         * @tparam K data type
         * @param range dataset
         */
        template<typename K, typename=typename std::enable_if<
                std::is_constructible<K, T>::value>::type>
        explicit Tap(const std::initializer_list<K> &range)
                : self(range.begin(), range.end()) {}

        /**
         * give initializer_list to generate
         * @tparam Range range type
         * @param range range of dataset
         */
        template<typename Range, typename=typename std::enable_if<
                is_iterable<Range>::value &&
                std::is_convertible<typename has_iterator<Range>::forward_value_type, T>::value>::type>
        explicit Tap(const Range &range, int= 0)
                : self(range.begin(), range.end()) {}

        /**
         * give iterator piar to generate data
         * @tparam It iterator type
         * @param beg begin iterator
         * @param end end iterator
         */
        template<typename It, typename=typename std::enable_if<
                has_iterator_tag<It, std::input_iterator_tag>::value &&
                std::is_convertible<typename std::iterator_traits<It>::reference, T>::value>::type>
        explicit Tap(It beg, It end) {
            auto generator = [=]() {
                static auto it = beg;
                if (it == end) throw PipeBreak();
                auto tmp = *it;
                ++it;
                return tmp;
            };
            m_generator = generator;
        }

        /**
         * generate one data to queue.
         * if would throw PipeBreak exception if no data could be generated.
         */
        void generate() {
            this->push(m_generator());
        }

        /**
         * loop call `generate` until catch `PipeBreak`
         */
        void loop() {
            try {
                while (true) {
                    generate();
                }
            } catch (PipeBreak) {
                return;
            }
        }

    private:
        Generator m_generator;
    };
}

#endif //OMEGA_PIPE_H
