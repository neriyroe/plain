/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     03/22/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Plain — C++ interface.
 *
 * Six rules cover everything:
 *   1. plain::Runtime  wraps a Plain context.
 *   2. plain::Value    wraps any Plain value; converts to/from C++ types.
 *   3. runtime.bind()       registers any C++ lambda as a Plain procedure.
 *   4. runtime.bind_class() registers a C++ class so Plain can construct and
 *                           call objects of that class.
 *   5. runtime.run()        evaluates Plain source code.
 *   6. runtime.call()       calls a Plain procedure from C++.
 *
 * Include this header from C++ code instead of <Plain/VM.h>.
 */

#pragma once

#include <Plain/VM.h>

#include <cassert>
#include <functional>
#include <initializer_list>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace plain {

    /* Forward declarations. */
    class Value;
    class Runtime;
    using Args = std::vector<Value>;

    /* -----------------------------------------------------------------------
     * Value — owns a single Plain value.  Converts automatically to and from
     * C++ scalar types.  Copy/move safe.
     * --------------------------------------------------------------------- */
    class Value {
    public:
        Value();                                     /* none (nil) */
        explicit Value(bool b);
        explicit Value(int n);
        explicit Value(double d);
        Value(const std::string& s);                 /* implicit: convenient for return "..." */
        Value(const char* s);                        /* implicit: convenient for return "..." */
        explicit Value(const PLAIN_VALUE* src);      /* copies a raw PLAIN_VALUE */

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
        void copy_from(const PLAIN_VALUE* src);
    };

    /* -----------------------------------------------------------------------
     * Handler — signature for procedures bound with runtime.bind().
     * --------------------------------------------------------------------- */
    using Handler = std::function<Value(const Args&)>;

    /* -----------------------------------------------------------------------
     * ClassMethod<T> — signature for methods registered with bind_class<T>().
     * The first argument is a reference to the C++ object instance.
     * --------------------------------------------------------------------- */
    template<typename T>
    using ClassMethod = std::function<Value(T&, const Args&)>;

    /* -----------------------------------------------------------------------
     * detail — internal types used by the object bridge.
     * --------------------------------------------------------------------- */
    namespace detail {

        using DispatchFn = std::function<Value(void*, const Args&)>;

        struct DispatchTable {
            std::unordered_map<std::string, DispatchFn> methods;
        };

        struct ObjectEntry {
            std::shared_ptr<void>          instance;
            std::shared_ptr<DispatchTable> dispatch;
        };

        struct List {
            std::vector<Value> items;

            List() = default;
            List(std::vector<Value> init) : items(std::move(init)) {}
        };

    } /* namespace detail */

    /* -----------------------------------------------------------------------
     * Runtime — the main entry point.  Create one, bind procedures, run code.
     * --------------------------------------------------------------------- */
    class Runtime {
    public:
        Runtime();
        ~Runtime();

        Runtime(const Runtime&)            = delete;
        Runtime& operator=(const Runtime&) = delete;

        /* Register a C++ lambda/function as a mutable Plain procedure.
         * Plain code can read, override, or pass it as a first-class value. */
        Runtime& bind(const std::string& name, Handler handler);

        /* Register a C++ class so Plain can construct and call objects.
         *
         *   constructor  — receives Plain arguments, returns a shared_ptr<T>.
         *   methods      — list of { "name", [](T& self, const Args&){...} } pairs.
         *
         * Plain usage:
         *   obj = [ClassName arg1, arg2]
         *   result = [obj method_name extra_arg]
         */
        template<typename T>
        Runtime& bind_class(
            const std::string& name,
            std::function<std::shared_ptr<T>(const Args&)> constructor,
            std::initializer_list<std::pair<std::string, ClassMethod<T>>> methods = {}
        );

        /* Evaluate a string of Plain source code. */
        Runtime& run(const std::string& source);

        /* Call a named Plain procedure from C++.
         * Looks up the binding by name and invokes it with the given arguments. */
        Value call(const std::string& name, const Args& args = {});

        /* Invoke any callable Value from C++ (e.g. an object method closure). */
        Value invoke(const Value& callable, const Args& args = {});

        /* Read the value of a Plain variable from the current frame. */
        Value get(const std::string& name) const;

        /* Write a variable into the current frame. */
        Runtime& set(const std::string& name, const Value& value);

        /* Install an error callback — called with a human-readable message on
         * any syntax or runtime error. */
        Runtime& on_error(std::function<void(const std::string&)> callback);

        /* Wrap a C++ object into a Plain value (PLAIN_TYPE_OBJECT).
         * The type must have been registered with bind_class<T>() first so that
         * method dispatch works when Plain calls the returned value. */
        template<typename T>
        Value wrap(std::shared_ptr<T> obj);

        /* Extract the C++ object from an PLAIN_TYPE_OBJECT Plain value.
         * The caller must supply the correct type T; no run-time type check. */
        template<typename T>
        T& as_object(const Value& v);

        /* Low-level access to the underlying C context — use sparingly. */
        PLAIN_CONTEXT* raw() { return &context; }

    private:
        PLAIN_CONTEXT                                                          context       = {};
        std::unordered_map<std::string, Handler>                               handlers;
        std::function<void(const std::string&)>                                error_callback;
        std::vector<std::unique_ptr<detail::ObjectEntry>>                     objects;
        std::unordered_map<std::type_index, std::shared_ptr<detail::DispatchTable>> dispatch_tables;

        void register_builtins();

        Value wrap_impl(std::shared_ptr<void> instance,
                        std::shared_ptr<detail::DispatchTable> dispatch);

        static void           error_delegate(void* ctx, const PLAIN_BYTE* data,
                                             PLAIN_WORD_DOUBLE length, PLAIN_WORD_DOUBLE type);
        static PLAIN_WORD_DOUBLE trampoline(void* raw, void* data,
                                            PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value);
        static PLAIN_WORD_DOUBLE object_method_handler(void* raw, void* data,
                                                        PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value);
    };

    /* -----------------------------------------------------------------------
     * Template implementations.
     * --------------------------------------------------------------------- */

    template<typename T>
    Runtime& Runtime::bind_class(
        const std::string& name,
        std::function<std::shared_ptr<T>(const Args&)> constructor,
        std::initializer_list<std::pair<std::string, ClassMethod<T>>> methods)
    {
        /* Build a dispatch table shared across all instances of T. */
        auto dispatch = std::make_shared<detail::DispatchTable>();
        for(const auto& entry : methods) {
            detail::DispatchFn fn = [handler = entry.second](void* ptr, const Args& a) -> Value {
                return handler(*static_cast<T*>(ptr), a);
            };
            dispatch->methods.emplace(entry.first, std::move(fn));
        }
        dispatch_tables[std::type_index(typeid(T))] = dispatch;

        /* Register constructor as a plain procedure. */
        bind(name, [this, constructor = std::move(constructor), dispatch](const Args& args) -> Value {
            auto instance = constructor(args);
            if(!instance) return Value{};
            return wrap_impl(std::static_pointer_cast<void>(instance), dispatch);
        });

        return *this;
    }

    template<typename T>
    Value Runtime::wrap(std::shared_ptr<T> obj) {
        auto it = dispatch_tables.find(std::type_index(typeid(T)));
        std::shared_ptr<detail::DispatchTable> dispatch;
        if(it != dispatch_tables.end()) dispatch = it->second;
        return wrap_impl(std::static_pointer_cast<void>(obj), dispatch);
    }

    template<typename T>
    T& Runtime::as_object(const Value& v) {
        auto* entry = reinterpret_cast<detail::ObjectEntry*>(v.native().data);
        return *static_cast<T*>(entry->instance.get());
    }

} /* namespace plain */
