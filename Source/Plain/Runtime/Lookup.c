/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/09/2013,
 * Revision 12/02/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

struct MOCOSEL_STATEMENT* MOCOSEL_LOOKUP(struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT keyword, struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT registry) {
    MOCOSEL_ASSERT(keyword != NULL);
    MOCOSEL_ASSERT(registry != NULL);
    if(keyword == NULL || registry == NULL) {
        return NULL;
    }
    MOCOSEL_BYTE* from = registry->from;
    MOCOSEL_BYTE* to = registry->to;
    for(; from != to; from += sizeof(struct MOCOSEL_STATEMENT)) {
        struct MOCOSEL_STATEMENT* statement = (struct MOCOSEL_STATEMENT*)from;
        if(keyword->to - keyword->from != statement->first.to - statement->first.from) {
            continue;
        }
        if(0 == strncmp((const char*)keyword->from, (const char*)statement->first.from, keyword->to - keyword->from)) {
            return statement;
        }
    }
    return NULL;
}
