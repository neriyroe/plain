 /*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/15/2013,
 * Revision 01/12/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_RESERVE(MOCOSEL_WORD_DOUBLE number, struct MOCOSEL_SEGMENT* segment) {
    MOCOSEL_ASSERT(segment != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(segment == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    if(number == 0) {
        return 0;
    }
    MOCOSEL_WORD_DOUBLE length = segment->to - segment->from + number;
    MOCOSEL_BYTE* pointer = (MOCOSEL_BYTE*)MOCOSEL_RESIZE(segment->from, length, length - number);
    /* MOCOSEL_ERROR_SYSTEM. */
    if(pointer == NULL) {
        return MOCOSEL_ERROR_SYSTEM;
    }
    segment->from = pointer;
    segment->to = pointer + length;
    return 0;
}
