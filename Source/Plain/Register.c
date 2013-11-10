/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/09/2013,
 * Revision 10/16/2013,
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
    MOCOSEL_WORD_DOUBLE length = statement->first.to - statement->first.from;
    MOCOSEL_WORD_DOUBLE number = distance + length + sizeof(struct MOCOSEL_STATEMENT);
    registry->from = (MOCOSEL_BYTE*)MOCOSEL_RESIZE(registry->from, number, distance);
    registry->to = registry->from + number;
    /* MOCOSEL_ERROR_SYSTEM. */
    if(registry->from == NULL) {
        return MOCOSEL_ERROR_SYSTEM;
    }
    MOCOSEL_BYTE* from = registry->from + distance + sizeof(struct MOCOSEL_STATEMENT);
    MOCOSEL_BYTE* to = from + length;
    memcpy(registry->from + distance, statement, sizeof(struct MOCOSEL_STATEMENT));
    memcpy(registry->from + distance + sizeof(struct MOCOSEL_STATEMENT), statement->first.from, length);
    memcpy(registry->from + distance, &from, sizeof(MOCOSEL_BYTE*));
    memcpy(registry->from + distance + sizeof(MOCOSEL_BYTE*), &to, sizeof(MOCOSEL_BYTE*));
    return 0;
}
