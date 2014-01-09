/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/12/2013,
 * Revision 01/09/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/Auxiliary.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_EXPORT(const MOCOSEL_CHARACTER* MOCOSEL_RESTRICT name, struct MOCOSEL_SEGMENT* MOCOSEL_RESTRICT registry, MOCOSEL_SUBROUTINE symbol) {
    MOCOSEL_ASSERT(name != NULL);
    MOCOSEL_ASSERT(registry != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(name == NULL || registry == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    struct MOCOSEL_STATEMENT statement = {{(MOCOSEL_BYTE*)name, (MOCOSEL_BYTE*)name + strlen(name)}, {(void*)symbol, 0, MOCOSEL_TYPE_SUBROUTINE}};
    if(symbol == NULL) {
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_UNREGISTER(&statement.first, registry);
        if(error != 0) {
            return error;
        }
    } else {
        MOCOSEL_WORD_DOUBLE error = MOCOSEL_REGISTER(registry, &statement);
        if(error != 0) {
            return error;
        }
    }
    return 0;
}
