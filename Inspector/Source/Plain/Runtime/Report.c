/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     11/01/2014.
 * Revision 08/30/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 *
 * Report — error reporting for the Inspector REPL.
 *
 * Translates numeric error codes from the tokenizer/runtime into
 * human-readable messages and prints them to stdout.
 */

#include <stdio.h>
#include <Plain/Parser.h>

/* Called by PLAIN_EVALUATE via the tracker delegate when a syntax or
 * runtime error is encountered.  Prints the error description followed
 * by the offending source fragment. */
void report(void* context, const PLAIN_BYTE* data, PLAIN_WORD_DOUBLE length, PLAIN_WORD_DOUBLE type) {
    const char* description = "Unknown error";

    if(type == PLAIN_ERROR_SYNTAX_ERRONEOUS_EXPRESSION) {
        description = "Erroneous expression";
    } else if(type == PLAIN_ERROR_SYNTAX_MISSING_BRACKET) {
        description = "Missing bracket";
    } else if(type == PLAIN_ERROR_SYNTAX_MISSING_QUOTATION_MARK) {
        description = "Missing quotation mark";
    } else if(type == PLAIN_ERROR_SYNTAX_UNKNOWN_TOKEN) {
        description = "Unknown token";
    }

    printf("%s: %.*s.\n", description, length, (const char*)data);
}
