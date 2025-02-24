/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     02/23/2013.
 * Revision 09/02/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 */

#include <Plain/Plain.h>

PLAIN_WORD_DOUBLE PLAIN_EXPORT(PLAIN_BYTE* data, PLAIN_WORD_DOUBLE length, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    PLAIN_ASSERT(value != NULL);
    /* PLAIN_ERROR_SYSTEM_WRONG_DATA. */
    if(value == NULL) {
        return PLAIN_ERROR_SYSTEM_WRONG_DATA;
    }
    switch(type) {
        case PLAIN_TYPE_INTEGER:
        case PLAIN_TYPE_KEYWORD:
        case PLAIN_TYPE_LIST:
        case PLAIN_TYPE_POINTER:
        case PLAIN_TYPE_REAL:
        case PLAIN_TYPE_STRING:
        case PLAIN_TYPE_SUBROUTINE:
            PLAIN_ASSERT(data != NULL);
        case PLAIN_TYPE_BOOLEAN:
        case PLAIN_TYPE_NIL:
            break;
        
        default:
            return PLAIN_ERROR_SYSTEM_WRONG_DATA;
    }
    value->data = data;
    value->length = length;
    value->type = type;
    if(length > 0) {
        value->data = (PLAIN_BYTE*)PLAIN_RESIZE(NULL, length, 0);
        /* PLAIN_ERROR_SYSTEM. */
        if(value->data == NULL) {
            return PLAIN_ERROR_SYSTEM;
        }
        /* Keyword, string. */
        if(type == PLAIN_TYPE_KEYWORD || type == PLAIN_TYPE_STRING) {
            PLAIN_WORD_DOUBLE i = 0;
            PLAIN_WORD_DOUBLE j = 0;
            PLAIN_WORD_DOUBLE k = length;
            for(--k; i < k; i++) {
                if(data[i] == '\\') {
                    if(++i == k) {
                        break;
                    }
                }
                value->data[j++] = data[i];
            }
            value->data[j] = 0;
        /* PLAIN_ERROR_SYSTEM. */
        } else if(memcpy(value->data, data, length) == NULL) {
            return PLAIN_ERROR_SYSTEM;
        }
    }
    return 0;
}

PLAIN_INLINE PLAIN_WORD_DOUBLE PLAIN_JOIN(PLAIN_BYTE* data, PLAIN_WORD_DOUBLE length, struct PLAIN_LIST* node, PLAIN_WORD_DOUBLE type) {
    PLAIN_ASSERT(node != NULL);
    /* PLAIN_ERROR_SYSTEM_WRONG_DATA. */
    if(node == NULL) {
        return PLAIN_ERROR_SYSTEM_WRONG_DATA;
    }
    PLAIN_WORD_DOUBLE distance = node->layout.to - node->layout.from;
    PLAIN_WORD_DOUBLE error = PLAIN_RESERVE(sizeof(struct PLAIN_VALUE), &node->layout);
    if(error != 0) {
        return error;
    }
    return PLAIN_EXPORT(data, length, type, (struct PLAIN_VALUE*)&node->layout.from[distance]);
}

PLAIN_WORD_DOUBLE PLAIN_TOKENIZE(void* context, struct PLAIN_LIST* node, struct PLAIN_LIST* parent, const PLAIN_BYTE* pattern, struct PLAIN_SEGMENT* segment, PLAIN_DELEGATE tracker) {
    PLAIN_ASSERT(node != NULL);
    PLAIN_ASSERT(pattern != NULL);
    PLAIN_ASSERT(segment != NULL);
    /* PLAIN_ERROR_SYSTEM_WRONG_DATA. */
    if(node == NULL || pattern == NULL || segment == NULL) {
        return PLAIN_ERROR_SYSTEM_WRONG_DATA;
    }
    /* Range. */
    PLAIN_WORD_DOUBLE i = 0;
    PLAIN_WORD_DOUBLE j = segment->to - segment->from;
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
    node->keyword.from = node->segment.from;
    node->keyword.to = &segment->from[i];
    /* PLAIN_ERROR_SYNTAX. */
    if(node->keyword.from == node->keyword.to) {
        if(tracker != NULL) {
           tracker(context, node->keyword.from, j - i, PLAIN_ERROR_SYNTAX_UNKNOWN_TOKEN);
        }
        return PLAIN_ERROR_SYNTAX;
    }
    PLAIN_WORD_DOUBLE k = 0;
    PLAIN_WORD_DOUBLE l = 0;
    PLAIN_WORD_DOUBLE m = 0;
    PLAIN_WORD_DOUBLE n = 0;
    PLAIN_WORD_DOUBLE o = 0;
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
            /* PLAIN_ERROR_SYNTAX. */
            if(l == j) {
                if(tracker != NULL) {
                   tracker(context, &segment->from[i - 1], j - i + 1, PLAIN_ERROR_SYNTAX_MISSING_QUOTATION_MARK);
                }
                return PLAIN_ERROR_SYNTAX;
            }
            /* String. */
            PLAIN_WORD_DOUBLE error = PLAIN_JOIN(&segment->from[i], l - i + 1, node, PLAIN_TYPE_STRING);
            if(error != 0) {
                return error;
            }
            i = l;
        /* List. */
        } else if(segment->from[i] == ':') {
            /* Keyword and data. */
            for(k = ++i; i < j; i++) {
                /* String. */
                if(segment->from[m] == '"' || segment->from[m] == '\'') {
                    for(l = i + 1; l < j; l++) {
                        if(segment->from[l] == '\\') {
                            l++;
                        } else if(segment->from[l] == segment->from[i]) {
                            break;
                        }
                    }
                    /* PLAIN_ERROR_SYNTAX. */
                    if(l == j) {
                        if(tracker != NULL) {
                           tracker(context, &segment->from[k - 1], j - k + 1, PLAIN_ERROR_SYNTAX_MISSING_QUOTATION_MARK);
                        }
                        return PLAIN_ERROR_SYNTAX;
                    }
                    i = l;
                /* Break. */
                } else if(segment->from[i] == ';') {
                    break;
                /* Comment. */
                } else if(segment->from[i] == '`') {
                    for(i++; i < j; i++) {
                        if(segment->from[i] == '\r' || segment->from[i] == '\n') {
                            break;
                        }
                    }
                }
            }
            struct PLAIN_SEGMENT subsegment = {&segment->from[k], &segment->from[i]};
            struct PLAIN_LIST* subnode = (struct PLAIN_LIST*)PLAIN_AUTO(sizeof(struct PLAIN_LIST));
            /* PLAIN_ERROR_SYSTEM. */
            if(subnode == NULL) {
                return PLAIN_ERROR_SYSTEM;
            }
            PLAIN_WORD_DOUBLE error = PLAIN_TOKENIZE(context, subnode, NULL, pattern, &subsegment, tracker);
            if(error != 0) {
                return error;
            }
            /* PLAIN_ERROR_SYSTEM. */
            if(PLAIN_JOIN((PLAIN_BYTE*)subnode, sizeof(struct PLAIN_LIST), node, PLAIN_TYPE_LIST) != 0) {
                return PLAIN_ERROR_SYSTEM;
            }
            break;
        /* List. */
        } else if(segment->from[i] == '[' || segment->from[i] == '{') {
            k = segment->from[i] + 0;
            l = segment->from[i] + 2;
            m = ++i;
            n = 1;
            /* Keyword and data. */
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
                    /* PLAIN_ERROR_SYNTAX. */
                    if(o == j) {
                        if(tracker != NULL) {
                           tracker(context, &segment->from[i - 1], j - i + 1, PLAIN_ERROR_SYNTAX_MISSING_QUOTATION_MARK);
                        }
                        return PLAIN_ERROR_SYNTAX;
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
            struct PLAIN_SEGMENT subsegment = {&segment->from[i], &segment->from[m]};
            /* PLAIN_ERROR_SYNTAX. */
            if(n != 0) {
                if(tracker != NULL) {
                   tracker(context, &segment->from[i - 1], j - i + 1, PLAIN_ERROR_SYNTAX_MISSING_BRACKET);
                }
                return PLAIN_ERROR_SYNTAX;
            }
            struct PLAIN_LIST* subnode = (struct PLAIN_LIST*)PLAIN_AUTO(sizeof(struct PLAIN_LIST));
            /* PLAIN_ERROR_SYSTEM. */
            if(subnode == NULL) {
                return PLAIN_ERROR_SYSTEM;
            }
            PLAIN_WORD_DOUBLE error = 0;
            /* Node. */
            if(k == '[') {
                error = PLAIN_TOKENIZE(context, subnode, node, pattern, &subsegment, tracker);
            /* List. */
            } else {
                error = PLAIN_TOKENIZE(context, subnode, NULL, pattern, &subsegment, tracker);
            }
            PLAIN_JOIN((PLAIN_BYTE*)subnode, sizeof(struct PLAIN_LIST), node, PLAIN_TYPE_LIST);
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
                    /* PLAIN_ERROR_SYNTAX. */
                    if(isdigit(segment->from[i - 1]) == 0) {
                        if(tracker != NULL) {
                           tracker(context, &segment->from[k - 1], i - k + 2, PLAIN_ERROR_SYNTAX_ERRONEOUS_EXPRESSION);
                        }
                        return PLAIN_ERROR_SYNTAX;
                    }
                    l++;
                /* Sign. */
                } else if(segment->from[i] == '-' || segment->from[i] == '+') {
                    switch(segment->from[i - 1]) {
                        case 'e':
                        case 'E':
                            break;

                        /* PLAIN_ERROR_SYNTAX. */
                        default: {
                            if(tracker != NULL) {
                               tracker(context, &segment->from[k], i - k + 1, PLAIN_ERROR_SYNTAX_ERRONEOUS_EXPRESSION);
                            }
                            return PLAIN_ERROR_SYNTAX;
                        }
                    }
                    m++;
                /* Scientific. */
                } else if(segment->from[i] == 'e' || segment->from[i] == 'E') {
                    /* PLAIN_ERROR_SYNTAX. */
                    if(isdigit(segment->from[i - 1]) == 0) {
                        if(tracker != NULL) {
                           tracker(context, &segment->from[k], i - k + 1, PLAIN_ERROR_SYNTAX_ERRONEOUS_EXPRESSION);
                        }
                        return PLAIN_ERROR_SYNTAX;
                    }
                    n++;
                /* Digit. */
                } else if(isdigit(segment->from[i]) == 0) {
                    break;
                }
            }
            /* Integer, real. */
            if(k != i - 1 || isdigit(segment->from[k])) {
               /* PLAIN_ERROR_SYNTAX. */
                if(l > 1 || m > 1 || n > 1) {
                    if(tracker != NULL) {
                       tracker(context, &segment->from[k], i - k, PLAIN_ERROR_SYNTAX_ERRONEOUS_EXPRESSION);
                    }
                    return PLAIN_ERROR_SYNTAX;
                }
                /* Real. */
                if(l > 0 || n > 0) {
                    double number = atof((const char*)&segment->from[k]);
                    /* PLAIN_ERROR_SYNTAX. */
                    if(number == HUGE_VAL) {
                        return PLAIN_ERROR_SYNTAX;
                    }
                    PLAIN_REAL real = (PLAIN_REAL)number;
                    /* PLAIN_ERROR_SYSTEM. */
                    if(PLAIN_JOIN((PLAIN_BYTE*)&real, sizeof(PLAIN_REAL), node, PLAIN_TYPE_REAL) != 0) {
                        return PLAIN_ERROR_SYSTEM;
                    }
                /* Integer. */
                } else {
                    PLAIN_WORD_DOUBLE integer = (PLAIN_WORD_DOUBLE)atoi((const char*)&segment->from[k]);
                    /* PLAIN_ERROR_SYSTEM. */
                    if(PLAIN_JOIN((PLAIN_BYTE*)&integer, sizeof(PLAIN_WORD_DOUBLE), node, PLAIN_TYPE_INTEGER) != 0) {
                        return PLAIN_ERROR_SYSTEM;
                    }
                }
            /* Keyword. */
            } else {
                for(m = i - 1; m < j; m++) {
                    if(strchr((const char*)pattern, (char)segment->from[m]) != NULL) {
                        break;
                    }
                }
                /* PLAIN_ERROR_SYNTAX. */
                if(m == k) {
                    if(tracker != NULL) {
                       tracker(context, &segment->from[k], m - k, PLAIN_ERROR_SYNTAX_UNKNOWN_TOKEN);
                    }
                    return PLAIN_ERROR_SYNTAX;
                }
                /* PLAIN_ERROR_SYSTEM. */
                if(PLAIN_JOIN(&segment->from[k], m - i++ + 2, node, PLAIN_TYPE_KEYWORD) != 0) {
                    return PLAIN_ERROR_SYSTEM;
                }
            }
            i--;
        } else if(segment->from[i] == ';') {
            break;
        /* Keyword. */
        } else {
            PLAIN_WORD_DOUBLE identifier = 2166136261U;
            for(k = i; i < j; i++) {
                if(strchr((const char*)pattern, (char)segment->from[i]) != NULL) {
                    break;
                }
                identifier ^= segment->from[i];
                /* 2 ^ 24 + 2 ^ 8 + 0x93. */
                identifier *= 16777619U;
            }
            /* PLAIN_ERROR_SYNTAX. */
            if(i == k) {
                if(tracker != NULL) {
                   tracker(context, &segment->from[i], k - i, PLAIN_ERROR_SYNTAX_UNKNOWN_TOKEN);
                }
                return PLAIN_ERROR_SYNTAX;
            }
            --i;
            /* Boolean. */
            if(identifier == 1647734778U) {
                PLAIN_WORD_DOUBLE error = PLAIN_JOIN(NULL, 0, node, PLAIN_TYPE_BOOLEAN);
                if(error != 0) {
                    return error;
                }
            /* Nil. */
            } else if(identifier == 2913447899U) {
                PLAIN_WORD_DOUBLE error = PLAIN_JOIN(NULL, 0, node, PLAIN_TYPE_NIL);
                if(error != 0) {
                    return error;
                }
            /* Boolean. */
            } else if(identifier == 1319056784U) {
                PLAIN_WORD_DOUBLE error = PLAIN_JOIN((PLAIN_BYTE*)0xFF, 0, node, PLAIN_TYPE_BOOLEAN);
                if(error != 0) {
                    return error;
                }
            /* Keyword. */
            } else {
                PLAIN_WORD_DOUBLE error = PLAIN_JOIN(&segment->from[k], i - k + 2, node, PLAIN_TYPE_KEYWORD);
                if(error != 0) {
                    return error;
                }
            }
        }
    }
    if(i == j || parent != NULL) {
        return 0;
    }
    struct PLAIN_SEGMENT subsegment = {&segment->from[++i], &segment->from[j]};
    if(subsegment.from == subsegment.to) {
        return 0;
    }
    node->node = (struct PLAIN_LIST*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_LIST), 0);
    /* PLAIN_ERROR_SYSTEM. */
    if(node->node == NULL) {
        return PLAIN_ERROR_SYSTEM;
    }
    return PLAIN_TOKENIZE(context, node->node, NULL, pattern, &subsegment, tracker);
}

void PLAIN_UNLINK(struct PLAIN_LIST* node) {
    if(node == NULL) {
        return;
    }
    PLAIN_BYTE* from = node->layout.from;
    PLAIN_BYTE* to = node->layout.to;
    for(; from != to; from += sizeof(struct PLAIN_VALUE)) {
        struct PLAIN_VALUE* value = (struct PLAIN_VALUE*)from;
        if(value->type == PLAIN_TYPE_LIST) {
            PLAIN_UNLINK((struct PLAIN_LIST*)value->data);
        }
        if(value->length > 0) {
            PLAIN_RESIZE(value->data, 0, value->length);
        }
    }
    if(node->layout.from != NULL) {
        PLAIN_RESIZE(node->layout.from, 0, node->layout.to - node->layout.from);
    }
    PLAIN_UNLINK(node->node);
    if(node->node != NULL) {
        PLAIN_RESIZE(node->node, 0, sizeof(struct PLAIN_LIST));
    }
}
