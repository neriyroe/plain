/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/09/2013,
 * Revision 11/16/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_WALK(void* MOCOSEL_RESTRICT context, struct MOCOSEL_LIST* MOCOSEL_RESTRICT node, struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT registry, struct MOCOSEL_VALUE* MOCOSEL_RESTRICT value) {
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
    /* Subroutine. */
    if(statement->second.type == MOCOSEL_TYPE_SUBROUTINE) {
        MOCOSEL_WORD_DOUBLE error = ((MOCOSEL_SUBROUTINE)statement->second.data)(context, node, registry, value);
        if (error != 0) {
            return error;
        }
    /* Value. */
    } else if(value != NULL) {
        /* MOCOSEL_ERROR_SYSTEM. */
        if(memcpy(value, &statement->second, sizeof(struct MOCOSEL_VALUE)) == NULL) {
            return MOCOSEL_ERROR_SYSTEM;
        }
    }
    if(node->node) {
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_WALK(context, node->node, registry, NULL);
        if(error != 0) {
            return error;
        }
    }
    return 0;
}
