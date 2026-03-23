/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     11/01/2014.
 * Revision 03/22/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 */

#include <Plain/VM.h>
#include <Plain/Framework/Scope.h>
#include <uthash.h>
#include <stdio.h>   /* sprintf — string formatting only, no output */

/* ------------------------------------------------------------------ */
/*  Helpers                                                           */
/* ------------------------------------------------------------------ */

PLAIN_INLINE PLAIN_WORD_DOUBLE PLAIN_KEYWORD_IS(struct PLAIN_LIST* node, const PLAIN_BYTE* name) {
    PLAIN_WORD_DOUBLE length = node->keyword.to - node->keyword.from;
    return length == strlen((const char*)name) && memcmp(node->keyword.from, name, length) == 0;
}

PLAIN_INLINE PLAIN_WORD_DOUBLE PLAIN_KEYWORD_EQ(struct PLAIN_VALUE* value, const PLAIN_BYTE* name) {
    return value != NULL && value->type == PLAIN_TYPE_KEYWORD && strcmp((const char*)value->data, (const char*)name) == 0;
}

PLAIN_INLINE void PLAIN_KEYWORD_EXTRACT(struct PLAIN_LIST* node, PLAIN_BYTE* buffer, PLAIN_WORD_DOUBLE size) {
    PLAIN_WORD_DOUBLE length = node->keyword.to - node->keyword.from;
    if(length >= size) length = size - 1;
    memcpy(buffer, node->keyword.from, length);
    buffer[length] = '\0';
}

/* Resolves the node keyword as a value: checks the frame first, then tries
 * to parse it as a numeric literal. Returns nil if neither matches. */
static PLAIN_WORD_DOUBLE PLAIN_KEYWORD_AS_VALUE(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, struct PLAIN_VALUE* destination) {
    PLAIN_BYTE buffer[256];
    PLAIN_KEYWORD_EXTRACT(node, buffer, sizeof(buffer));
    struct PLAIN_BINDING* binding = PLAIN_FRAME_FIND(context->frame, buffer);
    if(binding != NULL) {
        if(binding->callable != NULL) {
            destination->type = PLAIN_TYPE_CALLABLE;
            destination->data = (PLAIN_BYTE*)binding->callable;
            destination->length = 0;
            return 0;
        }
        return PLAIN_VALUE_COPY(destination, &binding->value);
    }
    /* Numeric literal fallback. */
    PLAIN_WORD_DOUBLE length = strlen((const char*)buffer);
    if(length > 0) {
        PLAIN_BYTE first = buffer[0];
        if(isdigit(first) || ((first == '+' || first == '-') && length > 1 && isdigit(buffer[1]))) {
            PLAIN_WORD_DOUBLE i = 0;
            for(; i < length; i++) {
                if(buffer[i] == '.' || buffer[i] == 'e' || buffer[i] == 'E') {
                    PLAIN_REAL f = (PLAIN_REAL)atof((const char*)buffer);
                    return PLAIN_EXPORT((PLAIN_BYTE*)&f, sizeof(PLAIN_REAL), PLAIN_TYPE_REAL, destination);
                }
            }
            PLAIN_WORD_DOUBLE n = (PLAIN_WORD_DOUBLE)atoi((const char*)buffer);
            return PLAIN_EXPORT((PLAIN_BYTE*)&n, sizeof(PLAIN_WORD_DOUBLE), PLAIN_TYPE_INTEGER, destination);
        }
    }
    destination->type = PLAIN_TYPE_NIL;
    destination->data = NULL;
    destination->length = 0;
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Operator dispatch table                                           */
/* ------------------------------------------------------------------ */

/* Integer tag for each operator — used in the dispatch switch. */
enum PLAIN_OPERATOR {
    PLAIN_OP_ADD,
    PLAIN_OP_SUBTRACT,
    PLAIN_OP_MULTIPLY,
    PLAIN_OP_DIVIDE,
    PLAIN_OP_MODULO,
    PLAIN_OP_EQUAL,
    PLAIN_OP_NOT_EQUAL,
    PLAIN_OP_LESS,
    PLAIN_OP_GREATER,
    PLAIN_OP_LESS_EQUAL,
    PLAIN_OP_GREATER_EQUAL,
    PLAIN_OP_AND,
    PLAIN_OP_OR
};

struct PLAIN_OPERATOR_ENTRY {
    const char* name;
    enum PLAIN_OPERATOR operation;
    UT_hash_handle hh;
};

static struct PLAIN_OPERATOR_ENTRY* PLAIN_OPERATOR_TABLE = NULL;

/* Populates the global operator table once. Called lazily on first use. */
static void PLAIN_OPERATOR_TABLE_INIT(void) {
    static const struct { const char* name; enum PLAIN_OPERATOR operation; } source[] = {
        { "+",   PLAIN_OP_ADD           },
        { "-",   PLAIN_OP_SUBTRACT      },
        { "*",   PLAIN_OP_MULTIPLY      },
        { "/",   PLAIN_OP_DIVIDE        },
        { "%",   PLAIN_OP_MODULO        },
        { "=",   PLAIN_OP_EQUAL         },
        { "!=",  PLAIN_OP_NOT_EQUAL     },
        { "<",   PLAIN_OP_LESS          },
        { ">",   PLAIN_OP_GREATER       },
        { "<=",  PLAIN_OP_LESS_EQUAL    },
        { ">=",  PLAIN_OP_GREATER_EQUAL },
        { "and", PLAIN_OP_AND           },
        { "or",  PLAIN_OP_OR            },
    };
    PLAIN_WORD_DOUBLE i;
    for(i = 0; i < 13; i++) {
        struct PLAIN_OPERATOR_ENTRY* entry = (struct PLAIN_OPERATOR_ENTRY*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_OPERATOR_ENTRY), 0);
        entry->name      = source[i].name;
        entry->operation = source[i].operation;
        HASH_ADD_KEYPTR(hh, PLAIN_OPERATOR_TABLE, entry->name, strlen(entry->name), entry);
    }
}

/* Returns the operator entry for <name>, or NULL if it is not an operator symbol. */
PLAIN_INLINE struct PLAIN_OPERATOR_ENTRY* PLAIN_OPERATOR_FIND(const PLAIN_BYTE* name) {
    if(PLAIN_OPERATOR_TABLE == NULL) PLAIN_OPERATOR_TABLE_INIT();
    struct PLAIN_OPERATOR_ENTRY* entry = NULL;
    HASH_FIND_STR(PLAIN_OPERATOR_TABLE, (const char*)name, entry);
    return entry;
}

/* Returns nonzero if <name> is a known infix operator symbol. */
PLAIN_INLINE PLAIN_WORD_DOUBLE PLAIN_IS_OPERATOR_SYMBOL(const PLAIN_BYTE* name) {
    return PLAIN_OPERATOR_FIND(name) != NULL;
}

/* Returns nonzero if <argument> is a keyword whose text is a known infix operator. */
PLAIN_INLINE PLAIN_WORD_DOUBLE PLAIN_IS_INFIX_OPERATOR(struct PLAIN_VALUE* argument) {
    if(argument == NULL || argument->type != PLAIN_TYPE_KEYWORD) return 0;
    return PLAIN_IS_OPERATOR_SYMBOL(argument->data);
}

static PLAIN_WORD_DOUBLE PLAIN_EVALUATE_BLOCK(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* block, struct PLAIN_VALUE* value) {
    if(block == NULL || block->segment.from == NULL || block->segment.from >= block->segment.to) return 0;
    PLAIN_WORD_DOUBLE length = block->segment.to - block->segment.from;
    PLAIN_BYTE* source = (PLAIN_BYTE*)PLAIN_AUTO(length + 1);
    if(source == NULL) return PLAIN_ERROR_SYSTEM;
    memcpy(source, block->segment.from, length);
    source[length] = '\0';
    return PLAIN_EVALUATE(&context->environment, &PLAIN_RESOLVE, source, context->tracker, value);
}

static PLAIN_BYTE* PLAIN_SEGMENT_COPY(struct PLAIN_LIST* block) {
    if(block == NULL || block->segment.from == NULL || block->segment.from >= block->segment.to) {
        PLAIN_BYTE* empty = (PLAIN_BYTE*)PLAIN_RESIZE(NULL, 1, 0);
        if(empty) empty[0] = '\0';
        return empty;
    }
    PLAIN_WORD_DOUBLE length = block->segment.to - block->segment.from;
    PLAIN_BYTE* data = (PLAIN_BYTE*)PLAIN_RESIZE(NULL, length + 1, 0);
    if(data) { memcpy(data, block->segment.from, length); data[length] = '\0'; }
    return data;
}

PLAIN_INLINE PLAIN_WORD_DOUBLE PLAIN_SET_INTEGER(struct PLAIN_VALUE* value, PLAIN_WORD_DOUBLE number) {
    if(value == NULL) return 0;
    return PLAIN_EXPORT((PLAIN_BYTE*)&number, sizeof(PLAIN_WORD_DOUBLE), PLAIN_TYPE_INTEGER, value);
}

PLAIN_INLINE PLAIN_WORD_DOUBLE PLAIN_SET_BOOLEAN(struct PLAIN_VALUE* value, PLAIN_WORD_DOUBLE boolean) {
    if(value == NULL) return 0;
    return PLAIN_EXPORT(boolean ? (PLAIN_BYTE*)0xFF : NULL, 0, PLAIN_TYPE_BOOLEAN, value);
}

PLAIN_INLINE PLAIN_REAL_DOUBLE PLAIN_AS_REAL(struct PLAIN_VALUE* value) {
    if(value->type == PLAIN_TYPE_REAL)    return (PLAIN_REAL_DOUBLE)*(PLAIN_REAL*)value->data;
    if(value->type == PLAIN_TYPE_INTEGER) return (PLAIN_REAL_DOUBLE)(int)*(PLAIN_WORD_DOUBLE*)value->data;
    if(value->type == PLAIN_TYPE_BOOLEAN) return value->data != NULL ? 1.0 : 0.0;
    return 0.0;
}

/* ------------------------------------------------------------------ */
/*  Core operations (work on two resolved values directly)           */
/* ------------------------------------------------------------------ */

static PLAIN_WORD_DOUBLE PLAIN_APPLY_ARITHMETIC(struct PLAIN_VALUE* a, struct PLAIN_VALUE* b, PLAIN_BYTE operation, struct PLAIN_VALUE* value) {
    if(a->type == PLAIN_TYPE_REAL || b->type == PLAIN_TYPE_REAL) {
        PLAIN_REAL_DOUBLE va = PLAIN_AS_REAL(a), vb = PLAIN_AS_REAL(b), result = 0;
        switch(operation) {
            case '+': result = va + vb; break;
            case '-': result = va - vb; break;
            case '*': result = va * vb; break;
            case '/': result = vb != 0 ? va / vb : 0; break;
        }
        PLAIN_REAL f = (PLAIN_REAL)result;
        if(value != NULL) return PLAIN_EXPORT((PLAIN_BYTE*)&f, sizeof(PLAIN_REAL), PLAIN_TYPE_REAL, value);
    } else {
        int va = (int)*(PLAIN_WORD_DOUBLE*)a->data;
        int vb = (int)*(PLAIN_WORD_DOUBLE*)b->data;
        int result = 0;
        switch(operation) {
            case '+': result = va + vb; break;
            case '-': result = va - vb; break;
            case '*': result = va * vb; break;
            case '/': result = vb != 0 ? va / vb : 0; break;
            case '%': result = vb != 0 ? va % vb : 0; break;
        }
        if(value != NULL) return PLAIN_SET_INTEGER(value, (PLAIN_WORD_DOUBLE)result);
    }
    return 0;
}

static PLAIN_WORD_DOUBLE PLAIN_APPLY_COMPARISON(struct PLAIN_VALUE* a, struct PLAIN_VALUE* b, PLAIN_BYTE operation, struct PLAIN_VALUE* value) {
    if(a->type == PLAIN_TYPE_STRING && b->type == PLAIN_TYPE_STRING) {
        int comparison = strcmp((const char*)a->data, (const char*)b->data);
        PLAIN_WORD_DOUBLE result = 0;
        switch(operation) {
            case '=': result = comparison == 0; break;
            case '<': result = comparison < 0;  break;
            case '>': result = comparison > 0;  break;
        }
        return PLAIN_SET_BOOLEAN(value, result);
    }
    PLAIN_REAL_DOUBLE va = PLAIN_AS_REAL(a), vb = PLAIN_AS_REAL(b);
    PLAIN_WORD_DOUBLE result = 0;
    switch(operation) {
        case '=': result = va == vb; break;
        case '<': result = va < vb;  break;
        case '>': result = va > vb;  break;
    }
    return PLAIN_SET_BOOLEAN(value, result);
}

/* ------------------------------------------------------------------ */
/*  Callable dispatch                                                 */
/* ------------------------------------------------------------------ */

PLAIN_WORD_DOUBLE PLAIN_CALL(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, struct PLAIN_CALLABLE* callable, struct PLAIN_VALUE* value) {
    /* Native callable — call it directly. The node arrives with arguments
     * already resolved (expressions substituted, blocks left unevaluated). */
    if(callable->native != NULL) {
        return callable->native((void*)context, (void*)node, PLAIN_TYPE_LIST, value);
    }
    /* User-defined callable — bind parameters and evaluate body.
     * The child frame parents the closure frame (lexical scope). */
    struct PLAIN_FRAME* frame = PLAIN_FRAME_CREATE(callable->closure != NULL ? callable->closure : context->frame);
    if(frame == NULL) return PLAIN_ERROR_SYSTEM;
    const PLAIN_BYTE* pointer = callable->parameters;
    PLAIN_WORD_DOUBLE index = 0;
    while(pointer != NULL && *pointer != '\0') {
        while(*pointer == ' ' || *pointer == '\t' || *pointer == ',' || *pointer == '\n' || *pointer == '\r') pointer++;
        if(*pointer == '\0') break;
        const PLAIN_BYTE* start = pointer;
        while(*pointer != '\0' && *pointer != ' ' && *pointer != '\t' && *pointer != ',' && *pointer != '\n' && *pointer != '\r') pointer++;
        PLAIN_BYTE name[256];
        PLAIN_WORD_DOUBLE length = pointer - start;
        if(length >= sizeof(name)) length = sizeof(name) - 1;
        memcpy(name, start, length);
        name[length] = '\0';
        struct PLAIN_VALUE* argument = PLAIN_ARGUMENT(node, index);
        if(argument != NULL) PLAIN_FRAME_BIND(frame, name, argument, NULL, 0);
        index++;
    }
    struct PLAIN_FRAME* saved = context->frame;
    context->frame = frame;
    struct PLAIN_VALUE body = {NULL, 0, PLAIN_TYPE_NIL};
    PLAIN_WORD_DOUBLE error = PLAIN_EVALUATE(&context->environment, &PLAIN_RESOLVE, callable->body, context->tracker, &body);
    context->frame = saved;

    /* Class constructor: the frame becomes the object instance. */
    if(callable->flags & PLAIN_CALLABLE_CLASS) {
        PLAIN_VALUE_CLEAR(&body);
        PLAIN_VALUE_CLEAR(&context->result);
        /* Break circular closure references: methods defined in the class body
         * retain this frame as their closure. Undo those retains so the frame's
         * ref count only reflects external holders (object values). */
        struct PLAIN_BINDING* b;
        struct PLAIN_BINDING* t;
        HASH_ITER(hh, frame->bindings, b, t) {
            if(b->callable != NULL && b->callable->closure == frame) {
                frame->references--;
            }
        }
        PLAIN_FRAME_RETAIN(frame);
        /* Bind `self` directly — set the value without PLAIN_VALUE_COPY to avoid
         * an extra retain that would create an unbreakable circular reference. */
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
                self_binding->value.type = PLAIN_TYPE_OBJECT;
                self_binding->value.data = (PLAIN_BYTE*)frame;
                self_binding->value.length = PLAIN_OBJECT_NATIVE;
                self_binding->flags = PLAIN_BINDING_IMMUTABLE;
            }
        }
        if(value != NULL) {
            value->type = PLAIN_TYPE_OBJECT;
            value->data = (PLAIN_BYTE*)frame;
            value->length = PLAIN_OBJECT_NATIVE;
        }
        PLAIN_FRAME_RELEASE(frame);
        return (error == PLAIN_SIGNAL_RETURN) ? 0 : error;
    }

    PLAIN_FRAME_RELEASE(frame);
    if(error == PLAIN_SIGNAL_RETURN) {
        if(value != NULL) { *value = context->result; context->result = (struct PLAIN_VALUE){NULL, 0, PLAIN_TYPE_NIL}; }
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
    const PLAIN_BYTE* pointer = callable->parameters;
    PLAIN_WORD_DOUBLE index = 0;
    while(pointer != NULL && *pointer != '\0') {
        while(*pointer == ' ' || *pointer == '\t' || *pointer == ',' || *pointer == '\n' || *pointer == '\r') pointer++;
        if(*pointer == '\0') break;
        const PLAIN_BYTE* start = pointer;
        while(*pointer != '\0' && *pointer != ' ' && *pointer != '\t' && *pointer != ',' && *pointer != '\n' && *pointer != '\r') pointer++;
        PLAIN_BYTE name[256];
        PLAIN_WORD_DOUBLE length = pointer - start;
        if(length >= sizeof(name)) length = sizeof(name) - 1;
        memcpy(name, start, length);
        name[length] = '\0';
        struct PLAIN_VALUE* argument = PLAIN_ARGUMENT(node, offset + index);
        if(argument != NULL) PLAIN_FRAME_BIND(frame, name, argument, NULL, 0);
        index++;
    }
    struct PLAIN_FRAME* saved = context->frame;
    context->frame = frame;
    struct PLAIN_VALUE body = {NULL, 0, PLAIN_TYPE_NIL};
    PLAIN_WORD_DOUBLE error = PLAIN_EVALUATE(&context->environment, &PLAIN_RESOLVE, callable->body, context->tracker, &body);
    context->frame = saved;
    PLAIN_FRAME_RELEASE(frame);
    if(error == PLAIN_SIGNAL_RETURN) {
        if(value != NULL) { *value = context->result; context->result = (struct PLAIN_VALUE){NULL, 0, PLAIN_TYPE_NIL}; }
        else { PLAIN_VALUE_CLEAR(&context->result); }
        PLAIN_VALUE_CLEAR(&body);
        return 0;
    }
    if(error == 0 && value != NULL) { *value = body; }
    else { PLAIN_VALUE_CLEAR(&body); }
    return error;
}

/* ------------------------------------------------------------------ */
/*  Assignment and infix                                              */
/* ------------------------------------------------------------------ */

/* Deep-copies a callable struct including its parameter and body strings. */
static struct PLAIN_CALLABLE* PLAIN_CALLABLE_DUPLICATE(struct PLAIN_CALLABLE* source) {
    struct PLAIN_CALLABLE* copy = (struct PLAIN_CALLABLE*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_CALLABLE), 0);
    if(copy == NULL) return NULL;
    memset(copy, 0, sizeof(struct PLAIN_CALLABLE));
    copy->native  = source->native;
    copy->closure = source->closure;
    copy->flags   = source->flags;
    if(copy->closure != NULL) PLAIN_FRAME_RETAIN(copy->closure);
    if(source->parameters != NULL) {
        PLAIN_WORD_DOUBLE length = strlen((const char*)source->parameters) + 1;
        copy->parameters = (PLAIN_BYTE*)PLAIN_RESIZE(NULL, length, 0);
        if(copy->parameters) memcpy(copy->parameters, source->parameters, length);
    }
    if(source->body != NULL) {
        PLAIN_WORD_DOUBLE length = strlen((const char*)source->body) + 1;
        copy->body = (PLAIN_BYTE*)PLAIN_RESIZE(NULL, length, 0);
        if(copy->body) memcpy(copy->body, source->body, length);
    }
    return copy;
}

static PLAIN_WORD_DOUBLE PLAIN_BUILTIN_ASSIGN(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, struct PLAIN_VALUE* value) {
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    if(arity < 2) return 0;
    PLAIN_BYTE name[256];
    PLAIN_KEYWORD_EXTRACT(node, name, sizeof(name));
    struct PLAIN_VALUE* second = PLAIN_ARGUMENT(node, 1);
    /* x = [function/procedure {params} {body}] — callable value from an expression */
    if(second->type == PLAIN_TYPE_CALLABLE && second->data != NULL) {
        struct PLAIN_CALLABLE* source = (struct PLAIN_CALLABLE*)second->data;
        struct PLAIN_CALLABLE* copy = PLAIN_CALLABLE_DUPLICATE(source);
        if(copy == NULL) return PLAIN_ERROR_SYSTEM;
        PLAIN_WORD_DOUBLE flags = (source->flags & PLAIN_CALLABLE_IMMUTABLE) ? PLAIN_BINDING_IMMUTABLE : 0;
        return PLAIN_FRAME_SET(context->frame, name, NULL, copy, flags);
    }
    /* x = value */
    PLAIN_FRAME_SET(context->frame, name, second, NULL, 0);
    if(value != NULL) PLAIN_VALUE_COPY(value, second);
    return 0;
}

/* Handles all infix expressions: name op value.
 * "=" in statement context (no parent node) always means assignment.
 * For all other operators, the frame is checked first so users can override
 * any operator by binding a callable to its symbol name. */
static PLAIN_WORD_DOUBLE PLAIN_BUILTIN_INFIX(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, struct PLAIN_VALUE* value) {
    struct PLAIN_VALUE* arg0 = PLAIN_ARGUMENT(node, 0);
    struct PLAIN_VALUE* right = PLAIN_ARGUMENT(node, 1);
    const PLAIN_BYTE* op = arg0->data;

    /* Assignment: name = value (statement context only, never overridable). */
    struct PLAIN_OPERATOR_ENTRY* entry = PLAIN_OPERATOR_FIND(op);
    if(entry != NULL && entry->operation == PLAIN_OP_EQUAL && node->parent == NULL) {
        return PLAIN_BUILTIN_ASSIGN(context, node, value);
    }

    /* User override: if the operator symbol is bound to a callable in the frame,
     * call it with (left, right) as its two parameters. */
    struct PLAIN_BINDING* override = PLAIN_FRAME_FIND(context->frame, op);
    if(override != NULL && override->callable != NULL) {
        struct PLAIN_VALUE left = {NULL, 0, PLAIN_TYPE_NIL};
        PLAIN_WORD_DOUBLE error = PLAIN_KEYWORD_AS_VALUE(context, node, &left);
        if(error != 0) return error;
        *arg0 = left;
        return PLAIN_CALL(context, node, override->callable, value);
    }

    /* Built-in operator logic — dispatch via the operator table entry. */
    struct PLAIN_VALUE left = {NULL, 0, PLAIN_TYPE_NIL};
    PLAIN_WORD_DOUBLE error = PLAIN_KEYWORD_AS_VALUE(context, node, &left);
    if(error != 0) return error;
    if(entry != NULL) {
        switch(entry->operation) {
            case PLAIN_OP_ADD:      error = PLAIN_APPLY_ARITHMETIC(&left, right, '+', value); break;
            case PLAIN_OP_SUBTRACT: error = PLAIN_APPLY_ARITHMETIC(&left, right, '-', value); break;
            case PLAIN_OP_MULTIPLY: error = PLAIN_APPLY_ARITHMETIC(&left, right, '*', value); break;
            case PLAIN_OP_DIVIDE:   error = PLAIN_APPLY_ARITHMETIC(&left, right, '/', value); break;
            case PLAIN_OP_MODULO:   error = PLAIN_APPLY_ARITHMETIC(&left, right, '%', value); break;
            case PLAIN_OP_EQUAL:    error = PLAIN_APPLY_COMPARISON(&left, right, '=', value); break;
            case PLAIN_OP_LESS:     error = PLAIN_APPLY_COMPARISON(&left, right, '<', value); break;
            case PLAIN_OP_GREATER:  error = PLAIN_APPLY_COMPARISON(&left, right, '>', value); break;
            case PLAIN_OP_NOT_EQUAL: {
                struct PLAIN_VALUE temp = {NULL, 0, PLAIN_TYPE_NIL};
                error = PLAIN_APPLY_COMPARISON(&left, right, '=', &temp);
                if(error == 0) error = PLAIN_SET_BOOLEAN(value, !PLAIN_IS_TRUE(&temp));
                PLAIN_VALUE_CLEAR(&temp);
                break;
            }
            case PLAIN_OP_LESS_EQUAL: {
                struct PLAIN_VALUE temp = {NULL, 0, PLAIN_TYPE_NIL};
                error = PLAIN_APPLY_COMPARISON(&left, right, '>', &temp);
                if(error == 0) error = PLAIN_SET_BOOLEAN(value, !PLAIN_IS_TRUE(&temp));
                PLAIN_VALUE_CLEAR(&temp);
                break;
            }
            case PLAIN_OP_GREATER_EQUAL: {
                struct PLAIN_VALUE temp = {NULL, 0, PLAIN_TYPE_NIL};
                error = PLAIN_APPLY_COMPARISON(&left, right, '<', &temp);
                if(error == 0) error = PLAIN_SET_BOOLEAN(value, !PLAIN_IS_TRUE(&temp));
                PLAIN_VALUE_CLEAR(&temp);
                break;
            }
            case PLAIN_OP_AND: error = PLAIN_SET_BOOLEAN(value, PLAIN_IS_TRUE(&left) && PLAIN_IS_TRUE(right)); break;
            case PLAIN_OP_OR:  error = PLAIN_SET_BOOLEAN(value, PLAIN_IS_TRUE(&left) || PLAIN_IS_TRUE(right)); break;
        }
    }
    PLAIN_VALUE_CLEAR(&left);
    return error;
}

/* ------------------------------------------------------------------ */
/*  Native callables — all match PLAIN_SUBROUTINE signature          */
/* ------------------------------------------------------------------ */

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_IF(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    PLAIN_WORD_DOUBLE i = 0;
    /* Walk condition+block pairs, optionally chained with "else".
     * "else {block}"           — final else branch.
     * "else condition {block}" — elseif: any non-block value is the condition. */
    while(i + 1 < arity) {
        struct PLAIN_VALUE* condition  = PLAIN_ARGUMENT(node, i);
        struct PLAIN_VALUE* next = PLAIN_ARGUMENT(node, i + 1);
        if(PLAIN_IS_TRUE(condition))
            return PLAIN_EVALUATE_BLOCK(context, (struct PLAIN_LIST*)next->data, value);
        i += 2;
        if(i >= arity) break;
        if(!PLAIN_KEYWORD_EQ(PLAIN_ARGUMENT(node, i), (const PLAIN_BYTE*)"else")) break;
        i++;
        if(i >= arity) break;
        /* "else {block}" — final else. */
        if(PLAIN_ARGUMENT(node, i)->type == PLAIN_TYPE_LIST)
            return PLAIN_EVALUATE_BLOCK(context, (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, i)->data, value);
        /* "else condition {block}" — elseif: loop with the new condition. */
    }
    return 0;
}

/* Counted loop with a scoped iteration variable: repeat {x} times 10 { ... }. */
static PLAIN_WORD_DOUBLE PLAIN_REPEAT_COUNTED_BINDING(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* variable, PLAIN_WORD_DOUBLE count, struct PLAIN_LIST* body) {
    const PLAIN_BYTE* from = variable->segment.from;
    const PLAIN_BYTE* to   = variable->segment.to;
    while(from < to && (*from == ' ' || *from == '\t' || *from == '\n' || *from == '\r')) from++;
    while(to > from && (*(to - 1) == ' ' || *(to - 1) == '\t' || *(to - 1) == '\n' || *(to - 1) == '\r')) to--;
    PLAIN_WORD_DOUBLE length = to - from;
    if(length == 0) return 0;
    PLAIN_BYTE* name = (PLAIN_BYTE*)PLAIN_AUTO(length + 1);
    if(name == NULL) return PLAIN_ERROR_SYSTEM;
    memcpy(name, from, length);
    name[length] = '\0';
    struct PLAIN_FRAME* frame = PLAIN_FRAME_CREATE(context->frame);
    if(frame == NULL) return PLAIN_ERROR_SYSTEM;
    PLAIN_FRAME_RETAIN(frame);
    struct PLAIN_FRAME* saved = context->frame;
    context->frame = frame;
    PLAIN_WORD_DOUBLE zero = 0;
    struct PLAIN_VALUE initial = {(PLAIN_BYTE*)&zero, sizeof(PLAIN_WORD_DOUBLE), PLAIN_TYPE_INTEGER};
    PLAIN_FRAME_BIND(frame, name, &initial, NULL, 0);
    struct PLAIN_BINDING* binding = PLAIN_FRAME_FIND(frame, name);
    for(PLAIN_WORD_DOUBLE index = 0; index < count; index++) {
        *(PLAIN_WORD_DOUBLE*)binding->value.data = index;
        struct PLAIN_VALUE result = {NULL, 0, PLAIN_TYPE_NIL};
        PLAIN_WORD_DOUBLE error = PLAIN_EVALUATE_BLOCK(context, body, &result);
        PLAIN_VALUE_CLEAR(&result);
        if(error == PLAIN_SIGNAL_BREAK) break;
        if(error == PLAIN_SIGNAL_CONTINUE) continue;
        if(error != 0) { context->frame = saved; PLAIN_FRAME_RELEASE(frame); return error; }
    }
    context->frame = saved;
    PLAIN_FRAME_RELEASE(frame);
    return 0;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_REPEAT(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    struct PLAIN_VALUE* first = PLAIN_ARGUMENT(node, 0);
    if(first == NULL) return 0;
    /* repeat {body} — infinite. */
    if(arity == 1 && first->type == PLAIN_TYPE_LIST) {
        struct PLAIN_LIST* body = (struct PLAIN_LIST*)first->data;
        for(;;) {
            struct PLAIN_VALUE result = {NULL, 0, PLAIN_TYPE_NIL};
            PLAIN_WORD_DOUBLE error = PLAIN_EVALUATE_BLOCK(context, body, &result);
            PLAIN_VALUE_CLEAR(&result);
            if(error == PLAIN_SIGNAL_BREAK) break;
            if(error == PLAIN_SIGNAL_CONTINUE) continue;
            if(error != 0) return error;
        }
        return 0;
    }
    /* repeat {variable} times count {body} — counted with automatic binding. */
    if(arity >= 4 && first->type == PLAIN_TYPE_LIST &&
       PLAIN_KEYWORD_EQ(PLAIN_ARGUMENT(node, 1), (const PLAIN_BYTE*)"times")) {
        struct PLAIN_VALUE* counter = PLAIN_ARGUMENT(node, 2);
        struct PLAIN_VALUE* fourth  = PLAIN_ARGUMENT(node, 3);
        if(counter != NULL && counter->type == PLAIN_TYPE_INTEGER &&
           fourth  != NULL && fourth->type  == PLAIN_TYPE_LIST) {
            return PLAIN_REPEAT_COUNTED_BINDING(context,
                (struct PLAIN_LIST*)first->data,
                *(PLAIN_WORD_DOUBLE*)counter->data,
                (struct PLAIN_LIST*)fourth->data);
        }
    }
    struct PLAIN_VALUE* second = PLAIN_ARGUMENT(node, 1);
    if(arity < 2 || second == NULL || second->type != PLAIN_TYPE_LIST) return 0;
    struct PLAIN_LIST* body = (struct PLAIN_LIST*)second->data;
    /* repeat count {body} — counted. */
    if(first->type == PLAIN_TYPE_INTEGER) {
        PLAIN_WORD_DOUBLE count = *(PLAIN_WORD_DOUBLE*)first->data;
        PLAIN_WORD_DOUBLE i = 0;
        for(; i < count; i++) {
            struct PLAIN_VALUE result = {NULL, 0, PLAIN_TYPE_NIL};
            PLAIN_WORD_DOUBLE error = PLAIN_EVALUATE_BLOCK(context, body, &result);
            PLAIN_VALUE_CLEAR(&result);
            if(error == PLAIN_SIGNAL_BREAK) break;
            if(error == PLAIN_SIGNAL_CONTINUE) continue;
            if(error != 0) return error;
        }
        return 0;
    }
    /* repeat {condition} {body} — conditional. */
    if(first->type == PLAIN_TYPE_LIST) {
        struct PLAIN_LIST* condition = (struct PLAIN_LIST*)first->data;
        for(;;) {
            struct PLAIN_VALUE test = {NULL, 0, PLAIN_TYPE_NIL};
            PLAIN_WORD_DOUBLE error = PLAIN_EVALUATE_BLOCK(context, condition, &test);
            PLAIN_WORD_DOUBLE ok = PLAIN_IS_TRUE(&test);
            PLAIN_VALUE_CLEAR(&test);
            if(error != 0) return error;
            if(!ok) break;
            struct PLAIN_VALUE result = {NULL, 0, PLAIN_TYPE_NIL};
            error = PLAIN_EVALUATE_BLOCK(context, body, &result);
            PLAIN_VALUE_CLEAR(&result);
            if(error == PLAIN_SIGNAL_BREAK) break;
            if(error == PLAIN_SIGNAL_CONTINUE) continue;
            if(error != 0) return error;
        }
        return 0;
    }
    return 0;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_RETURN(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    PLAIN_VALUE_CLEAR(&context->result);
    if(PLAIN_ARITY(node) > 0) PLAIN_VALUE_COPY(&context->result, PLAIN_ARGUMENT(node, 0));
    return PLAIN_SIGNAL_RETURN;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_BREAK(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    return PLAIN_SIGNAL_BREAK;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_CONTINUE(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    return PLAIN_SIGNAL_CONTINUE;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_DO(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    if(PLAIN_ARITY(node) < 1) return 0;
    struct PLAIN_VALUE* first = PLAIN_ARGUMENT(node, 0);
    if(first->type != PLAIN_TYPE_LIST) return 0;
    struct PLAIN_FRAME* frame = PLAIN_FRAME_CREATE(context->frame);
    if(frame == NULL) return PLAIN_ERROR_SYSTEM;
    struct PLAIN_FRAME* saved = context->frame;
    context->frame = frame;
    PLAIN_WORD_DOUBLE error = PLAIN_EVALUATE_BLOCK(context, (struct PLAIN_LIST*)first->data, value);
    context->frame = saved;
    PLAIN_FRAME_RELEASE(frame);
    if(error == PLAIN_SIGNAL_RETURN) {
        if(value != NULL) { *value = context->result; context->result = (struct PLAIN_VALUE){NULL, 0, PLAIN_TYPE_NIL}; }
        else { PLAIN_VALUE_CLEAR(&context->result); }
        return 0;
    }
    return error;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_TYPE(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE* argument = PLAIN_ARGUMENT(node, 0);
    const char* name = "none";
    if(argument != NULL) {
        switch(argument->type) {
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
    }
    if(value != NULL) return PLAIN_EXPORT((PLAIN_BYTE*)name, strlen(name) + 1, PLAIN_TYPE_STRING, value);
    return 0;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_FUNCTION(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    if(arity < 2) return 0;
    struct PLAIN_VALUE* first = PLAIN_ARGUMENT(node, 0);
    /* Named form: function name {parameters} {body} — name may be a keyword or quoted string */
    if(arity >= 3 && (first->type == PLAIN_TYPE_KEYWORD || first->type == PLAIN_TYPE_STRING)) {
        struct PLAIN_LIST* parameters = (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 1)->data;
        struct PLAIN_LIST* body       = (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 2)->data;
        struct PLAIN_CALLABLE* callable = (struct PLAIN_CALLABLE*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_CALLABLE), 0);
        if(callable == NULL) return PLAIN_ERROR_SYSTEM;
        memset(callable, 0, sizeof(struct PLAIN_CALLABLE));
        callable->parameters = PLAIN_SEGMENT_COPY(parameters);
        callable->body       = PLAIN_SEGMENT_COPY(body);
        callable->closure    = context->frame;
        callable->flags      = PLAIN_CALLABLE_IMMUTABLE;
        PLAIN_FRAME_RETAIN(context->frame);
        return PLAIN_FRAME_BIND(context->frame, first->data, NULL, callable, PLAIN_BINDING_IMMUTABLE);
    }
    /* Anonymous form: [function {parameters} {body}] — returns a callable value */
    if(first->type == PLAIN_TYPE_LIST) {
        struct PLAIN_LIST* parameters = (struct PLAIN_LIST*)first->data;
        struct PLAIN_LIST* body       = (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 1)->data;
        struct PLAIN_CALLABLE* callable = (struct PLAIN_CALLABLE*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_CALLABLE), 0);
        if(callable == NULL) return PLAIN_ERROR_SYSTEM;
        memset(callable, 0, sizeof(struct PLAIN_CALLABLE));
        callable->parameters = PLAIN_SEGMENT_COPY(parameters);
        callable->body       = PLAIN_SEGMENT_COPY(body);
        callable->closure    = context->frame;
        callable->flags      = PLAIN_CALLABLE_IMMUTABLE;
        PLAIN_FRAME_RETAIN(context->frame);
        if(value != NULL) { value->type = PLAIN_TYPE_CALLABLE; value->data = (PLAIN_BYTE*)callable; value->length = 0; }
        return 0;
    }
    return 0;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_PROCEDURE(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    if(arity < 2) return 0;
    struct PLAIN_VALUE* first = PLAIN_ARGUMENT(node, 0);
    /* Named form: procedure name {parameters} {body} — name may be a keyword or quoted string */
    if(arity >= 3 && (first->type == PLAIN_TYPE_KEYWORD || first->type == PLAIN_TYPE_STRING)) {
        struct PLAIN_LIST* parameters = (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 1)->data;
        struct PLAIN_LIST* body       = (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 2)->data;
        struct PLAIN_CALLABLE* callable = (struct PLAIN_CALLABLE*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_CALLABLE), 0);
        if(callable == NULL) return PLAIN_ERROR_SYSTEM;
        memset(callable, 0, sizeof(struct PLAIN_CALLABLE));
        callable->parameters = PLAIN_SEGMENT_COPY(parameters);
        callable->body       = PLAIN_SEGMENT_COPY(body);
        callable->closure    = context->frame;
        PLAIN_FRAME_RETAIN(context->frame);
        return PLAIN_FRAME_BIND(context->frame, first->data, NULL, callable, 0);
    }
    /* Anonymous form: [procedure {parameters} {body}] — returns a callable value */
    if(first->type == PLAIN_TYPE_LIST) {
        struct PLAIN_LIST* parameters = (struct PLAIN_LIST*)first->data;
        struct PLAIN_LIST* body       = (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 1)->data;
        struct PLAIN_CALLABLE* callable = (struct PLAIN_CALLABLE*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_CALLABLE), 0);
        if(callable == NULL) return PLAIN_ERROR_SYSTEM;
        memset(callable, 0, sizeof(struct PLAIN_CALLABLE));
        callable->parameters = PLAIN_SEGMENT_COPY(parameters);
        callable->body       = PLAIN_SEGMENT_COPY(body);
        callable->closure    = context->frame;
        PLAIN_FRAME_RETAIN(context->frame);
        if(value != NULL) { value->type = PLAIN_TYPE_CALLABLE; value->data = (PLAIN_BYTE*)callable; value->length = 0; }
        return 0;
    }
    return 0;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_CLASS(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    if(arity < 2) return 0;
    struct PLAIN_VALUE* first = PLAIN_ARGUMENT(node, 0);
    /* Named form: class Name {parameters} {body} */
    if(arity >= 3 && (first->type == PLAIN_TYPE_KEYWORD || first->type == PLAIN_TYPE_STRING)) {
        struct PLAIN_LIST* parameters = (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 1)->data;
        struct PLAIN_LIST* body       = (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 2)->data;
        struct PLAIN_CALLABLE* callable = (struct PLAIN_CALLABLE*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_CALLABLE), 0);
        if(callable == NULL) return PLAIN_ERROR_SYSTEM;
        memset(callable, 0, sizeof(struct PLAIN_CALLABLE));
        callable->parameters = PLAIN_SEGMENT_COPY(parameters);
        callable->body       = PLAIN_SEGMENT_COPY(body);
        callable->closure    = context->frame;
        callable->flags      = PLAIN_CALLABLE_IMMUTABLE | PLAIN_CALLABLE_CLASS;
        PLAIN_FRAME_RETAIN(context->frame);
        return PLAIN_FRAME_BIND(context->frame, first->data, NULL, callable, PLAIN_BINDING_IMMUTABLE);
    }
    /* Anonymous form: [class {parameters} {body}] — returns a constructor callable */
    if(first->type == PLAIN_TYPE_LIST) {
        struct PLAIN_LIST* parameters = (struct PLAIN_LIST*)first->data;
        struct PLAIN_LIST* body       = (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 1)->data;
        struct PLAIN_CALLABLE* callable = (struct PLAIN_CALLABLE*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_CALLABLE), 0);
        if(callable == NULL) return PLAIN_ERROR_SYSTEM;
        memset(callable, 0, sizeof(struct PLAIN_CALLABLE));
        callable->parameters = PLAIN_SEGMENT_COPY(parameters);
        callable->body       = PLAIN_SEGMENT_COPY(body);
        callable->closure    = context->frame;
        callable->flags      = PLAIN_CALLABLE_IMMUTABLE | PLAIN_CALLABLE_CLASS;
        PLAIN_FRAME_RETAIN(context->frame);
        if(value != NULL) { value->type = PLAIN_TYPE_CALLABLE; value->data = (PLAIN_BYTE*)callable; value->length = 0; }
        return 0;
    }
    return 0;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_NOT(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE* a = PLAIN_ARGUMENT(node, 0);
    if(a == NULL) return 0;
    return PLAIN_SET_BOOLEAN(value, !PLAIN_IS_TRUE(a));
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_AND(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    PLAIN_WORD_DOUBLE i = 0;
    for(; i < arity; i++) { if(!PLAIN_IS_TRUE(PLAIN_ARGUMENT(node, i))) return PLAIN_SET_BOOLEAN(value, 0); }
    return PLAIN_SET_BOOLEAN(value, 1);
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_OR(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    PLAIN_WORD_DOUBLE i = 0;
    for(; i < arity; i++) { if(PLAIN_IS_TRUE(PLAIN_ARGUMENT(node, i))) return PLAIN_SET_BOOLEAN(value, 1); }
    return PLAIN_SET_BOOLEAN(value, 0);
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_ADD(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE* a = PLAIN_ARGUMENT(node, 0); struct PLAIN_VALUE* b = PLAIN_ARGUMENT(node, 1);
    return (a && b) ? PLAIN_APPLY_ARITHMETIC(a, b, '+', value) : 0;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_SUBTRACT(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE* a = PLAIN_ARGUMENT(node, 0); struct PLAIN_VALUE* b = PLAIN_ARGUMENT(node, 1);
    return (a && b) ? PLAIN_APPLY_ARITHMETIC(a, b, '-', value) : 0;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_MULTIPLY(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE* a = PLAIN_ARGUMENT(node, 0); struct PLAIN_VALUE* b = PLAIN_ARGUMENT(node, 1);
    return (a && b) ? PLAIN_APPLY_ARITHMETIC(a, b, '*', value) : 0;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_DIVIDE(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE* a = PLAIN_ARGUMENT(node, 0); struct PLAIN_VALUE* b = PLAIN_ARGUMENT(node, 1);
    return (a && b) ? PLAIN_APPLY_ARITHMETIC(a, b, '/', value) : 0;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_MODULO(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE* a = PLAIN_ARGUMENT(node, 0); struct PLAIN_VALUE* b = PLAIN_ARGUMENT(node, 1);
    return (a && b) ? PLAIN_APPLY_ARITHMETIC(a, b, '%', value) : 0;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_EQUAL(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE* a = PLAIN_ARGUMENT(node, 0); struct PLAIN_VALUE* b = PLAIN_ARGUMENT(node, 1);
    return (a && b) ? PLAIN_APPLY_COMPARISON(a, b, '=', value) : 0;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_LESS(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE* a = PLAIN_ARGUMENT(node, 0); struct PLAIN_VALUE* b = PLAIN_ARGUMENT(node, 1);
    return (a && b) ? PLAIN_APPLY_COMPARISON(a, b, '<', value) : 0;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_GREATER(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE* a = PLAIN_ARGUMENT(node, 0); struct PLAIN_VALUE* b = PLAIN_ARGUMENT(node, 1);
    return (a && b) ? PLAIN_APPLY_COMPARISON(a, b, '>', value) : 0;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_NOT_EQUAL(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE* a = PLAIN_ARGUMENT(node, 0); struct PLAIN_VALUE* b = PLAIN_ARGUMENT(node, 1);
    if(!a || !b) return 0;
    struct PLAIN_VALUE temp = {NULL, 0, PLAIN_TYPE_NIL};
    PLAIN_WORD_DOUBLE error = PLAIN_APPLY_COMPARISON(a, b, '=', &temp);
    if(error == 0) error = PLAIN_SET_BOOLEAN(value, !PLAIN_IS_TRUE(&temp));
    PLAIN_VALUE_CLEAR(&temp);
    return error;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_LESS_EQUAL(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE* a = PLAIN_ARGUMENT(node, 0); struct PLAIN_VALUE* b = PLAIN_ARGUMENT(node, 1);
    if(!a || !b) return 0;
    struct PLAIN_VALUE temp = {NULL, 0, PLAIN_TYPE_NIL};
    PLAIN_WORD_DOUBLE error = PLAIN_APPLY_COMPARISON(a, b, '>', &temp);
    if(error == 0) error = PLAIN_SET_BOOLEAN(value, !PLAIN_IS_TRUE(&temp));
    PLAIN_VALUE_CLEAR(&temp);
    return error;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_GREATER_EQUAL(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE* a = PLAIN_ARGUMENT(node, 0); struct PLAIN_VALUE* b = PLAIN_ARGUMENT(node, 1);
    if(!a || !b) return 0;
    struct PLAIN_VALUE temp = {NULL, 0, PLAIN_TYPE_NIL};
    PLAIN_WORD_DOUBLE error = PLAIN_APPLY_COMPARISON(a, b, '<', &temp);
    if(error == 0) error = PLAIN_SET_BOOLEAN(value, !PLAIN_IS_TRUE(&temp));
    PLAIN_VALUE_CLEAR(&temp);
    return error;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_CONCAT(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    if(value == NULL) return 0;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_SEGMENT result = {NULL, NULL};
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    PLAIN_WORD_DOUBLE i = 0;
    for(; i < arity; i++) {
        struct PLAIN_VALUE* argument = PLAIN_ARGUMENT(node, i);
        PLAIN_BYTE buffer[64];
        const PLAIN_BYTE* str = (const PLAIN_BYTE*)"";
        PLAIN_WORD_DOUBLE length = 0;
        switch(argument->type) {
            case PLAIN_TYPE_STRING:
                str = argument->data; length = (PLAIN_WORD_DOUBLE)strlen((const char*)str); break;
            case PLAIN_TYPE_INTEGER:
                length = sprintf((char*)buffer, "%d", (int)*(PLAIN_WORD_DOUBLE*)argument->data); str = buffer; break;
            case PLAIN_TYPE_REAL:
                length = sprintf((char*)buffer, "%g", (PLAIN_REAL_DOUBLE)*(PLAIN_REAL*)argument->data); str = buffer; break;
            case PLAIN_TYPE_BOOLEAN:
                str = argument->data != NULL ? (const PLAIN_BYTE*)"yes" : (const PLAIN_BYTE*)"no";
                length = (PLAIN_WORD_DOUBLE)strlen((const char*)str); break;
            default:
                str = (const PLAIN_BYTE*)"none"; length = 4; break;
        }
        if(length > 0) PLAIN_CONCAT(&result, length, str);
    }
    PLAIN_BYTE zero = 0;
    PLAIN_CONCAT(&result, 1, &zero);
    value->data = result.from;
    value->length = result.to - result.from;
    value->type = PLAIN_TYPE_STRING;
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Context init — register all built-ins in the root frame          */
/* ------------------------------------------------------------------ */

PLAIN_WORD_DOUBLE PLAIN_CONTEXT_REGISTER(struct PLAIN_CONTEXT* context, const PLAIN_BYTE* name, PLAIN_SUBROUTINE native) {
    struct PLAIN_CALLABLE* callable = (struct PLAIN_CALLABLE*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_CALLABLE), 0);
    if(callable == NULL) return PLAIN_ERROR_SYSTEM;
    memset(callable, 0, sizeof(struct PLAIN_CALLABLE));
    callable->native = native;
    return PLAIN_FRAME_BIND(context->frame, name, NULL, callable, 0);
}

PLAIN_WORD_DOUBLE PLAIN_CONTEXT_INIT(struct PLAIN_CONTEXT* context) {
    #define PLAIN_REGISTER(name, fn) \
        if(PLAIN_CONTEXT_REGISTER(context, (const PLAIN_BYTE*)(name), fn) != 0) return PLAIN_ERROR_SYSTEM;
    PLAIN_REGISTER("if",        PLAIN_NATIVE_IF)
    PLAIN_REGISTER("repeat",    PLAIN_NATIVE_REPEAT)
    PLAIN_REGISTER("return",    PLAIN_NATIVE_RETURN)
    PLAIN_REGISTER("break",     PLAIN_NATIVE_BREAK)
    PLAIN_REGISTER("continue",  PLAIN_NATIVE_CONTINUE)
    PLAIN_REGISTER("do",        PLAIN_NATIVE_DO)
    PLAIN_REGISTER("type",      PLAIN_NATIVE_TYPE)
    PLAIN_REGISTER("function",  PLAIN_NATIVE_FUNCTION)
    PLAIN_REGISTER("procedure", PLAIN_NATIVE_PROCEDURE)
    PLAIN_REGISTER("class",     PLAIN_NATIVE_CLASS)
    PLAIN_REGISTER("not",       PLAIN_NATIVE_NOT)
    PLAIN_REGISTER("and",       PLAIN_NATIVE_AND)
    PLAIN_REGISTER("or",        PLAIN_NATIVE_OR)
    PLAIN_REGISTER("add",       PLAIN_NATIVE_ADD)
    PLAIN_REGISTER("subtract",  PLAIN_NATIVE_SUBTRACT)
    PLAIN_REGISTER("multiply",  PLAIN_NATIVE_MULTIPLY)
    PLAIN_REGISTER("divide",    PLAIN_NATIVE_DIVIDE)
    PLAIN_REGISTER("modulo",    PLAIN_NATIVE_MODULO)
    PLAIN_REGISTER("equal",     PLAIN_NATIVE_EQUAL)
    PLAIN_REGISTER("less",      PLAIN_NATIVE_LESS)
    PLAIN_REGISTER("greater",   PLAIN_NATIVE_GREATER)
    PLAIN_REGISTER("join",      PLAIN_NATIVE_CONCAT)
    #undef PLAIN_REGISTER
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Main resolver                                                     */
/* ------------------------------------------------------------------ */

PLAIN_WORD_DOUBLE PLAIN_RESOLVE(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;

    /* Variable substitution. Callable bindings are returned as PLAIN_TYPE_CALLABLE
     * values so they can be passed around and called as first-class citizens.
     * Operator symbols (+, -, and, etc.) are intentionally not resolved here;
     * their frame overrides are looked up inside PLAIN_BUILTIN_INFIX instead,
     * so the infix detection by keyword name always works. */
    if(type == PLAIN_TYPE_KEYWORD) {
        if(!PLAIN_IS_OPERATOR_SYMBOL((const PLAIN_BYTE*)data)) {
            struct PLAIN_BINDING* binding = PLAIN_FRAME_FIND(context->frame, (const PLAIN_BYTE*)data);
            if(binding != NULL) {
                if(binding->callable != NULL) {
                    value->type = PLAIN_TYPE_CALLABLE;
                    value->data = (PLAIN_BYTE*)binding->callable;
                    value->length = 0;
                } else {
                    PLAIN_VALUE_COPY(value, &binding->value);
                }
            }
        }
        return 0;
    }

    if(type != PLAIN_TYPE_LIST) return 0;

    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    if(node->keyword.from == node->keyword.to) return 0;

    /* Infix expression: name op value — strictly binary (exactly 2 arguments).
     * More than 2 arguments means the first word is a command name, not a left operand. */
    if(PLAIN_ARITY(node) == 2 && PLAIN_IS_INFIX_OPERATOR(PLAIN_ARGUMENT(node, 0))) {
        return PLAIN_BUILTIN_INFIX(context, node, value);
    }

    /* Look up in frame — built-ins and user-defined callables live here.
     * A PLAIN_TYPE_CALLABLE value (from x = [function ...]) is also callable. */
    PLAIN_BYTE keyword[256];
    PLAIN_KEYWORD_EXTRACT(node, keyword, sizeof(keyword));
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
            if(binding->value.length == PLAIN_OBJECT_NATIVE && binding->value.data != NULL) {
                struct PLAIN_FRAME* instance = (struct PLAIN_FRAME*)binding->value.data;
                PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
                if(arity == 0) {
                    if(value != NULL) PLAIN_VALUE_COPY(value, &binding->value);
                    return 0;
                }
                struct PLAIN_VALUE* method_arg = PLAIN_ARGUMENT(node, 0);
                if(method_arg == NULL) return 0;
                /* If the method name was pre-resolved to a callable (because it
                 * was in the scope chain during argument walk), call it directly. */
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
