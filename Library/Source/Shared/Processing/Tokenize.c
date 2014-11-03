/*
 * Author   Nerijus Ramanauskas <nerijus.ramanauskas@mocosel.org>,
 * Date     02/23/2013,
 * Revision 11/04/2014,
 *
 * Copyright 2014 Nerijus Ramanauskas.
 */

#include <Plain/Mocosel.h>

MOCOSEL_WORD_DOUBLE MOCOSEL_EXPORT(MOCOSEL_BYTE* data, MOCOSEL_WORD_DOUBLE length, MOCOSEL_WORD_DOUBLE type, struct MOCOSEL_VALUE* value) {
    MOCOSEL_ASSERT(value != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(value == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    switch(type) {
        case MOCOSEL_TYPE_INTEGER:
        case MOCOSEL_TYPE_KEYWORD:
        case MOCOSEL_TYPE_LIST:
        case MOCOSEL_TYPE_POINTER:
        case MOCOSEL_TYPE_REAL:
        case MOCOSEL_TYPE_STRING:
            MOCOSEL_ASSERT(data != NULL);
            MOCOSEL_ASSERT(length > 0);
        case MOCOSEL_TYPE_BOOLEAN:
        case MOCOSEL_TYPE_NIL:
            break;
        
        default:
            return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    value->data = data;
    value->length = length;
    value->type = type;
    if(length > 0) {
        value->data = (MOCOSEL_BYTE*)MOCOSEL_RESIZE(NULL, length, 0);
        /* MOCOSEL_ERROR_SYSTEM. */
        if(value->data == NULL) {
            return MOCOSEL_ERROR_SYSTEM;
        }
        /* Keyword, string. */
        if(type == MOCOSEL_TYPE_KEYWORD || type == MOCOSEL_TYPE_STRING) {
            MOCOSEL_WORD_DOUBLE i = 0;
            MOCOSEL_WORD_DOUBLE j = 0;
            MOCOSEL_WORD_DOUBLE k = length;
            for(--k; i < k; i++) {
                if(data[i] == '\\') {
                    if(++i == k) {
                        break;
                    }
                }
                value->data[j++] = data[i];
            }
            value->data[j] = 0;
        /* MOCOSEL_ERROR_SYSTEM. */
        } else if(memcpy(value->data, data, length) == NULL) {
            return MOCOSEL_ERROR_SYSTEM;
        }
    }
    return 0;
}

MOCOSEL_INLINE MOCOSEL_WORD_DOUBLE MOCOSEL_JOIN(MOCOSEL_BYTE* data, MOCOSEL_WORD_DOUBLE length, struct MOCOSEL_LIST* node, MOCOSEL_WORD_DOUBLE type) {
    MOCOSEL_ASSERT(node != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(node == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    MOCOSEL_WORD_DOUBLE distance = node->layout.to - node->layout.from;
    MOCOSEL_WORD_DOUBLE error = MOCOSEL_RESERVE(sizeof(struct MOCOSEL_VALUE), &node->layout);
    if(error != 0) {
        return error;
    }
    return MOCOSEL_EXPORT(data, length, type, (struct MOCOSEL_VALUE*)&node->layout.from[distance]);
}

MOCOSEL_WORD_DOUBLE MOCOSEL_TOKENIZE(void* context, struct MOCOSEL_LIST* node, struct MOCOSEL_LIST* parent, const MOCOSEL_BYTE* pattern, struct MOCOSEL_SEGMENT* segment, MOCOSEL_DELEGATE tracker) {
    MOCOSEL_ASSERT(node != NULL);
    MOCOSEL_ASSERT(pattern != NULL);
    MOCOSEL_ASSERT(segment != NULL);
    /* MOCOSEL_ERROR_SYSTEM_WRONG_DATA. */
    if(node == NULL || pattern == NULL || segment == NULL) {
        return MOCOSEL_ERROR_SYSTEM_WRONG_DATA;
    }
    /* Range. */
    MOCOSEL_WORD_DOUBLE i = 0;
    MOCOSEL_WORD_DOUBLE j = segment->to - segment->from;
    /* Leading. */
    for(; i < j; i++) {
        /* Whitespace. */
        switch(segment->from[i]) {
            case '\t':
            case '\n':
            case '\r':
            case ' ':
                continue;
        }
        /* Comment. */
        if(segment->from[i] == '`') {
            for(i++; i < j; i++) {
                if(segment->from[i] == '\n' || segment->from[i] == '\r') {
                    break;
                }
            }
            i--;
        } else {
            break;
        }
    }
    /* Trailing. */
    for(; j > i; j--) {
        /* Whitespace. */
        switch(segment->from[j - 1]) {
            case '\t':
            case '\n':
            case '\r':
            case ' ':
                continue;
        }
        break;
    }
    /* Node. */
    node->keyword.from = NULL;
    node->keyword.to = NULL;
    node->layout.from = NULL;
    node->layout.to = NULL;
    node->node = NULL;
    node->parent = parent;
    node->segment.from = NULL;
    node->segment.to = NULL;
    /* Nil. */
    if(i == j) {
        return 0;
    }
    node->segment.from = &segment->from[i];
    node->segment.to = &segment->from[j];
    for(; i < j; i++) {
        if(strchr((const char*)pattern, (char)segment->from[i]) != NULL) {
            break;
        }
    }
    /* Keyword. */
    node->keyword.from = node->segment.from;
    node->keyword.to = &segment->from[i];
    /* MOCOSEL_ERROR_SYNTAX. */
    if(node->keyword.from == node->keyword.to) {
        if(tracker != NULL) {
           tracker(context, node->keyword.from, j - i, MOCOSEL_ERROR_SYNTAX_UNKNOWN_TOKEN);
        }
        return MOCOSEL_ERROR_SYNTAX;
    }
    MOCOSEL_WORD_DOUBLE k = 0;
    MOCOSEL_WORD_DOUBLE l = 0;
    MOCOSEL_WORD_DOUBLE m = 0;
    MOCOSEL_WORD_DOUBLE n = 0;
    MOCOSEL_WORD_DOUBLE o = 0;
    /* Data. */
    for(; i < j; i++) {
        /* Whitespace. */
        switch(segment->from[i]) {
            case '\t':
            case '\n':
            case '\r':
            case ' ':
            case ',':
                continue;
        }
        /* Comment. */
        if(segment->from[i] == '`') {
            for(i++; i < j; i++) {
                if(segment->from[i] == '\r' || segment->from[i] == '\n') {
                    break;
                }
            }
            continue;
        }
        /* String. */
        if(segment->from[i] == '"' || segment->from[i] == '\'') {
            k = segment->from[i];
            l = ++i;
            for(; l < j; l++) {
                /* Escape. */
                if(segment->from[l] == '\\') {
                    l++;
                /* Quotation mark. */
                } else if(segment->from[l] == k) {
                    break;
                }
            }
            /* MOCOSEL_ERROR_SYNTAX. */
            if(l == j) {
                if(tracker != NULL) {
                   tracker(context, &segment->from[i - 1], j - i + 1, MOCOSEL_ERROR_SYNTAX_MISSING_QUOTATION_MARK);
                }
                return MOCOSEL_ERROR_SYNTAX;
            }
            /* String. */
            MOCOSEL_WORD_DOUBLE error = MOCOSEL_JOIN(&segment->from[i], l - i + 1, node, MOCOSEL_TYPE_STRING);
            if(error != 0) {
                return error;
            }
            i = l;
        /* List. */
        } else if(segment->from[i] == '[' || segment->from[i] == '{') {
            k = segment->from[i] + 0;
            l = segment->from[i] + 2;
            m = ++i;
            n = 1;
            /* Arguments. */
            for(; m < j; m++) {
                /* String. */
                if(segment->from[m] == '"' || segment->from[m] == '\'') {
                    for(o = m + 1; o < j; o++) {
                        if(segment->from[o] == '\\') {
                            o++;
                        } else if(segment->from[o] == segment->from[m]) {
                            break;
                        }
                    }
                    /* MOCOSEL_ERROR_SYNTAX. */
                    if(o == j) {
                        if(tracker) {
                           tracker(context, &segment->from[i - 1], j - i + 1, MOCOSEL_ERROR_SYNTAX_MISSING_QUOTATION_MARK);
                        }
                        return MOCOSEL_ERROR_SYNTAX;
                    }
                    m = o;
                /* Bracket. */
                } else if(segment->from[m] == k) {
                    n++;
                /* Bracket. */
                } else if(segment->from[m] == l) {
                    n--;
                /* Comment. */
                } else if(segment->from[m] == '`') {
                    for(m++; m < j; m++) {
                        if(segment->from[m] == '\r' || segment->from[m] == '\n') {
                            break;
                        }
                    }
                }
                if(n == 0) {
                    break;
                }
            }
            struct MOCOSEL_SEGMENT subsegment = {&segment->from[i], &segment->from[m]};
            /* MOCOSEL_ERROR_SYNTAX. */
            if(n != 0) {
                if(tracker != NULL) {
                   tracker(context, &segment->from[i - 1], j - i + 1, MOCOSEL_ERROR_SYNTAX_MISSING_BRACKET);
                }
                return MOCOSEL_ERROR_SYNTAX;
            }
            /* Node. */
            struct MOCOSEL_LIST* child = (struct MOCOSEL_LIST*)MOCOSEL_RESIZE(NULL, sizeof(struct MOCOSEL_LIST), 0);
            /* MOCOSEL_ERROR_SYSTEM. */
            if(child == NULL) {
                return MOCOSEL_ERROR_SYSTEM;
            }
            MOCOSEL_WORD_DOUBLE error = 0;
            /* Node. */
            if(k == '[') {
                error = MOCOSEL_TOKENIZE(context, child, node, pattern, &subsegment, tracker);
            /* List. */
            } else {
                error = MOCOSEL_TOKENIZE(context, child, NULL, pattern, &subsegment, tracker);
            }
            /* Nil. */
            if(child->keyword.from == child->keyword.to || error != 0) {
                MOCOSEL_RESIZE(child, 0, sizeof(struct MOCOSEL_LIST));
                if(error != 0) {
                    return error;
                }
            }
            /* Nil. */
            if(child->keyword.from == child->keyword.to) {
                error = MOCOSEL_JOIN(NULL, 0, node, MOCOSEL_TYPE_NIL);
            /* List. */
            } else {
                error = MOCOSEL_JOIN((MOCOSEL_BYTE*)child, sizeof(struct MOCOSEL_LIST), node, MOCOSEL_TYPE_LIST);
            }
            if(error != 0) {
                return error;
            }
            i = m;
        /* Integer, real. */
        } else if(segment->from[i] == '+' || segment->from[i] == '-' || isdigit(segment->from[i])) {
            k = i++;
            l = 0;
            m = 0;
            n = 0;
            for(; i < j; i++) {
                /* Real. */
                if(segment->from[i] == '.') {
                    /* MOCOSEL_ERROR_SYNTAX. */
                    if(isdigit(segment->from[i - 1]) == 0) {
                        if(tracker != NULL) {
                           tracker(context, &segment->from[k - 1], i - k + 2, MOCOSEL_ERROR_SYNTAX_ERRONEOUS_EXPRESSION);
                        }
                        return MOCOSEL_ERROR_SYNTAX;
                    }
                    l++;
                /* Sign. */
                } else if(segment->from[i] == '-' || segment->from[i] == '+') {
                    switch(segment->from[i - 1]) {
                        case 'e':
                        case 'E':
                            break;

                        /* MOCOSEL_ERROR_SYNTAX. */
                        default: {
                            if(tracker != NULL) {
                               tracker(context, &segment->from[k], i - k + 1, MOCOSEL_ERROR_SYNTAX_ERRONEOUS_EXPRESSION);
                            }
                            return MOCOSEL_ERROR_SYNTAX;
                        }
                    }
                    m++;
                /* Scientific. */
                } else if(segment->from[i] == 'e' || segment->from[i] == 'E') {
                    /* MOCOSEL_ERROR_SYNTAX. */
                    if(isdigit(segment->from[i - 1]) == 0) {
                        if(tracker != NULL) {
                           tracker(context, &segment->from[k], i - k + 1, MOCOSEL_ERROR_SYNTAX_ERRONEOUS_EXPRESSION);
                        }
                        return MOCOSEL_ERROR_SYNTAX;
                    }
                    n++;
                /* Digit. */
                } else if(isdigit(segment->from[i]) == 0) {
                    break;
                }
            }
            /* Integer, real. */
            if(k != i - 1 || isdigit(segment->from[k])) {
               /* MOCOSEL_ERROR_SYNTAX. */
                if(l > 1 || m > 1 || n > 1) {
                    if(tracker != NULL) {
                       tracker(context, &segment->from[k], i - k, MOCOSEL_ERROR_SYNTAX_ERRONEOUS_EXPRESSION);
                    }
                    return MOCOSEL_ERROR_SYNTAX;
                }
                /* Real. */
                if(l > 0 || n > 0) {
                    double number = atof((const char*)&segment->from[k]);
                    /* MOCOSEL_ERROR_SYNTAX. */
                    if(number == HUGE_VAL) {
                        return MOCOSEL_ERROR_SYNTAX;
                    }
                    MOCOSEL_REAL real = (MOCOSEL_REAL)number;
                    /* MOCOSEL_ERROR_SYSTEM. */
                    if(MOCOSEL_JOIN((MOCOSEL_BYTE*)&real, sizeof(MOCOSEL_REAL), node, MOCOSEL_TYPE_REAL) != 0) {
                        return MOCOSEL_ERROR_SYSTEM;
                    }
                /* Integer. */
                } else {
                    MOCOSEL_WORD_DOUBLE integer = (MOCOSEL_WORD_DOUBLE)atoi((const char*)&segment->from[k]);
                    /* MOCOSEL_ERROR_SYSTEM. */
                    if(MOCOSEL_JOIN((MOCOSEL_BYTE*)&integer, sizeof(MOCOSEL_WORD_DOUBLE), node, MOCOSEL_TYPE_INTEGER) != 0) {
                        return MOCOSEL_ERROR_SYSTEM;
                    }
                }
            /* Keyword. */
            } else {
                for(m = i - 1; m < j; m++) {
                    if(strchr((const char*)pattern, (char)segment->from[m]) != NULL) {
                        break;
                    }
                }
                /* MOCOSEL_ERROR_SYNTAX. */
                if(m == k) {
                    if(tracker != NULL) {
                       tracker(context, &segment->from[k], m - k, MOCOSEL_ERROR_SYNTAX_UNKNOWN_TOKEN);
                    }
                    return MOCOSEL_ERROR_SYNTAX;
                }
                MOCOSEL_WORD_DOUBLE signature = MOCOSEL_JOIN(&segment->from[k], m - i++ + 2, node, MOCOSEL_TYPE_KEYWORD);
                if(signature != 0) {
                    return signature;
                }
            }
            i--;
        /* Node. */
        } else if(segment->from[i] == ';') {
            /* MOCOSEL_ERROR_SYNTAX. */
            if(node->parent != NULL) {
                if(tracker != NULL) {
                   tracker(context, &segment->from[i], j - i, MOCOSEL_ERROR_SYNTAX_ERRONEOUS_EXPRESSION);
                }
                return MOCOSEL_ERROR_SYNTAX;
            }
            struct MOCOSEL_SEGMENT subsegment = {&segment->from[++i], &segment->from[j]};
            if(subsegment.from == subsegment.to) {
                break;
            }
            /* Node. */
            struct MOCOSEL_LIST* next = (struct MOCOSEL_LIST*)MOCOSEL_RESIZE(NULL, sizeof(struct MOCOSEL_LIST), 0);
            /* MOCOSEL_ERROR_SYSTEM. */
            if(next == NULL) {
                return MOCOSEL_ERROR_SYSTEM;
            }
            MOCOSEL_WORD_DOUBLE error = MOCOSEL_TOKENIZE(context, next, NULL, pattern, &subsegment, tracker);
            if(next->keyword.from == next->keyword.to || error != 0) {
                MOCOSEL_RESIZE(next, 0, sizeof(struct MOCOSEL_LIST));
                if(error != 0) {
                    return error;
                }
            } else {
                node->node = next;
            }
            break;
        /* Keyword. */
        } else {
            MOCOSEL_WORD_DOUBLE identifier = 2166136261U;
            for(k = i; i < j; i++) {
                if(strchr((const char*)pattern, (char)segment->from[i]) != NULL) {
                    break;
                }
                identifier ^= segment->from[i];
                /* 2 ^ 24 + 2 ^ 8 + 0x93. */
                identifier *= 16777619U;
            }
            /* MOCOSEL_ERROR_SYNTAX. */
            if(i == k) {
                if(tracker != NULL) {
                   tracker(context, &segment->from[i], k - i, MOCOSEL_ERROR_SYNTAX_UNKNOWN_TOKEN);
                }
                return MOCOSEL_ERROR_SYNTAX;
            }
            --i;
            /* Boolean. */
            if(identifier == 1647734778U) {
                MOCOSEL_WORD_DOUBLE error = MOCOSEL_JOIN(NULL, 0, node, MOCOSEL_TYPE_BOOLEAN);
                if(error != 0) {
                    return error;
                }
            /* Nil. */
            } else if(identifier == 2913447899U) {
                MOCOSEL_WORD_DOUBLE error = MOCOSEL_JOIN(NULL, 0, node, MOCOSEL_TYPE_NIL);
                if(error != 0) {
                    return error;
                }
            /* Boolean. */
            } else if(identifier == 1319056784U) {
                MOCOSEL_WORD_DOUBLE error = MOCOSEL_JOIN((MOCOSEL_BYTE*)0xFF, 0, node, MOCOSEL_TYPE_BOOLEAN);
                if(error != 0) {
                    return error;
                }
            /* Keyword. */
            } else {
                MOCOSEL_WORD_DOUBLE error = MOCOSEL_JOIN(&segment->from[k], i - k + 2, node, MOCOSEL_TYPE_KEYWORD);
                if(error != 0) {
                    return error;
                }
            }
        }
    }
    return 0;
}

void MOCOSEL_UNLINK(struct MOCOSEL_LIST* node) {
    if(node == NULL) {
        return;
    }
    MOCOSEL_BYTE* from = node->layout.from;
    MOCOSEL_BYTE* to = node->layout.to;
    for(; from != to; from += sizeof(struct MOCOSEL_VALUE)) {
        struct MOCOSEL_VALUE* value = (struct MOCOSEL_VALUE*)from;
        if(value->type == MOCOSEL_TYPE_LIST) {
            MOCOSEL_UNLINK((struct MOCOSEL_LIST*)value->data);
        }
        if(value->length > 0) {
            MOCOSEL_RESIZE(value->data, 0, value->length);
        }
    }
    if(node->layout.from != NULL) {
        MOCOSEL_RESIZE(node->layout.from, 0, node->layout.to - node->layout.from);
    }
    MOCOSEL_UNLINK(node->node);
    if(node->node != NULL) {
        MOCOSEL_RESIZE(node->node, 0, sizeof(struct MOCOSEL_LIST));
    }
}
