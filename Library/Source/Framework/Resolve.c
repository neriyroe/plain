/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     11/01/2014.
 * Revision 03/22/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 */

#include <Plain/VM.h>
#include <Plain/Framework/Scope.h>

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
    if(length >= size) {
        length = size - 1;
    }
    memcpy(buffer, node->keyword.from, length);
    buffer[length] = '\0';
}

static PLAIN_WORD_DOUBLE PLAIN_EVALUATE_BLOCK(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* block, struct PLAIN_VALUE* value) {
    if(block == NULL || block->segment.from == NULL || block->segment.from >= block->segment.to) {
        return 0;
    }
    PLAIN_WORD_DOUBLE length = block->segment.to - block->segment.from;
    PLAIN_BYTE* source = (PLAIN_BYTE*)PLAIN_AUTO(length + 1);
    if(source == NULL) {
        return PLAIN_ERROR_SYSTEM;
    }
    memcpy(source, block->segment.from, length);
    source[length] = '\0';
    return PLAIN_EVALUATE(&context->environment, &PLAIN_RESOLVE, source, context->tracker, value);
}

static PLAIN_BYTE* PLAIN_SEGMENT_COPY(struct PLAIN_LIST* block) {
    PLAIN_BYTE* data = NULL;
    if(block == NULL || block->segment.from == NULL || block->segment.from >= block->segment.to) {
        data = (PLAIN_BYTE*)PLAIN_RESIZE(NULL, 1, 0);
        if(data != NULL) {
            data[0] = '\0';
        }
        return data;
    }
    PLAIN_WORD_DOUBLE length = block->segment.to - block->segment.from;
    data = (PLAIN_BYTE*)PLAIN_RESIZE(NULL, length + 1, 0);
    if(data != NULL) {
        memcpy(data, block->segment.from, length);
        data[length] = '\0';
    }
    return data;
}

PLAIN_INLINE PLAIN_WORD_DOUBLE PLAIN_SET_INTEGER(struct PLAIN_VALUE* value, PLAIN_WORD_DOUBLE number) {
    if(value == NULL) {
        return 0;
    }
    return PLAIN_EXPORT((PLAIN_BYTE*)&number, sizeof(PLAIN_WORD_DOUBLE), PLAIN_TYPE_INTEGER, value);
}

PLAIN_INLINE PLAIN_WORD_DOUBLE PLAIN_SET_BOOLEAN(struct PLAIN_VALUE* value, PLAIN_WORD_DOUBLE boolean) {
    if(value == NULL) {
        return 0;
    }
    return PLAIN_EXPORT(boolean ? (PLAIN_BYTE*)0xFF : NULL, 0, PLAIN_TYPE_BOOLEAN, value);
}

PLAIN_INLINE PLAIN_REAL_DOUBLE PLAIN_AS_REAL(struct PLAIN_VALUE* value) {
    if(value->type == PLAIN_TYPE_REAL)    return (PLAIN_REAL_DOUBLE)*(PLAIN_REAL*)value->data;
    if(value->type == PLAIN_TYPE_INTEGER) return (PLAIN_REAL_DOUBLE)(int)*(PLAIN_WORD_DOUBLE*)value->data;
    if(value->type == PLAIN_TYPE_BOOLEAN) return value->data != NULL ? 1.0 : 0.0;
    return 0.0;
}

/* ------------------------------------------------------------------ */
/*  Callable dispatch                                                 */
/* ------------------------------------------------------------------ */

static PLAIN_WORD_DOUBLE PLAIN_CALL(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, struct PLAIN_CALLABLE* callable, struct PLAIN_VALUE* value) {
    struct PLAIN_FRAME* frame = PLAIN_FRAME_CREATE(context->frame);
    if(frame == NULL) {
        return PLAIN_ERROR_SYSTEM;
    }
    /* Bind each named parameter to the corresponding argument. */
    const PLAIN_BYTE* pointer = callable->parameters;
    PLAIN_WORD_DOUBLE index = 0;
    while(pointer != NULL && *pointer != '\0') {
        while(*pointer == ' ' || *pointer == '\t' || *pointer == ',' || *pointer == '\n' || *pointer == '\r') {
            pointer++;
        }
        if(*pointer == '\0') {
            break;
        }
        const PLAIN_BYTE* start = pointer;
        while(*pointer != '\0' && *pointer != ' ' && *pointer != '\t' && *pointer != ',' && *pointer != '\n' && *pointer != '\r') {
            pointer++;
        }
        PLAIN_BYTE name[256];
        PLAIN_WORD_DOUBLE length = pointer - start;
        if(length >= sizeof(name)) {
            length = sizeof(name) - 1;
        }
        memcpy(name, start, length);
        name[length] = '\0';
        struct PLAIN_VALUE* argument = PLAIN_ARGUMENT(node, index);
        if(argument != NULL) {
            PLAIN_FRAME_BIND(frame, name, argument, NULL, 0);
        }
        index++;
    }
    /* Evaluate body in the new frame. */
    struct PLAIN_FRAME* saved = context->frame;
    context->frame = frame;
    struct PLAIN_VALUE body = {NULL, 0, PLAIN_TYPE_NIL};
    PLAIN_WORD_DOUBLE error = PLAIN_EVALUATE(&context->environment, &PLAIN_RESOLVE, callable->body, context->tracker, &body);
    context->frame = saved;
    PLAIN_FRAME_DESTROY(frame);
    /* Unwrap return signal. */
    if(error == PLAIN_SIGNAL_RETURN) {
        if(value != NULL) {
            *value = context->result;
            context->result = (struct PLAIN_VALUE){NULL, 0, PLAIN_TYPE_NIL};
        } else {
            PLAIN_VALUE_CLEAR(&context->result);
        }
        PLAIN_VALUE_CLEAR(&body);
        return 0;
    }
    if(error == 0 && value != NULL) {
        *value = body;
    } else {
        PLAIN_VALUE_CLEAR(&body);
    }
    return error;
}

/* ------------------------------------------------------------------ */
/*  Built-in commands                                                 */
/* ------------------------------------------------------------------ */

static PLAIN_WORD_DOUBLE PLAIN_BUILTIN_ASSIGN(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, struct PLAIN_VALUE* value) {
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    if(arity < 2) {
        return 0;
    }
    PLAIN_BYTE name[256];
    PLAIN_KEYWORD_EXTRACT(node, name, sizeof(name));
    struct PLAIN_VALUE* second = PLAIN_ARGUMENT(node, 1);
    /* x = function {parameters} {body} */
    if(arity >= 4 && PLAIN_KEYWORD_EQ(second, (const PLAIN_BYTE*)"function")) {
        struct PLAIN_LIST* parameters = (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 2)->data;
        struct PLAIN_LIST* body = (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 3)->data;
        struct PLAIN_CALLABLE* callable = (struct PLAIN_CALLABLE*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_CALLABLE), 0);
        if(callable == NULL) {
            return PLAIN_ERROR_SYSTEM;
        }
        callable->parameters = PLAIN_SEGMENT_COPY(parameters);
        callable->body = PLAIN_SEGMENT_COPY(body);
        PLAIN_FRAME_BIND(context->frame, name, NULL, callable, PLAIN_BINDING_IMMUTABLE);
        return 0;
    }
    /* x = procedure {parameters} {body} */
    if(arity >= 4 && PLAIN_KEYWORD_EQ(second, (const PLAIN_BYTE*)"procedure")) {
        struct PLAIN_LIST* parameters = (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 2)->data;
        struct PLAIN_LIST* body = (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 3)->data;
        struct PLAIN_CALLABLE* callable = (struct PLAIN_CALLABLE*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_CALLABLE), 0);
        if(callable == NULL) {
            return PLAIN_ERROR_SYSTEM;
        }
        callable->parameters = PLAIN_SEGMENT_COPY(parameters);
        callable->body = PLAIN_SEGMENT_COPY(body);
        PLAIN_FRAME_BIND(context->frame, name, NULL, callable, 0);
        return 0;
    }
    /* x = value */
    PLAIN_FRAME_BIND(context->frame, name, second, NULL, 0);
    if(value != NULL) {
        PLAIN_VALUE_COPY(value, second);
    }
    return 0;
}

static PLAIN_WORD_DOUBLE PLAIN_BUILTIN_DEFINE(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, PLAIN_WORD_DOUBLE flags) {
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    if(arity < 3) {
        return 0;
    }
    struct PLAIN_VALUE* name = PLAIN_ARGUMENT(node, 0);
    if(name->type != PLAIN_TYPE_KEYWORD) {
        return 0;
    }
    struct PLAIN_LIST* parameters = (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 1)->data;
    struct PLAIN_LIST* body = (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 2)->data;
    struct PLAIN_CALLABLE* callable = (struct PLAIN_CALLABLE*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_CALLABLE), 0);
    if(callable == NULL) {
        return PLAIN_ERROR_SYSTEM;
    }
    callable->parameters = PLAIN_SEGMENT_COPY(parameters);
    callable->body = PLAIN_SEGMENT_COPY(body);
    PLAIN_FRAME_BIND(context->frame, name->data, NULL, callable, flags);
    return 0;
}

static PLAIN_WORD_DOUBLE PLAIN_BUILTIN_IF(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, struct PLAIN_VALUE* value) {
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    if(arity < 2) {
        return 0;
    }
    if(PLAIN_IS_TRUE(PLAIN_ARGUMENT(node, 0))) {
        return PLAIN_EVALUATE_BLOCK(context, (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 1)->data, value);
    } else if(arity >= 3) {
        return PLAIN_EVALUATE_BLOCK(context, (struct PLAIN_LIST*)PLAIN_ARGUMENT(node, 2)->data, value);
    }
    return 0;
}

static PLAIN_WORD_DOUBLE PLAIN_BUILTIN_REPEAT(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, struct PLAIN_VALUE* value) {
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    struct PLAIN_VALUE* first = PLAIN_ARGUMENT(node, 0);
    if(first == NULL) {
        return 0;
    }
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
    if(arity < 2 || second == NULL || second->type != PLAIN_TYPE_LIST) {
        return 0;
    }
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

static PLAIN_WORD_DOUBLE PLAIN_BUILTIN_RETURN(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node) {
    PLAIN_VALUE_CLEAR(&context->result);
    if(PLAIN_ARITY(node) > 0) {
        PLAIN_VALUE_COPY(&context->result, PLAIN_ARGUMENT(node, 0));
    }
    return PLAIN_SIGNAL_RETURN;
}

static PLAIN_WORD_DOUBLE PLAIN_BUILTIN_ARITHMETIC(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, PLAIN_BYTE operation, struct PLAIN_VALUE* value) {
    struct PLAIN_VALUE* a = PLAIN_ARGUMENT(node, 0);
    struct PLAIN_VALUE* b = PLAIN_ARGUMENT(node, 1);
    if(a == NULL || b == NULL) {
        return 0;
    }
    if(a->type == PLAIN_TYPE_REAL || b->type == PLAIN_TYPE_REAL) {
        PLAIN_REAL_DOUBLE va = PLAIN_AS_REAL(a);
        PLAIN_REAL_DOUBLE vb = PLAIN_AS_REAL(b);
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
        if(value != NULL) {
            return PLAIN_SET_INTEGER(value, (PLAIN_WORD_DOUBLE)result);
        }
    }
    return 0;
}

static PLAIN_WORD_DOUBLE PLAIN_BUILTIN_COMPARISON(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, PLAIN_BYTE operation, struct PLAIN_VALUE* value) {
    struct PLAIN_VALUE* a = PLAIN_ARGUMENT(node, 0);
    struct PLAIN_VALUE* b = PLAIN_ARGUMENT(node, 1);
    if(a == NULL || b == NULL) {
        return 0;
    }
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
    PLAIN_REAL_DOUBLE va = PLAIN_AS_REAL(a);
    PLAIN_REAL_DOUBLE vb = PLAIN_AS_REAL(b);
    PLAIN_WORD_DOUBLE result = 0;
    switch(operation) {
        case '=': result = va == vb; break;
        case '<': result = va < vb;  break;
        case '>': result = va > vb;  break;
    }
    return PLAIN_SET_BOOLEAN(value, result);
}

static PLAIN_WORD_DOUBLE PLAIN_BUILTIN_NOT(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, struct PLAIN_VALUE* value) {
    struct PLAIN_VALUE* a = PLAIN_ARGUMENT(node, 0);
    if(a == NULL) {
        return 0;
    }
    return PLAIN_SET_BOOLEAN(value, !PLAIN_IS_TRUE(a));
}

static PLAIN_WORD_DOUBLE PLAIN_BUILTIN_AND(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, struct PLAIN_VALUE* value) {
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    PLAIN_WORD_DOUBLE i = 0;
    for(; i < arity; i++) {
        if(!PLAIN_IS_TRUE(PLAIN_ARGUMENT(node, i))) {
            return PLAIN_SET_BOOLEAN(value, 0);
        }
    }
    return PLAIN_SET_BOOLEAN(value, 1);
}

static PLAIN_WORD_DOUBLE PLAIN_BUILTIN_OR(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, struct PLAIN_VALUE* value) {
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    PLAIN_WORD_DOUBLE i = 0;
    for(; i < arity; i++) {
        if(PLAIN_IS_TRUE(PLAIN_ARGUMENT(node, i))) {
            return PLAIN_SET_BOOLEAN(value, 1);
        }
    }
    return PLAIN_SET_BOOLEAN(value, 0);
}

static PLAIN_WORD_DOUBLE PLAIN_BUILTIN_CONCAT(struct PLAIN_CONTEXT* context, struct PLAIN_LIST* node, struct PLAIN_VALUE* value) {
    if(value == NULL) {
        return 0;
    }
    struct PLAIN_SEGMENT result = {NULL, NULL};
    PLAIN_WORD_DOUBLE arity = PLAIN_ARITY(node);
    PLAIN_WORD_DOUBLE i = 0;
    for(; i < arity; i++) {
        struct PLAIN_VALUE* argument = PLAIN_ARGUMENT(node, i);
        PLAIN_BYTE buffer[64];
        const PLAIN_BYTE* data = (const PLAIN_BYTE*)"";
        PLAIN_WORD_DOUBLE length = 0;
        switch(argument->type) {
            case PLAIN_TYPE_STRING:
                data = argument->data;
                length = (PLAIN_WORD_DOUBLE)strlen((const char*)data);
                break;
            case PLAIN_TYPE_INTEGER:
                length = sprintf((char*)buffer, "%d", (int)*(PLAIN_WORD_DOUBLE*)argument->data);
                data = buffer;
                break;
            case PLAIN_TYPE_REAL:
                length = sprintf((char*)buffer, "%g", (PLAIN_REAL_DOUBLE)*(PLAIN_REAL*)argument->data);
                data = buffer;
                break;
            case PLAIN_TYPE_BOOLEAN:
                data = argument->data != NULL ? (const PLAIN_BYTE*)"yes" : (const PLAIN_BYTE*)"no";
                length = (PLAIN_WORD_DOUBLE)strlen((const char*)data);
                break;
            default:
                data = (const PLAIN_BYTE*)"none";
                length = 4;
                break;
        }
        if(length > 0) {
            PLAIN_CONCAT(&result, length, data);
        }
    }
    PLAIN_BYTE zero = 0;
    PLAIN_CONCAT(&result, 1, &zero);
    value->data = result.from;
    value->length = result.to - result.from;
    value->type = PLAIN_TYPE_STRING;
    return 0;
}

/* ------------------------------------------------------------------ */
/*  Main resolver                                                     */
/* ------------------------------------------------------------------ */

PLAIN_WORD_DOUBLE PLAIN_RESOLVE(void* raw, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    struct PLAIN_CONTEXT* context = (struct PLAIN_CONTEXT*)raw;

    /* Variable substitution. */
    if(type == PLAIN_TYPE_KEYWORD) {
        struct PLAIN_BINDING* binding = PLAIN_FRAME_FIND(context->frame, (const PLAIN_BYTE*)data);
        if(binding != NULL) {
            PLAIN_VALUE_COPY(value, &binding->value);
        }
        return 0;
    }

    if(type != PLAIN_TYPE_LIST) {
        return 0;
    }

    struct PLAIN_LIST* node = (struct PLAIN_LIST*)data;
    if(node->keyword.from == node->keyword.to) {
        return 0;
    }

    /* Assignment: name = value */
    if(PLAIN_ARITY(node) >= 2 && PLAIN_KEYWORD_EQ(PLAIN_ARGUMENT(node, 0), (const PLAIN_BYTE*)"=")) {
        return PLAIN_BUILTIN_ASSIGN(context, node, value);
    }

    /* Control flow. */
    if(PLAIN_KEYWORD_IS(node, (const PLAIN_BYTE*)"if"))        return PLAIN_BUILTIN_IF(context, node, value);
    if(PLAIN_KEYWORD_IS(node, (const PLAIN_BYTE*)"repeat"))    return PLAIN_BUILTIN_REPEAT(context, node, value);
    if(PLAIN_KEYWORD_IS(node, (const PLAIN_BYTE*)"return"))    return PLAIN_BUILTIN_RETURN(context, node);
    if(PLAIN_KEYWORD_IS(node, (const PLAIN_BYTE*)"break"))     return PLAIN_SIGNAL_BREAK;

    /* Definitions. */
    if(PLAIN_KEYWORD_IS(node, (const PLAIN_BYTE*)"function"))  return PLAIN_BUILTIN_DEFINE(context, node, PLAIN_BINDING_IMMUTABLE);
    if(PLAIN_KEYWORD_IS(node, (const PLAIN_BYTE*)"procedure")) return PLAIN_BUILTIN_DEFINE(context, node, 0);

    /* Arithmetic. */
    if(PLAIN_KEYWORD_IS(node, (const PLAIN_BYTE*)"add"))       return PLAIN_BUILTIN_ARITHMETIC(context, node, '+', value);
    if(PLAIN_KEYWORD_IS(node, (const PLAIN_BYTE*)"subtract"))  return PLAIN_BUILTIN_ARITHMETIC(context, node, '-', value);
    if(PLAIN_KEYWORD_IS(node, (const PLAIN_BYTE*)"multiply"))  return PLAIN_BUILTIN_ARITHMETIC(context, node, '*', value);
    if(PLAIN_KEYWORD_IS(node, (const PLAIN_BYTE*)"divide"))    return PLAIN_BUILTIN_ARITHMETIC(context, node, '/', value);
    if(PLAIN_KEYWORD_IS(node, (const PLAIN_BYTE*)"modulo"))    return PLAIN_BUILTIN_ARITHMETIC(context, node, '%', value);

    /* Comparison. */
    if(PLAIN_KEYWORD_IS(node, (const PLAIN_BYTE*)"equal"))     return PLAIN_BUILTIN_COMPARISON(context, node, '=', value);
    if(PLAIN_KEYWORD_IS(node, (const PLAIN_BYTE*)"less"))      return PLAIN_BUILTIN_COMPARISON(context, node, '<', value);
    if(PLAIN_KEYWORD_IS(node, (const PLAIN_BYTE*)"greater"))   return PLAIN_BUILTIN_COMPARISON(context, node, '>', value);

    /* Logic. */
    if(PLAIN_KEYWORD_IS(node, (const PLAIN_BYTE*)"not"))       return PLAIN_BUILTIN_NOT(context, node, value);
    if(PLAIN_KEYWORD_IS(node, (const PLAIN_BYTE*)"and"))       return PLAIN_BUILTIN_AND(context, node, value);
    if(PLAIN_KEYWORD_IS(node, (const PLAIN_BYTE*)"or"))        return PLAIN_BUILTIN_OR(context, node, value);

    /* String. */
    if(PLAIN_KEYWORD_IS(node, (const PLAIN_BYTE*)"concat"))    return PLAIN_BUILTIN_CONCAT(context, node, value);

    /* User-defined callable. */
    PLAIN_BYTE keyword[256];
    PLAIN_KEYWORD_EXTRACT(node, keyword, sizeof(keyword));
    struct PLAIN_BINDING* binding = PLAIN_FRAME_FIND(context->frame, keyword);
    if(binding != NULL && binding->callable != NULL) {
        return PLAIN_CALL(context, node, binding->callable, value);
    }

    /* Host-provided extension. */
    if(context->handler != NULL) {
        return context->handler(raw, data, type, value);
    }

    return 0;
}
