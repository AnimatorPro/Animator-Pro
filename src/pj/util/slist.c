/* slist.c
 *
 * Singly-linked list.
 *
 * Note: NULL is a perfectly good empty list.
 */

#include "linklist.h"
#include "memory.h"
#include "pjassert.h"

#ifndef NDEBUG
#define SLIST_SAFETY
#endif

/* Function: free_slist
 *
 *  Free a singly-linked list, assuming the nodes are simple.
 */
void
free_slist(Slnode *list)
{
	while (list != NULL) {
		Slnode *next = list->next;
		pj_free(list);
		list = next;
	}
}

/* Function: join_slists
 *
 *  Join two lists (or nodes).
 */
void *
join_slists(Slnode *s1, Slnode *s2)
{
	Slnode *t;

	if (s1 == NULL)
		return s2;

	if (s2 == NULL)
		return s1;

	t = slist_last(s1);
	t->next = s2;
	return s1;
}

/* Function: remove_el
 *
 *  Remove an element from the list.
 *  Returns the new list head, or NULL if the list is now empty.
 */
void *
remove_el(Slnode *list, Slnode *el)
{
	Slnode tnode;
	Slnode *prev;
	Slnode *next;

	if (!pj_assert(el != NULL)) return NULL;

	/* Backup with tnode to handle the NULL case,
	 * and the case where el is the first element.
	 */
	tnode.next = list;
	next = &tnode;

	for (;;) {
		prev = next;
		next = next->next;

		if (next == NULL)
			break;

		if (next == el) {
			prev->next = next->next;

#ifdef SLIST_SAFETY
			next->next = NULL;
#endif
			break;
		}
	}

	return tnode.next;
}

/* Function: slist_ix
 *
 *  Returns the list "index" of the given element, or -1 if not found.
 */
int
slist_ix(const Slnode *list, const Slnode *el)
{
	int ix = 0;

	if (!pj_assert(el != NULL)) return -1;

	while (list != NULL) {
		if (list == el)
			return ix;

		ix++;
		list = list->next;
	}

	return -1;
}

/* Function: slist_el
 *
 *  Returns the ix-th element in the list, or NULL if no such index.
 */
void *
slist_el(Slnode *list, int ix)
{
	while (list != NULL) {
		if (ix <= 0)
			break;

		ix--;
		list = list->next;
	}

	return list;
}

/* Function: slist_len
 *
 *  Returns the number of elements in the list.
 */
int
slist_len(const Slnode *list)
{
	int count = 0;

	while (list != NULL) {
		count++;
		list = list->next;
	}

	return count;
}

/* Function: slist_last
 *
 *  Returns the last element in the list, or NULL if list is empty.
 */
void *
slist_last(Slnode *list)
{
	Slnode *last = list;

	while (list != NULL) {
		last = list;
		list = list->next;
	}

	return last;
}
