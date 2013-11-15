 /*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/14/2013,
 * Revision 11/15/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_CONCAT(struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT destination, struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT source) {
    MOCOSEL_ASSERT(destination != NULL);
    MOCOSEL_ASSERT(source != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(destination == NULL || source == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    MOCOSEL_WORD_DOUBLE distance = destination->to - destination->from;
    MOCOSEL_WORD_DOUBLE length = source->to - source->from;
    if(length < 1) {
        return 0;
    }
    MOCOSEL_WORD_DOUBLE error = MOCOSEL_RESERVE(length, destination);
    if(error != 0) {
        return error;
    }
    /* MOCOSEL_ERROR_SYSTEM. */
    if(memcpy(destination->from + distance, source->from, length) == NULL) {
        return MOCOSEL_ERROR_SYSTEM;
    }
    return 0;
}
