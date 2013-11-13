/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/14/2013,
 * Revision 11/13/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_HASH(struct MOCOSEL_SEGMENT* segment) {
    MOCOSEL_WORD_DOUBLE hash = 2166136261;
    if(segment == NULL || segment->from == segment->to) {
        return hash;
    }
    MOCOSEL_BYTE* __restrict from = segment->from;
    MOCOSEL_BYTE* __restrict to = segment->to;
    for(; from != to; from++) {
        hash ^= *from;
        /* 2 ^ 24 + 2 ^ 8 + 0x93. */
        hash *= 16777619;
    } 
    return hash;
}
