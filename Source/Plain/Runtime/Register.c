/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/09/2013,
 * Revision 11/15/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_REGISTER(struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT registry, struct MOCOSEL_STATEMENT* MOCOSEL_RESTRICT statement) {
    MOCOSEL_ASSERT(registry != NULL);
    MOCOSEL_ASSERT(statement != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(registry == NULL || statement == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    MOCOSEL_WORD_DOUBLE distance = registry->to - registry->from;
    MOCOSEL_WORD_DOUBLE error = MOCOSEL_RESERVE(sizeof(struct MOCOSEL_STATEMENT), registry);
    if(error != 0) {
        return error;
    }
    struct MOCOSEL_STATEMENT* destination = (struct MOCOSEL_STATEMENT*)&registry->from[distance];
    /* Keyword. */
    destination->first.from = NULL;
    destination->first.to = NULL;
    /* Subroutine. */
    destination->second = statement->second;
    /* Keyword. */
    return MOCOSEL_CONCAT(&destination->first, &statement->first);
}
