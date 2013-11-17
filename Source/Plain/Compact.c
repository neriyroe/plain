/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     11/17/2013,
 * Revision 11/17/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

#include <Plain/Auxiliary.h>
#include <Plain/Compact.h>

MOCOSEL_PROTOTYPE(COMPACT_AND) {
    return 0;
}

MOCOSEL_PROTOTYPE(COMPACT_ARRAY) {
    return 0;
}

MOCOSEL_PROTOTYPE(COMPACT_DIFFERENCE) {
    return 0;
}

MOCOSEL_PROTOTYPE(COMPACT_DO) {
    return 0;
}

MOCOSEL_PROTOTYPE(COMPACT_IF) {
    return 0;
}

MOCOSEL_PROTOTYPE(COMPACT_IS) {
    return 0;
}

MOCOSEL_PROTOTYPE(COMPACT_OR) {
    return 0;
}

MOCOSEL_PROTOTYPE(COMPACT_QUOTIENT) {
    return 0;
}

MOCOSEL_PROTOTYPE(COMPACT_PRODUCT) {
    return 0;
}

MOCOSEL_PROTOTYPE(COMPACT_REMAINDER) {
    return 0;
}

MOCOSEL_PROTOTYPE(COMPACT_RETURN) {
    return 0;
}

MOCOSEL_PROTOTYPE(COMPACT_SUM) {
    return 0;
}

MOCOSEL_WORD_DOUBLE COMPACT_EXPORT(struct MOCOSEL_SEGMENT* registry) {
    MOCOSEL_ASSERT(registry != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(registry == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    MOCOSEL_EXPORT("and", registry, &COMPACT_AND);
    MOCOSEL_EXPORT("array", registry, &COMPACT_ARRAY);
    MOCOSEL_EXPORT("difference", registry, &COMPACT_DIFFERENCE);
    MOCOSEL_EXPORT("do", registry, &COMPACT_DO);
    MOCOSEL_EXPORT("if", registry, &COMPACT_IF);
    MOCOSEL_EXPORT("is", registry, &COMPACT_IS);
    MOCOSEL_EXPORT("or", registry, &COMPACT_OR);
    MOCOSEL_EXPORT("quotient", registry, &COMPACT_QUOTIENT);
    MOCOSEL_EXPORT("product", registry, &COMPACT_PRODUCT);
    MOCOSEL_EXPORT("remainder", registry, &COMPACT_REMAINDER);;
    MOCOSEL_EXPORT("return", registry, &COMPACT_RETURN);
    MOCOSEL_EXPORT("sum", registry, &COMPACT_SUM);
    return 0;
}
