/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/12/2013,
 * Revision 11/13/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/Auxiliary.h>

struct MOCOSEL_VALUE* MOCOSEL_ARGUMENT(struct MOCOSEL_LIST* node, MOCOSEL_WORD_DOUBLE position) {
    MOCOSEL_ASSERT(node != NULL);
    if(node == NULL) {
        return NULL;
    }
    MOCOSEL_WORD_DOUBLE length = node->layout.to - node->layout.from;
    MOCOSEL_WORD_DOUBLE offset = sizeof(struct MOCOSEL_VALUE) * position;
    if(offset >= length) {
        return NULL;
    }
    return (struct MOCOSEL_VALUE*)&node->layout.from[offset];
}

MOCOSEL_WORD_DOUBLE MOCOSEL_EXPORT(MOCOSEL_BYTE* __restrict name, struct MOCOSEL_SEGMENT* __restrict registry, MOCOSEL_SUBROUTINE symbol) {
    MOCOSEL_ASSERT(name != NULL);
    MOCOSEL_ASSERT(registry != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(name == NULL || registry == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    struct MOCOSEL_STATEMENT statement = {{name, name + strlen((const char*)name)}, symbol};
    if(symbol == NULL) {
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_UNREGISTER(&statement.first, registry);
        if(error != 0) {
            return error;
        }
    } else {
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_REGISTER(registry, &statement);
        if(error != 0) {
            return error;
        }
    }
    return 0;
}

MOCOSEL_WORD_DOUBLE MOCOSEL_MEASURE(struct MOCOSEL_LIST* node) {
    MOCOSEL_ASSERT(node != NULL);
    if(node == NULL) {
        return 0;
    }
    return (node->layout.to - node->layout.from) / sizeof(struct MOCOSEL_VALUE);
}
