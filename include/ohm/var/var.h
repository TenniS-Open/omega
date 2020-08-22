//
// Created by kier on 2020/8/19.
//

#ifndef OMEGA_VAR_VAR_H
#define OMEGA_VAR_VAR_H

#include "notation.h"
#include "type.h"
#include "scalar.h"
#include "null.h"
#include "./string.h"
#include "array.h"
#include "object.h"

#include "notation.h"

#include "../type_cast.h"

#include <type_traits>
#include <sstream>

namespace ohm {
    template <typename T, typename=void>
    struct is_var_assignable : public std::false_type {};

    template <typename T>
    struct is_var_assignable<T, typename std::enable_if<
            notation::is_notation_type<typename remove_cr<T>::type>::value &&
            std::is_constructible<T, typename notation::type_type<T>::type>::value
            >::type> : public std::true_type {};

//    inline constexpr bool is_var_code_signed()
//
//    inline constexpr bool is_var_code_convertible(notation::DataType from, notation::DataType to) {
//        if ((from & 0xFF00) == notation::type::Scalar && (to & 0xFF00) == notation::type::Scalar) {
//
//        }
//        return false;
//    }

    notation::Element::shared code2object(notation::DataType code) {
        using namespace notation;
        switch (code & 0xFF00) {
            default: return nullptr;
            case type::None: return code_type<type::None>::type::Make();
            case type::String: return code_type<type::String>::type::Make();
            case type::Array: return code_type<type::Array>::type::Make();
            case type::Object: return code_type<type::Object>::type::Make();
            case type::Binary: return code_type<type::String>::type::Make();
            case type::Undefined: return code_type<type::String>::type::Make();

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
                const std::string &op,
                const std::vector<notation::DataType> &supported)
                : supper(Message(type, op, supported)) {}

        static std::string Message(notation::DataType type,
                                   const std::string &op,
                                   const std::vector<notation::DataType> &supported) {
            std::ostringstream oss;
            oss << notation::type_string(type) << " operator " << op << " not supported, excpeting: ";
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
            oss << notation::main_type_string(type) << " operator " << op << " not supported.";
            return oss.str();
        }
    };

#pragma push_macro("CHECK")
#pragma push_macro("SWITCH")
#pragma push_macro("CASE")
#pragma push_macro("END")

#define CHECK(op) \
    std::string __check_op = op; \
    bool __checked = false; \
    std::vector<notation::DataType> __checked_type;

#define SWITCH \
    {

#define CASE(main_type) \
    } \
    __checked_type.push_back(main_type); \
    if (m_var && (m_var->code & 0xFF00) == (main_type & 0xFF00)) { \
        __checked = true; \
        auto &data = reinterpret_cast<typename notation::code_type<main_type>::type*>(m_var.get())->data;

#define ELSE \
    } \
    { \
        __checked = true;

#define END \
    } \
    if (!__checked) { \
        throw VarOperatorNotSupported(m_var ? m_var->code : notation::type::Undefined, \
            __check_op, __checked_type); \
    }

#define UNEXPECTED_END \
    } \
    throw VarOperatorNotSupported(m_var ? m_var->code : notation::type::Undefined, \
        __check_op, __checked_type);

#define DEFAULT(expr) \
    } \
    if (!__checked) { \
        throw VarOperatorNotSupported(m_var ? m_var->code : notation::type::Undefined, \
            __check_op, __checked_type); \
    } else { \
        return expr; \
    }

    class Var {
    public:
        using self = Var;

        Var() = default;

        // Var(notation::DataType code) : self(code2object(code)) {}

        template <typename T, typename=typename std::enable_if<
                is_var_assignable<T>::value
                >::type>
        Var(T &&t) : self(notation::type_type<T>::type::Make(std::forward<T>(t))) {}

        template <typename T>
        typename std::enable_if<is_var_assignable<T>::value, Var>::type &
        operator=(T &&t) {
            using Element = typename notation::type_type<T>::type;
            // TODO: check if there is need to update new
            m_var = Element::Make(std::forward<T>(t));
            if (m_notifier) {
                m_notifier(m_var);
            }
            return *this;
        }

        Var(const Var &var) : self(var.m_var) {}

        Var &operator=(const Var &var) {
            this->m_var = var.m_var;
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
            auto &data = reinterpret_cast<notation::ElementObject*>(m_var.get())->data;
            auto it = data.find(key);
            if (it == data.end()) {
                notation::Element::weak storage = m_var;
                auto notifier = [=](notation::Element::shared var) {
                    auto shared_storage = storage.lock();
                    if (!shared_storage) return;
                    auto obj = reinterpret_cast<notation::ElementObject*>(shared_storage.get());
                    obj->data[key] = std::move(var);
                };
                return Var(notifier);
            } else {
                notation::Element::weak storage = m_var;
                auto &value = it->second;
                auto notifier = [=, &value](notation::Element::shared var) {
                    auto shared_storage = storage.lock();
                    if (!shared_storage) return;
                    auto obj = reinterpret_cast<notation::ElementObject*>(shared_storage.get());
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
            auto &data = reinterpret_cast<notation::ElementObject*>(m_var.get())->data;
            auto it = data.find(key);
            if (it == data.end()) {
                return Var();
            } else {
                notation::Element::weak storage = m_var;
                auto &value = it->second;
                return Var(value);
            }
        }

        Var operator[](const int64_t &index) {
            if (!m_var) {
                throw VarNotSupportSlice(notation::type::Undefined, index);
            } else if (!m_var->is_array()) {
                throw VarNotSupportSlice(m_var->code, index);
            }
            auto &data = reinterpret_cast<notation::ElementArray*>(m_var.get())->data;
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
            auto &data = reinterpret_cast<notation::ElementArray*>(m_var.get())->data;
            auto data_size = int64_t(data.size());
            auto fixed_index = index >= 0 ? index : index + data_size;
            if (fixed_index < 0 || fixed_index >= data_size) {
                throw VarIndexOutOfRange(m_var->code, fixed_index, data.size());
            }
            auto &value = data.at(size_t(fixed_index));
            return Var(value);
        }

        void append(Var var) {
            CHECK("append")
            SWITCH
            CASE(notation::type::Array)
                data.push_back(var);
                break;
            END
        }

        void resize(size_t size) {
            CHECK("resize")
            SWITCH
            CASE(notation::type::Array)
                data.resize(size);
                return;
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
    };
}

#pragma pop_macro("END")
#pragma pop_macro("CASE")
#pragma pop_macro("SWITCH")
#pragma pop_macro("CHECK")

#endif //OMEGA_VAR_VAR_H
