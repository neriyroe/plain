/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/09/2013,
 * Revision 11/01/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include <Plain/VM.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_EVALUATE(void* context, MOCOSEL_ENVIRONMENT* environment, MOCOSEL_SUBROUTINE function, const MOCOSEL_BYTE* source, MOCOSEL_DELEGATE tracker, MOCOSEL_VALUE* value) {
    MOCOSEL_ASSERT(environment != NULL);
    MOCOSEL_ASSERT(source != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(environment == NULL || source == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    MOCOSEL_OBJECT object = {
        (MOCOSEL_BYTE*)source,
        (MOCOSEL_BYTE*)source + strlen((const char*)source),
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    };
    if(object.segment.data.from == object.segment.data.to) {
        return 0;
    }
    /* System failures and syntax errors, <tracker> shall be able to print a nice syntax report. */
    MOCOSEL_WORD_DOUBLE error = MOCOSEL_TOKENIZE(context,
                                                 &object.segment.structure,
                                                 NULL,
                                                 (const MOCOSEL_BYTE*)environment->meta.pattern,
                                                 &object.segment.data,
                                                 tracker);
    /* Only system failures, no runtime dependencies. */
    if(error != 0) {
       error = MOCOSEL_WALK(context, function, &object.segment.structure, value);
    }
    /* The object might be dummy. */
    if(object.segment.structure.layout.from != NULL ||
       object.segment.structure.node != NULL) {
        MOCOSEL_UNLINK(&object.segment.structure);
    }
    return error;
}
