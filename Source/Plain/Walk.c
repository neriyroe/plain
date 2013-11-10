/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/09/2013,
 * Revision 10/16/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_WALK(struct MOCOSEL_LIST* __restrict node, struct MOCOSEL_SEGMENT* __restrict registry, struct MOCOSEL_VALUE* __restrict value) {
    MOCOSEL_ASSERT(node != NULL);
    MOCOSEL_ASSERT(registry != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(node == NULL || registry == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    struct MOCOSEL_STATEMENT* statement = MOCOSEL_LOOKUP(&node->keyword, registry);
    /* MOCOSEL_ERROR_RUNTIME_UNDEFINED_STATEMENT. */
    if(statement == NULL) {
        return MOCOSEL_ERROR_RUNTIME_UNDEFINED_STATEMENT;
    }
    MOCOSEL_WORD_DOUBLE error = statement->second(node, registry, value);
    if (error != 0) {
        return error;
    }
    if(node->node) {
        error = MOCOSEL_WALK(node->node, registry, NULL);
        if(error != 0) {
            return error;
        }
    }
    return 0;
}
