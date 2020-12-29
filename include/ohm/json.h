//
// Created by kier on 2020/6/11.
//

#ifndef OMEGA_JSON_H
#define OMEGA_JSON_H

#include "var/var.h"
#include "type_name.h"
#include "except.h"
#include "var/parser.h"

#include <type_traits>

namespace ohm {
    using notation::Binary;

    class JSONBase {
    public:
        using self = JSONBase;

        virtual ~JSONBase() = default;

        virtual void parse(const Var &obj) = 0;
    };

    namespace json {
        inline Var from_string(const std::string &str) {
            return parser::from_string(str);
        }

        inline Var from_string(const char *str) {
            return parser::from_string(str);
        }

        inline Var from_string(const char *buff, size_t size) {
            return parser::from_string(buff, size);
        }

        inline std::string to_string(const Var &var) {
            return parser::to_string(var);
        }

        template<typename T>
        using Array = std::vector<T>;

        template<typename T>
        using Dict = std::map<std::string, T>;

        template<typename T, size_t N>
        using SizeArray = std::array<T, N>;


        namespace _ {

            template<typename T, typename Enable = void>
            class _parser;

            template<typename T>
            class _parser<T, typename std::enable_if<
                    !std::is_same<T, bool>::value && std::is_integral<T>::value>::type> {
            public:
                static T parse(const Var &obj) {
                    return static_cast<T>(obj);
                }

                static T parse(const Var &obj, const T &val) {
                    try {
                        return parse(obj);
                    } catch (...) {
                        return val;
                    }
                }
            };

            template<typename T>
            class _parser<T, typename std::enable_if<std::is_same<T, bool>::value>::type> {
            public:
                static T parse(const Var &obj) {
                    return static_cast<T>(static_cast<bool>(obj));
                }

                static T parse(const Var &obj, const T &val) {
                    try {
                        return parse(obj);
                    } catch (...) {
                        return val;
                    }
                }
            };

            template<typename T>
            class _parser<T, typename std::enable_if<std::is_floating_point<T>::value>::type> {
            public:
                static T parse(const Var &obj) {
                    return static_cast<T>(obj);
                }

                static T parse(const Var &obj, const T &val) {
                    try {
                        return parse(obj);
                    } catch (...) {
                        return val;
                    }
                }
            };

            template<typename T>
            class _parser<T, typename std::enable_if<std::is_same<T, std::string>::value>::type> {
            public:
                static T parse(const Var &obj) {
                    return static_cast<T>(obj);
                }

                static T parse(const Var &obj, const T &val) {
                    try {
                        return parse(obj);
                    } catch (...) {
                        return val;
                    }
                }
            };

            template<typename T>
            class _parser<T, typename std::enable_if<std::is_same<T, Binary>::value>::type> {
            public:
                static T parse(const Var &obj) {
                    return static_cast<T>(obj);
                }
            };

            template<typename T>
            class _parser<T, typename std::enable_if<std::is_same<T, Var>::value>::type> {
            public:
                static T parse(const Var &obj) {
                    return obj;
                }
            };

            template<typename T>
            struct is_Var_scalar {
                static constexpr bool value = std::is_integral<T>::value
                                              || std::is_floating_point<T>::value
                                              || std::is_same<T, std::string>::value
                                              || std::is_same<T, Binary>::value
                                              || std::is_same<T, Var>::value;
            };

            template<typename T>
            struct is_Var_with_default {
                static constexpr bool value = std::is_integral<T>::value
                                              || std::is_floating_point<T>::value
                                              || std::is_same<T, std::string>::value;
            };


            template<typename T, typename Enable = void>
            class ParserType;

            template<typename T>
            class ParserType<T, typename std::enable_if<is_Var_with_default<T>::value>::type>
                    : public JSONBase {
            public:
                explicit ParserType(T *value)
                        : m_value(value), m_has_default(false) {}

                explicit ParserType(T *value, const T &default_value)
                        : m_value(value), m_default(default_value), m_has_default(true) {}

                explicit ParserType(const std::string &name, T *value)
                        : m_name(name), m_value(value), m_has_default(false) {}

                explicit ParserType(const std::string &name, T *value, const T &default_value)
                        : m_name(name), m_value(value), m_default(default_value), m_has_default(true) {}

                void parse(const Var &obj) override {
                    if (m_has_default) {
                        *m_value = _parser<T>::parse(obj, m_default);
                    } else {
                        try {
                            *m_value = _parser<T>::parse(obj);
                        } catch (const Exception &e) {
                            if (m_name.empty()) {
                                throw Exception(e.what());
                            }
                            throw Exception("Can not parse \"" + m_name + "\": " + e.what());
                        }
                    }
                }

            private:
                std::string m_name;
                T *m_value;
                T m_default;
                bool m_has_default;
            };

            template<typename T>
            class ParserType<T, typename std::enable_if<
                    std::is_same<T, Binary>::value>::type>
                    : public JSONBase {
            public:
                explicit ParserType(T *value)
                        : m_value(value) {}

                explicit ParserType(const std::string &name, T *value)
                        : m_name(name), m_value(value) {}

                void parse(const Var &obj) override {
                    try {
                        *m_value = obj;
                        // do not believe IDE told you, there can be exception throw.
                    } catch (const Exception &e) {
                        if (m_name.empty()) {
                            throw Exception(e.what());
                        }
                        throw Exception("Can not parse \"" + m_name + "\": " + e.what());
                    }
                }

            private:
                std::string m_name;
                T *m_value;
            };

            template<typename T>
            class ParserType<T, typename std::enable_if<
                    std::is_same<T, Var>::value>::type>
                    : public JSONBase {
            public:
                explicit ParserType(T *value)
                        : m_value(value) {}

                explicit ParserType(const std::string &, T *value)
                        : m_value(value) {}

                void parse(const Var &obj) override {
                    *m_value = obj;
                }

            private:
                T *m_value;
            };

            template<typename T>
            class ParserType<T, typename std::enable_if<std::is_base_of<JSONBase, T>::value>::type>
                    : public JSONBase {
            public:
                explicit ParserType(T *value)
                        : m_value(value) {}

                explicit ParserType(const std::string &name, T *value)
                        : m_name(name), m_value(value) {}

                void parse(const Var &obj) override {
                    m_value->parse(obj);
                }

            private:
                std::string m_name;
                T *m_value;
            };

            template<typename>
            struct is_Var_array {
                static constexpr bool value = false;
            };

            template<typename>
            struct is_Var_dict {
                static constexpr bool value = false;
            };

            template<typename T>
            struct is_Var_array<Array<T>> {
                static constexpr bool value = is_Var_scalar<T>::value
                                              || (std::is_base_of<JSONBase, T>::value &&
                                                  std::is_default_constructible<T>::value)
                                              || is_Var_array<T>::value
                                              || is_Var_dict<T>::value;
            };

            template<typename T, size_t N>
            struct is_Var_array<SizeArray<T, N>> {
                static constexpr bool value = is_Var_scalar<T>::value
                                              || (std::is_base_of<JSONBase, T>::value &&
                                                  std::is_default_constructible<T>::value)
                                              || is_Var_array<T>::value
                                              || is_Var_dict<T>::value;
            };

            template<typename T>
            struct is_Var_dict<Dict<T>> {
                static constexpr bool value = is_Var_scalar<T>::value
                                              || (std::is_base_of<JSONBase, T>::value &&
                                                  std::is_default_constructible<T>::value)
                                              || is_Var_array<T>::value
                                              || is_Var_dict<T>::value;
            };

            template<typename T>
            class ParserType<Array<T>, typename std::enable_if<is_Var_scalar<T>::value>::type>
                    : public JSONBase {
            public:
                using Array = std::vector<T>;

                explicit ParserType(Array *value)
                        : m_value(value) {}

                explicit ParserType(const std::string &name, Array *value)
                        : m_name(name), m_value(value) {}

                void parse(const Var &obj) override {
                    try {
                        if (obj.type() != notation::type::Array) throw Exception("should be array");
                        m_value->resize(obj.size());
                        for (size_t i = 0; i < m_value->size(); ++i) {
                            m_value->at(i) = _parser<T>::parse(obj[i]);
                        }
                    } catch (const Exception &e) {
                        if (m_name.empty()) {
                            throw Exception(e.what());
                        }
                        throw Exception("Can not parse \"" + m_name + "\": " + e.what());
                    }
                }

            private:
                std::string m_name;
                Array *m_value;
            };

            template<typename T, size_t N>
            class ParserType<SizeArray<T, N>, typename std::enable_if<is_Var_scalar<T>::value>::type>
                    : public JSONBase {
            public:
                using Array = std::array<T, N>;

                explicit ParserType(Array *value)
                        : m_value(value) {}

                explicit ParserType(const std::string &name, Array *value)
                        : m_name(name), m_value(value) {}

                void parse(const Var &obj) override {
                    try {
                        if (obj.type() != notation::type::Array) throw Exception("should be array");
                        // m_value->resize(obj.size());
                        if (obj.size() != N) throw Exception("must has size " + std::to_string(N));
                        for (size_t i = 0; i < m_value->size(); ++i) {
                            m_value->at(i) = _parser<T>::parse(obj[i]);
                        }
                    } catch (const Exception &e) {
                        if (m_name.empty()) {
                            throw Exception(e.what());
                        }
                        throw Exception("Can not parse \"" + m_name + "\": " + e.what());
                    }
                }

            private:
                std::string m_name;
                Array *m_value;
            };

            template<typename T>
            class ParserType<Array<T>, typename std::enable_if<
                    (std::is_base_of<JSONBase, T>::value && std::is_default_constructible<T>::value)
                    || is_Var_array<T>::value
                    || is_Var_dict<T>::value>::type>
                    : public JSONBase {
            public:
                using Array = std::vector<T>;

                explicit ParserType(Array *value)
                        : m_value(value) {}

                explicit ParserType(const std::string &name, Array *value)
                        : m_name(name), m_value(value) {}

                void parse(const Var &obj) override {
                    try {
                        if (obj.type() != notation::type::Array) throw Exception("should be array");
                        m_value->resize(obj.size());
                        for (size_t i = 0; i < m_value->size(); ++i) {
                            auto setter = ParserType<T>(
                                    m_name + "[" + std::to_string(i) + "]",
                                    &m_value->at(i));
                            setter.parse(obj[i]);
                        }
                    } catch (const Exception &e) {
                        if (m_name.empty()) {
                            throw Exception(e.what());
                        }
                        throw Exception("Can not parse \"" + m_name + "\": " + e.what());
                    }
                }

            private:
                std::string m_name;
                Array *m_value;
            };

            template<typename T>
            class ParserType<Dict<T>, typename std::enable_if<is_Var_scalar<T>::value>::type>
                    : public JSONBase {
            public:
                using Dict = std::map<std::string, T>;

                explicit ParserType(Dict *value)
                        : m_value(value) {}

                explicit ParserType(const std::string &name, Dict *value)
                        : m_name(name), m_value(value) {}

                void parse(const Var &obj) override {
                    try {
                        if (obj.type() != notation::type::Object) throw Exception("should be object");
                        auto keys = obj.keys();
                        for (auto &key : keys) {
                            (*m_value)[key] = _parser<T>::parse(obj[key]);
                        }
                    } catch (const Exception &e) {
                        if (m_name.empty()) {
                            throw Exception(e.what());
                        }
                        throw Exception("Can not parse \"" + m_name + "\": " + e.what());
                    }
                }

            private:
                std::string m_name;
                Dict *m_value;
            };

            template<typename T>
            class ParserType<Dict<T>, typename std::enable_if<
                    (std::is_base_of<JSONBase, T>::value && std::is_default_constructible<T>::value)
                    || is_Var_array<T>::value
                    || is_Var_dict<T>::value>::type>
                    : public JSONBase {
            public:
                using Dict = std::map<std::string, T>;

                explicit ParserType(Dict *value)
                        : m_value(value) {}

                explicit ParserType(const std::string &name, Dict *value)
                        : m_name(name), m_value(value) {}

                void parse(const Var &obj) override {
                    try {
                        if (obj.type() != notation::type::Object) throw Exception("should be object");
                        auto keys = obj.keys();
                        for (auto &key : keys) {
                            auto setter = ParserType<T>(
                                    m_name + "." + key,
                                    &(*m_value)[key]);
                            setter.parse(obj[key]);
                        }
                    } catch (const Exception &e) {
                        if (m_name.empty()) {
                            throw Exception(e.what());
                        }
                        throw Exception("Can not parse \"" + m_name + "\": " + e.what());
                    }
                }

            private:
                std::string m_name;
                Dict *m_value;
            };

            template<typename T>
            struct support_base_parser {
                static constexpr bool value = is_Var_scalar<T>::value
                                              || std::is_base_of<JSONBase, T>::value
                                              || is_Var_array<T>::value
                                              || is_Var_dict<T>::value;
            };

            template<typename T>
            struct support_parser_with_default {
                static constexpr bool value = is_Var_with_default<T>::value;
            };

            template<typename T>
            struct support_parser {
                static constexpr bool value = is_Var_scalar<T>::value
                                              || std::is_base_of<JSONBase, T>::value
                                              || is_Var_array<T>::value
                                              || is_Var_dict<T>::value
                                              || is_Var_with_default<T>::value;
            };

            template<typename T, typename = typename std::enable_if<support_base_parser<T>::value>::type>
            inline std::function<void(const Var &)> parser(T *value) {
                auto setter = ParserType<T>(value);
                return [setter](const Var &obj) {
                    auto tmp = setter;
                    tmp.parse(obj);
                };
            }

            template<typename T, typename = typename std::enable_if<support_base_parser<T>::value>::type>
            inline std::function<void(const Var &)> parser(T &value) {
                return parser(&value);
            }

            template<typename T, typename = typename std::enable_if<support_base_parser<T>::value>::type>
            inline std::function<void(const Var &)> parser(const std::string &name, T *value) {
                auto setter = ParserType<T>(name, value);
                return [setter](const Var &obj) {
                    auto tmp = setter;
                    tmp.parse(obj);
                };
            }

            template<typename T, typename = typename std::enable_if<support_base_parser<T>::value>::type>
            inline std::function<void(const Var &)> parser(const std::string &name, T &value) {
                return parser(name, &value);
            }

            template<typename T, typename = typename std::enable_if<support_parser_with_default<T>::value>::type>
            inline std::function<void(const Var &)> parser(T *value, const T &default_value) {
                auto setter = ParserType<T>(value, default_value);
                return [setter](const Var &obj) {
                    auto tmp = setter;
                    tmp.parse(obj);
                };
            }

            template<typename T, typename = typename std::enable_if<support_parser_with_default<T>::value>::type>
            inline std::function<void(const Var &)> parser(T &value, const T &default_value) {
                return parser(&value, default_value);
            }

            template<typename T, typename = typename std::enable_if<support_parser_with_default<T>::value>::type>
            inline std::function<void(const Var &)> parser(const std::string &name, T *value, const T &default_value) {
                auto setter = ParserType<T>(name, value, default_value);
                return [setter](const Var &obj) {
                    auto tmp = setter;
                    tmp.parse(obj);
                };
            }

            template<typename T, typename = typename std::enable_if<support_parser_with_default<T>::value>::type>
            inline std::function<void(const Var &)> parser(const std::string &name, T &value, const T &default_value) {
                return parser(name, &value, default_value);
            }
        }
    }

    class JSONObjectBinder;

    class JSONObject : public JSONBase {
    public:
        using Parser = std::function<void(const Var &)>;
        static constexpr uint64_t __MAGIC = 0x8848;
        uint64_t __magic = __MAGIC;

        void parse(const Var &obj) final {
            if (obj.type() != notation::type::Object) throw Exception("JSONObject only parse be dict");
            for (auto &name_required_parser : __m_fields) {
                auto &name = name_required_parser.first;
                bool required = name_required_parser.second.first;
                Parser parser = name_required_parser.second.second;
                auto x = obj[name];
                if (x.type() == notation::type::Undefined) {
                    if (required) {
                        throw Exception("Missing required field \"" + name + "\"");
                    }
                    continue;
                }
                parser(x);
            }
        }

    protected:
        void bind(const std::string &name, Parser parser, bool required = false) {
            __m_fields[name] = std::make_pair(required, parser);
        }
        friend class JSONObjectBinder;

    private:
        std::map<std::string, std::pair<bool, Parser>> __m_fields;
    };

    class JSONObjectBinder {
    protected:
        void bind(JSONObject &object,
                  const std::string &name, JSONObject::Parser parser, bool required = false) {
            object.bind(name, parser, required);
        }
    };
}

#define ohm_risk_offsetof(TYPE, MEMBER)  ((size_t) &((TYPE *)0)->MEMBER)
#define _ohm_json_concat_(x, y) x##y
#define _ohm_json_concat(x, y) _ohm_json_concat_(x, y)

/**
 * @param [in] cls class name of binding, always be the name of defining class
 * @param [in] defined type base json allowed type.
 * @param [in] member name to bind and defined
 * @param [optinal] required if this member are required.
 * @notice type support: integer, floating point, std::string, bool, JSONBase or JSONObject
 *     and json::Array<T> or json::Dict<T> with T can be above supported types.
 */
#define JSONField(cls, type, member, ...) \
    struct _ohm_json_concat(__struct_bind_, member) : public ohm::JSONObjectBinder { \
        _ohm_json_concat(__struct_bind_, member)() { \
            static_assert(std::is_base_of<ohm::JSONObject, cls>::value, "JSONField only support in JSONObject"); \
            auto _supper = reinterpret_cast<cls*>(reinterpret_cast<char*>(this) - ohm_risk_offsetof(cls, _ohm_json_concat(__bind_, member))); \
            if (_supper->__magic != ohm::JSONObject::__MAGIC) { \
                throw ohm::Exception("Bind member out of class JSONObject"); \
            } \
            auto &_member = _supper->member; \
            bind(*_supper, #member, ohm::json::_::parser(ohm::classname<cls>() + "::" + #member, _member), ## __VA_ARGS__); \
        } \
    } _ohm_json_concat(__bind_, member); \
    type member

/**
 * @param [in] cls class name of binding, always be the name of defining class
 * @param [in] defined type base json allowed type.
 * @param [in] member name defined
 * @param [in] json_member name to bind in json
 * @param [optinal] required if this member are required.
 * @notice type support: integer, floating point, std::string, bool, JSONBase or JSONObject
 *     and json::Array<T> or json::Dict<T> with T can be above supported types.
 */
#define JSONFieldV2(cls, type, member, json_member, ...) \
    struct _ohm_json_concat(__struct_bind_, member) : public ohm::JSONObjectBinder { \
        _ohm_json_concat(__struct_bind_, member)() { \
            static_assert(std::is_base_of<ohm::JSONObject, cls>::value, "JSONFieldV2 only support in JSONObject"); \
            auto _supper = reinterpret_cast<cls*>(reinterpret_cast<char*>(this) - ohm_risk_offsetof(cls, _ohm_json_concat(__bind_, member))); \
            if (_supper->__magic != ohm::JSONObject::__MAGIC) { \
                throw ohm::Exception("Bind member out of class JSONObject"); \
            } \
            auto &_member = _supper->member; \
            bind(*_supper, json_member, ohm::json::_::parser(ohm::classname<cls>() + "::" + #member, _member), ## __VA_ARGS__); \
        } \
    } _ohm_json_concat(__bind_, member); \
    type member

/**
 * @param [in] cls class name of binding, always be the name of defining class
 * @param [in] member predefined member to bind with json
 * @param [optinal] required if this member are required.
 * @notice type support: integer, floating point, std::string, bool, JSONBase or JSONObject
 *     and json::Array<T> or json::Dict<T> with T can be above supported types.
 */
#define JSONBind(cls, member, ...) \
    struct _ohm_json_concat(__struct_bind_, member) : public ohm::JSONObjectBinder { \
        _ohm_json_concat(__struct_bind_, member)() { \
            static_assert(std::is_base_of<ohm::JSONObject, cls>::value, "JSONBind only support in JSONObject"); \
            auto _supper = reinterpret_cast<cls*>(reinterpret_cast<char*>(this) - ohm_risk_offsetof(cls, _ohm_json_concat(__bind_, member))); \
            if (_supper->__magic != ohm::JSONObject::__MAGIC) { \
                throw ohm::Exception("Bind member out of class JSONObject"); \
            } \
            auto &_member = _supper->member; \
            bind(*_supper, #member, ohm::json::_::parser(ohm::classname<cls>() + "::" + #member, _member), ## __VA_ARGS__); \
        } \
    } _ohm_json_concat(__bind_, member)

/**
 * @param [in] cls class name of binding, always be the name of defining class
 * @param [in] member predefined
 * @param [in] json_member name to bind in json
 * @param [optinal] required if this member are required.
 * @notice type support: integer, floating point, std::string, bool, JSONBase or JSONObject
 *     and json::Array<T> or json::Dict<T> with T can be above supported types.
 */
#define JSONBindV2(cls, member, json_member, ...) \
    struct _ohm_json_concat(__struct_bind_, member) : public ohm::JSONObjectBinder { \
        _ohm_json_concat(__struct_bind_, member)() { \
            static_assert(std::is_base_of<ohm::JSONObject, cls>::value, "JSONBindV2 only support in JSONObject"); \
            auto _supper = reinterpret_cast<cls*>(reinterpret_cast<char*>(this) - ohm_risk_offsetof(cls, _ohm_json_concat(__bind_, member))); \
            if (_supper->__magic != ohm::JSONObject::__MAGIC) { \
                throw ohm::Exception("Bind member out of class JSONObject"); \
            } \
            auto &_member = _supper->member; \
            bind(*_supper, json_member, ohm::json::_::parser(ohm::classname<cls>() + "::" + #member, _member), ## __VA_ARGS__); \
        } \
    } _ohm_json_concat(__bind_, member)

#endif //OMEGA_JSON_H
