/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/09/2013,
 * Revision 11/13/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/VM.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_VERSION(struct MOCOSEL_MANIFEST* manifest) {
    static const char PATTERN[] = "!$%^&*-_+=/|?<>0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static const char VERSION[] = "0.0.1";
    if(manifest != NULL) {
        manifest->pattern.from = (MOCOSEL_BYTE*)PATTERN;
        manifest->pattern.to = (MOCOSEL_BYTE*)PATTERN + sizeof(PATTERN);
        manifest->version.from = (MOCOSEL_BYTE*)VERSION;
        manifest->version.to = (MOCOSEL_BYTE*)VERSION + sizeof(VERSION);
    }
    #if MOCOSEL_TARGET == MOCOSEL_TARGET_UNKNOWN
    return 0;
    #else
    return 1;
    #endif
}
