/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     05/07/2013.
 * Revision 09/02/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

#include <Plain/Plain.h>

void* PLAIN_RESIZE(void* data, PLAIN_WORD_DOUBLE destination, PLAIN_WORD_DOUBLE source) {
    if(destination == source) {
        return data;
    }
    if(destination == 0) {
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
