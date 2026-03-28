/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     11/01/2014.
 * Revision 03/28/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 */

#include <Plain/Runtime.h>
#include <Plain/Runtime/Scope.h>

/* ------------------------------------------------------------------ */
/*  Private helpers                                                   */
/* ------------------------------------------------------------------ */

PLAIN_INLINE void PLAIN_KEYWORD_EXTRACT(struct PLAIN_LIST* node, PLAIN_BYTE* buffer, PLAIN_WORD_DOUBLE size) {
    PLAIN_WORD_DOUBLE length = node->keyword.to - node->keyword.from;
    if(length >= size) length = size - 1;
    memcpy(buffer, node->keyword.from, length);
    buffer[length] = '\0';
}

/* Resolves a raw argument (possibly a keyword) to its value for parameter binding.
 * Returns NIL for keywords not found in the frame. */
static struct PLAIN_VALUE PLAIN_RESOLVE_FOR_BINDING(struct PLAIN_CONTEXT* context, struct PLAIN_VALUE* raw_argument) {
    if(raw_argument->type != PLAIN_TYPE_KEYWORD) return *raw_argument;
    struct PLAIN_BINDING* keyword_binding = PLAIN_FRAME_FIND(context->frame, raw_argument->data);
    if(keyword_binding == NULL) {
        struct PLAIN_VALUE nil = {NULL, 0, PLAIN_TYPE_NIL, 0};
        return nil;
    }
    if(keyword_binding->callable != NULL) {
        struct PLAIN_VALUE callable_value = {(PLAIN_BYTE*)keyword_binding->callable, 0, PLAIN_TYPE_CALLABLE, 0};
        return callable_value;
    }
    return keyword_binding->value;
}

/* ------------------------------------------------------------------ */
/*  Callable dispatch                                                 */
/* ------------------------------------------------------------------ */

PLAIN_WORD_DOUBLE PLAIN_CALL(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, struct PLAIN_CALLABLE* callable, struct PLAIN_VALUE* value) {
    /* Native callable — call it directly. Keyword arguments are unresolved;
     * each native callable resolves the arguments it needs via PLAIN_ARGUMENT_VALUE. */
    if(callable->native != NULL) {
        return callable->native((void*)context, (void*)node, PLAIN_TYPE_LIST, value);
    }
    /* User-defined callable — resolve keyword arguments, bind parameters, evaluate body.
     * The child frame parents the closure frame (lexical scope). */
    struct PLAIN_FRAME* frame = PLAIN_FRAME_CREATE(callable->closure != NULL ? callable->closure : context->frame);
    if(frame == NULL) return PLAIN_ERROR_SYSTEM;
    for(PLAIN_WORD_DOUBLE i = 0; i < callable->parameter_count; i++) {
        struct PLAIN_VALUE* raw_argument = PLAIN_ARGUMENT(node, i);
        if(raw_argument != NULL) {
            struct PLAIN_VALUE argument_value = PLAIN_RESOLVE_FOR_BINDING(context, raw_argument);
            PLAIN_FRAME_BIND(frame, callable->parameters[i], &argument_value, NULL, 0);
        }
    }
    struct PLAIN_FRAME* saved = context->frame;
    context->frame = frame;
    struct PLAIN_VALUE body = {NULL, 0, PLAIN_TYPE_NIL, 0};
    PLAIN_WORD_DOUBLE error = PLAIN_EVALUATE(&context->environment, &PLAIN_RESOLVE, callable->body, context->tracker, &body);
    context->frame = saved;

    PLAIN_FRAME_RELEASE(frame);
    if(error == PLAIN_SIGNAL_RETURN) {
        if(value != NULL) { *value = context->result; context->result = (struct PLAIN_VALUE){NULL, 0, PLAIN_TYPE_NIL, 0}; }
        else { PLAIN_VALUE_CLEAR(&context->result); }
        PLAIN_VALUE_CLEAR(&body);
        return 0;
    }
    if(error == 0 && value != NULL) { *value = body; }
    else { PLAIN_VALUE_CLEAR(&body); }
    return error;
}

/* Like PLAIN_CALL, but binds parameters starting from PLAIN_ARGUMENT(node, offset).
 * Used for method dispatch where arg0 is the method name, not a parameter. */
static PLAIN_WORD_DOUBLE PLAIN_CALL_OFFSET(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, struct PLAIN_CALLABLE* callable, struct PLAIN_VALUE* value, PLAIN_WORD_DOUBLE offset) {
    if(callable->native != NULL) {
        return callable->native((void*)context, (void*)node, PLAIN_TYPE_LIST, value);
    }
    struct PLAIN_FRAME* frame = PLAIN_FRAME_CREATE(callable->closure != NULL ? callable->closure : context->frame);
    if(frame == NULL) return PLAIN_ERROR_SYSTEM;
    for(PLAIN_WORD_DOUBLE i = 0; i < callable->parameter_count; i++) {
        struct PLAIN_VALUE* raw_argument = PLAIN_ARGUMENT(node, offset + i);
        if(raw_argument != NULL) {
            struct PLAIN_VALUE argument_value = PLAIN_RESOLVE_FOR_BINDING(context, raw_argument);
            PLAIN_FRAME_BIND(frame, callable->parameters[i], &argument_value, NULL, 0);
        }
    }
    struct PLAIN_FRAME* saved = context->frame;
    context->frame = frame;
    struct PLAIN_VALUE body = {NULL, 0, PLAIN_TYPE_NIL, 0};
    PLAIN_WORD_DOUBLE error = PLAIN_EVALUATE(&context->environment, &PLAIN_RESOLVE, callable->body, context->tracker, &body);
    context->frame = saved;
    PLAIN_FRAME_RELEASE(frame);
    if(error == PLAIN_SIGNAL_RETURN) {
        if(value != NULL) { *value = context->result; context->result = (struct PLAIN_VALUE){NULL, 0, PLAIN_TYPE_NIL, 0}; }
        else { PLAIN_VALUE_CLEAR(&context->result); }
        PLAIN_VALUE_CLEAR(&body);
        return 0;
    }
    if(error == 0 && value != NULL) { *value = body; }
    else { PLAIN_VALUE_CLEAR(&body); }
    return error;
}

/* ------------------------------------------------------------------ */
/*  Host extension                                                    */
/* ------------------------------------------------------------------ */

PLAIN_WORD_DOUBLE PLAIN_CONTEXT_REGISTER(struct PLAIN_CONTEXT* context, const PLAIN_BYTE* name, PLAIN_SUBROUTINE native) {
    struct PLAIN_CALLABLE* callable = (struct PLAIN_CALLABLE*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_CALLABLE), 0);
    if(callable == NULL) return PLAIN_ERROR_SYSTEM;
    memset(callable, 0, sizeof(struct PLAIN_CALLABLE));
    callable->native = native;
    return PLAIN_FRAME_BIND(context->frame, name, NULL, callable, 0);
}

/* ------------------------------------------------------------------ */
/*  Main resolver                                                     */
/* ------------------------------------------------------------------ */

PLAIN_WORD_DOUBLE PLAIN_RESOLVE(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;

    /* Keyword arguments are intentionally left unresolved here. Each callable
     * receives them as PLAIN_TYPE_KEYWORD and decides whether to treat the text
     * as a name (e.g. set, define, class) or resolve it to a value via
     * PLAIN_ARGUMENT_VALUE. Sub-expressions [...] and interpolated strings are
     * fully resolved by the walker before dispatch reaches here. */
    if(type == PLAIN_TYPE_KEYWORD) return 0;
    if(type != PLAIN_TYPE_LIST) return 0;

    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    if(node->keyword.from == node->keyword.to) return 0;

    /* Look up the node keyword in the frame — built-ins and user-defined callables live here.
     * A PLAIN_TYPE_CALLABLE value (from set x, [define ...]) is also callable. */
    PLAIN_WORD_DOUBLE keyword_length = node->keyword.to - node->keyword.from;
    PLAIN_BYTE* keyword = (PLAIN_BYTE*)PLAIN_AUTO(keyword_length + 1);
    PLAIN_KEYWORD_EXTRACT(node, keyword, keyword_length + 1);
    struct PLAIN_BINDING* binding = PLAIN_FRAME_FIND(context->frame, keyword);
    if(binding != NULL) {
        if(binding->callable != NULL) {
            return PLAIN_CALL(context, node, binding->callable, value);
        }
        if(binding->value.type == PLAIN_TYPE_CALLABLE && binding->value.data != NULL) {
            return PLAIN_CALL(context, node, (struct PLAIN_CALLABLE*)binding->value.data, value);
        }
        /* Object dispatch. */
        if(binding->value.type == PLAIN_TYPE_OBJECT) {
            /* Plain-native instance — dispatch via frame lookup. */
            if((binding->value.owner & PLAIN_OWNER_USER) && binding->value.data != NULL) {
                struct PLAIN_FRAME* instance = (struct PLAIN_FRAME*)binding->value.data;
                PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
                if(arity == 0) {
                    if(value != NULL) PLAIN_VALUE_COPY(value, &binding->value);
                    return 0;
                }
                struct PLAIN_VALUE* method_arg = PLAIN_ARGUMENT(node, 0);
                if(method_arg == NULL) return 0;
                /* Allow an already-resolved callable as the method (edge case). */
                if(method_arg->type == PLAIN_TYPE_CALLABLE && method_arg->data != NULL) {
                    return PLAIN_CALL_OFFSET(context, node, (struct PLAIN_CALLABLE*)method_arg->data, value, 1);
                }
                const PLAIN_BYTE* method_name = NULL;
                if(method_arg->type == PLAIN_TYPE_KEYWORD || method_arg->type == PLAIN_TYPE_STRING)
                    method_name = method_arg->data;
                if(method_name == NULL) return 0;
                struct PLAIN_BINDING* member = PLAIN_FRAME_FIND(instance, method_name);
                if(member == NULL) return 0;
                if(member->callable != NULL)
                    return PLAIN_CALL_OFFSET(context, node, member->callable, value, 1);
                if(value != NULL) PLAIN_VALUE_COPY(value, &member->value);
                return 0;
            }
            /* C++ managed object — dispatch via context->handler. */
            if(context->handler != NULL) {
                return context->handler(raw, data, PLAIN_TYPE_OBJECT, value);
            }
        }
    }

    /* Host-provided extension. */
    if(context->handler != NULL) {
        return context->handler(raw, data, type, value);
    }

    return 0;
}
