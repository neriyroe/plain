/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     02/23/2013.
 * Revision 09/01/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 *
 * Type — runtime type tags stored in PLAIN_VALUE.type.
 */

enum {
    PLAIN_TYPE_CALLABLE,      /* User-defined or native callable (function/procedure). */
    PLAIN_TYPE_BOOLEAN,       /* yes (data != NULL) or no (data == NULL); length = 0. */
    PLAIN_TYPE_INTEGER,       /* Stored as PLAIN_WORD_QUADRUPLE (64-bit); length = sizeof(PLAIN_WORD_QUADRUPLE). */
    PLAIN_TYPE_INTERPOLATED,  /* Unresolved string interpolation — ephemeral, walker-internal. */
    PLAIN_TYPE_KEYWORD,       /* Unresolved identifier; data is a null-terminated name string. */
    PLAIN_TYPE_LIST,          /* Pointer to a PLAIN_LIST node tree (expression or block). */
    PLAIN_TYPE_NIL,           /* No value; data = NULL, length = 0. */
    PLAIN_TYPE_POINTER,       /* Raw host pointer — reserved for host extensions. */
    PLAIN_TYPE_REAL,          /* Stored as PLAIN_REAL (double); length = sizeof(PLAIN_REAL). */
    PLAIN_TYPE_STRING,        /* Null-terminated string; length includes the null terminator. */
    PLAIN_TYPE_OBJECT         /* Plain-native object (PLAIN_FRAME*) or C++ bridge object. */
};
