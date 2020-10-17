//
// Created by kier on 2020/10/10.
//

#ifndef OMEGA_PIPE_H
#define OMEGA_PIPE_H

#include "../thread/dispatcher_queue.h"
#include "../type_iterable.h"

#include "pipe_profiler.h"

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

    template<typename FUNC, typename=void>
    struct is_data_generator {
    public:
        static constexpr bool value = false;
        using mapped_type = void;
    };

    template<typename FUNC>
    struct is_data_generator<FUNC, typename std::enable_if<
            !std::is_same<void, decltype(std::declval<FUNC>()())>::value>::type> {
    public:
        static constexpr bool value = true;
        using forward_return_type = decltype(std::declval<FUNC>()());
        using return_type = typename std::decay<forward_return_type>::type;
    };

    /**
     * \brief Pipe for data process.
     * Support data discarding and generate data when process.
     * @tparam T processing data type
     */
    template<typename T>
    class Pipe {
    public:
        using self = Pipe;
        using Type = T;

        Pipe(std::shared_ptr<PipeProfiler> profiler)
                : m_queue(new DispatcherQueue<T>)
                , m_join_links(new std::vector<std::function<void(void)>>)
                , m_profiler(std::move(profiler)) {}

        Pipe()
            : self(std::make_shared<PipeProfiler>()) {
        }

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
         * @param func map function, generate 1 data from 1 data. throw PipeLeak for no data generated
         * @return mapped pipe, called child pipe.
         * Notice the child pipe has already relied on this parent pipe.
         * `SO` do not capture parent or parent's parent in map API of child pipe.
         * It may cause circular reference.
         */
        template<typename FUNC, typename=typename std::enable_if<
                is_pipe_mapper<FUNC, T>::value &&
                (std::is_copy_constructible<FUNC>::value ||
                 std::is_move_constructible<FUNC>::value)>::type>
        auto map11(size_t N, FUNC func) -> Pipe<typename is_pipe_mapper<FUNC, T>::mapped_type> {
            using mapped_type = typename is_pipe_mapper<FUNC, T>::mapped_type;
            Pipe<mapped_type> mapped(m_profiler);
            auto processor = [this, mapped, func](T data) {
                try {
                    const_cast<Pipe<mapped_type> &>(mapped).push(func(data));
                } catch (PipeLeak) {}
            };
            if (N == 0) {
                m_queue->bind(processor, true);
            } else {
                for (decltype(N) i = 0; i < N; ++i) m_queue->bind(processor);
            }
            m_join_links->emplace_back([mapped]() { const_cast<Pipe<mapped_type> &>(mapped).join(); });
            return mapped;
        }

        /**
         *
         * @tparam FUNC map function type
         * @param N number of thread using
         * @param func map function, return iterable range value, like vector or list
         * @return mapped pipe, called child pipe.
         * Notice the child pipe has already relied on this parent pipe.
         * `SO` do not capture parent or parent's parent in map API of child pipe.
         * It may cause circular reference.
         */
        template<typename FUNC, typename=typename std::enable_if<
                is_pipe_mapper<FUNC, T>::value &&
                (std::is_copy_constructible<FUNC>::value ||
                 std::is_move_constructible<FUNC>::value) &&
                is_iterable<typename is_pipe_mapper<FUNC, T>::mapped_type>::value>::type>
        auto map1n(size_t N, FUNC func)
        -> Pipe<typename has_iterator<typename is_pipe_mapper<FUNC, T>::mapped_type>::value_type> {
            using mapped_type = typename has_iterator<typename is_pipe_mapper<FUNC, T>::mapped_type>::value_type;
            Pipe<mapped_type> mapped(m_profiler);
            auto processor = [this, mapped, func](T data) {
                try {
                    auto range = func(data);
                    auto &pipe = const_cast<Pipe<mapped_type> &>(mapped);
                    for (auto &out : range) {
                        pipe.push(out);
                    }
                } catch (PipeLeak) {}
            };
            if (N == 0) {
                m_queue->bind(processor, true);
            } else {
                for (decltype(N) i = 0; i < N; ++i) m_queue->bind(processor);
            }
            m_join_links->emplace_back([mapped]() { const_cast<Pipe<mapped_type> &>(mapped).join(); });
            return mapped;
        }

        /**
         *
         * @tparam FUNC map function type
         * @param N number of thread using
         * @param func map function, return data generator(function return one data for each call).
         * @return mapped pipe, called child pipe.
         * Notice the child pipe has already relied on this parent pipe.
         * `SO` do not capture parent or parent's parent in map API of child pipe.
         * It may cause circular reference.
         */
        template<typename FUNC, typename=typename std::enable_if<
                is_pipe_mapper<FUNC, T>::value &&
                (std::is_copy_constructible<FUNC>::value ||
                 std::is_move_constructible<FUNC>::value) &&
                is_data_generator<typename is_pipe_mapper<FUNC, T>::mapped_type>::value>::type>
        auto map1x(size_t N, FUNC func)
        -> Pipe<typename is_data_generator<typename is_pipe_mapper<FUNC, T>::mapped_type>::return_type> {
            using mapped_type = typename is_data_generator<typename is_pipe_mapper<FUNC, T>::mapped_type>::return_type;
            Pipe<mapped_type> mapped(m_profiler);
            auto processor = [this, mapped, func](T data) {
                try {
                    auto generator = func(data);
                    auto &pipe = const_cast<Pipe<mapped_type> &>(mapped);
                    try {
                        while (true) {
                            pipe.push(generator());
                        }
                    } catch (PipeBreak) {}
                } catch (PipeLeak) {}
            };
            if (N == 0) {
                m_queue->bind(processor, true);
            } else {
                for (decltype(N) i = 0; i < N; ++i) m_queue->bind(processor);
            }
            m_join_links->emplace_back([mapped]() { const_cast<Pipe<mapped_type> &>(mapped).join(); });
            return mapped;
        }

        struct IsMap11 {
        };

        struct IsMap1n {
        };

        struct IsMap1x {
        };

        /**
         *
         * @tparam FUNC map function type
         * @param N number of thread using
         * @param func map function, generate 1 data from 1 data. throw PipeLeak for no data generated
         * @return mapped pipe, called child pipe.
         * Notice the child pipe has already relied on this parent pipe.
         * `SO` do not capture parent or parent's parent in map API of child pipe.
         * It may cause circular reference.
         */
        template<typename FUNC, typename=typename std::enable_if<
                is_pipe_mapper<FUNC, T>::value &&
                (std::is_copy_constructible<FUNC>::value ||
                 std::is_move_constructible<FUNC>::value) &&
                !is_iterable<typename is_pipe_mapper<FUNC, T>::mapped_type>::value &&
                !is_data_generator<typename is_pipe_mapper<FUNC, T>::mapped_type>::value>::type>
        auto map(size_t N, FUNC func, IsMap11= {})
        -> Pipe<typename is_pipe_mapper<FUNC, T>::mapped_type> {
            return map11(N, func);
        }

        /**
         *
         * @tparam FUNC map function type
         * @param N number of thread using
         * @param func map function, return iterable range value, like vector or list
         * @return mapped pipe, called child pipe.
         * Notice the child pipe has already relied on this parent pipe.
         * `SO` do not capture parent or parent's parent in map API of child pipe.
         * It may cause circular reference.
         */
        template<typename FUNC, typename=typename std::enable_if<
                is_pipe_mapper<FUNC, T>::value &&
                (std::is_copy_constructible<FUNC>::value ||
                 std::is_move_constructible<FUNC>::value) &&
                is_iterable<typename is_pipe_mapper<FUNC, T>::mapped_type>::value>::type>
        auto map(size_t N, FUNC func, IsMap1n= {})
        -> Pipe<typename has_iterator<typename is_pipe_mapper<FUNC, T>::mapped_type>::value_type> {
            return map1n(N, func);
        }

        /**
         *
         * @tparam FUNC map function type
         * @param N number of thread using
         * @param func map function, return data generator(function return one data for each call).
         * @return mapped pipe, called child pipe.
         * Notice the child pipe has already relied on this parent pipe.
         * `SO` do not capture parent or parent's parent in map API of child pipe.
         * It may cause circular reference.
         */
        template<typename FUNC, typename=typename std::enable_if<
                is_pipe_mapper<FUNC, T>::value &&
                (std::is_copy_constructible<FUNC>::value ||
                 std::is_move_constructible<FUNC>::value) &&
                is_data_generator<typename is_pipe_mapper<FUNC, T>::mapped_type>::value>::type>
        auto map(size_t N, FUNC func, IsMap1x= {})
        -> Pipe<typename is_data_generator<typename is_pipe_mapper<FUNC, T>::mapped_type>::return_type> {
            return map1x(N, func);
        }

        /**
         *
         * @param N number of thread using
         * @param func map function, could be map11 map1n and map1x function
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
            return this->map(0, func);
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
            return this->map(0, mapper);
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
            auto processor = [this, func](T data) {
                try {
                    func(data);
                } catch (PipeLeak) {}
            };
            if (N == 0) {
                m_queue->bind(processor, true);
            } else {
                for (decltype(N) i = 0; i < N; ++i) m_queue->bind(processor);
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
            seal(0, func);
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

        /**
         * Do profile by given name
         * @param name profile's queue name
         * @return self
         */
        self &profile(const std::string &name) {
            if (!m_profiler) m_profiler.reset(new PipeProfiler);
            auto queue = m_queue;
            auto callback = m_profiler->callback(
                    name,
                    [queue]() -> int64_t {
                        return queue->capacity();
                    },
                    [queue]() -> int64_t {
                        return int64_t(queue->threads());
                    });
            m_queue->set_io_action(callback.in, callback.out);
            m_queue->set_time_reporter(callback.time);
            return *this;
        }

        /**
         * Stop do profile
         * @return self
         */
        self &profile(std::nullptr_t) {
            m_queue->clear_io_action();
            m_queue->clear_time_reporter();
            return *this;
        }

        /**
         * Got report of all queue
         * @return
         */
        PipeProfiler::Report report() {
            if (m_profiler) {
                return m_profiler->report();
            } else {
                return PipeProfiler::Report();
            }
        }

        DispatcherQueue<T> &queue() { return *m_queue; }

        const DispatcherQueue<T> &queue() const { return *m_queue; }

    private:
        std::shared_ptr<DispatcherQueue<T>> m_queue;
        std::shared_ptr<std::vector<std::function<void(void)>>> m_join_links;

        std::shared_ptr<PipeProfiler> m_profiler;
    };

    template<>
    class Pipe<void> {
    public:
        using Type = void;
    };

    template<typename T, typename=typename std::enable_if<
            std::is_copy_constructible<T>::value>::type>
    inline std::shared_ptr<T> capture(const T &data) {
        return std::make_shared<T>(data);
    }

    template<typename T, typename=typename std::enable_if<
            !std::is_reference<T>::value &&
            std::is_move_constructible<T>::value>::type>
    inline std::shared_ptr<T> capture(T &&data) {
        return std::make_shared<T>(std::move(data));
    }

    template<typename T>
    inline std::function<T()>
    make_generator(std::initializer_list<T> range) {
        struct Local {
            std::initializer_list<T> range;
            typename std::initializer_list<T>::iterator it;

            Local(std::initializer_list<T> range)
                    : range(std::move(range)) {
                it = this->range.begin();
            }
        };
        auto tmp = std::make_shared<Local>(std::move(range));
        return [tmp]() -> T {
            if (tmp->it == tmp->range.end()) throw PipeBreak();
            return *(tmp->it)++;
        };
    }

    template<typename Range, typename=typename std::enable_if<
            is_iterable<Range>::value>::type>
    inline std::function<typename has_iterator<Range>::value_type()>
    make_generator(Range range) {
        struct Local {
            Range range;
            typename std::decay<decltype(std::declval<Range>().begin())>::type it;

            Local(Range range)
                    : range(std::move(range)) {
                it = this->range.begin();
            }
        };
        auto tmp = std::make_shared<Local>(std::move(range));
        return [tmp]() -> typename has_iterator<Range>::value_type {
            if (tmp->it == tmp->range.end()) throw PipeBreak();
            return *(tmp->it)++;
        };
    }

    template<typename It, typename=typename std::enable_if<
            has_iterator_tag<It, std::input_iterator_tag>::value>::type>
    inline std::function<typename std::iterator_traits<It>::value_type()>
    make_generator(It beg, It end) {
        struct Local {
            It end;
            It it;
        };
        auto tmp = std::make_shared<Local>();
        tmp->end = end;
        tmp->it = beg;
        return [tmp]() -> typename std::iterator_traits<It>::value_type {
            if (tmp->it == tmp->end) throw PipeBreak();
            return *(tmp->it)++;
        };
    }

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
        explicit Tap(std::initializer_list<K> range)
                : self(make_generator(std::move(range))) {}

        struct IsRange {
        };

        /**
         * give initializer_list to generate
         * @tparam Range range type
         * @param range range of dataset
         */
        template<typename Range, typename=typename std::enable_if<
                is_iterable<Range>::value &&
                std::is_convertible<typename has_iterator<Range>::forward_value_type, T>::value>::type>
        explicit Tap(Range range, IsRange= {})
                : self(make_generator(std::move(range))) {}

        /**
         * give iterator piar to generate data
         * @tparam It iterator type
         * @param beg begin iterator
         * @param end end iterator
         */
        template<typename It, typename=typename std::enable_if<
                has_iterator_tag<It, std::input_iterator_tag>::value &&
                std::is_convertible<typename std::iterator_traits<It>::reference, T>::value>::type>
        explicit Tap(It beg, It end)
                : self(make_generator(beg, end)) {}

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

        /**
         * loop call `generate` `times` times until catch `PipeBreak`
         * @param times loop times
         */
        template<typename I, typename=Required<std::is_integral<I>>>
        void loop(I times) {
            try {
                for (I i = 0; i < times; ++i) {
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
