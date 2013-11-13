 /*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/14/2013,
 * Revision 11/13/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_CONCAT(struct MOCOSEL_SEGMENT* __restrict destination, struct MOCOSEL_SEGMENT* __restrict source) {
    MOCOSEL_ASSERT(destination != NULL);
    MOCOSEL_ASSERT(source != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(destination == NULL || source == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    MOCOSEL_WORD_DOUBLE distance = destination->to - destination->from;
    MOCOSEL_WORD_DOUBLE length = source->to - source->from;
    MOCOSEL_WORD_DOUBLE number = distance + length;
    if(length < 1) {
        return 0;
    }
    MOCOSEL_BYTE* buffer = (MOCOSEL_BYTE*)MOCOSEL_RESIZE(destination->from, number, distance);
    /* MOCOSEL_ERROR_SYSTEM. */
    if(buffer == NULL) {
        return MOCOSEL_ERROR_SYSTEM;
    }
    destination->from = buffer;
    destination->to = buffer + number;
    if(length > 0) {
        memcpy(destination->from + distance, source->from, length);
    }
    return 0;
}
