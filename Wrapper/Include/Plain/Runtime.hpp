/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     2026-03-22.
 * Revision 2026-03-29.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Runtime — C++ interface to a Plain execution context.
 *
 *   plain::Runtime  wraps a Plain context.
 *   plain::Runtime::bind()        registers any C++ lambda as a Plain procedure.
 *   plain::Runtime::bind_class()  registers a C++ class so Plain can construct
 *                                 and call objects of that class.
 *   plain::Runtime::run()         evaluates Plain source code.
 *   plain::Runtime::call()        calls a Plain procedure from C++.
 */

#pragma once

#include <Plain/Value.hpp>

#include <cassert>
#include <functional>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace plain {

    /* -----------------------------------------------------------------------
     * detail — internal types used by the object bridge.
     * --------------------------------------------------------------------- */
    namespace detail {

        using DispatchFunction = std::function<Value(void*, const Arguments&)>;

        struct DispatchTable {
            std::unordered_map<std::string, DispatchFunction> methods;
        };

        struct ObjectEntry {
            std::shared_ptr<void>          instance;
            std::shared_ptr<DispatchTable> dispatch;
        };

        /* Automatic argument conversion: Value -> C++ type. */
        template<typename T>
        decltype(auto) convert_argument(const Value& value, Runtime& runtime) {
            using Bare = std::remove_cv_t<std::remove_reference_t<T>>;
            if constexpr (std::is_same_v<Bare, bool>)               return value.as_boolean();
            else if constexpr (std::is_integral_v<Bare>)            return static_cast<Bare>(value.as_integer());
            else if constexpr (std::is_floating_point_v<Bare>)      return static_cast<Bare>(value.as_real());
            else if constexpr (std::is_same_v<Bare, std::string>)   return value.as_string();
            else if constexpr (std::is_same_v<Bare, Value>)         return value;
            else                                                    return runtime.as_object<Bare>(value);
        }

        /* Automatic return conversion: C++ type -> Value. */
        template<typename T>
        Value convert_return(T&& result, Runtime& runtime) {
            using Bare = std::remove_cv_t<std::remove_reference_t<T>>;
            if constexpr (std::is_same_v<Bare, Value>)              return std::forward<T>(result);
            else if constexpr (std::is_same_v<Bare, bool>)          return Value(result);
            else if constexpr (std::is_integral_v<Bare>)            return Value(static_cast<long long>(result));
            else if constexpr (std::is_floating_point_v<Bare>)      return Value(static_cast<double>(result));
            else if constexpr (std::is_same_v<Bare, std::string>)   return Value(result);
            else if constexpr (std::is_same_v<Bare, const char*>)   return Value(result);
            else return runtime.wrap(std::make_shared<Bare>(std::forward<T>(result)));
        }

        inline Value convert_return_void() { return Value{}; }

    } /* namespace detail */

    /* -----------------------------------------------------------------------
     * Runtime — the main entry point.  Create one, bind procedures, run code.
     * --------------------------------------------------------------------- */
    class Runtime {
        template<typename T> friend class ClassBinding;

    public:
        Runtime();
        ~Runtime();

        Runtime(const Runtime&)            = delete;
        Runtime& operator=(const Runtime&) = delete;

        /* Register a C++ lambda/function as a mutable Plain procedure.
         * Plain code can read, override, or pass it as a first-class value. */
        Runtime& bind(const std::string& name, Handler handler);

        /* Register a C++ class — simplified builder API.
         * Returns a ClassBinding<T> for chaining .constructor<>() and .method().
         *
         *   runtime.bind_class<Vec2>("Vec2")
         *       .constructor<double, double>()
         *       .method("length", &Vec2::length)
         *       .method("add",    &Vec2::add);
         */
        template<typename T>
        ClassBinding<T> bind_class(const std::string& name);

        /* Register a C++ class — lambda API (full manual control).
         *
         *   constructor  — receives Plain arguments, returns a shared_ptr<T>.
         *   methods      — list of { "name", [](T& self, const Arguments&){...} } pairs.
         */
        template<typename T>
        Runtime& bind_class(
            const std::string& name,
            std::function<std::shared_ptr<T>(const Arguments&)> constructor,
            std::initializer_list<std::pair<std::string, ClassMethod<T>>> methods = {}
        );

        /* Evaluate a string of Plain source code. */
        Runtime& run(const std::string& source);

        /* Call a named Plain procedure from C++.
         * Looks up the binding by name and invokes it with the given arguments. */
        Value call(const std::string& name, const Arguments& arguments = {});

        /* Invoke any callable Value from C++ (e.g. a stored closure). */
        Value invoke(const Value& callable, const Arguments& arguments = {});

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
        Value wrap(std::shared_ptr<T> object);

        /* Extract the C++ object from a PLAIN_TYPE_OBJECT Plain value.
         * The caller must supply the correct type T; no run-time type check. */
        template<typename T>
        T& as_object(const Value& value);

        /* Low-level access to the underlying C context — use sparingly. */
        PLAIN_CONTEXT* raw() { return &context; }

    private:
        PLAIN_CONTEXT                                                               context = {};
        std::unordered_map<std::string, Handler>                                    handlers;
        std::function<void(const std::string&)>                                     error_callback;
        std::vector<std::unique_ptr<detail::ObjectEntry>>                           objects;
        std::unordered_map<std::type_index, std::shared_ptr<detail::DispatchTable>> dispatch_tables;

        Value register_object(std::shared_ptr<void> instance,
                        std::shared_ptr<detail::DispatchTable> dispatch);

        static void error_delegate(void* context, const PLAIN_BYTE* data, PLAIN_WORD_DOUBLE length, PLAIN_WORD_DOUBLE type);
        static PLAIN_WORD_DOUBLE trampoline(void* raw_context, void* data, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value);
        static PLAIN_WORD_DOUBLE object_method_handler(void* raw_context, void* data, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value);
    };

    /* -----------------------------------------------------------------------
     * Template implementations.
     * --------------------------------------------------------------------- */

    template<typename T>
    Runtime& Runtime::bind_class(
        const std::string& name,
        std::function<std::shared_ptr<T>(const Arguments&)> constructor,
        std::initializer_list<std::pair<std::string, ClassMethod<T>>> methods)
    {
        auto dispatch = std::make_shared<detail::DispatchTable>();
        for(const auto& entry : methods) {
            detail::DispatchFunction function = [handler = entry.second](void* pointer, const Arguments& arguments) -> Value {
                return handler(*static_cast<T*>(pointer), arguments);
            };
            dispatch->methods.emplace(entry.first, std::move(function));
        }
        dispatch_tables[std::type_index(typeid(T))] = dispatch;

        bind(name, [this, constructor = std::move(constructor), dispatch](const Arguments& arguments) -> Value {
            auto instance = constructor(arguments);
            if(!instance) return Value{};
            return register_object(std::static_pointer_cast<void>(instance), dispatch);
        });

        return *this;
    }

    template<typename T>
    Value Runtime::wrap(std::shared_ptr<T> object) {
        auto iterator = dispatch_tables.find(std::type_index(typeid(T)));
        std::shared_ptr<detail::DispatchTable> dispatch;
        if(iterator != dispatch_tables.end()) dispatch = iterator->second;
        return register_object(std::static_pointer_cast<void>(object), dispatch);
    }

    template<typename T>
    T& Runtime::as_object(const Value& value) {
        auto* entry = reinterpret_cast<detail::ObjectEntry*>(value.native().data);
        return *static_cast<T*>(entry->instance.get());
    }

    /* -----------------------------------------------------------------------
     * ClassBinding<T> — builder returned by Runtime::bind_class<T>(name).
     * Chain .constructor<Arguments...>() and .method("name", &T::fn) on it.
     * --------------------------------------------------------------------- */
    template<typename T>
    class ClassBinding {
    public:
        ClassBinding(Runtime* runtime, const std::string& name,
                     std::shared_ptr<detail::DispatchTable> dispatch)
            : runtime(runtime), name(name), dispatch(dispatch) {}

        /* Register the constructor with auto-converted arguments.
         *   .constructor<double, double>()
         * Plain: obj = [ClassName arg1, arg2]
         */
        template<typename... ConstructorArguments>
        ClassBinding& constructor() {
            return register_constructor<ConstructorArguments...>(
                std::index_sequence_for<ConstructorArguments...>{});
        }

        /* Bind a factory lambda as the constructor.
         *   .constructor([](const Arguments& args){ return std::make_shared<T>(...); })
         */
        ClassBinding& constructor(std::function<std::shared_ptr<T>(const Arguments&)> factory) {
            Runtime* rt = runtime;
            auto disp   = dispatch;
            rt->bind(name, [rt, disp, factory = std::move(factory)](const Arguments& arguments) -> Value {
                auto instance = factory(arguments);
                if(!instance) return Value{};
                return rt->register_object(std::static_pointer_cast<void>(instance), disp);
            });
            return *this;
        }

        /* Bind a const member function.
         *   .method("length", &Vec2::length)
         */
        template<typename ReturnType, typename... MethodArguments>
        ClassBinding& method(const std::string& method_name,
                             ReturnType(T::*member_function)(MethodArguments...) const) {
            return register_method(method_name, member_function,
                               std::index_sequence_for<MethodArguments...>{});
        }

        /* Bind a non-const member function. */
        template<typename ReturnType, typename... MethodArguments>
        ClassBinding& method(const std::string& method_name,
                             ReturnType(T::*member_function)(MethodArguments...)) {
            return register_method(method_name, member_function,
                               std::index_sequence_for<MethodArguments...>{});
        }

    private:
        Runtime*                               runtime;
        std::string                            name;
        std::shared_ptr<detail::DispatchTable> dispatch;

        template<typename... ConstructorArguments, size_t... Indices>
        ClassBinding& register_constructor(std::index_sequence<Indices...>) {
            Runtime* runtime = this->runtime;
            auto dispatch = this->dispatch;
            runtime->bind(this->name, [runtime, dispatch](const Arguments& arguments) -> Value {
                auto instance = std::make_shared<T>(
                    detail::convert_argument<ConstructorArguments>(
                        Indices < arguments.size() ? arguments[Indices] : Value{}, *runtime)...
                );
                return runtime->register_object(std::static_pointer_cast<void>(instance), dispatch);
            });
            return *this;
        }

        template<typename ReturnType, typename... MethodArguments, size_t... Indices>
        ClassBinding& register_method(const std::string& method_name,
                                  ReturnType(T::*member_function)(MethodArguments...) const,
                                  std::index_sequence<Indices...>) {
            Runtime* runtime = this->runtime;
            dispatch->methods[method_name] =
                [runtime, member_function](void* pointer, const Arguments& arguments) -> Value {
                    T& self = *static_cast<T*>(pointer);
                    if constexpr (std::is_void_v<ReturnType>) {
                        (self.*member_function)(detail::convert_argument<MethodArguments>(
                            Indices < arguments.size() ? arguments[Indices] : Value{}, *runtime)...);
                        return detail::convert_return_void();
                    } else {
                        return detail::convert_return(
                            (self.*member_function)(detail::convert_argument<MethodArguments>(
                                Indices < arguments.size() ? arguments[Indices] : Value{}, *runtime)...),
                            *runtime);
                    }
                };
            return *this;
        }

        template<typename ReturnType, typename... MethodArguments, size_t... Indices>
        ClassBinding& register_method(const std::string& method_name,
                                  ReturnType(T::*member_function)(MethodArguments...),
                                  std::index_sequence<Indices...>) {
            Runtime* runtime = this->runtime;
            dispatch->methods[method_name] =
                [runtime, member_function](void* pointer, const Arguments& arguments) -> Value {
                    T& self = *static_cast<T*>(pointer);
                    if constexpr (std::is_void_v<ReturnType>) {
                        (self.*member_function)(detail::convert_argument<MethodArguments>(
                            Indices < arguments.size() ? arguments[Indices] : Value{}, *runtime)...);
                        return detail::convert_return_void();
                    } else {
                        return detail::convert_return(
                            (self.*member_function)(detail::convert_argument<MethodArguments>(
                                Indices < arguments.size() ? arguments[Indices] : Value{}, *runtime)...),
                            *runtime);
                    }
                };
            return *this;
        }
    };

    template<typename T>
    ClassBinding<T> Runtime::bind_class(const std::string& name) {
        auto dispatch = std::make_shared<detail::DispatchTable>();
        dispatch_tables[std::type_index(typeid(T))] = dispatch;
        return ClassBinding<T>(this, name, dispatch);
    }

} /* namespace plain */
