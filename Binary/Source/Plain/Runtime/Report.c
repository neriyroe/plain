/*
 * Author   Nerijus Ramanauskas <nr@mocosel.com>,
 * Date     11/01/2014,
 * Revision 12/10/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include <stdio.h>
#include <Plain/Mocosel.h>

/* Printing to the global stream. */
void report(void* context, const MOCOSEL_BYTE* data, MOCOSEL_WORD_DOUBLE length, MOCOSEL_WORD_DOUBLE type) {
    const char* description = "Unknown error";
    if(type == MOCOSEL_ERROR_SYNTAX_ERRONEOUS_EXPRESSION) {
        description = "Erroneous expression";
    } else if(type == MOCOSEL_ERROR_SYNTAX_MISSING_BRACKET) {
        description = "Missing bracket";
    } else if(type == MOCOSEL_ERROR_SYNTAX_MISSING_QUOTATION_MARK) {
        description = "Missing quotation mark";
    } else if(type == MOCOSEL_ERROR_SYNTAX_UNKNOWN_TOKEN) {
        description = "Unknown token";
    }
    printf("%s: %.*s.\n", description, length, (const char*)data);
}
