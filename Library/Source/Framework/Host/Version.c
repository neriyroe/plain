/*
 * Author   Nerijus Ramanauskas <nr@mocosel.com>,
 * Date     11/09/2013,
 * Revision 12/27/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include <Plain/VM.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_VERSION(MOCOSEL_ENVIRONMENT* environment) {
    if(environment != NULL) {
        environment->meta.pattern = "`\"'[{]},; \t\r\n";
        environment->meta.version = "2014.4";
    }
    return MOCOSEL_API;
}
