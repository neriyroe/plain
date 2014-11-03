/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/01/2014,
 * Revision 11/03/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include "../../Plain.h"

void report(void* context, const MOCOSEL_BYTE* data, MOCOSEL_WORD_DOUBLE length, MOCOSEL_WORD_DOUBLE type) {
    const char* description = "Unknown error";
    if(type == MOCOSEL_ERROR_SYNTAX_ERRONEOUS_EXPRESSION) {
        description = "Erroneous expression";
    } else if(type == MOCOSEL_ERROR_SYNTAX_MISSING_BRACKET) {
        description = "Missing bracket";
    } else if(type == MOCOSEL_ERROR_SYNTAX_MISSING_QUOTATION_MARK) {
        description = "Missing quatation mark";
    } else if(type == MOCOSEL_ERROR_SYNTAX_UNKNOWN_TOKEN) {
        description = "Unknown token";
    }
    printf("%s: %.*s.\n", description, length, (const char*)data);
}
