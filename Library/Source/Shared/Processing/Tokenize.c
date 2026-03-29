/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     02/23/2013.
 * Revision 09/02/2015.
 *
 * Copyright 2015 Nerijus Ramanauskas.
 *
 * Tokenize — recursive-descent parser.
 * See Parser.h for the public contracts of PLAIN_EXPORT, PLAIN_TOKENIZE,
 * PLAIN_UNLINK, and PLAIN_LIST_COPY.
 */

#include <Plain/Parser.h>

PLAIN_WORD_DOUBLE PLAIN_EXPORT(PLAIN_BYTE* data, PLAIN_WORD_DOUBLE length, PLAIN_WORD_DOUBLE type, struct PLAIN_VALUE* value) {
    PLAIN_ASSERT(value != NULL);
    if(value == NULL) {
        return PLAIN_ERROR_SYSTEM_WRONG_DATA;
    }

    /* Validate type — data pointer is required for most types. */
    switch(type) {
        case PLAIN_TYPE_INTEGER:
        case PLAIN_TYPE_INTERPOLATED:
        case PLAIN_TYPE_KEYWORD:
        case PLAIN_TYPE_LIST:
        case PLAIN_TYPE_POINTER:
        case PLAIN_TYPE_REAL:
        case PLAIN_TYPE_STRING:
            PLAIN_ASSERT(data != NULL);
        case PLAIN_TYPE_BOOLEAN:
        case PLAIN_TYPE_NIL:
            break;

        default:
            return PLAIN_ERROR_SYSTEM_WRONG_DATA;
    }

    value->data   = data;
    value->length = length;
    value->type   = type;
    value->owner  = 0;

    /* Heap-copy for sized data. */
    if(length > 0) {
        value->data = (PLAIN_BYTE*)PLAIN_RESIZE(NULL, length, 0);
        if(value->data == NULL) {
            return PLAIN_ERROR_SYSTEM;
        }

        /* Strings and keywords: process backslash escapes during copy. */
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
        } else if(memcpy(value->data, data, length) == NULL) {
            return PLAIN_ERROR_SYSTEM;
        }
    }

    return 0;
}

/* Grows the node's argument array by one slot and stores data via PLAIN_EXPORT. */
PLAIN_INLINE PLAIN_WORD_DOUBLE PLAIN_JOIN(PLAIN_BYTE* data, PLAIN_WORD_DOUBLE length, struct PLAIN_LIST* node, PLAIN_WORD_DOUBLE type) {
    PLAIN_ASSERT(node != NULL);
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

/* ================================================================== */
/*  PLAIN_INTERPOLATE — parse string interpolation                    */
/* ================================================================== */

/* Handles a string containing {expressions}.  Splits the string into
 * literal segments and interpolated parts, tokenizes each interpolated
 * part as a sub-expression, and stores the result as a single
 * PLAIN_TYPE_INTERPOLATED argument.  The walker resolves these parts
 * and concatenates them at runtime. */
static PLAIN_WORD_DOUBLE PLAIN_INTERPOLATE(void* context, struct PLAIN_LIST* node,
    const PLAIN_BYTE* data, PLAIN_WORD_DOUBLE length,
    const PLAIN_BYTE* delimiters, PLAIN_DELEGATE tracker) {

    /* Create a temporary node to collect the interpolation parts. */
    struct PLAIN_LIST* subnode = (struct PLAIN_LIST*)PLAIN_AUTO(sizeof(struct PLAIN_LIST));
    if(subnode == NULL) {
        return PLAIN_ERROR_SYSTEM;
    }
    subnode->keyword.from = NULL;
    subnode->keyword.to   = NULL;
    subnode->layout.from  = NULL;
    subnode->layout.to    = NULL;
    subnode->node         = NULL;
    subnode->parent       = NULL;
    subnode->segment.from = (PLAIN_BYTE*)data;
    subnode->segment.to   = (PLAIN_BYTE*)(data + length);

    PLAIN_WORD_DOUBLE position = 0;
    PLAIN_WORD_DOUBLE literal_start = 0;

    while(position < length) {
        /* Skip escaped characters. */
        if(data[position] == '\\' && position + 1 < length) {
            position += 2;
            continue;
        }

        /* Not an interpolation start — advance. */
        if(data[position] != '{') {
            position++;
            continue;
        }

        /* Emit any literal text before this brace. */
        if(position > literal_start) {
            PLAIN_WORD_DOUBLE error = PLAIN_JOIN((PLAIN_BYTE*)&data[literal_start],
                (position - literal_start) + 1, subnode, PLAIN_TYPE_STRING);
            if(error != 0) {
                return error;
            }
        }

        /* Find the matching closing brace, respecting nesting and quotes. */
        PLAIN_WORD_DOUBLE content_start = position + 1;
        PLAIN_WORD_DOUBLE depth = 1;
        PLAIN_WORD_DOUBLE scan = content_start;

        while(scan < length && depth > 0) {
            if(data[scan] == '\\' && scan + 1 < length) {
                scan += 2;
                continue;
            }
            /* Skip quoted strings inside interpolation. */
            if(data[scan] == '"' || data[scan] == '\'') {
                PLAIN_BYTE quote = data[scan++];
                while(scan < length) {
                    if(data[scan] == '\\' && scan + 1 < length) {
                        scan += 2;
                        continue;
                    }
                    if(data[scan] == quote) {
                        scan++;
                        break;
                    }
                    scan++;
                }
                continue;
            }
            if(data[scan] == '{') {
                depth++;
            } else if(data[scan] == '}') {
                depth--;
                if(depth == 0) {
                    break;
                }
            }
            scan++;
        }

        /* Tokenize the content between the braces as a sub-expression. */
        PLAIN_WORD_DOUBLE content_end = scan;
        if(content_start < content_end) {
            struct PLAIN_SEGMENT subsegment = {
                (PLAIN_BYTE*)&data[content_start],
                (PLAIN_BYTE*)&data[content_end]
            };
            struct PLAIN_LIST* expression = (struct PLAIN_LIST*)PLAIN_AUTO(sizeof(struct PLAIN_LIST));
            if(expression == NULL) {
                return PLAIN_ERROR_SYSTEM;
            }
            PLAIN_WORD_DOUBLE error = PLAIN_TOKENIZE(context, expression, subnode, delimiters, &subsegment, tracker);
            if(error != 0) {
                return error;
            }
            error = PLAIN_JOIN((PLAIN_BYTE*)expression, sizeof(struct PLAIN_LIST), subnode, PLAIN_TYPE_LIST);
            if(error != 0) {
                return error;
            }
        }

        position = content_end + 1;
        literal_start = position;
    }

    /* Emit any trailing literal text. */
    if(position > literal_start) {
        PLAIN_WORD_DOUBLE error = PLAIN_JOIN((PLAIN_BYTE*)&data[literal_start],
            (position - literal_start) + 1, subnode, PLAIN_TYPE_STRING);
        if(error != 0) {
            return error;
        }
    }

    /* Store the parts list as a single PLAIN_TYPE_INTERPOLATED argument. */
    return PLAIN_JOIN((PLAIN_BYTE*)subnode, sizeof(struct PLAIN_LIST), node, PLAIN_TYPE_INTERPOLATED);
}

PLAIN_WORD_DOUBLE PLAIN_TOKENIZE(void* context, struct PLAIN_LIST* node, struct PLAIN_LIST* parent, const PLAIN_BYTE* delimiters, struct PLAIN_SEGMENT* segment, PLAIN_DELEGATE tracker) {
    PLAIN_ASSERT(node != NULL);
    PLAIN_ASSERT(delimiters != NULL);
    PLAIN_ASSERT(segment != NULL);
    if(node == NULL || delimiters == NULL || segment == NULL) {
        return PLAIN_ERROR_SYSTEM_WRONG_DATA;
    }

    PLAIN_WORD_DOUBLE i = 0;
    PLAIN_WORD_DOUBLE j = segment->to - segment->from;

    /* ---- Skip leading whitespace and comments. -------------------- */
    for(; i < j; i++) {
        switch(segment->from[i]) {
            case '\t':
            case '\n':
            case '\r':
            case ' ':
                continue;
        }
        /* Comments: ` until end of line. */
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

    /* ---- Skip trailing whitespace. -------------------------------- */
    for(; j > i; j--) {
        switch(segment->from[j - 1]) {
            case '\t':
            case '\n':
            case '\r':
            case ' ':
                continue;
        }
        break;
    }

    /* ---- Initialize the node. ------------------------------------- */
    node->keyword.from = NULL;
    node->keyword.to   = NULL;
    node->layout.from  = NULL;
    node->layout.to    = NULL;
    node->node         = NULL;
    node->parent       = parent;
    node->segment.from = NULL;
    node->segment.to   = NULL;

    /* Empty source — nothing to parse. */
    if(i == j) {
        return 0;
    }

    node->segment.from = &segment->from[i];
    node->segment.to   = &segment->from[j];

    /* ---- Extract the keyword (first token). ----------------------- */
    for(; i < j; i++) {
        if(strchr((const char*)delimiters, (char)segment->from[i]) != NULL) {
            break;
        }
    }
    node->keyword.from = node->segment.from;
    node->keyword.to   = &segment->from[i];

    if(node->keyword.from == node->keyword.to) {
        if(tracker != NULL) {
           tracker(context, node->keyword.from, j - i, PLAIN_ERROR_SYNTAX_UNKNOWN_TOKEN);
        }
        return PLAIN_ERROR_SYNTAX;
    }

    /* ---- Parse arguments. ----------------------------------------- */
    /* These locals track state for number parsing and bracket matching:
     *   k — multi-purpose: quote char, bracket open char, number start
     *   l — multi-purpose: scan position in strings/brackets, decimal count
     *   m — multi-purpose: scan position, interpolation flag, sign count
     *   n — bracket depth / scientific notation count
     *   o — inner scan position for strings inside brackets */
    PLAIN_WORD_DOUBLE k = 0;
    PLAIN_WORD_DOUBLE l = 0;
    PLAIN_WORD_DOUBLE m = 0;
    PLAIN_WORD_DOUBLE n = 0;
    PLAIN_WORD_DOUBLE o = 0;

    for(; i < j; i++) {
        /* Skip whitespace and commas between arguments. */
        switch(segment->from[i]) {
            case '\t':
            case '\n':
            case '\r':
            case ' ':
            case ',':
                continue;
        }

        /* Skip comments. */
        if(segment->from[i] == '`') {
            for(i++; i < j; i++) {
                if(segment->from[i] == '\r' || segment->from[i] == '\n') {
                    break;
                }
            }
            continue;
        }

        /* ---- String literal. -------------------------------------- */
        if(segment->from[i] == '"' || segment->from[i] == '\'') {
            k = segment->from[i];   /* Quote character. */
            l = ++i;                /* Start of string content. */
            m = 0;                  /* Interpolation flag. */

            /* Find the closing quote, tracking escapes and braces. */
            for(; l < j; l++) {
                if(segment->from[l] == '\\') {
                    l++;
                } else if(segment->from[l] == k) {
                    break;
                } else if(segment->from[l] == '{') {
                    m = 1;
                }
            }

            /* Unterminated string. */
            if(l == j) {
                if(tracker != NULL) {
                   tracker(context, &segment->from[i - 1], j - i + 1, PLAIN_ERROR_SYNTAX_MISSING_QUOTATION_MARK);
                }
                return PLAIN_ERROR_SYNTAX;
            }

            if(m) {
                /* String with interpolation — parse {expressions}. */
                PLAIN_WORD_DOUBLE error = PLAIN_INTERPOLATE(context, node,
                    &segment->from[i], l - i, delimiters, tracker);
                if(error != 0) {
                    return error;
                }
            } else {
                /* Plain string — store directly. */
                PLAIN_WORD_DOUBLE error = PLAIN_JOIN(&segment->from[i], l - i + 1, node, PLAIN_TYPE_STRING);
                if(error != 0) {
                    return error;
                }
            }
            i = l;

        /* ---- Colon shorthand: opens a block until semicolon. ------ */
        } else if(segment->from[i] == ':') {
            for(k = ++i; i < j; i++) {
                if(segment->from[m] == '"' || segment->from[m] == '\'') {
                    for(l = i + 1; l < j; l++) {
                        if(segment->from[l] == '\\') {
                            l++;
                        } else if(segment->from[l] == segment->from[i]) {
                            break;
                        }
                    }
                    if(l == j) {
                        if(tracker != NULL) {
                           tracker(context, &segment->from[k - 1], j - k + 1, PLAIN_ERROR_SYNTAX_MISSING_QUOTATION_MARK);
                        }
                        return PLAIN_ERROR_SYNTAX;
                    }
                    i = l;
                } else if(segment->from[i] == ';') {
                    break;
                } else if(segment->from[i] == '`') {
                    for(i++; i < j; i++) {
                        if(segment->from[i] == '\r' || segment->from[i] == '\n') {
                            break;
                        }
                    }
                }
            }

            /* Tokenize the colon block content as a sub-node. */
            struct PLAIN_SEGMENT subsegment = {&segment->from[k], &segment->from[i]};
            struct PLAIN_LIST* subnode = (struct PLAIN_LIST*)PLAIN_AUTO(sizeof(struct PLAIN_LIST));
            if(subnode == NULL) {
                return PLAIN_ERROR_SYSTEM;
            }
            PLAIN_WORD_DOUBLE error = PLAIN_TOKENIZE(context, subnode, NULL, delimiters, &subsegment, tracker);
            if(error != 0) {
                return error;
            }
            if(PLAIN_JOIN((PLAIN_BYTE*)subnode, sizeof(struct PLAIN_LIST), node, PLAIN_TYPE_LIST) != 0) {
                return PLAIN_ERROR_SYSTEM;
            }
            break;

        /* ---- Brackets: [...] expressions and { } blocks. ---------- */
        } else if(segment->from[i] == '[' || segment->from[i] == '{') {
            k = segment->from[i];       /* Opening bracket character. */
            l = segment->from[i] + 2;   /* Closing bracket (ASCII trick: [ -> ], { -> }). */
            m = ++i;                     /* Scan position. */
            n = 1;                       /* Bracket nesting depth. */

            /* Find the matching closing bracket. */
            for(; m < j; m++) {
                /* Skip strings inside brackets. */
                if(segment->from[m] == '"' || segment->from[m] == '\'') {
                    for(o = m + 1; o < j; o++) {
                        if(segment->from[o] == '\\') {
                            o++;
                        } else if(segment->from[o] == segment->from[m]) {
                            break;
                        }
                    }
                    if(o == j) {
                        if(tracker != NULL) {
                           tracker(context, &segment->from[i - 1], j - i + 1, PLAIN_ERROR_SYNTAX_MISSING_QUOTATION_MARK);
                        }
                        return PLAIN_ERROR_SYNTAX;
                    }
                    m = o;
                } else if(segment->from[m] == k) {
                    n++;
                } else if(segment->from[m] == l) {
                    n--;
                } else if(segment->from[m] == '`') {
                    /* Skip comments. */
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

            /* Unmatched bracket. */
            if(n != 0) {
                if(tracker != NULL) {
                   tracker(context, &segment->from[i - 1], j - i + 1, PLAIN_ERROR_SYNTAX_MISSING_BRACKET);
                }
                return PLAIN_ERROR_SYNTAX;
            }

            struct PLAIN_LIST* subnode = (struct PLAIN_LIST*)PLAIN_AUTO(sizeof(struct PLAIN_LIST));
            if(subnode == NULL) {
                return PLAIN_ERROR_SYSTEM;
            }

            PLAIN_WORD_DOUBLE error = 0;
            if(k == '[') {
                /* Expression [...] — parent is set so the walker knows to evaluate it. */
                error = PLAIN_TOKENIZE(context, subnode, node, delimiters, &subsegment, tracker);
            } else {
                /* Block { } — parent is NULL so the walker defers it. */
                error = PLAIN_TOKENIZE(context, subnode, NULL, delimiters, &subsegment, tracker);
            }

            PLAIN_JOIN((PLAIN_BYTE*)subnode, sizeof(struct PLAIN_LIST), node, PLAIN_TYPE_LIST);
            if(error != 0) {
                return error;
            }
            i = m;

        /* ---- Numeric literal (integer or real). ------------------- */
        } else if(segment->from[i] == '+' || segment->from[i] == '-' || isdigit(segment->from[i])) {
            k = i++;        /* Start of the number. */
            l = 0;          /* Decimal point count. */
            m = 0;          /* Sign count (after 'e'). */
            n = 0;          /* Scientific notation 'e' count. */

            /* Scan digits, decimal points, and scientific notation. */
            for(; i < j; i++) {
                if(segment->from[i] == '.') {
                    if(isdigit(segment->from[i - 1]) == 0) {
                        if(tracker != NULL) {
                           tracker(context, &segment->from[k - 1], i - k + 2, PLAIN_ERROR_SYNTAX_ERRONEOUS_EXPRESSION);
                        }
                        return PLAIN_ERROR_SYNTAX;
                    }
                    l++;
                } else if(segment->from[i] == '-' || segment->from[i] == '+') {
                    /* Signs are only valid after 'e'/'E'. */
                    switch(segment->from[i - 1]) {
                        case 'e':
                        case 'E':
                            break;
                        default: {
                            if(tracker != NULL) {
                               tracker(context, &segment->from[k], i - k + 1, PLAIN_ERROR_SYNTAX_ERRONEOUS_EXPRESSION);
                            }
                            return PLAIN_ERROR_SYNTAX;
                        }
                    }
                    m++;
                } else if(segment->from[i] == 'e' || segment->from[i] == 'E') {
                    if(isdigit(segment->from[i - 1]) == 0) {
                        if(tracker != NULL) {
                           tracker(context, &segment->from[k], i - k + 1, PLAIN_ERROR_SYNTAX_ERRONEOUS_EXPRESSION);
                        }
                        return PLAIN_ERROR_SYNTAX;
                    }
                    n++;
                } else if(isdigit(segment->from[i]) == 0) {
                    break;
                }
            }

            /* Distinguish a number from a bare +/- keyword. */
            if(k != i - 1 || isdigit(segment->from[k])) {
                /* Validate: at most one decimal, one sign, one exponent. */
                if(l > 1 || m > 1 || n > 1) {
                    if(tracker != NULL) {
                       tracker(context, &segment->from[k], i - k, PLAIN_ERROR_SYNTAX_ERRONEOUS_EXPRESSION);
                    }
                    return PLAIN_ERROR_SYNTAX;
                }

                if(l > 0 || n > 0) {
                    /* Real number. */
                    double number = atof((const char*)&segment->from[k]);
                    if(number == HUGE_VAL) {
                        return PLAIN_ERROR_SYNTAX;
                    }
                    PLAIN_REAL real = (PLAIN_REAL)number;
                    if(PLAIN_JOIN((PLAIN_BYTE*)&real, sizeof(PLAIN_REAL), node, PLAIN_TYPE_REAL) != 0) {
                        return PLAIN_ERROR_SYSTEM;
                    }
                } else {
                    /* Integer. */
                    PLAIN_WORD_QUADRUPLE integer = (PLAIN_WORD_QUADRUPLE)atoll((const char*)&segment->from[k]);
                    if(PLAIN_JOIN((PLAIN_BYTE*)&integer, sizeof(PLAIN_WORD_QUADRUPLE), node, PLAIN_TYPE_INTEGER) != 0) {
                        return PLAIN_ERROR_SYSTEM;
                    }
                }
            } else {
                /* Bare +/- sign — treat as a keyword (operator name). */
                for(m = i - 1; m < j; m++) {
                    if(strchr((const char*)delimiters, (char)segment->from[m]) != NULL) {
                        break;
                    }
                }
                if(m == k) {
                    if(tracker != NULL) {
                       tracker(context, &segment->from[k], m - k, PLAIN_ERROR_SYNTAX_UNKNOWN_TOKEN);
                    }
                    return PLAIN_ERROR_SYNTAX;
                }
                if(PLAIN_JOIN(&segment->from[k], m - i++ + 2, node, PLAIN_TYPE_KEYWORD) != 0) {
                    return PLAIN_ERROR_SYSTEM;
                }
            }
            i--;

        /* ---- Semicolon — statement separator. --------------------- */
        } else if(segment->from[i] == ';') {
            break;

        /* ---- Keyword (identifier, boolean, nil). ------------------ */
        } else {
            /* Hash the keyword text (FNV-1a) to detect reserved literals. */
            PLAIN_WORD_DOUBLE identifier = 2166136261U;
            for(k = i; i < j; i++) {
                if(strchr((const char*)delimiters, (char)segment->from[i]) != NULL) {
                    break;
                }
                identifier ^= segment->from[i];
                identifier *= 16777619U;   /* FNV prime: 2^24 + 2^8 + 0x93 */
            }

            if(i == k) {
                if(tracker != NULL) {
                   tracker(context, &segment->from[i], k - i, PLAIN_ERROR_SYNTAX_UNKNOWN_TOKEN);
                }
                return PLAIN_ERROR_SYNTAX;
            }
            --i;

            /* Check hash against reserved literal values. */
            if(identifier == 1647734778U) {
                /* "no" — boolean false. */
                PLAIN_WORD_DOUBLE error = PLAIN_JOIN(NULL, 0, node, PLAIN_TYPE_BOOLEAN);
                if(error != 0) {
                    return error;
                }
            } else if(identifier == 2913447899U) {
                /* "none" — nil. */
                PLAIN_WORD_DOUBLE error = PLAIN_JOIN(NULL, 0, node, PLAIN_TYPE_NIL);
                if(error != 0) {
                    return error;
                }
            } else if(identifier == 1319056784U) {
                /* "yes" — boolean true. */
                PLAIN_WORD_DOUBLE error = PLAIN_JOIN((PLAIN_BYTE*)0xFF, 0, node, PLAIN_TYPE_BOOLEAN);
                if(error != 0) {
                    return error;
                }
            } else {
                /* General keyword — variable name, procedure name, etc. */
                PLAIN_WORD_DOUBLE error = PLAIN_JOIN(&segment->from[k], i - k + 2, node, PLAIN_TYPE_KEYWORD);
                if(error != 0) {
                    return error;
                }
            }
        }
    }

    /* ---- Parse the next statement (after semicolon). --------------- */
    /* Only top-level and block-level parsing chains to siblings.
     * Expressions (parent != NULL) stop after one statement. */
    if(i == j || parent != NULL) {
        return 0;
    }

    struct PLAIN_SEGMENT subsegment = {&segment->from[++i], &segment->from[j]};
    if(subsegment.from == subsegment.to) {
        return 0;
    }

    /* Allocate the sibling node and recursively parse the rest. */
    node->node = (struct PLAIN_LIST*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_LIST), 0);
    if(node->node == NULL) {
        return PLAIN_ERROR_SYSTEM;
    }
    return PLAIN_TOKENIZE(context, node->node, NULL, delimiters, &subsegment, tracker);
}

void PLAIN_UNLINK(struct PLAIN_LIST* node) {
    if(node == NULL) {
        return;
    }

    /* Free each argument value. */
    PLAIN_BYTE* from = node->layout.from;
    PLAIN_BYTE* to   = node->layout.to;
    for(; from != to; from += sizeof(struct PLAIN_VALUE)) {
        struct PLAIN_VALUE* value = (struct PLAIN_VALUE*)from;

        /* Recursively free child node trees (expressions, blocks, interpolations). */
        if(value->type == PLAIN_TYPE_LIST || value->type == PLAIN_TYPE_INTERPOLATED) {
            PLAIN_UNLINK((struct PLAIN_LIST*)value->data);
        }

        /* Free heap-allocated value data (but not object frame pointers). */
        if(value->type != PLAIN_TYPE_OBJECT && value->length > 0) {
            PLAIN_RESIZE(value->data, 0, value->length);
        }
    }

    /* Free the argument array itself. */
    if(node->layout.from != NULL) {
        PLAIN_RESIZE(node->layout.from, 0, node->layout.to - node->layout.from);
    }

    /* Recursively free sibling nodes. */
    PLAIN_UNLINK(node->node);
    if(node->node != NULL) {
        PLAIN_RESIZE(node->node, 0, sizeof(struct PLAIN_LIST));
    }
}

static struct PLAIN_LIST* PLAIN_LIST_COPY_NODE(const struct PLAIN_LIST* node, struct PLAIN_LIST* new_parent);

struct PLAIN_LIST* PLAIN_LIST_COPY(const struct PLAIN_LIST* node) {
    return PLAIN_LIST_COPY_NODE(node, NULL);
}

/* Keyword and segment pointers borrow into the original source buffer.
 * Argument data is duplicated; child trees are recursively copied. */
static struct PLAIN_LIST* PLAIN_LIST_COPY_NODE(const struct PLAIN_LIST* node, struct PLAIN_LIST* new_parent) {
    if(node == NULL) {
        return NULL;
    }

    struct PLAIN_LIST* copy = (struct PLAIN_LIST*)PLAIN_RESIZE(NULL, sizeof(struct PLAIN_LIST), 0);
    if(copy == NULL) {
        return NULL;
    }

    /* Keyword and segment borrow from the original source — not owned. */
    copy->keyword      = node->keyword;
    copy->segment.from = NULL;
    copy->segment.to   = NULL;
    copy->parent       = new_parent;
    copy->node         = NULL;
    copy->layout.from  = NULL;
    copy->layout.to    = NULL;

    /* Copy the argument array. */
    PLAIN_WORD_DOUBLE lsize = node->layout.to - node->layout.from;
    if(lsize > 0 && node->layout.from != NULL) {
        copy->layout.from = (PLAIN_BYTE*)PLAIN_RESIZE(NULL, lsize, 0);
        if(copy->layout.from == NULL) {
            PLAIN_RESIZE(copy, 0, sizeof(struct PLAIN_LIST));
            return NULL;
        }
        copy->layout.to = copy->layout.from + lsize;

        /* Copy each argument value individually. */
        for(PLAIN_WORD_DOUBLE off = 0; off < lsize; off += sizeof(struct PLAIN_VALUE)) {
            const struct PLAIN_VALUE* sv = (const struct PLAIN_VALUE*)(node->layout.from + off);
            struct PLAIN_VALUE* dv = (struct PLAIN_VALUE*)(copy->layout.from + off);

            dv->type  = sv->type;
            dv->owner = sv->owner;

            if(sv->type == PLAIN_TYPE_LIST || sv->type == PLAIN_TYPE_INTERPOLATED) {
                /* Recursively copy child trees.  Preserve the parent != NULL
                 * distinction: inline expressions get the new enclosing node
                 * as parent; deferred blocks keep parent == NULL. */
                const struct PLAIN_LIST* child = (const struct PLAIN_LIST*)sv->data;
                struct PLAIN_LIST* child_parent = (child != NULL && child->parent != NULL) ? copy : NULL;
                dv->data   = (PLAIN_BYTE*)PLAIN_LIST_COPY_NODE(child, child_parent);
                dv->length = sv->length;
            } else if(sv->length > 0 && sv->data != NULL) {
                /* Heap-allocated data — duplicate. */
                dv->data = (PLAIN_BYTE*)PLAIN_RESIZE(NULL, sv->length, 0);
                if(dv->data == NULL) {
                    dv->length = 0;
                    dv->type   = PLAIN_TYPE_NIL;
                } else {
                    memcpy(dv->data, sv->data, sv->length);
                    dv->length = sv->length;
                }
            } else {
                /* Zero-length data: boolean marker, nil, or inline pointer. */
                dv->data   = sv->data;
                dv->length = sv->length;
            }
        }
    }

    /* Recursively copy sibling nodes. */
    copy->node = PLAIN_LIST_COPY_NODE(node->node, NULL);
    return copy;
}
