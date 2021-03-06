//
// Created by kier on 2020/9/15.
//

#ifndef OMEGA_THREAD_STATE_MODEL_H
#define OMEGA_THREAD_STATE_MODEL_H

#include "state_model.h"

#include <mutex>
#include <condition_variable>
#include <thread>

namespace ohm {
    namespace _ {
        template<typename MT>
        class UniqueLock {
        public:
            UniqueLock(MT *_mutex) {
                if (_mutex) {
                    _lock = std::make_shared<std::unique_lock<MT>>(*_mutex);
                }
            }

            ~UniqueLock() = default;

            UniqueLock(const UniqueLock &) = delete;

            UniqueLock(UniqueLock &&other)
                    : _lock(std::move(other.other)) {}

        private:
            std::shared_ptr<std::unique_lock<MT>> _lock;
        };

        /**
         * If event_lock is ture, he event should emit in single thread in same time,
         *     or use mute to prevent that.
         * else event can emit in multi thread with mutex cost.
         * Different with StateModel,
         * ThreadStateModel call loop to register action which would done in other thread automatically.
         * Remind to kill model firstly before any loop action could be active.
         */
        class ThreadStateModel {
        public:
            using self = ThreadStateModel;

            using Transition = std::function<StateCode()>;
            using VoidF = std::function<void()>;

            ThreadStateModel()
                    : m_state(STATE_START) {
            }

            ThreadStateModel(const ThreadStateModel &) = delete;

            ThreadStateModel(ThreadStateModel &&other)
                    : m_state(other.m_state.load()), m_transition(std::move(other.m_transition)),
                      m_enter(std::move(other.m_enter)) {
            }

            ~ThreadStateModel() {
                this->kill();
            }

            /**
             * Got now state
             * @return now state
             */
            StateCode state() const {
                return m_state;
            }

            /**
             * Emit event, the leave, transit and enter functions would done in this method.
             * The state will change to next state
             * @param code event code, which should be one of parameter in transit method.
             */
            void event(EventCode code) {
                UniqueLock<std::mutex> _lock(m_mutex_event.get());
                auto key = std::make_tuple(m_state.load(), code);
                auto it = m_transition.find(key);
                if (it == m_transition.end()) return;
                {
                    StateCode state = STATE_STILL;
                    this->before();
                    After _after([this, &state]() {
                        this->after(state);
                    });
                    state = it->second();
                }
            }

            /**
             * bind action when change other state to given state
             * @param state tell which will do `action` when enter the state.
             * @param action how action when enter state.
             */
            void enter(StateCode state, VoidF action) {
                m_enter[state] = std::move(action);
            }

            /**
             * bind action when given state change to other state
             * @param state tell which will do `action` when leave the state.
             * @param action how action when leave state.
             */
            void leave(StateCode state, VoidF action) {
                m_leave[state] = std::move(action);
            }

            /**
             * Set the transition function when emit event in `state`.
             * After transition function, state will change to the value of next.
             * @param state the current state when event emitted
             * @param event the emitted event
             * @param next the state will change to the value of next
             * @note The `next` should return STATE_STILL if state has not changed.
             * @note If `next` return same state to now state, the state's enter and leaving function will also be modified.
             */
            void transit(StateCode state, EventCode event, StateCode next) {
                transit(state, event, [next]() { return next; });
            }

            /**
             * Set the transition function when emit event in `state`.
             * After transition function, state will change to the return value of next.
             * @param state the current state when event emitted
             * @param event the emitted event
             * @param next the state will change to the return value of next
             * @param action transition function
             * @note The `next` should return STATE_STILL if state has not changed.
             * @note If `next` return same state to now state, the state's enter and leaving function will also be modified.
             */
            void transit(StateCode state, EventCode event, StateCode next,
                         const VoidF &action) {
                transit(state, event, [action, next]() {
                    action();
                    return next;
                });
            }

            /**
             * Set the transition function when emit event in `state`.
             * After transition function, state will change to the return value of next.
             * @param state the current state when event emitted
             * @param event the emitted event
             * @param next the transition function, the state will change to the return value of next
             * @note The `next` should return STATE_STILL if state has not changed.
             * @note If `next` return same state to now state, the state's enter and leaving function will also be modified.
             */
            void transit(StateCode state, EventCode event, const Transition &next) {
                // state should not be STATE_STILL or STATE_STOP
                auto key = std::make_tuple(state, event);
                m_transition[key] = next;
            }

            /**
             * Same as `transit(state, event.code(), next)`.
             * @param state the current state when event emitted
             * @param event the emitted event
             * @param next the transition function, the state will change to the return value of next
             */
            void transit(StateCode state, const Event<> &event,
                         const std::function<StateCode()> &next) {
                transit(state, event.code(), next);
            }

            /**
             * Set the transition function when emit event in `state`.
             * After transition function, state will change to the return value of next.
             * @tparam FUNC transition function type, must be StateCode(Args...)
             * @tparam Args transition function parameters' type
             * @param state the current state when event emitted
             * @param event the emitted event
             * @param next the transition function, the state will change to the return value of next
             * @note there is no event code set, because the emitter should parse parameters
             */
            template<typename FUNC, typename... Args,
                    typename=typename std::enable_if<
                            std::is_constructible<std::function<StateCode(Args...)>, FUNC>::value>::type>
            void transit(StateCode state, const Event<Args...> &event,
                         const FUNC &next) {
                using ThisEmitter = ArgsEmitter<StateCode(Args...)>;
                auto emitter = std::make_shared<ThisEmitter>(next);
                auto key = std::make_tuple(state, event.code());
                m_args_transition[key] = emitter;
            }

            /**
             * same as call event's emit method
             * @tparam EventArgs tell event type
             * @tparam Args event parameters' type
             * @param obj event
             * @param args event parameters
             */
            template<typename... EventArgs, typename... Args,
                    typename=typename std::enable_if<
                    std::is_same<StateCode,
                            decltype(std::declval<ArgsEmitter < StateCode(EventArgs...)>>().emit(
                            std::declval<Args>()...))>::value>::type>

            void event(const Event<EventArgs...> &event, Args... args) {
                this->event(event.code(), pass_value(args)...);
            }

            /**
             * Same as this->event(event.code())
             * @param event emitted event
             */
            void event(const Event<> &event) {
                this->event(event.code());
            }

            /**
             * Set if use mutex lock when emit every event.
             * Default is false.
             * @param if_lock if lock
             * @note do not call the method after state model active
             * If system has no condition about multi-thread update event, there is no need to event_lock(true).
             */
            void event_lock(bool if_lock) {
                if (if_lock) {
                    m_mutex_event = std::make_shared<std::mutex>();
                } else {
                    m_mutex_event.reset();
                }
            }

            /**
             * Bind action when system in `state`.
             * The action will be loop called.
             * If you what action only be called once, change state by `event`.
             * The action will called when system is in `state`.
             * @param state which state has default action
             * @param action default action.
             * @note DO NOT throw exception in action method, ONLY if you what end the state model.
             */
            void loop(StateCode state, VoidF action) {
                m_state_action[state] = std::move(action);
                if (!m_thread_backend) {
                    m_mutex_backend = std::make_shared<std::mutex>();
                    m_cond_event = std::make_shared<std::condition_variable>();
                    m_thread_backend = std::make_shared<std::thread>(std::bind(&self::backend, this));
                }
            }

            /**
             * Well, kill the model before any thing will done.
             * The backend thread would join atomically.
             * After this method, all backend thread will dead.
             */
            void kill() {
                UniqueLock<std::mutex> _lock(m_mutex_event.get());
                if (state() != STATE_DEAD) {
                    after(STATE_DEAD);
                }
                if (m_thread_backend) {
                    m_thread_backend->join();
                    m_thread_backend.reset();
                    m_cond_event.reset();
                    m_mutex_backend.reset();
                }
            }

        private:
            template<typename... Args>
            void event(EventCode code, Args... args) {
                UniqueLock<std::mutex> _lock(m_mutex_event.get());
                auto key = std::make_tuple(m_state.load(), code);
                auto it = m_args_transition.find(key);
                if (it == m_args_transition.end()) return;

                using ThisEmitter = ArgsEmitter<StateCode(Args...)>;

                auto emitter = dynamic_cast<ThisEmitter *>(it->second.get());
                if (emitter == nullptr) {
                    throw StateModelException(
                            "The transition function does not match: " + classname<StateCode(Args...)>());
                }
                {
                    StateCode state = STATE_STILL;
                    this->before();
                    After _after([this, &state]() {
                        this->after(state);
                    });
                    state = emitter->emit(pass_value<Args>(args)...);
                }
            }

            void before() {
            }

            void after(StateCode state) {
                if (state == STATE_STILL) return;
                StateCode now_state = m_state;
                StateCode new_state = state;
                auto it_now = m_leave.find(now_state);
                if (it_now != m_leave.end()) {
                    it_now->second();
                }
                auto it_new = m_enter.find(new_state);
                if (it_new != m_enter.end()) {
                    it_new->second();
                }
                m_state = state;
                if (m_cond_event) {
                    m_cond_event->notify_one();
                }
            }

            void backend() {
                std::unique_lock<std::mutex> _lock(*m_mutex_backend);
                while (true) {
                    auto state = m_state.load();
                    if (state == STATE_DEAD) return;
                    auto it = m_state_action.find(m_state.load());
                    if (it == m_state_action.end()) {
                        m_cond_event->wait(_lock);
                        continue;
                    }
                    do {
                        it->second();
                        if (m_state != state) break;
                    } while (true);
                }
            }

            std::atomic<StateCode> m_state;
            std::map<StateCode, VoidF> m_enter;
            std::map<StateCode, VoidF> m_leave;
            std::map<std::tuple<StateCode, EventCode>, Transition> m_transition;
            std::map<std::tuple<StateCode, EventCode>, std::shared_ptr<Emitter>> m_args_transition;

            // to control no multi-thread changing state.
            std::shared_ptr<std::mutex> m_mutex_event;

            // use to run backend thread, ONLY backend use.
            std::shared_ptr<std::mutex> m_mutex_backend;

            // notify if state changed.
            std::shared_ptr<std::condition_variable> m_cond_event;

            // backend thread, to run loop's action function automatically.
            std::shared_ptr<std::thread> m_thread_backend;

            // {state, {if_loop, <backend function>}}
            std::map<StateCode, VoidF> m_state_action;
        };
    }

    using ThreadStateModel = _::ThreadStateModel;
}

#endif //OMEGA_THREAD_STATE_MODEL_H
