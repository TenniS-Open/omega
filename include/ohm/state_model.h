//
// Created by kier on 2020/9/15.
//

#ifndef OMEGA_STATE_MODEL_H
#define OMEGA_STATE_MODEL_H

#include <memory>
#include <functional>
#include <map>
#include <stdexcept>
#include <utility>
#include <atomic>

namespace ohm {
    using StateCode = int;
    using EventCode = int;

    enum ReservedStateCode {
        // means the start state
        STATE_START = 0,
        // means the end state
        STATE_END = -1,
        // can return by transition function, mean not change state
        STATE_STILL = -2,
        // means this state model never used.
        STATE_DEAD = -3,
    };

    class StateModelException : std::logic_error {
    public:
        using self = StateModelException;
        using supper = std::logic_error;

        explicit StateModelException(const std::string &message)
                : supper(message) {}
    };

    namespace _ {
        template<typename T>
        struct return_type;

        template<typename RET, typename... Args>
        struct return_type<RET(Args...)> {
            using type = RET;
        };

        template<typename T>
        struct void_function;

        template<typename RET, typename... Args>
        struct void_function<RET(Args...)> {
            using type = void ((Args...));
        };

        class After {
        public:
            using VoidF = std::function<void()>;

            After(const After &) = delete;

            explicit After(VoidF after)
                    : m_after(std::move(after)) {}

            ~After() {
                m_after();
            }

        private:
            mutable VoidF m_after;
        };

        template<typename T, typename=typename std::enable_if<
                std::is_literal_type<T>::value>::type>
        inline T pass_value(T t) {
            return t;
        }

        template<typename T, typename=typename std::enable_if<
                !std::is_literal_type<T>::value &&
                (std::is_move_constructible<T>::value || std::is_copy_constructible<T>::value)>::type>
        inline T pass_value(T &t) {
            return std::move(t);
        }

        template<typename T>
        class ArgsEmitter;

        template<typename... Args>
        class ArgsEmitter<StateCode(Args...)> {
        public:
            using CheckF = std::function<bool()>;
            using NextF = std::function<StateCode(Args...)>;
            using BeforeF = std::function<void()>;
            using AfterF = std::function<void(StateCode)>;

            ArgsEmitter(CheckF check,
                        NextF next,
                        BeforeF before,
                        AfterF after)
                    : m_check(std::move(check)), m_next(std::move(next)), m_before(std::move(before)),
                      m_after(std::move(after)) {}

            void operator()(Args ...args) const {
                if (!m_check()) return;
                StateCode state = STATE_STILL;
                m_before();
                After _after([this, &state]() {
                    m_after(state);
                });
                state = m_next(pass_value<Args>(args)...);
            }

        private:
            mutable CheckF m_check;
            mutable NextF m_next;
            mutable BeforeF m_before;
            mutable AfterF m_after;
        };

        /**
         * The event should emit in single thread in same time, or use mute to prevent that.
         *
         */
        class StateModel {
        public:
            using Transition = std::function<StateCode()>;
            using VoidF = std::function<void()>;

            StateModel()
                    : m_state(STATE_START) {
            }

            StateModel(const StateModel &) = delete;

            StateModel(StateModel &&other)
                    : m_state(other.m_state.load()), m_transition(std::move(other.m_transition)),
                      m_enter(std::move(other.m_enter)) {
            }

            ~StateModel() {
                if (state() != STATE_END) {
                    after(STATE_END);
                }
            };

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
                StateCode state = STATE_STILL;
                this->before();
                After _after([this, &state]() {
                    this->after(state);
                });
                auto key = std::make_tuple(m_state.load(), code);
                auto it = m_transition.find(key);
                if (it == m_transition.end()) return;
                state = it->second();
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
             * @return the event emitter, equal to event(event)
             * @note The `next` should return STATE_STILL if state has not changed.
             * @note If `next` return same state to now state, the state's enter and leaving function will also be modified.
             */
            std::function<void()> transit(StateCode state, EventCode event, StateCode next) {
                return transit(state, event, [next]() { return next; });
            }

            /**
             * Set the transition function when emit event in `state`.
             * After transition function, state will change to the return value of next.
             * @param state the current state when event emitted
             * @param event the emitted event
             * @param next the state will change to the return value of next
             * @param action transition function
             * @return the event emitter, equal to event(event)
             * @note The `next` should return STATE_STILL if state has not changed.
             * @note If `next` return same state to now state, the state's enter and leaving function will also be modified.
             */
            std::function<void()> transit(StateCode state, EventCode event, StateCode next,
                                          const VoidF &action) {
                return transit(state, event, [action, next]() {
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
             * @return the event emitter, equal to event(event)
             * @note The `next` should return STATE_STILL if state has not changed.
             * @note If `next` return same state to now state, the state's enter and leaving function will also be modified.
             */
            std::function<void()> transit(StateCode state, EventCode event, const Transition &next) {
                // state should not be STATE_STILL or STATE_STOP
                auto key = std::make_tuple(state, event);
                m_transition[key] = next;
                return [this, event]() { this->event(event); };
            }

            /**
             * Set the transition function when emit event in `state`.
             * After transition function, state will change to the return value of next.
             * @tparam T transition function type, must be StateCode(...)
             * @param state the current state when event emitted
             * @param next the transition function, the state will change to the return value of next
             * @return the event emitter, the parameters should parse to function `next`
             * @note there is no event code set, because the emitter should parse parameters
             */
            template<typename T, typename = typename std::enable_if<
                    std::is_function<T>::value &&
                    std::is_same<typename return_type<T>::type, StateCode>::value>::type>
            std::function<typename void_function<T>::type>
            transit(StateCode state,
                    const std::function<T> &next) {
                // state should not be STATE_STILL or STATE_STOP
                using ThisEmitter = ArgsEmitter<T>;
                return ThisEmitter(
                        [this, state] {
                            // return true for emit, false for ignoring this emit
                            return m_state == state;
                        },
                        next,
                        [this]() {
                            this->before();
                        },
                        [this](StateCode state) {
                            this->after(state);
                        }
                );
            }

        private:
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
            }

            std::atomic<StateCode> m_state;
            std::map<std::tuple<StateCode, EventCode>, Transition> m_transition;
            std::map<StateCode, VoidF> m_enter;
            std::map<StateCode, VoidF> m_leave;
        };
    }

    using StateModel = _::StateModel;
}

#endif //OMEGA_STATE_MODEL_H
