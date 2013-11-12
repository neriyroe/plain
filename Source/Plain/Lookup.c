/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/09/2013,
 * Revision 11/12/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

struct MOCOSEL_STATEMENT* MOCOSEL_LOOKUP(struct MOCOSEL_SEGMENT* __restrict keyword, struct MOCOSEL_SEGMENT* __restrict registry) {
    MOCOSEL_ASSERT(keyword != NULL);
    MOCOSEL_ASSERT(registry != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(keyword == NULL || registry == NULL) {
        return NULL;
    }
    MOCOSEL_BYTE* from = registry->from;
    MOCOSEL_BYTE* to = registry->to;
    for(; from != to; from += sizeof(struct MOCOSEL_STATEMENT)) {
        struct MOCOSEL_STATEMENT* statement = (struct MOCOSEL_STATEMENT*)from;
        if(0 == strncmp((const char*)keyword->from, (const char*)statement->first.from, MOCOSEL_MAXIMUM(keyword->to - keyword->from, statement->first.to - statement->first.from))) {
            return statement;
        }
    }
    return NULL;
}
