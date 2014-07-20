/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/09/2013,
 * Revision 07/20/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include <Plain/VM.h>

void MOCOSEL_FINALIZE(MOCOSEL_OBJECT* object) {
    if(object == NULL) {
        return;
    }
    MOCOSEL_UNLINK(&object->segment.structure);
    MOCOSEL_RESIZE(object->segment.data.from, 0, object->segment.data.to - object->segment.data.from);
}

MOCOSEL_WORD_DOUBLE MOCOSEL_RUN(MOCOSEL_CONTEXT* context, MOCOSEL_ENVIRONMENT* environment, MOCOSEL_WORD_DOUBLE flag, MOCOSEL_LOOKUP function, MOCOSEL_OBJECT* object, const MOCOSEL_BYTE* source) {
    MOCOSEL_ASSERT(environment != NULL);
    MOCOSEL_ASSERT(object != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(environment == NULL || object == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    /* MOCOSEL_SEGMENT_COMPILE. */
    if(flag & MOCOSEL_SEGMENT_COMPILE) {
        memset(object, 0, sizeof(MOCOSEL_OBJECT));
        if(source == NULL) {
            return 0;
        }
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_CONCAT(&object->segment.data, strlen((const char*)source), source);
        if(error != 0) {
            return error;
        }
        error = MOCOSEL_TOKENIZE(&object->segment.structure, NULL, (MOCOSEL_BYTE*)environment->meta.pattern, &object->segment.data);
        if(error != 0) {
            return error;
        }
    }
    /* MOCOSEL_SEGMENT_EXECUTE. */
    if(flag & MOCOSEL_SEGMENT_EXECUTE) {
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_WALK(context, function, &object->segment.structure, NULL);
        if(error != 0) {
            return error;
        }
    }
    return 0;
}
