/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/09/2013,
 * Revision 01/12/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include <Plain/VM.h>

void MOCOSEL_FINALIZE(struct MOCOSEL_OBJECT* object) {
    if(object == NULL) {
        return;
    }
    MOCOSEL_EMPTY(&object->registry.data);
    MOCOSEL_PURGE(&object->segment.structure);
    MOCOSEL_RESIZE(object->segment.data.from, 0, object->segment.data.to - object->segment.data.from);
}
