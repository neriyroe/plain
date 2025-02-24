/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     11/01/2014.
 * Revision 08/30/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

#include <stdio.h>
#include <Plain/VM.h>

/* Report syntax errors to the stream, passed by <context>. */
void report(void* context, const PLAIN_BYTE* data, PLAIN_WORD_DOUBLE length, PLAIN_WORD_DOUBLE type);

/* Resolve <data>, using <value> for optional substitution. */
PLAIN_WORD_DOUBLE resolve(void* context, void* data, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value);

/* Evaluate <source>, passing <context> to <report> and <resolve>. */
int evaluate(void* context, const char* source) {
    PLAIN_WORD_DOUBLE error = PLAIN_EVALUATE((PLAIN_ENVIRONMENT*)context, &resolve, (const PLAIN_BYTE*)source, &report, NULL);
    if(error == PLAIN_ERROR_SYNTAX) {
        printf("The evaluation failed due to a syntax error.\n");
    } else if(error != 0) {
        printf("The evaluation failed due to an unidentified error (0x%X).\n", error);
    }
    return 0;
}
