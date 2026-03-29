/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     03/22/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 */

#include <Plain/Runtime.hpp>

extern "C" {
    #include <Plain/Framework.h>
}

#include <cstdio>
#include <cstring>

namespace plain {

    /* -----------------------------------------------------------------------
     * Runtime — static callbacks
     * --------------------------------------------------------------------- */

    /* context is PLAIN_ENVIRONMENT*, which is the first member of PLAIN_CONTEXT,
     * so casting to PLAIN_CONTEXT* gives access to user_data. */
    void Runtime::error_delegate(void* context, const PLAIN_BYTE* data,
                                 PLAIN_WORD_DOUBLE length, PLAIN_WORD_DOUBLE) {
        auto* runtime = reinterpret_cast<Runtime*>(
            reinterpret_cast<PLAIN_CONTEXT*>(context)->user_data);
        if(runtime && runtime->_error_callback && data) {
            runtime->_error_callback(
                std::string((const char*)data, static_cast<size_t>(length)));
        }
    }

    /* Called for every procedure that was bound with bind().
     * Extracts the keyword from the node, looks up the handler, builds arguments. */
    PLAIN_WORD_DOUBLE Runtime::trampoline(void* raw_context, void* data,
                                         PLAIN_WORD_DOUBLE, PLAIN_VALUE* value) {
        auto* context = reinterpret_cast<PLAIN_CONTEXT*>(raw_context);
        auto* runtime = reinterpret_cast<Runtime*>(context->user_data);
        auto* node    = reinterpret_cast<PLAIN_LIST*>(data);

        std::string keyword(reinterpret_cast<const char*>(node->keyword.from),
                            static_cast<size_t>(node->keyword.to - node->keyword.from));

        auto iterator = runtime->_handlers.find(keyword);
        if(iterator == runtime->_handlers.end()) return 0;

        Arguments arguments;
        PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
        arguments.reserve(arity);
        for(PLAIN_WORD_DOUBLE index = 0; index < arity; index++) {
            arguments.emplace_back(PLAIN_ARGUMENT(node, index));
        }

        Value result = iterator->second(arguments);
        if(value != nullptr && !result.is_nil()) {
            PLAIN_VALUE_COPY(value, &result.native());
        }
        return 0;
    }

    /* Called by PLAIN_RESOLVE when a keyword resolves to a PLAIN_TYPE_OBJECT value.
     * Dispatches to the method named by the node keyword.
     * Also serves as a fallback for unknown keywords (type == PLAIN_TYPE_LIST);
     * in that case it does nothing and returns 0. */
    PLAIN_WORD_DOUBLE Runtime::object_method_handler(void* raw_context, void* data,
                                                     PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
        if(type != PLAIN_TYPE_OBJECT) return 0;

        auto* context = reinterpret_cast<PLAIN_CONTEXT*>(raw_context);
        auto* node    = reinterpret_cast<PLAIN_LIST*>(data);

        size_t keyword_length = static_cast<size_t>(node->keyword.to - node->keyword.from);
        std::string keyword((const char*)node->keyword.from, keyword_length);

        PLAIN_BINDING* binding = PLAIN_FRAME_FIND(context->frame,
                                                  reinterpret_cast<const PLAIN_BYTE*>(keyword.c_str()));
        if(!binding || binding->value.type != PLAIN_TYPE_OBJECT || !binding->value.data) return 0;

        auto* entry = reinterpret_cast<detail::ObjectEntry*>(binding->value.data);
        if(!entry->dispatch) return 0;

        if(PLAIN_ARITY(node) < 1) return 0;
        PLAIN_VALUE* method_argument = PLAIN_ARGUMENT(node, 0);
        if(!method_argument || !method_argument->data) return 0;
        if(method_argument->type != PLAIN_TYPE_STRING &&
           method_argument->type != PLAIN_TYPE_KEYWORD) return 0;

        std::string method_name((const char*)method_argument->data);
        auto method_iterator = entry->dispatch->methods.find(method_name);
        if(method_iterator == entry->dispatch->methods.end()) return 0;

        Arguments arguments;
        PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
        arguments.reserve(arity - 1);
        for(PLAIN_WORD_DOUBLE index = 1; index < arity; index++) {
            arguments.emplace_back(PLAIN_ARGUMENT(node, index));
        }

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
        _context.frame = PLAIN_FRAME_CREATE(nullptr);
        PLAIN_VERSION(&_context.environment);
        _context.tracker   = &error_delegate;
        _context.handler   = &object_method_handler;
        _context.user_data = this;
        PLAIN_FRAMEWORK_REGISTER(&_context);
        register_builtins();
    }

    void Runtime::register_builtins() {
        bind_class<detail::List>("list",
            [](const Arguments& arguments) {
                auto list = std::make_shared<detail::List>();
                list->items.reserve(arguments.size());
                for(const auto& argument : arguments) list->items.push_back(argument);
                return list;
            },
            {
                { "get", [](detail::List& list, const Arguments& arguments) -> Value {
                    if(arguments.empty()) return Value{};
                    int index = arguments[0].as_integer();
                    if(index < 0 || index >= static_cast<int>(list.items.size())) return Value{};
                    return list.items[index];
                }},
                { "set", [](detail::List& list, const Arguments& arguments) -> Value {
                    if(arguments.size() < 2) return Value{};
                    int index = arguments[0].as_integer();
                    if(index < 0 || index >= static_cast<int>(list.items.size())) return Value{};
                    list.items[index] = arguments[1];
                    return Value{};
                }},
                { "length", [](detail::List& list, const Arguments&) -> Value {
                    return Value(static_cast<int>(list.items.size()));
                }},
                { "append", [](detail::List& list, const Arguments& arguments) -> Value {
                    for(const auto& value : arguments) list.items.push_back(value);
                    return Value{};
                }},
                { "remove", [](detail::List& list, const Arguments& arguments) -> Value {
                    if(arguments.empty()) return Value{};
                    int index = arguments[0].as_integer();
                    if(index < 0 || index >= static_cast<int>(list.items.size())) return Value{};
                    list.items.erase(list.items.begin() + index);
                    return Value{};
                }}
            }
        );
    }

    Runtime::~Runtime() {
        if(_context.frame) PLAIN_FRAME_DESTROY(_context.frame);
    }

    /* -----------------------------------------------------------------------
     * Runtime — public API
     * --------------------------------------------------------------------- */

    Runtime& Runtime::bind(const std::string& name, Handler handler) {
        _handlers[name] = std::move(handler);
        PLAIN_REGISTER(&_context,
            reinterpret_cast<const PLAIN_BYTE*>(name.c_str()), &trampoline);
        return *this;
    }

    Runtime& Runtime::run(const std::string& source) {
        PLAIN_EVALUATE(&_context, &PLAIN_RESOLVE,
                       reinterpret_cast<const PLAIN_BYTE*>(source.c_str()),
                       _context.tracker, nullptr);
        return *this;
    }

    /* Build a synthetic PLAIN_LIST node on the stack and call PLAIN_CALL directly.
     * This avoids source-string generation and handles any value type as an argument. */
    static Value call_impl(PLAIN_CONTEXT* context, const std::string& name,
                           PLAIN_CALLABLE* callable, const Arguments& arguments) {
        std::vector<PLAIN_VALUE> raw;
        raw.reserve(arguments.size());
        for(const auto& argument : arguments) raw.push_back(argument.native());

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

    Value Runtime::call(const std::string& name, const Arguments& arguments) {
        PLAIN_BINDING* binding = PLAIN_FRAME_FIND(_context.frame,
            reinterpret_cast<const PLAIN_BYTE*>(name.c_str()));
        if(!binding) return Value{};

        PLAIN_CALLABLE* callable = binding->callable;
        if(!callable && binding->value.type == PLAIN_TYPE_CALLABLE && binding->value.data)
            callable = reinterpret_cast<PLAIN_CALLABLE*>(binding->value.data);
        if(!callable) return Value{};

        return call_impl(&_context, name, callable, arguments);
    }

    Value Runtime::invoke(const Value& callable_value, const Arguments& arguments) {
        if(callable_value.native().type != PLAIN_TYPE_CALLABLE || !callable_value.native().data)
            return Value{};
        auto* callable = reinterpret_cast<PLAIN_CALLABLE*>(callable_value.native().data);
        return call_impl(&_context, "", callable, arguments);
    }

    Value Runtime::get(const std::string& name) const {
        PLAIN_BINDING* binding = PLAIN_FRAME_FIND(_context.frame,
            reinterpret_cast<const PLAIN_BYTE*>(name.c_str()));
        if(!binding) return Value{};
        return Value(&binding->value);
    }

    Runtime& Runtime::set(const std::string& name, const Value& value) {
        PLAIN_FRAME_SET(_context.frame,
            reinterpret_cast<const PLAIN_BYTE*>(name.c_str()),
            const_cast<PLAIN_VALUE*>(&value.native()), nullptr, 0);
        return *this;
    }

    Runtime& Runtime::on_error(std::function<void(const std::string&)> callback) {
        _error_callback = std::move(callback);
        return *this;
    }

    /* -----------------------------------------------------------------------
     * Runtime — object wrapping
     * --------------------------------------------------------------------- */

    Value Runtime::wrap_impl(std::shared_ptr<void> instance,
                             std::shared_ptr<detail::DispatchTable> dispatch) {
        /* Allocate a stable ObjectEntry on the heap; _objects holds unique_ptrs
         * so the pointer is valid for the lifetime of the Runtime regardless of
         * how many objects are registered. */
        auto owned = std::make_unique<detail::ObjectEntry>(
            detail::ObjectEntry{std::move(instance), std::move(dispatch)});
        auto* pointer = owned.get();
        _objects.push_back(std::move(owned));

        /* Store the raw pointer in data. length = 0 and owner = 0 (no PLAIN_OWNER_USER)
         * so PLAIN_VALUE_CLEAR never frees it and PLAIN_VALUE_COPY only copies the pointer —
         * the Runtime owns the object lifetime, not individual Plain values. */
        Value result;
        result.native().data   = reinterpret_cast<PLAIN_BYTE*>(pointer);
        result.native().length = 0;
        result.native().owner  = 0;
        result.native().type   = PLAIN_TYPE_OBJECT;
        return result;
    }

} /* namespace plain */
