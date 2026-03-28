/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     03/28/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Context — runtime state and the primary dispatch entry point.
 */

#pragma once

#include <Plain/Runtime/Frame.h>
#include <Plain/Runtime/Host/Environment.h>

/*
 * PLAIN_CONTEXT — the full runtime state passed through every evaluation.
 * Cast freely to PLAIN_ENVIRONMENT* since environment is the first member.
 */
struct PLAIN_CONTEXT {
    struct PLAIN_ENVIRONMENT environment; /* Must be first member. */
    struct PLAIN_FRAME* frame;
    struct PLAIN_VALUE result;   /* Return value written by the last `return` statement. */
    PLAIN_DELEGATE tracker;      /* Receives syntax errors and diagnostics. */
    PLAIN_SUBROUTINE handler;    /* Host fallback: called for keywords not found in the frame. */
    void* user_data;             /* Arbitrary host pointer; not touched by the library. */
};

/* Resolves argument <index> from <node> inside <context>:
 * keywords are looked up in the frame and their bound value is returned;
 * all other types are returned by copy as-is.
 * Returned values borrow data pointers — do not call PLAIN_VALUE_CLEAR on them. */
PLAIN_INLINE struct PLAIN_VALUE PLAIN_ARGUMENT_VALUE(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, PLAIN_WORD_DOUBLE index) {
    struct PLAIN_VALUE* argument = PLAIN_ARGUMENT(node, index);
    if(argument == NULL) {
        struct PLAIN_VALUE nil = {NULL, 0, PLAIN_TYPE_NIL, 0};
        return nil;
    }
    if(argument->type != PLAIN_TYPE_KEYWORD) return *argument;
    struct PLAIN_BINDING* binding = PLAIN_FRAME_FIND(context->frame, argument->data);
    if(binding == NULL) return *argument;
    if(binding->callable != NULL) {
        struct PLAIN_VALUE callable_value = {(PLAIN_BYTE*)binding->callable, 0, PLAIN_TYPE_CALLABLE, 0};
        return callable_value;
    }
    return binding->value;
}

/* Dispatches a fully-walked list node: looks up the keyword, calls the bound
 * callable (built-in or user-defined), or invokes context->handler if not found. */
PLAIN_WORD_DOUBLE PLAIN_RESOLVE(struct PLAIN_CONTEXT* context, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value);

/* Registers a single native procedure in the current frame of <context> as a
 * mutable callable. Use this to extend Plain from the host application.
 * The binding is mutable, so Plain code can override it at any time. */
PLAIN_WORD_DOUBLE PLAIN_REGISTER(struct PLAIN_CONTEXT* context, const PLAIN_BYTE* name, PLAIN_SUBROUTINE native);

/* Calls <callable> with arguments from <node>, writing the result into <value>.
 * Handles both native callables (via the native pointer) and user-defined ones
 * (by evaluating callable->body with parameters bound from node arguments).
 * This is the core dispatch used by PLAIN_RESOLVE and callable from C++. */
PLAIN_WORD_DOUBLE PLAIN_CALL(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, struct PLAIN_CALLABLE* callable, struct PLAIN_VALUE* value);
