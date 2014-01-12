/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/09/2013,
 * Revision 01/12/2013,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

void MOCOSEL_EMPTY(struct MOCOSEL_SEGMENT* registry) {
    MOCOSEL_ASSERT(registry != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(registry == NULL) {
        return;
    }
    MOCOSEL_BYTE* from = registry->from;
    MOCOSEL_BYTE* to = registry->to;
    for(; from != to; from += sizeof(struct MOCOSEL_STATEMENT)) {
        struct MOCOSEL_STATEMENT* statement = (struct MOCOSEL_STATEMENT*)from;
        if(statement->first.from != NULL) {
            MOCOSEL_RESIZE(statement->first.from, 0, statement->first.to - statement->first.from);
        }
    }
    MOCOSEL_RESIZE(registry->from, 0, registry->to - registry->from);
}
