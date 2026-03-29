/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     2013-10-12.
 * Revision 2026-03-29.
 *
 * Copyright 2026 Nerijus Ramanauskas.
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
