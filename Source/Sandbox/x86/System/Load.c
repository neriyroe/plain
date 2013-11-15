/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/11/2013,
 * Revision 11/15/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include "../Sandbox.h"

void PLAIN_LOAD(const char* __restrict identifier, struct MOCOSEL_OBJECT* __restrict object, struct PLAIN_SESSION* __restrict session) {
    MOCOSEL_ASSERT(identifier != NULL);
    MOCOSEL_ASSERT(object != NULL);
    MOCOSEL_ASSERT(session != NULL);
    if(identifier == NULL || object == NULL || session == NULL) {
        return;
    }
    FILE* file = fopen(identifier, "rb");
    if(file == NULL) {
        return PLAIN_TYPE("System error: cannot access %s.\n", session, identifier);
    }
    if(fseek(file, 0, SEEK_END) != 0) {
        return PLAIN_TYPE("%s\n", session, "System error: unsupported file system.");
    }
    long int length = ftell(file);
    if(length > 0) {
        object->segment.data.from = (MOCOSEL_BYTE*)MOCOSEL_RESIZE(NULL, (MOCOSEL_WORD_DOUBLE)length, 0);
        object->segment.data.to = object->segment.data.from + length;
        if(object->segment.data.from == NULL) {
            PLAIN_TYPE("System error: cannot allocate %ld bytes of memory.\n", session, length);
        } else {
            fseek(file, 0, SEEK_SET);
            if(fread(object->segment.data.from, length, 1, file) != 1) {
                PLAIN_TYPE("System error: failed reading file %s.\n", session, identifier);
            }
        }
    }
    fclose(file);
}
