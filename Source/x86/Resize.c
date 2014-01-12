/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     05/07/2013,
 * Revision 01/12/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

void* MOCOSEL_RESIZE(void* data, MOCOSEL_WORD_DOUBLE destination, MOCOSEL_WORD_DOUBLE source) {
    if(destination == source) {
        return data;
    }
    if(destination < 1) {
        if(data != NULL) {
            free(data);
        }
        return NULL;
    }
    #if 0
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
    if(data != NULL) {
        free(data);
    }
    return buffer;
    #else
    return realloc(data, destination);
    #endif
}
