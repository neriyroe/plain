/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/09/2013,
 * Revision 11/12/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_REGISTER(struct MOCOSEL_SEGMENT* __restrict registry, struct MOCOSEL_STATEMENT* __restrict statement) {
    MOCOSEL_ASSERT(registry != NULL);
    MOCOSEL_ASSERT(statement != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(registry == NULL || statement == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    MOCOSEL_WORD_DOUBLE distance = registry->to - registry->from;
    MOCOSEL_WORD_DOUBLE number = distance + sizeof(struct MOCOSEL_STATEMENT);
    registry->from = (MOCOSEL_BYTE*)MOCOSEL_RESIZE(registry->from, number, distance);
    registry->to = registry->from + number;
    /* MOCOSEL_ERROR_SYSTEM. */
    if(registry->from == NULL) {
        return MOCOSEL_ERROR_SYSTEM;
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
