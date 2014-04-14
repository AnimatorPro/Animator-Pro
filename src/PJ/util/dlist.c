/* dlist.c
 *
 * Doubly-linked list.
 *
 * Note: Dlheader.tail is always NULL.
 *
 * Dlheader.head and Dlheader.tail forms the first Dlnode:
 * next = head, prev = tail = NULL.
 *
 * Dlheader.tail and Dlheader.tails_prev forms the last Dlnode:
 * next = tail = NULL, prev = tails_prev.
 */

#include "pjassert.h"
#include "errcodes.h"
#include "linklist.h"
#include "memory.h"

#ifndef NDEBUG
#define DLIST_SAFETY
#endif

/*--------------------------------------------------------------*/
/* Rank 0: list operations.                                     */
/*--------------------------------------------------------------*/

/* Function: init_list
 *
 *  Initialise a doubly-linked list.
 */
Errcode
init_list(Dlheader *list)
{
	if (!pj_assert(list != NULL)) return Err_bad_input;

	list->head = (Dlnode *)&(list->tail);
	list->tail = NULL;
	list->tails_prev = (Dlnode *)&(list->head);

	return Success;
}

/* Function: free_dl_list
 *
 *  Free a doubly-linked list, assuming the nodes are simple.
 */
Errcode
free_dl_list(Dlheader *list)
{
	Dlnode *node;
	Dlnode *nextnode;

	if (!pj_assert(list != NULL)) return Err_bad_input;
	if (!pj_assert(list->tails_prev != NULL)) return Err_internal_pointer;

	for (node = list->tails_prev; node->prev != NULL; node = nextnode) {
		nextnode = node->prev;
		pj_free(node);
	}

	return init_list(list);
}

/*--------------------------------------------------------------*/
/* Rank 1: node-node.                                           */
/*--------------------------------------------------------------*/

/* Function: insert_after
 *
 *  Insert node into the list after lnode.
 */
Errcode
insert_after(Dlnode *node, Dlnode *lnode)
{
	if (!pj_assert(node != NULL)) return Err_bad_input;
	if (!pj_assert(lnode != NULL)) return Err_bad_input;
	if (!pj_assert(lnode->next != NULL)) return Err_internal_pointer;

	node->prev = lnode;
	node->next = lnode->next;
	node->next->prev = lnode->next = node;

	return Success;
}

/* Function: insert_before
 *
 *  Insert node into the list before lnode.
 */
Errcode
insert_before(Dlnode *node, Dlnode *lnode)
{
	if (!pj_assert(node != NULL)) return Err_bad_input;
	if (!pj_assert(lnode != NULL)) return Err_bad_input;
	if (!pj_assert(lnode->prev != NULL)) return Err_internal_pointer;

	node->next = lnode;
	node->prev = lnode->prev;
	node->prev->next = lnode->prev = node;

	return Success;
}

/* Function: rem_node
 *
 *  Remove a node from the list.
 */
Errcode
rem_node(Dlnode *node)
{
	if (!pj_assert(node != NULL)) return Err_bad_input;
	if (!pj_assert(node->next != NULL)) return Err_internal_pointer;
	if (!pj_assert(node->prev != NULL)) return Err_internal_pointer;

	node->prev->next = node->next;
	node->next->prev = node->prev;

#ifdef DLIST_SAFETY
	node->next = NULL;
	node->prev = NULL;
#endif

	return Success;
}

/*--------------------------------------------------------------*/
/* Rank 2: list-node.                                           */
/*--------------------------------------------------------------*/

/* Function: add_head
 *
 *  Add a node to the head of the list.
 */
Errcode
add_head(Dlheader *list, Dlnode *node)
{
	if (!pj_assert(list != NULL)) return Err_bad_input;

	return insert_after(node, (Dlnode *)&(list->head));
}

/* Function: add_tail
 *
 *  Add a node to the tail of the list.
 */
Errcode
add_tail(Dlheader *list, Dlnode *node)
{
	if (!pj_assert(list != NULL)) return Err_bad_input;

	return insert_before(node, (Dlnode *)&(list->tail));
}

/* Function: see_head
 *
 *  Returns the head of the list.
 */
Dlnode *
see_head(Dlheader *list)
{
	if (!pj_assert(list != NULL)) return NULL;

	if ((Dlnode *)&(list->head) == list->tails_prev)
		return NULL;

	return list->head;
}

/* Function: see_tail
 *
 *  Returns the tail of the list.
 */
Dlnode *
see_tail(Dlheader *list)
{
	if (!pj_assert(list != NULL)) return NULL;

	if ((Dlnode *)&(list->head) == list->tails_prev)
		return NULL;

	return list->tails_prev;
}

/* Function: get_head
 *
 *  Remove and return the head of the list.
 */
Dlnode *
get_head(Dlheader *list)
{
	Dlnode *node;

	node = see_head(list);
	if (node != NULL)
		rem_node(node);

	return node;
}

/* Function: get_tail
 *
 *  Remove and return the tail of the list.
 */
Dlnode *
get_tail(Dlheader *list)
{
	Dlnode *node;

	node = see_tail(list);
	if (node != NULL)
		rem_node(node);

	return node;
}

/*--------------------------------------------------------------*/
/* Rank 3: list-list.                                           */
/*--------------------------------------------------------------*/

/* Function: list_tohead
 *
 *  Appends fromlist to the head of tolist.
 */
Errcode
list_tohead(Dlheader *fromlist, Dlheader *tolist)
{
	Dlnode *fromtail;

	if (!pj_assert(fromlist != NULL)) return Err_bad_input;
	if (!pj_assert(tolist != NULL)) return Err_bad_input;
	if (!pj_assert(fromlist->head != NULL)) return Err_internal_pointer;
	if (!pj_assert(tolist->head != NULL)) return Err_internal_pointer;

	fromtail = see_tail(fromlist);

	/* Nothing to move. */
	if (fromtail == NULL)
		return Success;

	/* Link fromtail to tohead. */
	fromtail->next = tolist->head;
	tolist->head->prev = fromtail;

	/* Link fromhead onto tolist->head. */
	tolist->head = fromlist->head;
	tolist->head->prev = (Dlnode *)&(tolist->head);

	/* Clear fromlist. */
	init_list(fromlist);

	return Success;
}

/* Function: list_totail
 *
 *  Appends fromlist to the tail of tolist.
 */
Errcode
list_totail(Dlheader *fromlist, Dlheader *tolist)
{
	Dlnode *fromhead;

	if (!pj_assert(fromlist != NULL)) return Err_bad_input;
	if (!pj_assert(tolist != NULL)) return Err_bad_input;
	if (!pj_assert(fromlist->tails_prev != NULL)) return Err_internal_pointer;
	if (!pj_assert(tolist->tails_prev != NULL)) return Err_internal_pointer;

	fromhead = see_head(fromlist);

	/* Nothing to move. */
	if (fromhead == NULL)
		return Success;

	/* Link fromhead to totail. */
	fromhead->prev = tolist->tails_prev;
	fromhead->prev->next = fromhead;

	/* Link fromtail to tolist->tail. */
	tolist->tails_prev = fromlist->tails_prev;
	tolist->tails_prev->next = (Dlnode *)&(tolist->tail);

	/* Clear fromlist. */
	init_list(fromlist);

	return Success;
}
