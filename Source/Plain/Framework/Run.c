/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/09/2013,
 * Revision 12/02/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/VM.h>
#include <stdio.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_RUN(void* MOCOSEL_RESTRICT context, MOCOSEL_WORD_DOUBLE flag, const struct MOCOSEL_MANIFEST* MOCOSEL_RESTRICT manifest, struct MOCOSEL_OBJECT* MOCOSEL_RESTRICT object, const struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT segment) {
    MOCOSEL_ASSERT(manifest != NULL);
    MOCOSEL_ASSERT(object != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(manifest == NULL || object == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    /* MOCOSEL_COMPILE. */
    if(flag & MOCOSEL_SEGMENT_COMPILE) {
        memset(object, 0, sizeof(struct MOCOSEL_OBJECT));
        if(segment == NULL) {
            return 0;
        }
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_CONCAT(&object->segment.data, segment);
        if(error != 0) {
            return error;
        }
        error = MOCOSEL_TOKENIZE(&object->segment.structure, NULL, manifest->pattern, &object->segment.data);
        if(error != 0) {
            return error;
        }
    }
    /* MOCOSEL_EXECUTE. */
    if(flag & MOCOSEL_SEGMENT_EXECUTE) {
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_WALK(context, &object->segment.structure, &object->registry.data, NULL);
        if(error != 0) {
            return error;
        }
    }
    return 0;
}
