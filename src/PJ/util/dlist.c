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

/* Function: add_head
 *
 *  Add a node to the head of the list.
 */
Errcode
add_head(Dlheader *list, Dlnode *node)
{
	if (!pj_assert(list != NULL)) return Err_bad_input;
	if (!pj_assert(node != NULL)) return Err_bad_input;
	if (!pj_assert(list->head != NULL)) return Err_internal_pointer;

	node->prev = (Dlnode *)(&list->head);
	node->next = list->head;
	node->next->prev = list->head = node;

	return Success;
}

/* Function: add_tail
 *
 *  Add a node to the tail of the list.
 */
Errcode
add_tail(Dlheader *list, Dlnode *node)
{
	if (!pj_assert(list != NULL)) return Err_bad_input;
	if (!pj_assert(node != NULL)) return Err_bad_input;
	if (!pj_assert(list->tails_prev != NULL)) return Err_internal_pointer;

	node->next = (Dlnode *)(&list->tail);
	node->prev = list->tails_prev;
	node->prev->next = list->tails_prev = node;

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
