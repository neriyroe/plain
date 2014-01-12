/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/09/2013,
 * Revision 01/11/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

void MOCOSEL_EMPTY(struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT registry) {
    MOCOSEL_ASSERT(registry != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(registry == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    MOCOSEL_BYTE* from = registry->from;
    MOCOSEL_BYTE* to = registry->to;
    /* TO DO. */
    MOCOSEL_RESIZE(registry->from, 0, registry->to - registry->from);
}
