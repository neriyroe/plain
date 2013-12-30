/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/09/2013,
 * Revision 12/22/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/VM.h>

void MOCOSEL_FINALIZE(struct MOCOSEL_OBJECT* object) {
    if(object == NULL) {
        return;
    }
    MOCOSEL_UNREGISTER(NULL, &object->registry.data);
    MOCOSEL_PURGE(&object->segment.structure);
    MOCOSEL_RESIZE(object->segment.data.from, 0, object->segment.data.to - object->segment.data.from);
}
