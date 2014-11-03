/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/01/2014,
 * Revision 11/03/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include "../../Plain.h"

void report(void* context, const MOCOSEL_BYTE* data, MOCOSEL_WORD_DOUBLE length, MOCOSEL_WORD_DOUBLE type) {
    printf("%.*s\n", length, (const char*)data);
}
