/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/09/2013,
 * Revision 01/11/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

struct MOCOSEL_VALUE* MOCOSEL_IMPORT(const MOCOSEL_GLYPH* MOCOSEL_RESTRICT keyword, struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT registry) {
    MOCOSEL_ASSERT(keyword != NULL);
    MOCOSEL_ASSERT(registry != NULL);
    if(keyword == NULL || registry == NULL) {
        return NULL;
    }
    /* TO DO. */
    return NULL;
}
