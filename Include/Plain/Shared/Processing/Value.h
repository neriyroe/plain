/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     02/23/2013.
 * Revision 03/28/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 */

struct PLAIN_VALUE {
    PLAIN_BYTE* data;
    PLAIN_WORD_DOUBLE length;
    PLAIN_WORD_DOUBLE type;
    PLAIN_WORD_DOUBLE owner;
};

/* Frees data owned by <value> and resets it to nil. */
void PLAIN_VALUE_CLEAR(struct PLAIN_VALUE* value);

/* Deep-copies <source> into <destination>. */
PLAIN_WORD_DOUBLE PLAIN_VALUE_COPY(struct PLAIN_VALUE* destination, const struct PLAIN_VALUE* source);

/* Returns nonzero if <value> represents a logically true state:
 * yes, non-zero integer, non-zero real, or non-empty string. */
PLAIN_WORD_DOUBLE PLAIN_VALUE_TRUTHY(const struct PLAIN_VALUE* value);
