 /*
 * Author   Neriy Roe <nr@mocosel.com>.
 * Date     05/14/2013.
 * Revision 09/02/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_HASH(const MOCOSEL_BYTE* data, MOCOSEL_WORD_DOUBLE length) {
    MOCOSEL_WORD_DOUBLE identifier = 2166136261U;
    MOCOSEL_WORD_DOUBLE index = 0;
    for(; index < length; index++) {
        identifier ^= data[index];
        /* 2 ^ 24 + 2 ^ 8 + 0x93. */
        identifier *= 16777619U;
    } 
    return identifier;
}
