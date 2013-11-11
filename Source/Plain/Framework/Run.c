/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/09/2013,
 * Revision 11/11/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/VM.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_RUN(MOCOSEL_WORD_DOUBLE flag, struct MOCOSEL_MANIFEST* __restrict manifest, struct MOCOSEL_OBJECT* __restrict object, struct MOCOSEL_SEGMENT* __restrict segment) {
    MOCOSEL_ASSERT(manifest != NULL);
    MOCOSEL_ASSERT(object != NULL);
    MOCOSEL_ASSERT(segment != NULL);
    if(object != NULL) {
        memset(object, 0, sizeof(struct MOCOSEL_OBJECT));
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    } else if(manifest == NULL || segment == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    if(flag & MOCOSEL_COMPILE) {
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_CONCAT(&object->segment.data, segment);
        if(error != 0) {
            return error;
        }
        error = MOCOSEL_TOKENIZE(&object->segment.structure, NULL, &manifest->pattern, segment);
        if(error != 0) {
            return error;
        }
    }
    if(flag & MOCOSEL_EXECUTE) {
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_WALK(&object->segment.structure, &object->registry.data, NULL);
        if(error != 0) {
            return error;
        }
    }
    return 0;
}
