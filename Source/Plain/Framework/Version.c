/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/09/2013,
 * Revision 11/09/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/VM.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_VERSION(struct MOCOSEL_MANIFEST* manifest) {
    static const char MOCOSEL_PATTERN[] = "!$%^&*-_+=/?<>0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static const char MOCOSEL_VERSION[] = "0.0.1";
    if(manifest != NULL) {
        manifest->pattern.from = (MOCOSEL_BYTE*)MOCOSEL_PATTERN;
        manifest->pattern.to = (MOCOSEL_BYTE*)MOCOSEL_PATTERN + sizeof(MOCOSEL_PATTERN);
        manifest->version.from = (MOCOSEL_BYTE*)MOCOSEL_VERSION;
        manifest->version.to = (MOCOSEL_BYTE*)MOCOSEL_VERSION + sizeof(MOCOSEL_VERSION);
    }
    return 1U;
}
