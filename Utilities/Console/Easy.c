/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/11/2013,
 * Revision 12/30/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include "Easy.h"

/* For use within the translation unit. */
MOCOSEL_PROTOTYPE(EASY_TYPE) {
    MOCOSEL_WORD_DOUBLE index = 0;
    MOCOSEL_WORD_DOUBLE length = MOCOSEL_MEASURE(node);
    for(; index < length; index++) {
        struct MOCOSEL_VALUE* value = MOCOSEL_ARGUMENT(node, index);
        /* MOCOSEL_TYPE_BOOLEAN. */
        if(value->type == MOCOSEL_TYPE_BOOLEAN) {
            if(value->data == NULL) {
                EASY_WRITE("<no>", (struct EASY_SESSION*)context);
            } else {
                EASY_WRITE("<yes>", (struct EASY_SESSION*)context);
            }
        /* MOCOSEL_TYPE_INTEGER. */
        } else if(value->type == MOCOSEL_TYPE_INTEGER) {
            EASY_WRITE("%d", (struct EASY_SESSION*)context, *(int*)value->data);
        /* MOCOSEL_TYPE_KEYWORD. */
        } else if(value->type == MOCOSEL_TYPE_KEYWORD) {
            EASY_WRITE("<%s>", (struct EASY_SESSION*)context, (const char*)value->data);
        /* MOCOSEL_TYPE_LIST. */
        } else if(value->type == MOCOSEL_TYPE_LIST) {
            EASY_WRITE("<List>", (struct EASY_SESSION*)context);
        /* MOCOSEL_TYPE_NIL. */
        } else if(value->type == MOCOSEL_TYPE_NIL) {
            EASY_WRITE("<none>", (struct EASY_SESSION*)context);
        /* MOCOSEL_TYPE_REAL. */
        } else if(value->type == MOCOSEL_TYPE_REAL) {
            EASY_WRITE("%f", (struct EASY_SESSION*)context, *(float*)value->data);
        /* MOCOSEL_TYPE_STRING. */
        } else if(value->type == MOCOSEL_TYPE_STRING) {
            EASY_WRITE("%s", (struct EASY_SESSION*)context, (const char*)value->data);
        }
    }
    EASY_WRITE("\n", (struct EASY_SESSION*)context);
    return 0;
}

int main(int count, const char* layout[]) {
    struct EASY_SESSION session;
    if(EASY_START(&session) != 0) {
        return 0;
    }
    if(count > 1) {
        MOCOSEL_WORD_DOUBLE error = EASY_LOAD(layout[1], &session.program, &session);
        if(error == 0) {
            error = MOCOSEL_EXPORT("type", &session.program.registry.data, &EASY_TYPE);
            if(error != 0) {
                EASY_WRITE("Warning: could not resolve `%s`.\n", &session, "out");
            }
            error = MOCOSEL_RUN(&session, MOCOSEL_SEGMENT_EXECUTE, &session.manifest, &session.program, NULL);
            /* MOCOSEL_ERROR_SYSTEM. */
            if(error & MOCOSEL_ERROR_SYSTEM) {
                EASY_WRITE("System error %d occured while executing %s.\n", &session, error, layout[1]);
            /* MOCOSEL_ERROR_RUNTIME. */
            } else if(error & MOCOSEL_ERROR_RUNTIME) {
                EASY_WRITE("Runtime error %d occured while executing %s.\n", &session, error, layout[1]);
            } else if(error != 0) {
                EASY_WRITE("Error %d occured while executing %s.\n", &session, error, layout[1]);
            }
        }
    } else {
        EASY_WRITE("%s", &session, "Usage: ./Easy <source>.\n");
    }
    EASY_STOP(&session);
    return 0;
}
