/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     05/14/2013.
 * Revision 09/02/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

#include <Plain/Plain.h>

PLAIN_WORD_DOUBLE PLAIN_CONCAT(struct PLAIN_SEGMENT* destination, PLAIN_WORD_DOUBLE length, const PLAIN_BYTE* source) {
    PLAIN_ASSERT(destination != NULL);
    PLAIN_ASSERT(source != NULL);
    /* PLAIN_ERROR_SYSTEM_WRONG_DATA. */
    if(destination == NULL || source == NULL) {
        return PLAIN_ERROR_SYSTEM_WRONG_DATA;
    }
    if(length == 0) {
        return 0;
    }
    PLAIN_WORD_DOUBLE distance = destination->to - destination->from;
    PLAIN_WORD_DOUBLE error = PLAIN_RESERVE(length, destination);
    if(error != 0) {
        return error;
    }
    /* PLAIN_ERROR_SYSTEM. */
    if(memcpy(destination->from + distance, source, length) == NULL) {
        return PLAIN_ERROR_SYSTEM;
    }
    return 0;
}

PLAIN_WORD_DOUBLE PLAIN_RESERVE(PLAIN_WORD_DOUBLE number, struct PLAIN_SEGMENT* segment) {
    PLAIN_ASSERT(segment != NULL);
    /* PLAIN_ERROR_SYSTEM_WRONG_DATA. */
    if(segment == NULL) {
        return PLAIN_ERROR_SYSTEM_WRONG_DATA;
    }
    if(number == 0) {
        return 0;
    }
    PLAIN_WORD_DOUBLE length = segment->to - segment->from + number;
    PLAIN_BYTE* pointer = (PLAIN_BYTE*)PLAIN_RESIZE(segment->from, length, length - number);
    /* PLAIN_ERROR_SYSTEM. */
    if(pointer == NULL) {
        return PLAIN_ERROR_SYSTEM;
    }
    segment->from = pointer;
    segment->to = pointer + length;
    return 0;
}
