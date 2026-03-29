/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     2013-05-07.
 * Revision 2026-03-29.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Resize — see CL.h for the public contract.
 *
 * Contains a #if 0 debug path that uses malloc + memcpy + free
 * instead of realloc, useful for catching use-after-realloc bugs.
 */

#include <Plain/Parser.h>

void* PLAIN_RESIZE(void* data, PLAIN_WORD_DOUBLE destination, PLAIN_WORD_DOUBLE source) {
    /* No-op when sizes match. */
    if(destination == source) {
        return data;
    }

    /* Free when destination size is zero. */
    if(destination == 0) {
        if(data != NULL) {
            free(data);
        }
        return NULL;
    }

    /* Allocate or reallocate. */
    #if 0
    /* Debug-friendly path: malloc + memcpy + free (catches use-after-realloc). */
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
