/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     03/22/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 */

#include <Plain/Value.hpp>

#include <cstdio>
#include <cstring>

namespace plain {

    /* -----------------------------------------------------------------------
     * Value — construction / destruction
     * --------------------------------------------------------------------- */

    void Value::clear() {
        PLAIN_VALUE_CLEAR(&_value);
    }

    void Value::copy_from(const PLAIN_VALUE* source) {
        if(source == nullptr) { _value = {}; return; }
        PLAIN_VALUE_COPY(&_value, source);
    }

    Value::Value()                            { _value = {}; }
    Value::Value(const Value& other)          { copy_from(&other._value); }
    Value::Value(Value&& other) noexcept      { _value = other._value; other._value = {}; }
    Value::~Value()                           { clear(); }

    Value::Value(bool boolean) {
        _value = {};
        PLAIN_EXPORT(boolean ? (PLAIN_BYTE*)0xFF : nullptr, 0, PLAIN_TYPE_BOOLEAN, &_value);
    }

    Value::Value(int number) {
        _value = {};
        PLAIN_WORD_DOUBLE storage = static_cast<PLAIN_WORD_DOUBLE>(number);
        PLAIN_EXPORT((PLAIN_BYTE*)&storage, sizeof(PLAIN_WORD_DOUBLE), PLAIN_TYPE_INTEGER, &_value);
    }

    Value::Value(double number) {
        _value = {};
        PLAIN_REAL storage = static_cast<PLAIN_REAL>(number);
        PLAIN_EXPORT((PLAIN_BYTE*)&storage, sizeof(PLAIN_REAL), PLAIN_TYPE_REAL, &_value);
    }

    Value::Value(const std::string& text) {
        _value = {};
        PLAIN_EXPORT((PLAIN_BYTE*)text.c_str(),
                     static_cast<PLAIN_WORD_DOUBLE>(text.size() + 1),
                     PLAIN_TYPE_STRING, &_value);
    }

    Value::Value(const char* text) {
        _value = {};
        if(text == nullptr) return;
        PLAIN_EXPORT((PLAIN_BYTE*)text,
                     static_cast<PLAIN_WORD_DOUBLE>(strlen(text) + 1),
                     PLAIN_TYPE_STRING, &_value);
    }

    Value::Value(const PLAIN_VALUE* source) {
        _value = {};
        copy_from(source);
    }

    Value& Value::operator=(const Value& other) {
        if(this != &other) { clear(); copy_from(&other._value); }
        return *this;
    }

    Value& Value::operator=(Value&& other) noexcept {
        if(this != &other) { clear(); _value = other._value; other._value = {}; }
        return *this;
    }

    /* -----------------------------------------------------------------------
     * Value — type queries
     * --------------------------------------------------------------------- */

    bool Value::is_nil()    const { return _value.type == PLAIN_TYPE_NIL; }
    bool Value::is_true()   const { return PLAIN_VALUE_TRUTHY(&_value) != 0; }
    bool Value::is_object() const { return _value.type == PLAIN_TYPE_OBJECT; }

    std::string Value::type_name() const {
        switch(_value.type) {
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
     * Value — conversions
     * --------------------------------------------------------------------- */

    bool Value::as_boolean() const {
        return PLAIN_VALUE_TRUTHY(&_value) != 0;
    }

    int Value::as_integer() const {
        if(_value.type == PLAIN_TYPE_INTEGER && _value.data)
            return static_cast<int>(*(PLAIN_WORD_DOUBLE*)_value.data);
        if(_value.type == PLAIN_TYPE_REAL && _value.data)
            return static_cast<int>(*(PLAIN_REAL*)_value.data);
        if(_value.type == PLAIN_TYPE_BOOLEAN)
            return _value.data != nullptr ? 1 : 0;
        if(_value.type == PLAIN_TYPE_STRING && _value.data)
            return static_cast<int>(atoi((const char*)_value.data));
        return 0;
    }

    double Value::as_real() const {
        if(_value.type == PLAIN_TYPE_REAL && _value.data)
            return static_cast<double>(*(PLAIN_REAL*)_value.data);
        if(_value.type == PLAIN_TYPE_INTEGER && _value.data)
            return static_cast<double>(static_cast<int>(*(PLAIN_WORD_DOUBLE*)_value.data));
        if(_value.type == PLAIN_TYPE_BOOLEAN)
            return _value.data != nullptr ? 1.0 : 0.0;
        if(_value.type == PLAIN_TYPE_STRING && _value.data)
            return atof((const char*)_value.data);
        return 0.0;
    }

    std::string Value::as_string() const {
        char buffer[64];
        switch(_value.type) {
            case PLAIN_TYPE_STRING:
            case PLAIN_TYPE_KEYWORD:
                return _value.data ? std::string((const char*)_value.data) : "";
            case PLAIN_TYPE_INTEGER:
                if(_value.data) {
                    snprintf(buffer, sizeof(buffer), "%d",
                             static_cast<int>(*(PLAIN_WORD_DOUBLE*)_value.data));
                    return buffer;
                }
                return "0";
            case PLAIN_TYPE_REAL:
                if(_value.data) {
                    snprintf(buffer, sizeof(buffer), "%g",
                             static_cast<double>(*(PLAIN_REAL*)_value.data));
                    return buffer;
                }
                return "0";
            case PLAIN_TYPE_BOOLEAN:
                return _value.data != nullptr ? "yes" : "no";
            case PLAIN_TYPE_OBJECT:
                return "<object>";
            case PLAIN_TYPE_NIL:
                return "none";
            default:
                return "";
        }
    }

} /* namespace plain */
