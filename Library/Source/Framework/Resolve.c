/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     11/01/2014.
 * Revision 03/22/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 */

#include <Plain/VM.h>
#include <Plain/Framework/Scope.h>
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
            destination->type = PLAIN_TYPE_SUBROUTINE;
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

/* Returns nonzero if <argument> is a known infix operator keyword. */
PLAIN_INLINE PLAIN_WORD_DOUBLE PLAIN_IS_INFIX_OPERATOR(struct PLAIN_VALUE* argument) {
    if(argument == NULL || argument->type != PLAIN_TYPE_KEYWORD) return 0;
    const PLAIN_BYTE* op = argument->data;
    return strcmp((const char*)op, "+")   == 0 ||
           strcmp((const char*)op, "-")   == 0 ||
           strcmp((const char*)op, "*")   == 0 ||
           strcmp((const char*)op, "/")   == 0 ||
           strcmp((const char*)op, "%")   == 0 ||
           strcmp((const char*)op, "=")   == 0 ||
           strcmp((const char*)op, "!=")  == 0 ||
           strcmp((const char*)op, "<")   == 0 ||
           strcmp((const char*)op, ">")   == 0 ||
           strcmp((const char*)op, "<=")  == 0 ||
           strcmp((const char*)op, ">=")  == 0 ||
           strcmp((const char*)op, "and") == 0 ||
           strcmp((const char*)op, "or")  == 0;
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

static PLAIN_WORD_DOUBLE PLAIN_CALL(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, struct PLAIN_CALLABLE* callable, struct PLAIN_VALUE* value) {
    /* Native callable — call it directly. The node arrives with arguments
     * already resolved (expressions substituted, blocks left unevaluated). */
    if(callable->native != NULL) {
        return callable->native((void*)context, (void*)node, PLAIN_TYPE_LIST, value);
    }
    /* User-defined callable — bind parameters and evaluate body. */
    struct PLAIN_FRAME* frame = PLAIN_FRAME_CREATE(context->frame);
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
    PLAIN_FRAME_DESTROY(frame);
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
/*  Forward declarations (needed by assign helpers below)            */
/* ------------------------------------------------------------------ */

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_FUNCTION(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value);
static PLAIN_WORD_DOUBLE PLAIN_NATIVE_PROCEDURE(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value);

/* ------------------------------------------------------------------ */
/*  Assignment and infix                                              */
/* ------------------------------------------------------------------ */

/* Checks whether <value> represents a function-defining callable.
 * Handles both the "function" keyword (if not in frame) and the
 * subroutine value that results when it has been resolved from the frame. */
PLAIN_INLINE PLAIN_WORD_DOUBLE PLAIN_IS_FUNCTION_DEFINER(struct PLAIN_VALUE* value) {
    if(PLAIN_KEYWORD_EQ(value, (const PLAIN_BYTE*)"function")) return 1;
    if(value->type == PLAIN_TYPE_SUBROUTINE && value->data != NULL)
        return ((struct PLAIN_CALLABLE*)value->data)->native == &PLAIN_NATIVE_FUNCTION;
    return 0;
}

PLAIN_INLINE PLAIN_WORD_DOUBLE PLAIN_IS_PROCEDURE_DEFINER(struct PLAIN_VALUE* value) {
    if(PLAIN_KEYWORD_EQ(value, (const PLAIN_BYTE*)"procedure")) return 1;
    if(value->type == PLAIN_TYPE_SUBROUTINE && value->data != NULL)
        return ((struct PLAIN_CALLABLE*)value->data)->native == &PLAIN_NATIVE_PROCEDURE;
    return 0;
}

static PLAIN_WORD_DOUBLE PLAIN_BUILTIN_ASSIGN(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, struct PLAIN_VALUE* value) {
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    if(arity < 2) return 0;
    PLAIN_BYTE name[256];
    PLAIN_KEYWORD_EXTRACT(node, name, sizeof(name));
    struct PLAIN_VALUE* second = PLAIN_ARGUMENT(node, 1);
    /* x = function {parameters} {body} */
    if(arity >= 4 && PLAIN_IS_FUNCTION_DEFINER(second)) {
        struct PLAIN_LIST* parameters = (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 2)->data;
        struct PLAIN_LIST* body       = (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 3)->data;
        struct PLAIN_CALLABLE* callable = (struct PLAIN_CALLABLE*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_CALLABLE), 0);
        if(callable == NULL) return PLAIN_ERROR_SYSTEM;
        memset(callable, 0, sizeof(struct PLAIN_CALLABLE));
        callable->parameters = PLAIN_SEGMENT_COPY(parameters);
        callable->body       = PLAIN_SEGMENT_COPY(body);
        return PLAIN_FRAME_BIND(context->frame, name, NULL, callable, PLAIN_BINDING_IMMUTABLE);
    }
    /* x = procedure {parameters} {body} */
    if(arity >= 4 && PLAIN_IS_PROCEDURE_DEFINER(second)) {
        struct PLAIN_LIST* parameters = (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 2)->data;
        struct PLAIN_LIST* body       = (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 3)->data;
        struct PLAIN_CALLABLE* callable = (struct PLAIN_CALLABLE*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_CALLABLE), 0);
        if(callable == NULL) return PLAIN_ERROR_SYSTEM;
        memset(callable, 0, sizeof(struct PLAIN_CALLABLE));
        callable->parameters = PLAIN_SEGMENT_COPY(parameters);
        callable->body       = PLAIN_SEGMENT_COPY(body);
        return PLAIN_FRAME_BIND(context->frame, name, NULL, callable, 0);
    }
    /* x = value */
    PLAIN_FRAME_BIND(context->frame, name, second, NULL, 0);
    if(value != NULL) PLAIN_VALUE_COPY(value, second);
    return 0;
}

/* Handles all infix expressions: name op value.
 * "=" in statement context becomes assignment; in expression context, equality. */
static PLAIN_WORD_DOUBLE PLAIN_BUILTIN_INFIX(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, struct PLAIN_VALUE* value) {
    const PLAIN_BYTE* op = PLAIN_ARGUMENT(node, 0)->data;
    struct PLAIN_VALUE* right = PLAIN_ARGUMENT(node, 1);
    /* Assignment: name = value (no parent means statement context). */
    if(strcmp((const char*)op, "=") == 0 && node->parent == NULL) {
        return PLAIN_BUILTIN_ASSIGN(context, node, value);
    }
    /* Resolve left operand from the node keyword. */
    struct PLAIN_VALUE left = {NULL, 0, PLAIN_TYPE_NIL};
    PLAIN_WORD_DOUBLE error = PLAIN_KEYWORD_AS_VALUE(context, node, &left);
    if(error != 0) return error;
    /* Arithmetic. */
    if(strcmp((const char*)op, "+") == 0) error = PLAIN_APPLY_ARITHMETIC(&left, right, '+', value);
    else if(strcmp((const char*)op, "-") == 0) error = PLAIN_APPLY_ARITHMETIC(&left, right, '-', value);
    else if(strcmp((const char*)op, "*") == 0) error = PLAIN_APPLY_ARITHMETIC(&left, right, '*', value);
    else if(strcmp((const char*)op, "/") == 0) error = PLAIN_APPLY_ARITHMETIC(&left, right, '/', value);
    else if(strcmp((const char*)op, "%") == 0) error = PLAIN_APPLY_ARITHMETIC(&left, right, '%', value);
    /* Comparison. */
    else if(strcmp((const char*)op, "=") == 0)  error = PLAIN_APPLY_COMPARISON(&left, right, '=', value);
    else if(strcmp((const char*)op, "<") == 0)  error = PLAIN_APPLY_COMPARISON(&left, right, '<', value);
    else if(strcmp((const char*)op, ">") == 0)  error = PLAIN_APPLY_COMPARISON(&left, right, '>', value);
    else if(strcmp((const char*)op, "!=") == 0) {
        struct PLAIN_VALUE temp = {NULL, 0, PLAIN_TYPE_NIL};
        error = PLAIN_APPLY_COMPARISON(&left, right, '=', &temp);
        if(error == 0) error = PLAIN_SET_BOOLEAN(value, !PLAIN_IS_TRUE(&temp));
        PLAIN_VALUE_CLEAR(&temp);
    }
    else if(strcmp((const char*)op, "<=") == 0) {
        struct PLAIN_VALUE temp = {NULL, 0, PLAIN_TYPE_NIL};
        error = PLAIN_APPLY_COMPARISON(&left, right, '>', &temp);
        if(error == 0) error = PLAIN_SET_BOOLEAN(value, !PLAIN_IS_TRUE(&temp));
        PLAIN_VALUE_CLEAR(&temp);
    }
    else if(strcmp((const char*)op, ">=") == 0) {
        struct PLAIN_VALUE temp = {NULL, 0, PLAIN_TYPE_NIL};
        error = PLAIN_APPLY_COMPARISON(&left, right, '<', &temp);
        if(error == 0) error = PLAIN_SET_BOOLEAN(value, !PLAIN_IS_TRUE(&temp));
        PLAIN_VALUE_CLEAR(&temp);
    }
    /* Logic. */
    else if(strcmp((const char*)op, "and") == 0) error = PLAIN_SET_BOOLEAN(value, PLAIN_IS_TRUE(&left) && PLAIN_IS_TRUE(right));
    else if(strcmp((const char*)op, "or")  == 0) error = PLAIN_SET_BOOLEAN(value, PLAIN_IS_TRUE(&left) || PLAIN_IS_TRUE(right));
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
    if(arity < 2) return 0;
    if(PLAIN_IS_TRUE(PLAIN_ARGUMENT(node, 0)))
        return PLAIN_EVALUATE_BLOCK(context, (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 1)->data, value);
    if(arity >= 3)
        return PLAIN_EVALUATE_BLOCK(context, (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 2)->data, value);
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
            if(error != 0) return error;
        }
        return 0;
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

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_FUNCTION(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    if(arity < 3) return 0;
    struct PLAIN_VALUE* name = PLAIN_ARGUMENT(node, 0);
    if(name->type != PLAIN_TYPE_KEYWORD) return 0;
    struct PLAIN_LIST* parameters = (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 1)->data;
    struct PLAIN_LIST* body       = (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 2)->data;
    struct PLAIN_CALLABLE* callable = (struct PLAIN_CALLABLE*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_CALLABLE), 0);
    if(callable == NULL) return PLAIN_ERROR_SYSTEM;
    memset(callable, 0, sizeof(struct PLAIN_CALLABLE));
    callable->parameters = PLAIN_SEGMENT_COPY(parameters);
    callable->body       = PLAIN_SEGMENT_COPY(body);
    return PLAIN_FRAME_BIND(context->frame, name->data, NULL, callable, PLAIN_BINDING_IMMUTABLE);
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_PROCEDURE(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    if(arity < 3) return 0;
    struct PLAIN_VALUE* name = PLAIN_ARGUMENT(node, 0);
    if(name->type != PLAIN_TYPE_KEYWORD) return 0;
    struct PLAIN_LIST* parameters = (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 1)->data;
    struct PLAIN_LIST* body       = (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 2)->data;
    struct PLAIN_CALLABLE* callable = (struct PLAIN_CALLABLE*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_CALLABLE), 0);
    if(callable == NULL) return PLAIN_ERROR_SYSTEM;
    memset(callable, 0, sizeof(struct PLAIN_CALLABLE));
    callable->parameters = PLAIN_SEGMENT_COPY(parameters);
    callable->body       = PLAIN_SEGMENT_COPY(body);
    return PLAIN_FRAME_BIND(context->frame, name->data, NULL, callable, 0);
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

PLAIN_WORD_DOUBLE PLAIN_CONTEXT_INIT(struct PLAIN_CONTEXT* context) {
    #define PLAIN_REGISTER(name, fn) {                                                              \
        struct PLAIN_CALLABLE* c = (struct PLAIN_CALLABLE*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_CALLABLE), 0); \
        if(c == NULL) return PLAIN_ERROR_SYSTEM;                                                    \
        memset(c, 0, sizeof(struct PLAIN_CALLABLE));                                                \
        c->native = fn;                                                                             \
        PLAIN_FRAME_BIND(context->frame, (const PLAIN_BYTE*)(name), NULL, c, 0);                   \
    }
    PLAIN_REGISTER("if",        PLAIN_NATIVE_IF)
    PLAIN_REGISTER("repeat",    PLAIN_NATIVE_REPEAT)
    PLAIN_REGISTER("return",    PLAIN_NATIVE_RETURN)
    PLAIN_REGISTER("break",     PLAIN_NATIVE_BREAK)
    PLAIN_REGISTER("function",  PLAIN_NATIVE_FUNCTION)
    PLAIN_REGISTER("procedure", PLAIN_NATIVE_PROCEDURE)
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
    PLAIN_REGISTER("concat",    PLAIN_NATIVE_CONCAT)
    #undef PLAIN_REGISTER
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Main resolver                                                     */
/* ------------------------------------------------------------------ */

PLAIN_WORD_DOUBLE PLAIN_RESOLVE(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;

    /* Variable substitution. Callable bindings are returned as PLAIN_TYPE_SUBROUTINE
     * values so they can be passed around and called as first-class citizens. */
    if(type == PLAIN_TYPE_KEYWORD) {
        struct PLAIN_BINDING* binding = PLAIN_FRAME_FIND(context->frame, (const PLAIN_BYTE*)data);
        if(binding != NULL) {
            if(binding->callable != NULL) {
                value->type = PLAIN_TYPE_SUBROUTINE;
                value->data = (PLAIN_BYTE*)binding->callable;
                value->length = 0;
            } else {
                PLAIN_VALUE_COPY(value, &binding->value);
            }
        }
        return 0;
    }

    if(type != PLAIN_TYPE_LIST) return 0;

    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    if(node->keyword.from == node->keyword.to) return 0;

    /* Infix expression: name op value. */
    if(PLAIN_ARITY(node) >= 2 && PLAIN_IS_INFIX_OPERATOR(PLAIN_ARGUMENT(node, 0))) {
        return PLAIN_BUILTIN_INFIX(context, node, value);
    }

    /* Look up in frame — built-ins and user-defined callables live here.
     * A PLAIN_TYPE_SUBROUTINE value (from x = somefunction) is also callable. */
    PLAIN_BYTE keyword[256];
    PLAIN_KEYWORD_EXTRACT(node, keyword, sizeof(keyword));
    struct PLAIN_BINDING* binding = PLAIN_FRAME_FIND(context->frame, keyword);
    if(binding != NULL) {
        if(binding->callable != NULL) {
            return PLAIN_CALL(context, node, binding->callable, value);
        }
        if(binding->value.type == PLAIN_TYPE_SUBROUTINE && binding->value.data != NULL) {
            return PLAIN_CALL(context, node, (struct PLAIN_CALLABLE*)binding->value.data, value);
        }
    }

    /* Host-provided extension. */
    if(context->handler != NULL) {
        return context->handler(raw, data, type, value);
    }

    return 0;
}
