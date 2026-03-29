/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     2014-11-01.
 * Revision 2026-03-29.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Resolve — see Context.h for the public contracts of PLAIN_RESOLVE,
 * PLAIN_CALL, and PLAIN_REGISTER.
 */

#include <Plain/Runtime.h>

/* ================================================================== */
/*  Private helpers                                                   */
/* ================================================================== */

/* Copies the null-terminated keyword text from a node into <buffer>,
 * truncating to <size>-1 characters. */
PLAIN_INLINE void PLAIN_KEYWORD_EXTRACT(struct PLAIN_LIST* node, PLAIN_BYTE* buffer, PLAIN_WORD_DOUBLE size) {
    PLAIN_WORD_DOUBLE length = node->keyword.to - node->keyword.from;
    if(length >= size) {
        length = size - 1;
    }
    memcpy(buffer, node->keyword.from, length);
    buffer[length] = '\0';
}

/* Resolves a raw argument value for parameter binding.  Keywords are
 * looked up in the frame chain; all other types pass through unchanged.
 * Returns NIL for keywords not found in any frame. */
static struct PLAIN_VALUE PLAIN_FRAME_RESOLVE(struct PLAIN_CONTEXT* context, struct PLAIN_VALUE* raw_argument) {
    /* Non-keyword arguments are already resolved. */
    if(raw_argument->type != PLAIN_TYPE_KEYWORD) {
        return *raw_argument;
    }

    /* Look up the keyword name in the frame chain. */
    struct PLAIN_BINDING* keyword_binding = PLAIN_FRAME_FIND(context->frame, raw_argument->data);
    if(keyword_binding == NULL) {
        struct PLAIN_VALUE nil = {NULL, 0, PLAIN_TYPE_NIL, 0};
        return nil;
    }

    /* If the binding holds a callable (define'd function), return it
     * as a PLAIN_TYPE_CALLABLE value so it can be passed around. */
    if(keyword_binding->callable != NULL) {
        struct PLAIN_VALUE callable_value = {(PLAIN_BYTE*)keyword_binding->callable, 0, PLAIN_TYPE_CALLABLE, 0};
        return callable_value;
    }

    return keyword_binding->value;
}

/* ================================================================== */
/*  Callable dispatch                                                 */
/* ================================================================== */

PLAIN_WORD_DOUBLE PLAIN_CALL(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, struct PLAIN_CALLABLE* callable, struct PLAIN_VALUE* value) {
    /* Native callable — dispatch directly. */
    if(callable->native != NULL) {
        return callable->native((void*)context, (void*)node, PLAIN_TYPE_LIST, value);
    }

    /* User-defined callable — set up the call frame. */
    struct PLAIN_FRAME* frame = PLAIN_FRAME_CREATE(
        callable->closure != NULL ? callable->closure : context->frame);
    if(frame == NULL) {
        return PLAIN_ERROR_SYSTEM;
    }

    /* Bind each parameter to the resolved argument value. */
    for(PLAIN_WORD_DOUBLE i = 0; i < callable->parameter_count; i++) {
        struct PLAIN_VALUE* raw_argument = PLAIN_ARGUMENT(node, i);
        if(raw_argument != NULL) {
            struct PLAIN_VALUE argument_value = PLAIN_FRAME_RESOLVE(context, raw_argument);
            PLAIN_FRAME_BIND(frame, callable->parameters[i], &argument_value, NULL, 0);
        }
    }

    /* Push the call frame and walk the body. */
    struct PLAIN_FRAME* saved = context->frame;
    context->frame = frame;

    struct PLAIN_VALUE result = {NULL, 0, PLAIN_TYPE_NIL, 0};
    struct PLAIN_LIST* body_copy = PLAIN_LIST_COPY(callable->body);
    PLAIN_WORD_DOUBLE error = PLAIN_ERROR_SYSTEM;
    if(body_copy != NULL) {
        error = PLAIN_WALK(context, PLAIN_RESOLVE, body_copy, &result);
        PLAIN_UNLINK(body_copy);
        PLAIN_RESIZE(body_copy, 0, sizeof(struct PLAIN_LIST));
    }

    /* Pop the call frame. */
    context->frame = saved;
    PLAIN_FRAME_RELEASE(frame);

    /* Intercept PLAIN_SIGNAL_RETURN — yield the returned value. */
    if(error == PLAIN_SIGNAL_RETURN) {
        if(value != NULL) {
            *value = result;
        } else {
            PLAIN_VALUE_CLEAR(&result);
        }
        return 0;
    }

    if(error == 0 && value != NULL) {
        *value = result;
    } else {
        PLAIN_VALUE_CLEAR(&result);
    }
    return error;
}

/* Like PLAIN_CALL, but binds parameters starting from argument <offset>.
 * Used for method dispatch on objects where argument 0 is the method
 * name keyword, not a parameter value. */
static PLAIN_WORD_DOUBLE PLAIN_CALL_OFFSET(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, struct PLAIN_CALLABLE* callable, struct PLAIN_VALUE* value, PLAIN_WORD_DOUBLE offset) {
    /* Native callable — pass the full node (native handles offset itself). */
    if(callable->native != NULL) {
        return callable->native((void*)context, (void*)node, PLAIN_TYPE_LIST, value);
    }

    /* User-defined callable — bind parameters from argument[offset] onward. */
    struct PLAIN_FRAME* frame = PLAIN_FRAME_CREATE(
        callable->closure != NULL ? callable->closure : context->frame);
    if(frame == NULL) {
        return PLAIN_ERROR_SYSTEM;
    }

    for(PLAIN_WORD_DOUBLE i = 0; i < callable->parameter_count; i++) {
        struct PLAIN_VALUE* raw_argument = PLAIN_ARGUMENT(node, offset + i);
        if(raw_argument != NULL) {
            struct PLAIN_VALUE argument_value = PLAIN_FRAME_RESOLVE(context, raw_argument);
            PLAIN_FRAME_BIND(frame, callable->parameters[i], &argument_value, NULL, 0);
        }
    }

    struct PLAIN_FRAME* saved = context->frame;
    context->frame = frame;

    struct PLAIN_VALUE result = {NULL, 0, PLAIN_TYPE_NIL, 0};
    struct PLAIN_LIST* body_copy = PLAIN_LIST_COPY(callable->body);
    PLAIN_WORD_DOUBLE error = PLAIN_ERROR_SYSTEM;
    if(body_copy != NULL) {
        error = PLAIN_WALK(context, PLAIN_RESOLVE, body_copy, &result);
        PLAIN_UNLINK(body_copy);
        PLAIN_RESIZE(body_copy, 0, sizeof(struct PLAIN_LIST));
    }

    context->frame = saved;
    PLAIN_FRAME_RELEASE(frame);

    if(error == PLAIN_SIGNAL_RETURN) {
        if(value != NULL) {
            *value = result;
        } else {
            PLAIN_VALUE_CLEAR(&result);
        }
        return 0;
    }

    if(error == 0 && value != NULL) {
        *value = result;
    } else {
        PLAIN_VALUE_CLEAR(&result);
    }
    return error;
}

/* ================================================================== */
/*  Host extension                                                    */
/* ================================================================== */

PLAIN_WORD_DOUBLE PLAIN_REGISTER(struct PLAIN_CONTEXT* context, const PLAIN_BYTE* name, PLAIN_SUBROUTINE native) {
    struct PLAIN_CALLABLE* callable = (struct PLAIN_CALLABLE*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_CALLABLE), 0);
    if(callable == NULL) {
        return PLAIN_ERROR_SYSTEM;
    }
    memset(callable, 0, sizeof(struct PLAIN_CALLABLE));
    callable->native = native;
    return PLAIN_FRAME_BIND(context->frame, name, NULL, callable, 0);
}

/* ================================================================== */
/*  Main resolver                                                     */
/* ================================================================== */

PLAIN_WORD_DOUBLE PLAIN_RESOLVE(struct PLAIN_CONTEXT* context, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    /* Pass keywords through — they are not statements to dispatch. */
    if(type == PLAIN_TYPE_KEYWORD) {
        return 0;
    }
    /* Only dispatch list nodes (statements/expressions). */
    if(type != PLAIN_TYPE_LIST) {
        return 0;
    }

    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;

    /* Empty keyword — nothing to dispatch. */
    if(node->keyword.from == node->keyword.to) {
        return 0;
    }

    /* Extract the keyword text into a null-terminated buffer. */
    PLAIN_WORD_DOUBLE keyword_length = node->keyword.to - node->keyword.from;
    PLAIN_BYTE* keyword = (PLAIN_BYTE*)PLAIN_AUTO(keyword_length + 1);
    PLAIN_KEYWORD_EXTRACT(node, keyword, keyword_length + 1);

    /* Look up the keyword in the frame chain. */
    struct PLAIN_BINDING* binding = PLAIN_FRAME_FIND(context->frame, keyword);

    if(binding != NULL) {
        /* Dispatch to a registered callable (define, set, if, etc.). */
        if(binding->callable != NULL) {
            return PLAIN_CALL(context, node, binding->callable, value);
        }

        /* Dispatch to a callable stored as a value (set fn, [define ...]). */
        if(binding->value.type == PLAIN_TYPE_CALLABLE && binding->value.data != NULL) {
            return PLAIN_CALL(context, node, (struct PLAIN_CALLABLE*)binding->value.data, value);
        }

        /* Object dispatch — the keyword resolved to an object value. */
        if(binding->value.type == PLAIN_TYPE_OBJECT) {

            /* Plain-native object: the data pointer is a PLAIN_FRAME*. */
            if((binding->value.owner & PLAIN_OWNER_USER) && binding->value.data != NULL) {
                struct PLAIN_FRAME* instance = (struct PLAIN_FRAME*)binding->value.data;
                PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);

                /* No arguments — return the object value itself. */
                if(arity == 0) {
                    if(value != NULL) {
                        PLAIN_VALUE_COPY(value, &binding->value);
                    }
                    return 0;
                }

                /* First argument is the method name (or callable). */
                struct PLAIN_VALUE* method_argument = PLAIN_ARGUMENT(node, 0);
                if(method_argument == NULL) {
                    return 0;
                }

                /* Edge case: pre-resolved callable passed as the method. */
                if(method_argument->type == PLAIN_TYPE_CALLABLE && method_argument->data != NULL) {
                    return PLAIN_CALL_OFFSET(context, node, (struct PLAIN_CALLABLE*)method_argument->data, value, 1);
                }

                /* Extract the method name from a keyword or string argument. */
                const PLAIN_BYTE* method_name = NULL;
                if(method_argument->type == PLAIN_TYPE_KEYWORD || method_argument->type == PLAIN_TYPE_STRING) {
                    method_name = method_argument->data;
                }
                if(method_name == NULL) {
                    return 0;
                }

                /* Look up the method in the object's frame. */
                struct PLAIN_BINDING* member = PLAIN_FRAME_FIND(instance, method_name);
                if(member == NULL) {
                    return 0;
                }

                /* Dispatch to the method callable, skipping the method name argument. */
                if(member->callable != NULL) {
                    return PLAIN_CALL_OFFSET(context, node, member->callable, value, 1);
                }

                /* Not a callable — return the field value. */
                if(value != NULL) {
                    PLAIN_VALUE_COPY(value, &member->value);
                }
                return 0;
            }

            /* C++ managed object — dispatch via the host handler. */
            if(context->handler != NULL) {
                return context->handler((void*)context, data, PLAIN_TYPE_OBJECT, value);
            }
        }
    }

    /* Host-provided extension — last resort for unknown keywords. */
    if(context->handler != NULL) {
        return context->handler((void*)context, data, type, value);
    }

    return 0;
}
