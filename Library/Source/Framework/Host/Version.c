/*
 * Author   Nerijus Ramanauskas <nr@mocosel.com>,
 * Date     11/09/2013,
 * Revision 08/22/2015,
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

#include <Plain/VM.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_VERSION(MOCOSEL_ENVIRONMENT* environment) {
    if(environment != NULL) {
        environment->meta.pattern = "`\"'[{]},;: \t\r\n";
        environment->meta.version = "2015.3";
    }
    return MOCOSEL_API;
}
