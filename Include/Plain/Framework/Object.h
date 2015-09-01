/*
 * Author   Neriy Roe <nr@mocosel.com>.
 * Date     10/11/2013.
 * Revision 09/01/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

struct MOCOSEL_OBJECT {
    struct {
        struct MOCOSEL_SEGMENT data;
        struct MOCOSEL_LIST structure;
    } segment;
};
