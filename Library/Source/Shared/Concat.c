 /*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/14/2013,
 * Revision 06/28/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_CC(struct MOCOSEL_SEGMENT* destination, MOCOSEL_WORD_DOUBLE length, const MOCOSEL_BYTE* source) {
    struct MOCOSEL_SEGMENT segment = {
        (MOCOSEL_BYTE*)source,
        (MOCOSEL_BYTE*)source + length
    };
    return MOCOSEL_CONCAT(destination, &segment);
}

MOCOSEL_WORD_DOUBLE MOCOSEL_CONCAT(struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT destination, const struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT source) {
    MOCOSEL_ASSERT(destination != NULL);
    MOCOSEL_ASSERT(source != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(destination == NULL || source == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    MOCOSEL_WORD_DOUBLE distance = destination->to - destination->from;
    MOCOSEL_WORD_DOUBLE length = source->to - source->from;
    if(length == 0) {
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
