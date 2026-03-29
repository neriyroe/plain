/*
 * Author   Nerijus Ramanauskas <nerijus@signaintermedia.com>.
 * Date     2013-02-23.
 * Revision 2026-03-29.
 *
 * Copyright 2026 Nerijus Ramanauskas.
 *
 * List — the parse tree node structure.
 *
 * Each PLAIN_LIST represents a single statement or expression.  Nodes
 * form two linked structures:
 *   - Sibling chain: node->node links consecutive statements (separated
 *     by semicolons in the source).
 *   - Argument array: node->layout holds a contiguous array of PLAIN_VALUE
 *     arguments (accessed via PLAIN_ARGUMENT / PLAIN_ARITY).
 *
 * The parent pointer distinguishes expressions from blocks:
 *   - parent != NULL  — inline expression [...], resolved by the walker.
 *   - parent == NULL  — deferred block { }, left for the callable to walk.
 */

struct PLAIN_LIST {
    struct PLAIN_SEGMENT keyword;  /* The first token (procedure/function name). Borrows into source. */
    struct PLAIN_SEGMENT layout;   /* Contiguous array of PLAIN_VALUE arguments. Heap-owned. */
    struct PLAIN_LIST* node;       /* Next sibling statement, or NULL if this is the last. */
    struct PLAIN_LIST* parent;     /* Enclosing node for expressions; NULL for blocks and top-level. */
    struct PLAIN_SEGMENT segment;  /* Full source text span of this node. Borrows into source. */
};
