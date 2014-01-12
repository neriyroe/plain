/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     10/11/2013,
 * Revision 01/12/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

struct MOCOSEL_OBJECT {
    struct {
        struct MOCOSEL_SEGMENT  data;
        struct MOCOSEL_LIST     structure;
    } segment;
};
