/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/11/2013,
 * Revision 11/13/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include "../Sandbox.h"

void PLAIN_LOAD(const char* __restrict identifier, struct PLAIN_UNIT* __restrict unit) {
    MOCOSEL_ASSERT(identifier != NULL);
    MOCOSEL_ASSERT(unit != NULL);
    if(identifier == NULL || unit == NULL) {
        return;
    }
    FILE* file = fopen(identifier, "rb");
    if(file == NULL) {
        return (void)printf("System error: cannot access %s.\n", identifier);
    }
    if(fseek(file, 0, SEEK_END) != 0) {
        return (void)printf("System error: unsupported file system.\n");
    }
    long int length = ftell(file);
    if(length > 0) {
        unit->segment.from = (MOCOSEL_BYTE*)MOCOSEL_RESIZE(NULL, (MOCOSEL_WORD_DOUBLE)length, 0);
        unit->segment.to = unit->segment.from + length;
        if(unit->segment.from == NULL) {
            printf("System error: cannot allocate %ld bytes of memory.\n", length);
        } else {
            fseek(file, 0, SEEK_SET);
            if(fread(unit->segment.from, length, 1, file) != 1) {
                printf("System error: failed reading file %s.\n", identifier);
            }
        }
    }
    fclose(file);
}
