/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     05/09/2013.
 * Revision 03/28/2026.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * Walk — see Parser.h for the public contract of PLAIN_WALK.
 */

#include <Plain/Parser.h>

PLAIN_WORD_DOUBLE PLAIN_WALK(void* context, PLAIN_SUBROUTINE resolver, struct PLAIN_LIST* node, struct PLAIN_VALUE* value) {
    PLAIN_ASSERT(resolver != NULL);
    PLAIN_ASSERT(node != NULL);
    if(resolver == NULL || node == NULL) {
        return PLAIN_ERROR_SYSTEM_WRONG_DATA;
    }
    if(node->keyword.from == NULL) {
        return 0;
    }

    /* --- Phase 1: resolve arguments -------------------------------- */
    PLAIN_WORD_DOUBLE length = PLAIN_ARITY(node);

    for(PLAIN_WORD_DOUBLE index = 0; index < length; index++) {
        struct PLAIN_VALUE* argument = (struct PLAIN_VALUE*)PLAIN_ARGUMENT(node, index);

        /* Keywords — let the resolver decide whether to treat as a name
         * or resolve to a value. */
        if(argument->type == PLAIN_TYPE_KEYWORD) {
            struct PLAIN_SEGMENT keyword = {
                argument->data,
                argument->data + argument->length
            };
            if(keyword.from == keyword.to) {
                continue;
            }
            PLAIN_WORD_DOUBLE error = resolver(context, keyword.from, PLAIN_TYPE_KEYWORD, argument);
            /* Free the old keyword data if the resolver replaced it. */
            if(argument->data != keyword.from) {
                PLAIN_RESIZE(keyword.from, 0, keyword.to - keyword.from);
            }
            if(error != 0) {
                return error;
            }

        /* Expressions [...] — recursively walk and substitute the result. */
        } else if(argument->type == PLAIN_TYPE_LIST) {
            struct PLAIN_LIST* child = (struct PLAIN_LIST*)argument->data;

            /* Blocks (parent == NULL) are deferred — skip them. */
            if(child == NULL || child->parent == NULL) {
                continue;
            }

            PLAIN_WORD_DOUBLE error = PLAIN_WALK(context, resolver, child, argument);
            /* Free the child tree if the walk replaced the argument data. */
            if((struct PLAIN_LIST*)argument->data != child) {
                PLAIN_UNLINK(child);
                PLAIN_RESIZE(child, 0, sizeof(struct PLAIN_LIST));
            }
            if(error != 0) {
                return error;
            }

        /* String interpolation — resolve all parts and concatenate. */
        } else if(argument->type == PLAIN_TYPE_INTERPOLATED) {
            struct PLAIN_LIST* parts = (struct PLAIN_LIST*)argument->data;
            if(parts == NULL) {
                continue;
            }

            /* First pass: resolve any sub-expression parts. */
            PLAIN_WORD_DOUBLE count = PLAIN_ARITY(parts);
            for(PLAIN_WORD_DOUBLE p = 0; p < count; p++) {
                struct PLAIN_VALUE* part = PLAIN_ARGUMENT(parts, p);
                if(part->type == PLAIN_TYPE_LIST) {
                    struct PLAIN_LIST* child = (struct PLAIN_LIST*)part->data;
                    if(child == NULL || child->parent == NULL) {
                        continue;
                    }
                    PLAIN_WORD_DOUBLE error = PLAIN_WALK(context, resolver, child, part);
                    if((struct PLAIN_LIST*)part->data != child) {
                        PLAIN_UNLINK(child);
                        PLAIN_RESIZE(child, 0, sizeof(struct PLAIN_LIST));
                    }
                    if(error != 0) {
                        return error;
                    }
                }
            }

            /* Second pass: concatenate all parts into a single string.
             * Keywords are resolved via the resolver to get their value. */
            struct PLAIN_SEGMENT result = {NULL, NULL};
            for(PLAIN_WORD_DOUBLE p = 0; p < count; p++) {
                struct PLAIN_VALUE* part = PLAIN_ARGUMENT(parts, p);

                /* Resolve keyword parts to their current value. */
                if(part->type == PLAIN_TYPE_KEYWORD && part->data != NULL) {
                    resolver(context, part->data, PLAIN_TYPE_KEYWORD, part);
                }

                /* Convert each part to its string representation. */
                PLAIN_BYTE buffer[64];
                const PLAIN_BYTE* str = (const PLAIN_BYTE*)"";
                PLAIN_WORD_DOUBLE len = 0;

                switch(part->type) {
                    case PLAIN_TYPE_STRING:
                        str = part->data;
                        len = (PLAIN_WORD_DOUBLE)strlen((const char*)str);
                        break;
                    case PLAIN_TYPE_INTEGER:
                        len = sprintf((char*)buffer, "%lld", (long long)*(PLAIN_WORD_QUADRUPLE*)part->data);
                        str = buffer;
                        break;
                    case PLAIN_TYPE_REAL:
                        len = sprintf((char*)buffer, "%g", (PLAIN_REAL_DOUBLE)*(PLAIN_REAL*)part->data);
                        str = buffer;
                        break;
                    case PLAIN_TYPE_BOOLEAN:
                        str = part->data != NULL ? (const PLAIN_BYTE*)"yes" : (const PLAIN_BYTE*)"no";
                        len = (PLAIN_WORD_DOUBLE)strlen((const char*)str);
                        break;
                    default:
                        str = (const PLAIN_BYTE*)"none";
                        len = 4;
                        break;
                }

                if(len > 0) {
                    PLAIN_CONCATENATE(&result, len, str);
                }
            }

            /* Null-terminate and replace the interpolation node with the string. */
            PLAIN_BYTE zero = 0;
            PLAIN_CONCATENATE(&result, 1, &zero);
            PLAIN_UNLINK(parts);
            PLAIN_RESIZE(parts, 0, sizeof(struct PLAIN_LIST));
            argument->data   = result.from;
            argument->length = result.to - result.from;
            argument->type   = PLAIN_TYPE_STRING;
            argument->owner  = 0;
        }
    }

    /* --- Phase 2: dispatch the node to the resolver ---------------- */
    PLAIN_WORD_DOUBLE error = resolver(context, node, PLAIN_TYPE_LIST, value);
    if(error != 0) {
        return error;
    }

    /* --- Phase 3: continue to the next sibling statement ----------- */
    if(node->node != NULL) {
        PLAIN_WORD_DOUBLE error = PLAIN_WALK(context, resolver, node->node, NULL);
        if(error != 0) {
            return error;
        }
    }

    return 0;
}
