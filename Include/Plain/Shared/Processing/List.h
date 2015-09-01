/*
 * Author   Neriy Roe <nr@mocosel.com>.
 * Date     02/23/2013.
 * Revision 09/01/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

struct MOCOSEL_LIST {
    struct MOCOSEL_SEGMENT keyword;
    struct MOCOSEL_SEGMENT layout;
    struct MOCOSEL_LIST* node;
    struct MOCOSEL_LIST* parent;
    struct MOCOSEL_SEGMENT segment;
};
