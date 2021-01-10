//
// Created by kier on 2020/8/19.
//

#ifndef OMEGA_VAR_VAR_H
#define OMEGA_VAR_VAR_H

#include "notation.h"
#include "type.h"
#include "scalar.h"
#include "boolean.h"
#include "null.h"
#include "./string.h"
#include "array.h"
#include "object.h"
#include "binary.h"
#include "vector.h"

#include "notation.h"
#include "repr.h"
#include "cast.h"

#include "varexception.h"

#include <type_traits>
#include <sstream>
#include <functional>

namespace ohm {
    template<typename T, typename=void>
    struct is_var_assignable : public std::false_type {
    };

    template<typename T, typename=void>
    struct is_var_convertible : public std::false_type {
    };

    template<typename T, typename=void>
    struct is_var_element : public std::false_type {
    };

    template<typename T>
    struct is_var_element<T, typename std::enable_if<
            notation::is_notation_type<typename std::decay<T>::type>::value &&
            std::is_constructible<typename notation::type_type<
            typename std::decay<T>::type>::type, T>::value>::type> : public std::true_type {
    };

    template <typename T>
    inline constexpr bool _do_var_assignable();

    template <typename T>
    inline constexpr bool _do_var_convertible();

    template<typename T, typename K>
    T *__at(K &t) { return reinterpret_cast<T *>(&t); }

    template<typename T, typename K>
    const T *__at(const K &t) { return reinterpret_cast<const T *>(&t); }

    template<typename T, typename K>
    T &__ref(K &t) { return *reinterpret_cast<T *>(&t); }

    template<typename T, typename K>
    const T &__ref(const K &t) { return *reinterpret_cast<const T *>(&t); }

    template<typename T, typename K>
    T *__at(K *t) { return reinterpret_cast<T *>(t); }

    template<typename T, typename K>
    const T *__at(const K *t) { return reinterpret_cast<const T *>(t); }

    template<typename T, typename K>
    T &__ref(K *t) { return *reinterpret_cast<T *>(t); }

    template<typename T, typename K>
    const T &__ref(const K *t) { return *reinterpret_cast<const T *>(t); }

#pragma push_macro("CHECK")
#pragma push_macro("SWITCH")
#pragma push_macro("CASE")
#pragma push_macro("ELSE")
#pragma push_macro("END")
#pragma push_macro("DEFAULT")
#pragma push_macro("UNEXPECTED_END")

#pragma push_macro("SWITCH_TYPE")
#pragma push_macro("CASE_TYPE")
#pragma push_macro("DEFAULT_TYPE")
#pragma push_macro("END_TYPE")
#pragma push_macro("CASE_TYPE_ANY")

#pragma push_macro("ID_SWITCH")
#pragma push_macro("ID_DEFAULT")
#pragma push_macro("ID_CASE")
#pragma push_macro("ID_END")

/**
 * If no CASE matched, it will throw VarNotSupportedException
 * @param op string tell check operator name
 */
#define CHECK(op) \
    std::string __op = op; \
    bool __checked = false; \
    std::vector<notation::DataType> __checked_type;

/**
 * Use after CHECK, following CASE ELSE END UNEXPECTED_END DEFAULT
 */
#define SWITCH \
    {

/**
 * The codes following case, will run if main_type matched
 * The CASE must follow SWITCH, with ELSE, END, UNEXPECTED_END, DEFAULT following
 */
#define CASE(main_type) \
    } \
    __checked_type.push_back(main_type); \
    if (!__checked && m_var && (m_var->type & 0xFF00) == (main_type & 0xFF00)) { \
        __checked = true; \
        auto &element = *static_cast<typename notation::code_type<main_type>::type*>(m_var.get()); \
        auto &content = __at<typename notation::code_type<main_type>::type>(m_var.get())->content; \
        (void)(content);

/**
 * Use for Has or not return function, achive if no case matched.
 * After this, END and DEFAULT won't check if case matched.
 * Only one or no ELSE can be set.
 */
#define ELSE \
    } \
    if (!__checked) { \
        __checked = true;

/**
 * following CASE or ELSE, active if no CASE or ELSE matched.
 * This line will throw VarNotSupportedException if no CASE or ELSE matched.
 */
#define END \
    } \
    if (!__checked) { \
        throw VarOperatorNotSupported(m_var ? m_var->type : notation::type::Undefined, \
            __op, __checked_type); \
    }
/**
 * throw VarNotSupportedException if got this line. it means no return above.
 */
#define UNEXPECTED_END \
    } \
    throw VarOperatorNotSupported(m_var ? m_var->type : notation::type::Undefined, \
        __op, __checked_type);

/**
 * return expr if case matched but on value return.
 * throw VarNotSupportedException no case matched.
 */
#define DEFAULT(expr) \
    } \
    if (!__checked) { \
        throw VarOperatorNotSupported(m_var ? m_var->code : notation::type::Undefined, \
            __op, __checked_type); \
    } else { \
        return expr; \
    }

#define SWITCH_TYPE(_type_code) \
    switch ((_type_code) & 0xFF) { \
    case 0xFF: {

#define CASE_TYPE(sub_type) \
    } \
    case sub_type: { \
        using type = notation::code_sub_type<sub_type>::type;

#define DEFAULT_TYPE(codes) \
    } \
    default : { \

#define END_TYPE \
    } \
    }

#define CASE_TYPE_ANY(codes) \
    CASE_TYPE(notation::type::INT8) codes; \
    CASE_TYPE(notation::type::UINT8) codes; \
    CASE_TYPE(notation::type::INT16) codes; \
    CASE_TYPE(notation::type::UINT16) codes; \
    CASE_TYPE(notation::type::INT32) codes; \
    CASE_TYPE(notation::type::UINT32) codes; \
    CASE_TYPE(notation::type::INT64) codes; \
    CASE_TYPE(notation::type::UINT64) codes; \
    CASE_TYPE(notation::type::FLOAT32) codes; \
    CASE_TYPE(notation::type::FLOAT64) codes; \
    CASE_TYPE(notation::type::BOOLEAN) codes; \
    CASE_TYPE(notation::type::FLOAT16) codes; \
    CASE_TYPE(notation::type::UNKNOWN8) codes; \
    CASE_TYPE(notation::type::UNKNOWN16) codes; \
    CASE_TYPE(notation::type::UNKNOWN32) codes; \
    CASE_TYPE(notation::type::UNKNOWN64) codes; \
    CASE_TYPE(notation::type::UNKNOWN128) codes; \
    CASE_TYPE(notation::type::COMPLEX32) codes; \
    CASE_TYPE(notation::type::COMPLEX64) codes; \
    CASE_TYPE(notation::type::COMPLEX128) codes; \
    CASE_TYPE(notation::type::CHAR8) codes; \
    CASE_TYPE(notation::type::CHAR16) codes; \
    CASE_TYPE(notation::type::CHAR32) codes; \
    CASE_TYPE(notation::type::VOID) codes; \
    CASE_TYPE(notation::type::PTR) codes;

#define ID_SWITCH(type) \
    switch (type & 0xFF00) { \
    case 0xFF00: \
    {

#define ID_DEFAULT \
    } \
    default: \
    {

#define ID_CASE(main_type) \
    } \
    case main_type: \
    { \
        auto &element = *static_cast<typename notation::code_type<main_type>::type*>(m_var.get()); \
        auto &content = __at<typename notation::code_type<main_type>::type>(m_var.get())->content;


#define ID_END \
    } \
    }

    class Var {
    public:
        using self = Var;

        Var() = default;

        // Var(notation::DataType code) : self(code2object(code)) {}

        template <typename T, typename=typename std::enable_if<
                _do_var_assignable<T>()>::type>
        Var(T &&t);

        explicit Var(void *ptr) {
            this->operator=(notation::ElementScalar(ptr));
        }

        Var &operator=(const Var &var) {
            m_var = var.m_var;
            if (m_notifier) {
                m_notifier(m_var);
            }
            return *this;
        }

        Var &operator=(Var &&var) {
            this->m_var = std::move(var.m_var);
            this->m_notifier = std::move(var.m_notifier);
            return *this;
        }

        Var &operator=(const decltype(notation::Undefined) &) {
            m_var.reset();
            if (m_notifier) {
                m_notifier(m_var);
            }
            return *this;
        }

        template<typename T>
        typename std::enable_if<
                is_var_element<T>::value && !std::is_pointer<T>::value
                , Var>::type &
        operator=(T &&t) {
            using Element = typename notation::type_type<typename std::decay<T>::type>::type;
            // TODO: check if there is need to update new
            m_var = std::shared_ptr<Element>(new Element(std::forward<T>(t)));
            if (m_notifier) {
                m_notifier(m_var);
            }
            return *this;
        }

        template<typename T>
        typename std::enable_if<
                !is_var_element<T>::value &&
                std::is_base_of<notation::Element, typename std::decay<T>::type>::value
                , Var>::type &
        operator=(T &&t) {
            using Element = typename std::decay<T>::type;
            m_var = std::make_shared<Element>(std::forward<T>(t));
            if (m_notifier) {
                m_notifier(m_var);
            }
            return *this;
        }

        template <typename T>
        typename std::enable_if<
                !is_var_element<T>::value &&
                !std::is_same<Var, typename std::decay<T>::type>::value &&
                std::is_integral<T>::value, Var>::type &
        operator=(T i) {
            return this->operator=(typename notation::other_int<T>::type(i));
        }

        template <typename T>
        typename std::enable_if<
                !is_var_element<T>::value &&
                !std::is_same<Var, typename std::decay<T>::type>::value &&
                std::is_constructible<std::string, T>::value
                , Var>::type &
        operator=(T &&t) {
            return this->operator=(std::string(std::forward<T>(t)));
        }

        Var(const Var &var) : self(var.m_var) {}

        operator notation::Element::shared() { return m_var; }

        operator notation::Element::shared() const { return m_var; }

        Var operator[](const std::string &key) {
            if (!m_var) {
                m_var = notation::code_type<notation::type::Object>::type::Make();
            } else if (!m_var->is_object()) {
                throw VarNotSupportSlice(m_var->type);
            }
            auto &data = reinterpret_cast<notation::ElementObject *>(m_var.get())->content;
            auto it = data.find(key);
            if (it == data.end()) {
                notation::Element::weak storage = m_var;
                auto notifier = [=](const notation::Element::shared &var) {
                    if (!var) return;
                    auto shared_storage = storage.lock();
                    if (!shared_storage) return;
                    auto obj = reinterpret_cast<notation::ElementObject *>(shared_storage.get());
                    obj->content[key] = std::move(var);
                };
                return Var(notifier);
            } else {
                notation::Element::weak stroage = m_var;
                auto &value = it->second;
                auto notifier = [stroage, it, &data, &value](const notation::Element::shared &var) {
                    if (!var) {
                        data.erase(it);
                        return;
                    }
                    auto shared_storage = stroage.lock();
                    if (!shared_storage) return;
                    auto obj = reinterpret_cast<notation::ElementObject *>(shared_storage.get());
                    value = std::move(var);
                };
                return Var(value, notifier);
            }
        }

        Var operator[](const std::string &key) const {
            if (!m_var) {
                throw VarNotSupportSlice(notation::type::Undefined, key);
            } else if (!m_var->is_object()) {
                throw VarNotSupportSlice(m_var->type);
            }
            auto &data = reinterpret_cast<notation::ElementObject *>(m_var.get())->content;
            auto it = data.find(key);
            if (it == data.end()) {
                return Var();
            } else {
                notation::Element::weak storage = m_var;
                auto &value = it->second;
                return Var(value);
            }
        }
        template<typename T, typename=typename std::enable_if<
                std::is_constructible<std::string, T>::value &&
                !std::is_same<std::string, typename std::decay<T>::type>::value>::type>
        Var operator[](T &&t) {
            return this->operator[](std::string(std::forward<T>(t)));
        }

        template<typename T, typename=typename std::enable_if<
                std::is_constructible<std::string, T>::value &&
                !std::is_same<std::string, typename std::decay<T>::type>::value>::type>
        Var operator[](T &&t) const {
            return this->operator[](std::string(std::forward<T>(t)));
        }

        Var operator[](const int64_t &index) {
            if (!m_var) {
                throw VarNotSupportSlice(notation::type::Undefined, index);
            } else if (!m_var->is_array()) {
                throw VarNotSupportSlice(m_var->type, index);
            }
            auto &data = reinterpret_cast<notation::ElementArray *>(m_var.get())->content;
            auto data_size = int64_t(data.size());
            auto fixed_index = index >= 0 ? index : index + data_size;
            if (fixed_index < 0 || fixed_index >= data_size) {
                throw VarIndexOutOfRange(m_var->type, fixed_index, data.size());
            }
            {
                notation::Element::weak storage = m_var;
                auto &value = data.at(size_t(fixed_index));
                auto notifier = [=, &value](notation::Element::shared var) {
                    auto shared_storage = storage.lock();
                    if (!shared_storage) return;
                    auto obj = reinterpret_cast<notation::ElementObject *>(shared_storage.get());
                    value = std::move(var);
                };
                return Var(value, notifier);
            }
            return Var();
        }

        Var operator[](const int64_t &index) const {
            if (!m_var) {
                throw VarNotSupportSlice(notation::type::Undefined, index);
            } else if (!m_var->is_array()) {
                throw VarNotSupportSlice(m_var->type, index);
            }
            auto &data = reinterpret_cast<notation::ElementArray *>(m_var.get())->content;
            auto data_size = int64_t(data.size());
            auto fixed_index = index >= 0 ? index : index + data_size;
            if (fixed_index < 0 || fixed_index >= data_size) {
                throw VarIndexOutOfRange(m_var->type, fixed_index, data.size());
            }
            auto &value = data.at(size_t(fixed_index));
            return Var(value);
        }

        template<typename T, typename=typename std::enable_if<
                std::is_integral<T>::value &&
                !std::is_same<int64_t, T>::value>::type>
        Var operator[](T &t) {
            return this->operator[](int64_t(t));
        }

        template<typename T, typename=typename std::enable_if<
                std::is_integral<T>::value &&
                !std::is_same<int64_t, T>::value>::type>
        Var operator[](T &t) const {
            return this->operator[](int64_t(t));
        }

        notation::DataType type() const {
            return m_var ? m_var->type : notation::type::Undefined;
        }

        template<typename T, typename=typename std::enable_if<
                std::is_same<T, bool>::value>::type>
        bool cpp() const {
            if (!m_var) return false;

            CHECK("bool()")
            SWITCH
            CASE(notation::type::None)
                return false;
            CASE(notation::type::Boolean)
                return content;
            CASE(notation::type::String)
                return !content.empty();
            CASE(notation::type::Array)
                return content.size();
            CASE(notation::type::Object)
                return content.size();
            CASE(notation::type::Scalar)
                SWITCH_TYPE(m_var->type)
                    CASE_TYPE_ANY(
                            try {
                                return (notation::cast2<bool, type>().eval(element.ref<type>()));
                            } catch (const notation::CastException &) {
                                throw VarOperatorNotSupported(this->type(), __op);
                            })
                END_TYPE
            CASE(notation::type::Binary)
                return content.size();
            CASE(notation::type::Vector)
                return element.size();
            UNEXPECTED_END
        }

        template<typename T, typename=typename std::enable_if<
                (std::is_integral<T>::value || std::is_floating_point<T>::value) &&
                !std::is_same<T, bool>::value>::type>
        T cpp() const {
            CHECK(std::string(typeid(T).name()) + "()")
            SWITCH
            CASE(notation::type::Boolean)
                return content ? T(1) : T(0);
            CASE(notation::type::String)
                try {
                    return notation::string2<T>(content);
                } catch (const notation::String2Exception &e) {
                    throw VarException(e.what());
                }
            CASE(notation::type::Scalar)
                SWITCH_TYPE(m_var->type)
                    CASE_TYPE_ANY(
                            try {
                                return (notation::cast2<T, type>().eval(element.ref<type>()));
                            } catch (const notation::CastException &) {
                                throw VarOperatorNotSupported(this->type(), __op);
                            })
                END_TYPE
            UNEXPECTED_END
        }

        template<typename T, typename=typename std::enable_if<
                notation::is_vector<T>::value>::type>
        notation::Vector<typename notation::is_vector<T>::value_type>
                cpp() const {
            using value_type = typename notation::is_vector<T>::value_type;
            CHECK("vector<" + std::string(typeid(T).name()) + ">()")
            SWITCH
            CASE(notation::type::Vector)
                auto wanted_type = notation::type::Vector | notation::type_code<T>::code;
                if (m_var->type != wanted_type)
                    throw VarOperatorNotSupported(type(), __op, { wanted_type });
                return notation::Vector<value_type>(content);
            UNEXPECTED_END
        }

        template<typename T, typename=typename std::enable_if<
                std::is_same<T, std::string>::value>::type>
        std::string cpp() const {
            return this->str();
        }

        template<typename T, typename=typename std::enable_if<
                std::is_same<T, notation::Binary>::value>::type>
        notation::Binary cpp() const {
            CHECK("binary()")
            SWITCH
            CASE(notation::type::Binary)
                return content;
            UNEXPECTED_END
        }

        void append(Var obj) {
            CHECK("append")
            SWITCH
            CASE(notation::type::Array)
                content.push_back(obj);
            END
        }

        void extend(Var var) {
            CHECK("extend")
            SWITCH
            CASE(notation::type::Array)
                if (var.type() != notation::type::Array) {
                    throw VarOperatorParameterMismatch(this->type(), __op, 0, {notation::type::Array});
                }
                auto &arr = reinterpret_cast<notation::ElementArray *>(m_var.get())->content;
                content.insert(content.end(), arr.begin(), arr.end());
            CASE(notation::type::Object)
                if (var.type() != notation::type::Object) {
                    throw VarOperatorParameterMismatch(this->type(), __op, 0, {notation::type::Object});
                }
                auto &obj = reinterpret_cast<notation::ElementObject *>(m_var.get())->content;
                for (auto &pair : obj) {
                    content[pair.first] = pair.second;
                }
            END
        }

        void resize(size_t size) {
            CHECK("resize")
            SWITCH
            CASE(notation::type::Array)
                content.resize(size);
            CASE(notation::type::Binary)
                content.resize(size);
            CASE(notation::type::Vector)
                element.resize(size);
            END
        }

        size_t size() const {
            CHECK("size")
            SWITCH
            CASE(notation::type::Array)
                return content.size();
            CASE(notation::type::Object)
                return content.size();
            CASE(notation::type::Binary)
                return content.size();
            CASE(notation::type::Vector)
                return element.size();
            UNEXPECTED_END
        }

        bool has(const std::string &key) const {
            CHECK("size")
            SWITCH
            CASE(notation::type::Object)
                return content.find(key) != content.end();
            UNEXPECTED_END
        }

        std::vector<std::string> keys() const {
            CHECK("keys")
            SWITCH
            CASE(notation::type::Object)
                std::vector<std::string> result;
                result.reserve(content.size());
                for (auto &pair : content) {
                    if (pair.second == nullptr) continue;
                    result.emplace_back(pair.first);
                }
                return result;
            UNEXPECTED_END
        }

        template<typename T, typename=typename std::enable_if<_do_var_convertible<T>()>::type>
        operator T() const {
            return cpp<T>();
        }

        /**
         * got cpp memory data, only scalar, vector, and binary is meaningful
         * @param ptr
         * @param size
         */
        void unsafe(void **ptr, size_t *size) const {
#define _UNSAFE_RETURN(a, b) \
            { *ptr = (a); *size = (b); return; }
            if (!m_var) {
                _UNSAFE_RETURN(nullptr, 0)
            }
            ID_SWITCH(m_var->type)
                ID_DEFAULT
                    _UNSAFE_RETURN(nullptr, 0)
                ID_CASE(notation::type::None)
                    _UNSAFE_RETURN(nullptr, 0)
                ID_CASE(notation::type::Boolean)
                    _UNSAFE_RETURN(&content, notation::element_size(content))
                ID_CASE(notation::type::String)
                    _UNSAFE_RETURN((void*)(content.data()), content.size())
                ID_CASE(notation::type::Array)
                    _UNSAFE_RETURN(&content, notation::element_size(content))
                ID_CASE(notation::type::Object)
                    _UNSAFE_RETURN(&content, notation::element_size(content))
                ID_CASE(notation::type::Binary)
                    _UNSAFE_RETURN(content.data(), content.size())
                ID_CASE(notation::type::Scalar)
                    _UNSAFE_RETURN(element.at<char>(), notation::sub_type_size(notation::type::SubType(element.type & 0xFF)))
                ID_CASE(notation::type::Vector)
                    _UNSAFE_RETURN(content.data(), content.capacity())
            ID_END
            _UNSAFE_RETURN(nullptr, 0)
#undef _UNSAFE_RETURN
        }

        void *id() {
            if (!m_var) return nullptr;
            ID_SWITCH(m_var->type)
                ID_DEFAULT
                    return nullptr;
                ID_CASE(notation::type::None)
                    return nullptr;
                ID_CASE(notation::type::Boolean)
                    return &content;
                ID_CASE(notation::type::String)
                    return &content;
                ID_CASE(notation::type::Array)
                    return &content;
                ID_CASE(notation::type::Object)
                    return &content;
                ID_CASE(notation::type::Binary)
                    return &content;
                ID_CASE(notation::type::Scalar)
                    SWITCH_TYPE(m_var->type)
                        CASE_TYPE_ANY(return element.at<type>());
                    END_TYPE
                ID_CASE(notation::type::Vector)
                    return &content;
            ID_END
            return nullptr;
        }

        const void *id() const {
            return const_cast<self *>(this)->id();
        }

        std::string repr() const {
            if (!m_var) return "\"@undefined\"";
            ID_SWITCH(m_var->type)
                ID_DEFAULT
                    return "\"@" + notation::type_string(type()) + "\"";
                ID_CASE(notation::type::None)
                    return "null";
                ID_CASE(notation::type::Boolean)
                    return content ? "true" : "false";
                ID_CASE(notation::type::String)
                    return notation::repr(content);
                ID_CASE(notation::type::Array)
                    return notation::repr(content);
                ID_CASE(notation::type::Object)
                    return notation::repr(content);
                ID_CASE(notation::type::Scalar)
                    SWITCH_TYPE(m_var->type)
                        CASE_TYPE_ANY(return notation::scalar_repr(element.ref<type>()))
                    END_TYPE
                ID_CASE(notation::type::Binary)
                    return notation::repr(content);
                ID_CASE(notation::type::Vector)
                    SWITCH_TYPE(m_var->type)
                        CASE_TYPE_ANY(return notation::repr(element.data<type>(), element.size()))
                    END_TYPE
            ID_END
            return nullptr;
        }

        std::string str() const {
            if (m_var && (m_var->type & 0xFF00) == notation::type::String) {
                auto &data = __at<typename notation::code_type<notation::type::String>::type>(m_var.get())->content;
                return data;
            }
            return repr();
        }

        static Var From(notation::Element::shared element) {
            return Var(std::move(element));
        }

        notation::Element::shared _element() const {
            return m_var;
        }

    private:
        using Notifier = std::function<void(notation::Element::shared)>;

        explicit Var(notation::Element::shared var)
                : m_var(std::move(var)) {}

        explicit Var(notation::Element::shared var, Notifier notifier)
                : m_var(std::move(var)), m_notifier(std::move(notifier)) {}

        explicit Var(Notifier notifier)
                : m_notifier(std::move(notifier)) {}

        notation::Element::shared m_var;
        std::function<void(const notation::Element::shared &)> m_notifier;

        friend std::string notation::repr(notation::Element::shared element);
    };

    template<typename T>
    struct is_var_convertible<T,
            typename std::enable_if<
                    std::is_same<T, decltype(std::declval<Var>().cpp<T>())>::value
            >::type> : public std::true_type {
    };

    template<typename T>
    struct is_var_assignable<T,
            typename std::enable_if<
                    std::is_same<Var&, decltype(std::declval<Var>().operator=(std::declval<T>()))>::value ||
                    std::is_same<const Var&, decltype(std::declval<Var>().operator=(std::declval<T>()))>::value
            >::type> : public std::true_type {
    };
    template <typename T>
    inline constexpr bool _do_var_assignable() {
        return is_var_assignable<T>::value;
    }

    template <typename T>
    inline constexpr bool _do_var_convertible() {
        return is_var_convertible<T>::value;
    }

    template<typename T, typename>
    Var::Var(T &&t) {
        this->operator=(std::forward<T>(t));
    }

    namespace notation {
        inline std::string repr(Element::shared element) {
            return Var(element).repr();
        }
    }

    inline std::ostream &operator<<(std::ostream &out, const Var &var) {
        return out << var.str();
    }

    template <typename T, typename=typename std::enable_if<is_var_element<T>::value>::type>
    inline Var var_cast(const Var &var) {
        throw VarException("Cast only support scalar, vector, and array.");
    }


}

#pragma pop_macro("CHECK")
#pragma pop_macro("SWITCH")
#pragma pop_macro("CASE")
#pragma pop_macro("ELSE")
#pragma pop_macro("END")
#pragma pop_macro("DEFAULT")
#pragma pop_macro("UNEXPECTED_END")

#pragma pop_macro("SWITCH_TYPE")
#pragma pop_macro("CASE_TYPE")
#pragma pop_macro("DEFAULT_TYPE")
#pragma pop_macro("END_TYPE")
#pragma pop_macro("CASE_TYPE_ANY")

#pragma pop_macro("ID_SWITCH")
#pragma pop_macro("ID_DEFAULT")
#pragma pop_macro("ID_CASE")
#pragma pop_macro("ID_END")

#endif //OMEGA_VAR_VAR_H
