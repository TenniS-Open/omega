//
// Created by kier on 2020/8/30.
//

#ifndef OMEGA_VAR_EXCEPTION_H
#define OMEGA_VAR_EXCEPTION_H

#include "type.h"
#include <stdexcept>

namespace ohm {

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
}

#endif //OMEGA_VAR_EXCEPTION_H
