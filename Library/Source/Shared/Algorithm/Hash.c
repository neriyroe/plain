/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     2013-05-14.
 * Revision 2026-03-29.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Hash — see CL.h for the public contract of PLAIN_HASH.
 */

#include <Plain/Parser.h>

PLAIN_WORD_DOUBLE PLAIN_HASH(const PLAIN_BYTE* data, PLAIN_WORD_DOUBLE length) {
    PLAIN_WORD_DOUBLE identifier = 2166136261U;
    for(PLAIN_WORD_DOUBLE index = 0; index < length; index++) {
        identifier ^= data[index];
        identifier *= 16777619U;
    }
    return identifier;
}
