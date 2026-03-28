/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     03/22/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 */

#include <Plain/Runtime.hpp>
#include <Plain/Framework.h>

#include <cstdio>
#include <cstring>

namespace plain {

    /* -----------------------------------------------------------------------
     * Value
     * --------------------------------------------------------------------- */

    void Value::clear() {
        PLAIN_VALUE_CLEAR(&value);
    }

    void Value::copy_from(const PLAIN_VALUE* src) {
        if(src == nullptr) { value = {}; return; }
        PLAIN_VALUE_COPY(&value, src);
    }

    Value::Value()                       { value = {}; }
    Value::Value(const Value& o)         { copy_from(&o.value); }
    Value::Value(Value&& o) noexcept     { value = o.value; o.value = {}; }
    Value::~Value()                      { clear(); }

    Value::Value(bool b) {
        value = {};
        PLAIN_EXPORT(b ? (PLAIN_BYTE*)0xFF : nullptr, 0, PLAIN_TYPE_BOOLEAN, &value);
    }

    Value::Value(int n) {
        value = {};
        PLAIN_WORD_DOUBLE storage = static_cast<PLAIN_WORD_DOUBLE>(n);
        PLAIN_EXPORT((PLAIN_BYTE*)&storage, sizeof(PLAIN_WORD_DOUBLE), PLAIN_TYPE_INTEGER, &value);
    }

    Value::Value(double d) {
        value = {};
        PLAIN_REAL storage = static_cast<PLAIN_REAL>(d);
        PLAIN_EXPORT((PLAIN_BYTE*)&storage, sizeof(PLAIN_REAL), PLAIN_TYPE_REAL, &value);
    }

    Value::Value(const std::string& s) {
        value = {};
        PLAIN_EXPORT((PLAIN_BYTE*)s.c_str(), static_cast<PLAIN_WORD_DOUBLE>(s.size() + 1),
                     PLAIN_TYPE_STRING, &value);
    }

    Value::Value(const char* s) {
        value = {};
        if(s == nullptr) return;
        PLAIN_EXPORT((PLAIN_BYTE*)s, static_cast<PLAIN_WORD_DOUBLE>(strlen(s) + 1),
                     PLAIN_TYPE_STRING, &value);
    }

    Value::Value(const PLAIN_VALUE* src) {
        value = {};
        copy_from(src);
    }

    Value& Value::operator=(const Value& o) {
        if(this != &o) { clear(); copy_from(&o.value); }
        return *this;
    }

    Value& Value::operator=(Value&& o) noexcept {
        if(this != &o) { clear(); value = o.value; o.value = {}; }
        return *this;
    }

    bool Value::is_nil()    const { return value.type == PLAIN_TYPE_NIL; }
    bool Value::is_true()   const { return PLAIN_IS_TRUE(&value) != 0; }
    bool Value::is_object() const { return value.type == PLAIN_TYPE_OBJECT; }

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

    bool Value::as_boolean() const {
        return PLAIN_IS_TRUE(&value) != 0;
    }

    int Value::as_integer() const {
        if(value.type == PLAIN_TYPE_INTEGER && value.data)
            return static_cast<int>(*(PLAIN_WORD_DOUBLE*)value.data);
        if(value.type == PLAIN_TYPE_REAL && value.data)
            return static_cast<int>(*(PLAIN_REAL*)value.data);
        if(value.type == PLAIN_TYPE_BOOLEAN)
            return value.data != nullptr ? 1 : 0;
        if(value.type == PLAIN_TYPE_STRING && value.data)
            return static_cast<int>(atoi((const char*)value.data));
        return 0;
    }

    double Value::as_real() const {
        if(value.type == PLAIN_TYPE_REAL && value.data)
            return static_cast<double>(*(PLAIN_REAL*)value.data);
        if(value.type == PLAIN_TYPE_INTEGER && value.data)
            return static_cast<double>(static_cast<int>(*(PLAIN_WORD_DOUBLE*)value.data));
        if(value.type == PLAIN_TYPE_BOOLEAN)
            return value.data != nullptr ? 1.0 : 0.0;
        if(value.type == PLAIN_TYPE_STRING && value.data)
            return atof((const char*)value.data);
        return 0.0;
    }

    std::string Value::as_string() const {
        char buf[64];
        switch(value.type) {
            case PLAIN_TYPE_STRING:
            case PLAIN_TYPE_KEYWORD:
                return value.data ? std::string((const char*)value.data) : "";
            case PLAIN_TYPE_INTEGER:
                if(value.data) {
                    snprintf(buf, sizeof(buf), "%d", static_cast<int>(*(PLAIN_WORD_DOUBLE*)value.data));
                    return buf;
                }
                return "0";
            case PLAIN_TYPE_REAL:
                if(value.data) {
                    snprintf(buf, sizeof(buf), "%g", static_cast<double>(*(PLAIN_REAL*)value.data));
                    return buf;
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

    /* -----------------------------------------------------------------------
     * Runtime — static helpers
     * --------------------------------------------------------------------- */

    /* ctx is PLAIN_ENVIRONMENT*, which is the first member of PLAIN_CONTEXT,
     * so casting to PLAIN_CONTEXT* gives access to user_data. */
    void Runtime::error_delegate(void* ctx, const PLAIN_BYTE* data,
                                  PLAIN_WORD_DOUBLE length, PLAIN_WORD_DOUBLE) {
        auto* self = reinterpret_cast<Runtime*>(
            reinterpret_cast<PLAIN_CONTEXT*>(ctx)->user_data);
        if(self && self->error_callback && data) {
            self->error_callback(std::string((const char*)data, static_cast<size_t>(length)));
        }
    }

    /* Called for every procedure that was bound with bind().
     * Extracts the keyword from the node, looks up the handler, builds args. */
    PLAIN_WORD_DOUBLE Runtime::trampoline(void* raw, void* data,
                                           PLAIN_WORD_DOUBLE, PLAIN_VALUE* value) {
        auto* context = reinterpret_cast<PLAIN_CONTEXT*>(raw);
        auto* self    = reinterpret_cast<Runtime*>(context->user_data);
        auto* node    = reinterpret_cast<PLAIN_LIST*>(data);

        std::string name(reinterpret_cast<const char*>(node->keyword.from),
                         static_cast<size_t>(node->keyword.to - node->keyword.from));

        auto it = self->handlers.find(name);
        if(it == self->handlers.end()) return 0;

        Args args;
        PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
        args.reserve(arity);
        for(PLAIN_WORD_DOUBLE i = 0; i < arity; i++) {
            args.emplace_back(PLAIN_ARGUMENT(node, i));
        }

        Value result = it->second(args);
        if(value != nullptr && !result.is_nil()) {
            PLAIN_VALUE_COPY(value, &result.native());
        }
        return 0;
    }

    /* Called by PLAIN_RESOLVE when a keyword resolves to a PLAIN_TYPE_OBJECT value.
     * Dispatches to the method named by the first string argument.
     * Also serves as a fallback for unknown keywords (type == PLAIN_TYPE_LIST);
     * in that case it does nothing and returns 0. */
    PLAIN_WORD_DOUBLE Runtime::object_method_handler(void* raw, void* data,
                                                      PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
        if(type != PLAIN_TYPE_OBJECT) return 0;  /* unrecognised keyword — ignore */

        auto* context = reinterpret_cast<PLAIN_CONTEXT*>(raw);
        auto* self    = reinterpret_cast<Runtime*>(context->user_data);
        auto* node    = reinterpret_cast<PLAIN_LIST*>(data);

        /* Resolve the object from the frame using the node keyword. */
        size_t kw_len = static_cast<size_t>(node->keyword.to - node->keyword.from);
        std::string kw((const char*)node->keyword.from, kw_len);

        PLAIN_BINDING* binding = PLAIN_FRAME_FIND(context->frame,
                                                   reinterpret_cast<const PLAIN_BYTE*>(kw.c_str()));
        if(!binding || binding->value.type != PLAIN_TYPE_OBJECT || !binding->value.data) return 0;

        auto* entry = reinterpret_cast<detail::ObjectEntry*>(binding->value.data);
        if(!entry->dispatch) return 0;

        /* First argument must be the method name (string). */
        if(PLAIN_ARITY(node) < 1) return 0;
        PLAIN_VALUE* method_arg = PLAIN_ARGUMENT(node, 0);
        if(!method_arg || !method_arg->data) return 0;
        if(method_arg->type != PLAIN_TYPE_STRING && method_arg->type != PLAIN_TYPE_KEYWORD) return 0;

        std::string method_name((const char*)method_arg->data);
        auto mit = entry->dispatch->methods.find(method_name);
        if(mit == entry->dispatch->methods.end()) return 0;

        /* Remaining arguments are passed to the method. */
        Args args;
        PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
        args.reserve(arity - 1);
        for(PLAIN_WORD_DOUBLE i = 1; i < arity; i++) {
            args.emplace_back(PLAIN_ARGUMENT(node, i));
        }

        Value result = mit->second(entry->instance.get(), args);
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
        PLAIN_FRAMEWORK_REGISTER(&context);
        register_builtins();
    }

    void Runtime::register_builtins() {
        bind_class<detail::List>("list",
            [](const Args& args) {
                auto l = std::make_shared<detail::List>();
                l->items.reserve(args.size());
                for(const auto& a : args) l->items.push_back(a);
                return l;
            },
            {
                { "get", [](detail::List& l, const Args& a) -> Value {
                    if(a.empty()) return Value{};
                    int idx = a[0].as_integer();
                    if(idx < 0 || idx >= static_cast<int>(l.items.size())) return Value{};
                    return l.items[idx];
                }},
                { "set", [](detail::List& l, const Args& a) -> Value {
                    if(a.size() < 2) return Value{};
                    int idx = a[0].as_integer();
                    if(idx < 0 || idx >= static_cast<int>(l.items.size())) return Value{};
                    l.items[idx] = a[1];
                    return Value{};
                }},
                { "length", [](detail::List& l, const Args&) -> Value {
                    return Value(static_cast<int>(l.items.size()));
                }},
                { "append", [](detail::List& l, const Args& a) -> Value {
                    for(const auto& v : a) l.items.push_back(v);
                    return Value{};
                }},
                { "remove", [](detail::List& l, const Args& a) -> Value {
                    if(a.empty()) return Value{};
                    int idx = a[0].as_integer();
                    if(idx < 0 || idx >= static_cast<int>(l.items.size())) return Value{};
                    l.items.erase(l.items.begin() + idx);
                    return Value{};
                }}
            }
        );
    }

    Runtime::~Runtime() {
        if(context.frame) PLAIN_FRAME_DESTROY(context.frame);
    }

    /* -----------------------------------------------------------------------
     * Runtime — public API
     * --------------------------------------------------------------------- */

    Runtime& Runtime::bind(const std::string& name, Handler handler) {
        handlers[name] = std::move(handler);
        PLAIN_CONTEXT_REGISTER(&context,
            reinterpret_cast<const PLAIN_BYTE*>(name.c_str()), &trampoline);
        return *this;
    }

    Runtime& Runtime::run(const std::string& source) {
        PLAIN_EVALUATE(&context.environment, &PLAIN_RESOLVE,
                       reinterpret_cast<const PLAIN_BYTE*>(source.c_str()),
                       context.tracker, nullptr);
        return *this;
    }

    /* Build a synthetic PLAIN_LIST node on the stack and call PLAIN_CALL directly.
     * This avoids source-string generation and handles any value type as an argument. */
    static Value call_impl(PLAIN_CONTEXT* ctx, const std::string& name,
                           PLAIN_CALLABLE* callable, const Args& args) {
        std::vector<PLAIN_VALUE> raw;
        raw.reserve(args.size());
        for(const auto& a : args) raw.push_back(a.native());

        PLAIN_LIST node = {};
        node.keyword.from = reinterpret_cast<PLAIN_BYTE*>(const_cast<char*>(name.c_str()));
        node.keyword.to   = node.keyword.from + name.size();
        if(!raw.empty()) {
            node.layout.from = reinterpret_cast<PLAIN_BYTE*>(raw.data());
            node.layout.to   = node.layout.from + raw.size() * sizeof(PLAIN_VALUE);
        }

        Value result;
        PLAIN_CALL(ctx, &node, callable, &result.native());
        return result;
    }

    Value Runtime::call(const std::string& name, const Args& args) {
        PLAIN_BINDING* binding = PLAIN_FRAME_FIND(context.frame,
            reinterpret_cast<const PLAIN_BYTE*>(name.c_str()));
        if(!binding) return Value{};

        PLAIN_CALLABLE* callable = binding->callable;
        if(!callable && binding->value.type == PLAIN_TYPE_CALLABLE && binding->value.data)
            callable = reinterpret_cast<PLAIN_CALLABLE*>(binding->value.data);
        if(!callable) return Value{};

        return call_impl(&context, name, callable, args);
    }

    Value Runtime::invoke(const Value& callable_val, const Args& args) {
        if(callable_val.native().type != PLAIN_TYPE_CALLABLE || !callable_val.native().data)
            return Value{};
        auto* callable = reinterpret_cast<PLAIN_CALLABLE*>(callable_val.native().data);
        return call_impl(&context, "", callable, args);
    }

    Value Runtime::get(const std::string& name) const {
        PLAIN_BINDING* binding = PLAIN_FRAME_FIND(context.frame,
            reinterpret_cast<const PLAIN_BYTE*>(name.c_str()));
        if(!binding) return Value{};
        return Value(&binding->value);
    }

    Runtime& Runtime::set(const std::string& name, const Value& value) {
        PLAIN_FRAME_SET(context.frame,
            reinterpret_cast<const PLAIN_BYTE*>(name.c_str()),
            const_cast<PLAIN_VALUE*>(&value.native()), nullptr, 0);
        return *this;
    }

    Runtime& Runtime::on_error(std::function<void(const std::string&)> callback) {
        error_callback = std::move(callback);
        return *this;
    }

    /* -----------------------------------------------------------------------
     * Runtime — object wrapping
     * --------------------------------------------------------------------- */

    Value Runtime::wrap_impl(std::shared_ptr<void> instance,
                             std::shared_ptr<detail::DispatchTable> dispatch) {
        /* Allocate a stable ObjectEntry on the heap; the vector holds unique_ptrs
         * so the pointer is valid for the lifetime of the Runtime regardless of
         * how many objects are registered. */
        auto owned = std::make_unique<detail::ObjectEntry>(
            detail::ObjectEntry{std::move(instance), std::move(dispatch)});
        auto* ptr = owned.get();
        objects.push_back(std::move(owned));

        /* Store the raw pointer in data. length = 0 and owner = 0 (no PLAIN_OWNER_USER)
         * so PLAIN_VALUE_CLEAR never frees it and PLAIN_VALUE_COPY only copies the pointer —
         * the Runtime owns the object lifetime, not individual Plain values. */
        Value result;
        result.native().data   = reinterpret_cast<PLAIN_BYTE*>(ptr);
        result.native().length = 0;
        result.native().owner  = 0;
        result.native().type   = PLAIN_TYPE_OBJECT;
        return result;
    }

} /* namespace plain */
