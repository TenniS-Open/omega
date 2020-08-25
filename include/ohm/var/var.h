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

#include "notation.h"
#include "repr.h"
#include "cast.h"

#include <type_traits>
#include <sstream>

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
            notation::is_notation_type<typename notation::remove_cr<T>::type>::value &&
            std::is_constructible<typename notation::type_type<
            typename notation::remove_cr<T>::type>::type, T>::value>::type> : public std::true_type {
    };

//    template <typename T>
//    struct is_var_element : public std::integral_constant<bool,
//            notation::is_notation_type<typename remove_cr<T>::type>::value &&
//            std::is_constructible<typename notation::type_type<
//                    typename remove_cr<T>::type>::type, T>::value> {};

//    inline constexpr bool is_var_code_signed()
//
//    inline constexpr bool is_var_code_convertible(notation::DataType from, notation::DataType to) {
//        if ((from & 0xFF00) == notation::type::Scalar && (to & 0xFF00) == notation::type::Scalar) {
//
//        }
//        return false;
//    }

    inline notation::Element::shared code2object(notation::DataType code) {
        using namespace notation;
        switch (code & 0xFF00) {
            default:
                return nullptr;
            case type::None:
                return code_type<type::None>::type::Make();
            case type::Boolean:
                return code_type<type::Boolean>::type::Make();
            case type::String:
                return code_type<type::String>::type::Make();
            case type::Array:
                return code_type<type::Array>::type::Make();
            case type::Object:
                return code_type<type::Object>::type::Make();
            case type::Binary:
                return code_type<type::String>::type::Make();
            case type::Undefined:
                return code_type<type::String>::type::Make();

//            case type::Scalar: return code_type<type::Scalar>::type::Make();
//            case type::Repeat: return code_type<type::Repeat>::type::Make();
        }
    }

    class VarException : public std::logic_error {
    public:
        using self = VarException;
        using supper = std::logic_error;

        explicit VarException(const std::string &msg) : std::logic_error(msg) {}
    };

    class VarNotSupportSlice : public VarException {
    public:
        using self = VarNotSupportSlice;
        using supper = VarException;

        explicit VarNotSupportSlice(notation::DataType type)
                : supper(Message(type)) {}

        explicit VarNotSupportSlice(notation::DataType type, int64_t index)
                : supper(Message(type, index)) {}

        explicit VarNotSupportSlice(notation::DataType type, const std::string &key)
                : supper(Message(type, key)) {}

        static std::string Message(notation::DataType type) {
            std::ostringstream oss;
            oss << notation::type_string(type) << "[] does not supported.";
            return oss.str();
        }

        static std::string Message(notation::DataType type, int64_t index) {
            std::ostringstream oss;
            oss << notation::type_string(type) << "[" << index << "] does not supported.";
            return oss.str();
        }

        static std::string Message(notation::DataType type, const std::string &key) {
            std::ostringstream oss;
            oss << notation::type_string(type) << "[\"" << key << "\"] does not supported.";
            return oss.str();
        }
    };

    class VarAttributeNotFound : public VarException {
    public:
        using self = VarAttributeNotFound;
        using supper = VarException;

        explicit VarAttributeNotFound(notation::DataType type, const std::string &key)
                : supper(Message(type, key)) {}

        static std::string Message(notation::DataType type, const std::string &key) {
            std::ostringstream oss;
            oss << notation::type_string(type) << " attribute \"" << key << "\" not found.";
            return oss.str();
        }
    };

    class VarIndexOutOfRange : public VarException {
    public:
        using self = VarAttributeNotFound;
        using supper = VarException;

        explicit VarIndexOutOfRange(notation::DataType type, int64_t index)
                : supper(Message(type, index)) {}

        explicit VarIndexOutOfRange(notation::DataType type, int64_t index, size_t size)
                : supper(Message(type, index, size)) {}

        static std::string Message(notation::DataType type, int64_t index) {
            std::ostringstream oss;
            oss << notation::type_string(type) << " index " << index << " out of range.";
            return oss.str();
        }

        static std::string Message(notation::DataType type, int64_t index, size_t size) {
            std::ostringstream oss;
            if (size) {
                oss << notation::type_string(type) << " index " << index << " out of range."
                    << " Must in [-" << size << ", " << size - 1 << "].";
            } else {
                oss << notation::type_string(type) << " index " << index << " out of range."
                    << " The " << notation::type_string(type) << " is empty.";
            }
            return oss.str();
        }
    };

    class VarOperatorNotSupported : public VarException {
    public:
        using self = VarOperatorNotSupported;
        using supper = VarException;

        explicit VarOperatorNotSupported(notation::DataType type,
                                         const std::string &op)
                : supper(Message(type, op)) {}

        explicit VarOperatorNotSupported(notation::DataType type,
                                         const std::string &op,
                                         const std::vector<notation::DataType> &supported)
                : supper(Message(type, op, supported)) {}

        static std::string Message(notation::DataType type,
                                   const std::string &op,
                                   const std::vector<notation::DataType> &supported) {
            std::ostringstream oss;
            oss << notation::type_string(type) << " operator " << op << " not supported, expecting: ";
            for (size_t i = 0; i < supported.size(); ++i) {
                if (i) oss << ", ";
                oss << notation::main_type_string(supported[i]);
            }
            oss << ".";
            return oss.str();
        }

        static std::string Message(notation::DataType type,
                                   const std::string &op) {
            std::ostringstream oss;
            oss << notation::type_string(type) << " operator " << op << " not supported.";
            return oss.str();
        }
    };

    class VarOperatorParameterMismatch : public VarException {
    public:
        using self = VarOperatorParameterMismatch;
        using supper = VarException;

        explicit VarOperatorParameterMismatch(notation::DataType type,
                                              const std::string &op,
                                              int param)
                : supper(Message(type, op, param)) {}

        explicit VarOperatorParameterMismatch(notation::DataType type,
                                              const std::string &op,
                                              int param,
                                              const std::vector<notation::DataType> &supported)
                : supper(Message(type, op, param, supported)) {}

        static std::string Message(notation::DataType type,
                                   const std::string &op,
                                   int param,
                                   const std::vector<notation::DataType> &supported) {
            std::ostringstream oss;
            oss << notation::type_string(type) << " operator " << op
                << " parameter " << param << " mismatched, expecting: ";
            for (size_t i = 0; i < supported.size(); ++i) {
                if (i) oss << ", ";
                oss << notation::main_type_string(supported[i]);
            }
            oss << ".";
            return oss.str();
        }

        static std::string Message(notation::DataType type,
                                   const std::string &op,
                                   int param) {
            std::ostringstream oss;
            oss << notation::type_string(type) << " operator " << op
                << " parameter " << param << " mismatched.";
            return oss.str();
        }
    };

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

#pragma push_macro("CASE_NO_TYPE")
#pragma push_macro("CASE_TYPE_INTEGER")
#pragma push_macro("CASE_TYPE_FLOOT")
#pragma push_macro("CASE_TYPE_BOOL")
#pragma push_macro("CASE_TYPE_NOT_SUPPORTED")
#pragma push_macro("CASE_TYPE_CHAR")
#pragma push_macro("CASE_TYPE_VOID")
#pragma push_macro("CASE_TYPE_POINTER")
#pragma push_macro("CASE_TYPE_ANY")

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
    if (!__checked && m_var && (m_var->code & 0xFF00) == (main_type & 0xFF00)) { \
        __checked = true; \
        auto &data = __at<typename notation::code_type<main_type>::type>(m_var.get())->data; \
        (void)(data);

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
        throw VarOperatorNotSupported(m_var ? m_var->code : notation::type::Undefined, \
            __op, __checked_type); \
    }
/**
 * throw VarNotSupportedException if got this line. it means no return above.
 */
#define UNEXPECTED_END \
    } \
    throw VarOperatorNotSupported(m_var ? m_var->code : notation::type::Undefined, \
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
        using type = notation::code_type<sub_type>::type; \
        auto &scalar = __at<notation::code_type<notation::type::Scalar | sub_type>::type>(m_var.get())->data; \
        (void)(scalar);

#define DEFAULT_TYPE(codes) \
    } \
    default : { \

#define END_TYPE \
    } \
    }

#define CASE_TYPE_INTEGER(codes) \
    CASE_TYPE(notation::type::INT8) codes; \
    CASE_TYPE(notation::type::UINT8) codes; \
    CASE_TYPE(notation::type::INT16) codes; \
    CASE_TYPE(notation::type::UINT16) codes; \
    CASE_TYPE(notation::type::INT32) codes; \
    CASE_TYPE(notation::type::UINT32) codes; \
    CASE_TYPE(notation::type::INT64) codes; \
    CASE_TYPE(notation::type::UINT64) codes;

#define CASE_TYPE_FLOOT(codes) \
    CASE_TYPE(notation::type::FLOAT32) codes; \
    CASE_TYPE(notation::type::FLOAT64) codes;

#define CASE_TYPE_BOOL(codes) \
    CASE_TYPE(notation::type::BOOLEAN) codes;

#define CASE_TYPE_NOT_SUPPORTED(codes) \
    CASE_TYPE(notation::type::FLOAT16) codes; \
    CASE_TYPE(notation::type::UNKNOWN8) codes; \
    CASE_TYPE(notation::type::UNKNOWN16) codes; \
    CASE_TYPE(notation::type::UNKNOWN32) codes; \
    CASE_TYPE(notation::type::UNKNOWN64) codes; \
    CASE_TYPE(notation::type::UNKNOWN128) codes; \
    CASE_TYPE(notation::type::COMPLEX32) codes; \
    CASE_TYPE(notation::type::COMPLEX64) codes; \
    CASE_TYPE(notation::type::COMPLEX128) codes;

#define CASE_TYPE_CHAR(codes) \
    CASE_TYPE(notation::type::CHAR8) codes; \
    CASE_TYPE(notation::type::CHAR16) codes; \
    CASE_TYPE(notation::type::CHAR32) codes;

#define CASE_TYPE_VOID(codes) \
    CASE_TYPE(notation::type::VOID) codes;

#define CASE_TYPE_POINTER(codes) \
    CASE_TYPE(notation::type::PTR) codes;

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

#define CASE_TYPE_NONVOID(codes) \
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
    CASE_TYPE(notation::type::PTR) codes;

    class Var {
    public:
        using self = Var;

        Var() = default;

        // Var(notation::DataType code) : self(code2object(code)) {}

        template <typename T, typename>
        Var(T &&t);

        template<typename T>
        typename std::enable_if<is_var_element<T>::value, Var>::type &
        operator=(T &&t) {
            using Element = typename notation::type_type<typename notation::remove_cr<T>::type>::type;
            // TODO: check if there is need to update new
            m_var = Element::Make(std::forward<T>(t));
            if (m_notifier) {
                m_notifier(m_var);
            }
            return *this;
        }

        template <typename T>
        typename std::enable_if<
                !is_var_element<T>::value &&
                std::is_integral<T>::value, Var>::type &
        operator=(T i) {
            return this->operator=(notation::other_int<T>::type(i));
        }

        template <typename T>
        typename std::enable_if<
                !is_var_element<T>::value &&
                std::is_constructible<std::string, T>::value
                , Var>::type &
        operator=(T &&t) {
            return this->operator=(std::string(std::forward<T>(t)));
        }

        Var(const Var &var) : self(var.m_var) {}

        Var &operator=(const Var &var) {
            this->m_var = var.m_var;
            return *this;
        }

        Var &operator=(Var &&var) {
            this->m_var = std::move(var.m_var);
            this->m_notifier = std::move(var.m_notifier);
            return *this;
        }

        operator notation::Element::shared() { return m_var; }

        operator notation::Element::shared() const { return m_var; }

        Var operator[](const std::string &key) {
            if (!m_var) {
                m_var = notation::code_type<notation::type::Object>::type::Make();
            } else if (!m_var->is_object()) {
                throw VarNotSupportSlice(m_var->code);
            }
            auto &data = reinterpret_cast<notation::ElementObject *>(m_var.get())->data;
            auto it = data.find(key);
            if (it == data.end()) {
                notation::Element::weak storage = m_var;
                auto notifier = [=](notation::Element::shared var) {
                    auto shared_storage = storage.lock();
                    if (!shared_storage) return;
                    auto obj = reinterpret_cast<notation::ElementObject *>(shared_storage.get());
                    obj->data[key] = std::move(var);
                };
                return Var(notifier);
            } else {
                notation::Element::weak storage = m_var;
                auto &value = it->second;
                auto notifier = [=, &value](notation::Element::shared var) {
                    auto shared_storage = storage.lock();
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
                throw VarNotSupportSlice(m_var->code);
            }
            auto &data = reinterpret_cast<notation::ElementObject *>(m_var.get())->data;
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
                !std::is_same<std::string, typename notation::remove_cr<T>::type>::value>::type>
        Var operator[](T &&t) {
            return this->operator[](std::string(std::forward<T>(t)));
        }

        template<typename T, typename=typename std::enable_if<
                std::is_constructible<std::string, T>::value &&
                !std::is_same<std::string, typename notation::remove_cr<T>::type>::value>::type>
        Var operator[](T &&t) const {
            return this->operator[](std::string(std::forward<T>(t)));
        }

        Var operator[](const int64_t &index) {
            if (!m_var) {
                throw VarNotSupportSlice(notation::type::Undefined, index);
            } else if (!m_var->is_array()) {
                throw VarNotSupportSlice(m_var->code, index);
            }
            auto &data = reinterpret_cast<notation::ElementArray *>(m_var.get())->data;
            auto data_size = int64_t(data.size());
            auto fixed_index = index >= 0 ? index : index + data_size;
            if (fixed_index < 0 || fixed_index >= data_size) {
                throw VarIndexOutOfRange(m_var->code, fixed_index, data.size());
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
                throw VarNotSupportSlice(m_var->code, index);
            }
            auto &data = reinterpret_cast<notation::ElementArray *>(m_var.get())->data;
            auto data_size = int64_t(data.size());
            auto fixed_index = index >= 0 ? index : index + data_size;
            if (fixed_index < 0 || fixed_index >= data_size) {
                throw VarIndexOutOfRange(m_var->code, fixed_index, data.size());
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
            return m_var ? m_var->code : notation::type::Undefined;
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
                return data;
            CASE(notation::type::String)
                return !data.empty();
            CASE(notation::type::Array)
                return data.size();
            CASE(notation::type::Object)
                return data.size();
            CASE(notation::type::Scalar)
                SWITCH_TYPE(m_var->code)
                    CASE_TYPE_INTEGER(return bool(scalar))
                    CASE_TYPE_FLOOT(return scalar != 0)
                    CASE_TYPE_BOOL(return scalar)
                    CASE_TYPE_VOID(return false)
                    CASE_TYPE_CHAR(return bool(scalar))
                    CASE_TYPE_POINTER(return bool(scalar))
                    CASE_TYPE_NOT_SUPPORTED(throw VarOperatorNotSupported(m_var->code, "bool()"))
                END_TYPE
            UNEXPECTED_END
        }

        template<typename T, typename=typename std::enable_if<
                (std::is_integral<T>::value || std::is_floating_point<T>::value) &&
                !std::is_same<T, bool>::value>::type>
        T cpp() const {
            CHECK(std::string(typeid(T).name()) + "()")
            SWITCH
            CASE(notation::type::Boolean)
                return data ? 1 : 0;
            CASE(notation::type::String)
                try {
                    return notation::string2<T>(data);
                } catch (const notation::String2Exception &e) {
                    throw VarException(e.what());
                }
            CASE(notation::type::Scalar)
                SWITCH_TYPE(m_var->code)
                    CASE_TYPE_INTEGER(return T(scalar))
                    CASE_TYPE_FLOOT(return T(T(scalar)))
                    CASE_TYPE_BOOL(return scalar ? 1 : 0)
                    CASE_TYPE_VOID(return 0)
                    CASE_TYPE_CHAR(return T(scalar))
                    DEFAULT_TYPE(throw VarOperatorNotSupported(this->type(), __op))
                END_TYPE
            UNEXPECTED_END
        }

        template<typename T, typename=typename std::enable_if<
                std::is_same<T, std::string>::value>::type>
        std::string cpp() const {
            return this->str();
        }

        void append(Var var) {
            CHECK("append")
            SWITCH
            CASE(notation::type::Array)
                data.push_back(var);
            END
        }

        void extend(Var var) {
            CHECK("extend")
            SWITCH
            CASE(notation::type::Array)
                if (var.type() != notation::type::Array) {
                    throw VarOperatorParameterMismatch(this->type(), __op, 0, {notation::type::Array});
                }
                auto &arr = reinterpret_cast<notation::ElementArray *>(m_var.get())->data;
                data.insert(data.end(), arr.begin(), arr.end());
            CASE(notation::type::Object)
                if (var.type() != notation::type::Object) {
                    throw VarOperatorParameterMismatch(this->type(), __op, 0, {notation::type::Object});
                }
                auto &obj = reinterpret_cast<notation::ElementObject *>(m_var.get())->data;
                for (auto &pair : obj) {
                    data[pair.first] = pair.second;
                }
            END
        }

        void resize(size_t size) {
            CHECK("resize")
            SWITCH
            CASE(notation::type::Array)
                data.resize(size);
            END
        }

        size_t size() const {
            CHECK("size")
            SWITCH
            CASE(notation::type::Array)
                return data.size();
            CASE(notation::type::Object)
                return data.size();
            UNEXPECTED_END
        }

        template<typename T, typename=typename std::enable_if<is_var_convertible<T>::value>::type>
        operator T() const {
            return cpp<T>();
        }

#pragma push_macro("ID_SWITCH")
#pragma push_macro("ID_DEFAULT")
#pragma push_macro("ID_CASE")
#pragma push_macro("ID_END")

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
        auto &data = __at<typename notation::code_type<main_type>::type>(m_var.get())->data;


#define ID_END \
    } \
    }

        void unsafe(void **ptr, size_t *size) const {
#define _UNSAFE_RETURN(a, b) \
            { *ptr = (a); *size = (b); return; }
            if (!m_var) {
                _UNSAFE_RETURN(nullptr, 0)
            }
            ID_SWITCH(m_var->code)
                ID_DEFAULT
                    _UNSAFE_RETURN(nullptr, 0)
                ID_CASE(notation::type::None)
                    _UNSAFE_RETURN(nullptr, 0)
                ID_CASE(notation::type::Boolean)
                    _UNSAFE_RETURN(&data, notation::element_size(data))
                ID_CASE(notation::type::String)
                    _UNSAFE_RETURN(&data, notation::element_size(data))
                ID_CASE(notation::type::Array)
                    _UNSAFE_RETURN(&data, notation::element_size(data))
                ID_CASE(notation::type::Object)
                    _UNSAFE_RETURN(&data, notation::element_size(data))
                ID_CASE(notation::type::Scalar)
                    SWITCH_TYPE(m_var->code)
                        CASE_TYPE_ANY(_UNSAFE_RETURN(&scalar, notation::element_size(scalar)))
                    END_TYPE
            ID_END
            _UNSAFE_RETURN(nullptr, 0)
#undef _UNSAFE_RETURN
        }

        void *id() {
            if (!m_var) return nullptr;
            ID_SWITCH(m_var->code)
                ID_DEFAULT
                    return nullptr;
                ID_CASE(notation::type::None)
                    return nullptr;
                ID_CASE(notation::type::Boolean)
                    return &data;
                ID_CASE(notation::type::String)
                    return &data;
                ID_CASE(notation::type::Array)
                    return &data;
                ID_CASE(notation::type::Object)
                    return &data;
                ID_CASE(notation::type::Scalar)
                    SWITCH_TYPE(m_var->code)
                        CASE_TYPE_ANY(return &scalar);
                    END_TYPE
            ID_END
            return nullptr;
        }

        const void *id() const {
            return const_cast<self *>(this)->id();
        }

        std::string repr() const {
            if (!m_var) return "\"@undefined\"";
            ID_SWITCH(m_var->code)
                ID_DEFAULT
                    return "\"@undefined\"";
                ID_CASE(notation::type::None)
                    return "null";
                ID_CASE(notation::type::Boolean)
                    return data ? "true" : "false";
                ID_CASE(notation::type::String)
                    return "\"" + data + "\"";
                ID_CASE(notation::type::Array)
                    return notation::repr(data);
                ID_CASE(notation::type::Object)
                    return notation::repr(data);
                ID_CASE(notation::type::Scalar)
                    SWITCH_TYPE(m_var->code)
                        CASE_TYPE_INTEGER(return notation::repr(scalar))
                        CASE_TYPE_FLOOT(return notation::repr(scalar))
                        CASE_TYPE_BOOL(return notation::repr(m_var->code, &scalar, notation::element_size(scalar)))
                        CASE_TYPE_VOID(return notation::repr(m_var->code, nullptr, 0))
                        CASE_TYPE_CHAR(return notation::repr(m_var->code, &scalar, notation::element_size(scalar)))
                        CASE_TYPE_POINTER(return notation::repr(m_var->code, &scalar, notation::element_size(scalar)))
                        CASE_TYPE_NOT_SUPPORTED(return notation::repr(m_var->code, &scalar, notation::element_size(scalar)))
                    END_TYPE
            ID_END
            return nullptr;
        }

        std::string str() const {
            if (m_var && (m_var->code & 0xFF00) == notation::type::String) {
                auto &data = __at<typename notation::code_type<notation::type::String>::type>(m_var.get())->data;
                return data;
            }
            return repr();
        }

#pragma pop_macro("ID_SWITCH")
#pragma pop_macro("ID_DEFAULT")
#pragma pop_macro("ID_CASE")
#pragma pop_macro("ID_END")

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
        std::function<void(notation::Element::shared)> m_notifier;

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

    template<typename T, typename=typename std::enable_if<
            is_var_assignable<T>::value>::type>
    Var::Var(T &&t) {
        this->operator=(std::forward<T>(t));
    }

    namespace notation {
        inline std::string repr(Element::shared element) {
            return Var(element).repr();
        }
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
#pragma pop_macro("CASE_NO_TYPE")
#pragma pop_macro("CASE_TYPE_INTEGER")
#pragma pop_macro("CASE_TYPE_FLOOT")
#pragma pop_macro("CASE_TYPE_BOOL")
#pragma pop_macro("CASE_TYPE_NOT_SUPPORTED")
#pragma pop_macro("CASE_TYPE_CHAR")
#pragma pop_macro("CASE_TYPE_VOID")
#pragma pop_macro("CASE_TYPE_POINTER")
#pragma pop_macro("CASE_TYPE_ANY")

#endif //OMEGA_VAR_VAR_H
