/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/11/2013,
 * Revision 11/11/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <stdio.h>
#include "plain.h"

int main(int count, const char* layout[]) {
    struct MOCOSEL_MANIFEST manifest;
    if(MOCOSEL_VERSION(&manifest) == 0) {
        printf("Warning: Plain might not function properly on this platform or operating system.\n");
    }
    struct MOCOSEL_OBJECT object;
    /* NWW: one per object. */
    struct MOCOSEL_SEGMENT segment;
    if(count > 1) {
        PLAIN_FETCH(layout[1], &segment);
        if(segment.from == NULL) {
            return 0;
        }
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_RUN(MOCOSEL_COMPILE, &manifest, &object, &segment);
        if(error != 0) {
            printf("Failed compiling %s: code %d.\n", layout[1], error);
        }
        MOCOSEL_FREE(segment.from);
        if(error == 0) {
            error = MOCOSEL_RUN(MOCOSEL_EXECUTE, &manifest, &object, NULL);
            if(error != 0) {
                printf("Failed executing %s: code %d.\n", layout[1], error);
            }
        }
        MOCOSEL_FINALIZE(&object);
    } else {
        printf("Usage: plain <source>.\n");
    }
    return 0;
}
