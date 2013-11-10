/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/09/2013,
 * Revision 10/16/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_UNREGISTER(struct MOCOSEL_SEGMENT* __restrict keyword, struct MOCOSEL_SEGMENT* __restrict registry) {
    MOCOSEL_ASSERT(registry != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(registry == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    if(keyword == NULL) {
        MOCOSEL_FREE(registry->from);
    } else {
        MOCOSEL_BYTE* from = registry->from;
        MOCOSEL_BYTE* to = registry->to;
        while(from != to) {
            struct MOCOSEL_STATEMENT* statement = (struct MOCOSEL_STATEMENT*)from;
            if(0 == strncmp((const char*)keyword->from, (const char*)statement->first.from, MOCOSEL_MAXIMUM(keyword->to - keyword->from, statement->first.to - statement->first.from))) {
                break;
            }
            from += statement->first.to - statement->first.from;
            from += sizeof(struct MOCOSEL_STATEMENT);
        }
        struct MOCOSEL_STATEMENT* statement = (struct MOCOSEL_STATEMENT*)from;
        /* MOCOSEL_ERROR_RUNTIME_WRONG_DATA. */
        if(from == to) {
            return MOCOSEL_ERROR_RUNTIME_WRONG_DATA;
        }
        MOCOSEL_WORD_DOUBLE length = statement->first.to - statement->first.from;
        MOCOSEL_WORD_DOUBLE number = length + sizeof(struct MOCOSEL_STATEMENT);
        MOCOSEL_WORD_DOUBLE size = registry->to - registry->from - number;
        if(size < 1) {
            return MOCOSEL_UNREGISTER(NULL, registry);
        }
        memmove(from, from + number, size);
        /* MOCOSEL_ERROR_SYSTEM. */
        if(MOCOSEL_RESIZE(registry->from, size, registry->to - registry->from) == NULL) {
            return MOCOSEL_ERROR_SYSTEM;
        } 
    }
    return 0;
}
