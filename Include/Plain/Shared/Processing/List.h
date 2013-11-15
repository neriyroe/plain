/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     02/23/2013,
 * Revision 11/15/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

struct MOCOSEL_LIST {
    struct MOCOSEL_SEGMENT  keyword;
    struct MOCOSEL_SEGMENT  layout;
    struct MOCOSEL_LIST*    node;
    struct MOCOSEL_LIST*    parent;
    struct MOCOSEL_SEGMENT  segment;
};
