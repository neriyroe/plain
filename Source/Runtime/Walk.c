/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/09/2013,
 * Revision 01/09/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

/* For use within the translation unit. */
MOCOSEL_WORD_DOUBLE MOCOSEL_EXPAND(void* context, struct MOCOSEL_VALUE* destination, struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT registry, struct MOCOSEL_LIST* source) {
    MOCOSEL_ASSERT(destination != NULL);
    MOCOSEL_ASSERT(registry != NULL);
    MOCOSEL_ASSERT(source != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(destination == NULL || registry == NULL || source == NULL) {
        return 0;
    }
    MOCOSEL_WORD_DOUBLE error = MOCOSEL_WALK(context, source, registry, destination);
    if(error != 0) {
        return error;
    }
    source = (struct MOCOSEL_LIST*)destination->data;
    if(destination->type == MOCOSEL_TYPE_LIST) {
        if(source->parent != NULL) {
            error = MOCOSEL_EXPAND(context, destination, registry, source);
            if(error != 0) {
                return error;
            }
        }
        MOCOSEL_PURGE(source);
    }
    return 0;
}

MOCOSEL_WORD_DOUBLE MOCOSEL_WALK(void* context, struct MOCOSEL_LIST* MOCOSEL_RESTRICT node, struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT registry, struct MOCOSEL_VALUE* value) {
    MOCOSEL_ASSERT(node != NULL);
    MOCOSEL_ASSERT(registry != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(node == NULL || registry == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    if(node->keyword.from == NULL) {
        return 0;
    }
    struct MOCOSEL_STATEMENT* statement = MOCOSEL_LOOKUP(&node->keyword, registry);
    /* MOCOSEL_ERROR_RUNTIME_UNDEFINED_STATEMENT. */
    if(statement == NULL) {
        return MOCOSEL_ERROR_RUNTIME_UNDEFINED_STATEMENT;
    }
    /* Layout. */
    MOCOSEL_WORD_DOUBLE index = 0;
    MOCOSEL_WORD_DOUBLE length = MOCOSEL_MEASURE(node);
    for(; index < length; index++) {
        struct MOCOSEL_VALUE* destination = MOCOSEL_ARGUMENT(node, index);
        if(destination->type == MOCOSEL_TYPE_LIST) {
            struct MOCOSEL_LIST* source = (struct MOCOSEL_LIST*)destination->data;
            if(source->parent == NULL) {
                continue;
            }
            MOCOSEL_WORD_DOUBLE error = MOCOSEL_EXPAND(context, destination, registry, source);
            if(error != 0) {
                return error;
            }
            MOCOSEL_PURGE(source);
        }
    }
    /* Subroutine. */
    if(statement->second.type == MOCOSEL_TYPE_SUBROUTINE) {
        MOCOSEL_WORD_DOUBLE error = ((MOCOSEL_SUBROUTINE)statement->second.data)(context, node, registry, value);
        if(error != 0) {
            return error;
        }
    /* Value. */
    } else {
        /* MOCOSEL_ERROR_RUNTIME_WRONG_DATA. */
        if(value == NULL) {
            return MOCOSEL_ERROR_RUNTIME_WRONG_DATA;
        }
        value->data = statement->second.data;
        value->length = statement->second.length;
        value->type = statement->second.type;
    }
    /* Node. */
    if(node->node) {
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_WALK(context, node->node, registry, NULL);
        if(error != 0) {
            return error;
        }
    }
    return 0;
}
