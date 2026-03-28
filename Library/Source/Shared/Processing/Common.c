/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     03/28/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 */

#include <Plain/Runtime/Frame.h>

void PLAIN_VALUE_CLEAR(struct PLAIN_VALUE* value) {
    if(value->type == PLAIN_TYPE_OBJECT && (value->owner & PLAIN_OWNER_USER)) {
        if(value->data != NULL) PLAIN_FRAME_RELEASE((struct PLAIN_FRAME*)value->data);
        value->data = NULL;
        value->length = 0;
        value->owner = 0;
        value->type = PLAIN_TYPE_NIL;
        return;
    }
    if(value->length > 0 && value->data != NULL) {
        PLAIN_RESIZE(value->data, 0, value->length);
    }
    value->data = NULL;
    value->length = 0;
    value->owner = 0;
    value->type = PLAIN_TYPE_NIL;
}

PLAIN_WORD_DOUBLE PLAIN_VALUE_COPY(struct PLAIN_VALUE* destination, const struct PLAIN_VALUE* source) {
    destination->type = source->type;
    destination->length = source->length;
    destination->owner = source->owner;
    if(source->type == PLAIN_TYPE_OBJECT && (source->owner & PLAIN_OWNER_USER)) {
        destination->data = source->data;
        if(source->data != NULL) PLAIN_FRAME_RETAIN((struct PLAIN_FRAME*)source->data);
        return 0;
    }
    if(source->length > 0 && source->data != NULL) {
        destination->data = (PLAIN_BYTE*)PLAIN_RESIZE(NULL, source->length, 0);
        if(destination->data == NULL) return PLAIN_ERROR_SYSTEM;
        memcpy(destination->data, source->data, source->length);
    } else {
        destination->data = source->data;
    }
    return 0;
}

PLAIN_WORD_DOUBLE PLAIN_VALUE_TRUTHY(const struct PLAIN_VALUE* value) {
    switch(value->type) {
        case PLAIN_TYPE_BOOLEAN: return value->data != NULL;
        case PLAIN_TYPE_INTEGER: return *(PLAIN_WORD_DOUBLE*)value->data != 0;
        case PLAIN_TYPE_REAL:    return *(PLAIN_REAL*)value->data != 0.0f;
        case PLAIN_TYPE_STRING:  return value->data[0] != '\0';
        case PLAIN_TYPE_NIL:     return 0;
        default:                 return value->data != NULL;
    }
}
