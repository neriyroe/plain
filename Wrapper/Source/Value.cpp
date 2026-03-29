/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     03/22/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Value — C++ wrapper for PLAIN_VALUE.
 *
 * Provides RAII ownership, copy/move semantics, and convenient
 * conversion to/from C++ scalar types.
 */

#include <Plain/Value.hpp>

#include <cstdio>
#include <cstring>

namespace plain {

    /* -----------------------------------------------------------------------
     * Value — internal helpers
     * --------------------------------------------------------------------- */

    /* Releases all resources owned by the wrapped PLAIN_VALUE. */
    void Value::clear() {
        PLAIN_VALUE_CLEAR(&value);
    }

    /* Deep-copies a raw PLAIN_VALUE into this wrapper. */
    void Value::copy_from(const PLAIN_VALUE* source) {
        if(source == nullptr) {
            value = {};
            return;
        }
        PLAIN_VALUE_COPY(&value, source);
    }

    /* -----------------------------------------------------------------------
     * Value — constructors and destructor
     * --------------------------------------------------------------------- */

    Value::Value() {
        value = {};
    }

    Value::Value(const Value& other) {
        copy_from(&other.value);
    }

    Value::Value(Value&& other) noexcept {
        value = other.value;
        other.value = {};
    }

    Value::~Value() {
        clear();
    }

    /* Construct from a C++ bool. */
    Value::Value(bool boolean) {
        value = {};
        PLAIN_EXPORT(boolean ? (PLAIN_BYTE*)0xFF : nullptr, 0, PLAIN_TYPE_BOOLEAN, &value);
    }

    /* Construct from a C++ integer (any width promotes to long long). */
    Value::Value(long long number) {
        value = {};
        PLAIN_WORD_QUADRUPLE storage = static_cast<PLAIN_WORD_QUADRUPLE>(number);
        PLAIN_EXPORT((PLAIN_BYTE*)&storage, sizeof(PLAIN_WORD_QUADRUPLE), PLAIN_TYPE_INTEGER, &value);
    }

    /* Construct from a C++ double. */
    Value::Value(double number) {
        value = {};
        PLAIN_REAL storage = number;
        PLAIN_EXPORT((PLAIN_BYTE*)&storage, sizeof(PLAIN_REAL), PLAIN_TYPE_REAL, &value);
    }

    /* Construct from a std::string — implicit for convenience. */
    Value::Value(const std::string& text) {
        value = {};
        PLAIN_EXPORT(
            (PLAIN_BYTE*)text.c_str(),
            static_cast<PLAIN_WORD_DOUBLE>(text.size() + 1),
            PLAIN_TYPE_STRING, &value);
    }

    /* Construct from a C string — implicit for convenience. */
    Value::Value(const char* text) {
        value = {};
        if(text == nullptr) {
            return;
        }
        PLAIN_EXPORT(
            (PLAIN_BYTE*)text,
            static_cast<PLAIN_WORD_DOUBLE>(strlen(text) + 1),
            PLAIN_TYPE_STRING, &value);
    }

    /* Construct by copying a raw PLAIN_VALUE. */
    Value::Value(const PLAIN_VALUE* source) {
        value = {};
        copy_from(source);
    }

    /* -----------------------------------------------------------------------
     * Value — assignment operators
     * --------------------------------------------------------------------- */

    Value& Value::operator=(const Value& other) {
        if(this != &other) {
            clear();
            copy_from(&other.value);
        }
        return *this;
    }

    Value& Value::operator=(Value&& other) noexcept {
        if(this != &other) {
            clear();
            value = other.value;
            other.value = {};
        }
        return *this;
    }

    /* -----------------------------------------------------------------------
     * Value — type queries
     * --------------------------------------------------------------------- */

    bool Value::is_nil() const {
        return value.type == PLAIN_TYPE_NIL;
    }

    bool Value::is_true() const {
        return PLAIN_VALUE_TRUTHY(&value) != 0;
    }

    bool Value::is_object() const {
        return value.type == PLAIN_TYPE_OBJECT;
    }

    std::string Value::type_name() const {
        switch(value.type) {
            case PLAIN_TYPE_BOOLEAN:  return "boolean";
            case PLAIN_TYPE_INTEGER:  return "integer";
            case PLAIN_TYPE_REAL:     return "real";
            case PLAIN_TYPE_STRING:   return "string";
            case PLAIN_TYPE_KEYWORD:  return "keyword";
            case PLAIN_TYPE_LIST:     return "list";
            case PLAIN_TYPE_CALLABLE: return "callable";
            case PLAIN_TYPE_OBJECT:   return "object";
            default:                  return "none";
        }
    }

    /* -----------------------------------------------------------------------
     * Value — conversions to C++ types
     * --------------------------------------------------------------------- */

    bool Value::as_boolean() const {
        return PLAIN_VALUE_TRUTHY(&value) != 0;
    }

    long long Value::as_integer() const {
        if(value.type == PLAIN_TYPE_INTEGER && value.data) {
            return static_cast<long long>(*(PLAIN_WORD_QUADRUPLE*)value.data);
        }
        if(value.type == PLAIN_TYPE_REAL && value.data) {
            return static_cast<long long>(*(PLAIN_REAL*)value.data);
        }
        if(value.type == PLAIN_TYPE_BOOLEAN) {
            return value.data != nullptr ? 1 : 0;
        }
        if(value.type == PLAIN_TYPE_STRING && value.data) {
            return atoll((const char*)value.data);
        }
        return 0;
    }

    double Value::as_real() const {
        if(value.type == PLAIN_TYPE_REAL && value.data) {
            return *(PLAIN_REAL*)value.data;
        }
        if(value.type == PLAIN_TYPE_INTEGER && value.data) {
            return static_cast<double>(static_cast<long long>(*(PLAIN_WORD_QUADRUPLE*)value.data));
        }
        if(value.type == PLAIN_TYPE_BOOLEAN) {
            return value.data != nullptr ? 1.0 : 0.0;
        }
        if(value.type == PLAIN_TYPE_STRING && value.data) {
            return atof((const char*)value.data);
        }
        return 0.0;
    }

    /* Produces a human-readable string for any value type. */
    std::string Value::as_string() const {
        char buffer[64];
        switch(value.type) {
            case PLAIN_TYPE_STRING:
            case PLAIN_TYPE_KEYWORD:
                return value.data ? std::string((const char*)value.data) : "";

            case PLAIN_TYPE_INTEGER:
                if(value.data) {
                    snprintf(buffer, sizeof(buffer), "%lld",
                             static_cast<long long>(*(PLAIN_WORD_QUADRUPLE*)value.data));
                    return buffer;
                }
                return "0";

            case PLAIN_TYPE_REAL:
                if(value.data) {
                    snprintf(buffer, sizeof(buffer), "%g",
                             *(PLAIN_REAL*)value.data);
                    return buffer;
                }
                return "0";

            case PLAIN_TYPE_BOOLEAN:
                return value.data != nullptr ? "yes" : "no";

            case PLAIN_TYPE_OBJECT:
                return "<object>";

            case PLAIN_TYPE_NIL:
                return "none";

            default:
                return "";
        }
    }

} /* namespace plain */
