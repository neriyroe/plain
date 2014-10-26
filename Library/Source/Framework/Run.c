/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/09/2013,
 * Revision 10/26/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include <Plain/VM.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_EVALUATE(MOCOSEL_CONTEXT* context, MOCOSEL_ENVIRONMENT* environment, MOCOSEL_SUBROUTINE function, const MOCOSEL_BYTE* source, MOCOSEL_DELEGATE tracker, MOCOSEL_VALUE* value) {
    MOCOSEL_ASSERT(environment != NULL);
    MOCOSEL_ASSERT(source != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(environment == NULL || source == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    MOCOSEL_OBJECT object = {(MOCOSEL_BYTE*)source, (MOCOSEL_BYTE*)source + strlen(source), NULL, NULL, NULL,  NULL, NULL, NULL, NULL, NULL};
    if(object.segment.data.from == object.segment.data.to) {
        return 0;
    }
    MOCOSEL_WORD_DOUBLE error = MOCOSEL_TOKENIZE(&object.segment.structure, NULL, (MOCOSEL_BYTE*)environment->meta.pattern, &object.segment.data, tracker);
    if(error != 0) {
       error = MOCOSEL_WALK(context, function, &object.segment.structure, value);
    }
    MOCOSEL_UNLINK(&object.segment.structure);
    return error;
}
