/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     03/22/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 */

#include <Plain/Framework/Scope.h>

struct PLAIN_FRAME* PLAIN_FRAME_CREATE(struct PLAIN_FRAME* parent) {
    struct PLAIN_FRAME* frame = (struct PLAIN_FRAME*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_FRAME), 0);
    if(frame == NULL) return NULL;
    memset(frame, 0, sizeof(struct PLAIN_FRAME));
    frame->parent = parent;
    return frame;
}

static void PLAIN_BINDING_FREE(struct PLAIN_BINDING* binding) {
    if(binding->name != NULL) {
        PLAIN_RESIZE(binding->name, 0, strlen((const char*)binding->name) + 1);
    }
    PLAIN_VALUE_CLEAR(&binding->value);
    if(binding->callable != NULL) {
        if(binding->callable->parameters != NULL) {
            PLAIN_RESIZE(binding->callable->parameters, 0, strlen((const char*)binding->callable->parameters) + 1);
        }
        if(binding->callable->body != NULL) {
            PLAIN_RESIZE(binding->callable->body, 0, strlen((const char*)binding->callable->body) + 1);
        }
        PLAIN_RESIZE(binding->callable, 0, sizeof(struct PLAIN_CALLABLE));
    }
    PLAIN_RESIZE(binding, 0, sizeof(struct PLAIN_BINDING));
}

void PLAIN_FRAME_DESTROY(struct PLAIN_FRAME* frame) {
    if(frame == NULL) return;
    struct PLAIN_BINDING* binding;
    struct PLAIN_BINDING* tmp;
    HASH_ITER(hh, frame->bindings, binding, tmp) {
        HASH_DEL(frame->bindings, binding);
        PLAIN_BINDING_FREE(binding);
    }
    PLAIN_RESIZE(frame, 0, sizeof(struct PLAIN_FRAME));
}

struct PLAIN_BINDING* PLAIN_FRAME_FIND(struct PLAIN_FRAME* frame, const PLAIN_BYTE* name) {
    PLAIN_WORD_DOUBLE length = strlen((const char*)name);
    while(frame != NULL) {
        struct PLAIN_BINDING* binding = NULL;
        HASH_FIND(hh, frame->bindings, name, length, binding);
        if(binding != NULL) return binding;
        frame = frame->parent;
    }
    return NULL;
}

PLAIN_WORD_DOUBLE PLAIN_FRAME_BIND(struct PLAIN_FRAME* frame, const PLAIN_BYTE* name, struct PLAIN_VALUE* value, struct PLAIN_CALLABLE* callable, PLAIN_WORD_DOUBLE flags) {
    PLAIN_WORD_DOUBLE length = strlen((const char*)name);
    struct PLAIN_BINDING* binding = NULL;
    HASH_FIND(hh, frame->bindings, name, length, binding);

    if(binding != NULL) {
        if(binding->flags & PLAIN_BINDING_IMMUTABLE) return PLAIN_ERROR_SYSTEM_WRONG_DATA;
        PLAIN_VALUE_CLEAR(&binding->value);
        if(binding->callable != NULL) {
            if(binding->callable->parameters != NULL) {
                PLAIN_RESIZE(binding->callable->parameters, 0, strlen((const char*)binding->callable->parameters) + 1);
            }
            if(binding->callable->body != NULL) {
                PLAIN_RESIZE(binding->callable->body, 0, strlen((const char*)binding->callable->body) + 1);
            }
            PLAIN_RESIZE(binding->callable, 0, sizeof(struct PLAIN_CALLABLE));
            binding->callable = NULL;
        }
    } else {
        binding = (struct PLAIN_BINDING*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_BINDING), 0);
        if(binding == NULL) return PLAIN_ERROR_SYSTEM;
        memset(binding, 0, sizeof(struct PLAIN_BINDING));
        binding->name = (PLAIN_BYTE*)PLAIN_RESIZE(NULL, length + 1, 0);
        if(binding->name == NULL) {
            PLAIN_RESIZE(binding, 0, sizeof(struct PLAIN_BINDING));
            return PLAIN_ERROR_SYSTEM;
        }
        memcpy(binding->name, name, length + 1);
        HASH_ADD_KEYPTR(hh, frame->bindings, binding->name, length, binding);
    }

    if(value != NULL) PLAIN_VALUE_COPY(&binding->value, value);
    binding->callable = callable;
    binding->flags = flags;
    return 0;
}

void PLAIN_VALUE_CLEAR(struct PLAIN_VALUE* value) {
    if(value->length > 0 && value->data != NULL) {
        PLAIN_RESIZE(value->data, 0, value->length);
    }
    value->data = NULL;
    value->length = 0;
    value->type = PLAIN_TYPE_NIL;
}

PLAIN_WORD_DOUBLE PLAIN_VALUE_COPY(struct PLAIN_VALUE* destination, const struct PLAIN_VALUE* source) {
    destination->type = source->type;
    destination->length = source->length;
    if(source->length > 0 && source->data != NULL) {
        destination->data = (PLAIN_BYTE*)PLAIN_RESIZE(NULL, source->length, 0);
        if(destination->data == NULL) return PLAIN_ERROR_SYSTEM;
        memcpy(destination->data, source->data, source->length);
    } else {
        destination->data = source->data;
    }
    return 0;
}

PLAIN_WORD_DOUBLE PLAIN_IS_TRUE(const struct PLAIN_VALUE* value) {
    switch(value->type) {
        case PLAIN_TYPE_BOOLEAN: return value->data != NULL;
        case PLAIN_TYPE_INTEGER: return *(PLAIN_WORD_DOUBLE*)value->data != 0;
        case PLAIN_TYPE_REAL:    return *(PLAIN_REAL*)value->data != 0.0f;
        case PLAIN_TYPE_STRING:  return value->data[0] != '\0';
        case PLAIN_TYPE_NIL:     return 0;
        default:                 return value->data != NULL;
    }
}
