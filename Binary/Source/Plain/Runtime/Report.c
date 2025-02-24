/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     11/01/2014.
 * Revision 08/30/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

#include <stdio.h>
#include <Plain/Plain.h>

/* Printing to the global stream. */
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
