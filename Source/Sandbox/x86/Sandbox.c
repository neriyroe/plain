/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/11/2013,
 * Revision 11/13/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include "Sandbox.h"

static struct MOCOSEL_MANIFEST manifest;
static struct PLAIN_UNIT program;

int main(int count, const char* layout[]) {
    if(MOCOSEL_VERSION(&manifest) == 0) {
        printf("Warning: Plain might not function properly on this platform or operating system.\n");
    }
    memset(&program, 0, sizeof(struct PLAIN_UNIT));
    if(count > 1) {
        PLAIN_SYNTHESIZE(layout[1], &program);
        if(program.segment.from == NULL) {
            return 0;
        }
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_RUN(MOCOSEL_COMPILE, &manifest, &program.object, &program.segment);
        if(error != 0) {
            printf("Failed compiling %s: code %d.\n", layout[1], error);
        }
        MOCOSEL_FREE(program.segment.from);
        if(error == 0) {
            error = MOCOSEL_RUN(MOCOSEL_EXECUTE, &manifest, &program.object, NULL);
            if(error != 0) {
                printf("Failed executing %s: code %d.\n", layout[1], error);
            }
        }
        MOCOSEL_FINALIZE(&program.object);
    } else {
        printf("Usage: plain <source>.\n");
    }
    return 0;
}
