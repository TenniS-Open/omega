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

    notation::TypedField::shared code2object(notation::DataType code) {
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

        static std::string Message(notation::DataType type) {
            std::ostringstream oss;
            oss << notation::type_string(type) << " does not support slice.";
            return oss.str();
        }
    };


    class Var {
    public:
        using self = Var;

        Var() = default;

        Var(notation::DataType code) : self(code2object(code)) {}

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

        Var operator[](const std::string &key) {
            if (!m_var) {
                // throw VarNotSupportSlice(notation::type::Undefined);
                m_var = notation::code_type<notation::type::Object>::type::Make();
            } else if (!m_var->is_object()) {
                throw VarNotSupportSlice(m_var->code);
            }
            auto data = reinterpret_cast<notation::Object*>(m_var.get());
            auto it = data->data.find(key);
            if (it == data->data.end()) {
                notation::TypedField::weak storage = m_var;
                auto notifier = [=](notation::TypedField::shared var) {
                    auto shared_storage = storage.lock();
                    if (!shared_storage) return;
                    auto obj = reinterpret_cast<notation::Object*>(shared_storage.get());
                    obj->data[key] = std::move(var);
                };
                return Var(notifier);
            } else {
                notation::TypedField::weak storage = m_var;
                auto &value = it->second;
                auto notifier = [=, &value](notation::TypedField::shared var) {
                    auto shared_storage = storage.lock();
                    if (!shared_storage) return;
                    auto obj = reinterpret_cast<notation::Object*>(shared_storage.get());
                    value = std::move(var);
                };
                return Var(value, notifier);
            }
        }

    private:
        using Notifier = std::function<void(notation::TypedField::shared)>;
        explicit Var(notation::TypedField::shared var)
            : m_var(std::move(var)) {}
        explicit Var(notation::TypedField::shared var, Notifier notifier)
            : m_var(std::move(var)), m_notifier(std::move(notifier)) {}
        explicit Var(Notifier notifier)
                : m_notifier(std::move(notifier)) {}

        notation::TypedField::shared m_var;
        std::function<void(notation::TypedField::shared)> m_notifier;
    };
}

#endif //OMEGA_VAR_VAR_H
