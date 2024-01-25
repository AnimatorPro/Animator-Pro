/*****************************************************************************
 *
 * linklist.c - utilities for handling singly and doubly linked lists.
 *
 *	Note:  If DLL_SAFETY is #define'd, we include extra code to set the
 *		   cur->next pointer to NULL when removing an element from a list.
 *
 * MAINTENANCE
 *	08/19/90	(Ian)
 *  			Routines in here are not properly commented yet, but some
 *				old-style function declarations were changed to ANSI/C++
 *				style.  Also, some occurances of the variable name 'this'
 *				were changed to 'cur' or 'new' (as appropriate), because
 *				apparently 'this' is a reserved word in C++.
 *	08/22/90	(Ian)
 *				Occurances of 'SAFETY' were changed to 'DLL_SAFETY', to
 *				match what is coded in Jim's LINKLIST.H.  Also, LINKLIST.H
 *				was updated to reflect the C++ style declaration changes.
 *  08/27/90	(Ian)
 *				Put #ifdef DEADWOOD/#endif around all the code in this 
 *				module except the four routines actually used by poco now:
 *					rem_node()  add_head()  init_list()  join_slists()
 *				This is to shrink the exe size so it can run under TPROF.
 *
 ****************************************************************************/

#include <string.h>
#include "poco.h"
#include "linklist.h"

/***********************************************************/
/* DOUBLY linked list (dl) calls */
/***********************************************************/
#ifdef DEADWOOD

/**********************************************************/
#ifdef DLL_SAFETY
void safe_rem_node(register Dlnode *node)
{
	if(node->next == NULL)
		return;
	node->prev->next = node->next;
	node->next->prev = node->prev;
	node->next = NULL;
}
#endif /* DLL_SAFETY */
/***********************************************************/
/* returns length of doubly linked list */

LONG listlen(Dlheader *list)
{
register Dlnode *node;
register Dlnode *next;
register LONG len;

	for(node = list->head, len = 0;
		NULL != (next = node->next);
		node = next)
	{
		++len;
	}
	return(len);
}
/***********************************************************/
/* returns header node is attached to 
 * (if DLL_SAFETY returns NULL if not attached) */

Dlheader *find_header(register Dlnode *node)
{
#ifdef DLL_SAFETY
	if(node->next == NULL)
		return(NULL);
#endif /* DLL_SAFETY */

	while(node->prev != NULL)
		node = node->prev;
	return((Dlheader *)node);
}
/****************************************************************/
Boolean isin_list(register Dlnode *testnode,Dlheader *list)

/* returns 1 if node found in list 0 if not */
{
register Dlnode *node;

	node = list->head;
	while(node->next != NULL)
	{
		if(testnode == node)
			return(1);
		node = node->next;
	}
	return(0);
}
/****************************************************************/
/* inserts a node in a list given compare function and the list
 * only works if list is in sorted order using this compare function */

void insert_compare(register Dlnode *node,
					Dlheader *list,
					int (*compare)(void *a, void *b))
/* FUNC a function that is called given *node A and *node B
 * compare(Dlnode *nodea Dlnode *nodeb) returns 0 if
 * A == B, < 0 if A < B, > 0 if A > B */
{
register Dlnode *lnode; /* current list node */
register Dlnode *nextnode;

	for(lnode = list->tails_prev;
	    NULL != (nextnode = lnode->prev);
		lnode = nextnode)
	{
		if((*compare)(node,lnode) > 0)
			break;
	}
	insert_after(node,lnode);
}
/*************************************************************/
/* insert sorts a list given compare function and the list
 * list will be in ascending order based on compare() */

void isort_list(Dlheader *list, FUNC compare)

/* a function that is called given *node A and *node B
 * compare(Dlnode *nodea Dlnode *nodeb) returns 0 if
 * A == B, < 0 if A < B, > 0 if A > B */
{
Dlheader tlist; /* tempory list */
Dlnode *node;

	init_list(&tlist);
	list_tohead(list,&tlist);

	while(NULL != (node = get_head(&tlist)))
		insert_compare(node,list,compare);
}

/* some sort routines.  Why here??? Why not??? */
void sort_indarray(void **array, LONG count, int (*cmp)(void *a, void *b))

/* a little shell on an array of indirect pointers to things.
 * takes a function that is like strcmp() to compare things pointed to */
{
register void **pt1, **pt2;
register void *swap;
register LONG swaps;
register LONG space, ct;

	if (count < 2)  /*very short arrays are already sorted*/
		return;

	space = count/2;
	--count; /* since look at two elements at once...*/
	for (;;)
	{
		swaps = 1;
		while (swaps)
		{
			pt1 = array;
			pt2 = array + space;
			ct = count - space + 1;
			swaps = 0;
			while (--ct >= 0)
			{
				if ((*cmp)(*pt1, *pt2) < 0)
				{
					swaps = 1;
					swap = *pt1;
					*pt1 = *pt2;
					*pt2 = swap;
				}
				pt1++;
				pt2++;
			}
		}
		if ( (space /= 2) == 0)
			break;
	}
}

void *sort_slist(register Slnode *list, FUNC cmp)
{
register void **array, **array_pt;
register Slnode *pt;
register int elements, i;

	elements = slist_len(list);
	if (elements <= 1)
		return(list);	/* length 0 or 1 lists already sorted */

	array = pj_malloc( elements * sizeof(void *));
	if (array)
	{
		pt = list;
		array_pt = array;
		while ( pt )
		{
			*array_pt++ = pt;
			pt = pt->next;
		}
		sort_indarray(array, elements, cmp);
		array_pt = array;
		list = NULL;
		i = elements;
		while (--i >= 0)
		{
			pt = *array_pt++;
			pt->next = list;
			list = pt;
		}
		pj_free( array );
	}
	return(list);
}


static int cmp_names(Names *l1, Names *l2)
{
	return(strcmp(l1->name, l2->name) );
}

Names *sort_names(register Names *list)
{
	return((Names *)sort_slist((Slnode *)list, cmp_names));
}

#endif /* DEADWOOD */

