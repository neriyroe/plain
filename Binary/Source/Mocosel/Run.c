/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/01/2014,
 * Revision 11/01/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include "../Plain.h"

void run(void* context, const char* source) {
    MOCOSEL_WORD_DOUBLE error = MOCOSEL_EVALUATE(context, (MOCOSEL_ENVIRONMENT*)context, &resolve, (const MOCOSEL_BYTE*)source, &report, NULL);
    if(error == MOCOSEL_ERROR_SYNTAX) {
        printf("The evaluation failed due to a syntax error.\n", error);
    } else {
        printf("The evaluation failed due to an unidentified error (0x%X).\n", error);
    }
}
