/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/07/2013,
 * Revision 10/23/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

void* MOCOSEL_RESIZE(void* data, MOCOSEL_WORD_DOUBLE destination, MOCOSEL_WORD_DOUBLE source) {
    if(destination == source) {
        return data;
    }
    if(destination < 1) {
        if(data) {
            free(data);
        }
        return NULL;
    }
    /* NWW: realloc doesn't always work for some reason. */
    void* buffer = malloc(destination);
    if(buffer == NULL) {
        return data;
    }
    if(source > destination) {
        source = destination;
    }
    if(source > 0) {
        memcpy(buffer, data, source);
    }
    if(data) {
        free(data);
    }
    return buffer;
}
