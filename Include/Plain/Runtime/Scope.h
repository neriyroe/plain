/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     03/22/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Framework — variable frames, callable definitions, and the standard
 * built-in command set (PLAIN_RESOLVE). Include via <Plain/VM.h>.
 */

#pragma once

#include <uthash.h>
#include <Plain/Plain.h>
#include <Plain/Runtime/Host/Environment.h>

/* Control-flow signals, returned instead of error codes. */
enum {
    PLAIN_SIGNAL_BREAK    = 0x300,
    PLAIN_SIGNAL_RETURN   = 0x301,
    PLAIN_SIGNAL_CONTINUE = 0x302
};

/* Owner flags for PLAIN_VALUE. */
enum {
    /* Set on PLAIN_TYPE_OBJECT values whose data pointer is a PLAIN_FRAME*
     * (Plain-native instance created with object { ... }).
     * PLAIN_VALUE_CLEAR will call PLAIN_FRAME_RELEASE; PLAIN_VALUE_COPY will
     * call PLAIN_FRAME_RETAIN. Without this flag, data is an externally managed
     * C/C++ pointer and the runtime never frees or retains it. */
    PLAIN_OWNER_INTERNAL = 0x00,
    PLAIN_OWNER_USER = 0x01
};

/*
 * PLAIN_CALLABLE — a user-defined function or procedure, captured at
 * definition time and re-evaluated on each call. Parameters are extracted
 * once from the parsed tree; the body is stored as source text for
 * re-evaluation. For built-in commands, native is set and all other
 * fields are NULL/zero.
 */
struct PLAIN_CALLABLE {
    PLAIN_BYTE** parameters;            /* Array of parameter name strings. NULL for native callables. */
    PLAIN_WORD_DOUBLE parameter_count;  /* Number of parameters. 0 for native callables. */
    PLAIN_BYTE* body;                   /* Source text of the body to evaluate on each call. NULL for native callables. */
    PLAIN_SUBROUTINE native;            /* C implementation. NULL for user-defined callables. */
    struct PLAIN_FRAME* closure;        /* Frame captured at definition time (closure). NULL for native callables. */
    PLAIN_WORD_DOUBLE flags;            /* PLAIN_CALLABLE_IMMUTABLE etc. */
};

/*
 * PLAIN_BINDING — a single name-to-value association inside a PLAIN_FRAME.
 * May hold either a plain value or a callable (never both simultaneously).
 * The UT_hash_handle field is required by uthash and must not be touched directly.
 */
struct PLAIN_BINDING {
    PLAIN_BYTE* name;            /* hash key (heap-allocated, null-terminated) */
    struct PLAIN_VALUE value;
    struct PLAIN_CALLABLE* callable;
    PLAIN_WORD_DOUBLE flags;     /* PLAIN_BINDING_IMMUTABLE etc. */
    UT_hash_handle hh;           /* uthash intrusive handle */
};

/*
 * PLAIN_FRAME — one level of the variable namespace, corresponding to a
 * single call level: the global program, or one function/procedure call.
 * Frames chain upward via <parent>; variable lookup walks the chain.
 * <bindings> is the uthash table head (NULL when the frame is empty).
 * <references> is a reference count: when a callable captures this frame
 * as its closure, the count is incremented; when the callable is freed,
 * the count is decremented. PLAIN_FRAME_RELEASE frees the frame only
 * when the count reaches zero.
 */
struct PLAIN_FRAME {
    struct PLAIN_FRAME* parent;
    struct PLAIN_BINDING* bindings;    /* uthash table head; NULL = empty */
    PLAIN_WORD_DOUBLE references;      /* reference count; 0 = owned by call stack */
};

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

/* Allocates a new frame with reference count 0. Only <parent> can be NULL (root frame). */
struct PLAIN_FRAME* PLAIN_FRAME_CREATE(struct PLAIN_FRAME* parent);

/* Increments the reference count on <frame>. */
void PLAIN_FRAME_RETAIN(struct PLAIN_FRAME* frame);

/* Decrements the reference count and frees the frame when it reaches zero.
 * Use instead of PLAIN_FRAME_DESTROY when the frame may be shared by closures. */
void PLAIN_FRAME_RELEASE(struct PLAIN_FRAME* frame);

/* Frees all memory owned by <frame> unconditionally (ignores reference count). */
void PLAIN_FRAME_DESTROY(struct PLAIN_FRAME* frame);

/* Returns the binding for <name>, searching <frame> and all parent frames. */
struct PLAIN_BINDING* PLAIN_FRAME_FIND(struct PLAIN_FRAME* frame, const PLAIN_BYTE* name);

/* Creates or updates a binding in <frame> (never in a parent). Returns
 * PLAIN_ERROR_SYSTEM_WRONG_DATA if the existing binding is immutable. */
PLAIN_WORD_DOUBLE PLAIN_FRAME_BIND(struct PLAIN_FRAME* frame, const PLAIN_BYTE* name, struct PLAIN_VALUE* value, struct PLAIN_CALLABLE* callable, PLAIN_WORD_DOUBLE flags);

/* Like PLAIN_FRAME_BIND, but first searches the entire chain for an existing
 * binding. If found, updates it in place; otherwise creates a new local binding.
 * This is what variable assignment (x = value) uses to support closures. */
PLAIN_WORD_DOUBLE PLAIN_FRAME_SET(struct PLAIN_FRAME* frame, const PLAIN_BYTE* name, struct PLAIN_VALUE* value, struct PLAIN_CALLABLE* callable, PLAIN_WORD_DOUBLE flags);

/* Frees data owned by <value> and resets it to nil. */
void PLAIN_VALUE_CLEAR(struct PLAIN_VALUE* value);

/* Deep-copies <source> into <destination>. */
PLAIN_WORD_DOUBLE PLAIN_VALUE_COPY(struct PLAIN_VALUE* destination, const struct PLAIN_VALUE* source);

/* Returns nonzero if <value> represents a logically true state:
 * yes, non-zero integer, non-zero real, or non-empty string. */
PLAIN_WORD_DOUBLE PLAIN_IS_TRUE(const struct PLAIN_VALUE* value);

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

/* Evaluates all recognised built-in commands and substitutes variables from
 * the current frame. Delegates unrecognised commands to context->handler. */
PLAIN_WORD_DOUBLE PLAIN_RESOLVE(void* context, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value);

/* Registers a single native procedure in the current frame of <context> as a
 * mutable callable. Use this to extend Plain from the host application.
 * The binding is mutable, so Plain code can override it at any time. */
PLAIN_WORD_DOUBLE PLAIN_CONTEXT_REGISTER(struct PLAIN_CONTEXT* context, const PLAIN_BYTE* name, PLAIN_SUBROUTINE native);

/* Calls <callable> with arguments from <node>, writing the result into <value>.
 * Handles both native callables (via the native pointer) and user-defined ones
 * (by evaluating callable->body with parameters bound from node arguments).
 * This is the core dispatch used by PLAIN_RESOLVE and callable from C++. */
PLAIN_WORD_DOUBLE PLAIN_CALL(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, struct PLAIN_CALLABLE* callable, struct PLAIN_VALUE* value);
