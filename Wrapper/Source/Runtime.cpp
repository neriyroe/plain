/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     03/22/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Runtime — C++ wrapper implementation.
 *
 * Bridges the C runtime (PLAIN_CONTEXT) with a C++ API.  Provides:
 *   - Static callbacks (trampoline, object_method_handler, error_delegate)
 *     that the C runtime calls, which dispatch to C++ handlers/lambdas.
 *   - Public API methods (bind, run, call, invoke, get, set, on_error).
 *   - Object wrapping: register_object stores C++ objects as Plain values
 *     with method dispatch via a DispatchTable.
 */

#include <Plain/Runtime.hpp>
#include "Framework/List.hpp"

extern "C" {
    #include <Plain/Framework.h>
}

#include <cstdio>
#include <cstring>

namespace plain {

    /* -----------------------------------------------------------------------
     * Runtime — static callbacks (called by the C runtime)
     * --------------------------------------------------------------------- */

    /* Error reporting callback.  PLAIN_CONTEXT is layout-compatible such
     * that the environment field is first — casting from the environment
     * pointer gives access to user_data (which points to this Runtime). */
    void Runtime::error_delegate(void* context, const PLAIN_BYTE* data,
                                 PLAIN_WORD_DOUBLE length, PLAIN_WORD_DOUBLE) {
        auto* runtime = reinterpret_cast<Runtime*>(
            reinterpret_cast<PLAIN_CONTEXT*>(context)->user_data);
        if(runtime && runtime->error_callback && data) {
            runtime->error_callback(
                std::string((const char*)data, static_cast<size_t>(length)));
        }
    }

    /* Trampoline for procedures registered via bind().
     *
     * The C runtime calls this for every native callable.  It extracts the
     * keyword from the node, looks up the corresponding C++ Handler in the
     * handlers map, builds a plain::Arguments vector, and invokes it. */
    PLAIN_WORD_DOUBLE Runtime::trampoline(void* raw_context, void* data,
                                         PLAIN_WORD_DOUBLE, PLAIN_VALUE* value) {
        auto* context = reinterpret_cast<PLAIN_CONTEXT*>(raw_context);
        auto* runtime = reinterpret_cast<Runtime*>(context->user_data);
        auto* node    = reinterpret_cast<PLAIN_LIST*>(data);

        /* Extract the keyword text. */
        std::string keyword(
            reinterpret_cast<const char*>(node->keyword.from),
            static_cast<size_t>(node->keyword.to - node->keyword.from));

        auto iterator = runtime->handlers.find(keyword);
        if(iterator == runtime->handlers.end()) {
            return 0;
        }

        /* Build the arguments vector from the node's raw arguments. */
        Arguments arguments;
        PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
        arguments.reserve(arity);
        for(PLAIN_WORD_DOUBLE index = 0; index < arity; index++) {
            arguments.emplace_back(PLAIN_ARGUMENT(node, index));
        }

        /* Invoke the handler and copy the result. */
        Value result = iterator->second(arguments);
        if(value != nullptr && !result.is_nil()) {
            PLAIN_VALUE_COPY(value, &result.native());
        }
        return 0;
    }

    /* Method dispatch for C++ objects registered via bind_class<T>().
     *
     * Called by PLAIN_RESOLVE when a keyword resolves to a PLAIN_TYPE_OBJECT
     * that is not a Plain-native frame (no PLAIN_OWNER_USER flag).  Looks up
     * the method name (first argument) in the object's DispatchTable and
     * invokes the corresponding C++ function. */
    PLAIN_WORD_DOUBLE Runtime::object_method_handler(void* raw_context, void* data,
                                                     PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
        if(type != PLAIN_TYPE_OBJECT) {
            return 0;
        }

        auto* context = reinterpret_cast<PLAIN_CONTEXT*>(raw_context);
        auto* node    = reinterpret_cast<PLAIN_LIST*>(data);

        /* Find the object binding by keyword. */
        size_t keyword_length = static_cast<size_t>(node->keyword.to - node->keyword.from);
        std::string keyword((const char*)node->keyword.from, keyword_length);

        PLAIN_BINDING* binding = PLAIN_FRAME_FIND(
            context->frame,
            reinterpret_cast<const PLAIN_BYTE*>(keyword.c_str()));
        if(!binding || binding->value.type != PLAIN_TYPE_OBJECT || !binding->value.data) {
            return 0;
        }

        auto* entry = reinterpret_cast<detail::ObjectEntry*>(binding->value.data);
        if(!entry->dispatch) {
            return 0;
        }

        /* First argument is the method name. */
        if(PLAIN_ARITY(node) < 1) {
            return 0;
        }
        PLAIN_VALUE* method_argument = PLAIN_ARGUMENT(node, 0);
        if(!method_argument || !method_argument->data) {
            return 0;
        }
        if(method_argument->type != PLAIN_TYPE_STRING &&
           method_argument->type != PLAIN_TYPE_KEYWORD) {
            return 0;
        }

        /* Look up the method in the dispatch table. */
        std::string method_name((const char*)method_argument->data);
        auto method_iterator = entry->dispatch->methods.find(method_name);
        if(method_iterator == entry->dispatch->methods.end()) {
            return 0;
        }

        /* Build arguments (skipping the method name at index 0). */
        Arguments arguments;
        PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
        arguments.reserve(arity - 1);
        for(PLAIN_WORD_DOUBLE index = 1; index < arity; index++) {
            arguments.emplace_back(PLAIN_ARGUMENT(node, index));
        }

        /* Invoke the method and copy the result. */
        Value result = method_iterator->second(entry->instance.get(), arguments);
        if(value && !result.is_nil()) {
            PLAIN_VALUE_COPY(value, &result.native());
        }
        return 0;
    }

    /* -----------------------------------------------------------------------
     * Runtime — construction / destruction
     * --------------------------------------------------------------------- */

    Runtime::Runtime() {
        context.frame = PLAIN_FRAME_CREATE(nullptr);
        PLAIN_VERSION(&context.environment);
        context.tracker   = &error_delegate;
        context.handler   = &object_method_handler;
        context.user_data = this;

        /* Register the built-in framework (if, repeat, set, define, etc.). */
        PLAIN_FRAMEWORK_REGISTER(&context);

        /* Register built-in data structure classes. */
        framework::List::bind(*this);
    }

    Runtime::~Runtime() {
        if(context.frame) {
            PLAIN_FRAME_DESTROY(context.frame);
        }
    }

    /* -----------------------------------------------------------------------
     * Runtime — public API
     * --------------------------------------------------------------------- */

    /* Register a C++ lambda/function as a mutable Plain procedure. */
    Runtime& Runtime::bind(const std::string& name, Handler handler) {
        handlers[name] = std::move(handler);
        PLAIN_REGISTER(
            &context,
            reinterpret_cast<const PLAIN_BYTE*>(name.c_str()),
            &trampoline);
        return *this;
    }

    /* Evaluate a string of Plain source code. */
    Runtime& Runtime::run(const std::string& source) {
        PLAIN_EVALUATE(
            &context,
            &PLAIN_RESOLVE,
            reinterpret_cast<const PLAIN_BYTE*>(source.c_str()),
            context.tracker,
            nullptr);
        return *this;
    }

    /* Internal helper: builds a synthetic PLAIN_LIST node from the given
     * arguments and calls a callable directly.  Avoids going through source
     * string generation and re-parsing. */
    static Value perform_call(PLAIN_CONTEXT* context, const std::string& name,
                              PLAIN_CALLABLE* callable, const Arguments& arguments) {
        /* Convert Arguments to a raw PLAIN_VALUE array. */
        std::vector<PLAIN_VALUE> raw;
        raw.reserve(arguments.size());
        for(const auto& argument : arguments) {
            raw.push_back(argument.native());
        }

        /* Build a synthetic node with the keyword and argument layout. */
        PLAIN_LIST node = {};
        node.keyword.from = reinterpret_cast<PLAIN_BYTE*>(const_cast<char*>(name.c_str()));
        node.keyword.to   = node.keyword.from + name.size();
        if(!raw.empty()) {
            node.layout.from = reinterpret_cast<PLAIN_BYTE*>(raw.data());
            node.layout.to   = node.layout.from + raw.size() * sizeof(PLAIN_VALUE);
        }

        Value result;
        PLAIN_CALL(context, &node, callable, &result.native());
        return result;
    }

    /* Call a named Plain procedure from C++. */
    Value Runtime::call(const std::string& name, const Arguments& arguments) {
        PLAIN_BINDING* binding = PLAIN_FRAME_FIND(
            context.frame,
            reinterpret_cast<const PLAIN_BYTE*>(name.c_str()));
        if(!binding) {
            return Value{};
        }

        /* Check both callable slots: the binding's own callable, and a
         * callable stored as a value (from set fn, [define ...]). */
        PLAIN_CALLABLE* callable = binding->callable;
        if(!callable && binding->value.type == PLAIN_TYPE_CALLABLE && binding->value.data) {
            callable = reinterpret_cast<PLAIN_CALLABLE*>(binding->value.data);
        }
        if(!callable) {
            return Value{};
        }

        return perform_call(&context, name, callable, arguments);
    }

    /* Invoke a callable Value directly (e.g. a stored closure). */
    Value Runtime::invoke(const Value& callable_value, const Arguments& arguments) {
        if(callable_value.native().type != PLAIN_TYPE_CALLABLE || !callable_value.native().data) {
            return Value{};
        }
        auto* callable = reinterpret_cast<PLAIN_CALLABLE*>(callable_value.native().data);
        return perform_call(&context, "", callable, arguments);
    }

    /* Read the value of a Plain variable from the current frame. */
    Value Runtime::get(const std::string& name) const {
        PLAIN_BINDING* binding = PLAIN_FRAME_FIND(
            context.frame,
            reinterpret_cast<const PLAIN_BYTE*>(name.c_str()));
        if(!binding) {
            return Value{};
        }
        return Value(&binding->value);
    }

    /* Write a variable into the current frame. */
    Runtime& Runtime::set(const std::string& name, const Value& value) {
        PLAIN_FRAME_SET(
            context.frame,
            reinterpret_cast<const PLAIN_BYTE*>(name.c_str()),
            const_cast<PLAIN_VALUE*>(&value.native()),
            nullptr, 0);
        return *this;
    }

    /* Install an error callback. */
    Runtime& Runtime::on_error(std::function<void(const std::string&)> callback) {
        error_callback = std::move(callback);
        return *this;
    }

    /* -----------------------------------------------------------------------
     * Runtime — object wrapping
     * --------------------------------------------------------------------- */

    /* Wraps a C++ object (via shared_ptr) into a Plain value.
     *
     * The ObjectEntry is heap-allocated and stored in the Runtime's objects
     * vector so it lives as long as the Runtime.  The PLAIN_VALUE stores a
     * raw pointer to the ObjectEntry with length=0 and no PLAIN_OWNER_USER,
     * so PLAIN_VALUE_CLEAR/COPY treat it as a non-owning shallow copy. */
    Value Runtime::register_object(std::shared_ptr<void> instance,
                                   std::shared_ptr<detail::DispatchTable> dispatch) {
        auto owned = std::make_unique<detail::ObjectEntry>(
            detail::ObjectEntry{std::move(instance), std::move(dispatch)});
        auto* pointer = owned.get();
        objects.push_back(std::move(owned));

        Value result;
        result.native().data   = reinterpret_cast<PLAIN_BYTE*>(pointer);
        result.native().length = 0;
        result.native().owner  = 0;
        result.native().type   = PLAIN_TYPE_OBJECT;
        return result;
    }

} /* namespace plain */
