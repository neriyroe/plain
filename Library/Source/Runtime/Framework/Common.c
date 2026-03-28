/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     03/28/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Framework — Plain standard library.
 * Contains all built-in procedure implementations and PLAIN_FRAMEWORK_REGISTER.
 */

#include <Plain/Framework.h>
#include <uthash.h>
#include <stdio.h>   /* sprintf — string formatting only, no output */

/* ------------------------------------------------------------------ */
/*  Private helpers                                                   */
/* ------------------------------------------------------------------ */

PLAIN_INLINE PLAIN_WORD_DOUBLE PLAIN_KEYWORD_IS(struct PLAIN_LIST* node, const PLAIN_BYTE* name) {
    PLAIN_WORD_DOUBLE length = node->keyword.to - node->keyword.from;
    return length == strlen((const char*)name) && memcmp(node->keyword.from, name, length) == 0;
}

PLAIN_INLINE PLAIN_WORD_DOUBLE PLAIN_KEYWORD_EQ(struct PLAIN_VALUE* value, const PLAIN_BYTE* name) {
    return value != NULL && value->type == PLAIN_TYPE_KEYWORD && strcmp((const char*)value->data, (const char*)name) == 0;
}

static PLAIN_WORD_DOUBLE PLAIN_EVALUATE_BLOCK(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* block, struct PLAIN_VALUE* value) {
    if(block == NULL || block->segment.from == NULL || block->segment.from >= block->segment.to) return 0;
    PLAIN_WORD_DOUBLE length = block->segment.to - block->segment.from;
    PLAIN_BYTE* source = (PLAIN_BYTE*)PLAIN_AUTO(length + 1);
    if(source == NULL) return PLAIN_ERROR_SYSTEM;
    memcpy(source, block->segment.from, length);
    source[length] = '\0';
    return PLAIN_EVALUATE(&context->environment, (PLAIN_SUBROUTINE)PLAIN_RESOLVE, source, context->tracker, value);
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

static PLAIN_BYTE** PLAIN_PARAMETERS_EXTRACT(struct PLAIN_LIST* block, PLAIN_WORD_DOUBLE* count) {
    *count = 0;
    if(block == NULL || block->keyword.from == NULL) return NULL;
    struct PLAIN_LIST* cursor = block;
    PLAIN_WORD_DOUBLE n = 0;
    while(cursor != NULL && cursor->keyword.from != NULL) { n++; cursor = cursor->node; }
    PLAIN_BYTE** names = (PLAIN_BYTE**)PLAIN_RESIZE(NULL, sizeof(PLAIN_BYTE*) * n, 0);
    if(names == NULL) return NULL;
    cursor = block;
    for(PLAIN_WORD_DOUBLE i = 0; i < n; i++) {
        PLAIN_WORD_DOUBLE length = cursor->keyword.to - cursor->keyword.from;
        names[i] = (PLAIN_BYTE*)PLAIN_RESIZE(NULL, length + 1, 0);
        if(names[i] == NULL) {
            for(PLAIN_WORD_DOUBLE j = 0; j < i; j++) PLAIN_RESIZE(names[j], 0, strlen((const char*)names[j]) + 1);
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

/* Variadic left-to-right fold over all arguments using the given arithmetic operation. */
static PLAIN_WORD_DOUBLE PLAIN_ARITHMETIC_FOLD(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, PLAIN_BYTE operation, struct PLAIN_VALUE* value) {
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    if(arity == 0) return 0;
    struct PLAIN_VALUE first = PLAIN_ARGUMENT_VALUE(context, node, 0);
    if(arity == 1) { return value != NULL ? PLAIN_VALUE_COPY(value, &first) : 0; }
    struct PLAIN_VALUE accumulator = {NULL, 0, PLAIN_TYPE_NIL, 0};
    PLAIN_VALUE_COPY(&accumulator, &first);
    PLAIN_WORD_DOUBLE i;
    for(i = 1; i < arity; i++) {
        struct PLAIN_VALUE next = PLAIN_ARGUMENT_VALUE(context, node, i);
        struct PLAIN_VALUE result = {NULL, 0, PLAIN_TYPE_NIL, 0};
        PLAIN_WORD_DOUBLE error = PLAIN_APPLY_ARITHMETIC(&accumulator, &next, operation, &result);
        PLAIN_VALUE_CLEAR(&accumulator);
        if(error != 0) { PLAIN_VALUE_CLEAR(&result); return error; }
        accumulator = result;
    }
    if(value != NULL) *value = accumulator;
    else PLAIN_VALUE_CLEAR(&accumulator);
    return 0;
}

/* Deep-copies a callable struct including its parameter and body strings. */
static struct PLAIN_CALLABLE* PLAIN_CALLABLE_DUPLICATE(struct PLAIN_CALLABLE* source) {
    struct PLAIN_CALLABLE* copy = (struct PLAIN_CALLABLE*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_CALLABLE), 0);
    if(copy == NULL) return NULL;
    memset(copy, 0, sizeof(struct PLAIN_CALLABLE));
    copy->native  = source->native;
    copy->closure = source->closure;
    copy->flags   = source->flags;
    if(copy->closure != NULL) PLAIN_FRAME_RETAIN(copy->closure);
    copy->parameter_count = source->parameter_count;
    if(source->parameters != NULL && source->parameter_count > 0) {
        copy->parameters = (PLAIN_BYTE**)PLAIN_RESIZE(NULL, sizeof(PLAIN_BYTE*) * source->parameter_count, 0);
        if(copy->parameters != NULL) {
            for(PLAIN_WORD_DOUBLE i = 0; i < source->parameter_count; i++) {
                PLAIN_WORD_DOUBLE length = strlen((const char*)source->parameters[i]) + 1;
                copy->parameters[i] = (PLAIN_BYTE*)PLAIN_RESIZE(NULL, length, 0);
                if(copy->parameters[i]) memcpy(copy->parameters[i], source->parameters[i], length);
            }
        }
    }
    if(source->body != NULL) {
        PLAIN_WORD_DOUBLE length = strlen((const char*)source->body) + 1;
        copy->body = (PLAIN_BYTE*)PLAIN_RESIZE(NULL, length, 0);
        if(copy->body) memcpy(copy->body, source->body, length);
    }
    return copy;
}

/* ------------------------------------------------------------------ */
/*  Native callables — all match PLAIN_SUBROUTINE signature          */
/* ------------------------------------------------------------------ */

static PLAIN_WORD_DOUBLE PLAIN_FW_SET(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    if(PLAIN_ARITY(node) < 2) return 0;
    struct PLAIN_VALUE* name_arg = PLAIN_ARGUMENT(node, 0);
    if(name_arg == NULL || name_arg->type != PLAIN_TYPE_KEYWORD) return 0;
    PLAIN_WORD_DOUBLE name_length = strlen((const char*)name_arg->data);
    PLAIN_BYTE* name = (PLAIN_BYTE*)PLAIN_AUTO(name_length + 1);
    memcpy(name, name_arg->data, name_length);
    name[name_length] = '\0';
    struct PLAIN_VALUE assignment_value = PLAIN_ARGUMENT_VALUE(context, node, 1);
    if(assignment_value.type == PLAIN_TYPE_CALLABLE && assignment_value.data != NULL) {
        struct PLAIN_CALLABLE* source = (struct PLAIN_CALLABLE*)assignment_value.data;
        struct PLAIN_CALLABLE* copy = PLAIN_CALLABLE_DUPLICATE(source);
        if(copy == NULL) return PLAIN_ERROR_SYSTEM;
        return PLAIN_FRAME_SET(context->frame, name, NULL, copy, 0);
    }
    PLAIN_FRAME_SET(context->frame, name, &assignment_value, NULL, 0);
    if(value != NULL) PLAIN_VALUE_COPY(value, &assignment_value);
    return 0;
}

static PLAIN_WORD_DOUBLE PLAIN_FW_IF(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
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
        if(PLAIN_VALUE_TRUTHY(&condition))
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

static PLAIN_WORD_DOUBLE PLAIN_FW_WHEN(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
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
            struct PLAIN_VALUE result = {NULL, 0, PLAIN_TYPE_NIL, 0};
            PLAIN_APPLY_COMPARISON(&target, &candidate, '=', &result);
            if(PLAIN_VALUE_TRUTHY(&result)) {
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
    struct PLAIN_VALUE initial = {(PLAIN_BYTE*)&zero, sizeof(PLAIN_WORD_DOUBLE), PLAIN_TYPE_INTEGER, 0};
    PLAIN_FRAME_BIND(frame, name, &initial, NULL, 0);
    struct PLAIN_BINDING* binding = PLAIN_FRAME_FIND(frame, name);
    for(PLAIN_WORD_DOUBLE index = 0; index < count; index++) {
        *(PLAIN_WORD_DOUBLE*)binding->value.data = index;
        struct PLAIN_VALUE result = {NULL, 0, PLAIN_TYPE_NIL, 0};
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

static PLAIN_WORD_DOUBLE PLAIN_FW_REPEAT(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    struct PLAIN_VALUE* first = PLAIN_ARGUMENT(node, 0);
    if(first == NULL) return 0;
    /* repeat {body} — infinite. */
    if(arity == 1 && first->type == PLAIN_TYPE_LIST) {
        struct PLAIN_LIST* body = (struct PLAIN_LIST*)first->data;
        for(;;) {
            struct PLAIN_VALUE result = {NULL, 0, PLAIN_TYPE_NIL, 0};
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
            struct PLAIN_VALUE result = {NULL, 0, PLAIN_TYPE_NIL, 0};
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
            struct PLAIN_VALUE test = {NULL, 0, PLAIN_TYPE_NIL, 0};
            PLAIN_WORD_DOUBLE error = PLAIN_EVALUATE_BLOCK(context, condition, &test);
            PLAIN_WORD_DOUBLE ok = PLAIN_VALUE_TRUTHY(&test);
            PLAIN_VALUE_CLEAR(&test);
            if(error != 0) return error;
            if(!ok) break;
            struct PLAIN_VALUE result = {NULL, 0, PLAIN_TYPE_NIL, 0};
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

static PLAIN_WORD_DOUBLE PLAIN_FW_RETURN(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    PLAIN_VALUE_CLEAR(&context->result);
    if(PLAIN_ARITY(node) > 0) {
        struct PLAIN_VALUE resolved = PLAIN_ARGUMENT_VALUE(context, node, 0);
        PLAIN_VALUE_COPY(&context->result, &resolved);
    }
    return PLAIN_SIGNAL_RETURN;
}

static PLAIN_WORD_DOUBLE PLAIN_FW_BREAK(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    return PLAIN_SIGNAL_BREAK;
}

static PLAIN_WORD_DOUBLE PLAIN_FW_CONTINUE(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    return PLAIN_SIGNAL_CONTINUE;
}

static PLAIN_WORD_DOUBLE PLAIN_FW_DO(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
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
        if(value != NULL) { *value = context->result; context->result = (struct PLAIN_VALUE){NULL, 0, PLAIN_TYPE_NIL, 0}; }
        else { PLAIN_VALUE_CLEAR(&context->result); }
        return 0;
    }
    return error;
}

static PLAIN_WORD_DOUBLE PLAIN_FW_TYPE(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
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

static PLAIN_WORD_DOUBLE PLAIN_FW_DEFINE(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
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
        callable->parameters = PLAIN_PARAMETERS_EXTRACT(parameters, &callable->parameter_count);
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
        callable->parameters = PLAIN_PARAMETERS_EXTRACT(parameters, &callable->parameter_count);
        callable->body       = PLAIN_SEGMENT_COPY(body);
        callable->closure    = context->frame;
        PLAIN_FRAME_RETAIN(context->frame);
        if(value != NULL) { value->type = PLAIN_TYPE_CALLABLE; value->data = (PLAIN_BYTE*)callable; value->length = 0; }
        return 0;
    }
    return 0;
}

static PLAIN_WORD_DOUBLE PLAIN_FW_OBJECT(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
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
    struct PLAIN_VALUE body = {NULL, 0, PLAIN_TYPE_NIL, 0};
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
            self_binding->value.owner  = PLAIN_OWNER_USER;
            self_binding->flags        = PLAIN_BINDING_IMMUTABLE;
        }
    }
    if(value != NULL) {
        value->type   = PLAIN_TYPE_OBJECT;
        value->data   = (PLAIN_BYTE*)frame;
        value->length = 0;
        value->owner  = PLAIN_OWNER_USER;
    }
    PLAIN_FRAME_RELEASE(frame);
    return 0;
}

static PLAIN_WORD_DOUBLE PLAIN_FW_NOT(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE a = PLAIN_ARGUMENT_VALUE(context, node, 0);
    return PLAIN_SET_BOOLEAN(value, !PLAIN_VALUE_TRUTHY(&a));
}

static PLAIN_WORD_DOUBLE PLAIN_FW_AND(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    PLAIN_WORD_DOUBLE i;
    for(i = 0; i < arity; i++) {
        struct PLAIN_VALUE arg = PLAIN_ARGUMENT_VALUE(context, node, i);
        if(!PLAIN_VALUE_TRUTHY(&arg)) return PLAIN_SET_BOOLEAN(value, 0);
    }
    return PLAIN_SET_BOOLEAN(value, 1);
}

static PLAIN_WORD_DOUBLE PLAIN_FW_OR(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    PLAIN_WORD_DOUBLE i;
    for(i = 0; i < arity; i++) {
        struct PLAIN_VALUE arg = PLAIN_ARGUMENT_VALUE(context, node, i);
        if(PLAIN_VALUE_TRUTHY(&arg)) return PLAIN_SET_BOOLEAN(value, 1);
    }
    return PLAIN_SET_BOOLEAN(value, 0);
}

static PLAIN_WORD_DOUBLE PLAIN_FW_ADD(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    return PLAIN_ARITHMETIC_FOLD((struct PLAIN_CONTEXT*)raw, (struct PLAIN_LIST*)data, '+', value);
}

static PLAIN_WORD_DOUBLE PLAIN_FW_SUBTRACT(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    return PLAIN_ARITHMETIC_FOLD((struct PLAIN_CONTEXT*)raw, (struct PLAIN_LIST*)data, '-', value);
}

static PLAIN_WORD_DOUBLE PLAIN_FW_MULTIPLY(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    return PLAIN_ARITHMETIC_FOLD((struct PLAIN_CONTEXT*)raw, (struct PLAIN_LIST*)data, '*', value);
}

static PLAIN_WORD_DOUBLE PLAIN_FW_DIVIDE(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    return PLAIN_ARITHMETIC_FOLD((struct PLAIN_CONTEXT*)raw, (struct PLAIN_LIST*)data, '/', value);
}

static PLAIN_WORD_DOUBLE PLAIN_FW_MODULO(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    return PLAIN_ARITHMETIC_FOLD((struct PLAIN_CONTEXT*)raw, (struct PLAIN_LIST*)data, '%', value);
}

static PLAIN_WORD_DOUBLE PLAIN_FW_EQUAL(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE a = PLAIN_ARGUMENT_VALUE(context, node, 0);
    struct PLAIN_VALUE b = PLAIN_ARGUMENT_VALUE(context, node, 1);
    return PLAIN_APPLY_COMPARISON(&a, &b, '=', value);
}

static PLAIN_WORD_DOUBLE PLAIN_FW_LESS(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE a = PLAIN_ARGUMENT_VALUE(context, node, 0);
    struct PLAIN_VALUE b = PLAIN_ARGUMENT_VALUE(context, node, 1);
    return PLAIN_APPLY_COMPARISON(&a, &b, '<', value);
}

static PLAIN_WORD_DOUBLE PLAIN_FW_GREATER(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE a = PLAIN_ARGUMENT_VALUE(context, node, 0);
    struct PLAIN_VALUE b = PLAIN_ARGUMENT_VALUE(context, node, 1);
    return PLAIN_APPLY_COMPARISON(&a, &b, '>', value);
}

static PLAIN_WORD_DOUBLE PLAIN_FW_NOT_EQUAL(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE a = PLAIN_ARGUMENT_VALUE(context, node, 0);
    struct PLAIN_VALUE b = PLAIN_ARGUMENT_VALUE(context, node, 1);
    struct PLAIN_VALUE temp = {NULL, 0, PLAIN_TYPE_NIL, 0};
    PLAIN_WORD_DOUBLE error = PLAIN_APPLY_COMPARISON(&a, &b, '=', &temp);
    if(error == 0) error = PLAIN_SET_BOOLEAN(value, !PLAIN_VALUE_TRUTHY(&temp));
    PLAIN_VALUE_CLEAR(&temp);
    return error;
}

static PLAIN_WORD_DOUBLE PLAIN_FW_LESS_EQUAL(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE a = PLAIN_ARGUMENT_VALUE(context, node, 0);
    struct PLAIN_VALUE b = PLAIN_ARGUMENT_VALUE(context, node, 1);
    struct PLAIN_VALUE temp = {NULL, 0, PLAIN_TYPE_NIL, 0};
    PLAIN_WORD_DOUBLE error = PLAIN_APPLY_COMPARISON(&a, &b, '>', &temp);
    if(error == 0) error = PLAIN_SET_BOOLEAN(value, !PLAIN_VALUE_TRUTHY(&temp));
    PLAIN_VALUE_CLEAR(&temp);
    return error;
}

static PLAIN_WORD_DOUBLE PLAIN_FW_GREATER_EQUAL(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;
    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    struct PLAIN_VALUE a = PLAIN_ARGUMENT_VALUE(context, node, 0);
    struct PLAIN_VALUE b = PLAIN_ARGUMENT_VALUE(context, node, 1);
    struct PLAIN_VALUE temp = {NULL, 0, PLAIN_TYPE_NIL, 0};
    PLAIN_WORD_DOUBLE error = PLAIN_APPLY_COMPARISON(&a, &b, '<', &temp);
    if(error == 0) error = PLAIN_SET_BOOLEAN(value, !PLAIN_VALUE_TRUTHY(&temp));
    PLAIN_VALUE_CLEAR(&temp);
    return error;
}

static PLAIN_WORD_DOUBLE PLAIN_FW_CONCAT(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
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
        if(length > 0) PLAIN_CONCATENATE(&result, length, str);
    }
    PLAIN_BYTE zero = 0;
    PLAIN_CONCATENATE(&result, 1, &zero);
    value->data = result.from;
    value->length = result.to - result.from;
    value->type = PLAIN_TYPE_STRING;
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Framework init — register all built-ins in the root frame        */
/* ------------------------------------------------------------------ */

PLAIN_WORD_DOUBLE PLAIN_FRAMEWORK_REGISTER(struct PLAIN_CONTEXT* context) {
    #define PLAIN_INTERNAL_REGISTER(name, fn) \
        if(PLAIN_CONTEXT_REGISTER(context, (const PLAIN_BYTE*)(name), fn) != 0) return PLAIN_ERROR_SYSTEM
    PLAIN_INTERNAL_REGISTER("if",            PLAIN_FW_IF);
    PLAIN_INTERNAL_REGISTER("when",          PLAIN_FW_WHEN);
    PLAIN_INTERNAL_REGISTER("repeat",        PLAIN_FW_REPEAT);
    PLAIN_INTERNAL_REGISTER("return",        PLAIN_FW_RETURN);
    PLAIN_INTERNAL_REGISTER("break",         PLAIN_FW_BREAK);
    PLAIN_INTERNAL_REGISTER("continue",      PLAIN_FW_CONTINUE);
    PLAIN_INTERNAL_REGISTER("do",            PLAIN_FW_DO);
    PLAIN_INTERNAL_REGISTER("type",          PLAIN_FW_TYPE);
    PLAIN_INTERNAL_REGISTER("set",           PLAIN_FW_SET);
    PLAIN_INTERNAL_REGISTER("define",        PLAIN_FW_DEFINE);
    PLAIN_INTERNAL_REGISTER("object",        PLAIN_FW_OBJECT);
    PLAIN_INTERNAL_REGISTER("not",           PLAIN_FW_NOT);
    PLAIN_INTERNAL_REGISTER("and",           PLAIN_FW_AND);
    PLAIN_INTERNAL_REGISTER("or",            PLAIN_FW_OR);
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
    PLAIN_INTERNAL_REGISTER("join",          PLAIN_FW_CONCAT);
    #undef PLAIN_INTERNAL_REGISTER
    return 0;
}
