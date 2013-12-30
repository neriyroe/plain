/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/09/2013,
 * Revision 12/02/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/VM.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_VERSION(struct MOCOSEL_MANIFEST* manifest) {
    static const char PATTERN[] = "!$%^&*-_+=/|?<>0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static const char VERSION[] = "0.0.1";
    if(manifest != NULL) {
        manifest->pattern = (const MOCOSEL_BYTE*)PATTERN;
        manifest->version = (const MOCOSEL_BYTE*)VERSION;
    }
    return 1;
}
