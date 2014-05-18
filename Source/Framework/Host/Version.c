/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/09/2013,
 * Revision 05/15/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include <Plain/VM.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_VERSION(struct MOCOSEL_MANIFEST* manifest) {
    if(manifest != NULL) {
        manifest->pattern = "`\"'[{]},; \t\r\n";
        manifest->version = "2014.2";
    }
    return 20142;
}
