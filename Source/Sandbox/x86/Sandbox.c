/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/11/2013,
 * Revision 11/13/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include "Sandbox.h"

int main(int count, const char* layout[]) {
    struct MOCOSEL_MANIFEST manifest;
    if(MOCOSEL_VERSION(&manifest) == 0) {
        printf("Warning: Plain might not function properly on this platform or operating system.\n");
    }
    struct PLAIN_UNIT unit;
    memset(&unit, 0, sizeof(struct PLAIN_UNIT));
    if(count > 1) {
        PLAIN_SYNTHESIZE(layout[1], &unit);
        if(unit.segment.from == NULL) {
            return 0;
        }
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_RUN(MOCOSEL_COMPILE, &manifest, &unit.object, &unit.segment);
        if(error != 0) {
            printf("Failed compiling %s: code %d.\n", layout[1], error);
        }
        MOCOSEL_FREE(unit.segment.from);
        if(error == 0) {
            error = MOCOSEL_RUN(MOCOSEL_EXECUTE, &manifest, &unit.object, NULL);
            if(error != 0) {
                printf("Failed executing %s: code %d.\n", layout[1], error);
            }
        }
        MOCOSEL_FINALIZE(&unit.object);
    } else {
        printf("Usage: plain <source>.\n");
    }
    return 0;
}
