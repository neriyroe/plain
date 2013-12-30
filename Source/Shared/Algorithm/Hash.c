 /*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/14/2013,
 * Revision 12/02/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_HASH(const MOCOSEL_BYTE* data, MOCOSEL_WORD_DOUBLE length) {
    MOCOSEL_WORD_DOUBLE identifier = 2166136261;
    MOCOSEL_WORD_DOUBLE index = 0;
    for(; index < length; index++) {
        identifier ^= data[index];
        /* 2 ^ 24 + 2 ^ 8 + 0x93. */
        identifier *= 16777619;
    } 
    return identifier;
}
