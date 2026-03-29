/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     10/12/2013.
 * Revision 09/01/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 *
 * Environment — runtime metadata populated by PLAIN_VERSION.
 *
 * Contains the delimiter string used by the tokenizer to split tokens
 * and the library version string.  Must be the first member of
 * PLAIN_CONTEXT so that a PLAIN_CONTEXT* can be cast to PLAIN_ENVIRONMENT*.
 */

#pragma once

struct PLAIN_ENVIRONMENT {
    struct {
        const char* delimiters;  /* Characters that terminate a keyword token during parsing. */
        const char* version;     /* Library version string (e.g. "2015.3"). */
    } meta;
};
