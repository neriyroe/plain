/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     03/22/2026.
 * Revision 03/28/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 */

#include <Plain/Runtime/Context.h>
#include <uthash.h>

struct PLAIN_FRAME* PLAIN_FRAME_CREATE(struct PLAIN_FRAME* parent) {
    struct PLAIN_FRAME* frame = (struct PLAIN_FRAME*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_FRAME), 0);
    if(frame == NULL) return NULL;
    memset(frame, 0, sizeof(struct PLAIN_FRAME));
    frame->parent = parent;
    return frame;
}

void PLAIN_FRAME_RETAIN(struct PLAIN_FRAME* frame) {
    if(frame != NULL) frame->references++;
}

void PLAIN_FRAME_RELEASE(struct PLAIN_FRAME* frame) {
    if(frame == NULL) return;
    if(frame->references > 0) { frame->references--; return; }
    PLAIN_FRAME_DESTROY(frame);
}

static void PLAIN_FRAME_BINDING_FREE(struct PLAIN_BINDING* binding) {
    if(binding->name != NULL) {
        PLAIN_RESIZE(binding->name, 0, strlen((const char*)binding->name) + 1);
    }
    PLAIN_VALUE_CLEAR(&binding->value);
    if(binding->callable != NULL) {
        if(binding->callable->closure != NULL) {
            PLAIN_FRAME_RELEASE(binding->callable->closure);
        }
        if(binding->callable->parameters != NULL) {
            for(PLAIN_WORD_DOUBLE i = 0; i < binding->callable->parameter_count; i++) {
                if(binding->callable->parameters[i] != NULL)
                    PLAIN_RESIZE(binding->callable->parameters[i], 0, strlen((const char*)binding->callable->parameters[i]) + 1);
            }
            PLAIN_RESIZE(binding->callable->parameters, 0, sizeof(PLAIN_BYTE*) * binding->callable->parameter_count);
        }
        if(binding->callable->body != NULL) {
            PLAIN_UNLINK(binding->callable->body);
            PLAIN_RESIZE(binding->callable->body, 0, sizeof(struct PLAIN_LIST));
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
        /* Break circular references before freeing to avoid re-entrant destroy:
         * - Method closures that point back to this frame.
         * - The `self` binding that holds this frame as a PLAIN_TYPE_OBJECT value. */
        if(binding->callable != NULL && binding->callable->closure == frame) {
            binding->callable->closure = NULL;
        }
        if(binding->value.type == PLAIN_TYPE_OBJECT && binding->value.data == (PLAIN_BYTE*)frame) {
            binding->value.data = NULL;
            binding->value.length = 0;
            binding->value.owner = 0;
            binding->value.type = PLAIN_TYPE_NIL;
        }
        HASH_DEL(frame->bindings, binding);
        PLAIN_FRAME_BINDING_FREE(binding);
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
                for(PLAIN_WORD_DOUBLE i = 0; i < binding->callable->parameter_count; i++) {
                    if(binding->callable->parameters[i] != NULL)
                        PLAIN_RESIZE(binding->callable->parameters[i], 0, strlen((const char*)binding->callable->parameters[i]) + 1);
                }
                PLAIN_RESIZE(binding->callable->parameters, 0, sizeof(PLAIN_BYTE*) * binding->callable->parameter_count);
            }
            if(binding->callable->body != NULL) {
                PLAIN_UNLINK(binding->callable->body);
                PLAIN_RESIZE(binding->callable->body, 0, sizeof(struct PLAIN_LIST));
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

PLAIN_WORD_DOUBLE PLAIN_FRAME_SET(struct PLAIN_FRAME* frame, const PLAIN_BYTE* name, struct PLAIN_VALUE* value, struct PLAIN_CALLABLE* callable, PLAIN_WORD_DOUBLE flags) {
    struct PLAIN_BINDING* existing = PLAIN_FRAME_FIND(frame, name);
    if(existing != NULL) {
        struct PLAIN_FRAME* target = frame;
        PLAIN_WORD_DOUBLE length = strlen((const char*)name);
        while(target != NULL) {
            struct PLAIN_BINDING* local = NULL;
            HASH_FIND(hh, target->bindings, name, length, local);
            if(local == existing) return PLAIN_FRAME_BIND(target, name, value, callable, flags);
            target = target->parent;
        }
    }
    return PLAIN_FRAME_BIND(frame, name, value, callable, flags);
}

