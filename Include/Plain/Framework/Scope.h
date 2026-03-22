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
#include <Plain/Framework/Host/Environment.h>

/* Control-flow signals, returned instead of error codes. */
enum {
    PLAIN_SIGNAL_BREAK    = 0x300,
    PLAIN_SIGNAL_RETURN   = 0x301,
    PLAIN_SIGNAL_CONTINUE = 0x302
};

/* Flags for PLAIN_CALLABLE. */
enum {
    PLAIN_CALLABLE_IMMUTABLE = 0x01  /* Callable was created with function (not procedure); binding should be immutable. */
};

/* Flags for PLAIN_BINDING. */
enum {
    PLAIN_BINDING_IMMUTABLE = 0x01  /* Binding cannot be reassigned (functions). */
};

/*
 * PLAIN_CALLABLE — the stored source text of a user-defined function or
 * procedure, captured at definition time and re-evaluated on each call.
 * For built-in commands, body and parameters are NULL and native is set.
 */
struct PLAIN_CALLABLE {
    PLAIN_BYTE* parameters;       /* Source text of the parameter list. NULL for native callables. */
    PLAIN_BYTE* body;             /* Source text of the body to evaluate on each call. NULL for native callables. */
    PLAIN_SUBROUTINE native;      /* C implementation. NULL for user-defined callables. */
    struct PLAIN_FRAME* closure;  /* Frame captured at definition time (closure). NULL for native callables. */
    PLAIN_WORD_DOUBLE flags;      /* PLAIN_CALLABLE_IMMUTABLE etc. */
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
    struct PLAIN_VALUE result;   /* Return value written by the last `return` command. */
    PLAIN_DELEGATE tracker;      /* Receives syntax errors and diagnostics. */
    PLAIN_SUBROUTINE handler;    /* Host-provided extension: called for unrecognised commands. */
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

/* Evaluates all recognised built-in commands and substitutes variables from
 * the current frame. Delegates unrecognised commands to context->handler. */
PLAIN_WORD_DOUBLE PLAIN_RESOLVE(void* context, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value);

/* Registers all standard built-in procedures in the root frame of <context>.
 * Call once after PLAIN_FRAME_CREATE, before any evaluation. Built-ins are
 * mutable bindings — the user may freely override any of them. */
PLAIN_WORD_DOUBLE PLAIN_CONTEXT_INIT(struct PLAIN_CONTEXT* context);

/* Registers a single native procedure in the current frame of <context> as a
 * mutable callable. Use this to extend Plain from the host application.
 * The binding is mutable, so Plain code can override it at any time. */
PLAIN_WORD_DOUBLE PLAIN_CONTEXT_REGISTER(struct PLAIN_CONTEXT* context, const PLAIN_BYTE* name, PLAIN_SUBROUTINE native);
