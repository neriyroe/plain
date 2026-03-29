/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     02/23/2013.
 * Revision 09/02/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 *
 * Segment — a contiguous byte range defined by a [from, to) pointer pair.
 *
 * Used throughout the runtime as a growable buffer (for string building
 * and argument arrays) and as a non-owning view into source text (for
 * keyword and segment fields in PLAIN_LIST).
 */

struct PLAIN_SEGMENT {
    PLAIN_BYTE* from;   /* First byte (inclusive). */
    PLAIN_BYTE* to;     /* One past the last byte (exclusive). */
};
