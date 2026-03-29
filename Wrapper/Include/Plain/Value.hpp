/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     03/22/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Value — owns a single Plain value. Converts automatically to and from
 * C++ scalar types. Copy/move safe.
 */

#pragma once

extern "C" {
    #include <Plain/Runtime.h>
}

#include <functional>
#include <string>
#include <vector>

namespace plain {

    /* Forward declarations. */
    class Value;
    class Runtime;
    template<typename T> class ClassBinding;

    /* A list of Plain values passed to a bound procedure or method. */
    using Arguments = std::vector<Value>;

    /* -----------------------------------------------------------------------
     * Value — owns a single Plain value.  Converts automatically to and from
     * C++ scalar types.  Copy/move safe.
     * --------------------------------------------------------------------- */
    class Value {
    public:
        Value();                                     /* none (nil) */
        explicit Value(bool boolean);
        explicit Value(int number);
        explicit Value(double number);
        Value(const std::string& text);              /* implicit: convenient for return "..." */
        Value(const char* text);                     /* implicit: convenient for return "..." */
        explicit Value(const PLAIN_VALUE* source);   /* copies a raw PLAIN_VALUE */

        Value(const Value& other);
        Value(Value&& other) noexcept;
        ~Value();

        Value& operator=(const Value& other);
        Value& operator=(Value&& other) noexcept;

        /* Type queries. */
        bool        is_nil()     const;
        bool        is_true()    const;
        bool        is_object()  const;   /* PLAIN_TYPE_OBJECT; use runtime.as_object<T>() */
        std::string type_name()  const;   /* "none","boolean","integer","real","string","object",... */

        /* Conversions — safe for any type; return zero/false/empty on mismatch. */
        bool        as_boolean() const;
        int         as_integer() const;
        double      as_real()    const;
        std::string as_string()  const;   /* always produces a human-readable representation */

        /* Raw access — valid for the lifetime of this Value. */
        const PLAIN_VALUE& native() const { return value; }
              PLAIN_VALUE& native()       { return value; }

    private:
        PLAIN_VALUE value = {};

        void clear();
        void copy_from(const PLAIN_VALUE* source);
    };

    /* -----------------------------------------------------------------------
     * Handler — signature for procedures bound with Runtime::bind().
     * --------------------------------------------------------------------- */
    using Handler = std::function<Value(const Arguments&)>;

    /* -----------------------------------------------------------------------
     * ClassMethod<T> — signature for methods registered with
     * Runtime::bind_class<T>(). The first argument is a reference to the
     * C++ object instance.
     * --------------------------------------------------------------------- */
    template<typename T>
    using ClassMethod = std::function<Value(T&, const Arguments&)>;

} /* namespace plain */
