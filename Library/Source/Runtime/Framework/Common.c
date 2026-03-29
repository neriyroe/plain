/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     03/28/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Framework — Plain standard library.
 * Contains all built-in procedure implementations and PLAIN_FRAMEWORK_REGISTER.
 *
 * Every function here follows the native callable signature:
 *   (PLAIN_CONTEXT*, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value)
 * where <node> holds the parsed arguments and <value> receives the return value.
 */

#include <Plain/Framework.h>
#include <uthash.h>
#include <stdio.h>

/* ================================================================== */
/*  Private helpers                                                   */
/* ================================================================== */

/* Returns true if <value> is a keyword whose text equals <name>. */
PLAIN_INLINE PLAIN_WORD_DOUBLE PLAIN_FW_KEYWORD_EQ(PLAIN_VALUE* value, const PLAIN_BYTE* name) {
    return value != NULL
        && value->type == PLAIN_TYPE_KEYWORD
        && strcmp((const char*)value->data, (const char*)name) == 0;
}

/* Deep-copies a block's node tree, walks it through PLAIN_RESOLVE, then frees
 * the copy.  Used by control-flow procedures (if, repeat, when, do) to
 * execute deferred { } blocks without consuming the stored canonical tree. */
static PLAIN_WORD_DOUBLE PLAIN_FW_WALK_BLOCK(PLAIN_CONTEXT* context, PLAIN_LIST* block, PLAIN_VALUE* value) {
    if(block == NULL || block->keyword.from == NULL) {
        return 0;
    }
    PLAIN_LIST* copy = PLAIN_LIST_COPY(block);
    if(copy == NULL) {
        return PLAIN_ERROR_SYSTEM;
    }
    PLAIN_WORD_DOUBLE error = PLAIN_WALK(context, PLAIN_RESOLVE, copy, value);
    PLAIN_UNLINK(copy);
    PLAIN_RESIZE(copy, 0, sizeof(PLAIN_LIST));
    return error;
}

/* Extracts parameter names from a {params} block into a heap-allocated
 * array of null-terminated strings.  The caller owns the returned array
 * and every string in it.  Sets *count to the number of parameters. */
static PLAIN_BYTE** PLAIN_FW_PARAMETERS_EXTRACT(PLAIN_LIST* block, PLAIN_WORD_DOUBLE* count) {
    *count = 0;
    if(block == NULL || block->keyword.from == NULL) {
        return NULL;
    }

    /* First pass: count how many keyword nodes are in the block. */
    PLAIN_LIST* cursor = block;
    PLAIN_WORD_DOUBLE n = 0;
    while(cursor != NULL && cursor->keyword.from != NULL) {
        n++;
        cursor = cursor->node;
    }

    /* Allocate the array of name pointers. */
    PLAIN_BYTE** names = (PLAIN_BYTE**)PLAIN_RESIZE(NULL, sizeof(PLAIN_BYTE*) * n, 0);
    if(names == NULL) {
        return NULL;
    }

    /* Second pass: copy each keyword into a heap-allocated string. */
    cursor = block;
    for(PLAIN_WORD_DOUBLE i = 0; i < n; i++) {
        PLAIN_WORD_DOUBLE length = cursor->keyword.to - cursor->keyword.from;
        names[i] = (PLAIN_BYTE*)PLAIN_RESIZE(NULL, length + 1, 0);
        if(names[i] == NULL) {
            /* Roll back all previously allocated names. */
            for(PLAIN_WORD_DOUBLE j = 0; j < i; j++) {
                PLAIN_RESIZE(names[j], 0, strlen((const char*)names[j]) + 1);
            }
            PLAIN_RESIZE(names, 0, sizeof(PLAIN_BYTE*) * n);
            return NULL;
        }
        memcpy(names[i], cursor->keyword.from, length);
        names[i][length] = '\0';
        cursor = cursor->node;
    }

    *count = n;
    return names;
}

/* Writes an integer result into <value>. No-op when value is NULL
 * (statement position — return value discarded). */
PLAIN_INLINE PLAIN_WORD_DOUBLE PLAIN_FW_SET_INTEGER(PLAIN_VALUE* value, PLAIN_WORD_QUADRUPLE number) {
    if(value == NULL) {
        return 0;
    }
    return PLAIN_EXPORT((PLAIN_BYTE*)&number, sizeof(PLAIN_WORD_QUADRUPLE), PLAIN_TYPE_INTEGER, value);
}

/* Writes a boolean result into <value>.  Booleans are stored as a
 * non-NULL pointer for true and NULL for false, with length = 0. */
PLAIN_INLINE PLAIN_WORD_DOUBLE PLAIN_FW_SET_BOOLEAN(PLAIN_VALUE* value, PLAIN_WORD_DOUBLE boolean) {
    if(value == NULL) {
        return 0;
    }
    return PLAIN_EXPORT(boolean ? (PLAIN_BYTE*)0xFF : NULL, 0, PLAIN_TYPE_BOOLEAN, value);
}

/* Coerces any numeric or boolean value to a PLAIN_REAL_DOUBLE for
 * arithmetic and comparison operations. */
PLAIN_INLINE PLAIN_REAL_DOUBLE PLAIN_FW_AS_REAL(PLAIN_VALUE* value) {
    if(value->type == PLAIN_TYPE_REAL) {
        return (PLAIN_REAL_DOUBLE)*(PLAIN_REAL*)value->data;
    }
    if(value->type == PLAIN_TYPE_INTEGER) {
        return (PLAIN_REAL_DOUBLE)(long long)*(PLAIN_WORD_QUADRUPLE*)value->data;
    }
    if(value->type == PLAIN_TYPE_BOOLEAN) {
        return value->data != NULL ? 1.0 : 0.0;
    }
    return 0.0;
}

/* Applies a single binary arithmetic operation.  If either operand is
 * real, the result is real (automatic promotion); otherwise integer. */
static PLAIN_WORD_DOUBLE PLAIN_FW_APPLY_ARITHMETIC(PLAIN_VALUE* a, PLAIN_VALUE* b, PLAIN_BYTE operation, PLAIN_VALUE* value) {
    /* Real path — at least one operand is a floating-point value. */
    if(a->type == PLAIN_TYPE_REAL || b->type == PLAIN_TYPE_REAL) {
        PLAIN_REAL_DOUBLE va = PLAIN_FW_AS_REAL(a);
        PLAIN_REAL_DOUBLE vb = PLAIN_FW_AS_REAL(b);
        PLAIN_REAL_DOUBLE result = 0;
        switch(operation) {
            case '+': result = va + vb; break;
            case '-': result = va - vb; break;
            case '*': result = va * vb; break;
            case '/': result = vb != 0 ? va / vb : 0; break;
        }
        PLAIN_REAL f = (PLAIN_REAL)result;
        if(value != NULL) {
            return PLAIN_EXPORT((PLAIN_BYTE*)&f, sizeof(PLAIN_REAL), PLAIN_TYPE_REAL, value);
        }
    } else {
        /* Integer path — both operands are integers (or booleans). */
        long long va = (long long)*(PLAIN_WORD_QUADRUPLE*)a->data;
        long long vb = (long long)*(PLAIN_WORD_QUADRUPLE*)b->data;
        long long result = 0;
        switch(operation) {
            case '+': result = va + vb; break;
            case '-': result = va - vb; break;
            case '*': result = va * vb; break;
            case '/': result = vb != 0 ? va / vb : 0; break;
            case '%': result = vb != 0 ? va % vb : 0; break;
        }
        if(value != NULL) {
            return PLAIN_FW_SET_INTEGER(value, (PLAIN_WORD_QUADRUPLE)result);
        }
    }
    return 0;
}

/* Applies a binary comparison.  Strings are compared lexicographically;
 * all other types are compared as reals. */
static PLAIN_WORD_DOUBLE PLAIN_FW_APPLY_COMPARISON(PLAIN_VALUE* a, PLAIN_VALUE* b, PLAIN_BYTE operation, PLAIN_VALUE* value) {
    /* String comparison — both operands must be strings. */
    if(a->type == PLAIN_TYPE_STRING && b->type == PLAIN_TYPE_STRING) {
        int comparison = strcmp((const char*)a->data, (const char*)b->data);
        PLAIN_WORD_DOUBLE result = 0;
        switch(operation) {
            case '=': result = comparison == 0; break;
            case '<': result = comparison < 0;  break;
            case '>': result = comparison > 0;  break;
        }
        return PLAIN_FW_SET_BOOLEAN(value, result);
    }

    /* Numeric comparison — coerce both to real. */
    PLAIN_REAL_DOUBLE va = PLAIN_FW_AS_REAL(a);
    PLAIN_REAL_DOUBLE vb = PLAIN_FW_AS_REAL(b);
    PLAIN_WORD_DOUBLE result = 0;
    switch(operation) {
        case '=': result = va == vb; break;
        case '<': result = va < vb;  break;
        case '>': result = va > vb;  break;
    }
    return PLAIN_FW_SET_BOOLEAN(value, result);
}

/* Variadic left-to-right fold: resolves all arguments and combines them
 * pairwise using the given arithmetic operation.  With one argument,
 * returns the argument unchanged. */
static PLAIN_WORD_DOUBLE PLAIN_FW_ARITHMETIC_FOLD(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_BYTE operation, PLAIN_VALUE* value) {
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    if(arity == 0) {
        return 0;
    }

    PLAIN_VALUE first = PLAIN_RESOLVE_ARGUMENT(context, node, 0);

    /* Single argument — return it as-is. */
    if(arity == 1) {
        if(value != NULL) {
            return PLAIN_VALUE_COPY(value, &first);
        }
        return 0;
    }

    /* Fold: accumulator = first; accumulator = accumulator <op> next; ... */
    PLAIN_VALUE accumulator = {NULL, 0, PLAIN_TYPE_NIL, 0};
    PLAIN_VALUE_COPY(&accumulator, &first);

    for(PLAIN_WORD_DOUBLE i = 1; i < arity; i++) {
        PLAIN_VALUE next = PLAIN_RESOLVE_ARGUMENT(context, node, i);
        PLAIN_VALUE result = {NULL, 0, PLAIN_TYPE_NIL, 0};
        PLAIN_WORD_DOUBLE error = PLAIN_FW_APPLY_ARITHMETIC(&accumulator, &next, operation, &result);
        PLAIN_VALUE_CLEAR(&accumulator);
        if(error != 0) {
            PLAIN_VALUE_CLEAR(&result);
            return error;
        }
        accumulator = result;
    }

    if(value != NULL) {
        *value = accumulator;
    } else {
        PLAIN_VALUE_CLEAR(&accumulator);
    }
    return 0;
}

/* Deep-copies a callable struct including its parameter name strings,
 * body tree, and closure reference.  Used by set when assigning a
 * callable value to ensure independent ownership. */
static struct PLAIN_CALLABLE* PLAIN_FW_CALLABLE_DUPLICATE(struct PLAIN_CALLABLE* source) {
    struct PLAIN_CALLABLE* copy = (struct PLAIN_CALLABLE*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_CALLABLE), 0);
    if(copy == NULL) {
        return NULL;
    }
    memset(copy, 0, sizeof(struct PLAIN_CALLABLE));

    copy->native  = source->native;
    copy->closure = source->closure;
    copy->flags   = source->flags;

    /* Retain the closure frame — the copy shares the same captured scope. */
    if(copy->closure != NULL) {
        PLAIN_FRAME_RETAIN(copy->closure);
    }

    /* Duplicate parameter name strings. */
    copy->parameter_count = source->parameter_count;
    if(source->parameters != NULL && source->parameter_count > 0) {
        copy->parameters = (PLAIN_BYTE**)PLAIN_RESIZE(NULL, sizeof(PLAIN_BYTE*) * source->parameter_count, 0);
        if(copy->parameters != NULL) {
            for(PLAIN_WORD_DOUBLE i = 0; i < source->parameter_count; i++) {
                PLAIN_WORD_DOUBLE length = strlen((const char*)source->parameters[i]) + 1;
                copy->parameters[i] = (PLAIN_BYTE*)PLAIN_RESIZE(NULL, length, 0);
                if(copy->parameters[i]) {
                    memcpy(copy->parameters[i], source->parameters[i], length);
                }
            }
        }
    }

    /* Duplicate the pre-parsed body tree. */
    if(source->body != NULL) {
        copy->body = PLAIN_LIST_COPY(source->body);
    }

    return copy;
}

/* ================================================================== */
/*  Built-in procedure implementations                                */
/* ================================================================== */

/* ---- set --------------------------------------------------------- */
/* set name, value
 * Assigns <value> to <name> in the nearest enclosing frame where <name>
 * already exists, or creates a new local binding.  The first argument
 * is kept as a raw keyword (the variable name); the second is resolved. */
static PLAIN_WORD_DOUBLE PLAIN_FW_SET(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    if(PLAIN_ARITY(node) < 2) {
        return 0;
    }

    /* First argument: the variable name (raw keyword, not resolved). */
    PLAIN_VALUE* name_arg = PLAIN_ARGUMENT(node, 0);
    if(name_arg == NULL || name_arg->type != PLAIN_TYPE_KEYWORD) {
        return 0;
    }

    /* Copy the name to a temporary buffer — the argument memory may be
     * invalidated during resolution of the second argument. */
    PLAIN_WORD_DOUBLE name_length = strlen((const char*)name_arg->data);
    PLAIN_BYTE* name = (PLAIN_BYTE*)PLAIN_AUTO(name_length + 1);
    memcpy(name, name_arg->data, name_length);
    name[name_length] = '\0';

    /* Second argument: the value to assign (resolved). */
    PLAIN_VALUE assignment_value = PLAIN_RESOLVE_ARGUMENT(context, node, 1);

    /* Callable values need to be duplicated so the binding owns its own
     * copy of the parameter list and body tree. */
    if(assignment_value.type == PLAIN_TYPE_CALLABLE && assignment_value.data != NULL) {
        struct PLAIN_CALLABLE* source = (struct PLAIN_CALLABLE*)assignment_value.data;
        struct PLAIN_CALLABLE* copy = PLAIN_FW_CALLABLE_DUPLICATE(source);
        if(copy == NULL) {
            return PLAIN_ERROR_SYSTEM;
        }
        return PLAIN_FRAME_SET(context->frame, name, NULL, copy, 0);
    }

    PLAIN_FRAME_SET(context->frame, name, &assignment_value, NULL, 0);

    if(value != NULL) {
        PLAIN_VALUE_COPY(value, &assignment_value);
    }
    return 0;
}

/* ---- if ---------------------------------------------------------- */
/* if condition {then}
 * if condition {then} else {else}
 * if condition {then} else condition {then} else {else}
 *
 * Walks condition+block pairs.  "else" followed by a block is the final
 * fallback.  "else" followed by a non-block value starts an elseif chain. */
static PLAIN_WORD_DOUBLE PLAIN_FW_IF(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    PLAIN_WORD_DOUBLE i = 0;

    while(i + 1 < arity) {
        /* Resolve the condition at position i. */
        PLAIN_VALUE condition = PLAIN_RESOLVE_ARGUMENT(context, node, i);
        PLAIN_VALUE* block = PLAIN_ARGUMENT(node, i + 1);

        /* If truthy, execute the corresponding block and return. */
        if(PLAIN_VALUE_TRUTHY(&condition)) {
            return PLAIN_FW_WALK_BLOCK(context, (PLAIN_LIST*)block->data, value);
        }

        /* Skip past the condition and its block. */
        i += 2;
        if(i >= arity) {
            break;
        }

        /* Expect "else" keyword to continue the chain. */
        if(!PLAIN_FW_KEYWORD_EQ(PLAIN_ARGUMENT(node, i), (const PLAIN_BYTE*)"else")) {
            break;
        }
        i++;
        if(i >= arity) {
            break;
        }

        /* "else {block}" — final else: execute unconditionally. */
        if(PLAIN_ARGUMENT(node, i)->type == PLAIN_TYPE_LIST) {
            return PLAIN_FW_WALK_BLOCK(context, (PLAIN_LIST*)PLAIN_ARGUMENT(node, i)->data, value);
        }

        /* "else condition {block}" — elseif: loop back to evaluate condition. */
    }
    return 0;
}

/* ---- when -------------------------------------------------------- */
/* when target
 *     value1 {body1}
 *     value2 {body2}
 *     else   {default}
 *
 * Compares the resolved target against each case value using equality.
 * Executes the body of the first match and stops. */
static PLAIN_WORD_DOUBLE PLAIN_FW_WHEN(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    if(arity < 3) {
        return 0;
    }

    /* First argument: the value to match against. */
    PLAIN_VALUE target = PLAIN_RESOLVE_ARGUMENT(context, node, 0);
    PLAIN_WORD_DOUBLE i = 1;

    /* Walk value+block pairs. */
    while(i + 1 < arity) {
        PLAIN_VALUE* block = PLAIN_ARGUMENT(node, i + 1);

        /* "else" keyword — execute the fallback block. */
        if(PLAIN_FW_KEYWORD_EQ(PLAIN_ARGUMENT(node, i), (const PLAIN_BYTE*)"else")) {
            if(block != NULL && block->type == PLAIN_TYPE_LIST) {
                return PLAIN_FW_WALK_BLOCK(context, (PLAIN_LIST*)block->data, value);
            }
            break;
        }

        /* Regular case — compare target to candidate. */
        if(block != NULL && block->type == PLAIN_TYPE_LIST) {
            PLAIN_VALUE candidate = PLAIN_RESOLVE_ARGUMENT(context, node, i);
            PLAIN_VALUE result = {NULL, 0, PLAIN_TYPE_NIL, 0};
            PLAIN_FW_APPLY_COMPARISON(&target, &candidate, '=', &result);

            if(PLAIN_VALUE_TRUTHY(&result)) {
                PLAIN_VALUE_CLEAR(&result);
                return PLAIN_FW_WALK_BLOCK(context, (PLAIN_LIST*)block->data, value);
            }
            PLAIN_VALUE_CLEAR(&result);
        }
        i += 2;
    }
    return 0;
}

/* ---- repeat (counted with binding) ------------------------------- */
/* repeat {variable} times count {body}
 * Creates a child frame with <variable> bound to the iteration index
 * (0 to count-1) on each iteration.  The variable is scoped to the
 * loop body and does not leak into the enclosing frame. */
static PLAIN_WORD_DOUBLE PLAIN_REPEAT_COUNTED_BINDING(PLAIN_CONTEXT* context, PLAIN_LIST* variable, PLAIN_WORD_QUADRUPLE count, PLAIN_LIST* body, PLAIN_VALUE* value) {
    /* Extract the iteration variable name from the tokenized keyword. */
    PLAIN_WORD_DOUBLE length = variable->keyword.to - variable->keyword.from;
    if(length == 0) {
        return 0;
    }
    PLAIN_BYTE* name = (PLAIN_BYTE*)PLAIN_AUTO(length + 1);
    if(name == NULL) {
        return PLAIN_ERROR_SYSTEM;
    }
    memcpy(name, variable->keyword.from, length);
    name[length] = '\0';

    /* Create a child frame for the loop scope. */
    struct PLAIN_FRAME* frame = PLAIN_FRAME_CREATE(context->frame);
    if(frame == NULL) {
        return PLAIN_ERROR_SYSTEM;
    }
    PLAIN_FRAME_RETAIN(frame);
    struct PLAIN_FRAME* saved = context->frame;
    context->frame = frame;

    /* Bind the iteration variable with an initial value of 0. */
    PLAIN_WORD_QUADRUPLE zero = 0;
    PLAIN_VALUE initial = {(PLAIN_BYTE*)&zero, sizeof(PLAIN_WORD_QUADRUPLE), PLAIN_TYPE_INTEGER, 0};
    PLAIN_FRAME_BIND(frame, name, &initial, NULL, 0);

    /* Cache the binding pointer — we update the value in-place each iteration
     * to avoid repeated hash lookups. */
    struct PLAIN_BINDING* binding = PLAIN_FRAME_FIND(frame, name);

    for(PLAIN_WORD_QUADRUPLE index = 0; index < count; index++) {
        /* Update the iteration variable directly. */
        *(PLAIN_WORD_QUADRUPLE*)binding->value.data = index;

        PLAIN_VALUE iter = {NULL, 0, PLAIN_TYPE_NIL, 0};
        PLAIN_WORD_DOUBLE error = PLAIN_FW_WALK_BLOCK(context, body, &iter);

        if(error == PLAIN_SIGNAL_RETURN) {
            if(value != NULL) {
                *value = iter;
            } else {
                PLAIN_VALUE_CLEAR(&iter);
            }
            context->frame = saved;
            PLAIN_FRAME_RELEASE(frame);
            return PLAIN_SIGNAL_RETURN;
        }

        PLAIN_VALUE_CLEAR(&iter);

        if(error == PLAIN_SIGNAL_BREAK) {
            break;
        }
        if(error == PLAIN_SIGNAL_CONTINUE) {
            continue;
        }
        if(error != 0) {
            context->frame = saved;
            PLAIN_FRAME_RELEASE(frame);
            return error;
        }
    }

    context->frame = saved;
    PLAIN_FRAME_RELEASE(frame);
    return 0;
}

/* ---- repeat ------------------------------------------------------ */
/* Dispatches to one of four loop forms based on argument pattern:
 *   repeat {body}                         — infinite loop (use break)
 *   repeat count {body}                   — counted loop
 *   repeat {condition} {body}             — conditional (while) loop
 *   repeat {variable} times count {body}  — counted loop with binding */
static PLAIN_WORD_DOUBLE PLAIN_FW_REPEAT(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    PLAIN_VALUE* first = PLAIN_ARGUMENT(node, 0);
    if(first == NULL) {
        return 0;
    }

    /* --- Form 1: repeat {body} — infinite loop. ------------------- */
    if(arity == 1 && first->type == PLAIN_TYPE_LIST) {
        PLAIN_LIST* body = (PLAIN_LIST*)first->data;
        for(;;) {
            PLAIN_VALUE iter = {NULL, 0, PLAIN_TYPE_NIL, 0};
            PLAIN_WORD_DOUBLE error = PLAIN_FW_WALK_BLOCK(context, body, &iter);

            if(error == PLAIN_SIGNAL_RETURN) {
                if(value != NULL) {
                    *value = iter;
                } else {
                    PLAIN_VALUE_CLEAR(&iter);
                }
                return PLAIN_SIGNAL_RETURN;
            }

            PLAIN_VALUE_CLEAR(&iter);

            if(error == PLAIN_SIGNAL_BREAK) {
                break;
            }
            if(error == PLAIN_SIGNAL_CONTINUE) {
                continue;
            }
            if(error != 0) {
                return error;
            }
        }
        return 0;
    }

    /* --- Form 4: repeat {variable} times count {body} ------------- */
    if(arity >= 4 && first->type == PLAIN_TYPE_LIST &&
       PLAIN_FW_KEYWORD_EQ(PLAIN_ARGUMENT(node, 1), (const PLAIN_BYTE*)"times")) {
        PLAIN_VALUE counter = PLAIN_RESOLVE_ARGUMENT(context, node, 2);
        PLAIN_VALUE* fourth = PLAIN_ARGUMENT(node, 3);
        if(counter.type == PLAIN_TYPE_INTEGER && fourth != NULL && fourth->type == PLAIN_TYPE_LIST) {
            return PLAIN_REPEAT_COUNTED_BINDING(context,
                (PLAIN_LIST*)first->data,
                *(PLAIN_WORD_QUADRUPLE*)counter.data,
                (PLAIN_LIST*)fourth->data,
                value);
        }
    }

    /* Forms 2 and 3 both need a second argument that is a block. */
    PLAIN_VALUE* second = PLAIN_ARGUMENT(node, 1);
    if(arity < 2 || second == NULL || second->type != PLAIN_TYPE_LIST) {
        return 0;
    }
    PLAIN_LIST* body = (PLAIN_LIST*)second->data;

    /* --- Form 2: repeat count {body} — counted loop. -------------- */
    PLAIN_VALUE first_value = PLAIN_RESOLVE_ARGUMENT(context, node, 0);
    if(first_value.type == PLAIN_TYPE_INTEGER) {
        PLAIN_WORD_QUADRUPLE count = *(PLAIN_WORD_QUADRUPLE*)first_value.data;
        for(PLAIN_WORD_QUADRUPLE i = 0; i < count; i++) {
            PLAIN_VALUE iter = {NULL, 0, PLAIN_TYPE_NIL, 0};
            PLAIN_WORD_DOUBLE error = PLAIN_FW_WALK_BLOCK(context, body, &iter);

            if(error == PLAIN_SIGNAL_RETURN) {
                if(value != NULL) {
                    *value = iter;
                } else {
                    PLAIN_VALUE_CLEAR(&iter);
                }
                return PLAIN_SIGNAL_RETURN;
            }

            PLAIN_VALUE_CLEAR(&iter);

            if(error == PLAIN_SIGNAL_BREAK) {
                break;
            }
            if(error == PLAIN_SIGNAL_CONTINUE) {
                continue;
            }
            if(error != 0) {
                return error;
            }
        }
        return 0;
    }

    /* --- Form 3: repeat {condition} {body} — while loop. ---------- */
    if(first->type == PLAIN_TYPE_LIST) {
        PLAIN_LIST* condition = (PLAIN_LIST*)first->data;
        for(;;) {
            /* Evaluate the condition block. */
            PLAIN_VALUE test = {NULL, 0, PLAIN_TYPE_NIL, 0};
            PLAIN_WORD_DOUBLE error = PLAIN_FW_WALK_BLOCK(context, condition, &test);
            PLAIN_WORD_DOUBLE ok = PLAIN_VALUE_TRUTHY(&test);
            PLAIN_VALUE_CLEAR(&test);
            if(error != 0) {
                return error;
            }
            if(!ok) {
                break;
            }

            /* Execute the loop body. */
            PLAIN_VALUE iter = {NULL, 0, PLAIN_TYPE_NIL, 0};
            error = PLAIN_FW_WALK_BLOCK(context, body, &iter);

            if(error == PLAIN_SIGNAL_RETURN) {
                if(value != NULL) {
                    *value = iter;
                } else {
                    PLAIN_VALUE_CLEAR(&iter);
                }
                return PLAIN_SIGNAL_RETURN;
            }

            PLAIN_VALUE_CLEAR(&iter);

            if(error == PLAIN_SIGNAL_BREAK) {
                break;
            }
            if(error == PLAIN_SIGNAL_CONTINUE) {
                continue;
            }
            if(error != 0) {
                return error;
            }
        }
        return 0;
    }

    return 0;
}

/* ---- return ------------------------------------------------------ */
/* return [value]
 * Resolves the optional first argument and propagates PLAIN_SIGNAL_RETURN
 * up the call stack.  The caller (PLAIN_CALL) intercepts this signal. */
static PLAIN_WORD_DOUBLE PLAIN_FW_RETURN(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    if(PLAIN_ARITY(node) > 0) {
        PLAIN_VALUE resolved = PLAIN_RESOLVE_ARGUMENT(context, node, 0);
        if(value != NULL) {
            PLAIN_VALUE_COPY(value, &resolved);
        }
    } else {
        if(value != NULL) {
            PLAIN_VALUE_CLEAR(value);
        }
    }
    return PLAIN_SIGNAL_RETURN;
}

/* ---- break ------------------------------------------------------- */
/* break — exits the nearest enclosing repeat loop. */
static PLAIN_WORD_DOUBLE PLAIN_FW_BREAK(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    return PLAIN_SIGNAL_BREAK;
}

/* ---- continue ---------------------------------------------------- */
/* continue — skips to the next iteration of the nearest repeat loop. */
static PLAIN_WORD_DOUBLE PLAIN_FW_CONTINUE(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    return PLAIN_SIGNAL_CONTINUE;
}

/* ---- do ---------------------------------------------------------- */
/* do {block}
 * Evaluates the block in a fresh child scope.  Variables created inside
 * the block do not leak into the surrounding frame.  A return inside the
 * block yields the do-expression's value. */
static PLAIN_WORD_DOUBLE PLAIN_FW_DO(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    if(PLAIN_ARITY(node) < 1) {
        return 0;
    }
    PLAIN_VALUE* first = PLAIN_ARGUMENT(node, 0);
    if(first->type != PLAIN_TYPE_LIST) {
        return 0;
    }

    /* Push a fresh child frame. */
    struct PLAIN_FRAME* frame = PLAIN_FRAME_CREATE(context->frame);
    if(frame == NULL) {
        return PLAIN_ERROR_SYSTEM;
    }
    struct PLAIN_FRAME* saved = context->frame;
    context->frame = frame;

    PLAIN_WORD_DOUBLE error = PLAIN_FW_WALK_BLOCK(context, (PLAIN_LIST*)first->data, value);

    /* Pop the scope. */
    context->frame = saved;
    PLAIN_FRAME_RELEASE(frame);

    /* Intercept return — do consumes it so it doesn't propagate further. */
    if(error == PLAIN_SIGNAL_RETURN) {
        return 0;
    }
    return error;
}

/* ---- type -------------------------------------------------------- */
/* type value — returns a string describing the runtime type. */
static PLAIN_WORD_DOUBLE PLAIN_FW_TYPE(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    PLAIN_VALUE argument = PLAIN_RESOLVE_ARGUMENT(context, node, 0);
    const char* name = "none";
    switch(argument.type) {
        case PLAIN_TYPE_BOOLEAN:  name = "boolean";  break;
        case PLAIN_TYPE_INTEGER:  name = "integer";  break;
        case PLAIN_TYPE_REAL:     name = "real";     break;
        case PLAIN_TYPE_STRING:   name = "string";   break;
        case PLAIN_TYPE_KEYWORD:  name = "keyword";  break;
        case PLAIN_TYPE_LIST:     name = "list";     break;
        case PLAIN_TYPE_CALLABLE: name = "callable"; break;
        case PLAIN_TYPE_NIL:      name = "none";     break;
        case PLAIN_TYPE_OBJECT:   name = "object";   break;
        default:                  name = "unknown";  break;
    }
    if(value != NULL) {
        return PLAIN_EXPORT((PLAIN_BYTE*)name, strlen(name) + 1, PLAIN_TYPE_STRING, value);
    }
    return 0;
}

/* ---- define ------------------------------------------------------ */
/* Named form:     define name {parameters} {body}
 * Anonymous form: [define {parameters} {body}]
 *
 * Named form binds the callable in the current frame.
 * Anonymous form returns a callable value (for use in expressions).
 * Both capture the current frame as the closure. */
static PLAIN_WORD_DOUBLE PLAIN_FW_DEFINE(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    if(arity < 2) {
        return 0;
    }
    PLAIN_VALUE* first = PLAIN_ARGUMENT(node, 0);

    /* Named form: define name {parameters} {body} */
    if(arity >= 3 && (first->type == PLAIN_TYPE_KEYWORD || first->type == PLAIN_TYPE_STRING)) {
        PLAIN_LIST* parameters = (PLAIN_LIST*)PLAIN_ARGUMENT(node, 1)->data;
        PLAIN_LIST* body       = (PLAIN_LIST*)PLAIN_ARGUMENT(node, 2)->data;

        struct PLAIN_CALLABLE* callable = (struct PLAIN_CALLABLE*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_CALLABLE), 0);
        if(callable == NULL) {
            return PLAIN_ERROR_SYSTEM;
        }
        memset(callable, 0, sizeof(struct PLAIN_CALLABLE));
        callable->parameters = PLAIN_FW_PARAMETERS_EXTRACT(parameters, &callable->parameter_count);
        callable->body       = PLAIN_LIST_COPY(body);
        callable->closure    = context->frame;
        PLAIN_FRAME_RETAIN(context->frame);

        return PLAIN_FRAME_BIND(context->frame, first->data, NULL, callable, 0);
    }

    /* Anonymous form: [define {parameters} {body}] */
    if(first->type == PLAIN_TYPE_LIST) {
        PLAIN_LIST* parameters = (PLAIN_LIST*)first->data;
        PLAIN_LIST* body       = (PLAIN_LIST*)PLAIN_ARGUMENT(node, 1)->data;

        struct PLAIN_CALLABLE* callable = (struct PLAIN_CALLABLE*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_CALLABLE), 0);
        if(callable == NULL) {
            return PLAIN_ERROR_SYSTEM;
        }
        memset(callable, 0, sizeof(struct PLAIN_CALLABLE));
        callable->parameters = PLAIN_FW_PARAMETERS_EXTRACT(parameters, &callable->parameter_count);
        callable->body       = PLAIN_LIST_COPY(body);
        callable->closure    = context->frame;
        PLAIN_FRAME_RETAIN(context->frame);

        /* Return the callable as a value. */
        if(value != NULL) {
            value->type   = PLAIN_TYPE_CALLABLE;
            value->data   = (PLAIN_BYTE*)callable;
            value->length = 0;
        }
        return 0;
    }

    return 0;
}

/* ---- object ------------------------------------------------------ */
/* object {block}
 * Evaluates the block in a fresh child frame and returns that frame as
 * an object value.  Fields and methods are created by set/define inside
 * the block.  The object frame's parent is the enclosing (constructor)
 * frame, so methods can reach constructor parameters via the closure chain. */
static PLAIN_WORD_DOUBLE PLAIN_FW_OBJECT(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    if(PLAIN_ARITY(node) < 1) {
        return 0;
    }
    PLAIN_VALUE* first = PLAIN_ARGUMENT(node, 0);
    if(first == NULL || first->type != PLAIN_TYPE_LIST) {
        return 0;
    }

    /* Create the object frame as a child of the current (constructor) frame. */
    struct PLAIN_FRAME* frame = PLAIN_FRAME_CREATE(context->frame);
    if(frame == NULL) {
        return PLAIN_ERROR_SYSTEM;
    }

    /* Execute the block in the object frame to populate fields and methods. */
    struct PLAIN_FRAME* saved = context->frame;
    context->frame = frame;
    PLAIN_VALUE body = {NULL, 0, PLAIN_TYPE_NIL, 0};
    PLAIN_WORD_DOUBLE error = PLAIN_FW_WALK_BLOCK(context, (PLAIN_LIST*)first->data, &body);
    context->frame = saved;
    PLAIN_VALUE_CLEAR(&body);

    if(error == PLAIN_SIGNAL_RETURN) {
        PLAIN_VALUE_CLEAR(&body);
    } else if(error != 0) {
        PLAIN_FRAME_RELEASE(frame);
        return error;
    }

    /* Break circular closure references: each define inside the block
     * called PLAIN_FRAME_RETAIN on this frame (as its closure).  Undo
     * those retains so the frame's ref count only reflects external
     * holders — otherwise the object could never be freed. */
    struct PLAIN_BINDING* b;
    struct PLAIN_BINDING* t;
    HASH_ITER(hh, frame->bindings, b, t) {
        if(b->callable != NULL && b->callable->closure == frame) {
            frame->references--;
        }
    }
    PLAIN_FRAME_RETAIN(frame);

    /* Auto-bind "self" — a reference to the object itself.  The value is
     * set directly (no PLAIN_VALUE_COPY) to avoid an extra retain that
     * would create an unbreakable circular reference. */
    {
        const PLAIN_BYTE* self_name = (const PLAIN_BYTE*)"self";
        PLAIN_WORD_DOUBLE self_len = 4;
        struct PLAIN_BINDING* self_binding = NULL;
        HASH_FIND(hh, frame->bindings, self_name, self_len, self_binding);

        if(self_binding == NULL) {
            self_binding = (struct PLAIN_BINDING*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_BINDING), 0);
            if(self_binding != NULL) {
                memset(self_binding, 0, sizeof(struct PLAIN_BINDING));
                self_binding->name = (PLAIN_BYTE*)PLAIN_RESIZE(NULL, self_len + 1, 0);
                if(self_binding->name != NULL) {
                    memcpy(self_binding->name, self_name, self_len + 1);
                    HASH_ADD_KEYPTR(hh, frame->bindings, self_binding->name, self_len, self_binding);
                }
            }
        }

        if(self_binding != NULL) {
            self_binding->value.type   = PLAIN_TYPE_OBJECT;
            self_binding->value.data   = (PLAIN_BYTE*)frame;
            self_binding->value.length = 0;
            self_binding->value.owner  = PLAIN_OWNER_USER;
            self_binding->flags        = PLAIN_BINDING_IMMUTABLE;
        }
    }

    /* Return the frame as an object value. */
    if(value != NULL) {
        value->type   = PLAIN_TYPE_OBJECT;
        value->data   = (PLAIN_BYTE*)frame;
        value->length = 0;
        value->owner  = PLAIN_OWNER_USER;
    }
    PLAIN_FRAME_RELEASE(frame);
    return 0;
}

/* ---- Logic: not, and, or ----------------------------------------- */

/* not value — boolean negation. */
static PLAIN_WORD_DOUBLE PLAIN_FW_NOT(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    PLAIN_VALUE a = PLAIN_RESOLVE_ARGUMENT(context, node, 0);
    return PLAIN_FW_SET_BOOLEAN(value, !PLAIN_VALUE_TRUTHY(&a));
}

/* and arg1, arg2, ... — variadic short-circuit conjunction. */
static PLAIN_WORD_DOUBLE PLAIN_FW_AND(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    for(PLAIN_WORD_DOUBLE i = 0; i < arity; i++) {
        PLAIN_VALUE arg = PLAIN_RESOLVE_ARGUMENT(context, node, i);
        if(!PLAIN_VALUE_TRUTHY(&arg)) {
            return PLAIN_FW_SET_BOOLEAN(value, 0);
        }
    }
    return PLAIN_FW_SET_BOOLEAN(value, 1);
}

/* or arg1, arg2, ... — variadic short-circuit disjunction. */
static PLAIN_WORD_DOUBLE PLAIN_FW_OR(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    for(PLAIN_WORD_DOUBLE i = 0; i < arity; i++) {
        PLAIN_VALUE arg = PLAIN_RESOLVE_ARGUMENT(context, node, i);
        if(PLAIN_VALUE_TRUTHY(&arg)) {
            return PLAIN_FW_SET_BOOLEAN(value, 1);
        }
    }
    return PLAIN_FW_SET_BOOLEAN(value, 0);
}

/* ---- Arithmetic: add/+, subtract/-, multiply/*, divide//, modulo/% */
/* Each delegates to the variadic fold helper. */

static PLAIN_WORD_DOUBLE PLAIN_FW_ADD(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    return PLAIN_FW_ARITHMETIC_FOLD(context, node, '+', value);
}

static PLAIN_WORD_DOUBLE PLAIN_FW_SUBTRACT(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    return PLAIN_FW_ARITHMETIC_FOLD(context, node, '-', value);
}

static PLAIN_WORD_DOUBLE PLAIN_FW_MULTIPLY(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    return PLAIN_FW_ARITHMETIC_FOLD(context, node, '*', value);
}

static PLAIN_WORD_DOUBLE PLAIN_FW_DIVIDE(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    return PLAIN_FW_ARITHMETIC_FOLD(context, node, '/', value);
}

static PLAIN_WORD_DOUBLE PLAIN_FW_MODULO(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    return PLAIN_FW_ARITHMETIC_FOLD(context, node, '%', value);
}

/* ---- Comparison: =, !=, <, >, <=, >= ----------------------------- */
/* Each resolves two arguments and delegates to PLAIN_FW_APPLY_COMPARISON.
 * !=, <=, >= are implemented by negating the complementary operation. */

static PLAIN_WORD_DOUBLE PLAIN_FW_EQUAL(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    PLAIN_VALUE a = PLAIN_RESOLVE_ARGUMENT(context, node, 0);
    PLAIN_VALUE b = PLAIN_RESOLVE_ARGUMENT(context, node, 1);
    return PLAIN_FW_APPLY_COMPARISON(&a, &b, '=', value);
}

static PLAIN_WORD_DOUBLE PLAIN_FW_LESS(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    PLAIN_VALUE a = PLAIN_RESOLVE_ARGUMENT(context, node, 0);
    PLAIN_VALUE b = PLAIN_RESOLVE_ARGUMENT(context, node, 1);
    return PLAIN_FW_APPLY_COMPARISON(&a, &b, '<', value);
}

static PLAIN_WORD_DOUBLE PLAIN_FW_GREATER(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    PLAIN_VALUE a = PLAIN_RESOLVE_ARGUMENT(context, node, 0);
    PLAIN_VALUE b = PLAIN_RESOLVE_ARGUMENT(context, node, 1);
    return PLAIN_FW_APPLY_COMPARISON(&a, &b, '>', value);
}

/* != is !(a = b). */
static PLAIN_WORD_DOUBLE PLAIN_FW_NOT_EQUAL(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    PLAIN_VALUE a = PLAIN_RESOLVE_ARGUMENT(context, node, 0);
    PLAIN_VALUE b = PLAIN_RESOLVE_ARGUMENT(context, node, 1);
    PLAIN_VALUE temp = {NULL, 0, PLAIN_TYPE_NIL, 0};
    PLAIN_WORD_DOUBLE error = PLAIN_FW_APPLY_COMPARISON(&a, &b, '=', &temp);
    if(error == 0) {
        error = PLAIN_FW_SET_BOOLEAN(value, !PLAIN_VALUE_TRUTHY(&temp));
    }
    PLAIN_VALUE_CLEAR(&temp);
    return error;
}

/* <= is !(a > b). */
static PLAIN_WORD_DOUBLE PLAIN_FW_LESS_EQUAL(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    PLAIN_VALUE a = PLAIN_RESOLVE_ARGUMENT(context, node, 0);
    PLAIN_VALUE b = PLAIN_RESOLVE_ARGUMENT(context, node, 1);
    PLAIN_VALUE temp = {NULL, 0, PLAIN_TYPE_NIL, 0};
    PLAIN_WORD_DOUBLE error = PLAIN_FW_APPLY_COMPARISON(&a, &b, '>', &temp);
    if(error == 0) {
        error = PLAIN_FW_SET_BOOLEAN(value, !PLAIN_VALUE_TRUTHY(&temp));
    }
    PLAIN_VALUE_CLEAR(&temp);
    return error;
}

/* >= is !(a < b). */
static PLAIN_WORD_DOUBLE PLAIN_FW_GREATER_EQUAL(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    PLAIN_VALUE a = PLAIN_RESOLVE_ARGUMENT(context, node, 0);
    PLAIN_VALUE b = PLAIN_RESOLVE_ARGUMENT(context, node, 1);
    PLAIN_VALUE temp = {NULL, 0, PLAIN_TYPE_NIL, 0};
    PLAIN_WORD_DOUBLE error = PLAIN_FW_APPLY_COMPARISON(&a, &b, '<', &temp);
    if(error == 0) {
        error = PLAIN_FW_SET_BOOLEAN(value, !PLAIN_VALUE_TRUTHY(&temp));
    }
    PLAIN_VALUE_CLEAR(&temp);
    return error;
}

/* ---- join -------------------------------------------------------- */
/* join arg1, arg2, ... — variadic string concatenation.
 * Each argument is resolved and coerced to a string representation:
 * integers and reals are formatted with sprintf, booleans become
 * "yes"/"no", and nil becomes "none". */
static PLAIN_WORD_DOUBLE PLAIN_FW_CONCATENATE(PLAIN_CONTEXT* context, PLAIN_LIST* node, PLAIN_WORD_DOUBLE type, PLAIN_VALUE* value) {
    if(value == NULL) {
        return 0;
    }

    struct PLAIN_SEGMENT result = {NULL, NULL};
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);

    for(PLAIN_WORD_DOUBLE i = 0; i < arity; i++) {
        PLAIN_VALUE argument = PLAIN_RESOLVE_ARGUMENT(context, node, i);
        PLAIN_BYTE buffer[64];
        const PLAIN_BYTE* str = (const PLAIN_BYTE*)"";
        PLAIN_WORD_DOUBLE length = 0;

        switch(argument.type) {
            case PLAIN_TYPE_STRING:
                str = argument.data;
                length = (PLAIN_WORD_DOUBLE)strlen((const char*)str);
                break;
            case PLAIN_TYPE_INTEGER:
                length = sprintf((char*)buffer, "%lld", (long long)*(PLAIN_WORD_QUADRUPLE*)argument.data);
                str = buffer;
                break;
            case PLAIN_TYPE_REAL:
                length = sprintf((char*)buffer, "%g", (PLAIN_REAL_DOUBLE)*(PLAIN_REAL*)argument.data);
                str = buffer;
                break;
            case PLAIN_TYPE_BOOLEAN:
                str = argument.data != NULL ? (const PLAIN_BYTE*)"yes" : (const PLAIN_BYTE*)"no";
                length = (PLAIN_WORD_DOUBLE)strlen((const char*)str);
                break;
            default:
                str = (const PLAIN_BYTE*)"none";
                length = 4;
                break;
        }

        if(length > 0) {
            PLAIN_CONCATENATE(&result, length, str);
        }
    }

    /* Null-terminate the result string. */
    PLAIN_BYTE zero = 0;
    PLAIN_CONCATENATE(&result, 1, &zero);

    value->data   = result.from;
    value->length = result.to - result.from;
    value->type   = PLAIN_TYPE_STRING;
    return 0;
}

/* ================================================================== */
/*  Framework registration                                            */
/* ================================================================== */

/* Registers all built-in procedures as mutable callables in the root frame.
 * Each name+symbol pair shares the same implementation; both can be
 * independently overridden by user code. */
PLAIN_WORD_DOUBLE PLAIN_FRAMEWORK_REGISTER(PLAIN_CONTEXT* context) {
    /* Control flow. */
    PLAIN_INTERNAL_REGISTER("if",            PLAIN_FW_IF);
    PLAIN_INTERNAL_REGISTER("when",          PLAIN_FW_WHEN);
    PLAIN_INTERNAL_REGISTER("repeat",        PLAIN_FW_REPEAT);
    PLAIN_INTERNAL_REGISTER("return",        PLAIN_FW_RETURN);
    PLAIN_INTERNAL_REGISTER("break",         PLAIN_FW_BREAK);
    PLAIN_INTERNAL_REGISTER("continue",      PLAIN_FW_CONTINUE);

    /* Scoping and definitions. */
    PLAIN_INTERNAL_REGISTER("do",            PLAIN_FW_DO);
    PLAIN_INTERNAL_REGISTER("type",          PLAIN_FW_TYPE);
    PLAIN_INTERNAL_REGISTER("set",           PLAIN_FW_SET);
    PLAIN_INTERNAL_REGISTER("define",        PLAIN_FW_DEFINE);
    PLAIN_INTERNAL_REGISTER("object",        PLAIN_FW_OBJECT);

    /* Logic. */
    PLAIN_INTERNAL_REGISTER("not",           PLAIN_FW_NOT);
    PLAIN_INTERNAL_REGISTER("and",           PLAIN_FW_AND);
    PLAIN_INTERNAL_REGISTER("or",            PLAIN_FW_OR);

    /* Arithmetic — named and symbol forms. */
    PLAIN_INTERNAL_REGISTER("add",           PLAIN_FW_ADD);
    PLAIN_INTERNAL_REGISTER("+",             PLAIN_FW_ADD);
    PLAIN_INTERNAL_REGISTER("subtract",      PLAIN_FW_SUBTRACT);
    PLAIN_INTERNAL_REGISTER("-",             PLAIN_FW_SUBTRACT);
    PLAIN_INTERNAL_REGISTER("multiply",      PLAIN_FW_MULTIPLY);
    PLAIN_INTERNAL_REGISTER("*",             PLAIN_FW_MULTIPLY);
    PLAIN_INTERNAL_REGISTER("divide",        PLAIN_FW_DIVIDE);
    PLAIN_INTERNAL_REGISTER("/",             PLAIN_FW_DIVIDE);
    PLAIN_INTERNAL_REGISTER("modulo",        PLAIN_FW_MODULO);
    PLAIN_INTERNAL_REGISTER("%",             PLAIN_FW_MODULO);

    /* Comparison — named and symbol forms. */
    PLAIN_INTERNAL_REGISTER("equal",         PLAIN_FW_EQUAL);
    PLAIN_INTERNAL_REGISTER("=",             PLAIN_FW_EQUAL);
    PLAIN_INTERNAL_REGISTER("not_equal",     PLAIN_FW_NOT_EQUAL);
    PLAIN_INTERNAL_REGISTER("!=",            PLAIN_FW_NOT_EQUAL);
    PLAIN_INTERNAL_REGISTER("less",          PLAIN_FW_LESS);
    PLAIN_INTERNAL_REGISTER("<",             PLAIN_FW_LESS);
    PLAIN_INTERNAL_REGISTER("greater",       PLAIN_FW_GREATER);
    PLAIN_INTERNAL_REGISTER(">",             PLAIN_FW_GREATER);
    PLAIN_INTERNAL_REGISTER("less_equal",    PLAIN_FW_LESS_EQUAL);
    PLAIN_INTERNAL_REGISTER("<=",            PLAIN_FW_LESS_EQUAL);
    PLAIN_INTERNAL_REGISTER("greater_equal", PLAIN_FW_GREATER_EQUAL);
    PLAIN_INTERNAL_REGISTER(">=",            PLAIN_FW_GREATER_EQUAL);

    /* String operations. */
    PLAIN_INTERNAL_REGISTER("join",          PLAIN_FW_CONCATENATE);

    return 0;
}
