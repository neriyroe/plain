 /*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     05/14/2013.
 * Revision 09/02/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

#include <Plain/Plain.h>

PLAIN_WORD_DOUBLE PLAIN_HASH(const PLAIN_BYTE* data, PLAIN_WORD_DOUBLE length) {
    PLAIN_WORD_DOUBLE identifier = 2166136261U;
    PLAIN_WORD_DOUBLE index = 0;
    for(; index < length; index++) {
        identifier ^= data[index];
        /* 2 ^ 24 + 2 ^ 8 + 0x93. */
        identifier *= 16777619U;
    } 
    return identifier;
}
