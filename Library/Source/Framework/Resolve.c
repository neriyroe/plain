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

/* Resolves a keyword argument to its frame value and returns it by copy.
 * If the argument is not a keyword, or the keyword is not found in the frame,
 * the argument value is returned as-is. Returned values borrow data pointers
 * from the binding or node — do not call PLAIN_VALUE_CLEAR on the result. */
PLAIN_INLINE struct PLAIN_VALUE PLAIN_ARGUMENT_VALUE(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, PLAIN_WORD_DOUBLE index) {
    struct PLAIN_VALUE* argument = PLAIN_ARGUMENT(node, index);
    if(argument == NULL) {
        struct PLAIN_VALUE nil = {NULL, 0, PLAIN_TYPE_NIL};
        return nil;
    }
    if(argument->type != PLAIN_TYPE_KEYWORD) return *argument;
    struct PLAIN_BINDING* binding = PLAIN_FRAME_FIND(context->frame, argument->data);
    if(binding == NULL) return *argument;
    if(binding->callable != NULL) {
        struct PLAIN_VALUE callable_value = {(PLAIN_BYTE*)binding->callable, 0, PLAIN_TYPE_CALLABLE};
        return callable_value;
    }
    return binding->value;
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

/* Variadic left-to-right fold over all arguments using the given arithmetic operation.
 * Accumulates from the first argument, applying the operation with each subsequent one. */
static PLAIN_WORD_DOUBLE PLAIN_ARITHMETIC_FOLD(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, PLAIN_BYTE operation, struct PLAIN_VALUE* value) {
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    if(arity == 0) return 0;
    struct PLAIN_VALUE first = PLAIN_ARGUMENT_VALUE(context, node, 0);
    if(arity == 1) { return value != NULL ? PLAIN_VALUE_COPY(value, &first) : 0; }
    struct PLAIN_VALUE accumulator = {NULL, 0, PLAIN_TYPE_NIL};
    PLAIN_VALUE_COPY(&accumulator, &first);
    PLAIN_WORD_DOUBLE i;
    for(i = 1; i < arity; i++) {
        struct PLAIN_VALUE next = PLAIN_ARGUMENT_VALUE(context, node, i);
        struct PLAIN_VALUE result = {NULL, 0, PLAIN_TYPE_NIL};
        PLAIN_WORD_DOUBLE error = PLAIN_APPLY_ARITHMETIC(&accumulator, &next, operation, &result);
        PLAIN_VALUE_CLEAR(&accumulator);
        if(error != 0) { PLAIN_VALUE_CLEAR(&result); return error; }
        accumulator = result;
    }
    if(value != NULL) *value = accumulator;
    else PLAIN_VALUE_CLEAR(&accumulator);
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Callable dispatch                                                 */
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

/* Resolves a raw argument (possibly a keyword) to its value for parameter binding.
 * Returns NIL for keywords not found in the frame. */
static struct PLAIN_VALUE PLAIN_RESOLVE_FOR_BINDING(struct PLAIN_CONTEXT* context, struct PLAIN_VALUE* raw_argument) {
    if(raw_argument->type != PLAIN_TYPE_KEYWORD) return *raw_argument;
    struct PLAIN_BINDING* keyword_binding = PLAIN_FRAME_FIND(context->frame, raw_argument->data);
    if(keyword_binding == NULL) {
        struct PLAIN_VALUE nil = {NULL, 0, PLAIN_TYPE_NIL};
        return nil;
    }
    if(keyword_binding->callable != NULL) {
        struct PLAIN_VALUE callable_value = {(PLAIN_BYTE*)keyword_binding->callable, 0, PLAIN_TYPE_CALLABLE};
        return callable_value;
    }
    return keyword_binding->value;
}

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
    const PLAIN_BYTE* pointer = callable->parameters;
    PLAIN_WORD_DOUBLE index = 0;
    while(pointer != NULL && *pointer != '\0') {
        while(*pointer == ' ' || *pointer == '\t' || *pointer == ',' || *pointer == ';' || *pointer == '\n' || *pointer == '\r') pointer++;
        if(*pointer == '\0') break;
        const PLAIN_BYTE* start = pointer;
        while(*pointer != '\0' && *pointer != ' ' && *pointer != '\t' && *pointer != ',' && *pointer != ';' && *pointer != '\n' && *pointer != '\r') pointer++;
        PLAIN_WORD_DOUBLE length = pointer - start;
        PLAIN_BYTE* name = (PLAIN_BYTE*)PLAIN_AUTO(length + 1);
        memcpy(name, start, length);
        name[length] = '\0';
        struct PLAIN_VALUE* raw_argument = PLAIN_ARGUMENT(node, index);
        if(raw_argument != NULL) {
            struct PLAIN_VALUE argument_value = PLAIN_RESOLVE_FOR_BINDING(context, raw_argument);
            PLAIN_FRAME_BIND(frame, name, &argument_value, NULL, 0);
        }
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
        while(*pointer == ' ' || *pointer == '\t' || *pointer == ',' || *pointer == ';' || *pointer == '\n' || *pointer == '\r') pointer++;
        if(*pointer == '\0') break;
        const PLAIN_BYTE* start = pointer;
        while(*pointer != '\0' && *pointer != ' ' && *pointer != '\t' && *pointer != ',' && *pointer != ';' && *pointer != '\n' && *pointer != '\r') pointer++;
        PLAIN_WORD_DOUBLE length = pointer - start;
        PLAIN_BYTE* name = (PLAIN_BYTE*)PLAIN_AUTO(length + 1);
        memcpy(name, start, length);
        name[length] = '\0';
        struct PLAIN_VALUE* raw_argument = PLAIN_ARGUMENT(node, offset + index);
        if(raw_argument != NULL) {
            struct PLAIN_VALUE argument_value = PLAIN_RESOLVE_FOR_BINDING(context, raw_argument);
            PLAIN_FRAME_BIND(frame, name, &argument_value, NULL, 0);
        }
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
/*  Native callables — all match PLAIN_SUBROUTINE signature          */
/* ------------------------------------------------------------------ */

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_SET(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    if(PLAIN_ARITY(node) < 2) return 0;
    /* arg0: the variable name — must be a keyword, never resolved to its current value */
    struct PLAIN_VALUE* name_arg = PLAIN_ARGUMENT(node, 0);
    if(name_arg == NULL || name_arg->type != PLAIN_TYPE_KEYWORD) return 0;
    PLAIN_WORD_DOUBLE name_length = strlen((const char*)name_arg->data);
    PLAIN_BYTE* name = (PLAIN_BYTE*)PLAIN_AUTO(name_length + 1);
    memcpy(name, name_arg->data, name_length);
    name[name_length] = '\0';
    /* arg1: the value to assign — resolve if it is an unresolved keyword */
    struct PLAIN_VALUE assignment_value = PLAIN_ARGUMENT_VALUE(context, node, 1);
    /* Callable value: duplicate and store it in the callable slot of the binding */
    if(assignment_value.type == PLAIN_TYPE_CALLABLE && assignment_value.data != NULL) {
        struct PLAIN_CALLABLE* source = (struct PLAIN_CALLABLE*)assignment_value.data;
        struct PLAIN_CALLABLE* copy = PLAIN_CALLABLE_DUPLICATE(source);
        if(copy == NULL) return PLAIN_ERROR_SYSTEM;
        return PLAIN_FRAME_SET(context->frame, name, NULL, copy, 0);
    }
    /* Plain value: store directly */
    PLAIN_FRAME_SET(context->frame, name, &assignment_value, NULL, 0);
    if(value != NULL) PLAIN_VALUE_COPY(value, &assignment_value);
    return 0;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_IF(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    PLAIN_WORD_DOUBLE i = 0;
    /* Walk condition+block pairs, optionally chained with "else".
     * "else {block}"           — final else branch.
     * "else condition {block}" — elseif: any non-block value is the condition. */
    while(i + 1 < arity) {
        struct PLAIN_VALUE condition = PLAIN_ARGUMENT_VALUE(context, node, i);
        struct PLAIN_VALUE* next = PLAIN_ARGUMENT(node, i + 1);
        if(PLAIN_IS_TRUE(&condition))
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

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_WHEN(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    if(arity < 3) return 0;
    struct PLAIN_VALUE target = PLAIN_ARGUMENT_VALUE(context, node, 0);
    PLAIN_WORD_DOUBLE i = 1;
    while(i + 1 < arity) {
        struct PLAIN_VALUE* block = PLAIN_ARGUMENT(node, i + 1);
        if(PLAIN_KEYWORD_EQ(PLAIN_ARGUMENT(node, i), (const PLAIN_BYTE*)"else")) {
            if(block != NULL && block->type == PLAIN_TYPE_LIST)
                return PLAIN_EVALUATE_BLOCK(context, (struct PLAIN_LIST*)block->data, value);
            break;
        }
        if(block != NULL && block->type == PLAIN_TYPE_LIST) {
            struct PLAIN_VALUE candidate = PLAIN_ARGUMENT_VALUE(context, node, i);
            struct PLAIN_VALUE result = {NULL, 0, PLAIN_TYPE_NIL};
            PLAIN_APPLY_COMPARISON(&target, &candidate, '=', &result);
            if(PLAIN_IS_TRUE(&result)) {
                PLAIN_VALUE_CLEAR(&result);
                return PLAIN_EVALUATE_BLOCK(context, (struct PLAIN_LIST*)block->data, value);
            }
            PLAIN_VALUE_CLEAR(&result);
        }
        i += 2;
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
        struct PLAIN_VALUE counter = PLAIN_ARGUMENT_VALUE(context, node, 2);
        struct PLAIN_VALUE* fourth  = PLAIN_ARGUMENT(node, 3);
        if(counter.type == PLAIN_TYPE_INTEGER &&
           fourth  != NULL && fourth->type  == PLAIN_TYPE_LIST) {
            return PLAIN_REPEAT_COUNTED_BINDING(context,
                (struct PLAIN_LIST*)first->data,
                *(PLAIN_WORD_DOUBLE*)counter.data,
                (struct PLAIN_LIST*)fourth->data);
        }
    }
    struct PLAIN_VALUE* second = PLAIN_ARGUMENT(node, 1);
    if(arity < 2 || second == NULL || second->type != PLAIN_TYPE_LIST) return 0;
    struct PLAIN_LIST* body = (struct PLAIN_LIST*)second->data;
    /* repeat count {body} — counted. */
    struct PLAIN_VALUE first_value = PLAIN_ARGUMENT_VALUE(context, node, 0);
    if(first_value.type == PLAIN_TYPE_INTEGER) {
        PLAIN_WORD_DOUBLE count = *(PLAIN_WORD_DOUBLE*)first_value.data;
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
    if(PLAIN_ARITY(node) > 0) {
        struct PLAIN_VALUE resolved = PLAIN_ARGUMENT_VALUE(context, node, 0);
        PLAIN_VALUE_COPY(&context->result, &resolved);
    }
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
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE argument = PLAIN_ARGUMENT_VALUE(context, node, 0);
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
    if(value != NULL) return PLAIN_EXPORT((PLAIN_BYTE*)name, strlen(name) + 1, PLAIN_TYPE_STRING, value);
    return 0;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_DEFINE(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    if(arity < 2) return 0;
    struct PLAIN_VALUE* first = PLAIN_ARGUMENT(node, 0);
    /* Named form: define name {parameters} {body} — name may be a keyword or quoted string */
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
    /* Anonymous form: [define {parameters} {body}] — returns a callable value */
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

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_OBJECT(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    if(PLAIN_ARITY(node) < 1) return 0;
    struct PLAIN_VALUE* first = PLAIN_ARGUMENT(node, 0);
    if(first == NULL || first->type != PLAIN_TYPE_LIST) return 0;
    /* Create the object frame as a child of the current (constructor) frame,
     * so methods can reach constructor parameters via the closure chain. */
    struct PLAIN_FRAME* frame = PLAIN_FRAME_CREATE(context->frame);
    if(frame == NULL) return PLAIN_ERROR_SYSTEM;
    struct PLAIN_FRAME* saved = context->frame;
    context->frame = frame;
    struct PLAIN_VALUE body = {NULL, 0, PLAIN_TYPE_NIL};
    PLAIN_WORD_DOUBLE error = PLAIN_EVALUATE_BLOCK(context, (struct PLAIN_LIST*)first->data, &body);
    context->frame = saved;
    PLAIN_VALUE_CLEAR(&body);
    if(error == PLAIN_SIGNAL_RETURN) {
        /* return inside an object block exits early; discard the return value. */
        PLAIN_VALUE_CLEAR(&context->result);
    } else if(error != 0) {
        PLAIN_FRAME_RELEASE(frame);
        return error;
    }
    /* Break circular closure references: callables defined in the block
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
    /* Auto-bind self — points to the object itself. Direct struct assignment
     * avoids an extra retain that would create an unbreakable circular reference. */
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
            self_binding->value.flags  = PLAIN_VALUE_USER_DEFINED;
            self_binding->flags        = PLAIN_BINDING_IMMUTABLE;
        }
    }
    if(value != NULL) {
        value->type   = PLAIN_TYPE_OBJECT;
        value->data   = (PLAIN_BYTE*)frame;
        value->length = 0;
        value->flags  = PLAIN_VALUE_USER_DEFINED;
    }
    PLAIN_FRAME_RELEASE(frame);
    return 0;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_NOT(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE a = PLAIN_ARGUMENT_VALUE(context, node, 0);
    return PLAIN_SET_BOOLEAN(value, !PLAIN_IS_TRUE(&a));
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_AND(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    PLAIN_WORD_DOUBLE i;
    for(i = 0; i < arity; i++) {
        struct PLAIN_VALUE arg = PLAIN_ARGUMENT_VALUE(context, node, i);
        if(!PLAIN_IS_TRUE(&arg)) return PLAIN_SET_BOOLEAN(value, 0);
    }
    return PLAIN_SET_BOOLEAN(value, 1);
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_OR(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    PLAIN_WORD_DOUBLE i;
    for(i = 0; i < arity; i++) {
        struct PLAIN_VALUE arg = PLAIN_ARGUMENT_VALUE(context, node, i);
        if(PLAIN_IS_TRUE(&arg)) return PLAIN_SET_BOOLEAN(value, 1);
    }
    return PLAIN_SET_BOOLEAN(value, 0);
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_ADD(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    return PLAIN_ARITHMETIC_FOLD((struct PLAIN_CONTEXT*)raw, (struct PLAIN_LIST*)data, '+', value);
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_SUBTRACT(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    return PLAIN_ARITHMETIC_FOLD((struct PLAIN_CONTEXT*)raw, (struct PLAIN_LIST*)data, '-', value);
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_MULTIPLY(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    return PLAIN_ARITHMETIC_FOLD((struct PLAIN_CONTEXT*)raw, (struct PLAIN_LIST*)data, '*', value);
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_DIVIDE(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    return PLAIN_ARITHMETIC_FOLD((struct PLAIN_CONTEXT*)raw, (struct PLAIN_LIST*)data, '/', value);
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_MODULO(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    return PLAIN_ARITHMETIC_FOLD((struct PLAIN_CONTEXT*)raw, (struct PLAIN_LIST*)data, '%', value);
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_EQUAL(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE a = PLAIN_ARGUMENT_VALUE(context, node, 0);
    struct PLAIN_VALUE b = PLAIN_ARGUMENT_VALUE(context, node, 1);
    return PLAIN_APPLY_COMPARISON(&a, &b, '=', value);
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_LESS(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE a = PLAIN_ARGUMENT_VALUE(context, node, 0);
    struct PLAIN_VALUE b = PLAIN_ARGUMENT_VALUE(context, node, 1);
    return PLAIN_APPLY_COMPARISON(&a, &b, '<', value);
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_GREATER(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE a = PLAIN_ARGUMENT_VALUE(context, node, 0);
    struct PLAIN_VALUE b = PLAIN_ARGUMENT_VALUE(context, node, 1);
    return PLAIN_APPLY_COMPARISON(&a, &b, '>', value);
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_NOT_EQUAL(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE a = PLAIN_ARGUMENT_VALUE(context, node, 0);
    struct PLAIN_VALUE b = PLAIN_ARGUMENT_VALUE(context, node, 1);
    struct PLAIN_VALUE temp = {NULL, 0, PLAIN_TYPE_NIL};
    PLAIN_WORD_DOUBLE error = PLAIN_APPLY_COMPARISON(&a, &b, '=', &temp);
    if(error == 0) error = PLAIN_SET_BOOLEAN(value, !PLAIN_IS_TRUE(&temp));
    PLAIN_VALUE_CLEAR(&temp);
    return error;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_LESS_EQUAL(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE a = PLAIN_ARGUMENT_VALUE(context, node, 0);
    struct PLAIN_VALUE b = PLAIN_ARGUMENT_VALUE(context, node, 1);
    struct PLAIN_VALUE temp = {NULL, 0, PLAIN_TYPE_NIL};
    PLAIN_WORD_DOUBLE error = PLAIN_APPLY_COMPARISON(&a, &b, '>', &temp);
    if(error == 0) error = PLAIN_SET_BOOLEAN(value, !PLAIN_IS_TRUE(&temp));
    PLAIN_VALUE_CLEAR(&temp);
    return error;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_GREATER_EQUAL(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE a = PLAIN_ARGUMENT_VALUE(context, node, 0);
    struct PLAIN_VALUE b = PLAIN_ARGUMENT_VALUE(context, node, 1);
    struct PLAIN_VALUE temp = {NULL, 0, PLAIN_TYPE_NIL};
    PLAIN_WORD_DOUBLE error = PLAIN_APPLY_COMPARISON(&a, &b, '<', &temp);
    if(error == 0) error = PLAIN_SET_BOOLEAN(value, !PLAIN_IS_TRUE(&temp));
    PLAIN_VALUE_CLEAR(&temp);
    return error;
}

static PLAIN_WORD_DOUBLE PLAIN_NATIVE_CONCAT(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    if(value == NULL) return 0;
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_SEGMENT result = {NULL, NULL};
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    PLAIN_WORD_DOUBLE i;
    for(i = 0; i < arity; i++) {
        struct PLAIN_VALUE argument = PLAIN_ARGUMENT_VALUE(context, node, i);
        PLAIN_BYTE buffer[64];
        const PLAIN_BYTE* str = (const PLAIN_BYTE*)"";
        PLAIN_WORD_DOUBLE length = 0;
        switch(argument.type) {
            case PLAIN_TYPE_STRING:
                str = argument.data; length = (PLAIN_WORD_DOUBLE)strlen((const char*)str); break;
            case PLAIN_TYPE_INTEGER:
                length = sprintf((char*)buffer, "%d", (int)*(PLAIN_WORD_DOUBLE*)argument.data); str = buffer; break;
            case PLAIN_TYPE_REAL:
                length = sprintf((char*)buffer, "%g", (PLAIN_REAL_DOUBLE)*(PLAIN_REAL*)argument.data); str = buffer; break;
            case PLAIN_TYPE_BOOLEAN:
                str = argument.data != NULL ? (const PLAIN_BYTE*)"yes" : (const PLAIN_BYTE*)"no";
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

PLAIN_WORD_DOUBLE PLAIN_CONTEXT_REGISTER_RUNTIME(struct PLAIN_CONTEXT* context) {
    #define PLAIN_REGISTER(name, fn) \
        if(PLAIN_CONTEXT_REGISTER(context, (const PLAIN_BYTE*)(name), fn) != 0) return PLAIN_ERROR_SYSTEM;
    PLAIN_REGISTER("if",            PLAIN_NATIVE_IF)
    PLAIN_REGISTER("when",          PLAIN_NATIVE_WHEN)
    PLAIN_REGISTER("repeat",        PLAIN_NATIVE_REPEAT)
    PLAIN_REGISTER("return",        PLAIN_NATIVE_RETURN)
    PLAIN_REGISTER("break",         PLAIN_NATIVE_BREAK)
    PLAIN_REGISTER("continue",      PLAIN_NATIVE_CONTINUE)
    PLAIN_REGISTER("do",            PLAIN_NATIVE_DO)
    PLAIN_REGISTER("type",          PLAIN_NATIVE_TYPE)
    PLAIN_REGISTER("set",           PLAIN_NATIVE_SET)
    PLAIN_REGISTER("define",        PLAIN_NATIVE_DEFINE)
    PLAIN_REGISTER("object",        PLAIN_NATIVE_OBJECT)
    PLAIN_REGISTER("not",           PLAIN_NATIVE_NOT)
    PLAIN_REGISTER("and",           PLAIN_NATIVE_AND)
    PLAIN_REGISTER("or",            PLAIN_NATIVE_OR)
    PLAIN_REGISTER("add",           PLAIN_NATIVE_ADD)
    PLAIN_REGISTER("+",             PLAIN_NATIVE_ADD)
    PLAIN_REGISTER("subtract",      PLAIN_NATIVE_SUBTRACT)
    PLAIN_REGISTER("-",             PLAIN_NATIVE_SUBTRACT)
    PLAIN_REGISTER("multiply",      PLAIN_NATIVE_MULTIPLY)
    PLAIN_REGISTER("*",             PLAIN_NATIVE_MULTIPLY)
    PLAIN_REGISTER("divide",        PLAIN_NATIVE_DIVIDE)
    PLAIN_REGISTER("/",             PLAIN_NATIVE_DIVIDE)
    PLAIN_REGISTER("modulo",        PLAIN_NATIVE_MODULO)
    PLAIN_REGISTER("%",             PLAIN_NATIVE_MODULO)
    PLAIN_REGISTER("equal",         PLAIN_NATIVE_EQUAL)
    PLAIN_REGISTER("=",             PLAIN_NATIVE_EQUAL)
    PLAIN_REGISTER("not_equal",     PLAIN_NATIVE_NOT_EQUAL)
    PLAIN_REGISTER("!=",            PLAIN_NATIVE_NOT_EQUAL)
    PLAIN_REGISTER("less",          PLAIN_NATIVE_LESS)
    PLAIN_REGISTER("<",             PLAIN_NATIVE_LESS)
    PLAIN_REGISTER("greater",       PLAIN_NATIVE_GREATER)
    PLAIN_REGISTER(">",             PLAIN_NATIVE_GREATER)
    PLAIN_REGISTER("less_equal",    PLAIN_NATIVE_LESS_EQUAL)
    PLAIN_REGISTER("<=",            PLAIN_NATIVE_LESS_EQUAL)
    PLAIN_REGISTER("greater_equal", PLAIN_NATIVE_GREATER_EQUAL)
    PLAIN_REGISTER(">=",            PLAIN_NATIVE_GREATER_EQUAL)
    PLAIN_REGISTER("join",          PLAIN_NATIVE_CONCAT)
    #undef PLAIN_REGISTER
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Main resolver                                                     */
/* ------------------------------------------------------------------ */

PLAIN_WORD_DOUBLE PLAIN_RESOLVE(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;

    /* Keyword arguments are intentionally left unresolved here. Each callable
     * receives them as PLAIN_TYPE_KEYWORD and decides whether to treat the text
     * as a name (e.g. set, define, class) or resolve it to a value via
     * PLAIN_ARGUMENT_VALUE. Subexpressions [...] are evaluated by the walker. */
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
            if((binding->value.flags & PLAIN_VALUE_USER_DEFINED) && binding->value.data != NULL) {
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
