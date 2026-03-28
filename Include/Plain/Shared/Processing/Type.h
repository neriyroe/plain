/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     02/23/2013.
 * Revision 09/01/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

enum {
    PLAIN_TYPE_CALLABLE,  /* User-defined or native callable value. */
    PLAIN_TYPE_BOOLEAN,
    PLAIN_TYPE_INTEGER,
    PLAIN_TYPE_INTERPOLATED, /* Unresolved string interpolation — ephemeral, internal use. */
    PLAIN_TYPE_KEYWORD,
    PLAIN_TYPE_LIST,
    PLAIN_TYPE_NIL,
    PLAIN_TYPE_POINTER,
    PLAIN_TYPE_REAL,
    PLAIN_TYPE_STRING,
    PLAIN_TYPE_OBJECT
};
