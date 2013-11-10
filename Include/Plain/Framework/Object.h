/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     10/11/2013,
 * Revision 10/16/2013,
 *
 * Copyright 2013 Nerijus Ramanauskas.
 */

struct MOCOSEL_OBJECT {
    struct {
        struct MOCOSEL_SEGMENT  data;
    } registry;

    struct {
        struct MOCOSEL_SEGMENT  data;
        struct MOCOSEL_LIST     structure;
    } segment;
};
