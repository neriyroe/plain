/*
 * Author   Nerijus Ramanauskas <nr@mocosel.com>,
 * Date     02/23/2013,
 * Revision 11/14/2014,
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
