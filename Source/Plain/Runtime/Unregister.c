/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/09/2013,
 * Revision 11/15/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_UNREGISTER(struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT keyword, struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT registry) {
    MOCOSEL_ASSERT(registry != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(registry == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    if(keyword == NULL) {
        MOCOSEL_BYTE* from = registry->from;
        MOCOSEL_BYTE* to = registry->to;
        for(; from != to; from += sizeof(struct MOCOSEL_STATEMENT)) {
            struct MOCOSEL_STATEMENT* statement = (struct MOCOSEL_STATEMENT*)from;
            if(statement->first.from != NULL) {
                MOCOSEL_FREE(statement->first.from);
            }
        }
        MOCOSEL_FREE(registry->from);
    } else {
        MOCOSEL_BYTE* from = registry->from;
        MOCOSEL_BYTE* to = registry->to;
        for(; from != to; from += sizeof(struct MOCOSEL_STATEMENT)) {
            struct MOCOSEL_STATEMENT* statement = (struct MOCOSEL_STATEMENT*)from;
            if(0 == strncmp((const char*)keyword->from, (const char*)statement->first.from, MOCOSEL_MAXIMUM(keyword->to - keyword->from, statement->first.to - statement->first.from))) {
                MOCOSEL_FREE(statement->first.from);
                break;
            }
        }
        /* MOCOSEL_ERROR_RUNTIME_WRONG_DATA. */
        if(from == to) {
            return MOCOSEL_ERROR_RUNTIME_WRONG_DATA;
        }
        MOCOSEL_WORD_DOUBLE number = registry->to - registry->from - sizeof(struct MOCOSEL_STATEMENT);
        MOCOSEL_WORD_DOUBLE remainder = to - from - sizeof(struct MOCOSEL_STATEMENT);
        if(remainder > 0) {
            memmove(from, from + sizeof(struct MOCOSEL_STATEMENT), remainder);
        }
        MOCOSEL_BYTE* buffer = (MOCOSEL_BYTE*)MOCOSEL_RESIZE(registry->from, number, number);
        if(number > 0) {
            /* MOCOSEL_ERROR_SYSTEM. */
            if(registry->from == NULL) {
                return MOCOSEL_ERROR_SYSTEM;
            }
        }
        registry->from = buffer;
        registry->to = buffer + number;
    }
    return 0;
}
